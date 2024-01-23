#ifndef EVOLUTIONPEAKS_HPP
#define EVOLUTIONPEAKS_HPP

#include "../libRoot.hpp"

#include "../Analyse/SpectraCo.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/DataFile.hpp"

#include "../Modules/Calibrator.hpp"

/**
 * @brief Shows the evolution of peak position for given detectors
 * @details 
 * 
 * This class will read all the histograms all the TFiles in a folder, or just the list of histograms given.
 * 
 * Each file name has to follow this format : name_index.root
 * 
 * Then a linear calibration is applied is wanted.
 * Measures the centroid of the peak.
 * 
 * 
 */
class EvolutionPeaks
{
public:
  EvolutionPeaks() = default;
  EvolutionPeaks(int argc, char** argv) {loadParameters(argc, argv); run();}

  void printParameters() const;
  void loadParameters(int argc, char** argv);
  void setPeaks(std::vector<double> const & peaks) {m_peaks = peaks;}
  void run();

  template<class... ARGS>
  void loadCalibration(ARGS &&... args) {m_calib.load(std::forward<ARGS>(args)...);}
  void setNbFiles(int const & nb) {m_nb_files = nb;}
  void overwrite(bool const & _overwrite = true) {m_overwrite = _overwrite;}
  void setOutputName(std::string const & _output) {m_output = _output;}
  void loadRebin(File const & filename) {m_rebins.load(filename);}
  void loadDetectors(std::string const & filename){detectors.load(filename);}

private:
  Calibration m_calib;
  Calibration m_recalibration;
  Path m_dataPath;
  int m_nb_files = -1;
  bool m_overwrite = false;
  File m_output = "follow_evolution.root";

  std::vector<double> m_peaks = {511};
  
  DataFile<Label, int> m_rebins;
};

void EvolutionPeaks::printParameters() const
{
  print("");
  print("Usage of EvolutionPeaks : /path/to/data [[parameters]]");
  print("");
  print("parameters :");
  print("");
  print("-c [calibration_file]  : Loads the calibration file");
  print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-i [ID_file]           : Load ID file");
  print("-o                     : Overwrites the output");
  print("-O [output_name]       : Set the name of the output");
  exit(1);
}

void EvolutionPeaks::loadParameters(int argc, char** argv)
{
  // Handle the case of not enough parameters (0 or 1) and print the parameters :
  if (argc<2) 
  {
    print("Not enough parameters !!!");
    printParameters();
  }

  // Path of the input data :
  m_dataPath = argv [1];

  // Other parameters :
  for (int i = 2; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    else if (command == "-i") loadDetectors(argv[++i]);
    else if (command == "-o") overwrite(true);
    else if (command == "-O") setOutputName(argv[++i]);
    else if (command == "--rebin") loadRebin(argv[++i]);
    else {throw_error("Unkown command" + command);}
  }

  print("Loaded");
}

