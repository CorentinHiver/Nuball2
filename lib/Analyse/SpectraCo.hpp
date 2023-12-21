#ifndef SPECTRACO_HPP
#define SPECTRACO_HPP

#include "Recalibration.hpp"
#include "../Classes/Calibration.hpp"

class SpectraCo
{
public:

  SpectraCo() = default;

  ~SpectraCo()
  {
    if (drawer) delete drawer;
  }

  SpectraCo(TH1* root_spectra)
  {
    load(root_spectra);
  }
  
  SpectraCo(std::vector<float> const & spectra, float const & min_value = 0, float const & max_value = 0, std::string name = "", std::string title = "") : 
    m_name      (name),
    m_title     (title),
    m_size      (int_cast(spectra.size())),
    m_integral  (sum(spectra)),
    m_min_value (min_value),
    m_max_value (max_value)
  {
    m_spectra.reserve(m_size);
    for (auto const & value : spectra) m_spectra.push_back(double_cast(value));
  }

  SpectraCo(std::vector<double> const & spectra, float const & min_value = 0, float const & max_value = 0, std::string name = "", std::string title = "") : 
    m_spectra   (spectra),
    m_name      (name),
    m_title     (title),
    m_size      (int_cast(spectra.size())),
    m_integral  (sum(spectra)),
    m_min_value (min_value),
    m_max_value (max_value)
  {
  }

  SpectraCo(SpectraCo const & other) : 
    m_spectra   (other.m_spectra),
    m_name      (other.m_name),
    m_title     (other.m_title),
    m_size      (other.m_size),
    m_integral  (other.m_integral),
    m_min_value (other.m_min_value),
    m_max_value (other.m_max_value)
  {
  }

  SpectraCo(SpectraCo const & other, Recalibration const & recal) : 
    m_spectra   (other.m_spectra),
    m_name      (other.m_name),
    m_title     (other.m_title),
    m_size      (other.m_size),
    m_integral  (other.m_integral),
    m_min_value (other.m_min_value),
    m_max_value (other.m_max_value)
  {
    this -> recalibrate(recal);
  }

  SpectraCo(SpectraCo const & other, Calibration const & cal, Label const & label) : 
    m_spectra   (other.m_spectra),
    m_name      (other.m_name),
    m_title     (other.m_title),
    m_size      (other.m_size),
    m_integral  (other.m_integral),
    m_min_value (other.m_min_value),
    m_max_value (other.m_max_value)
  {
    this -> calibrate(cal, label);
  }

  SpectraCo(std::vector<double> const & data) :
    m_spectra(data),
    m_size(data.size()),
    m_min_value(0),
    m_max_value(double_cast(m_size))
  {
  }

  SpectraCo& operator=(TH1* root_spectra)
  {
    load(root_spectra);
    return *this;
  }

  void load(TH1* root_spectra);

  // Getters :
  std::vector<double> const & derivativeData() {return m_derivative;}
  std::vector<double> const & derivative2Data() {return m_derivative2;}

  SpectraCo derivative() {return SpectraCo(m_derivative, m_min_value, m_max_value, m_name+"_f\'", m_title+" derivative");}
  SpectraCo derivative(std::string const & name) {return SpectraCo(m_derivative, m_min_value, m_max_value, name+"_f\'", m_title+" derivative");}
  SpectraCo derivative2() {return SpectraCo(m_derivative2, m_min_value, m_max_value, m_name+"_f\'\'", m_title+" second derivative");}
  SpectraCo derivative2(std::string const & name) {return SpectraCo(m_derivative2, m_min_value, m_max_value, name+"_f\'\'", m_title+" second derivative");}

  // Calculators :
  std::vector<double> const & derivate(uint const & smooth = 1);
  std::vector<double> const & derivate2(uint const & smooth = 1);

  // Setters :
  void setMinValue( double const & _min_value) {m_min_value = _min_value;}
  void setMaxValue( double const & _max_value) {m_max_value = _max_value;}

  auto name(std::string const & name) {return (m_name = name);}
  auto title(std::string const & title) {return (m_title = title);}

  // Getters :
  auto const & name()     const {return m_name     ;}
  auto const & title()    const {return m_title    ;}
  auto const & size()     const {return m_size     ;}
  auto const & integral() const {return m_integral ;}
  auto const & minValue() const {return m_min_value;}
  auto const & maxValue() const {return m_max_value;}
  auto const & factor()   const {return m_factor   ;}

  auto       & get()           {return m_spectra       ;}
  auto const & get()     const {return m_spectra       ;}
  auto const & lastBin() const {return m_spectra.back();}
  auto const & back()    const {return m_spectra.back();}

