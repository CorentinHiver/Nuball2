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

// int i = 0; 

void DataAlignement(bool overwrite = true)
{
#ifdef MULTI
    MTObject::Initialise(MULTI);
#endif //MULTI

  Timer timer;

  // Parameters :

  std::unordered_map<std::string, int> minX = {
    {"bgo", 100},
    {"ge" , 50}
  }; 
  std::unordered_map<std::string, int> maxX = {
    {"bgo", 10000},
    {"ge" , 6000 }
  };
  std::unordered_map<std::string, int> rebining = {
    {"bgo", 20},
    {"ge" , 1}
  };
  
  std::unordered_map<std::string, MinVar> b = {
    {"bgo", {0, 20  , 10}},
    {"ge" , {0, 2   , 10}}
  };
  std::unordered_map<std::string, MinVar> a = {
    {"bgo", {1, 1e-1, 10}},
    {"ge" , {1, 5e-3, 10}}
  };
  std::unordered_map<std::string, MinVar> C = {
    {"bgo", {1, 5e-2, 10}},
    {"ge" , {1, 5e-2, 10}}
  };
  
  Colib::Minimiser::fillHisto(true);

  // Open ref file :
  TString ref_filename = "../136/calibrate_2025/histos/run_75.root";
  
  detectors.load("../136/index_129.list");

  for (auto const & detector : detectors)
  {
    if (detector == "R3A6_blue" || detector == "R3A6_blue") continue;
    if (detector == "") continue;
    TString spectrumName = detector.c_str();
    auto outFilename = "Alignement/" + spectrumName + ".root";
    if (!overwrite && File(outFilename).exists()) {print("Do not overwrite", outFilename); continue;}
    print(outFilename);

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
    
    auto refFile = TFile::Open(ref_filename, "READ"); refFile->cd();
    auto refHistoRaw = refFile->Get<TH1F>(spectrumName);
    if (!refHistoRaw){
      print(spectrumName, "is not found");
      continue;
    }
    auto refHisto = Colib::subHisto(refHistoRaw, minX[type], maxX[type]);
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
    std::vector<double> best_bs(150, 0);
    std::vector<double> best_as(150, 1);
    std::vector<double> best_Cs(150, 1);
    std::vector<CalibAndScale> bestCalibs;
    std::vector<TH3*> first_chi2maps;
    std::vector<TH3*> final_chi2maps;
  
    FilesManager files("../136/calibrate_2025/histos/");
  
  #ifdef MULTI
    MTList MTfiles(files.get());
    MTObject::parallelise_function([&](){
  #endif //MULTI
      
    // Define the thread local stuff here :
      Colib::Chi2Calculator chi2calc(refHisto);
  
    #ifdef MULTI
      std::string filename;
      while(MTfiles.getNext(filename))
    #else // !MULTI
      for (auto const & filename : files)
    #endif // MULTI
      {
        auto file = File(filename);
        int run_number = std::stoi(split(file.filename().shortName(), "_")[1]);
        if (filename == std::string(ref_filename.Data())) continue; // Do not compare the test spectrum with itself
    
        auto testFile = TFile::Open(filename.c_str(), "READ"); testFile->cd();
        auto testHisto = Colib::subHisto(testFile->Get<TH1F>(spectrumName), minX[type], maxX[type]);
    
        testHisto->Rebin(rebining[type]);
        auto testName = testHisto->GetName() + std::string("_") + std::to_string(run_number);
        testHisto->SetName(testName.c_str());
        testHisto->Scale(refHisto->Integral()/testHisto->Integral());
  
        //////////////
        // Minimise //
        //////////////
  
        Colib::Minimiser firstMini;
        firstMini.calculate(refHisto, testHisto, b.at(type), a.at(type), C.at(type));

        auto chi2map = firstMini.getChi2Map();
        if (chi2map) 
        {
          chi2map->SetDirectory(nullptr);
          chi2map->SetName((testName+"_chi2_init").c_str());
          chi2map->SetTitle((testName+";b;a;C").c_str());
          first_chi2maps.push_back(chi2map);
        }

        auto firstCalib = firstMini.getCalib().get();
        std::unordered_map<std::string, MinVar> b2 = {
          {"bgo", {firstCalib[0], 2  , 10}},
          {"ge" , {firstCalib[0], 0.5, 10}}
        };
        std::unordered_map<std::string, MinVar> a2 = {
          {"bgo", {firstCalib[1], 1e-2, 10}},
          {"ge" , {firstCalib[1], 1e-3, 10}}
        };
        std::unordered_map<std::string, MinVar> C2 = {
          {"bgo", {firstCalib[2], 1e-2, 10}},
          {"ge" , {firstCalib[2], 1e-2, 10}}
        };
  
        Colib::Minimiser mini;
        mini.calculate(refHisto, testHisto, b2.at(type), a2.at(type), C2.at(type));
  
  #ifdef MULTI
        lock_mutex lock(MTObject::mutex); // Make the end of the loop thread safe
  #endif // MULTI
  
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
    
        testFile->Close();

        mini.getCalib().writeTo(dataFile, std::to_string(run_number));
      }
  #ifdef MULTI
    });
  #endif //MULTI
  
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

// g++ -O2 -o exec DataAlignement.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++20