void EvolutionPeaks::run()
{
  FilesManager files(m_dataPath);
  if (!files) throw_error("DATA");
  if (m_output.exists() && !m_overwrite) throw_error(concatenate(m_output, " already exists"));

  std::vector<int> run_numbers;
  std::string file;
  while(files>>file) run_numbers.push_back(std::stoi(lastPart(removeExtension(file), '_')));
  std::vector<int> gindex; //We have to resort the vector by the index because 100 comes before 11 
  bubbleSort(run_numbers, gindex);
  std::vector<int> ordered_runs_number; for (auto const & index : gindex) ordered_runs_number.push_back(run_numbers[index]);

  auto const & first_run_number = run_numbers[gindex[0]];
  auto const & last_run_number = run_numbers[gindex.back()];
  auto const & number_of_runs = files.size();

  TH2F* evolution_BR2D1 = nullptr;
  TH1F* BR2D1_total = nullptr;
  std::vector<TGraph*> maximum_graph;
  std::vector<TGraph*> resolution_graph;
  std::vector<TGraph*> resolution_VS_maximum_graph;
  std::vector<TGraph*> resolution_VS_nb_hits_graph;

  unique_TFile totalFile(nullptr); 

  std::vector<SpectraCo> tot_histos;// Will be filled only if the tot file has been found

  // Get the informations from the first file or the total file to adjust accordingly the spectra:
  {
    Path totpath(m_dataPath+"total");
    if (totpath.exists()) 
    {
      // Load total file
      File totFilename(totpath+"fused_histo.root");
      if (totFilename.exists()) totalFile.reset(TFile::Open(totFilename.c_str()));
      totalFile->cd();
      auto histo_names = getTH1FNames(totalFile.get());
      std::vector<int> labels;
      std::vector<TH1F*> histos_total;
      
      // Load the histograms :
      for (auto const & histo_name : histo_names)
      {
        auto histo = totalFile->Get<TH1F>(histo_name.c_str());
        try {labels.push_back(string_to<Label>(histo_name));}
        catch(CastImpossible const & error)
        {
          if(detectors) labels.push_back(detectors[histo_name]);
          else throw_error(concatenate("As the histograms are labeled with names, one has to provide the correct index.list file !! "
                            "Use parameter -i [filename] or detectors.load(filename)."));
        }
        if (m_calib && m_calib.order(labels.back())>-1 && m_calib.order(labels.back())<2) 
        {
          m_calib.calibrateAxis(histo, labels.back());
          histo->GetXaxis()->SetRangeUser(0, 30000);
        }
        histo->SetName(concatenate(histo_name, "_total").c_str());
        histo->SetTitle(concatenate(histo->GetTitle(), " total").c_str());
        histos_total.push_back(histo);
        tot_histos.push_back(histo);
      }

      // Recalibrate using the peaks :
      std::string nameDerFile = "second_derivative.root";
      auto totDerivateFile = TFile::Open(nameDerFile.c_str(), "recreate");

      for (size_t spectra_i = 0; spectra_i<tot_histos.size(); ++spectra_i)
      {
        auto const & label = labels[spectra_i];
        print(label);
        if (!found(m_rebins.indexes(), ushort_cast(label))) continue;
        auto & smooth = m_rebins[label];
        auto spectra = histos_total[spectra_i];
        if (smooth>100)
        {
          int rebin = smooth/100;
          information(label, "smooth factor > 100 : smooth set to 100 and spectra rebinned by", rebin);
          spectra->Rebin(rebin);
          smooth = 100;
        }
        // auto & spectra = tot_histos[spectra_i];
        // spectra.derivate2(m_rebins[label]);

        // auto der = spectra.derivative2().createTH1F();
        CoAnalyse::removeBackground(spectra, smooth);
        totDerivateFile->cd();
        spectra->Write();
        // der->Write();
        gROOT->cd();
      }
      totDerivateFile->cd();
      totDerivateFile->Write();
      totDerivateFile->Close();
      print(nameDerFile, "written");

    }
    else 
    {
      unique_TFile file(TFile::Open(files[gindex[0]].c_str(), "READ"));
      file->cd();
      auto histo_BR2D1 = file->Get<TH1F>("R3A1_red");

      if (m_calib) m_calib.calibrateAxis(histo_BR2D1, 23);
      
      auto nbins = histo_BR2D1->GetNbinsX();
      auto min   = histo_BR2D1->GetXaxis()->GetXmin();
      auto max   = histo_BR2D1->GetXaxis()->GetXmax();

      file->Close();
      
      BR2D1_total = new TH1F("BR2D1_total","BR2D1_total", nbins,min,max);
      evolution_BR2D1 = new TH2F("evolution_BR2D1","evolution_BR2D1", number_of_runs,first_run_number,last_run_number, nbins,min,max);
    }
  }

  // std::vector<std::vector<int>> maximums(m_peaks.size());
  // auto maximums = peaks.size();

  
  // // To carefully manage scope !!!!
  // unique_TFile output(TFile::Open(m_output.c_str(), "RECREATE"));
  // gROOT->cd(); // Get ou of the output file scope for the moment
  
  // // Fill the histograms and fit the peaks :
  // for (auto const & index : gindex)
  // {// Looping through the runs

  //   // ------------------------- //
  //   // -- Extract the spectra -- //
  //   // ------------------------- //
  //   auto const & run_number = run_numbers[index];
  //   auto const & name = files[index];
  //   unique_TFile file(TFile::Open(name.c_str(), "READ"));
  //   file->cd();// Going back to the input file scope
  //   auto histo_BR2D1 = file->Get<TH1F>("R3A1_red");
  //   if (!histo_BR2D1) throw_error("CANT FIND PARIS_BR2D1");

  //   // ------------------------------ //
  //   // -- First linear calibration -- //
  //   // ------------------------------ //
  //   if (m_calib) m_calib.calibrateAxis(histo_BR2D1, 301);
  //   BR2D1_total->Add(histo_BR2D1);
  //   // histo_BR2D1->Rebin(100);
  //   CoAnalyse::removeBackground(histo_BR2D1, 100);
  //   AddTH1(evolution_BR2D1, histo_BR2D1, index);

  //   // ------------------------ //
  //   // -- Better calibration -- //
  //   // ------------------------ //
  //   auto const & binToX = histo_BR2D1->GetXaxis()->GetXmax()/histo_BR2D1->GetNbinsX();

  //   for (size_t peak_i = 0; peak_i<m_peaks.size(); ++peak_i)
  //   {
  //     auto const & peak = m_peaks[peak_i];
  //     maximum_graph.push_back(new TGraph());
  //     resolution_graph.push_back(new TGraph());
  //     resolution_VS_maximum_graph.push_back(new TGraph());
  //     resolution_VS_nb_hits_graph.push_back(new TGraph());

  //     // auto const & sigma = mean*0.03-0.00001*peak*peak;
  //     auto const & sigma = 3;
  //     // auto const & sigma = peak*(5.04375534e+01/sqrt(peak)+2.75778254e+02/peak-8.89257900e-02)/100;
  //     auto const & window = 5*sigma;
  //     auto const & min = peak-window;
  //     auto const & max = peak+window;

  //     histo_BR2D1->GetXaxis()->SetRangeUser(min, max);

  //     auto constante = histo_BR2D1->GetMaximum();
  //     auto mean = histo_BR2D1->GetMaximumBin()*binToX;

  //     TF1* gaus_pol0 = new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)");
  //     // gaus_pol0 -> SetRange(min*binToX, max*binToX);
  //     histo_BR2D1->GetXaxis()->SetRangeUser(min, max);
  //     gaus_pol0 -> SetRange(mean-window, mean+window);
  //     gaus_pol0 -> SetParameter(0, constante);
  //     gaus_pol0 -> SetParameter(1, mean);
  //     gaus_pol0 -> SetParameter(2, sigma);

  //     print(constante, mean, sigma);
  //     histo_BR2D1 -> Fit(gaus_pol0,"RQN+");
  //     print(gaus_pol0 -> GetParameter(0), gaus_pol0 -> GetParameter(1), gaus_pol0 -> GetParameter(2));
  //     print("");

  //     maximums[peak_i].push_back(gaus_pol0 -> GetParameter(1));
  //     maximum_graph[peak_i]->AddPoint(run_number, gaus_pol0 -> GetParameter(1));
  //     resolution_graph[peak_i]->AddPoint(run_number, gaus_pol0 -> GetParameter(2));
  //     resolution_VS_maximum_graph[peak_i]->AddPoint(gaus_pol0 -> GetParameter(1), gaus_pol0 -> GetParameter(2));
  //     resolution_VS_nb_hits_graph[peak_i]->AddPoint(histo_BR2D1->GetEntries(), gaus_pol0 -> GetParameter(2));
  //   }
  //   output->cd();
  //   histo_BR2D1->SetName(concatenate(histo_BR2D1->GetName(), "_", run_number).c_str());
  //   histo_BR2D1->SetTitle(concatenate(histo_BR2D1->GetName(), "_", run_number).c_str());
  //   histo_BR2D1->Write();
  //   file->cd();
  //   file->Close();
  //   gROOT->cd();
  // }

  // CoAnalyse::normalizeY(evolution_BR2D1);
  // output->cd();
  // evolution_BR2D1->Write();
  // BR2D1_total->Write();

  // for (size_t peak_i = 0; peak_i<m_peaks.size(); ++peak_i)
  // {
  //   auto const & peak = m_peaks[peak_i];
  //   maximum_graph[peak_i]->SetName(Form("Evolution_%i", (int)peak));
  //   maximum_graph[peak_i]->SetTitle(Form("Evolution_%i", (int)peak));
  //   maximum_graph[peak_i]->SetMarkerStyle(5);
  //   maximum_graph[peak_i]->SetMarkerSize(1);
  //   maximum_graph[peak_i]->SetMarkerColor(kRed);
  //   maximum_graph[peak_i]->Write();

  //   resolution_graph[peak_i]->SetName(Form("Evolution_resolution_%i", (int)peak));
  //   resolution_graph[peak_i]->SetTitle(Form("Evolution_resolution_%i", (int)peak));
  //   resolution_graph[peak_i]->SetMarkerStyle(5);
  //   resolution_graph[peak_i]->SetMarkerSize(1);
  //   resolution_graph[peak_i]->SetMarkerColor(kRed);
  //   resolution_graph[peak_i]->SetLineColor(0);
  //   resolution_graph[peak_i]->Write();
    
  //   resolution_VS_maximum_graph[peak_i]->SetName(Form("Resolution_VS_maximum%i", (int)peak));
  //   resolution_VS_maximum_graph[peak_i]->SetTitle(Form("Resolution_VS_maximum%i", (int)peak));
  //   resolution_VS_maximum_graph[peak_i]->SetMarkerStyle(5);
  //   resolution_VS_maximum_graph[peak_i]->SetMarkerSize(1);
  //   resolution_VS_maximum_graph[peak_i]->SetMarkerColor(kRed);
  //   resolution_VS_maximum_graph[peak_i]->SetLineColor(0);
  //   resolution_VS_maximum_graph[peak_i]->Write();

  //   resolution_VS_nb_hits_graph[peak_i]->SetName(Form("Resolution_VS_nb_hits%i", (int)peak));
  //   resolution_VS_nb_hits_graph[peak_i]->SetTitle(Form("Resolution_VS_nb_hits%i", (int)peak));
  //   resolution_VS_nb_hits_graph[peak_i]->SetMarkerStyle(5);
  //   resolution_VS_nb_hits_graph[peak_i]->SetMarkerSize(1);
  //   resolution_VS_nb_hits_graph[peak_i]->SetMarkerColor(kRed);
  //   resolution_VS_nb_hits_graph[peak_i]->SetLineColor(0);
  //   resolution_VS_nb_hits_graph[peak_i]->Write();
  // }

  // output->Write();
  // output->Close();
  // print(m_output, "written");
}

#endif //EVOLUTIONPEAKS_HPP