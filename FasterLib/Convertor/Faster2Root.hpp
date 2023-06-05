#ifndef FASTER2ROOT_HPP
#define FASTER2ROOT_HPP

#include <Calibration.hpp>
#include <libCo.hpp>
#include <DetectorsList.hpp>
#include <FilesManager.hpp>
#include <Timeshifts.hpp>
#include <Manip.hpp>
#include <RF_Manager.hpp>
#include <EventBuilder.hpp>
#include <Timer.hpp>

class Faster2Root
{
public:
  Faster2Root
  (
    DetectorsList const & detectorList, 
    std::string   const & timeshifts_file = "", 
    std::string   const & calib_file = "", 
    std::string   const & folder = "", 
    std::string   const & outfolder = "", 
    ushort        const & nb_threads = 2
  );
  void convert(std::string const & folder, ushort const & nb_threads);
  static void treatFilesMT(MTList<std::string> & files_list, Faster2Root * f2r);
  void treatFile(std::string const & filename);

  void setListDet(std::string const & filename) {m_detList = filename;}
  void setListDet(DetectorsList const & detectorList) {m_detList = detectorList;}
  void setRunPath(std::string const & datapath);
  void setOutDir(std::string const & outfolder);

  void loadTimeshifts(std::string const & ts_filename) {m_ts.load(ts_filename);}
  void loadCalibration(std::string const & calib_filename) {m_calib.load(calib_filename, m_detList.size());}
  
  void calculateTimeshifts(std::string const & parameters, int const & nb_files = -1) 
  {
    m_ts.setParameters(parameters);
    m_ts.calculate(m_runpath, nb_files);
  }

  FilesManager * p_files  () {return &m_files  ;}
  DetectorsList* p_detList() {return &m_detList;}
  Timeshifts   * p_ts     () {return &m_ts     ;}
  Calibration  * p_calib  () {return &m_calib  ;}

  bool const & ok () const {return m_ok;} 

private:
  bool m_ok = true;
  DetectorsList m_detList;
  FilesManager  m_files;
  Timeshifts    m_ts;
  Calibration   m_calib;

  std::string m_runpath = "";
  std::string m_outDir = "";
};

void Faster2Root::setRunPath(std::string const & runpath)
{
  if (!folder_exists(runpath, true)) m_ok = false;
  m_runpath = runpath;
}

Faster2Root::Faster2Root
(
  DetectorsList const & detectorList, 
  std::string   const & timeshifts_file, 
  std::string   const & calib_file, 
  std::string   const & datafolder, 
  std::string   const & outfolder, 
  ushort        const & nb_threads
)
{
  m_detList = detectorList;
  m_ts.setDetectorsList(m_detList);
  m_calib.setDetectorsList(&m_detList);
  if (timeshifts_file!="") m_ts.load(timeshifts_file);
  if (calib_file!="") m_calib.load(calib_file, m_detList.size());
  if (outfolder!= "") this -> setOutDir(outfolder);
  if (datafolder != "") convert(datafolder, nb_threads);
}

void Faster2Root::convert(std::string const & datafolder = "", ushort const & nb_threads = 2)
{
  m_files.addFolder(datafolder);

  if (nb_threads>1 && !MTObject::ON) MTObject::Initialize(nb_threads);

  if (MTObject::ON)
  {
    MTList<std::string> filesMT(m_files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, filesMT, this);
  }
  else 
  {
    std::string filename;
    while(m_files.nextFileName(filename))
    {
      treatFile(filename);
    }
  }
}

void Faster2Root::treatFilesMT(MTList<std::string> & files_list, Faster2Root * f2r)
{
  std::string filename;
  while(files_list.getNext(filename))
  {
    f2r->treatFile(filename);
  }
}

void Faster2Root::treatFile(std::string const & filename)
{
  // The out .root file name should have the same name as the .fast file
  std::string outfile = m_outDir+rmPathAndExt(filename)+".root";
  if ( file_exists(outfile) ) {print(outfile, "already exists !");return;}
  
  Hit hit;
  RF_Manager rf;
  Event event;
  EventBuilder coincBuilder(&event, &rf);
  
  Timer time_read;
  FasterReader reader(&hit, filename);
  if (!reader.isReady()) { print("CAN'T READ", filename); return;}
}

void Faster2Root::setOutDir(std::string const & outfolder)
{
  m_outDir = outfolder;
  create_folder_if_none(m_outDir);
  if (!folder_exists(m_outDir)) {print("Can't read the", m_outDir, "folder!");return;}
}

#endif //FASTER2ROOT_HPP
