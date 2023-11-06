// #define USE_RF 200
// #define N_SI_136
// #define USE_DSSD
// #define USE_PARIS
// #define USE_RF 200

// #include "../../libRoot.hpp"
// #include <Detectors.hpp>
// #include <Calibration.hpp>

#include "ParisCalib.C"

int main(int argc, char ** argv)
{
  // // --- Up to date example : --- //
  int nb_files = -1;
  int nb_threads = 1;
  bool readRow = false;
  bool readSpectra = false;
  std::string folder = "/home/corentin/faster_data/N-SI-136/152_Eu_center_after.fast/";
  std::string index_file = "../index_129.list";
  if (argc > 1)
  {
    std::string command;
    for(int i = 1; i < argc; i++)
    {
      command = argv[i];
           if (command == "-f") {nb_files = std::atoi(argv[++i]);}
      else if (command == "-n") {FasterReader::setMaxHits(std::atoi(argv[++i]));}
      else if (command == "-m") 
      {
        nb_threads = std::atoi(argv[++i]);
        // MTObject::Initialize(nb_threads);
      }
      else if (command == "-i") {index_file = argv[++i];}
      else if (command == "r") {readRow = true;}
      else if (command == "s") {readSpectra = true;}
      else {throw std::runtime_error("command " + command + " unkown");}
    }
  }

  if (readRow) ParisCalib(folder, nb_files, nb_threads);
  else if (readSpectra) 
  {
    auto name = removeExtension(Path(folder).folder().name())+".root";
    analyseSpectra(name);
  }
  // print(nb_files);

  // detectors.load(index_file);

  // Calibration calib;
  // Calibration::treatOnlyParis(true);
  // calib.calculate("/home/corentin/faster_data/N-SI-136/152_Eu_center_after.fast/", 9);


  return 0;
}