#ifndef GEPEAK_HPP
#define GEPEAK_HPP

#include "../libRoot.hpp"
#include "../fits.hpp"

class GePeak
{
public:
  GePeak() noexcept = default;
  GePeak(TH1* hist, double min, double max) noexcept {this->fit(hist, min, max);}
  auto operator->(){return m_fit;}
  void fit(TH1* hist = nullptr, double min, double max);

private:
  TF1* gaus = new TF1("gaus", "gaus(0)+pol1(3)");
  TF1* m_fit = new TF1("GeFit", DoubleTailedStepedGaussian);
};

void GePeak::fit(TH1* hist, int min, int max)
{
  if (hist == nullptr)
  {
    if (gPad)
    { // Try to get the histo in the currently display pad
      auto const & hist_list = pad_get_histos<TH1>();
      if (!hist_list.empty()) {error("GePeak::fit : no histogram in pad "+TString(gPad->GetName())); return;}
      else if (hist_list.size()>1) {error("GePeak::fit : more than one histogram in pad "+TString(gPad->GetName())); return;}
      else hist = hist_list[0];
    }
    else {error("GePeak::fit : no histogram nor pad"); return;}
  }

  auto xaxis = hist->GetXaxis();
  auto yaxis = hist->GetYaxis();
  auto zaxis = hist->GetZaxis();

  bool is1D = (yaxis->GetNbins() == 1);
  bool is2D = (yaxis->GetNbins() > 1 && zaxis->GetNbins() == 1);
  bool is3D = zaxis->GetNbins() > 1;
  if (!is1D) {error("GePeak::fit works only in 1D histogram"); return;}

  double xpb = X_per_bin(xaxis);

  bool resize = (bin_min > 1 || bin_max>-1);
  if (resize)
  {
    xaxis->SetRange(bin_min, (bin_max>-1) ? bin_max : xaxis->GetNbins());
    gaus->SetRange(bin_min, (bin_max>-1) ? bin_max : xaxis->GetNbins());
    m_fit->SetRange(bin_min*xpb, ((bin_max>-1) ? bin_max : xaxis->GetNbins())*xpb);
  }

  double maximum = hist->GetMaximum();
  double mean = hist->GetMean();
  double sigma = (hist->FindLastBinAbove(0.61*maximum) - hist->FindFirstBinAbove(0.61*maximum));

  gaus->SetParameters(maximum, mean, sigma);
  hist->Fit(gaus, "RIQE+");

  m_fit->SetParameters(1, gaus->GetParameter(3), gaus->GetParameter(4), 0, gaus->GetParameter(0), gaus->GetParameter(1), gaus->GetParameter(2));
  m_fit->SetParameter(7, -2.);
  m_fit->SetParLimits(7, -5., -0.1);
  m_fit->SetParameter(8, 2.);
  m_fit->SetParLimits(8, 0.1, 5);
  m_fit->SetParameter(9, 0.01);
  m_fit->SetParLimits(9, -1., 1.);

  m_fit->SetParName(0, "NumberOfPeaks");
  m_fit->SetParName(1, "BkgConst");
  m_fit->SetParName(2, "BkgSlope");
  m_fit->SetParName(3, "BkgExp");
  m_fit->SetParName(4 + 0, "Height");
  m_fit->SetParName(4 + 1, "Position");
  m_fit->SetParName(4 + 2, "FWHM");
  m_fit->SetParName(4 + 3, "LeftTail");
  m_fit->SetParName(4 + 4, "RightTail");
  m_fit->SetParName(4 + 5, "AmplitudeStep");
  
  hist->Fit(m_fit, "RIE+");

}
#endif //GEPEAK_HPP