#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
// #include "../lib/Analyse/SpectraAlignator.hpp"

void DataAlignement()
{
  // Parameters :
  bool complete = true;

  int minX = 0; 
  int maxX = 5000;
  int rebining = 2;
  
  CoLib::MinimiserVariable b = {-2, 2, 100};
  CoLib::MinimiserVariable a = {0.99, 1.01, 100};
  CoLib::MinimiserVariable C = {0.98, 1.02, 10};

  TString ref_filename = "../136/calibrate_2025/histos/run_75.root";
  TString spectrumName = "R3A1_red";

  // Open ref file

  auto refFile = TFile::Open(ref_filename, "READ"); refFile->cd();
  auto refHisto = CoAnalyse::subHisto(refFile->Get<TH1F>(spectrumName), minX, maxX);
  refHisto->Rebin(rebining);
  refHisto->SetName(spectrumName+"_ref");

  auto totalHisto = dynamic_cast<TH1F*> (refHisto->Clone("totalHisto"));
  auto totalHistoCorrected = dynamic_cast<TH1F*> (refHisto->Clone("totalHistoCorrected"));

  refHisto->SetDirectory(nullptr);
  totalHisto->SetDirectory(nullptr);
  totalHistoCorrected->SetDirectory(nullptr);

  refFile->Close();

  // Calculate and minimize chi2
  
  CoLib::Chi2Calculator chi2calc(refHisto);

  auto spectra = new TH2F("spectra", "spectra;run;energy", refHisto->GetNbinsX(), minX, maxX, 150, 0, 150);
  spectra->SetDirectory(nullptr);
  auto spectraCorrected = new TH2F("spectraCorrected", "spectraCorrected;run;energy", refHisto->GetNbinsX(), minX, maxX, 150, 0, 150);
  spectraCorrected->SetDirectory(nullptr);
  std::vector<double> init_chi2s(150, 0);
  std::vector<double> best_chi2s(150, 0);
  std::vector<TH3*> chi2maps;

  FilesManager files("../136/calibrate_2025/histos/");
  // int bib = 0;// todo remove
  for (auto const & filename : files)
  {
    // if (complete) if (++bib > 2) break; // todo remove
    int run_number = std::stoi(split(File(filename).filename().shortName(), "_")[1]);
    if (filename == std::string(ref_filename)) continue; // Do not compare the test spectrum with itself

    auto testFile = TFile::Open(filename.c_str(), "READ"); testFile->cd();
    auto testHisto = CoAnalyse::subHisto(testFile->Get<TH1F>(spectrumName), minX, maxX);

    testHisto->Rebin(rebining);
    auto testName = testHisto->GetName() + std::string("_") + std::to_string(run_number);
    testHisto->SetName(testName.c_str());
    testHisto->Scale(refHisto->Integral()/testHisto->Integral());
    
    init_chi2s[run_number] = chi2calc.calculate(testHisto);
    print(run_number, init_chi2s[run_number]);

    for (int bin = 1; bin<=testHisto->GetNbinsX(); ++bin)
    {
      spectra->SetBinContent(bin, run_number, testHisto->GetBinContent(bin));
      totalHisto->SetBinContent(bin, totalHisto->GetBinContent(bin) + testHisto->GetBinContent(bin));
    }

    if (complete)
    {
      CoLib::Minimiser mini(refHisto, testHisto, b, a, C);
  
      auto calibratedHisto = mini.getCalib().getCalibratedHisto(testName+"_best_calib");
      for (int bin = 1; bin<=calibratedHisto->GetNbinsX(); ++bin)
      {
        spectraCorrected->SetBinContent(bin, run_number, testHisto->GetBinContent(bin));
        totalHistoCorrected->SetBinContent(bin, totalHistoCorrected->GetBinContent(bin) + calibratedHisto->GetBinContent(bin));
      }
  
      best_chi2s[run_number] = mini.getMinChi2();
  
      auto chi2map = mini.getChi2Map(); chi2map->SetDirectory(nullptr);
      chi2map->SetName((testName+"_chi2").c_str());
      chi2maps.push_back(chi2map);
    }

    testFile->Close();
  }

  auto outFile = TFile::Open("Alignement.root", "RECREATE"); outFile->cd();

  auto init_chi2_evol = new TH1F("init_chi2_evol", "init_chi2_evol;run;init #chi^{2}", init_chi2s.size(), 0, init_chi2s.size());
  for (int bin = 0; bin<init_chi2s.size(); ++bin) init_chi2_evol->SetBinContent(bin+1, init_chi2s[bin]);
  init_chi2_evol->Write("init_chi2_evol");

  auto best_chi2_evol = new TH1F("best_chi2_evol", "best_chi2_evol;run;best #chi^{2}", init_chi2s.size(), 0, init_chi2s.size());
  for (int bin = 0; bin<init_chi2s.size(); ++bin) best_chi2_evol->SetBinContent(bin+1, init_chi2s[bin]);
  best_chi2_evol->Write("best_chi2_evol");

  refHisto->Write();
  totalHisto->Write();
  totalHistoCorrected->Write();

  for (auto & histo : chi2maps) histo->Write();

  spectra->Write();
  spectraCorrected->Write();
  outFile->Close();
  print("Alignement.root written");
}