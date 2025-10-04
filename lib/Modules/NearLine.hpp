#ifndef NEARLINE_HPP
#define NEARLINE_HPP

#include "../libRoot.hpp"

#include "Histogramor.hpp"

#include "../Classes/Detectors.hpp"

/**
 * @brief @todo
 * 
 */
class NearLine
{
public:
  NearLine(int argc, char ** argv);
  bool readParam(int argc, char ** argv);
  ~NearLine();

private:
  bool m_hr = false;
};

inline void CloverSpectra::printParameters()
{
  print("");
  print("Usage of NearLine : /path/to/data module [[parameters]]");
  print("");
  print("modules : ");
  print("--histo : Create the spectra histo");
  // print("parameters :");
  // print("");
  // print("-c [calibration_file]  : Loads the calibration file");
  // print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  // print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  // print("-i [ID_file]           : Load ID file");
  // print("-n [hits_number]       : Choose the number of hits to read inside each file");
  // print("-m [threads_number]    : Choose the number of files to treat in parallel");
  // print("-t [time_window_ns]    : Loads timeshift data");
  // print("--throw-singles        : Throw the single hits, i.e. no coincidence");
}

bool NearLine::readParam(int argc, char ** argv)
{
  if (argc<2)
  {
    printParameters();
    return false;
  }
  setDataPath(argv[1]);

  for (int i = 2; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-e") setTimeWindow_ns(std::stoi(argv[++i]));
    else if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    else if (command == "-i") detectors.load(argv[++i]);
    else if (command == "-m") setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-t") loadTimeshifts(argv[++i]);
    else if (command == "--throw-singles") m_throw_single = true;
    else if (command == "-w") setOutName(argv[++i]);
    else {print("Unkown command", command);}
  }
  return true;
}

NearLine::NearLine(int argc, char ** argv)
{
  if (!readParam) Colib::throw_error("Wrong parameters...");
}

NearLine::~NearLine()
{
}


#endif // NEARLINE_HPP