#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Hit.h"
#include "../libCo.hpp"
#include "Detectors.hpp"
#include "DetectorsList.hpp"
#include "FilesManager.hpp"

class Calibration
{
public:

  Calibration(){};
  Calibration(std::string const & file, int const & label_max){load(file, label_max);};
  Calibration(std::string const & file, DetectorsList const & detList)
  {
    m_detList = detList;
    load(file, detList.size());
  };

  bool load(std::string const & calibFileName, int const & label_max);
  void calculate(std::string const & dataDir, int const & nb_files);

  void setDetectorsList(DetectorsList const & ID_file) {m_detList = ID_file;}
  void setDetectorsList(DetectorsList *ID_file) {m_detList = *ID_file;}

  void  calibrate(Hit & hit) const;
  float calibrate(float const & nrj, Label const & label) const;

  /**
   * @brief Wrapper around calibrate method
  */
  template<class... ARGS>
  auto operator()(ARGS &&... args) const {return calibrate(std::forward<ARGS>(args)...);}

  bool const & isFilled() const {return m_ok;}

  void Print();

  operator bool() {return m_ok;}

  //DEV :
  // void calibrate_fast(Hit & hit){}
  // void calibrate_fast(Label label, ADC energy, NRJ energy_calibrated){}
  void setCalibrationTables();
  //!DEV

  Label const & size() const {return m_nb_labels;}

private:
  //Private methods :
  void set(Label label, float intercept, float slope, float binom, float trinom);

  //Attributs for the calculations :
  bool      m_verbose   = false;
  bool      m_residus   = false;
  bool      m_outRoot_b = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibration/";
  // void        Write(std::string _histoFilename){}
  // void        residus_calculate(){}

  DetectorsList m_detList;
  FilesManager files;

  //Attributs for the tables :
  bool m_ok = false;
  Label m_nb_labels = 0;
  Label m_max_labels = 0;
  std::vector<char> m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<float>  m_intercept;
  std::vector<float>  m_slope;
  std::vector<float>  m_binom;
  std::vector<float>  m_trinom;
  std::vector<std::vector<std::vector<float>>> calibration_tables;
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
  std::vector<std::vector<float>> *calib_vec;
  for (Label i = 0; i<m_max_labels; i++)
  {
    calib_vec = &calibration_tables[i];
    calib_vec->resize(200000);
  }
  print("Done !");
  print();
}
//!DEV

inline float Calibration::calibrate(float const & nrj, Label const & label) const 
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

inline void Calibration::calibrate(Hit & hit) const
{
  auto const & label = hit.label;
  if (label > m_max_labels) return;

#if defined (LICORNE)
  if (is_EDEN(label))
  {
    if (hit.nrj2==0) hit.nrj2 = 1;
    hit.nrjcal = static_cast<float>(hit.nrj2)/hit.nrj;
  }
#elif defined (PARIS)
  if (isParis[label]) hit.nrj2 = calibrate(hit.nrj2, label);
#endif

  if (isBGO[label]) hit.nrjcal = (hit.nrj+gRandom->Uniform(0,1))/100;
  else hit.nrjcal = calibrate(hit.nrj, label);
}

void Calibration::set(Label _label, float _intercept = 0.f, float _slope = 1.f, float _binom = 0.f, float _trinom = 0.f)
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

bool Calibration::load(std::string const & calibFileName, int const & label_max)
{
  m_max_labels = label_max;
  std::ifstream inputfile;
  inputfile.open(calibFileName);
  if (!inputfile.good()) return false;
  std::string line = "";
  Label label = 0;
  m_order    .resize(label_max, -1);
  m_intercept.resize(label_max);
  m_slope    .resize(label_max, 1.f); //Fill with 1
  m_binom    .resize(label_max);
  m_trinom   .resize(label_max);
  float intercept = 0.f, slope = 1.f, binom = 0.f, trinom = 0.f;
  while (getline(inputfile, line))
  {
    m_nb_labels++;
    std::istringstream iss(line);
    iss >> label >> intercept >> slope >> binom >> trinom;
    this -> set(label, intercept, slope, binom, trinom);
    intercept = 0.f; slope = 1.f; binom = 0.f; trinom = 0.f;
  }
  print("Calibration extracted from", calibFileName);
  return (m_ok = true);
}

void Calibration::Print()
{
  for (Label label = 0; label<m_max_labels; label++)
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

#endif //CALIBRATION_H
