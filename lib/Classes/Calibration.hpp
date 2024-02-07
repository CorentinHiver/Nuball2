#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

#include "../libCo.hpp"

#include "../Classes/Hit.hpp"
#include "../Classes/Detectors.hpp"

/**
 * @brief A convenient class for handling coefficient calibration up to 3rd order
 */
class Calibration
{
public:

  Calibration() = default;

  /// @brief Copy constructor
  Calibration (Calibration const & otherCalib) :
    m_filename  (otherCalib.m_filename),
    m_ok        (otherCalib.m_ok),
    m_nb_det    (otherCalib.m_nb_det),
    m_size      (otherCalib.m_size),
    m_order     (otherCalib.m_order),
    m_intercept (otherCalib.m_intercept),
    m_slope     (otherCalib.m_slope),
    m_binom     (otherCalib.m_binom),
    m_trinom    (otherCalib.m_trinom)
  {
  }

  /// @brief Copy operator
  Calibration const & operator=(Calibration const & otherCalib) 
  {
    m_ok        = otherCalib.m_ok;
    m_nb_det    = otherCalib.m_nb_det;
    m_size      = otherCalib.m_size;
    m_order     = otherCalib.m_order;
    m_intercept = otherCalib.m_intercept;
    m_slope     = otherCalib.m_slope;
    m_binom     = otherCalib.m_binom;
    m_trinom    = otherCalib.m_trinom;
    return *this;
  }

  /// @brief Constructor loading calibration from a file
  Calibration(File const & file) {load(file);}
  /// @brief Copy operator twicked to load calibration from a file
  Calibration const & operator=(File const & file) {load(file); return *this;}
  /// @brief Loading calibration from a file
  bool load(File const & file);
  void set(Label const & _label, float const & _intercept, float const & _slope, float const & _binom, float const &_trinom);
  std::vector<float> get(Label const & _label) const noexcept;

  #ifdef FIT_HPP
  void loadFits(Fits const & fits);
  #endif //FIT_HPP

  void write(std::string const & outfilename);
  
  void resize(int const & size);
  void clear() noexcept;

#ifdef ROOT_TH1
  void calibrateAxis(TH1F * histo, Label const & label)
  {
    auto const & order = m_order[label];
         if (order<0) return;
    else if (order<2) // For linear and affine calibration
    {
      // Technical point : we have to cast to int in order to always have the same values
      // (without it, the floating point of the min and max was always having a slight difference for two identical axis...)
      int new_min = calibrate(histo->GetXaxis()->GetXmin(), label);
      int new_max = calibrate(histo->GetXaxis()->GetXmax(), label);
      histo->GetXaxis()->Set(histo->GetNbinsX(), new_min, new_max);
      histo->GetXaxis()->SetRangeUser(new_min, new_max);
    }
    else throw_error(concatenate("Can't use Calibration::calibrateAxis(TH1F*, Label) with calibration order > 1 yet", error_message["DEV"]));
  }
#endif //ROOT_TH1

  /// @brief Calibrate the nrj value using the parameters extracted from the calibration data
  float calibrate(float  const & nrj, Label const & label) const noexcept;
  float calibrate(double const & nrj, Label const & label) const noexcept {return calibrate(float_cast(nrj), label);}
  float calibrate(int    const & adc, Label const & label) const noexcept {return calibrate(float_cast(adc), label);}
  float calibrate(size_t const & bin, Label const & label) const noexcept {return calibrate(float_cast(bin), label);}

  void calibrate(Hit & hit) const noexcept;

  /// @brief Wrapper around the Calibration::calibrate() methods
  template<class... ARGS>
  auto operator()(ARGS &&... args) const noexcept {return calibrate(std::forward<ARGS>(args)...);}

  /// @brief Return true if the data has been loaded
  operator bool() const & noexcept {return (m_ok && m_size>0);}

  /// @brief Returns a vector holding the coefficients.
  std::vector<float> operator[](Label const & label) const noexcept {return this->get(label);};

  /// @brief Get the number of detectors with calibration coefficients
  auto const & size        () const noexcept {return m_size     ;}
  /// @brief Get the number of detectors with calibration coefficients
  auto &       size        ()       noexcept {return m_size     ;}
  /// @brief Get the number of detectors with calibration coefficients
  auto const & nb_detectors() const noexcept {return m_nb_det   ;}
  /// @brief Get the number of detectors with calibration coefficients
  auto &       nb_detectors()       noexcept {return m_nb_det   ;}
  /// @brief Returns true if there is at least one coefficient loaded
  bool const & isFilled    () const noexcept {return m_ok       ;}
  /// @brief Returns true if there is at least one coefficient loaded
  bool &       isFilled    ()       noexcept {return m_ok       ;}
  /// @brief Gets the name of the file from while the data has been loaded
  auto const & file        () const noexcept {return m_filename ;}
  /// @brief Gets the name of the file from while the data has been loaded
  auto &       file        ()       noexcept {return m_filename ;}