  // Usefull methods :
  TH1D* createTH1D(std::string newName = "", std::string newTitle = "");

  TH1F* createTH1F(std::string newName = "", std::string newTitle = "");

  // To get only the bin :
  auto const & get        (int const & bin) const {return m_spectra[bin];}
  auto const & operator[] (int const & bin) const {return m_spectra[bin];}
  auto       & operator[] (int const & bin)       {return m_spectra[bin];}

  // Using interpolation if the given bin is a double
  double interpolate(double const & bin) const ;
  double operator[] (double const & bin)       {return interpolate(bin);}
  double operator[] (double const & bin) const {return interpolate(bin);}
  
  
  SpectraCo operator+(SpectraCo const & other);
  SpectraCo operator-(SpectraCo const & other);
  /// @todo Can be optimized 
  SpectraCo operator*(double const & factor);
  /// @todo Can be optimized 
  SpectraCo operator/(double const & factor);
  void recalibrate(Recalibration const & recal);
  void calibrate(Calibration const & calib, Label const & label);

  void Draw(const char* param = "")
  {
    TH1F* drawer = createTH1F(m_name+"_drawing");
    drawer->Draw(param);
  }

  void findPeaks(int const & smooth = 1, float const & threshold = 20);

private:
  TH1F* drawer = nullptr;
  std::vector<double> m_spectra;
  std::vector<double> m_derivative;
  std::vector<double> m_derivative2; // Second derivative

  std::string m_name = "Unnamed";
  std::string m_title = "Untitled";

  std::vector<double> peaks;

  int m_size = 0;
  int m_integral = 0;
  double m_min_value = 0;
  double m_max_value = 0;
  double m_factor = 1;
};

///////////////////
// DEFINITIONS : //
///////////////////

void SpectraCo::load(TH1* root_spectra)
{
  if (root_spectra->IsA()->InheritsFrom(TH2::Class()) || root_spectra->IsA()->InheritsFrom(TH3::Class()))
  {
    throw_error("In SpectraAlignator::SpectraAlignator(TH1* root_spectra) : root_spectra is a TH2 or TH3 !!");
  }

  m_name = root_spectra->GetName();
  m_title = root_spectra->GetTitle();
  
  m_integral = root_spectra->Integral();
  m_size = root_spectra -> GetXaxis() -> GetNbins();
  m_min_value = root_spectra -> GetXaxis() -> GetBinLowEdge(0)+1;
  m_max_value = root_spectra -> GetXaxis() -> GetBinLowEdge(m_size)+1;

  m_spectra.resize(m_size);
  for (int bin = 0; bin<m_size; bin++) m_spectra[bin] = double_cast(root_spectra->GetBinContent(bin));
}

