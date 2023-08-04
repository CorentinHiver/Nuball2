#ifndef CONVERTOR_HPP
#define CONVERTOR_HPP

#include "../libCo.hpp"

#include "Detectors.hpp"
#include "Nuball2Tree.hpp"
#include "CoincBuilder.hpp"
#include "Timer.hpp"

#include "../MTObjects/MTFasterReader.hpp"

#include "../Modules/Timeshifts.hpp"
#include "../Modules/Calibration.hpp"

class Convertor
{
public:

  Convertor() {m_ok = false;}

  /// @brief Raw conversion :
  Convertor(Path const & inputFolder = "", Path const & outputFolder = "", int const & nb_files = -1) 
  {// Raw convertor : no time shift nor energy calibration
    m_ok = true;
    Timer timer;

    this -> buildEvents(false);
    this -> setOutDir(outputFolder);
    
    this -> addFolder(inputFolder, nb_files);
    this -> convert();

    print("Data written to", outputFolder, "in", timer(), timer.unit());
  }

  // Convertor(Detectors&& detectors, Timeshifts&& timeshifts, Path const & inputFolder, Path const & outputFolder, int const & nb_files = -1)
  // { // Coincidence event builder : with timeshifts
  //   Timer timer;

  //   this -> buildEvents(true);
  //   this -> setOutDir(outputFolder);

  //   this -> setDetectors(std::move(detectors));
  //   this -> setTimeshifts(std::move(timeshifts));
    
  //   this -> addFolder(inputFolder, nb_files);
  //   this -> convert();

  //   print("Data written to", outputFolder, "in", timer(), timer.unit());
  // }

  // Convertor(Detectors&& detectors, Calibration&& calibration, Path const & inputFolder, Path const & outputFolder, int const & nb_files = -1) 
  // { // Coincidence event builder : with calibration
  //   Timer timer;

  //   this -> buildEvents(true);
  //   this -> setOutDir(outputFolder);

  //   this -> setDetectors(std::move(detectors));
  //   this -> setCalibration(std::move(calibration));
    
  //   this -> addFolder(inputFolder, nb_files);
  //   this -> convert();

  //   print("Data written to", outputFolder, "in", timer(), timer.unit());
  // }

  // Convertor(Detectors&& detectors, Timeshifts&& timeshifts, Calibration&& calibration, Path const & inputFolder, Path const & outputFolder, int const & nb_files = -1) 
  // { // Coincidence event builder : with timeshits and calibration
  //   Timer timer;

  //   this -> buildEvents(true);
  //   this -> setOutDir(outputFolder);

  //   this -> setDetectors(std::move(detectors));
  //   this -> setTimeshifts(std::move(timeshifts));
  //   this -> setCalibration(std::move(calibration));
    
  //   this -> addFolder(inputFolder, nb_files);
  //   this -> convert();

  //   print("Data written to", outputFolder, "in", timer(), timer.unit());
  // }


  void addFolder(Path const & path, int const & nb_files = -1) {this -> setNbFiles(nb_files); m_MTreader.addFolder(path, m_nb_files);}
  void setDetectors (Detectors const & detectors)   {m_detectors = detectors;}
  void setTimeshifts(Timeshifts const & timeshifts) {m_timeshifts = timeshifts;}
  void setCalibration(Calibration const & calibration) {m_calibration = calibration;}
  void setOutDir(Path const & outputFolder, bool create = true) {m_outPath = Path(outputFolder,create);}
  void setNbFiles(int const & nb_files = -1) {m_nb_files = nb_files;}
  void convert() {m_MTreader.execute(s_convertFile,*this);}
  void buildEvents(bool const & eventbuilding = true) {m_eventbuilding = eventbuilding;}

private:
  static void s_convertFile(Hit & hit, FasterReader & reader, Convertor & convertor) {convertor.convertFile(hit,reader);}
  void convertFile(Hit & hit, FasterReader & reader);

  MTFasterReader m_MTreader;

  Timeshifts m_timeshifts;
  Detectors m_detectors;
  Calibration m_calibration;

  Path m_dataPath;
  Path m_outPath;
  bool m_eventbuilding = true;
  bool m_calibrate = false;
  bool m_nb_files = -1;
  bool m_ok = false;
};

void Convertor::convertFile(Hit & hit, FasterReader & reader)
{
  Nuball2Tree tree;

  tree.writting(hit,"ltEQp");

  if (m_calibration)
  {
    while(reader.Read())
    {
      hit.nrjcal = m_calibration(hit.nrj,hit.label);
      hit.nrj2cal = m_calibration(hit.nrj2,hit.label);
      if (isBGO[hit.label]) hit.nrjcal /= 100;
      tree -> Fill();
    }
  }
  else
  {
    while(reader.Read())
    {
      hit.nrjcal = hit.nrj;
      hit.nrj2cal = hit.nrj2;
      tree -> Fill();
    }
  }

  if (m_eventbuilding)
  {
    Event event; 
    CoincBuilder builder(&event, 500);
    tree.writting(event, "Nuball2","ltEQp");
    while(reader.Read())
    {
      event.push_back(hit);
      if (builder.build(hit))
      {
        hit.nrjcal = hit.nrj;
        hit.nrj2cal = hit.nrj2;
      }
      tree -> Fill();
    }
  }

  //Filename manipulations :
  File filename = reader.filename();
  
  tree.Write(m_outPath.string()+filename.shortName());
  
}

#endif //CONVERTOR_HPP