  /// @brief Get the order of the calibration (0 : linear, 1 : affine, 2 : quadratic, 3 : 3rd order)
  auto const & getOrder    () const noexcept {return m_order    ;}
  /// @brief Get the order of the calibration (0 : linear, 1 : affine, 2 : quadratic, 3 : 3rd order)
  auto &       getOrder    ()       noexcept {return m_order    ;}
  /// @brief Get the intercept
  auto const & getIntercept() const noexcept {return m_intercept;}
  /// @brief Get the intercept
  auto &       getIntercept()       noexcept {return m_intercept;}
  /// @brief Get the slope parameter
  auto const & getSlope    () const noexcept {return m_slope    ;}
  /// @brief Get the slope parameter
  auto &       getSlope    ()       noexcept {return m_slope    ;}
  /// @brief Get the quadratic parameter
  auto const & getBinom    () const noexcept {return m_binom    ;}
  /// @brief Get the quadratic parameter
  auto &       getBinom    ()       noexcept {return m_binom    ;}
  /// @brief Get the 3rd order parameter
  auto const & getTrinom   () const noexcept {return m_trinom   ;}
  /// @brief Get the 3rd order parameter
  auto &       getTrinom   ()      noexcept {return m_trinom   ;}

  /// @brief Get the order of the calibration (0 : linear, 1 : affine, 2 : quadratic, 3 : 3rd order)
  auto const & order     (Label const & label) const noexcept {return m_order[label]     ;}
  /// @brief Get the order of the calibration (0 : linear, 1 : affine, 2 : quadratic, 3 : 3rd order)
  auto &       order     (Label const & label)       noexcept {return m_order[label]     ;}
  /// @brief Get the intercept
  auto const & intercept (Label const & label) const noexcept {return m_intercept[label] ;}
  /// @brief Get the intercept
  auto &       intercept (Label const & label)       noexcept {return m_intercept[label] ;}
  /// @brief Get the slope parameter
  auto const & slope     (Label const & label) const noexcept {return m_slope[label]     ;}
  /// @brief Get the slope parameter
  auto &       slope     (Label const & label)       noexcept {return m_slope[label]     ;}
  /// @brief Get the quadratic parameter
  auto const & binom     (Label const & label) const noexcept {return m_binom[label]     ;}
  /// @brief Get the quadratic parameter
  auto &       binom     (Label const & label)       noexcept {return m_binom[label]     ;}
  /// @brief Get the 3rd order parameter
  auto const & trinom    (Label const & label) const noexcept {return m_trinom[label]    ;}
  /// @brief Get the 3rd order parameter
  auto &       trinom    (Label const & label)       noexcept {return m_trinom[label]     ;}

  void Print();

private:

  std::string m_filename;
  bool m_ok = false;
  Label m_nb_det = 0;
  Label m_size = 0;
  std::vector<char> m_order;
  std::vector<float> m_intercept;
  std::vector<float> m_slope;
  std::vector<float> m_binom;
  std::vector<float> m_trinom;
  std::vector<std::vector<std::vector<float>>> calibration_tables; // TODO
};

inline float Calibration::calibrate(float const & nrj, Label const & label) const noexcept
{
  // First, one has to randomize the nrj within its bin
  auto nrj_r = nrj+random_uniform();
  
  // Then, return the new value depending on the order of the calibration for this label
  switch(m_order[label])
  {
    case -1: return nrj_r; // No calibration
    case  0: return                      m_slope[label]*nrj_r; // Linear calibration (no offset)
    case  1: return m_intercept[label] + m_slope[label]*nrj_r; // Affine calibration
    case  2: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r; // Quadratic calibration
    case  3: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r + m_trinom[label]*nrj_r*nrj_r*nrj_r; // Third order calibration
    default: return -1; // This should normally never happen
  }
}

/**
 * @brief Calibrate a hit
 * @details
 * Reads hit.adc and writes the calibrated value in hit.nrj
 * If hit.qdc2 > 0, writes the calibrated value in hit.nrj2
*/ 
inline void Calibration::calibrate(Hit & hit) const noexcept
{
  auto const & label = hit.label;      // Extracts the label of the detector
  if (label > m_size) return;          // If the label is out of range then do no treat it
  hit.nrj = calibrate(hit.adc, label); // Call to the Calibration::calibrate(energy, label) method
#ifndef QDC1MAX                        // This line allows one to calibrate a bit faster if not interested in the QDC2 field
  if (hit.qdc2!=0.0) hit.nrj2 = calibrate(hit.qdc2, label); // Only calibrate the QDC2 if there is a value in the field
  else hit.nrj2 = 0.0;
#endif //QDC1MAX
}

