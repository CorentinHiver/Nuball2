// 1. Global compilation parameters
  // RF : 
  // Detectors :
  // Triggers :
#define TRIGGER
  // Event building :
// #define PREPROMPT
// #define UNSAFE

// 2. Include library
#include <MTObject.hpp>
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want²
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information

#include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
std::string IDFile = "../index_129.list";
std::string calibFile = "../136_2024.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string output = "-root_";
int nb_files_ts = 60;
int nb_files = -1;
int rf_shift = 20;
bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool one_run = false;
bool check_preprompt = false;
bool write_rf = false;
std::string one_run_folder = "";
ulonglong max_hits = -1;
bool treat_129 = false;

bool extend_periods = false; // To take more than one period after a event trigger
uint nb_periods_more = 0; // Number of periods to extend after an event that triggered

char trigger_choice = -1;
std::map<char, std::string> trigger_name = 
{
  {-1,"all"},
  {0, "P"},
  {1, "M3G1"},
  {2, "P_M3G1"},
  {3, "PM2G1"},
  {4, "P_M4G1"},
  {5, "M4G1"},
  {6, "G2"},
  {7, "C2"},
  {8, "PC2"},
  {9, "PC1"}
};

std::string trigger_legend = "Legend : P = particle. G = Germanium. M = Module. _ = OR.";

bool trigger(Counter136 & counter)
{
  switch (trigger_choice)
  {
    case 0: return counter.nb_dssd>0; //P
    case 1: return counter.nb_modules>2 && counter.nb_Ge>0; //M3G1
    case 2: return (counter.nb_dssd>0 || (counter.nb_modules>2 && counter.nb_Ge>0)); //P_M3G1
    case 3: return (counter.nb_dssd>0 && counter.nb_modules>1 && counter.nb_Ge>0); //PM2G1
    case 4: return (counter.nb_dssd>0 || (counter.nb_modules>3 && counter.nb_Ge>0)); //P_M4G1
    case 5: return (counter.nb_modules>3 && counter.nb_Ge>0); //M4G1
    case 6: return counter.nb_Ge > 1; // G2
    
    case 7: counter.analyse(); return counter.nb_clovers_clean > 1; //C2
    case 8: counter.analyse(); return counter.nb_clovers_clean > 1 && counter.nb_dssd > 0; // PC2
    case 9: counter.analyse(); return counter.nb_clovers_clean > 0 && counter.nb_dssd > 0; // PC2
    default: return true;
  }
}

