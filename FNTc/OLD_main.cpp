// 1. Parameters
  // RF : 
#define USE_RF 200 //ns
  // Detectors :
#define USE_DSSD
#define USE_PARIS
  // Triggers :
#define TRIGGER
  // Event building :
#define PREPROMPT
// #define UNSAFE

// 2. Include library
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want²
#include <MTTHist.hpp>        // Use this to thread safely fill histograms
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <RF_Manager.hpp>     // Eases manipulation of RF information

// 5. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  std::string run = "";
  bool sinogram = false;
  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string const & command = argv[i];
      if (command == "-r" || command == "--run") run = argv[++i];
      else if (command == "-s" || command == "--sinogram") sinogram = true;
    }
  }

  if (run=="") throw_error("No run...");

  // Setup the path accordingly to the machine :
  Path path = "~/FNT/"+run;

  // Load some modules :
  detectors.load("index_FNTc.list");
  Calibration calib("FNTc.calib");
  Timeshifts timeshifts("FNTc.timeshifts");

  FilesManager files(path);

  std::string filename;
  while (files.getNext(filename))
  {
    Hit hit;
    FasterReader reader(&hit, filename);

    unique_tree tempTree(new TTree("temp","temp"));
    hit.writting(tempTree);

    while(reader.Read())
    {
      hit.stamp+=timeshifts[hit.label];
      tempTree->Fill();
    }

    hit.reset();
    hit.reading(tempTree);

    Alignator gindex(readTree.get());
    RF_Manager rf;
    RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);

    auto const & nb_hits = tempTree->GetEntries();

    for (int hit_i = 0; hit_i<nb_hits; hit_i++)
    {
      tempTree->GetEntries();
      //Hi, add stuff here
    }
    
  }

  return 1;
}
