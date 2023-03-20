#include "../utils.hpp"
class Calibration
{
public:

  Calibration(){};

  Bool_t readFile(std::string const & calibFileName, int const & label_max);

  Bool_t calibrate(Hit & hit);
  Float_t calibrate(Float_t const & nrj, UShort_t const & order, Bool_t & good);

  Bool_t const & isFilled() const {return m_isFilled;}

  void Print();

  operator bool() {return m_isFilled;}

  //DEV :
  void calibrate_fast(Hit & hit);
  void calibrate_fast(Label label, ADC energy, NRJ energy_calibrated);
  void setCalibrationTables();
  //!DEV

  UShort_t const & size() const {return m_nb_labels;}

private:
  //Private methods :
  void set(UShort_t label, Float_t intercept, Float_t slope, Float_t binom, Float_t trinom);

  //Attributs :
  Bool_t m_isFilled = false;
  UShort_t m_nb_labels = 0;
  UShort_t m_max_labels = 0;
  std::vector<UShort_t> m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<Float_t>  m_intercept;
  std::vector<Float_t>  m_slope;
  std::vector<Float_t>  m_binom;
  std::vector<Float_t>  m_trinom;
  std::vector<std::vector<std::vector<Float_t>>> calibration_tables;
};

void Calibration::Print()
{
  for (UShort_t i = 0; i<m_max_labels; i++)
  {
    std::cout << i << " : " << m_intercept[i] << " " << m_slope[i] << " " << m_binom[i] << std::endl;
  }
}

//DEV :
void Calibration::setCalibrationTables()
{
  print("creating calibration tables");
  calibration_tables.resize(m_max_labels);
  std::vector<std::vector<Float_t>> *calib_vec;
  for (UShort_t i = 0; i<m_max_labels; i++)
  {
    calib_vec = &calibration_tables[i];
    calib_vec->resize(1000000);
  }
  print("Done !");
  print();
}
//!DEV

inline Float_t Calibration::calibrate(Float_t const & nrj, Label const & label, Bool_t & good)
{
  auto nrj_r = nrj+gRandom->Uniform(0,1);
  switch(m_order[label])
  {
    case 0:
      good = true;
      return nrj_r;

    case 1:
      good = true;
      return m_intercept[label]
             + m_slope[label] * nrj_r;

    case 2:
      good = true;
      return m_intercept[label]
             + m_slope[label] * nrj_r
             + m_binom[label] * nrj_r * nrj_r;

    case 3:
      good = true;
      return m_intercept[label]
             + m_slope [label] * nrj_r
             + m_binom [label] * nrj_r * nrj_r
             + m_trinom[label] * nrj_r * nrj_r * nrj_r;

    default:
      good = false;
      return 0;
  }
}

inline Bool_t Calibration::calibrate(Hit & hit)
{
  const Label& label = hit.label; //constant alias
  #if defined (LICORNE)
  if (is_EDEN(label))
  {
    if (hit.nrj2==0) hit.nrj2 = 1;
    hit.nrjcal = (Float_t) hit.nrj2/hit.nrj;
    return true;
  }
  #elif defined (PARIS)
  if (isParis[label])
  {
    Bool_t good = false;
    hit.nrj2 = calibrate(hit.nrj2, label, good);
  }
  #endif
  if (isBGO[label])
  {
    hit.nrjcal = hit.nrj/100;
    return true;
  }
  if (label > m_max_labels) return false;
  Bool_t good = false;
  hit.nrjcal = calibrate(hit.nrj, label, good);

  return good;
}

void Calibration::set(UShort_t _label, Float_t _intercept = 0.f, Float_t _slope = 1.f, Float_t _binom = 0.f, Float_t _trinom = 0.f)
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

Bool_t Calibration::readFile(std::string const & calibFileName, int const & label_max)
{
  m_max_labels = label_max;
  print(label_max);
  std::ifstream inputfile;
  inputfile.open(calibFileName);
  if (!inputfile.good()) return false;
  std::string line = "";
  UShort_t label = 0;
  m_order    .resize(label_max);
  m_intercept.resize(label_max);
  m_slope    .resize(label_max, 1.f); //Fill with 1
  m_binom    .resize(label_max);
  m_trinom   .resize(label_max);
  Float_t intercept = 0.f, slope = 1.f, binom = 0.f, trinom = 0.f;
  while (getline(inputfile, line))
  {
    m_nb_labels++;
    std::istringstream iss(line);
    iss >> label >> intercept >> slope >> binom >> trinom;
    this -> set(label, intercept, slope, binom, trinom);
    intercept = 0.f; slope = 1.f; binom = 0.f; trinom = 0.f;
  }
  m_isFilled = true;
  return true;
}
