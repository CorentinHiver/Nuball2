#ifndef CALIBANDSCALE_HPP
#define CALIBANDSCALE_HPP

#include "../libCo.hpp"


/**
 * @brief X-Calibrating and Y-Scaling a spectrum
 * @details 
 * Object representing an energy calibration followed by a Y scaling, used primarly for specta alignement.
 * Full file and streams i/o interface.
 */
class CalibAndScale
{
  using Coeffs = std::vector<double>;

public:
  // Constructors and loaders :
  CalibAndScale() noexcept = default;

  /// @brief Initializes a new instance with a vector : the first terms are the calibration, the last one the scale factor
  CalibAndScale(std::vector<double> const & vec       ) noexcept;
  /// @brief Initializes a new instance with an initializer list : the first terms are the calibration, the last one the scale factor
  CalibAndScale(std::initializer_list<double> initList) noexcept;
  /// @brief Copy constructor
  CalibAndScale(CalibAndScale const & other) noexcept : 
    m_ok(other.m_ok), m_coeffs(other.m_coeffs), m_scale(other.m_scale), m_order(other.m_order)
  {}

  /// @brief See constructor
  CalibAndScale& operator=(std::initializer_list<double> initList );
  /// @brief See constructor
  CalibAndScale& operator=(Coeffs              const &   vec      );
  /// @brief See constructor
  CalibAndScale& operator=(CalibAndScale       const &   other    );

  /////////////
  // Getters //
  /////////////

  /// @brief Gets the calibration coefficient (size depends on the polynomial coefficient) and adds the scaling factor at the end
  std::vector<double> get() const;
  /// @brief Gets the calibration coefficient (size depends on the polynomial coefficient)
  auto const & getCoeffs () const {return m_coeffs;}
  /// @brief Gets the scaling factor
  auto const & getScale  () const {return m_scale ;}
 /// @brief Gets the polynomial order of the calibration
  auto const & order() const {return m_order;}
 /// @brief Gets the i'th power coefficient of the polynomial calibration
  auto const & operator[] (int const & i) const {return m_coeffs[i];}
  
  /////////////
  // Setters //
  /////////////

  /// @brief Sets the scaling factor
  void setScale(double const & scale ) noexcept {m_scale = scale;}  
  /// @brief Set new coefficients for the polynomial calibration
  void setCoeffs(std::initializer_list<double> newCoeffs);
  /// @brief Set new coefficients for the polynomial calibration
  void setCoeffs(Coeffs newCoeffs);

  /////////////
  // Methods //
  /////////////

  /// @brief Returns the calibrated X value
  double linear_inv_calib(double const & value) const;
  double calibrate (double const & value ) const;

  // Inputs :
  /// @brief Gets the parameters from a file for a given label
  bool readFrom(std::string const & filename, std::string const & label);
  /// @brief Gets the parameters from an input stream, such as a file (handled by the User, who needs to handle the label as well)
  friend std::istream& operator>>(std::istream& is, CalibAndScale & calib);

  /// @brief Writes the coefficients in a file in the following way : "label coeff0 coeff1 coeff2 ... scale"
  void writeTo(std::string const & filename, std::string const & label) const;
  void writeTo(std::ostream & out, std::string const  label) const;
  /// @brief Print or write
  friend std::ostream& operator<<(std::ostream& out, CalibAndScale const & calib);

  operator bool() const & {return m_ok;}

private:

  void init();

  bool m_ok = false;
  Coeffs m_coeffs;
  double m_scale = 1.;
  int m_order = 0;
};


class CalibAndScales
{
public:
  CalibAndScales() noexcept = default;
  CalibAndScales(std::string const & filename)
  {
    std::ifstream in(filename);
    if (!in.good()) {if (sVerbose>0) error("in CalibSCales::CalibSCales(filename) : ", filename, "unreachable"); return;}
    std::string line;

    while(std::getline(in, line))
    {
      int label;
      std::istringstream iss(line);
      iss >> label;
      iss >> m_calibs[label];
    }
    m_ok = true;
  }

  void setCalib(int const & run_number, CalibAndScale const & calib)
  {
    m_calibs.emplace(run_number, calib);
  }

  static void verbose(int v) {sVerbose = v;}

  auto const & operator[](int const & run) const {return m_calibs.at(run);}
  
  bool hasRun(int const & run) const {return key_found(m_calibs, run);}

  auto const & get() const {return m_calibs;}
  
  friend std::ostream& operator<<(std::ostream& out, CalibAndScales const & calibs)
  {
    auto runs = Colib::list_of_keys(calibs.get());

    if (dynamic_cast<std::ofstream*>(&out)) for (auto const & run : runs) calibs[run].writeTo(out, std::to_string(run));
    else for (auto const & run : runs) out << calibs[run] << std::endl;

    return out;
  }

