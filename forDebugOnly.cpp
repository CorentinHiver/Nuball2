#define DEVCO
#include "lib/libRoot.hpp"
#include "lib/Classes/FilesManager.hpp"
#include "lib/Analyse/SpectraAlignator.hpp"

int main()
{
  // auto fileRef = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); fileRef->cd();
  // auto histoRef = fileRef->Get<TH1F>("PARIS_FR1D5");

  // // CoLib::CalibAndScale test = {0,0.5,1};
  // // auto calibrated_lin = test(histo); calibrated_lin->SetName("calibrated");
  // // auto calibrated_quad = test(histo, "quad"); calibrated_quad->SetName("quad");

  // // auto file = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); file->cd();
  // // auto histo = file->Get<TH1F>("PARIS_FR1D5");
  // // CoLib::InterpolatedSpectrum inter(histo, 1);

  // auto fileTest = TFile::Open("136/calibrate_2025/histos/run_77.root", "READ"); fileTest->cd();
  // auto histoTest = fileTest->Get<TH1F>("PARIS_FR1D5");
  
  // CoLib::MinimiserVariable b = {-10, 10, 1};
  // CoLib::MinimiserVariable a = {0.9, 1.1, 0.1};
  // CoLib::MinimiserVariable C = {0.5, 1.5, 0.1};

  
  // auto outFile = TFile::Open("Tests.root", "RECREATE"); outFile->cd();
  // CoLib::Minimiser mini(histoRef, histoRef, b, a, C);
  // mini->Write();
  // // histo->Write("histo");
  // // calibrated_lin->Write("cl");
  // // calibrated_quad->Write("cq");
  // outFile->Close();
  // print("Tests.root written");

  // fileTest->Close();
  // fileRef->Close();

  


  return 0;
};