#ifndef SPECTRACO_HPP
#define SPECTRACO_HPP

#include "../libRoot.hpp"

#include "Recalibration.hpp"

#include "../Classes/Calibration.hpp"

class MyError
{public:
  MyError(std::string const & message = "") {print((message=="") ? "error" : message);}
};

using SpectraPoint = std::pair<int, double>;
using SpectraPoints = std::vector<SpectraPoint>;

class SpectraCo; // Forward declaration so the class can be used within itself
class SpectraCo
{
public:

  SpectraCo() noexcept = default;

  /// @brief @todo
  ~SpectraCo() noexcept
  {
    if (m_derivative) delete m_derivative;
    // A coder !!!!!
    // for (auto & spectra : root_spectra_pointers) if (spectra && !bool_cast(spectra->GetDirectory())) delete spectra;
  }

  SpectraCo(std::nullptr_t) noexcept
  {
    m_name = "empty";
    m_title= "empty";
    m_size = 0;
    m_integral = 0;
    m_min_x = 0;
    m_max_x = 0;
  }

  SpectraCo(std::vector<float> const & spectra, float const & min_value = 0, float const & max_value = 0, std::string name = "", std::string title = "") : 
    m_name      (name),
    m_title     (title),
    m_size      (int_cast(spectra.size())),
    m_integral  (sum(spectra)),
    m_min_x (min_value),
    m_max_x (max_value)
  {
    m_spectra.reserve(m_size);
    for (auto const & value : spectra) m_spectra.push_back(double_cast(value));
    calculateCoeff();
  }

  SpectraCo(std::vector<double> const & spectra, float const & min_value = 0, float const & max_value = 0, std::string name = "", std::string title = "") : 
    m_spectra  (spectra),
    m_name     (name),
    m_title    (title),
    m_size     (int_cast(spectra.size())),
    m_integral (sum(spectra)),
    m_min_x    (min_value),
    m_max_x    (max_value)
  {
    calculateCoeff();
  }

  SpectraCo(SpectraCo const & other, std::string name = "", std::string title = "") : 
    m_loaded_TH1  (other.m_loaded_TH1),
    m_spectra     (other.m_spectra),
    m_name        (other.m_name),
    m_title       (other.m_title),
    m_size        (other.m_size),
    m_integral    (other.m_integral),
    m_min_x       (other.m_min_x),
    m_max_x       (other.m_max_x)
  {
    if (name  != "") m_name = name;
    if (title != "") m_title = title;
    calculateCoeff();
  }

  SpectraCo(SpectraCo* other, std::string name = "", std::string title = "") : 
    m_loaded_TH1 (other->m_loaded_TH1),
    m_spectra     (other->m_spectra),
    m_name        (other->m_name),
    m_title       (other->m_title),
    m_size        (other->m_size),
    m_integral    (other->m_integral),
    m_min_x       (other->m_min_x),
    m_max_x       (other->m_max_x)
  {
    if (name != "") m_name = name;
    if (title != "") m_title = title;
    calculateCoeff();
  }

  SpectraCo(SpectraCo const & other, Recalibration const & recal) : 
    m_loaded_TH1 (other.m_loaded_TH1),
    m_spectra     (other.m_spectra),
    m_name        (other.m_name),
    m_title       (other.m_title),
    m_size        (other.m_size),
    m_integral    (other.m_integral),
    m_min_x       (other.m_min_x),
    m_max_x       (other.m_max_x)
  {
    this -> recalibrate(recal);
    calculateCoeff();
  }

  SpectraCo(SpectraCo* other, Recalibration const & recal) : 
    m_loaded_TH1 (other->m_loaded_TH1),
    m_spectra     (other->m_spectra),
    m_name        (other->m_name),
    m_title       (other->m_title),
    m_size        (other->m_size),
    m_integral    (other->m_integral),
    m_min_x       (other->m_min_x),
    m_max_x       (other->m_max_x)
  {
    this -> recalibrate(recal);
    calculateCoeff();
  }

  SpectraCo(SpectraCo const & other, Calibration const & cal, Label const & label) : 
    m_loaded_TH1 (other.m_loaded_TH1),
    m_spectra     (other.m_spectra),
    m_name        (other.m_name),
    m_title       (other.m_title),
    m_size        (other.m_size),
    m_integral    (other.m_integral),
    m_min_x       (other.m_min_x),
    m_max_x       (other.m_max_x)
  {
    this -> calibrate(cal, label);
    calculateCoeff();
  }

  SpectraCo(SpectraCo* other, Calibration const & cal, Label const & label) : 
    m_loaded_TH1 (other->m_loaded_TH1),
    m_spectra     (other->m_spectra),
    m_name        (other->m_name),
    m_title       (other->m_title),
    m_size        (other->m_size),
    m_integral    (other->m_integral),
    m_min_x       (other->m_min_x),
    m_max_x       (other->m_max_x)
  {
    this -> calibrate(cal, label);
  }

