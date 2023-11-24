#include "../Classes/SpectraAlignator.hpp"
void Macro(int nb_iterations = 20)
{
  // auto file_ref = TFile::Open("histos/run_80_matrixated.root", "READ");
  // auto file_test = TFile::Open("histos/run_120_matrixated.root", "READ");
  // // auto histo_ref = file_ref->Get<TH1F>("PARIS_BR3D2_prompt_singles");
  // auto histo_ref = file_ref->Get<TH1F>("R3A1_black_prompt_singles");
  // auto histo_test = file_test->Get<TH1F>("R3A1_black_prompt_singles");
  
  // SpectraCo spectra(histo_ref);
  // SpectraCo spectra_test(histo_test);

  // // spectra.derivate();
  // // auto derivativeData(spectra.derivative());
  // // SpectraCo derivativeSpectra(derivativeData);
  // // auto derivativeTH1F = derivativeSpectra.createTH1F();

  // spectra.derivate2(3);
  // auto secondDerivativeData(spectra.derivative2());
  // SpectraCo secondDerivativeSpectra(secondDerivativeData);

  // spectra_test.derivate2(3);
  // auto secondDerivativeData_test(spectra_test.derivative2());
  // SpectraCo secondDerivativeSpectra_test(secondDerivativeData_test);
  // // auto secondDerivativeTH1F = secondDerivativeSpectra.createTH1F();
  // // for (int i = 0; i<secondDerivativeSpectra.size(); i++) print(secondDerivativeSpectra[i]);
  // auto c = new TCanvas("test","test");
  // c->cd();
  // // SpectraCo(spectra.derivative()).createTH1F("smooth1")->Draw();

  // // spectra.derivate(2);
  // // SpectraCo(spectra.derivative()).createTH1F("smooth2")->Draw("same");

  // // spectra.derivate2(5);
  // // auto derivative = SpectraCo(spectra.derivative());
  // // derivative.setMinValue(histo_ref->GetXaxis() -> GetXmin());
  // // derivative.setMaxValue(histo_ref->GetXaxis() -> GetXmax());
  // // derivative.createTH1F("1 derivative")->Draw();
  // auto derivative2 = SpectraCo(spectra.derivative2());
  // derivative2.setMinValue(histo_ref->GetXaxis() -> GetXmin());
  // derivative2.setMaxValue(histo_ref->GetXaxis() -> GetXmax());
  // auto derivative2Spectra = derivative2.createTH1F("2 derivative ref");
  // // derivative2Spectra->SetLineColor(kBlack);
  // // derivative2Spectra->SetLineStyle(2);
  // // derivative2Spectra->Draw();

  // auto derivative2_test = SpectraCo(spectra_test.derivative2());
  // derivative2_test.setMinValue(histo_test->GetXaxis() -> GetXmin());
  // derivative2_test.setMaxValue(histo_test->GetXaxis() -> GetXmax());
  // auto derivative2Spectra_test = derivative2_test.createTH1F("2 derivative test");
  // derivative2Spectra_test->SetLineColor(kGreen);
  // // derivative2Spectra_test->SetLineStyle(2);
  // derivative2Spectra_test->Draw("same");


  // auto test_sum = derivative2-derivative2_test;
  // // test_sum.Draw("same");

  // Recalibration recal;
  // recal.seta0(0.5);
  // recal.seta1(1.5);
  // derivative2_test.recalibrate(recal);
  // // derivative2_test.Draw("same");

  // // histo_ref->SetLineColor(kRed);
  // // histo_ref->Draw("same");





  auto file_ref = TFile::Open("histos/run_80_matrixated.root", "READ");
  // auto histo_ref = file_ref->Get<TH1F>("PARIS_BR3D2_prompt_singles");
  auto histo_ref = file_ref->Get<TH1F>("R3A1_black_prompt_singles");
  // SpectraCo spectra(histo_ref);

  // auto histo_ref = file_ref->Get<TH1F>("PARIS_BR3D2_prompt_singles");
  // histo_ref->Rebin(2);
  SpectraAlignator alignator(histo_ref);
  alignator.setIterations(nb_iterations);
  alignator.setBruteForce();

  auto file_test = TFile::Open("histos/run_101_matrixated.root", "READ");
  // // auto histo_test = file_test->Get<TH1F>("R3A1_black_prompt_singles");
  auto histo_test = file_test->Get<TH1F>("PARIS_BR3D2_prompt_singles");
  // histo_test->Rebin(2);
  auto histo_test_realigned = new TH1F();
  // auto free_degrees = 4;
  // // print(argc);
  // // if (argc>3) deg = std::stoi(argv[2]);
  alignator.alignSpectra(histo_test, histo_test_realigned);
  // histo_test_realigned.recalibrate(Recalibration(0.5, 1.5));

  // auto file_out = TFile::Open("test_recal.root", "RECREATE");
  auto file_out = TFile::Open("test_derivatives.root", "RECREATE");
  file_out->cd();

  histo_ref->SetLineColor(kRed);
  histo_ref->Write();
  histo_test->SetName((histo_test->GetName()+std::string("_before")).c_str());
  histo_test->SetTitle((histo_test->GetName()+std::string("_before")).c_str());
  histo_test->SetLineStyle(2);
  histo_test->SetLineColor(kBlue);
  histo_test->Write();
  histo_test_realigned->SetLineColor(kBlue);
  histo_test_realigned->Write();
  alignator.writeChi2Spectra(file_out);

  file_out->Write();
  file_out->Close();
  print("test_recal.root written");

  file_ref->Close();
  file_test->Close();
}