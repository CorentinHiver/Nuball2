// 1. Parameters
  // RF : 
#define USE_RF 200 //ns
  // Detectors :
#define USE_DSSD
#define USE_PARIS
  // Triggers :
#define TRIGGER
// #define KEEP_ALL

// 2. Include library
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want
#include <MultiHist.hpp>        // Use this to thread safely fill histograms
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <RF_Extractor.hpp>   // Extracts the first RF hit from a ttree

#include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
std::string IDFile = "index_129.list";
std::string calibFile = "136.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string time_ref = "301";
std::string timewindow = "1500";
int nb_files_ts = 20;
Long64_t min_time_ns = -50;
Long64_t max_time_ns = 180;
Long64_t min_time = min_time_ns*1000;
Long64_t max_time = max_time_ns*1000;
bool overwrite = false; // Overwrite already existing converted root files

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, Detectors const & detList, Calibration const & calibration, Timeshifts const & timeshifts, Path const & outPath)
{
  Timer timer;
  // Checking the lookup tables :
  if (!detList || !timeshifts || !calibration || !reader) return;

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  std::string run_path = raw_datafile.path(); // "/path/to/manip/run_number.fast/"
  std::string temp = run_path;                // "/path/to/manip/run_number.fast/"
  temp.pop_back();                            // "/path/to/manip/run_number.fast"
  std::string run = rmPathAndExt(temp);       //                "run_number"
  int run_number = std::stoi(lastPart(run,'_'));//                   number

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
  Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
  File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  // Initialise the temporary TTree :
  std::unique_ptr<TTree> readTree (new TTree("temp","temp"));
  readTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  readTree -> Branch("label"  , &hit.label );
  readTree -> Branch("time"   , &hit.time  );
  readTree -> Branch("nrjcal" , &hit.nrjcal);
  readTree -> Branch("nrj2"   , &hit.nrj2  );
  readTree -> Branch("pileup" , &hit.pileup);

  // Loop over the TTree :
  Timer read_timer;
  int count = 0;
  int RF_counter_raw = 0;
  while(reader.Read())
  {
    hit.time+=timeshifts[hit.label];
    hit.nrjcal = calibration(hit.nrj, hit.label);
    hit.nrj2 = calibration(hit.nrj2, hit.label);
    readTree -> Fill();
    count++;
    if (hit.label == RF_Manager::label) RF_counter_raw++;
  }
  read_timer.Stop();

#ifdef DEBUG
  print("Read finished here,", count,"counts and", RF_counter_raw, "RF counts, in", read_timer.TimeElapsedSec(),"s");
#endif //DEBUG

if (count==0) return;

  // Realign switched hits after timeshifts :
  Alignator gindex(readTree.get());

  // Switch the temporary TTree to reading mode :
  hit.reset();
  readTree -> ResetBranchAddresses();
  readTree -> SetBranchAddress("label"  , &hit.label );
  readTree -> SetBranchAddress("time"   , &hit.time  );
  readTree -> SetBranchAddress("nrjcal" , &hit.nrjcal);
  readTree -> SetBranchAddress("nrj2"   , &hit.nrj2);
  readTree -> SetBranchAddress("pileup" , &hit.pileup);

  // Initialise output TTree :
  std::unique_ptr<TFile> outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  TTree* outTree = new TTree("Nuball2","Nuball2");
  Event event(outTree, "ltnNp", "w");

  // Initialise event builder based on RF :
  RF_Manager rf;
  EventBuilder_136 eventBuilder(&event, &rf);

  // Initialise event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Handle the first RF downscaled :
  RF_Extractor first_rf(readTree.get(), rf, hit, gindex);
  if (!first_rf) return;
  eventBuilder.setFirstRF(hit);

  // Position in the root file
  int loop = 0;

  // Handle the first hit :
  readTree -> GetEntry(gindex[loop++]);
  eventBuilder.set_first_hit(hit);

  //Loop over the data :
  Timer convert_timer;
  auto const & nb_data = readTree->GetEntries();
  ulong hits_count = 0;
  ulong evts_count = 0;
  ulong RF_counter_written = 0;
  ulong sector_counter = 0;
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
      RF_counter_written++;
      continue;
    }

    readTree -> GetEntry(gindex[loop]);
    
    if(isSector[hit.label]) 
    {// Sectors trigger the event
      sector_counter++;
      auto const sector_time = hit.time;  // Timestamp  of the reference sector
      int loop2 = loop;// Actual position in the root file
      // Go back in time until we find a 
      do { 
        loop2--;
        readTree -> GetEntry(gindex[loop2]); 
        }
      while (static_cast<Long64_t>(hit.time - sector_time) > min_time && loop2>0);

      while(static_cast<Long64_t>(hit.time - sector_time) < max_time)
      {
        loop2++;
        readTree -> GetEntry(gindex[loop2]);
        event.push_back(hit);
      }

      loop = loop2;

      counter.count(event);

      if(counter.modules > 1 && counter.nb_Ge > 0)
      {
        outTree->Fill();
      }
      event.clear();
    }
    else loop++;

  }
  convert_timer.Stop();
#ifdef DEBUG
  print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s");
#endif //DEBUG
  Timer write_timer;
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  write_timer.Stop();

  float dataSize = raw_datafile.size("Mo");
  float outSize = size_file_conversion(outFile->GetSize(), "o", "Mo");

  timer();
  print(outfile, "written in", timer(), timer.unit()+". Input file", dataSize, "Mo and output file", outSize, "Mo : ", 100*count/loop, "% hits kept (", dataSize/outSize,"size factor)");
}

// 5. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string command = argv[i];
           if (command == "-m")
      {// Multithreading 
        nb_threads = atoi(argv[++i]);
      }
      else if (command == "-o")
      {// Overwright already existing .root files
        print("CAREFULL !! Already existing root files will be overwritten....");
        overwrite = true;
      }
    }
  }

  // MANDATORY : Initialise the multithreading !
  MTObject::Initialise(nb_threads);

  // Setup the path accordingly to the machine :
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp source code ^^^^^^^^"); return -1;}

  Path manipPath = datapath+manip;
  Path outPath (datapath+(manip.name()+"-root"), true);

  // Load some modules :
  Detectors detList(IDFile);
  Calibration calibration(calibFile, detList);
  Manip runs(File(manipPath+list_runs));

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