  SpectraCo(std::vector<double> const & _data) :
    m_spectra(_data),
    m_size(_data.size()),
    m_min_x(0),
    m_max_x(double_cast(m_size))
  {
    calculateCoeff();
  }

  SpectraCo(size_t const & size) : 
    m_size(size),
    m_min_x(0),
    m_max_x(double_cast(m_size))
  {
    m_spectra.resize(size);
  }

  SpectraCo(TH1* root_spectra, std::string name = "", std::string title = "")
  {
    load(root_spectra);
    if (name != "") m_name = name;
    if (title != "") m_title = title;
  }

  SpectraCo& operator=(SpectraCo const & other)
  {
    m_loaded_TH1 = other.m_loaded_TH1,
    m_spectra     = other.m_spectra;
    m_name        = other.m_name;
    m_title       = other.m_title;
    m_size        = other.m_size;
    m_integral    = other.m_integral;
    m_min_x       = other.m_min_x;
    m_max_x       = other.m_max_x;
    calculateCoeff();
    return *this;
  }

  SpectraCo& operator=(TH1* root_spectra)
  {
    load(root_spectra);
    return *this;
  }

  void load(TH1* root_spectra);
  auto const & data() const {return m_spectra;}
  auto & data() {return m_spectra;}

  // Setters :
  void setMinX(double const & _min_x) {m_min_x = _min_x; calculateCoeff();}
  void setMaxX(double const & _max_x) {m_max_x = _max_x; calculateCoeff();}
  void setRangeX(double const & _min_x, double const & _max_x) {m_min_x = _min_x; m_max_x = _max_x; calculateCoeff();}

  auto name(std::string const & name) {return (m_name = name);}
  auto title(std::string const & title) {return (m_title = title);}

  // Getters :
  auto const & name()     const {return m_name          ;}
  auto       & name()           {return m_name          ;}
  auto const & title()    const {return m_title         ;}
  auto       & title()          {return m_title         ;}
  auto const & size()     const {return m_size          ;}
  auto       & size()           {return m_size          ;}
  auto const & integral() const {return m_integral      ;}
  auto       & integral()       {return m_integral      ;}
  auto const & minX()     const {return m_min_x         ;}
  auto       & minX()           {return m_min_x         ;}
  auto const & maxX()     const {return m_max_x         ;}
  auto       & maxX()           {return m_max_x         ;}
  auto const & factor()   const {return m_factor        ;}
  auto       & factor()         {return m_factor        ;}
  auto const & lastBin()  const {return m_spectra.back();}
  auto       & lastBin()        {return m_spectra.back();}
  auto const & back()     const {return m_spectra.back();}
  auto       & back()           {return m_spectra.back();}
  auto const & get()      const {return m_spectra       ;}
  auto       & get()            {return m_spectra       ;}
  auto const & peaks()    const {return m_peaks         ;}
  auto       & peaks()          {return m_peaks         ;}
  
  // Calculators :
  SpectraCo* derivate(int smooth = 1) noexcept;
  SpectraCo* derivate2(int smooth = 1) noexcept;
  void removeBackground(int const & smooth, std::string const & fit_options = "") noexcept;
  double integralInRangeBin(int const & bin_min, int const & bin_max) noexcept;
  double integralInRange(double const & value_min, double const & value_max) noexcept;
  double meanInRangeBin(int const & bin_min, int const & bin_max) noexcept;
  double meanInRange(double const & value_min, double const & value_max) noexcept;

  // Getters :
  /// @attention One has to check that the derivative has already been calculated;
  std::vector<double> const & derivativeData() {return m_derivative->data();}
  /// @attention One has to check that the second derivative has already been calculated;
  std::vector<double> const & derivative2Data() {return m_derivative2->data();}

  SpectraCo* derivative() {return m_derivative;}
  SpectraCo* derivative(std::string const & name);

  SpectraCo* derivative2() {return m_derivative2;}
  SpectraCo* derivative2(std::string const & name);

  /**
   * @brief Inverses the spectra : negative values become positive and positive values become negative
   * @details Do not touch the derivatives
   */
  void inverse() {for (auto & value : m_spectra) value = -value;}

  int lastBinWithValue() const noexcept
  {
    int bin_it = m_size;
    while(bin_it>0) if (m_spectra[--bin_it]!=0.0) return bin_it;
    return 0;
  }

  int firstBinWithValue() const noexcept
  {
    int bin_it = 0;
    while(bin_it<m_size) if (m_spectra[bin_it++]!=0.0) return bin_it;
    return 0;
  }

  // Root interface methods :
  void fill(double const & X) noexcept;
  void draw(const char* param = "");
  /// @brief TODO
  void rebin(int const & factor, bool const & rebin_derivatives = false);
  TH1D* createTH1D(std::string newName = "", std::string newTitle = "");
  TH1F* createTH1F(std::string newName = "", std::string newTitle = "");
  
