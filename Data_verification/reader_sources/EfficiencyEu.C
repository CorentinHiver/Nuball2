#include "../../lib/libRoot.hpp"

std::vector<double> peaks_E = {121.7817, 244.6974, 344.2785, 411.1165, 443.965, 778.9045, 867.380, 964.072, 1112.076, 1212.948, 1299.142, 1408.013 };
std::vector<double> peaks_I = {28.41   , 7.55    , 26.58   , 2.237   , 3.125  , 12.96   , 4.241  , 14.62  , 13.40   , 1.415   , 1.632   , 20.85    };

// std::vector<int>

void EfficiencyEu(std::string filename = "152Eu_bidims_source.root")
{
  auto file = TFile::Open(filename.c_str());
  auto metadata_keepsingles = file->Get<TNamed>("KeepSingles");
  if (metadata_keepsingles->GetTitle() != std::string("True")) {Colib::throw_error("KeepSingles Required");}
  auto Ge = file->Get<TH1F>("Ge_clean");
  new TCanvas;
  Ge->Draw();
  auto const & axisGe = Ge->GetXaxis();
  gPad->Update();
  std::vector<double> energies;std::vector<double> energies_err;
  std::vector<double> integrals;std::vector<double> integrals_err;
  std::vector<double> resolutions;std::vector<double> resolutions_err;
  auto const N_peaks = peaks_E.size();
  for (int peak_i = 0; peak_i<N_peaks; ++peak_i)
  {
    auto const & E = peaks_E[peak_i];
    auto const & I = peaks_I[peak_i]/100;
    axisGe->SetRangeUser(E-5, E+5);
    axisGe->SetRangeUser(Ge->GetMaximumBin()-10, Ge->GetMaximumBin()+10);
    
    TF1* gaus0(new TF1(("gaus_"+std::to_string(E)).c_str(),"gaus"));
    TF1* gaus_pol(new TF1(("gaus+pol_"+std::to_string(E)).c_str(),"gaus(0)+pol1(3)"));
    TF1* gaus_pol2(new TF1(("gaus+pol2_"+std::to_string(E)).c_str(),"gaus(0)+pol2(3)"));

    double mean = Ge->GetMean();
    double maximum = Ge->GetMaximum();
    double sigma = Ge->FindLastBinAbove(0.61*maximum) - Ge->FindFirstBinAbove(0.61*maximum);
    gaus0 -> SetParameters(maximum, mean, sigma);
    Ge -> Fit(gaus0,"QN+");
    gaus_pol->SetParameters(gaus0->GetParameter(0), gaus0->GetParameter(1), gaus0->GetParameter(2), Ge->GetBinContent(0));
    Ge -> Fit(gaus_pol,"QN+");
    gaus_pol2->SetParameters(gaus0->GetParameter(0), gaus0->GetParameter(1), gaus0->GetParameter(2), gaus0->GetParameter(3), gaus0->GetParameter(4));
    Ge -> Fit(gaus_pol2,"Q+");
    // Ge->Draw();
    // gPad->Update();
    // gPad->WaitPrimitive();
    // gPad->Update();

    maximum = gaus_pol2->GetParameter(0);
    mean = gaus_pol2->GetParameter(1);
    sigma = gaus_pol2->GetParameter(2);
    auto maximum_err = gaus_pol2->GetParError(0);
    auto mean_err = gaus_pol2->GetParError(1);
    auto sigma_err = gaus_pol2->GetParError(2);
    auto const & integral = sqrt(2*3.1415926)*maximum*sigma;
    auto const & integral_err = sqrt(2*3.1415926)*maximum_err*sigma_err;

    energies.push_back(mean);
    integrals.push_back(integral/I);
    resolutions.push_back(sigma*2.35);
    energies_err.push_back(mean_err);
    integrals_err.push_back(integral_err/I);
    resolutions_err.push_back(sigma_err*2.35);
  }

  axisGe->UnZoom();
  Ge->Draw();

  new TCanvas;
  auto R_VS_E = new TGraphErrors(N_peaks, energies.data(), resolutions.data(), energies_err.data(), resolutions_err.data());
  R_VS_E->Draw("AP");

  new TCanvas;
  auto I_VS_E = new TGraphErrors(N_peaks, energies.data(), integrals.data(), energies_err.data(), integrals_err.data());
  I_VS_E->Draw("AP");
}