void Calibration::set(Label const & _label, float const & _intercept = 0.f, float const & _slope = 1.f, float const & _binom = 0.f, float const &_trinom = 0.f)
{
  if (_label+1>m_size) this->resize(_label+1);
  if (_slope == 1.f && _intercept == 0.f) {m_order[_label] = -1;}
  else if (_intercept == 0.f) 
  { // Linear calibration
    m_order     [_label] = 0;
    m_slope     [_label] = _slope;
  }
  else if (_binom == 0.f)
  {// Affine calibration
    m_order     [_label] = 1;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
  }
  else if (_trinom == 0.f)
  {// Quadratic calibration
    m_order     [_label] = 2;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
  }
  else
  {// Third order calibration
    m_order     [_label] = 3;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
    m_trinom    [_label] = _trinom;
  }
}

std::vector<float> Calibration::get(Label const & label) const noexcept
{
  if (label>m_size) return {-1};
  switch (m_order[label])
  {
    case 0 : return {0, m_slope[label]};
    case 1 : return {m_intercept[label], m_slope[label]};
    case 2 : return {m_intercept[label], m_slope[label], m_binom[label]};
    case 3 : return {m_intercept[label], m_slope[label], m_binom[label], m_trinom[label]};
   default : return {0};
  }
}

void Calibration::resize(int const & size)
{
  m_order    .resize(size,-1  );
  m_intercept.resize(size, 0.f);
  m_slope    .resize(size, 1.f);
  m_binom    .resize(size, 0.f);
  m_trinom   .resize(size, 0.f);
  m_size = size;
}

void Calibration::clear() noexcept
{
m_order    .clear();
m_intercept.clear();
m_slope    .clear();
m_binom    .clear();
m_trinom   .clear();
}

bool Calibration::load(File const & file)
{
  m_filename = file.string();
  std::ifstream inputfile(m_filename, std::ifstream::in);

  if (!inputfile.good()) {print("CAN'T OPEN THE CALIBRATION FILE " + m_filename); throw std::runtime_error("CALIBRATION");}
  else if (file_is_empty(inputfile)) {print("CALIBRATION FILE", m_filename, "EMPTY !"); throw std::runtime_error("CALIBRATION");}
  std::string line = "";
  Label size = 0;
  Label label = 0;
  // ----------------------------------------------------- //
  // First extract the maximum label
  // Infer the number of detectors from the higher label in calibration file
  while (getline(inputfile, line))
  {
    std::istringstream iss (line);
    iss >> label;
    if (label>size) size = label;
  }
  size++; // The size of the vector must be label_max+1
  // Ensure there is no mismatch with the detectors module :
  if (detectors)
  {
    if (size<detectors.size()) size = detectors.size();
    else detectors.resize(size);
  }
  inputfile.clear();
  inputfile.seekg(0, std::ios::beg); // back to the start of the file
  m_size = size;
  // ----------------------------------------------------- //
  // Now fill the vectors
  this -> resize(m_size);
  float slope = 1.f, binom = 0.f, trinom = 0.f, intercept = 0.f;
  while (getline(inputfile, line))
  {
    m_nb_det++;
    std::istringstream iss (line);
    iss >> label >> intercept >> slope >> binom >> trinom;
    this -> set(label, intercept, slope, binom, trinom);
    intercept = 0.f; slope = 1.f; binom = 0.f; trinom = 0.f;
  }
  print("Calibration extracted from", m_filename);
  return (m_ok = true);
}

/// @brief Displays the calibration coefficients
std::ostream& operator<<(std::ostream& out, Calibration const & calib)
{
  for (Label label = 0; label<calib.size(); label++)
  {
    auto const & calib_order = calib.getOrder()[label];
    if (calib_order < 0) continue;
    else if (calib_order == 0) {out << label << " " << 0 << " " << calib.getSlope()[label];}
    else //if (calib_order > 0)
    {
      out << label << " " << calib.getIntercept()[label] << " " << calib.getSlope()[label];
      if (calib_order > 1)
      {
        out << " " << calib.getBinom()[label];
        if (calib_order > 2)
        {
          out << " " << calib.getTrinom()[label];
        }
      }
    }
    out << "\n";
  }
  return out;
}

void Calibration::Print() {print(*this);}

/// @brief Writes down the calibration file
void Calibration::write(std::string const & outfilename)
{
  File outFile(outfilename);
  outFile.setExtension("calib");
  outFile.makePath(); // Create the path if it doesn't already exist

  std::ofstream outfile(outFile.string());
  outfile << *this;
  outfile.close();
  print(outFile.string(), "written");
}

#ifdef FIT_HPP
/**
 * @brief Allows one to load the results of a fit
 * @attention This function exists only if "lib/Classes/Fits.hpp" has been included before
 */
void Calibration::loadFits(Fits const & fits)
{
  this -> clear();
  this -> resize(fits.size());
  for (size_t fit_i = 0; fit_i<fits.size(); fit_i++)
  {
    auto const & fit = fits[fit_i];
    this->set(fit_i, fit.parameter0, fit.parameter1, fit.parameter2, fit.parameter3);
  }
}
#endif //FIT_HPP

#endif //CALIBRATION_HPP