  void write() {this->createTH1F(name())->Write();}
  void write(TDirectory* directory)
  {
    auto histo = this->createTH1F(name()); 
    directory->cd(); 
    histo->Write(); 
    gROOT->cd();
  }
  void writeTH1D() {this->createTH1D(name())->Write();}
  void writeTH1D(TDirectory* directory) 
  {
    directory->cd(); 
    this->createTH1D(name())->Write(); 
    gROOT->cd();
  }

  // To get the bin's content (TH1::getBinContent):
  auto const & get        (int const & bin) const noexcept {return m_spectra[bin];}
  auto const & operator[] (int const & bin) const noexcept {return m_spectra[bin];}
  auto       & operator[] (int const & bin)       noexcept {return m_spectra[bin];}

  // Using interpolation if the given bin is a double (TH1F::Eval)
  double interpolate(double const & bin) const noexcept ;
  double operator[] (double const & bin)       noexcept {return interpolate(bin);}
  double operator[] (double const & bin) const noexcept {return interpolate(bin);}

  // Get the X value : 
  double getX(double const & bin) {return bin * (m_max_x/m_size);}
  // Get the bin corresponding for a given X value
  double getBin(double const & x) 
  {
    auto const & ret = int_cast(std::trunc(x*m_slope + m_intercept));
    return (ret>-1) ? ret : 0;
  }

  /// @brief First order calibration
  void calibrateX(double const & slope, double const & intercept = 0)
  {
    m_min_x = m_min_x * slope + intercept;
    m_max_x = m_max_x * slope + intercept;
    calculateCoeff();
    if (m_derivative) m_derivative->calibrateX(intercept, slope);
  }
  
  
  SpectraCo operator+(SpectraCo const & other);
  SpectraCo operator-(SpectraCo const & other);
  SpectraCo operator*(double const & factor);
  SpectraCo operator/(double const & factor);
  SpectraCo & operator*=(double const & factor);
  void recalibrate(Recalibration const & recal);
  void calibrate(Calibration const & calib, Label const & label);
  void calibrate(std::vector<double> const & coeffs);
  void resizeBin(size_t const & new_size);
  void resizeX(double const & maxX);
  void resizeX(double const & minX, double const & maxX);


  SpectraPoints const & findPeaks(int const & threshold, int const & nb_bins_below_threshold);
  SpectraPoints const & findPeaks(int const & threshold, int const & nb_bins_below_threshold, int const & smooth);

  // Manage resources :
  void deleteDerivative() {delete m_derivative;}

  // Other operations :

  /// @brief Calculates the chi2 between this spectra and another one
  double chi2 (SpectraCo & other)
  {
    double sum_errors_squared = 0.0;
    for (int bin = 0; bin<other.size(); bin++) if (other[bin]>0)
    {
      // Calculate the error for this bin :
      auto const & error = m_spectra[bin]-other[bin];

      // Variance of the bin :
      double const & weight = 1/other[bin]; // V = sigma² = 1/N

      // Add the error to the total squared error of the spectra :
      sum_errors_squared += error*error*weight;

    }
    return sum_errors_squared/other.size();
  }

  // Histogram manipulations :
  ///@brief Remove the bins after the last bin with content (= detector's output max range)
  void setActualRange() {this -> resizeBin(this -> lastBinWithValue());}

  class HistoNull
  {
    public:
    HistoNull() {printC(GREY, "SpectraCo::HistoNull :", RESET, RED, " loaded histogram is nullptr", RESET);}
  };

  class HistoZombie
  {
    public:
    HistoZombie() {printC(GREY, "SpectraCo::HistoZombie :", RESET, RED, " loaded histogram is zombie", RESET);}
  };

  class SizeMissmatch
  {
  public:
    SizeMissmatch() = default;
    SizeMissmatch(std::string const & message){warning(message);}
  };

private:
  void calculateCoeff();

  TH1* m_loaded_TH1 = nullptr;
  std::vector<double> m_spectra;
  SpectraCo* m_derivative = nullptr;
  SpectraCo* m_derivative2 = nullptr; // Second derivative
  SpectraPoints m_peaks;

  std::string m_name = "Unnamed";
  std::string m_title = "Untitled";

  int m_size = 0;
  int m_integral = 0;
  double m_min_x = 0;
  double m_max_x = 0;
  double m_factor = 1; // If the spectra has been multiplied or divided
  std::vector<TH1*> root_spectra_pointers;
  int nb_histo_drawn = 0;
  double m_rebin = 1;
  
  // Relation bewteen X value and bin number : bin = m_slope*X + m_intercept
  double m_slope = -1;
  double m_intercept = 0;
};

std::ostream& operator<<(std::ostream& out, SpectraCo const & spectra)
{
  out << spectra.name() << " : " << spectra.integral() << " counts";
  if(spectra.peaks().size()>0) out << ", " << spectra.peaks().size() << " peaks found" << std::endl;
  return out;
}

