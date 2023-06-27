// 1. Parameters
  // Detectors :
#define USE_LICORNE
  // Triggers :
#define KEEP_ALL

// 2. Include library
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want
#include <MTTHist.hpp>        // Use this to thread safely fill histograms
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <RF_Extractor.hpp>   // Extracts the first RF hit from a ttree

#include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
std::string IDFile = "index_120.list";
std::string calibFile = "calib_120.dat";
Folder manip = "N-SI-120";
std::string list_runs = "calib_files.list";
std::string time_ref = "201";
std::string timewindow = "1500";
int nb_files_ts = 20;
bool overwrite = true; // Overwrite already existing converted root files

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, DetectorsList const & detList, Calibration const & calibration, Timeshifts const & timeshifts, Path const & outPath)
{
  // Checking the lookup tables :
  if (!detList || !timeshifts || !calibration) return;

  // Extracting the run name :
  Filename filename = reader.getFilename(); // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  std::string run_path = filename.path();   // "/path/to/manip/run_number.fast/"
  std::string temp = run_path;              // "/path/to/manip/run_number.fast/"
  temp.pop_back();                          // "/path/to/manip/run_number.fast"
  std::string run = rmPathAndExt(temp);     //                "run_number"

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                      // /path/to/manip-root/run_number.fast/
  std::string outfile = outFolder+filename.name()+".root"; // /path/to/manip-root/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> readTree (new TTree("temp","temp"));
  readTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  readTree -> Branch("label"  , &hit.label );
  readTree -> Branch("time"   , &hit.time  );
  readTree -> Branch("nrjcal" , &hit.nrjcal);
  readTree -> Branch("nrj2"   , &hit.nrj2  );
  readTree -> Branch("pileup" , &hit.pileup);

  // Loop over the TTree :
  int count = 0;
  int RF_counter = 0;
  while(reader.Read())
  {
    hit.time+=timeshifts[hit.label];
    hit.nrjcal = calibration(hit.nrj, hit.label);
    readTree -> Fill();
    count++;
    if (hit.label == RF_Manager::label) RF_counter++;
  }

#ifdef DEBUG
  print("Read finished here");
#endif //DEBUG

  // Switch the temporary TTree to reading mode :
  hit.reset();
  readTree -> ResetBranchAddresses();
  readTree -> SetBranchAddress("label"  , &hit.label );
  readTree -> SetBranchAddress("time"   , &hit.time  );
  readTree -> SetBranchAddress("nrjcal" , &hit.nrjcal);
  readTree -> SetBranchAddress("nrj2"   , &hit.nrj2);
  readTree -> SetBranchAddress("pileup" , &hit.pileup);

  // Realign shuffled hits after timeshifts :
  Alignator gindex(readTree.get());

  // Initialize output TTree :
  std::unique_ptr<TTree> outTree(new TTree("Nuball2","Nuball2"));
  Event event(outTree.get(), "ltnNp", "w");

  // Initialize event builder based on RF :
  RF_Manager rf;
  EventBuilder_136 eventBuilder(&event, &rf);

  // Initialize event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Handle the first RF downscale :
  RF_Extractor first_rf(readTree.get(), rf, hit, gindex);
  if (!first_rf) return;
  eventBuilder.setFirstRF(hit);

  // Handle the first hit :
  int loop = 0;
  readTree -> GetEntry(gindex[loop++]);
  eventBuilder.set_first_hit(hit);

  //Loop over the data :
  auto const & nb_data = readTree->GetEntries();
  while (loop<nb_data)
  {
    readTree -> GetEntry(gindex[loop++]);

    // Handle the RF data :
    if (hit.label == RF_Manager::label)
    {
      auto temp = event;
      event = hit;
      outTree -> Fill();
      event = temp;
      rf.setHit(hit);
      continue;
    }

    // Event building :
    if (eventBuilder.build(hit))
    {
      counter.count(event); 
    #ifdef M2G1_TRIG
      if (counter.nb_modules>2 && counter.clovers.size()>1)
      {
        outTree->Fill();
      }
    #else
      outTree->Fill();
    #endif
    }
  #ifdef KEEP_ALL
    if (eventBuilder.isSingle())
    {
      auto temp = event;
      event = eventBuilder.singleHit();
      outTree -> Fill();
      event = temp;
      continue;
    }
  #endif
  }
#ifdef DEBUG
  print("Conversion finished here");
#endif //DEBUG
  std::unique_ptr<TFile> outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();
  print(outfile, "written");
}

// 5. Main
int main(int argc, char** argv)
{

  // MANDATORY : initialize the multithreading !
  MTObject::Initialize(2);


  // MANDATORY : initialize the detectors labels manager !
  Detectors::Initialize();

  // Setup the path accordingly to the machine :
    Path datapath = std::string(std::getenv("HOME"));
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line ---|---"); return -1;}

  Path manipPath = datapath+manip;
  Path outPath (datapath+manip.name()+"-root", true);

  // Load some modules :
  DetectorsList detList(IDFile);
  Calibration calibration(calibFile, detList);
  Manip runs(manipPath+list_runs);

  // Checking of all the modules have been loaded correctly :
  if (!detList || !calibration || !runs) return -1;

  // Loop sequentially through the runs and treat theirs files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    Path runpath = manipPath+run;
    auto run_name = removeExtension(run);

    // Timeshifts loading : 
    print("----------------");
    print("Treating ", run_name);
    Timeshifts timeshifts(outPath,run_name);

    // If no timeshifts data already available, calculate it :
    if (!timeshifts) 
    { 
      timeshifts.setDetectorsList(detList);
      timeshifts.setOutDir(outPath);
      timeshifts.setOutRoot(run_name+".root");
      timeshifts.setOutData(run_name+".dT");
      timeshifts.setMult(2,3);
      // timeshifts.setMaxHits(10000);
      timeshifts.calculate(runpath, nb_files_ts);
    }

    // Loop over the files in parallel :
    MTFasterReader readerMT(runpath);
    readerMT.execute(convert, detList, calibration, timeshifts, outPath);
  }

  return 1;
}
