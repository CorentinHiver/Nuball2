#include "../lib/libRoot.hpp"
void short_macros(int choice = 1)
{
  auto file = TFile::Open("data/merge_C2.root");
  if (!file) {error("no file"); return;}
  if (choice == 0)
  {
    // First macro : get the prompt peaks feeding the K=4- isomer of 236U
    auto dp_p = file->Get<TH2F>("dp_p");
    auto test = dp_p->ProjectionX("test",640,644);
    test->Draw();
    auto toremove = dp_p->ProjectionX("toremove");
    toremove->Draw("same");
    test->GetXaxis()->SetRangeUser(800,900);
    toremove->GetXaxis()->SetRangeUser(800,900);
    gPad->Modified();
    gPad->Update();
    pad_normalize_histos_min();
    test->GetXaxis()->UnZoom();
    toremove->GetXaxis()->UnZoom();
    gPad->Modified();
    gPad->Update();
    pad_subtract_histos();
    toremove->Delete();
    pad_show_peaks();

    new TCanvas;
    auto dp = file->Get<TH2F>("dp");
    auto test2 = dp->ProjectionX("test2",640,644);
    test2->Draw();
    auto toremove2 = dp->ProjectionX("toremove2");
    toremove2->Draw("same");
    test2->GetXaxis()->SetRangeUser(800,900);
    toremove2->GetXaxis()->SetRangeUser(800,900);
    gPad->Modified();
    gPad->Update();
    pad_normalize_histos_min();
    test2->GetXaxis()->UnZoom();
    toremove2->GetXaxis()->UnZoom();
    gPad->Modified();
    gPad->Update();
    pad_subtract_histos();
    toremove2->Delete();
    pad_show_peaks();
  }

  if (choice == 1)
  {
    // Second macro : test fourrier transform cutoff (may be used to compress histograms)
    auto d2 = file->Get<TH2F>("dp_p");
    auto d1 = d2->ProjectionX("test",640,644);
    auto d = CoAnalyse::subHisto(d1, 600, 1600);
    // auto d = file->Get<TH1F>("d");
    // Fourrier transform of a spectrum
    int n = d->GetNbinsX();
    Double_t *mydata = new Double_t[n];
    for (Int_t i = 0; i < n; i++) {mydata[i] =d->GetBinContent(i+1);}
    TVirtualFFT *fft = TVirtualFFT::FFT(1, &n, "R2C M");
    fft->SetPoints(mydata);
    fft->Transform();
    TH1 *output = 0;
    output = TH1::TransformHisto(fft, output, "RE");
    output->Draw();
    // Remove low_frequency :
    // for (Int_t i = 0; i < 10; i++) {output -> SetBinContent(i+1, 0);}
    // for (Int_t i = 0; i < 10; i++) {output -> SetBinContent(10000-i, 0);}
    double *re = new double[n];
    double *im = new double[n];
    fft->GetPointsComplex(re, im);
    
    size_t k_cutoff_high = 85; // %
    size_t k_cutoff_low = 0; // %
    for (size_t i = 0; i < size_cast(k_cutoff_high*n/100.); i++) {re[n-i-1] = 0;im[n-i-1] = 0;}
    for (size_t i = 0; i < size_cast(k_cutoff_low*n/100.) ; i++) {re[i  ] = 0;im[i  ] = 0;}
    
    // Inverse :
    TVirtualFFT *ifft = TVirtualFFT::FFT(1, &n, "C2R M"); // Complex-to-Real
    ifft->SetPointsComplex(re, im);
    ifft->Transform();
    
    double *filtered_data = new double[n];
    ifft->GetPoints(filtered_data);
    // Normalize the result correctly
    for (size_t i = 0; i < n; i++) {filtered_data[i] /= n;}
    TH1F *h_filtered = new TH1F("filtered_spectrum", "Filtered Gamma Spectrum", n, d->GetXaxis()->GetXmin(), d->GetXaxis()->GetXmax());
    for (size_t i = 0; i < n; i++) {h_filtered->SetBinContent(i + 1, filtered_data[i]);} // Normalize
    h_filtered->Draw();
    d->Draw("same");
    h_filtered->SetLineColor(kBlack);
    d->SetLineColor(kRed);
    // SubInteger(d, h_filtered)->Draw();
  }

  if (choice == 2)
  {
    // Third macro : delayed gate prompt-prompt matrices
    auto dpp_gate_642 = file->Get<TH2F>("dpp_gate_642");
    auto dpp_gate_942 = file->Get<TH2F>("dpp_gate_942");

    auto dpp_236U = dynamic_cast<TH2I*>(dpp_gate_642->Clone("dpp_236U"));
    dpp_236U->Add(dpp_gate_942);
    auto pp136 = dpp_236U->ProjectionX("pp136",135,137);
    auto pp367 = dpp_236U->ProjectionX("pp367",365,370);
    auto pp521 = dpp_236U->ProjectionX("pp521",518,522);
    auto pp1181 = dpp_236U->ProjectionX("pp1181",1179,1183);
    auto pp1912 = dpp_236U->ProjectionX("pp1912",1911,1913);
    auto pp990 = dpp_236U->ProjectionX("pp990",989,991);
    pp1181->Draw();
    auto cpk_pp1181 = CoAnalyse::counts_per_keV(pp1181, 10);
    // cpk_pp1181    
  }
  else if (choice == 3)
  {
    auto dd_p = file->Get<TH2F>("dd_p");
    auto dd_p_prompt_veto = file->Get<TH2F>("dd_p_prompt_veto");
    auto dd_p_veto_clean = Colib::removeVeto(dd_p, dd_p_prompt_veto, 505, 515);
    auto d642 = dd_p_veto_clean->ProjectionX("d642", 641, 644);
    CoAnalyse::removeBackground(d642, 15);
    auto nndc = dynamic_cast<TH1D*>(d642->Clone("nndc"));
    for (int i = 0; i<nndc->GetNbinsX(); ++i) nndc->SetBinContent(i, 0);
    Colib::simulate_peak(nndc, 103.4, 2, 88   );
    Colib::simulate_peak(nndc, 204.6, 2, 1800 );
    Colib::simulate_peak(nndc, 243.6, 2, 21   );
    Colib::simulate_peak(nndc, 300  , 2, 255  );
    Colib::simulate_peak(nndc, 308  , 2, 67   );
    Colib::shiftX(d642, -1);
    d642->GetXaxis()->SetRangeUser(0, 500);
    d642->SetTitle("gate 642 keV");
    d642->Draw();
    nndc->SetLineColor(kRed);
    nndc->SetTitle("NNDC expectations");
    nndc->Draw("same");
  }
}