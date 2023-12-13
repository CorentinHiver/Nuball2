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

  /// @brief Loading calibration from file name
  Calibration(File const & calibFileName) {load(calibFileName);}
  /// @brief Loading calibration from file name
  Calibration const & operator=(std::string const & calibFileName) {load(calibFileName); return *this;}
  /// @brief Loading calibration from file name
  bool load(File const & calibFileName);

  void writeData(std::string const & outfilename);

  void calibrate(Hit & hit) const noexcept;

  /// @brief Calibrate the nrj value using the parameters extracted from the calibration data
  NRJ calibrate(NRJ const & nrj, Label const & label) const noexcept;

  /// @brief Wrapper around the Calibration::calibrate() methods
  template<class... ARGS>
  auto operator()(ARGS &&... args) const {return calibrate(std::forward<ARGS>(args)...);}

  /// @brief Return true if the data has been loaded
  operator bool() const & {return (m_ok && m_size>0);}

  //DEV :
  // void calibrate_fast(Hit & hit){}
  // void calibrate_fast(Label label, ADC energy, NRJ energy_calibrated){}
  void setCalibrationTables();
  //!DEV


  // Accessors to the calibration parameters :
  std::vector<NRJ> operator[](Label const & label) const noexcept;

  auto const & size()         const noexcept {return m_nb_det   ;}
  auto const & getOrder()     const noexcept {return m_order    ;}
  auto const & getIntercept() const noexcept {return m_intercept;}
  auto const & getSlope()     const noexcept {return m_slope    ;}
  auto const & getBinom()     const noexcept {return m_binom    ;}
  auto const & getTrinom()    const noexcept {return m_trinom   ;}
  bool const & isFilled()     const noexcept {return m_ok       ;}
  auto const & file()         const noexcept {return m_filename ;}

  void Print();

private:
  //Private methods :
  void set(Label label, NRJ intercept, NRJ slope, NRJ binom, NRJ trinom);

  std::string m_filename;
  bool m_ok = false;
  Label m_nb_det = 0;
  Label m_size = 0;
  std::vector<char> m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<NRJ> m_intercept;
  std::vector<NRJ> m_slope;
  std::vector<NRJ> m_binom;
  std::vector<NRJ> m_trinom;
  std::vector<std::vector<std::vector<NRJ>>> calibration_tables;
};


/// @brief @todo
void Calibration::setCalibrationTables()
{
  print("creating calibration tables");
  calibration_tables.resize(m_size);
  std::vector<std::vector<NRJ>> *calib_vec;
  for (Label i = 0; i<m_size; i++)
  {
    calib_vec = &calibration_tables[i];
    calib_vec->resize(200000);
  }
  print("Done !");
  print();
}

inline NRJ Calibration::calibrate(NRJ const & nrj, Label const & label) const noexcept
{
  // First, one has to randomize the nrj within its bin
  auto nrj_r = nrj+random_uniform();
  
  // Then, return the new value depending on the order of the calibration for this label
  switch(m_order[label])
  {
    case 0: return nrj_r; // No calibration
    case 1: return m_intercept[label] + m_slope[label]*nrj_r; // Linear calibration
    case 2: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r; // Quadratic calibration
    case 3: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r + m_trinom[label]*nrj_r*nrj_r*nrj_r; // Third order calibration
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
  auto const & label = hit.label;
  if (label > m_size) return;
  if (hit.qdc2!=0.0) hit.nrj2 = calibrate(hit.qdc2, label);
  hit.nrj = calibrate(hit.adc, label);
}

void Calibration::set(Label _label, NRJ _intercept = 0.f, NRJ _slope = 1.f, NRJ _binom = 0.f, NRJ _trinom = 0.f)
{
  if (_slope == 1.f && _intercept == 0.f) {m_order[_label] = 0;}
  else if (_binom == 0.f)
  {
    m_order     [_label] = 1;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
  }
  else if (_trinom == 0.f)
  {
    m_order     [_label] = 2;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
  }
  else
  {
    m_order     [_label] = 3;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
    m_trinom    [_label] = _trinom;
  }
}

std::vector<NRJ> Calibration::operator[](Label const & label) const noexcept
{
  if (label>m_size) return {-1};
  switch (m_order[label])
  {
    case 0 : return {0};
    case 1 : return {m_intercept[label], m_slope[label]};
    case 2 : return {m_intercept[label], m_slope[label], m_binom[label]};
    case 3 : return {m_intercept[label], m_slope[label], m_binom[label], m_trinom[label]};
    default : return {0};
  }
}

bool Calibration::load(File const & calibFileName)
{
  m_filename = calibFileName.string();
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
  m_order    .resize(m_size,-1);
  m_intercept.resize(m_size, 0.f);
  m_slope    .resize(m_size, 1.f);
  m_binom    .resize(m_size, 0.f);
  m_trinom   .resize(m_size, 0.f);
  NRJ slope = 1.f, binom = 0.f, trinom = 0.f, intercept = 0.f;
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

void Calibration::Print()
{
  for (Label label = 0; label<m_size; label++)
  {
    if (m_order[label] < 0) continue;
    std::cout << label << " : ";
    std::cout << m_intercept[label];
    if (m_order[label] > 0)
    {
      std::cout << " " << m_slope[label];
      if (m_order[label] > 1)
      {
        std::cout << " " << m_binom[label];
        if (m_order[label] > 2)
        {
          std::cout << " " << m_trinom[label];
        }
      }
    }
    std::cout << std::endl;


  }
}

/// @brief 
/// @todo  
void Calibration::writeData(std::string const & outfilename)
{
  File outFile(outfilename);
  outFile.setExtension("calib");
  outFile.makePath(); // Create the path if it doesn't already exist

  // std::ofstream outfile(outFile);
  // for (auto const & fit : m_fits) if (detectors.exists(fit.label())) outfile << fit;
  // print("Data written to", outfilename);
}

std::ostream& operator<<(std::ostream& cout, Calibration const & calib)
{
  for (Label label = 0; label<detectors.size(); label++) 
  {
    if (detectors && !detectors.exists(label)) continue;
     cout << label << " " << calib[label] << std::endl;
  }
  return cout;
}

#endif //CALIBRATION_HPP
