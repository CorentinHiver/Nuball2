#define MULTI 3
#ifdef MULTI
  #include "../lib/MTObjects/MTObject.hpp"
  #include "../lib/MTObjects/MTList.hpp"
#endif //MULTI

#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Timer.hpp"
#include "../lib/Classes/Detectors.hpp"
#include "../lib/Analyse/Minimiser.hpp"

#include "TTree.h"
#include "TBranch.h"
using MinVar = Colib::MinimiserVariable;

constexpr double precision = 2.;

constexpr bool more_precise = false;

/// @brief Get a sub-histogram between x1 and x2
template<class THist>
THist* subHisto(THist* histo, int xmin, int xmax)
{
  auto name = TString(histo->GetName())+("_"+std::to_string(xmin)+"_"+std::to_string(xmax)).c_str();
  auto bin_low = histo->GetBinLowEdge(xmin);
  auto bin_high = histo->GetBinLowEdge(xmax);
  auto const & N = bin_high-bin_low;
  auto ret = new THist(name, name, N, xmin, xmax);
  int dest_bin = 0;
  for (int bin_i = bin_low; bin_i<=bin_high; ++bin_i) ret->SetBinContent(dest_bin++, histo->GetBinContent(bin_i));
  return ret;
}

void DataAlignement(bool overwrite = true)
{
  Timer timer;
#ifdef MULTI
  MTObject::Initialise(MULTI);
#endif //MULTI

  // Parameters :

  std::unordered_map<std::string, int> minX = {
    {"bgo"  , 100 },
    {"ge"   , 50  },
    {"paris", 50  },
    {"dssd" , 2000}
  }; 
  std::unordered_map<std::string, int> maxX = {
    {"bgo"  , 15000},
    {"ge"   , 15000},
    {"paris", 10000},
    {"dssd" , 15000}
  };

  std::unordered_map<std::string, int> rebining = {
    {"bgo"  , 50},
    {"ge"   , 2 },
    {"paris", 10},
    {"dssd" , 50}
  };
  
  std::unordered_map<std::string, MinVar> b = {
    {"bgo"   , {0, 10  , 20*precision}},
    {"ge"    , {0, 2   , 10*precision}},
    {"dssd"  , {0, 10  , 20*precision}},
    {"paris" , {0, 2   , 20*precision}}
  };
  std::unordered_map<std::string, MinVar> a = {
    {"bgo"   , {1, 2e-1, 200*precision}},
    {"ge"    , {1, 5e-3, 20 *precision}},
    {"dssd"  , {1, 2e-1, 200*precision}},
    {"paris" , {1, 1e-2, 100*precision}}
  };
  std::unordered_map<std::string, MinVar> C = {
    {"bgo"   , {1, 1e-1, 20*precision}},
    {"ge"    , {1, 5e-2, 20*precision}},
    {"dssd"  , {1, 2e-1, 20*precision}},
    {"paris" , {1, 5e-2, 20*precision}}
  };
  
  Colib::Minimiser::fillHisto(true);

  // Open ref file :
  File ref_filename = "../136/calibrate_2025/histos/run_75.root";
  int ref_run_number = std::stoi(split(ref_filename.shortName(), "_")[1]);
  detectors.load("../136/index_129.list");

  for (auto const & detector : detectors)
  {
    if (detector == "") continue;
    TString spectrumName = detector.c_str();
    auto outFilename = "Alignement/" + spectrumName + ".root";
    if (!overwrite && File(outFilename).exists()) {print("Do not overwrite", outFilename); continue;}

    auto label = detectors[detector];
    auto type = detectors.type(label);
    if (!found(minX, type))
    {
      error(type, "dont have minX");
      continue;
    }
    File dataFile = "Alignement/"+detector+".align";
    if (dataFile.exists()) std::remove(dataFile.c_str());
    
    // Some initializations :
    
    auto refFile = TFile::Open(ref_filename.c_str(), "READ"); refFile->cd();
    auto refHistoRaw = refFile->Get<TH1F>(spectrumName);
    if (!refHistoRaw){
      print(spectrumName, "is not found");
      continue;
    }
    auto refHisto = subHisto(refHistoRaw, minX[type], maxX[type]);
    refHisto->Rebin(rebining[type]);
    refHisto->SetName(spectrumName+"_ref");
  
    auto totalHisto = dynamic_cast<TH1F*> (refHisto->Clone("totalHisto"));
    auto totalHistoCorrected = dynamic_cast<TH1F*> (refHisto->Clone("totalHistoCorrected"));
  
    refHisto            ->  SetDirectory(nullptr);
    totalHisto          ->  SetDirectory(nullptr);
    totalHistoCorrected ->  SetDirectory(nullptr);
  
    refFile->Close();
  
    // Calculate and minimize chi2 :
    
    auto spectra = new TH2F("spectra", "spectra;run;energy", refHisto->GetNbinsX(), minX[type], maxX[type], 150, 0, 150);
    spectra -> SetDirectory(nullptr);
    auto spectraCorrected = new TH2F("spectraCorrected", "spectraCorrected;run;energy", refHisto -> GetNbinsX(), minX[type], maxX[type], 150, 0, 150);
    spectraCorrected -> SetDirectory(nullptr);
    std::vector<double> init_chi2s(150, 0);
    std::vector<double> best_chi2s(150, 0);
    std::vector<double> best_first_bs(150, 0);
    std::vector<double> best_first_as(150, 1);
    std::vector<double> best_first_Cs(150, 1);
    std::vector<double> best_bs(150, 0);
    std::vector<double> best_as(150, 1);
    std::vector<double> best_Cs(150, 1);
    std::vector<CalibAndScale> bestCalibs;
    std::vector<TH3*> first_chi2maps;
    std::vector<TH3*> final_chi2maps;
  
    FilesManager files("../136/calibrate_2025/histos/");
    CalibAndScales calibs;
    for (auto const & it : a) print(it.first, it.second);
    // print(spectrumName, a);
    calibs.setCalib(ref_run_number, CalibAndScale({a.at(type).initGuess, b.at(type).initGuess, C.at(type).initGuess}));
    
  #ifdef MULTI
    MTList MTfiles (files.get());
    MTObject::parallelise_function([&](){
      
    Colib::Chi2Calculator chi2calc(refHisto);
    std::string filename;
    while(MTfiles.getNext(filename))
    {

      print(filename);

  #else // !MULTI

    Colib::Chi2Calculator chi2calc(refHisto);
    for (auto const & filename : files)
    {

  #endif //MULTI
      auto file = File(filename);
      int run_number = std::stoi(split(file.filename().shortName(), "_")[1]);
      if (file == ref_filename) continue; // Do not compare the test spectrum with itself
  
      auto testFile = TFile::Open(filename.c_str(), "READ"); testFile->cd();
      auto testHisto = subHisto(testFile->Get<TH1F>(spectrumName), minX[type], maxX[type]);
  
      testHisto->Rebin(rebining[type]);
      auto testName = testHisto->GetName() + std::string("_") + std::to_string(run_number);
      testHisto->SetName(testName.c_str());
      testHisto->Scale(refHisto->Integral()/testHisto->Integral());

    #ifdef MULTI
      MTObject::mutex.lock();
    #endif //MULTI

      for (int bin = 1; bin<=testHisto->GetNbinsX(); ++bin)
      {
        spectra->SetBinContent(bin, run_number, testHisto->GetBinContent(bin));
        totalHisto->SetBinContent(bin, totalHisto->GetBinContent(bin) + testHisto->GetBinContent(bin));
      }

      //////////////
      // Minimise //
      //////////////

      Colib::Minimiser firstMini;

    #ifdef MULTI
      MTObject::mutex.unlock();
    #endif //MULTI

      firstMini.calculate(chi2calc, testHisto, b.at(type), a.at(type), C.at(type));

      init_chi2s[run_number] = chi2calc.calculate(testHisto);

      auto chi2map = firstMini.getChi2Map();
      if (chi2map) 
      {
        chi2map->SetDirectory(nullptr);
        chi2map->SetName((testName+"_chi2_init").c_str());
        chi2map->SetTitle((testName+";b;a;C").c_str());

      #ifdef MULTI
        lock_mutex lock(MTObject::mutex);
      #endif //MULTI
        first_chi2maps.push_back(chi2map);
      }

      auto first_calib = firstMini.getCalib();

    #ifdef MULTI
      {
        lock_mutex lock(MTObject::mutex);
    #endif //MULTI

        best_first_bs[run_number] = first_calib.getCoeffs()[0];
        best_first_as[run_number] = first_calib.getCoeffs()[1];
        best_first_Cs[run_number] = first_calib.getScale();
      
        calibs.setCalib(run_number, first_calib);
    #ifdef MULTI
      }      
    #endif //MULTI

      if (more_precise)
      { // TODO Rendre thread-safe
        std::unordered_map<std::string, MinVar> b2 = {
          {"bgo", {first_calib[0], 2  , 10}},
          {"ge" , {first_calib[0], 0.5, 10}}
        };
        std::unordered_map<std::string, MinVar> a2 = {
          {"bgo", {first_calib[1], 1e-2, 10}},
          {"ge" , {first_calib[1], 1e-3, 10}}
        };
        std::unordered_map<std::string, MinVar> C2 = {
          {"bgo", {first_calib[2], 1e-2, 10}},
          {"ge" , {first_calib[2], 1e-2, 10}}
        };

        Colib::Minimiser mini;
        mini.calculate(refHisto, testHisto, b2.at(type), a2.at(type), C2.at(type));
  
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
        auto chi2map2 = mini.getChi2Map(); 
        if (chi2map2)
        {
          chi2map2->SetDirectory(nullptr);
          chi2map2->SetName((testName+"_chi2_final").c_str());
          chi2map2->SetTitle((testName+";b;a;C").c_str());
          final_chi2maps.push_back(chi2map2);
        }
        
        auto best_calib = mini.getCalib().get();
  
        best_bs[run_number] = best_calib[0];
        best_as[run_number] = best_calib[1];
        best_Cs[run_number] = best_calib[2];
        
        print(run_number, mini.getMinChi2(), best_calib);
        
        mini.getCalib().writeTo(dataFile, std::to_string(run_number));
      }
      else 
      {
        auto calibratedHisto = firstMini.getCalib().getCalibratedHisto(testName+"_best_calib");
      #ifdef MULTI
        lock_mutex lock(MTObject::mutex);
      #endif //MULTI
        for (int bin = 1; bin<=calibratedHisto->GetNbinsX(); ++bin)
        {
          spectraCorrected->SetBinContent(bin, run_number, calibratedHisto->GetBinContent(bin));
          totalHistoCorrected->SetBinContent(bin, totalHistoCorrected->GetBinContent(bin) + calibratedHisto->GetBinContent(bin));
        }
        best_chi2s[run_number] = firstMini.getMinChi2();
        print(run_number, firstMini.getMinChi2(), first_calib);
        // firstMini.getCalib().writeTo(dataFile, std::to_string(run_number)); 
      }
  
      testFile->Close();
    }
  #ifdef MULTI
    });
  #endif //MULTI

    std::ofstream datafile(dataFile);
    datafile << calibs;
  
    auto outFile = TFile::Open(outFilename, "RECREATE"); outFile->cd();
  
    auto init_chi2_evol = new TH1F("init_chi2_evol", "init_chi2_evol;run;init #chi^{2}", init_chi2s.size(), 0, init_chi2s.size());
    for (int bin = 0; bin<init_chi2s.size(); ++bin) init_chi2_evol->SetBinContent(bin+1, init_chi2s[bin]);
    init_chi2_evol->Write("init_chi2_evol");
  
    auto best_chi2_evol = new TH1F("best_chi2_evol", "best_chi2_evol;run;best #chi^{2}", best_chi2s.size(), 0, best_chi2s.size());
    for (int bin = 0; bin<best_chi2s.size(); ++bin) best_chi2_evol->SetBinContent(bin+1, best_chi2s[bin]);
    best_chi2_evol->Write("best_chi2_evol");

    
  
    auto best_first_bs_evol = new TH1F("best_first_bs_evol", "best_first_bs_evol;run;best b", best_first_bs.size(), 0, best_first_bs.size());
    for (int bin = 0; bin<best_first_bs.size(); ++bin) best_first_bs_evol->SetBinContent(bin+1, best_first_bs[bin]);
    best_first_bs_evol->Write("best_first_bs_evol");
  
    auto best_first_as_evol = new TH1F("best_first_as_evol", "best_first_as_evol;run;best a", best_first_as.size(), 0, best_first_as.size());
    for (int bin = 0; bin<best_first_as.size(); ++bin) best_first_as_evol->SetBinContent(bin+1, best_first_as[bin]);
    best_first_as_evol->Write("best_first_as_evol");
    
    auto best_first_Cs_evol = new TH1F("best_first_Cs_evol", "best_first_Cs_evol;run;best C", best_first_Cs.size(), 0, best_first_Cs.size());
    for (int bin = 0; bin<best_first_Cs.size(); ++bin) best_first_Cs_evol->SetBinContent(bin+1, best_first_Cs[bin]);
    best_first_Cs_evol->Write("best_first_Cs_evol");


  
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
  
    for (auto & histo : first_chi2maps) histo->Write();
    for (auto & histo : final_chi2maps) histo->Write();
  
    spectra->Write();
    spectraCorrected->Write();
    outFile->Close();
    print(outFilename, "written");
    print(timer());
  }
}

int main(int argc, char** argv)
{
  if (argc == 2) DataAlignement(true);
  DataAlignement(false);
  return 1;
}

// g++ -g  -o exec DataAlignement.C ` root-config --cflags` `root-config --glibs` -lSpectrum
// g++ -O2 -o exec DataAlignement.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++20