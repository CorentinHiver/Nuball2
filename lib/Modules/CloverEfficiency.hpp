#ifndef CLOVEREFFICIENCY_HPP
#define CLOVEREFFICIENCY_HPP

#include "../Analyse/Clovers.hpp"

class CloverEfficiency
{
public:
  CloverEfficiency(int argc, char** argv)
  {
    this -> Initialize(argc, argv);
  }
  void Initialize(int argc, char** argv);
  void printParameters();
  void setTimeshifts(Timeshifts const & timeshifts) {m_timeshifts = timeshifts;}
  void loadTimeshifts(std::string const & dTfile)   {m_timeshifts.load(dTfile);}
  void setCalibration(Calibration const & calibration) {m_calibration = calibration;}
  void loadCalibration(std::string const & calibFile)  {m_calibration.load(calibFile);}
  void setNbFiles(int const & nb_files = -1) {m_nb_files = nb_files;}
  void setNbThreads(int const & nb_threads = -1) {m_nb_threads = nb_threads;}
  void setTimeWindow(int const & timewindow = 1500)

  run(Path const & m_dataPath, File const & outName);


private:
Calibration m_calibration;
int m_timewindow

};

CloverEfficiency::run(Path const & m_dataPath, File const & outName);

void CloverEfficiency::printParameters()
{
  print("");
  print("Usage of CloverEfficiency : /path/to/data outputFile [[parameters]]");
  print("");
  print("parameters :");
  print("");
  print("-c [calibration_file]  : Loads the calibration file");
  print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-i [ID_file]           : Load ID file");
  print("-n [hits_number]       : Choose the number of hits to read inside each file");
  print("-m [threads_number]    : Choose the number of files to treat in parallel");
  print("-t [time_window_ns]    : Loads timeshift data");
}

void CloverEfficiency::Initialize(int argc, char** argv)
{
  if (argc<2) 
  {
    print("Not enough parameters !!!");
    printParameters();
    throw_error("Parameters");
  }

  Path m_dataPath(argv[1]);
  File outName(argv[2]);

  for (int i = 3; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-e") setTimeWindow(std::stoi(argv[++i]));
    else if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    else if (command == "-i") detectors.load(argv[++i]);
    else if (command == "-m") setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-t") loadTimeshifts(argv[++i]);
    else if (command == "--throw-singles") m_throw_single = true;
    else {print("Unkown command", command);}
  }
}

#endif CLOVEREFFICIENCY_HPP