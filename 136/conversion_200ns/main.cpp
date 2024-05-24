// Event building compilation parameters :
// #define FASTER_GROUP // If the data are grouped by faster, need to degroup them
// #define PREPROMPT    // EventBuilder_RF : add a preprompt (DO NOT WORK YET)
// #define UNSAFE       // To unlock a little bit of speed, should not be significant though... and may lead to unexpected surprises !

// 2. Include library
#include <MTObject.hpp> // Include this first to activate the multithreading additions (#define MULTITHREADING)
#include <MTTHist.hpp>  // Thread-safe wrapper around root histograms

#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <MTCounter.hpp>      // Use this to thread safely count what you want²
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors with respect to a reference
#include <EventBuilderRF.hpp> // Event builder based on RF

constexpr bool inline isPrompt(Time const & time)  noexcept {return -30_ns < time && time < 30_ns ;}
constexpr bool inline isDelayed(Time const & time) noexcept {return  30_ns < time && time < 180_ns;}

std::unordered_set<int> runs_preprompt = {13, 14, 15, 16, 18, 19, 20};
std::unordered_set<int> runs_blacklist = {22};
std::unordered_set<int> runs_use_previous_timeshifts = {25, 32, 38, 73};

#include "Trigger136.hpp"

// 3. Declare some global variables :
std::string IDFile = "../index_129.list";
std::string calibFile = "../136_2024.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string output = "-root_";
int nb_files_ts = 200;
int nb_files = -1;
int rf_shift = 20_ns;
int max_hits_in_event = -1;

bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool keep_rf = false; // Write the rf hits
bool fill_histos = false; // 
bool keep_first_hits = false; // Do not throw away the hits before the first rf downscale

std::string one_run_folder = "";
Path timeshifts_path = "";
ulonglong max_hits = -1;
bool treat_129 = false;

bool check_preprompt = false; // For timeshifts calculations : check there is no preprompt peak ()

bool extend_periods = false; // To take more than one period after a event trigger, do nothing anymore
uint nb_periods_more = 0; // Number of periods to extend after an event that triggered, do nothing anymore

