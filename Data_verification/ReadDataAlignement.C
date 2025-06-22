#include "../lib/Analyse/Minimiser.hpp"
#include "../lib/libRoot.hpp"

void ReadDataAlignement()
{
  CalibAndScale calib;
  auto file = TFile::Open("Alignement/PARIS_BR2D14.root", "READ");
  auto raw_spectra = file->Get<TH2F>("spectra");
  auto corrected_spectra = new TH2F("myCorrectedSpectra", "myCorrectedSpectra", 
      raw_spectra->GetNbinsX(), raw_spectra->GetXaxis()->GetXmin(), raw_spectra->GetXaxis()->GetXmax(), 
      raw_spectra->GetNbinsY(), raw_spectra->GetYaxis()->GetXmin(), raw_spectra->GetYaxis()->GetXmax()
    );
  double max = raw_spectra->GetMaximum();
  double minx = raw_spectra->GetXaxis()->GetXmin();
  for (int run_i = 75; run_i<122; ++run_i)
  {
    auto run_i_str = std::to_string(run_i);
    print(run_i);
    calib.readFrom("Alignement/R3A1_blue.align", run_i_str);
    if (calib)
    {
      for (int bin_x = 1; bin_x<raw_spectra->GetNbinsX(); ++bin_x)
      {
        auto nb_counts = raw_spectra->GetBinContent(bin_x, run_i) / max * 1000;
        for (int count_i = 0; count_i<nb_counts; ++count_i)
        {
          double calibrated_x = calib.linear_inv_calib(bin_x+randomCo::uniform());
          corrected_spectra->Fill(calibrated_x+minx, run_i);
        }

      }
    }
  }
  new TCanvas;
  raw_spectra->Draw();
  new TCanvas;
  file->Get<TH2F>("spectraCorrected")->Draw();
  new TCanvas;
  corrected_spectra->Draw();
  // while(gPad->WaitPrimitive()) continue;
  // file->Close();
  // corrected_spectra->SetDirectory(nullptr);
  // auto outfile = TFile::Open("test.root", "recreate"); outfile->cd();
  // corrected_spectra->Write();
  // outfile->Close();
}