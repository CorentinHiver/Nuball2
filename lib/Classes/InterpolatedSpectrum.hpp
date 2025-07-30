#ifndef INTERPOLATED_SPECTRUM_HPP
#define INTERPOLATED_SPECTRUM_HPP

#include "../libCo.hpp"
#include "TH1.h"

class InterpolatedSpectrum
{
public:
  InterpolatedSpectrum() noexcept = default;
  InterpolatedSpectrum(TH1 const * hist, int order = 1) noexcept {
    this -> setHisto(hist, order);
  }

  void setHisto(TH1 const * hist, int order = 1)
  {
    m_order = order;
    m_coeffs.resize(m_order + 1);
    int nbins = hist->GetNbinsX();
    for (auto & coeff : m_coeffs) coeff.resize(nbins + 1); // account for bin numbering starting at 1
  
    for (int bin = 1; bin <= nbins; ++bin)
    {
      if (m_order == 1)
      {
        auto const & coeff = this -> linInterpol(hist, bin);
        m_coeffs[0][bin] = coeff.second; // b (intercept)
        m_coeffs[1][bin] = coeff.first;  // a (slope)
      }
      else
      {
        throw_error("InterpolatedSpectrum : the only handled order is 1 so far");
      }
    }
    m_ok = true;
  }

  InterpolatedSpectrum& operator=(TH1 const * hist)
  {
    this->setHisto(hist);
    return *this;
  }

  /// @brief Calculates the linear interpolation coefficients of the spectrum between the given bin and bin+1
  /// @details If at the extrema of the spectrum, returns slope = 0 and the intercept is the maximum bin value
  /// @returns A pair <slope, intercept>
  std::pair<double, double> linInterpol(TH1 const * hist, int const & bin)
  {
    int nbins = hist->GetNbinsX();
  
    if (bin < 1 || bin >= nbins)
    { // If at the limits of the spectrum, returns slope = 0 and the intercept is the maximum bin value
      auto const & y = hist->GetBinContent(std::min(std::max(bin, 1), nbins));
      return {0.0, y};
    }
  
    auto const & y0 = hist->GetBinContent(bin);
    auto const & y1 = hist->GetBinContent(bin + 1);
  
    double a = y1 - y0;      // slope
    double b = y0 - a * bin; // intercept
  
    return {a, b}; // return (a, b)
  }
  
  double operator[](double const & new_bin) const noexcept
  {
    int bin = int_cast(new_bin);
    if (bin < 0 || bin >= int_cast(m_coeffs[0].size())) return 0.0;
  
    return m_coeffs[0][bin] + m_coeffs[1][bin] * new_bin;
  }

  operator bool() const & {return m_ok;}

private:
  size_t m_order = -1; 
  std::vector<std::vector<double>> m_coeffs;
  bool m_ok = false;
};
#endif //INTERPOLATED_SPECTRUM_HPP
