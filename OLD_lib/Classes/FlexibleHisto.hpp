#include "InterpolatedSpectrum.hpp"
#include "CalibAndScale.hpp"

class FlexibleHisto
{
public:
  FlexibleHisto() noexcept = default;
  FlexibleHisto(TH1 * histo) {this->setHisto(histo);}
  void setHisto(TH1 * histo) {m_interpol.setHisto(histo); m_base_histo = histo;}

  TH1F * getCalibratedHisto(std::string const & _name) const;
  auto const & getHisto() const {return m_base_histo;}
  auto const & getCalib() const {return m_calibnscale;}

  /// @brief Sets the calibration
  void setCalibAndScale(CalibAndScale const & calib) {m_calibnscale = calib;}

  /// @brief Gets the bin content of the calibrated and scaled histogram
  double operator[](double const & bin) const {return m_interpol[m_calibnscale.calibrate(bin)] * m_calibnscale.getScale();}

private:

  InterpolatedSpectrum m_interpol;
  CalibAndScale        m_calibnscale;

  /// @brief Shifts a histogram by 'shift' value on the x axis
  void shiftX (TH1* histo, int shift) const;

  TH1* m_base_histo = nullptr;
};

/// @brief Shifts the histogram by 'shift' value on the x axis
void FlexibleHisto::shiftX(TH1* histo, int shift) const
{
  auto temp = static_cast<TH1*> (histo->Clone(concatenate(histo->GetName(), "_shifted").c_str()));
  auto const & xmin = histo->GetXaxis()->GetXmin();
  auto const & xmax = histo->GetXaxis()->GetXmax();

  auto const & nb_bins = histo->GetNbinsX();
  for (int bin_i = 1; bin_i<nb_bins+1; ++bin_i)
  {
    auto const & value = histo->GetBinCenter(bin_i);
    auto const & shifted_value = value-shift;
    if (shifted_value < xmin|| shifted_value > xmax) 
        temp->SetBinContent(bin_i, 0);
    // I don't use yet the InterpolatedSpectrum but I should...
    else temp->SetBinContent(bin_i, histo->Interpolate(shifted_value));
  }
  for (int bin_i = 1; bin_i<nb_bins+1; ++bin_i) histo->SetBinContent(bin_i, temp->GetBinContent(bin_i));
  delete temp;
}

TH1F* FlexibleHisto::getCalibratedHisto(std::string const & _name) const
{
  TString name  = (m_base_histo->GetName()  + std::string("_") + _name).c_str();
  TString title = (m_base_histo->GetTitle() + std::string("_") + _name).c_str();

  auto const & order = m_calibnscale.order();

  if (order < 0) // No calibration
  {
    auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
    return ret;
  }

  auto xaxis = m_base_histo->GetXaxis();
  auto const & bins = xaxis->GetNbins();
  auto const & xmin = xaxis->GetBinLowEdge(1);
  auto const & xmax = xaxis->GetBinLowEdge(bins+1);

  if (order == 0) // Simple shift
  {
    auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
    shiftX(ret, m_calibnscale[0]);
    return ret;
  }
  else // Calibration
  {
    auto ret = new TH1F(name, title, bins, xmin, xmax);
    for (int bin = 1; bin<=bins; ++bin) ret->SetBinContent(bin, m_interpol[m_calibnscale.calibrate(bin)] * m_calibnscale.getScale());
    return ret;
  }
}
