#ifndef FIT_HPP
#define FIT_HPP

#include "../libCo.hpp"
#include "Hit.hpp"

/**
 * @brief Allows one to manipulate the results of peak fitting
*/
class Fit
{
public:
  Fit(){};

  void resize(int size)
  {
    peaks.resize(size);
    cmeasures.resize(size);
    mean.resize(size);
    sigma.resize(size);
  };

  void clear()
  {
    peaks.clear();
    cmeasures.clear();

    mean.clear();
    sigma.clear();

    x.clear() ;
    y.clear() ;
    ex.clear();
    ey.clear();
  
    integral = -1.0;
    chi2 = -1.0;
    parameter0 = 0.0;
    parameter1 = 1;
    parameter2 = 0.0;
    parameter3 = 0.0;
    scalefactor = 0.0;
    keVperADC = 0.0;
    order = 0;

    name = "";

    m_label = 0;
    m_exist = false;
    m_enough_counts = false;
    m_peaks_found = false;
  }

  auto const & label() const {return m_label;}

  /// @brief Returns the number of peaks used for calibration
  auto size() const {return peaks.size();};

  /// @brief Returns true if fit succeeded
  auto const & exists() const {return this->m_exist;}

  /// @brief Returns true if fit had enough counts
  auto enough_counts() const {return this -> m_enough_counts;}

  /// @brief Returns true if the fit found the peaks
  auto found_peaks() const {return this -> m_peaks_found;}

  /// @brief Set the label
  void setLabel(Label const & label) {m_label = label;}

  /// @brief Set if the fit succeeded :
  bool const & exists(bool exist) {return (m_exist = bool_cast(exist));}

  /// @brief Set if there was not enough counts :
  bool too_few_counts(bool few_counts) {return (m_enough_counts = bool_cast(!few_counts));}

  /// @brief Set if there was not enough counts :
  bool peaks_found(bool found) {return (m_peaks_found = bool_cast(found));}

  std::vector<double> peaks;
  std::vector<double> cmeasures;

  std::vector<double> mean;
  std::vector<double> sigma;

  std::vector<double> x ;
  std::vector<double> y ;
  std::vector<double> ex;
  std::vector<double> ey;

  double integral = -1.0;
  double chi2 = -1.0;
  double parameter0 = 0.0;
  double parameter1 = 1;
  double parameter2 = 0.0;
  double parameter3 = 0.0;
  double scalefactor = 0.0;
  double keVperADC = 0.0;
  char order = 0;

  std::string name;

private:
  Label m_label = 0;
  bool m_exist = false;
  bool m_enough_counts = false;
  bool m_peaks_found = false;
};

std::ofstream& operator<<(std::ofstream& fout, Fit const & fit)
{
  fout << fit.label();
  fout << std::setprecision(4);
  fout << " " << fit.parameter0;
  fout << std::setprecision(6);
  fout << " " << fit.parameter1;
  if (fit.parameter2!=0.0) 
  {
    fout << std::setprecision(8);
    fout << " " << fit.parameter2;
  }
  if (fit.parameter3!=0.0) 
  {
    fout << std::setprecision(10);
    fout << " " << fit.parameter3;
  }
  fout << std::endl;
  return fout;
}

class Fits 
{
public:
  Fits() = default;
  Fits(int const & _size) {this -> resize(_size);}
  
  void resize(size_t const & i = 0) {m_fits.resize(i);}
  void reserve(size_t const & i = 0) {m_fits.reserve(i);}
  auto size() {return m_fits.size();}
  auto size() const {return m_fits.size();}

  /// @brief Accesses the vector of fits
  auto const & get() const {return m_fits;}
  /// @brief Accesses the vector of fits
  auto & get() {return m_fits;}

  /// @brief Directly access the fit :
  auto const & operator[](size_t const & i) const {return m_fits[i];}
  /// @brief Directly access the fit :
  auto & operator[](size_t const & i) {return m_fits[i];}


  // Range-based iterators :

  /// @brief Returns the iterator to the start of the vector
  auto begin() const {return m_fits.begin();}
  /// @brief Returns the iterator to the start of the vector
  auto begin() {return m_fits.begin();}
  /// @brief Returns the iterator to the end of the vector
  auto end  () const {return m_fits.end(  );}
  /// @brief Returns the iterator to the end of the vector
  auto end  () {return m_fits.end(  );}

private:
  std::vector<Fit> m_fits;
};

// std::ostream & operator<<(std::ostream& out, Fits const & fits)
// {
//   for (auto const & fit : fits)
//   {
//     if (fit.out << fit;
//   }
// }


#endif //FIT_HPP