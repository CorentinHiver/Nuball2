#ifndef INTERPOLATED_SPECTRUM_HPP
#define INTERPOLATED_SPECTRUM_HPP

#include "../libCo.hpp"

class InterpolatedSpectrum
{
public:
  InterpolatedSpectrum() noexcept = default;
  InterpolatedSpectrum(TH1 const * hist, int order = 1) noexcept {
    set(hist, order);
  }

  void set(TH1 const * hist, int order = 1)
  {
    m_order = order;
    m_coeffs.resize(m_order + 1);
    int nbins = hist->GetNbinsX();
    for (auto & coeff : m_coeffs)
      coeff.resize(nbins + 2); // account for bin numbering starting at 1
  
    for (int bin = 1; bin <= nbins; ++bin)
    {
      if (m_order == 1)
      {
        auto const & coeff = linearInterpolationCoeffs(hist, bin);
        m_coeffs[0][bin] = coeff.second; // b (intercept)
        m_coeffs[1][bin] = coeff.first;  // a (slope)
      }
      else
      {
        throw_error("InterpolatedSpectrum : order must be 1");
      }
    }
    m_ok = true;
  }

  InterpolatedSpectrum& operator=(TH1 const * hist)
  {
    this->set(hist);
    return *this;
  }

  std::pair<double, double> linearInterpolationCoeffs(TH1 const * hist, int const & bin)
  {
    int nbins = hist->GetNbinsX();
  
    if (bin < 2 || bin >= nbins)
    {
      auto const & y = hist->GetBinContent(std::min(std::max(bin, 1), nbins));
      return {0.0, y};
    }
  
    auto const & y0 = hist->GetBinContent(bin);
    auto const & y1 = hist->GetBinContent(bin + 1);
  
    double a = y1 - y0;//    // slope
    double b = y0 - a * bin; // intercept
  
    return {a, b}; // return (a, b)
  }
  
  double operator[](double const & new_bin) const noexcept
  {
    int bin = int_cast(new_bin);
    if (bin < 1 || bin >= static_cast<int>(m_coeffs[0].size())) return 0.0;
  
    return m_coeffs[0][bin] + m_coeffs[1][bin] * new_bin;
  }

  operator bool() const & {return m_ok;}

private:
  size_t m_order = -1; 
  std::vector<std::vector<double>> m_coeffs;
  bool m_ok = false;
};
#endif //INTERPOLATED_SPECTRUM_HPP
