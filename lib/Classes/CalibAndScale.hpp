#ifndef CALIBANDSCALE_HPP
#define CALIBANDSCALE_HPP

#include "../libCo.hpp"
#include "InterpolatedSpectrum.hpp"

/**
 * @brief X-Calibrating and Y-Scaling a spectrum
 */
class CalibAndScale
{
public:
  // Constructors and loaders :
  CalibAndScale(){}

#ifdef ROOT_TH1
  template<class THist> CalibAndScale(THist * histo){this->setHisto(histo);}
#endif //ROOT_TH1

  void init()
  {
    m_order = m_coeffs.size()-1;
    if (!m_coeffs.empty() || m_scale != 1.) m_ok = true;
  }

  CalibAndScale(std::initializer_list<double> initList)
  {
    m_coeffs.clear();
    auto it = initList.begin();
    for (int i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
    m_scale = double_cast(*it++);
    init();
  }
  CalibAndScale& operator=(std::initializer_list<double> initList)
  {
    m_coeffs.clear();
    auto it = initList.begin();
    for (int i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
    m_scale = double_cast(*it++);
    init();
    return *this;
  }    
  void setCoeffs(std::initializer_list<double> initCoeffs)
  {
      m_coeffs.clear();
      m_coeffs = initCoeffs;
      init();
  }
  void setScale (double scale) {m_scale = scale;}

  CalibAndScale(std::vector<double> const & vec)
  {
    m_coeffs.clear();
    for (int i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
    m_scale = vec.back();
    init();
  }
  CalibAndScale& operator=(std::vector<double> const & vec)
  {
    m_coeffs.clear();
    for (int i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
    m_scale = vec.back();
    init();
    return *this;
  }
  void setCoeffs(std::vector<double> initCoeffs)
  {
    m_coeffs = initCoeffs;
    init();
  }

  CalibAndScale(CalibAndScale const & other) : 
  m_coeffs(other.m_coeffs), m_scale(other.m_scale), m_order(other.m_order), m_ok(other.m_ok)
  {
  }

  CalibAndScale& operator=(CalibAndScale const & other)
  {
    m_coeffs = other.m_coeffs;
    m_scale  = other.m_scale ;
    m_order  = other.m_order;
    return *this;
  }

#ifdef ROOT_TH1
  template<class THist>
  void setHisto(THist * histo)
  {
    m_base_histo = histo;
    m_interpol.set(histo);
  }
#endif //ROOT_TH1

  // Getters :
  std::vector<double> get() const {
      auto ret = m_coeffs;
      ret.push_back(m_scale);
      return ret;
  }
  auto const & getCoeffs() const {return m_coeffs;}
  auto const & getScale () const {return m_scale ;}

  // Methods :

  double calibrate(double const & value) const
  {
    if (m_order < 0) return value;
    double ret = 0.;
    double powered_value = 1.;
    for (int order = 0; order <= m_order; ++order)
    {
      ret += powered_value * m_coeffs[order];
      powered_value *= value;  // Compute value^order iteratively instead of using std::pow
    }
    return ret;
  }

  double linear_inv_calib(double const & value) const
  {
    if (m_order != 1) error("CalibAndScale::linear_inv_calib : order is not 1");
    return (value-m_coeffs[0])/m_coeffs[1];
  }

  double operator[](double const & bin) const
  {
    return m_interpol[this->calibrate(bin)]*m_scale;
  }

#ifdef ROOT_TH1  
  TH1F* getCalibratedHisto(std::string const & _name) const
  {
    TString name  = (m_base_histo->GetName()  + std::string("_") + _name).c_str();
    TString title = (m_base_histo->GetTitle() + std::string("_") + _name).c_str();

    if (m_order < 0) // No calibration
    {
      auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
      return ret;
    }

    auto xaxis = m_base_histo->GetXaxis();
    auto const & bins = xaxis->GetNbins();
    auto const & xmin = xaxis->GetBinLowEdge(1);
    auto const & xmax = xaxis->GetBinLowEdge(bins+1);

    if (m_order == 0) // Simple shift
    {
      auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
      shiftX(ret, m_coeffs[0]);
      return ret;
    }
    else // Calibration
    {
      auto ret = new TH1F(name, title, bins, xmin, xmax);
      for (int bin = 1; bin<=bins; ++bin) ret->SetBinContent(bin, m_interpol[this->calibrate(bin)]*m_scale);
      return ret;
    }
  }

  TH1F const * getHisto() const {return m_base_histo;}
#endif //ROOT_TH1

  void writeTo(std::string const & filename, std::string const & prepend) const
  {
    std::ofstream out(filename, std::ios::out | std::ios::app);
    out << prepend;
    for (auto const & coeff : m_coeffs) out << " " << coeff;
    out << " " << m_scale << std::endl;
  }

  void readFrom(std::string const & filename, std::string const & prepend)
  {
    std::ifstream in(filename, std::ios::in);
    std::string line;
    m_ok = false;
    m_coeffs.clear();

    while(std::getline(in, line))
    {
      if(line.substr(0, prepend.size()) == prepend)
      {
        std::istringstream iss(line.substr(prepend.size()));
        double tmp;
        while(iss >> tmp) m_coeffs.push_back(tmp);
        m_scale = m_coeffs.back();
        m_coeffs.pop_back();
        init();
      }
    }
  }

  friend std::ostream& operator<<(std::ostream& out, CalibAndScale const & calib)
  {
    out << "coeffs " << calib.m_coeffs << " scale " << calib.m_scale;
    return out;
  }

  operator bool() const & {return m_ok;}

private:
  /// @brief Shifts a histogram by 'shift' X value
  /// @param shift Shifts each bin content by 'shift' units of the x axis
  void shiftX(TH1* histo, double shift) const
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
      else temp->SetBinContent(bin_i, histo->Interpolate(shifted_value));
    }
    for (int bin_i = 1; bin_i<nb_bins+1; ++bin_i) histo->SetBinContent(bin_i, temp->GetBinContent(bin_i));
    delete temp;
  }

  bool m_ok = false;
#ifdef ROOT_TH1
  TH1F* m_base_histo = nullptr;
#endif //ROOT_TH1

  InterpolatedSpectrum m_interpol;
  std::vector<double> m_coeffs;
  double m_scale = 1.;
  int m_order = 0;
};


#endif //CALIBANDSCALE_HPP