template<>
std::string type_of<SpectraCo>(SpectraCo const & spectra)
{
  return "SpectraCo_"+type_of(spectra.minX());
}

///////////////////
// DEFINITIONS : //
///////////////////

void SpectraCo::calculateCoeff()
{
  // Coefficients to convert X value to bin 
  m_slope = m_size/(m_max_x-m_min_x);
  m_intercept = - m_slope*m_min_x;
}

/// @brief Fills a bin of the spectra based on the X value
/// @attention Has not been tested @todo test it
void SpectraCo::fill(double const & X) noexcept
{
  int const & bin = getBin(X);
  if (bin>-1 && bin<m_size) m_spectra[bin]++;
}

/// @brief Creates a new TH1F to be drawn
void SpectraCo::draw(const char* param)
{
  auto name = concatenate(m_name, "_drawing_", nb_histo_drawn);
  nb_histo_drawn++;
  root_spectra_pointers.push_back(createTH1F(name, name));
  root_spectra_pointers.back()->Draw(param);
}

/**
 * @brief Uses the TH1::Rebin method;
 * @todo create a custom method without the use of ROOT (attempt commented at the bottom of the file)
 */
void SpectraCo::rebin(int const & factor, bool const & rebin_derivatives)
{
  m_rebin*=factor; // Saves the rebin factor
  auto histo = createTH1F(concatenate(name(), " rebinning by ", factor));
  histo->Rebin(factor);
  this->load(histo);
  if (rebin_derivatives && m_derivative) m_derivative->rebin(factor, rebin_derivatives);
}

void SpectraCo::load(TH1* root_spectra)
{
  if (!root_spectra) throw HistoNull();
  else if (root_spectra->IsZombie()) throw HistoZombie();
  if (root_spectra->IsA()->InheritsFrom(TH2::Class()) || root_spectra->IsA()->InheritsFrom(TH3::Class()))
  {
    throw_error("In SpectraCo::load(TH1* root_spectra) : root_spectra is a TH2 or TH3 !!");
  }
  m_loaded_TH1 = root_spectra;


  m_name = root_spectra->GetName();
  m_title = root_spectra->GetTitle();
  
  m_integral = root_spectra->Integral();
  m_size = root_spectra -> GetXaxis() -> GetNbins();
  m_min_x = root_spectra -> GetXaxis() -> GetBinLowEdge(1);
  m_max_x = root_spectra -> GetXaxis() -> GetBinLowEdge(m_size+1);

  m_spectra.resize(m_size);
  for (int bin = 1; bin<m_size; bin++) m_spectra[bin] = double_cast(root_spectra->GetBinContent(bin));
  calculateCoeff();
}

