#include "../../../lib/libRoot.hpp"

// J'essaie ic d'appliquer une fréquence de coupure à un spectre mais ça marche pas ...

void dft()
{
  int freqCut = -1;
  int kind = 0;
  int nRebin = 1;
  auto projection=new TH1D();
  // auto file = TFile::Open("DSSD_rings_proj.root", "READ");
  // auto matrix = file -> Get<TH2>("S1_9_DSSD_proj510");
  // CoAnalyse::projectY(matrix, projection, 1);

  auto file = TFile::Open("DSSD_final.root", "READ");
  auto matrix = file -> Get<TH2>("S1_9_DSSD");
  CoAnalyse::projectX(matrix, projection, 0, 10000);
  print(matrix->GetName(), "projected");

  projection->Rebin(nRebin);
  auto const & N = nRebin*projection->GetNbinsX();
  freqCut = (freqCut<0) ? N : freqCut;
  vector<double> input(N, 0);
  for (int i = 0; i<N; i++) input[i] = projection->GetBinContent(i);
  
  print("fourrier");
  auto transforms = new TFFTReal(N);
  transforms->SetPoints(input.data());
  transforms->Init("M", 1, &kind);
  transforms->Transform();
  vector<double> output(N, 0);
  transforms->GetPoints(output.data());
  // output.resize(freqCut);
  auto fourrierHisto = new TH1F("DFT","DFT", N, 0, N);

  print("inverse");
  auto N2 = output.size();
  auto inverseDFT = new TFFTReal(N2);
  inverseDFT->SetPoints(output.data());
  inverseDFT->Init("M", 1, &kind);
  inverseDFT->Transform();
  vector<double> inversed(N2, 0);
  inverseDFT->GetPoints(inversed.data());
  auto histoInverse = new TH1F("DFT inverse","DFT inverse", N2, 0, N2);
  for (int i = 0; i<N2; i++) histoInverse -> SetBinContent(i, inversed[i]/(2*(N-1)));

  print("fourrier2");
  int kind2 = 4;
  auto fourrier2 = new TFFTReal(N2);
  fourrier2->SetPoints(output.data());
  fourrier2->Init("M", 1, &kind2);
  fourrier2->Transform();
  vector<double> output2(N2, 0);
  fourrier2->GetPoints(output2.data());
  auto fourrierHisto2 = new TH1F("fourrier2","fourrier2", N2, 0, N2);
  for (int i = 0; i<freqCut; i++) fourrierHisto2 -> SetBinContent(i, output2[i]);
  

  print("Inversed");

  auto c = new TCanvas(("c"+to_string(nRebin)).c_str(),"c");
  c->Divide(3);
  c->cd(1);
  projection->Draw();
  histoInverse->SetLineColor(kRed);
  histoInverse->Draw("same");
  c->cd(2);
  fourrierHisto->Draw();
  c->cd(3);
  fourrierHisto2->Draw();
}