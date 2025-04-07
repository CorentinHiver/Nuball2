#define DEVCO
#include "lib/libRoot.hpp"
#include "lib/Analyse/SpectraAlignator.hpp"

int main()
{
  CoLib::CalibAndScale test = {0,0.5,1};
  auto file = TFile::Open("Data_verification/data/merge_C2.root", "READ"); file->cd();
  auto d = file->Get<TH1F>("d");
  auto calibrated_lin = test(d, "lin"); calibrated_lin->SetName("lin");
  auto calibrated_quad = test(d, "quad"); calibrated_quad->SetName("quad");
  auto outFile = TFile::Open("tests.root", "RECREATE"); outFile->cd();
  d->Write("d");
  calibrated_lin->Write("cl");
  calibrated_quad->Write("cq");
  file->Close();
  outFile->Close();
  return 0;
};