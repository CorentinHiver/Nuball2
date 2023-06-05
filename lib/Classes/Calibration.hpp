#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Hit.h>
#include <libCo.hpp>
#include <Detectors.hpp>
#include <DetectorsList.hpp>
#include <FilesManager.hpp>
// #include <

class Calibration
{
public:

  Calibration(){};
  Calibration(std::string const & file, int const & label_max){load(file, label_max);};

  Bool_t load(std::string const & calibFileName, int const & label_max);
  void calculate(std::string const & dataDir, int const & nb_files);

  void setDetectorsList(DetectorsList const & ID_file) {m_detList = ID_file;}
  void setDetectorsList(DetectorsList *ID_file) {m_detList = *ID_file;}

  void    calibrate(Hit & hit);
  Float_t calibrate(Float_t const & nrj, Label const & label);

  Bool_t const & isFilled() const {return m_isFilled;}

  void Print();

  operator bool() {return m_isFilled;}

  //DEV :
  // void calibrate_fast(Hit & hit){}
  // void calibrate_fast(Label label, ADC energy, NRJ energy_calibrated){}
  void setCalibrationTables();
  //!DEV

  UShort_t const & size() const {return m_nb_labels;}

private:
  //Private methods :
  void set(UShort_t label, Float_t intercept, Float_t slope, Float_t binom, Float_t trinom);


  //Attributs for the calculations :

  Bool_t      m_verbose   = false;
  Bool_t      m_residus   = false;
  Bool_t      m_outRoot_b = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibration/";
  // void        Write(std::string _histoFilename){}
  // void        residus_calculate(){}

  //Attributs for the tables :
  Bool_t m_isFilled = false;
  UShort_t m_nb_labels = 0;
  UShort_t m_max_labels = 0;
  std::vector<UShort_t> m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<Float_t>  m_intercept;
  std::vector<Float_t>  m_slope;
  std::vector<Float_t>  m_binom;
  std::vector<Float_t>  m_trinom;
  std::vector<std::vector<std::vector<Float_t>>> calibration_tables;

  DetectorsList m_detList;
  FilesManager files;
};

void Calibration::calculate(std::string const & dataDir, int const & nb_files = -1)
{
  print ("Calculating calibrations");

  files.addFolder(dataDir, nb_files);

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

inline Float_t Calibration::calibrate(Float_t const & nrj, Label const & label)
{
  auto nrj_r = nrj+gRandom->Uniform(0,1);
  switch(m_order[label])
  {
    case 0:
      return nrj_r;

    case 1:
      return m_intercept[label]
             + m_slope[label] * nrj_r;

    case 2:
      return m_intercept[label]
             + m_slope[label] * nrj_r
             + m_binom[label] * nrj_r * nrj_r;

    case 3:
      return m_intercept[label]
             + m_slope [label] * nrj_r
             + m_binom [label] * nrj_r * nrj_r
             + m_trinom[label] * nrj_r * nrj_r * nrj_r;

    default:
      return nrj;
  }
}

inline void Calibration::calibrate(Hit & hit)
{
  auto const & label = hit.label;
  if (label > m_max_labels) return;

#if defined (LICORNE)
  if (is_EDEN(label))
  {
    if (hit.nrj2==0) hit.nrj2 = 1;
    hit.nrjcal = static_cast<Float_t>(hit.nrj2)/hit.nrj;
  }
#elif defined (PARIS)
  if (isParis[label]) hit.nrj2 = calibrate(hit.nrj2, label);
#endif

  if (isBGO[label]) hit.nrjcal = (hit.nrj+gRandom->Uniform(0,1))/100;
  else hit.nrjcal = calibrate(hit.nrj, label);
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

Bool_t Calibration::load(std::string const & calibFileName, int const & label_max)
{
  m_max_labels = label_max;
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

void Calibration::Print()
{
  for (UShort_t i = 0; i<m_max_labels; i++)
  {
    std::cout << i << " : " << m_intercept[i] << " " << m_slope[i] << " " << m_binom[i] << std::endl;
  }
}

#endif //CALIBRATION_H
