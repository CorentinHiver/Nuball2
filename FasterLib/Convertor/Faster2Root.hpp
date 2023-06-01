#ifndef FASTER2ROOT
#define FASTER2ROOT

#include <libCo.hpp>
#include <DetectorsList.hpp>
#include <FilesManager.hpp>
#include <Timeshifts.hpp>
#include <Calibration.hpp>

class Faster2Root
{
public:
  Faster2Root(){}
  void launch(){}

  void setListDet(std::string const & filename) {m_detList.load(filename);}
  void setListDet(DetectorsList const & detectorList) {m_detList = detectorList;}

  void setFolder(std::string const & folder, int const & nb_max = -1) {m_files.addFolder(folder, nb_max);}

  FilesManager * p_files  () {return &m_files  ;}
  DetectorsList* p_detList() {return &m_detList;}
  Timeshifts   * p_ts     () {return &m_ts     ;}
  Calibration  * p_calib  () {return &m_calib  ;}

private:

  DetectorsList m_detList;
  FilesManager  m_files;
  Timeshifts    m_ts;
  Calibration   m_calib;

};


#endif //FASTER2ROOT