// 4. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  if (argc > 1)
  {
    std::string list_trigger;
    for (auto const & trig : Trigger136::names) 
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
        else if (command == "-H" || command == "--histograms")
        {
          fill_histos = true;
          print("No histograms will be saved");
        }
        else if (                   command == "--keep-singles")
        {
          Builder::keepSingles(true);
        }
        else if (                   command == "--keep-first-hits")
        {
          keep_first_hits = true;
        }
        else if (                   command == "--keep-rf")
        {
          keep_rf = true;
        }
        else if (command == "-m" || command == "--multithread")
        {// Multithreading : number of threads
          nb_threads = atoi(argv[++i]);
        }
        else if (command == "-n" || command == "--number-hits-file")
        {// Number of hits per file
          FasterReader::setMaxHits(std::atoi(argv[++i]));
        }
        else if (command == "-N" || command == "--number-hits-event")
        {
          max_hits_in_event = std::atoi(argv[++i]);
        }
        else if (command == "-o" || command == "--overwrite")
        {// Overwrite already existing .root files
          overwrite = true;
        }
        else if (command == "-O" || command == "--output")
        {
          output = argv[++i];
        }
        else if (command == "-s" || command == "--rf-shif")
        {
          rf_shift = std::stoi(argv[++i]);
        }
        else if (command == "-t" || command == "--trigger")
        {
          Trigger136::choice = char_cast(std::stoi(argv[++i]));
          print("Trigger :", Trigger136::names[Trigger136::choice]);
        }
        else if (command == "-T" || command == "--timeshifts")
        {
          timeshifts_path = argv[++i];
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
        else if (command == "--run")
        {
          one_run_folder = argv[++i];
        }
        else if (command == "-h" || command == "--help")
        {
          print("List of the commands :");
          // print("(-e  || --extend-period)  [nb_periods]    : set the number of periods to extend the time window after trigger (not functional)");
          print("(-f  || --files-number)   [files-number]  : set the number of files to convert in each folder");
          print("(-h  || --help)                           : display this help");
          print("(       --keep-singles)                   : to keep singles hits");
          print("(       --keep-first-hits)                : to keep the hits before the first RF downscale");
          print("(       --keep-rf)                        : to keep the downscale pulsation hits in the output data");
          print("(-m  || --multithread)    [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
          print("(-n  || --number-hits)    [hits_number]   : set the number of hits to read in each file.");
          print("(-o  || --overwrite)                      : overwrites the already written folders. If a folder is incomplete, you need to delete it");
          print("(       --only-timeshift)                 : Calculate only timeshifts, force it even if it already has been calculated");
          print("(       --run)            [runName]       : set only one folder to convert");
          print("(-t  || --trigger)        [trigger]       : ", list_trigger, "|", Trigger136::legend);
          print("(-T  || --timeshifts)     [/path/to/run]  : set the path to a folder containing the required Timeshifts/ folder");
          print("(-Th || --Thorium)                        : Treats only the thorium runs (run_nb < 75)");
          print("(-U  || --Uranium)                        : Treats only the uranium runs (run_nb >= 75)");
          print("(       --129)                            : Treats the N-SI-129 pulsed runs");
          return 0;
        }
        else 
        {
          print(command, "command not recognized");
          exit(-1);
        }
      }
      catch(std::invalid_argument const & error) 
      {
        throw_error("Can't interpret " + std::string(argv[i]) + " as an integer...");
      }
    }
  }

  // MANDATORY : Initialise the multithreading !
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(nb_files);
  MTObject::Initialise();


  Path dataPath = Path::home()+"/nuball2";
  Path manipPath = dataPath+manip;

  // Output file :
  output+=Trigger136::names.at(Trigger136::choice);
  if (extend_periods) output+="_"+std::to_string(nb_periods_more)+"periods";
  Path outPath (dataPath+(manip.name()+output), true);

  print("Reading :", manipPath.string());
  print("Writing in : ", outPath.string());

  // Load some modules :
  detectors.load("index_129.list");
  Calibration calibration(calibFile);
  Manip runs(list_runs);
  if (one_run_folder!="") runs.setFolder(one_run_folder);

  // Checking that all the modules have been loaded correctly :
  if (!calibration) throw Calibration::NotFound(calibFile);
  if (!runs) throw Manip::NotFound(runs);

  // Setup some parameters :
  RF_Manager::setOffset(rf_shift);
  
  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    MTCounter total_read_size;
    Timer timer_total;
    auto const & run_path = manipPath+run;
    auto const & run_name = removeExtension(run);
    auto const & run_number = std::stoi(split(run_name, '_')[1]);
    if(found(runs_blacklist, run_number)) continue;

    MTTHist<TH1F> histo_mult_trigger;
    MTTHist<TH1F> labels_histo;
    MTTHist<TH1F> histo_mult;
    MTTHist<TH2F> pure_singles_cristals;
    MTTHist<TH2F> pure_singles_cristals_neutrons;
    MTTHist<TH2F> pure_single_Ge_VS_time;
    MTTHist<TH2F> cleanGe_VS_time;
    MTTHist<TH2F> cleanGe_trigged_VS_time;
    MTTHist<TH2F> cleanGe_trigger_particle_VS_time;

    // Initialise the histograms
    histo_mult_trigger.reset("mult_trig","mult_trig", 50,-1, 50);
    histo_mult.reset("mult","mult", 50,-1, 50);
    cleanGe_VS_time.reset("cleanGe_VS_time","cleanGe_VS_time", 400,-100_ns,300_ns, 10000,0,10000);
    pure_single_Ge_VS_time.reset("pure_single_Ge_VS_time","pure_single_Ge_VS_time", 400,-100_ns,300_ns, 10000,0,10000);
    pure_singles_cristals.reset("pure_singles_cristals","pure_singles_cristals", 1000,0,1000, 10000,0,10000);
    pure_singles_cristals_neutrons.reset("pure_singles_cristals_neutrons","pure_singles_cristals_neutrons", 1000,0,1000, 10000,0,10000);
    cleanGe_trigged_VS_time.reset("cleanGe_trigged_VS_time","cleanGe_trigged_VS_time", 400,-100_ns,300_ns, 10000,0,10000);
    cleanGe_trigger_particle_VS_time.reset("cleanGe_trigger_particle_VS_time","cleanGe_trigger_particle_VS_time", 400,-100_ns,300_ns, 10000,0,10000);
    labels_histo.reset("labels","labels", 1000,0,1000);

    print("----------------");
    print("Treating ", run_name);

    // Timeshifts loading : 
    Timeshifts timeshifts;
    if (check_preprompt) timeshifts.checkForPreprompt(true);

    // Creating a lambda that calculates the timeshifts directly from the data :
    auto calculateTimeshifts = [&](){
      if (treat_129)// for N-SI-129 :
      {
        for (Label i = 800; i<856; i++) timeshifts.dT_with_RF(i); // Using RF to align the DSSD
        for (Label i = 800; i<856; i++) timeshifts.dT_with_raising_edge(i); // Use the raising edge of the dT spectra because of the timewalk of DSSD in this experiment
        timeshifts.periodRF_ns(400);
      }
      else // for N-SI-136 :
      {
        timeshifts.dT_with_biggest_peak_finder(); // Best peak finder for N-SI-136
        for (Label i = 840; i<856; i++) timeshifts.dT_with_raising_edge(i);
        timeshifts.periodRF_ns(200);
      }

      timeshifts.setMult(2, 4);// Only events with 2 to 4 hits are included (more means they are less likely to be correlated)

      timeshifts.setOutDir(outPath.string());
      timeshifts.calculate(run_path, nb_files_ts);
      timeshifts.verify(run_path, 10);

      timeshifts.write(run_name);
    };

    std::string file_dT_str = outPath.string() + "Timeshifts/" + run_name + ".dT";
    if (timeshifts_path.string() != "" && timeshifts_path.exists()) file_dT_str = timeshifts_path.string() + "Timeshifts/" + run_name + ".dT";
    if (found(runs_use_previous_timeshifts, run_number)) file_dT_str = timeshifts_path.string() + "Timeshifts/run_" + std::to_string(run_number-1) + ".dT";
    File file_dT(file_dT_str);

    // If only overwriting the timeshifts then no need to try to load it first :
    if(overwrite && only_timeshifts) {calculateTimeshifts(); continue;}
    else
    {
      try // Try to load the data
      {
        timeshifts.load(file_dT.string());
      }
      catch(Timeshifts::NotFoundError & error)
      { // If no timeshifts data if available for the run already, calculate it from the data :
        calculateTimeshifts();
      }
      if (only_timeshifts) continue;
    }

    // Loop over the files in parallel :
    MTFasterReader readerMT(run_path, nb_files);
    readerMT.readRaw([&](Hit & hit, FasterReader & reader)
    {
      //////////////////////////////////////
      //    READ EACH FILE IN PARALLEL    //
      //////////////////////////////////////

      Timer timer;
      // Checking the lookup tables :
      if (!timeshifts || !calibration || !reader) return;

      // Extracting the run name :
      File raw_dataFile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_fileNumber.fast"
      Filename filename = raw_dataFile.filename();// "                               run_number_fileNumber.fast"
      int fileNumber = std::stoi(lastPart(filename.shortName(), '_'));//                        fileNumber
      std::string run = removeLastPart(filename.shortName(), '_'); 

      if (run == "run_22") return;
      // if (run == "run_20" || "run_19") for (int label = 0; label<detectors.size(); ++label) timeshifts[label]-=30_ns;

      // Setting the name of the output file :
      Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
      Filename outFilename(raw_dataFile.shortName()+".root"); //                                     run_number_fileNumber.root
      File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_fileNumber.root

      // Important : if the output file already exists, then do not overwrite it !
      if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

      total_read_size+=raw_dataFile.size();
      auto dataSize = float_cast(raw_dataFile.size("Mo"));

      // ------------------------------ //
      // Initialise the temporary TTree //
      // ------------------------------ //
      std::string tree_name = run_name + "_" + std::to_string(fileNumber);
      std::unique_ptr<TTree> tempTree (new TTree(tree_name.c_str(),tree_name.c_str()));
      tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
      hit.writing(tempTree.get(), "lteq"); // Writting label, absolute timestamp, adc and qdc2 into temporary tree

      // ------------------------ //
      // Loop over the .fast file //
      // ------------------------ //
      Timer read_timer;
      while(reader.Read())
      {
        // Time calibration :
        hit.stamp+=timeshifts[hit.label];
        // There is a error for most DSSD rings when the timeshift have been calculated, some are slightly too early
        // Pushing all of them by 50 ns allows us to ensure they all fit in the event they belong to
        if (839 < hit.label  && hit.label < 856) hit.stamp += 50_ns;
        if (hit.label == 251) hit.nrj = float_cast(hit.adc);
      
        tempTree -> Fill();
      }
      
      auto rawCounts = tempTree->GetEntries();

      read_timer.Stop();
      
      print("Read of",raw_dataFile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

      if (rawCounts==0) {print("NO HITS IN", run); return;}

      // -------------------------------------- //
      // Realign switched hits after timeshifts //
      // -------------------------------------- //
      Timer convert_timer;

      Alignator gIndex(tempTree.get());

      // ------------------------------------------------------ //
      // Prepare the reading of the TTree and the output Events //
      // ------------------------------------------------------ //
      // Switch the temporary TTree to reading mode :
      hit.clear();
      hit.reading(tempTree.get(), "lteq"); // Reading label, timestamp, adc, qdc2

      std::string const outTree_name = "Nuball2"+std::to_string(MTObject::getThreadIndex());
      auto outTree (new TTree(outTree_name.c_str(), "Nuball2"));
      outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - so much faster if enough RAM
      Event event;
      event.writing(outTree, "ltTEQ"); // Writing labels, timestamp, relative times, calibrated energies, calibrated qdc2s

      // Initialise object that handles the rf
      RF_Manager rf;
      rf.setPeriod_ns(200);

      // Handle the first RF downscale :
      RF_Extractor first_rf(tempTree.get(), rf, hit, gIndex); // The hit now contains the first downscaled RF
      if (!first_rf) return;
      rf.setHit(hit);

      debug("first RF found at hit n°", first_rf.cursor());

      // Initialise event builder based on RF :
      EventBuilderRF eventBuilder(&event, &rf);
      eventBuilder.setFirstHit(hit);

      // Initialise event analyzer :
      Trigger136 trigger(&event);

      // Handle the first hit //
      int loop = 0;
      // In the first file of each run, the first few hundreds of thousands of hits don't have RF downscale. 
      // So we have to skip them in order to maintain good temporal resolution of all events:
      if (!keep_first_hits) loop = first_rf.cursor();
      
      tempTree -> GetEntry(gIndex[loop++]);

      // ---------------------------- //
      // Loop over the realigned data //
      // ---------------------------- //
      auto const & nb_data = tempTree->GetEntries();
      ulong evts_count = 0;
      ulong trig_count = 0;
      ulong trig_hits_count = 0;
      for (;loop<nb_data; ++loop)
      {
        tempTree -> GetEntry(gIndex[loop]);

        // Energy calibration : 
        hit.nrj = calibration(hit.adc,  hit.label); // Normal calibration
        hit.nrj2 = ((hit.qdc2 == 0) ? 0.f : calibration(hit.qdc2, hit.label)); // Calibrate the qdc2 if any

        if (rf.setHit(hit))
        { // Handle rf
          if (keep_rf)
          {// Write down the rf in the data if the option keep-rf is activated
            Event tempEvent (event);
            event = hit;
            outTree -> Fill();
            event = tempEvent;
            ++trig_hits_count; // To count the hit
          }
          // The rf hits cannot participate in the construction of an event as it is not a detector
          continue;
        }

        if (eventBuilder.build(hit))
        {// Here, an event is made of at least of 2 hits (but if the Builder::keepSingles() is activated, can be only one)
         // in coincidence within the time window centered around the pulsation t0 (between -rf_shift and period-rf_shift)
          ++evts_count;
          if (max_hits_in_event > 0  &&  event.mult > max_hits_in_event) continue;
          if ((evts_count%(int)(1.E+6)) == 0) debug(evts_count/(int)(1.E+6), "MEvts");
          bool pass_trigger = trigger.pass();
          if (fill_histos)
          {
            for (auto const & index_i : trigger.clovers.GeClean) cleanGe_VS_time.Fill(trigger.clovers[index_i].time, trigger.clovers[index_i].nrj);
            histo_mult.Fill(event.mult);
          }

          if (pass_trigger)
          {
            if (fill_histos) 
            {
              for (auto const & index_i : trigger.clovers.GeClean) cleanGe_trigged_VS_time.Fill(trigger.clovers[index_i].time, trigger.clovers[index_i].nrj);
              histo_mult_trigger.Fill(event.mult);
            }
            trig_hits_count+=event.mult;
            ++trig_count;

            outTree->Fill();
          }
          if (fill_histos && trigger.nb_dssd > 0) for (auto const & index_i : trigger.clovers.GeClean) cleanGe_trigger_particle_VS_time.Fill(trigger.clovers[index_i].time, trigger.clovers[index_i].nrj);
        }
        if (eventBuilder.isSingle())
        {
          if (Builder::getKeepSingles()) {outTree->Fill(); ++trig_count; ++trig_hits_count;}

          if (fill_histos)
          {
            // Fill histo :
            auto const & label = eventBuilder.getEvent()->labels[0];
            auto const & time = eventBuilder.getEvent()->times[0];
            auto const & nrj = eventBuilder.getEvent()->nrjs[0];
            auto const & nrj2 = eventBuilder.getEvent()->nrj2s[0];

            if (CloversV2::isGe(label))
            {
              pure_single_Ge_VS_time.Fill(time, nrj);
              cleanGe_VS_time.Fill(time, nrj);
            }

            if (300 < label && label < 800) if (0.8 < nrj/nrj2 || nrj/nrj2 > 1.2) continue; // Keep only LaBr3 part from paris
            pure_singles_cristals.Fill(label, nrj);
            if (10_ns < time && time < 50_ns) pure_singles_cristals_neutrons.Fill(label, nrj);
          }
        }
        // if (eventBuilder.isSingle()) {print(*(eventBuilder.getEvent())); pauseCo();}
      }
      print("Conversion finished here done in", convert_timer() , " (",dataSize/convert_timer.TimeSec() ,"Mo/s)");
      Timer write_timer;

      // Write output TTree in file :

      unique_TFile outFile (TFile::Open(outfile.c_str(), "RECREATE"));
      outFile -> cd();
      outTree -> SetDirectory(outFile.get());
      outTree -> Write("Nuball2", TObject::kOverwrite);
      outFile -> Close();

      write_timer.Stop();

      auto outSize  = float_cast(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

      print(outfile, "written in", timer(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
            "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.l*trig_hits_count)/(1.l*nb_data),"% hits kept",
            "|", (100.l*trig_count)/(1.l*evts_count), "% evts");
    });

    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer_total.TimeSec(), "Mo/s");

    // Merging the files using hadd command from ROOT software :

    Path outDataPath(outPath.string() + "/" + run_name);
    Path outMergedPath (outPath.string()+"merged", true);
    File outMergedFile (outMergedPath, run_name+".root");

    if (outMergedFile.exists() && !overwrite) continue;

    auto nb_threads_hadd = int_cast(std::thread::hardware_concurrency()*0.25);
    auto const & outRootName = outMergedPath.string() + run_name + ".root ";
    // hadd options : -v 1 for more minimum verbosity, -j for multithreading, -d to write the sub-files (due to multithreading) in current repository
    std::string merge_command = "hadd -v 1 -d . -j " + std::to_string(nb_threads_hadd)+ " ";
    if (overwrite) merge_command+= "-f "; // -f is to force overwriting of the merged file
    merge_command += outRootName + outDataPath.string() + run_name + "_*.root";
    print(merge_command);
    system(merge_command.c_str());

    // Write down the histograms :

    unique_TFile histoFile (TFile::Open(outRootName.c_str(), "UPDATE"));
    histoFile->cd();
    cleanGe_VS_time.Write();
    pure_single_Ge_VS_time.Write();
    cleanGe_trigged_VS_time.Write();
    cleanGe_trigger_particle_VS_time.Write();
    histo_mult.Write();
    histo_mult_trigger.Write();
    labels_histo.Write();
    pure_singles_cristals.Write();
    pure_singles_cristals_neutrons.Write();
    histoFile->Close();
  }
  return 0;
}