SpectraCo SpectraCo::operator+(SpectraCo const & other)
{
  if (other.m_size != m_size) throw_error("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
  SpectraCo spectra(*this);
  for (int bin = 0; bin<m_size; bin++) spectra[bin] += other[bin];
  spectra.name(spectra.name()+" + "+other.name());
  spectra.title(spectra.title()+" + "+other.title());
  return spectra;
}

/// @todo 
SpectraCo SpectraCo::operator-(SpectraCo const & other)
{
  if (other.m_size != m_size) throw_error("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
  SpectraCo spectra(*this);
  for (int bin = 0; bin<m_size; bin++) spectra[bin] -= other[bin];
  spectra.name(spectra.name()+" - "+other.name());
  spectra.title(spectra.title()+" - "+other.title());
  return spectra;
}

/// @todo 
SpectraCo SpectraCo::operator*(double const & factor)
{
  SpectraCo spectra(*this);
  for (int bin = 0; bin<m_size; bin++) spectra[bin] = spectra[bin]*factor;
  // if (m_factor!=1)// If the spectra has already been multiplied by a factor :
  // {
  //   // Check the factor displayed in the name and the title are consistent with m_factor :
  //   auto factor_str = std::to_string(m_factor);
  //   auto factor_name = spectra.name().substr(spectra.name().find_last_of("_(x"));
  // }
  // spectra.name(concatenate(spectra.name(), "_(x", factor, ")"));
  // spectra.title(concatenate(spectra.title(), " (x", factor, ")"));
  return spectra;
}

// SpectraCo SpectraCo::operator/(double const & factor)
// {
//   SpectraCo spectra(*this);
//   for (int bin = 0; bin<m_size; bin++) spectra[bin] += other[bin];
//   spectra.name(spectra.name()+" + "+other.name());
//   spectra.title(spectra.title()+" + "+other.title());
//   return spectra;
// }

void SpectraCo::recalibrate(Recalibration const & recal)
{
  std::vector<double> newSpectra(m_size);
  for (int bin = 0; bin<m_size; bin++)
  {
    auto const & new_bin = recal.calculate(bin);
    newSpectra[bin] = interpolate(new_bin);
  }
  m_spectra = newSpectra;
}

void SpectraCo::calibrate(Calibration const & calib, Label const & label)
{
  if (calib.order(label)<2)
  {
    m_min_value = calib(m_min_value, label);
    m_max_value = calib(m_max_value, label);
  }
  else 
  {
    std::vector<double> newSpectra(m_size);
    for (int bin = 0; bin<m_size; bin++)
    {
      auto const & new_bin = calib(bin, label);
      newSpectra[bin] = interpolate(new_bin);
    }
    m_spectra = newSpectra;
  }

  // replaces adc to energy if found in the name :
  replace(m_name, "adc", "Energy");
  replace(m_title, "adc", "Energy");
}

double SpectraCo::interpolate(double const & bin) const 
{
  int i = static_cast<int>(bin); //bin_i
  if (i<0) i = 0;
  else if (i > (m_size-1)) i = m_size;
  double const & a = m_spectra[i+1] - m_spectra[i];// a  =  y_i+1 - y_i
  double const & b = m_spectra[i] - a*i;           // b  =  y_i - a*bin_i
  return a*bin+b;
} 

std::vector<double> const & SpectraCo::derivate(uint const & smooth)
{
  m_derivative.resize(m_size);
  double low_sum = 0.0;
  double up_sum = 0.0;
  int lower_bin = 0;
  int upper_bin = m_size;
  for (int bin = 0; bin<m_size; bin++)
  {
    // First, sum the value of all the bins on the left :
    low_sum = 0.0;
    for (int bin_low = ((lower_bin = bin-smooth) < 1) ? 0 : lower_bin; bin_low<bin; bin_low++)
    {
      low_sum+=m_spectra[bin_low];
    }

    // Second, sum the value of all the bins on the right :
    up_sum = 0.0;
    upper_bin = ((upper_bin = bin+smooth)<(m_size+1)) ? upper_bin : m_size;
    for (int bin_up = bin; bin_up<upper_bin; bin_up++)
    {
      up_sum+=m_spectra[bin_up];
    }

    // Calculate the derivative : (sum_left - sum_right) / (x_right - x_left)
    m_derivative[bin] = (up_sum-low_sum)/(2*smooth);
  }
  return m_derivative;
}

std::vector<double> const & SpectraCo::derivate2(uint const & smooth)
{
  derivate(smooth);
  m_derivative2.resize(m_size);
  double low_sum = 0.0;
  double up_sum = 0.0;
  int lower_bin = 0;
  int upper_bin = m_size;
  for (int bin = 0; bin<m_size; bin++)
  {
    // First, sum the value of all the bins on the left :
    low_sum = 0.0;
    for (int bin_low = ((lower_bin = bin-smooth) < 1) ? 0 : lower_bin; bin_low<bin; bin_low++)
    {
      low_sum+=m_derivative[bin_low];
    }

    // Second, sum the value of all the bins on the right :
    up_sum = 0.0;
    upper_bin = ((upper_bin = bin+smooth)<(m_size+1)) ? upper_bin : m_size;
    for (int bin_up = bin; bin_up<upper_bin; bin_up++)
    {
      up_sum+=m_derivative[bin_up];
    }

    // Calculate the derivative : (sum_left - sum_right) / (x_right - x_left)
    m_derivative2[bin] = (up_sum-low_sum)/(2*smooth);
  }
  return m_derivative2;
}

TH1D* SpectraCo::createTH1D(std::string newName, std::string newTitle)
{
  if (newName == "") newName = m_name;
  if (newTitle == "") newTitle = m_title;
  TH1D* out = new TH1D(m_name.c_str(), m_title.c_str(), m_size, this->minValue(), this->maxValue());
  for (int bin = 0; bin<m_size; bin++) out->SetBinContent(bin, m_spectra[bin]);
  return out;
}

TH1F* SpectraCo::createTH1F(std::string newName, std::string newTitle)
{
  if (newName == "") newName = m_name;
  if (newTitle == "") newTitle = m_title;
  TH1F* out = new TH1F(newName.c_str(), newTitle.c_str(), m_size, this->minValue(), this->maxValue());
  for (int bin = 0; bin<m_size; bin++) out->SetBinContent(bin, m_spectra[bin]);
  return out;
}


#endif //SPECTRACO_HPP