// 4. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  if (argc > 1)
  {
    std::string list_trigger;
    for (auto const & trig : trigger_name) 
    {
      list_trigger.append(std::to_string(trig.first)+std::string(" : ")+trig.second+std::string(". "));
    }
    for(int i = 1; i < argc; i++)
    {
      try
      {
        std::string command = argv[i];
            if (command == "-e" || command == "--extend-period")
        {// To get more than 1 period after pulse if trigger activated 
          extend_periods = true;
          nb_periods_more = atoi(argv[++i]);
        }
        else if (command == "-f" || command == "--files-number")
        {
          nb_files = atoi(argv[++i]);
        }
        else if (command == "--run")
        {
          one_run = true;
          one_run_folder = argv[++i];
        }
        else if (command == "-m" || command == "--multithread")
        {// Multithreading : number of threads
          nb_threads = atoi(argv[++i]);
        }
        else if (command == "-n" || command == "--number-hits")
        {// Number of hits per file
          FasterReader::setMaxHits(std::atoi(argv[++i]));
        }
        else if (command == "-o" || command == "--overwrite")
        {// Overwright already existing .root files
          overwrite = true;
        }
        else if (command == "-O" || command == "--output")
        {
          output = argv[++i];
        }
        else if (command == "-t" || command == "--trigger")
        {
          trigger_choice = std::stoi(argv[++i]);
        }
        else if (                   command == "--only-timeshift")
        {
          only_timeshifts = true;
        }
        else if (command == "-Th" || command == "--Thorium")
        {
          list_runs = "Thorium.list";
          check_preprompt = true;
        }
        else if (command == "-U" || command == "--Uranium")
        {
          list_runs = "Uranium.list";
        }
        else if (                   command == "--129")
        {
          treat_129 = true;
          list_runs = "129.list";
          calibFile = "129.calib";
          manip = "N-SI-129";
          check_preprompt = true;
        }
        else if (                   command == "--write-rf")
        {
          write_rf = true;
        }
        else if (command == "-h" || command == "--help")
        {
          print("List of the commands :");
          print("(-f  || --files-number)   [files-number]  : set the number of files");
          print("(-e  || --extend-period)  [nb_periods]    : set the number of periods to extend the time window after trigger");
          print("(       --run)            [runname]       : set only one folder to convert");
          print("(-h  || --help)                           : display this help");
          print("(-m  || --multithread)    [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
          print("(-n  || --number-hits)    [hits_number]   : set the number of hit to read in each file.");
          print("(-o  || --overwrite)                      : overwrites the already written folders. If a folder is incomplete, you need to delete it");
          print("(       --only-timeshift)                 : Calculate only timeshifts, force it even if it already has been calculated");
          print("(-t  || --trigger)        [trigger]       : Default ",list_trigger,"|", trigger_legend);
          print("(-Th || --Thorium)                        : Treats only the thorium runs (run_nb < 75)");
          print("(-U  || --Uranium)                        : Treats only the uranium runs (run_nb >= 75)");
          print("(       --129)                            : Treats the N-SI-129 pulsed runs");
          print("(       --write-rf)                       : Include the downscaled pulsation hits in the output data");
          return 0;
        }
      }
      catch(std::invalid_argument const & error) 
      {
        throw_error("Can't interpret " + std::string(argv[i]) + " as an integer");
      }
    }
  }

  // MANDATORY : Initialise the multithreading !
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(nb_files);
  MTObject::Initialise();

  // Setup the path accordingly to the machine :
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp ^^^^^^^^"); return -1;}

  // Input file :
  Path manipPath = datapath+manip;

  // Output file :
  output+=trigger_name.at(trigger_choice);
  if (extend_periods) output+="_"+std::to_string(nb_periods_more)+"periods";
  Path outPath (datapath+(manip.name()+output), true);

  print("Reading :", manipPath.string());
  print("Writting in : ", outPath.string());

  // Load some modules :
  detectors.load("index_129.list");
  Calibration calibration(calibFile);
  Manip runs(list_runs);
  if (one_run) runs.setFolder(one_run_folder);

  // Checking that all the modules have been loaded correctly :
  if (!calibration || !runs) return -1;

  // Setup some parameters :
  RF_Manager::set_offset_ns(rf_shift);
  
  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    MTCounter total_read_size;
    Timer timer;
    auto const & run_path = manipPath+run;
    auto const & run_name = removeExtension(run);

    print("----------------");
    print("Treating ", run_name);

    // Timeshifts loading : 

    // Creating a lambda that calculates the timeshifts directly from the data :
    auto calculateTimeshifts = [&](Timeshifts & timeshifts){
      if (treat_129)// for N-SI-129 :
      {
        for (int i = 800; i<856; i++) timeshifts.dT_with_RF(i); // Using RF to align the DSSD
        for (int i = 800; i<856; i++) timeshifts.dT_with_raising_edge(i); // Use the raising edge of the dT spectra because of the timewalk of DSSD in this experiment
        timeshifts.periodRF_ns(400);
      }
      else // for N-SI-136 :
      {
        timeshifts.dT_with_biggest_peak_finder(); // Best peak finder for N-SI-136
        timeshifts.periodRF_ns(200);
      }

      timeshifts.setMult(2, 4);// Only events with 2 to 4 hits are included

      timeshifts.setOutDir(outPath.string());
      timeshifts.calculate(run_path, nb_files_ts);
      timeshifts.verify(run_path, 10);

      timeshifts.write(run_name);
    };
    Timeshifts timeshifts;
    File file(outPath.string() + "Timeshifts/" + run_name + ".dT");

    if(overwrite && only_timeshifts) {calculateTimeshifts(timeshifts);}
    else
    {
      try
      {
        timeshifts.load(file.string());
      }
      catch(Timeshifts::NotFoundError & error)
      { // If no timeshifts data if available for the run already, calculate it :
        calculateTimeshifts(timeshifts);
      }
      if (only_timeshifts) continue;
    }

    // Loop over the files in parallel :
    MTFasterReader readerMT(run_path, nb_files);

    readerMT.readRaw([&](Hit & hit, FasterReader & reader){

      //////////////////////////////////////
      //    READ EACH FILE IN PARALLEL    //
      //////////////////////////////////////

      Timer timer;
      // Checking the lookup tables :
      if (!timeshifts || !calibration || !reader) return;

      // Extracting the run name :
      File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
      Filename filename = raw_datafile.filename();// "                               run_number_filenumber.fast"
      int filenumber = std::stoi(lastPart(filename.shortName(), '_'));//                        filenumber
      std::string run = removeLastPart(filename.shortName(), '_'); 

      // Setting the name of the output file :
      Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
      Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
      File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

      // Important : if the output file already exists, then do not overwrite it !
      if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

      total_read_size+=raw_datafile.size();
      auto dataSize = float_cast(raw_datafile.size("Mo"));

      // ------------------------------ //
      // Initialise the temporary TTree //
      // ------------------------------ //
      std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
      tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
      hit.writting(tempTree.get(), "lsEQp");

      // ------------------------ //
      // Loop over the .fast file //
      // ------------------------ //
      Timer read_timer;
      while(reader.Read())
      {
        // Time calibration :
        hit.stamp+=timeshifts[hit.label];

        // Energy calibration : 
        hit.nrj = calibration(hit.adc,  hit.label); // Normal calibration
        hit.nrj2 = ((hit.qdc2 == 0) ? 0.f : calibration(hit.qdc2, hit.label)); // Calibrate the qdc2 if any
      
        tempTree -> Fill();
      }
      
      auto rawCounts = tempTree->GetEntries();

      read_timer.Stop();
      
      print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

      if (rawCounts==0) {print("NO HITS IN",run); return;}

      // -------------------------------------- //
      // Realign switched hits after timeshifts //
      // -------------------------------------- //

      Alignator gindex(tempTree.get());

      // ------------------------------------------------------ //
      // Prepare the reading of the TTree and the output Events //
      // ------------------------------------------------------ //
      // Switch the temporary TTree to reading mode :
      hit.reset();
      hit.reading(tempTree.get(), "lsEQp");

      std::string const outTree_name = "Nuball2"+std::to_string(MTObject::getThreadIndex());
      TTree* outTree (new TTree(outTree_name.c_str(),"Nuball2"));
      outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
      Event event;
      event.writting(outTree, "lstEQp");

      // Initialise event builder based on RF :
      RF_Manager rf;
      rf.set_period_ns(200);

      // Handle the first RF downscale :
      RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
      if (!first_rf) return;

      debug("first RF found at hit n°", first_rf.cursor());

      // Initialise event builder :
      EventBuilder_136 eventBuilder(&event, &rf);
      eventBuilder.setFirstRF(hit);

      // Initialise event analyser :
      Counter136 counter(&event);

      // Handle the first hit :
      int loop = 0;
      // In the first file of each run, the first few hundreds of thousands of hits don't have RF downscale. 
      // So we have to skip them in order to maintain good temporal resolution :
      if (filenumber == 1) loop = first_rf.cursor(); 
      tempTree -> GetEntry(gindex[loop++]);
      eventBuilder.set_first_hit(hit);

      // --------------------------------- //
      // Loop over the temporary root tree //
      // --------------------------------- //
      Timer convert_timer;
      auto const & nb_data = tempTree->GetEntries();
      ulong hits_count = 0;
      ulong evts_count = 0;
      ulong trig_count = 0;
      while (loop<nb_data)
      {
        tempTree -> GetEntry(gindex[loop++]);

        if (rf.setHit(hit))
        { // Handle rf
          if (write_rf)
          {
            Event temp (event);
            event = hit;
            outTree -> Fill();
            event = temp;
          }
          continue;
        }
        
        if (eventBuilder.build(hit))
        {
          evts_count++;
          counter.count();
          if ((evts_count%(int)(1.E+6)) == 0) debug(evts_count/(int)(1.E+6), "Mevts");
          if (trigger(counter))
          {

            hits_count+=event.size();
            ++trig_count;

            outTree->Fill();
          }
        }
      }
      convert_timer.Stop();
      print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s (",dataSize/convert_timer.TimeElapsedSec() ,"Mo/s)");
      Timer write_timer;

      // Initialise output TTree :
      unique_TFile outFile (TFile::Open(outfile.c_str(), "RECREATE"));
      outFile -> cd();
      outTree -> SetDirectory(outFile.get());
      outTree -> Write("Nuball2", TObject::kOverwrite);
      outFile -> Close();

      write_timer.Stop();

      auto outSize  = float_cast(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

      print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
            "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.*double_cast(hits_count))/double_cast(rawCounts),"% hits kept");
    });
    
    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer.TimeSec());
  }
  return 1;
}
;