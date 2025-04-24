#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Timer.hpp"
// #include "../lib/Analyse/SpectraAlignator.hpp"
// #define MT
#ifdef MT
  #include "../lib/MTObjects/MTObject.hpp"
  #include "../lib/MTObjects/MTList.hpp"
#endif //MT

using MinVar = CoLib::MinimiserVariable;

void DataAlignement()
{
  Timer timer;

  // Parameters :

  int minX = 0; 
  int maxX = 6000;
  int rebining = 1;
  
  // MinVar b = {-2, 2, 10};
  // MinVar a = {0.995, 1.005, 10};
  // MinVar C = {0.995, 1.005, 10};
  MinVar b = {0, 5e-1, 10};
  MinVar a = {1, 5e-3, 10};
  MinVar C = {1, 5e-3, 10};

  // Open ref file :
  TString ref_filename = "../136/calibrate_2025/histos/run_75.root";
  auto refFile = TFile::Open(ref_filename, "READ"); refFile->cd();

  TString spectrumName = "R3A1_red";

  // Some initializations :

#ifdef MT
  ROOT::EnableThreadSafety();
#endif //MT

  auto refHisto = CoAnalyse::subHisto(refFile->Get<TH1F>(spectrumName), minX, maxX);
  refHisto->Rebin(rebining);
  refHisto->SetName(spectrumName+"_ref");

  auto totalHisto = dynamic_cast<TH1F*> (refHisto->Clone("totalHisto"));
  auto totalHistoCorrected = dynamic_cast<TH1F*> (refHisto->Clone("totalHistoCorrected"));

  refHisto->SetDirectory(nullptr);
  totalHisto->SetDirectory(nullptr);
  totalHistoCorrected->SetDirectory(nullptr);

  refFile->Close();

  // Calculate and minimize chi2 :
  
  auto spectra = new TH2F("spectra", "spectra;run;energy", refHisto->GetNbinsX(), minX, maxX, 150, 0, 150);
  spectra->SetDirectory(nullptr);
  auto spectraCorrected = new TH2F("spectraCorrected", "spectraCorrected;run;energy", refHisto->GetNbinsX(), minX, maxX, 150, 0, 150);
  spectraCorrected->SetDirectory(nullptr);
  std::vector<double> init_chi2s(150, 0);
  std::vector<double> best_chi2s(150, 0);
  std::vector<double> best_bs(150, 0);
  std::vector<double> best_as(150, 1);
  std::vector<double> best_Cs(150, 1);
  std::vector<CoLib::CalibAndScale> bestCalibs;
  std::vector<TH3*> chi2maps;

  FilesManager files("../136/calibrate_2025/histos/");

#ifdef MT
  MTList MTfiles(files.get());
  MTObject::Initialise(3);
  MTObject::parallelise_function([&](){
#endif //MT
    
  // Define the thread local stuff here :
    CoLib::Chi2Calculator chi2calc(refHisto);

    int bib = 0;// todo remove
  #ifdef MT
    std::string filename;
    while(MTfiles.getNext(filename))
  #else // !MT
    for (auto const & filename : files)
  #endif // MT
    {
      if (++bib > 10) break;
      int run_number = std::stoi(split(File(filename).filename().shortName(), "_")[1]);
      if (filename == std::string(ref_filename)) continue; // Do not compare the test spectrum with itself
  
      auto testFile = TFile::Open(filename.c_str(), "READ"); testFile->cd();
      auto testHisto = CoAnalyse::subHisto(testFile->Get<TH1F>(spectrumName), minX, maxX);
  
      testHisto->Rebin(rebining);
      auto testName = testHisto->GetName() + std::string("_") + std::to_string(run_number);
      testHisto->SetName(testName.c_str());
      testHisto->Scale(refHisto->Integral()/testHisto->Integral());

      //////////////
      // Minimise //
      //////////////

      CoLib::Minimiser firstMini;
      firstMini.calculate(refHisto, testHisto, b, a, C);
      auto firstCalib = firstMini.getCalib().get();
      firstMini.getChi2Map()->SetName("chi2map_first");
      print(firstCalib);
      MinVar b2 = {firstCalib[0], 1e-1, 20};
      MinVar a2 = {firstCalib[1], 5e-4, 20};
      MinVar C2 = {firstCalib[2], 5e-4, 10};

      CoLib::Minimiser mini;
      mini.calculate(refHisto, testHisto, b2, a2, C2);

#ifdef MT
      lock_mutex lock(MTObject::mutex); // Make the end of the loop thread safe
#endif // MT

      init_chi2s[run_number] = chi2calc.calculate(testHisto);
  
      for (int bin = 1; bin<=testHisto->GetNbinsX(); ++bin)
      {
        spectra->SetBinContent(bin, run_number, testHisto->GetBinContent(bin));
        totalHisto->SetBinContent(bin, totalHisto->GetBinContent(bin) + testHisto->GetBinContent(bin));
      }
  
      auto calibratedHisto = mini.getCalib().getCalibratedHisto(testName+"_best_calib");
      for (int bin = 1; bin<=calibratedHisto->GetNbinsX(); ++bin)
      {
        spectraCorrected->SetBinContent(bin, run_number, calibratedHisto->GetBinContent(bin));
        totalHistoCorrected->SetBinContent(bin, totalHistoCorrected->GetBinContent(bin) + calibratedHisto->GetBinContent(bin));
      }
  
      best_chi2s[run_number] = mini.getMinChi2();
      auto chi2map = mini.getChi2Map(); chi2map->SetDirectory(nullptr);
      chi2map->SetName((testName+"_chi2").c_str());
      chi2maps.push_back(chi2map);
      
      auto best_calib = mini.getCalib().get();

      best_bs[run_number] = best_calib[0];
      best_as[run_number] = best_calib[1];
      best_Cs[run_number] = best_calib[2];

      print(run_number, mini.getMinChi2(), best_calib);
  
      testFile->Close();

      // break;
    }
#ifdef MT
  });
#endif //MT

  auto outFilename = "Alignement_" + spectrumName + ".root";
  auto outFile = TFile::Open(outFilename, "RECREATE"); outFile->cd();

  auto init_chi2_evol = new TH1F("init_chi2_evol", "init_chi2_evol;run;init #chi^{2}", init_chi2s.size(), 0, init_chi2s.size());
  for (int bin = 0; bin<init_chi2s.size(); ++bin) init_chi2_evol->SetBinContent(bin+1, init_chi2s[bin]);
  init_chi2_evol->Write("init_chi2_evol");

  auto best_chi2_evol = new TH1F("best_chi2_evol", "best_chi2_evol;run;best #chi^{2}", best_chi2s.size(), 0, best_chi2s.size());
  for (int bin = 0; bin<best_chi2s.size(); ++bin) best_chi2_evol->SetBinContent(bin+1, best_chi2s[bin]);
  best_chi2_evol->Write("best_chi2_evol");

  auto best_bs_evol = new TH1F("best_bs_evol", "best_bs_evol;run;best b", best_bs.size(), 0, best_bs.size());
  for (int bin = 0; bin<best_bs.size(); ++bin) best_bs_evol->SetBinContent(bin+1, best_bs[bin]);
  best_bs_evol->Write("best_bs_evol");

  auto best_as_evol = new TH1F("best_as_evol", "best_as_evol;run;best a", best_as.size(), 0, best_as.size());
  for (int bin = 0; bin<best_as.size(); ++bin) best_as_evol->SetBinContent(bin+1, best_as[bin]);
  best_as_evol->Write("best_as_evol");
  
  auto best_Cs_evol = new TH1F("best_Cs_evol", "best_Cs_evol;run;best C", best_Cs.size(), 0, best_Cs.size());
  for (int bin = 0; bin<best_Cs.size(); ++bin) best_Cs_evol->SetBinContent(bin+1, best_Cs[bin]);
  best_Cs_evol->Write("best_Cs_evol");

  refHisto->Write();
  totalHisto->Write();
  totalHistoCorrected->Write();

  for (auto & histo : chi2maps) histo->Write();

  spectra->Write();
  spectraCorrected->Write();
  outFile->Close();
  print(outFilename, "written");
  print(timer());
}

int main()
{
  DataAlignement();
  return 1;
}

// g++ -O2 -o exec DataAlignement.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++20 -DMT