SpectraCo SpectraCo::operator+(SpectraCo const & other)
{
  if (other.m_size != m_size) throw SizeMissmatch("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
  SpectraCo spectra(*this);// Optimize here
  for (int bin = 0; bin<m_size; bin++) spectra[bin] += other[bin];
  spectra.name(spectra.name()+" + "+other.name());
  spectra.title(spectra.title()+" + "+other.title());
  return spectra;
}

SpectraCo SpectraCo::operator-(SpectraCo const & other)
{
  if (other.m_size != m_size) throw SizeMissmatch("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
  SpectraCo spectra(*this);// Optimize here
  for (int bin = 0; bin<m_size; bin++) spectra[bin] -= other[bin];
  spectra.name(spectra.name()+" - "+other.name());
  spectra.title(spectra.title()+" - "+other.title());
  return spectra;
}

/**
 * @brief @todo can be optimized
 */
SpectraCo SpectraCo::operator*(double const & factor)
{
  SpectraCo spectra(*this);// Optimize here
  m_factor = factor;
  for (int bin = 0; bin<m_size; bin++) spectra[bin] = spectra[bin]*m_factor;
  // TODO if time : rename the spectra based on the multiplicative factor
  // if (m_factor!=1)// If the spectra has already been multiplied by another factor :
  // {
  //   auto begin_factor_it = spectra.name().find_last_of("_(x");
  //   auto factor_name = spectra.name().substr(begin_factor_it, );
  // }
  // spectra.name(concatenate(spectra.name(), "_(x", factor, ")"));
  // spectra.title(concatenate(spectra.title(), " (x", factor, ")"));
  return spectra;
}

SpectraCo & SpectraCo::operator*=(double const & factor)
{
  for (int bin = 0; bin<m_size; bin++) m_spectra[bin] *= factor;
  return *this;
}

/**
 * @brief @todo can be optimized
 */
SpectraCo SpectraCo::operator/(double const & factor)
{
  SpectraCo spectra(*this);
  if (m_factor == 0) {error("In SpectraCo::operator/(factor) : factor is equal to zero !!"); return spectra;}
  m_factor = factor;
  for (int bin = 0; bin<m_size; bin++) spectra[bin] = spectra[bin]/m_factor;
  // TODO if time : 
  // if (m_factor!=1)// If the spectra has already been multiplied by another factor :
  // {
  //   auto begin_factor_it = spectra.name().find_last_of("_(x");
  //   auto factor_name = spectra.name().substr(begin_factor_it, );
  // }
  // spectra.name(concatenate(spectra.name(), "_(x", factor, ")"));
  // spectra.title(concatenate(spectra.title(), " (x", factor, ")"));
  return spectra;
}

void SpectraCo::recalibrate(Recalibration const & recal)
{
  std::vector<double> newSpectra(m_size);
  for (int bin = 0; bin<m_size; bin++)
  {
    auto const & new_bin = recal.calculate(bin);
    newSpectra[bin] = interpolate(new_bin);
  }
  m_spectra = newSpectra;
  if (m_derivative) m_derivative->recalibrate(recal);
  calculateCoeff();
}

void SpectraCo::calibrate(Calibration const & calib, Label const & label)
{
  if (calib.order(label)<2)
  {
    m_min_x = calib.apply(m_min_x, label);
    m_max_x = calib.apply(m_max_x, label);
  }
  else 
  {
    std::vector<double> newSpectra(m_size);
    for (int bin = 0; bin<m_size; bin++)
    {
      auto const & new_bin = calib.apply(bin, label);
      newSpectra[bin] = this->interpolate(new_bin);
    }
    m_spectra = newSpectra;
  }

  // Replaces "adc" with "Energy" if found in the name and/or title :
  replace(m_name, "adc", "Energy");
  replace(m_title, "adc", "Energy");

  if (m_derivative) m_derivative->calibrate(calib, label);
  calculateCoeff();
}

void SpectraCo::calibrate(std::vector<double> const & coeffs)
{
  int order = coeffs.size()-1;
  if (order<0) return;
  else if (order<2)
  {
    this->calibrateX(coeffs[1], coeffs[0]); // calibrateX(slope, intercept(default = 0))
  }
  else 
  {
    std::vector<double> newSpectra(m_size);
    for (int bin = 0; bin<m_size; bin++)
    {
      switch(order)
      {
        case 2 : newSpectra[bin] = this->interpolate(coeffs[0] + bin*coeffs[1] + bin*bin*coeffs[2]); break;
        case 3 : newSpectra[bin] = this->interpolate(coeffs[0] + bin*coeffs[1] + bin*bin*coeffs[2] + bin*bin*bin*coeffs[3]); break;
        default: error("SpectraCo::calibrate(vector<double> coeffs) : can't handle", order+1, "coefficients");
      }
    }
    m_spectra = newSpectra;
  }

  // Replaces "adc" with "Energy" if found in the name and/or title :
  replace(m_name, "adc", "Energy");
  replace(m_title, "adc", "Energy");

  if (m_derivative) m_derivative->calibrate(coeffs);
  calculateCoeff();
}

double SpectraCo::interpolate(double const & _bin) const noexcept
{
  int bin_i = static_cast<int>(_bin); //bin_i
  if (bin_i<0 || bin_i > (m_size-2)) return 0;
  double const & a = m_spectra[bin_i+1] - m_spectra[bin_i];// a  =  y_i+1 - y_i
  double const & b = m_spectra[bin_i]   - a*bin_i;         // b  =  y_i - a*bin_i
  return a*_bin+b;
} 

/**
 * @brief @todo Try to adapt not to have to use root for this function
 * 
 * @param smooth 
 * @param fit_options 
 */
void SpectraCo::removeBackground(int const & smooth, std::string const & fit_options) noexcept
{
  auto file = gROOT -> GetFile();
  if (file) gROOT->cd();
  auto root_histo = this->createTH1F();
  CoAnalyse::removeBackground(root_histo, smooth, fit_options);
  this->load(root_histo);
  delete root_histo;
  if (file) file->cd();
}


double SpectraCo::integralInRangeBin(int const & bin_min, int const & bin_max) noexcept
{
  double ret = 0.0;
  for (int bin_it = bin_min; bin_it<bin_max; bin_it++) ret+=m_spectra[bin_it];
  return ret;
}

double SpectraCo::integralInRange(double const & value_min, double const & value_max) noexcept
{
  double ret = 0.0; 
  int bin_min = getBin(value_min);
  int bin_max = getBin(value_max);
  for (int bin_it = bin_min; bin_it<bin_max; bin_it++) ret+=m_spectra[bin_it];
  return ret;
}


double SpectraCo::meanInRangeBin(int const & bin_min, int const & bin_max) noexcept
{
  double mean = 0.0; double integral = 0.0;
  for (int bin_it = bin_min; bin_it<bin_max; bin_it++) 
  {
    mean+=bin_it*abs(m_spectra[bin_it]);
    integral+=abs(m_spectra[bin_it]);
  }
  mean/=integralInRangeBin(bin_min, bin_max);
  return this->getX(mean);
}

double SpectraCo::meanInRange(double const & value_min, double const & value_max) noexcept
{
  double mean = 0.0; double integral = 0.0;
  int bin_min = getBin(value_min);
  int bin_max = getBin(value_max);
  for (int bin_it = bin_min; bin_it<bin_max; bin_it++) 
  {
    mean+=bin_it*abs(m_spectra[bin_it]);
    integral+=abs(m_spectra[bin_it]);
  }
  mean/=integral;
  return this->getX(mean);
}


SpectraCo* SpectraCo::derivate(int smooth) noexcept
{
  m_derivative = new SpectraCo(*this); // Can optimize here
  m_derivative->name() = m_name+"_der";

  auto const & smooth_range = 2*smooth;

  int lower_bin = 0;
  int upper_bin = 0;
  double low_sum = 0.0;
  double up_sum = 0.0;
  for (int bin = 0; bin<m_size; bin++)
  {
    lower_bin = bin-smooth;
    upper_bin = bin+smooth;

    // Before all, handle side effects : 
    // At the beginning and the end of the spectra, there are not enough bins on both sides to smooth correctly
    // Therefore, we have to set a correct number of bins
    if (lower_bin<0)
    {// For the first bins of the histogram
      lower_bin = 0;
      upper_bin = 2*bin;
    }
    else if (upper_bin > m_size-1)
    {// For the last bins of the histogram
      lower_bin = 2*bin-m_size;
      upper_bin = m_size;
    }

    low_sum = 0.0;
    up_sum = 0.0;

    // First, sum the content of all the bins on the left :
    for (int bin_low = lower_bin; bin_low<bin; bin_low++) {low_sum+=m_spectra[bin_low];}

    // Second, sum the content of all the bins on the right :
    for (int bin_up = bin+1; bin_up<upper_bin; bin_up++) {up_sum+=m_spectra[bin_up];}

    // Calculate the derivative : (sum_right - sum_left) / (x_right - x_left)
    (*m_derivative)[bin] = (up_sum-low_sum)/(smooth_range);
  }
  return m_derivative;
}

SpectraCo* SpectraCo::derivate2(int smooth)noexcept
{
  derivate(smooth);
  m_derivative2 = m_derivative->derivate(smooth);
  return m_derivative2;
}


SpectraCo* SpectraCo::derivative(std::string const & name)
{
  if (!m_derivative) 
  {
    error(m_name, "has no derivative spectra !!");
    return nullptr;
  }
  else
  {
    m_derivative->name() = name+"_der"; 
    return m_derivative;
  }
}

SpectraCo* SpectraCo::derivative2(std::string const & name)
{
  if (!m_derivative2) 
  {
    error(m_name, "has no second derivative spectra !!");
    return nullptr;
  }
  else
  {
    m_derivative2->name() = name+"_derder"; 
    return m_derivative2;
  }
}

/**
 * @brief Resize allows one to select the number of bins. Affects only the maximum bin.
 */
void SpectraCo::resizeBin(size_t const & new_size)
{
  m_max_x = m_max_x * (new_size/double_cast(m_size));
  m_size = new_size;
  m_spectra.resize(m_size);
  if (m_derivative) m_derivative->resizeBin(new_size);
  m_integral = sum(m_spectra);
  calculateCoeff();
}

/**
 * @brief Carefull, this resize leads to a copy of the data
 * 
 * @param minX 
 * @param maxX 
 */
void SpectraCo::resizeX(double const & minX, double const & maxX)
{
  print(minX, maxX);
}

void SpectraCo::resizeX(double const & maxX)
{
  print(maxX);
}

TH1D* SpectraCo::createTH1D(std::string newName, std::string newTitle)
{
  if (newName == "") newName = m_name;
  if (newTitle == "") newTitle = m_title;
  TH1D* out = new TH1D(m_name.c_str(), m_title.c_str(), m_size, this->minX(), this->maxX());
  for (int bin = 1; bin<m_size+1; bin++) out->SetBinContent(bin, double_cast(m_spectra[bin-1]));
  root_spectra_pointers.push_back(out);
  return out;
}

TH1F* SpectraCo::createTH1F(std::string newName, std::string newTitle)
{
  if (newName == "") newName = m_name;
  if (newTitle == "") newTitle = m_title;
  TH1F* out = new TH1F(newName.c_str(), newTitle.c_str(), m_size, this->minX(), this->maxX());
  for (int bin = 1; bin<m_size; bin++) out->SetBinContent(bin, m_spectra[bin]);
  root_spectra_pointers.push_back(out);
  return out;
}

/// @brief Returns the position where the histogram goes under the threshold
/// @details  Must start above threshold. There must be at least nb_below points below the threshold
bool goingBelow(std::vector<double> & m_spectra, int & bin, int const & threshold, int const & nb_below)
{
  // auto const & bin_max = bin+nb_below;
  for(;bin<int_cast(m_spectra.size());bin++)
  {
    if (m_spectra[bin]<threshold)
    {
      // print(bin, m_spectra[bin]);
      bool stay_below = true;
      for (int bin_j = bin+1; bin_j<bin+nb_below; bin_j++)
      {
        if (bin_j==int_cast(m_spectra.size())) return false;// Bounds check
        // Going back to the main loop if spectra goes back above threshold :
        if (m_spectra[bin_j]>threshold) {bin = bin_j; stay_below = false; break;} 
      }
      // print(stay_below);
      if (stay_below) return true;
    }
  }
  return false;
}

/// @brief Returns the position where the histogram goes under the threshold
/// @details  Must start above threshold. There must be at least nb_below points below the threshold
bool goingAbove(std::vector<double> & m_spectra, int & bin, int const & threshold, int const & nb_above)
{
  // auto const & bin_max = bin+nb_above;
  for(;bin<int_cast(m_spectra.size());bin++)
  {
    if (m_spectra[bin]>threshold)
    {
      // print(bin, m_spectra[bin]);
      bool stay_above = true;
      for (int bin_j = bin+1; bin_j<bin+nb_above; bin_j++)
      {
        if (bin_j==int_cast(m_spectra.size())) return false;// Bounds check
        // Going back to the main loop if spectra goes back above threshold :
        if (m_spectra[bin_j]<threshold) {bin = bin_j; stay_above = false; break;} 
      }
      if (stay_above) return true;
    }
  }
  return false;
}

SpectraPoint minimum_in_range(std::vector<double> & m_spectra, int bin_min, int const & bin_max)
{
  SpectraPoint point;
  for (;bin_min<bin_max; bin_min++)
  {
    if (point.second>m_spectra[bin_min]) point = {bin_min, m_spectra[bin_min]};
  }
  return point;
}

SpectraPoint maximum_in_range(std::vector<double> & m_spectra, int bin_min, int const & bin_max)
{
  SpectraPoint point;
  for (;bin_min<bin_max; bin_min++)
  {
    if (point.second<m_spectra[bin_min]) point = {bin_min, m_spectra[bin_min]};
  }
  return point;
}

/// @brief The spectra must not have had background substraction yet
SpectraPoints const & SpectraCo::findPeaks(int const & threshold, int const & nb_bins_below_threshold, int const & smooth)
{
  this->removeBackground(smooth);
  this->derivate2(smooth);
  return findPeaks(threshold, nb_bins_below_threshold);
}

/**
 * @brief Uses the second derivative spectra to get the peaks.
 * 
 * @param threshold 
 * @param nb_bins_below_threshold 
 * @return SpectraPoints: a vector<pair<int, double>> with the keys the bin and the values the height of the second derivative peak
 */
SpectraPoints const & SpectraCo::findPeaks(int const & threshold, int const & nb_bins_below_threshold)
{
  m_peaks.clear();
  if ((*m_derivative2).size()==0) {error("in SpectraCo::findPeaks(int threshold, int nb_bins_below_threshold) : first use SpectraCo::derivate2"); return m_peaks;}

  // Aliasing the parameters (only for the code to be more readable)
  auto const & thres = -threshold; // The second derivative spectra has peaks down
  auto const & nb_below = nb_bins_below_threshold;

  std::vector<int> bin_starts;
  std::vector<int> bin_stops ;

  // Iterator : starts at the left point of the spectra
  int bin_it = 0;

  // If the spectra first bin is below the threshold, look for first bin above it
  if ((*m_derivative2)[0] < thres) goingAbove(m_derivative2->data(), bin_it, threshold, 1);

  // SpectraCo::goingBelow returns true when a new peak is found, and false at the end point of the spectra
  // The function does the iterator increment (bin_it++) automatically
  while(goingBelow(m_derivative2->data(), bin_it, thres, nb_below))
  {
    // The bin iterator current value is the first bin whose value is below the threshold
    bin_starts.push_back(bin_it);

    // SpectraCo::goingBelow returns true at the end of the peak (iterator increment (bin_it++) automatically)
    goingAbove(m_derivative2->data(), bin_it, thres, 1);

    // The bin iterator current value is the first bin whose value is above the threshold
    bin_stops.push_back(bin_it);

    // Finds the minimum point between the first bin to go below threshold and next bin to go above
    m_peaks.push_back(minimum_in_range(m_derivative2->data(), bin_starts.back(), bin_stops.back()));

    // Next call to SpectraCo::goingBelow will return true if the spectra goes below the threshold again
  }
  return m_peaks;
}


/**
 * @brief TODO
 * @todo Il faut finir la méthode MAJ qui permet de rendre les 
 * histogrames compatibles entre eux (même nombre de bins, même minimum et maximum)
 * Cela permet ensuite de les merge dans un TH2F
 * 
 */
// class SpectraCo2D
// {
// public:
//   SpectraCo2D() noexcept = default;
//   SpectraCo2D(std::vector<SpectraCo> const & spectra) 
//   {
//     m_spectra.reserve(spectra.size());
//     int i = 0;
//     for (auto const & sprectrum : spectra) 
//     {
//       m_spectra.emplace(i, sprectrum);
//       m_indexes.push_back(i);
//     }
//     this->MAJ();
//   }
//   SpectraCo2D(std::map<int, SpectraCo> const & spectra) 
//   {
//     m_spectra.reserve(spectra.size());
//     for (auto const & it : spectra) 
//     {
//       m_spectra[it.first] = it.second;
//       m_indexes.push_back(it.first);
//     }
//     this->MAJ();
//   }

//   void MAJ()
//   {
//     for (auto const & it : m_spectra)
//     {
//       auto const & spectrum = it.second;
//       if (m_min_X>spectrum.minX()) m_min_X = spectrum.minX();
//       if (m_max_X<spectrum.maxX()) m_max_X = spectrum.maxX();
//       if (nb_bins<spectrum.size()) nb_bins = spectrum.size();
//     }
//     // Resize all the spectra at the same size, and setup the same beginning and end
//     for (auto const & spectrum : m_spectra)
//     {
//       print(spectrum);
//     }
//   }

//   TH2F* createTH2F(std::string const & name = "all_histo")
//   {
//     TH2F* ret (new TH2F(name.c_str(), 
//                 name.c_str(), m_spectra.size(), 0, m_spectra.size(), 
//                 nb_bins, m_min_X, m_max_X));
//     return ret;
//   }

// private:
//   std::vector<int> m_indexes;
//   std::unordered_map<int, SpectraCo> m_spectra;

//   double m_min_X=0;
//   double m_max_X=0;
//   int nb_bins = 0;
// };

// /**
//  * @brief TODO
//  * @todo this function is not working (issue at 'if (fine_left != 0) new_bin += fine_left * m_spectra[reader_it++];')
//  * @param factor 
//  */
// void SpectraCo::rebin(int const & factor)
// {
//   if (factor<1) throw_error("in SpectraCo::rebin(int factor) : factor can't be < 1");
//   auto const & rest     = m_size % factor;
//   int  const & new_size = m_size / factor;
//   std::vector<double> new_vector(new_size, 0); // Fills the new vector with 0

//   if(rest == 0) 
//   {
//     m_rebin*=factor; // Saves the rebin factor
//     for (int new_bin_it = 0; new_bin_it<new_size; new_bin_it++){ 
//       // pauseCo(); print(new_bin_it);
//       for (int bin_j = 0; bin_j<factor; bin_j ++){
//         // print(int_cast(new_bin_it * factor + bin_j), m_spectra[int_cast(new_bin_it * factor + bin_j)]);
//         new_vector[new_bin_it] += m_spectra[int_cast(new_bin_it * factor + bin_j)];
//     }
//     // print(new_vector[new_bin_it]);
//     }
//   }

//   else
//   {
//     print("Can't rebin if the factor isn't a diviser of the number of bins");
//     return;
//     m_rebin*=factor; // Saves the rebin factor
//     // If the rebin factor is not a divisor of the initial size,
//     // one has to extend a little bit each of the new bins. For instance, 
//     // if 'size = 20' and 'factor = 3' then 'rest = size%factor = 1' and 'new_size = 6, 
//     // so there is one bin "missing". To compensate, each bin will be extended 
//     // by 'extend = rest/new_size', so here '1/6'.

//     double extend = rest / new_size;
    
//     // Starts to loop over the histograms :
//     for (int old_bin_it = 0; old_bin_it<m_size; old_bin_it++){
//       auto & new_bin = new_vector[old_bin_it / factor]; // Create an alias

//       // First, calculate the actual shift for the current bin :
//       double const & shift_left  =  old_bin_it    * extend;
//       double const & shift_right = (old_bin_it+1) * extend;

//       // Then calculate the number of bins to shift :
//       int const & bins_shift_left  = int_cast(std::trunc(shift_left));
//       int const & bins_shift_right = int_cast(std::trunc(shift_right));

//       // And the 'fine shift' (the rest of the trucation)
//       double const & fine_left  = 1 - (shift_left  - double_cast(bins_shift_left ));
//       double const & fine_right =      shift_right - double_cast(bins_shift_right );

//       // The first bin is shifted accordingly :
//       int reader_it = old_bin_it + bins_shift_left;

//       // Take the fine left shift into account, which means 
//       if (fine_left != 0) new_bin += fine_left * m_spectra[reader_it++];

//       // Calculate the stop bin :
//       auto const & stop_bin = old_bin_it + factor + bins_shift_right;

//       // Read all the other bins :
//       for (; reader_it < stop_bin; reader_it ++){
//         new_bin += m_spectra[reader_it];
//       }

//       // Take the fine right shift into account :
//       if (fine_right != 0) new_bin += fine_right * m_spectra[reader_it++];
//   }}

//   m_spectra = new_vector;
//   m_size    = new_size;
// }

#endif //SPECTRACO_HPP