  auto begin() {return m_calibs.begin();}
  auto end  () {return m_calibs.end  ();}
  auto begin() const {return m_calibs.begin();}
  auto end  () const {return m_calibs.end  ();}
  
private:
  bool m_ok = false;
  std::unordered_map<int, CalibAndScale> m_calibs;
  static size_t sVerbose;
};

size_t CalibAndScales::sVerbose = 1;

/////////////////////////////
// CalibAndScale methods : //
/////////////////////////////

void CalibAndScale::init()
{
  m_order = m_coeffs.size()-1;
  if (!m_coeffs.empty() || m_scale != 1.) m_ok = true;
}

CalibAndScale::CalibAndScale(std::initializer_list<double> initList) noexcept
{
  m_coeffs.clear();
  auto it = initList.begin();
  for (size_t i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
  m_scale = double_cast(*it++);
  this->init();
}

CalibAndScale& CalibAndScale::operator=(std::initializer_list<double> initList)
{
  m_coeffs.clear();
  auto it = initList.begin();
  for (size_t i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
  m_scale = double_cast(*it++);
  this->init();
  return *this;
}

void CalibAndScale::setCoeffs(std::initializer_list<double> newCoeffs)
{
  m_coeffs.clear();
  m_coeffs = newCoeffs;
  this->init();
}

CalibAndScale::CalibAndScale(Coeffs const & vec) noexcept
{
  m_coeffs.clear();
  for (size_t i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
  m_scale = vec.back();
  this->init();
}

CalibAndScale& CalibAndScale::operator=(Coeffs const & vec)
{
  m_coeffs.clear();
  for (size_t i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
  m_scale = vec.back();
  this->init();
  return *this;
}

void CalibAndScale::setCoeffs(Coeffs newCoeffs)
{
  m_coeffs = newCoeffs;
  this->init();
}

CalibAndScale& CalibAndScale::operator=(CalibAndScale const & other)
{
  m_coeffs = other.m_coeffs;
  m_scale  = other.m_scale ;
  m_order  = other.m_order;
  return *this;
}

/////////////
// Getters //
/////////////

std::vector<double> CalibAndScale::get() const {
    auto ret = m_coeffs;
    ret.push_back(m_scale);
    return ret;
}

/////////////
// Methods //
/////////////

std::ostream& operator<<(std::ostream& out, CalibAndScale const & calib)  
{
  out << "coeffs " << calib.m_coeffs << " scale " << calib.m_scale;
  return out;
}

// std::ofstream& operator<<(std::ofstream& fout, CalibAndScale const & calib)  
// {
//   fout << calib;
//   return fout;
// }

double CalibAndScale::linear_inv_calib(double const & value) const
{
  if (m_order != 1) error("CalibAndScale::linear_inv_calib : order must be 1");
  return (value - m_coeffs[0]) / m_coeffs[1];
}


/// @brief Returns the X value after calibration
double CalibAndScale::calibrate(double const & value) const
{
  auto const & order = m_order;
  if (order < 0) return value;
  double ret = 0.;
  double powered_value = 1.;
  for (int power = 0; power <= order; ++power)
  {
    ret += powered_value * m_coeffs[power];
    powered_value *= value;  // Compute value^order iteratively instead of using std::pow, much faster because order is an integer
  }
  return ret;
}

void CalibAndScale::writeTo(std::string const & filename, std::string const & label) const
{
  std::ofstream out(filename, std::ios::out | std::ios::app);
  this -> writeTo(filename, label);
}

void CalibAndScale::writeTo(std::ostream & out, std::string const  label) const
{
  out << label;
  for (auto const & coeff : m_coeffs) out << " " << coeff;
  out << " " << m_scale << std::endl;
}

std::istream& operator>>(std::istream& is, CalibAndScale & calib) 
{
  double tmp = 0;
  while(is >> tmp) 
  {
    if (tmp > 1.e-10) calib.m_coeffs.push_back(tmp);
    else              calib.m_coeffs.push_back(0  );
  }
  calib.m_scale = calib.m_coeffs.back();
  calib.m_coeffs.pop_back();
  calib.init();
  return is;
}

bool CalibAndScale::readFrom(std::string const & filename, std::string const & label)
{
  std::ifstream in(filename, std::ios::in);
  std::string line;
  m_ok = false;
  m_coeffs.clear();

  while(std::getline(in, line))
  {
    if(line.substr(0, label.size()) == label)
    {
      std::istringstream iss(line.substr(label.size()));
      iss >> *this;
      return true;
    }
  }
  return false;
}

#endif //CALIBANDSCALE_HPP