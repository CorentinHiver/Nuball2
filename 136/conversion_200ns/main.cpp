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
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information

#include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
std::string IDFile = "index_129.list";
std::string calibFile = "136_final.calib";
Folder manip = "N-SI-129";
std::string list_runs = "list_runs.list";
std::string output = "-root_P";
int  nb_files_ts = 50;
int  nb_files = -1;
int rf_shift = 40;
bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool histoed = false;
bool one_run = false;
std::string one_run_folder = "";
ulonglong max_hits = -1;
bool treat_129 = true;

bool extend_periods = false; // To take more than one period after a event trigger
uint nb_periods_more = 0; // Number of periods to extend after an event that triggered

bool trigger(Counter136 const & counter)
{
  // return true;
  return (counter.nb_dssd>0);
  // return ((counter.nb_modules>2 && counter.nb_clovers>0));
  // return ((counter.nb_modules>2 && counter.nb_clovers>0) || counter.nb_dssd>0);
}

struct Histos
{
  MTTHist<TH1F> energy_all_Ge_raw;
  MTTHist<TH1F> rf_all_raw;
  MTTHist<TH2F> energy_each_raw;
  MTTHist<TH2F> rf_each_raw;

  MTTHist<TH1F> energy_all_Ge;
  MTTHist<TH1F> rf_all;
  MTTHist<TH2F> energy_each;
  MTTHist<TH2F> rf_each;

  MTTHist<TH1F> energy_all_Ge_event;
  MTTHist<TH1F> rf_all_event;
  MTTHist<TH2F> energy_each_event;
  MTTHist<TH2F> rf_each_event;

  MTTHist<TH1F> energy_all_Ge_trig;
  MTTHist<TH1F> rf_all_trig;
  MTTHist<TH2F> energy_each_trig;
  MTTHist<TH2F> rf_each_trig;

  void Initialize(Detectors const & detectors)
  {
    auto const & nbDet = detectors.number();

    energy_all_Ge_raw.reset("energy_all_Ge_raw", "Ge spectra raw", 20000,0,10000);
    rf_all_raw.reset("rf_all_raw", "RF Time spectra raw", 2000, -1200, 800);
    energy_each_raw.reset("energy_each_raw", "Energy spectra each raw", nbDet,0,nbDet, 5000,0,15000);
    rf_each_raw.reset("rf_each_raw", "RF timing each raw", nbDet,0,nbDet, 2000, -1200, 800);

    energy_all_Ge.reset("Ge_spectra", "Ge spectra", 20000,0,10000);
    rf_all.reset("RF_Time_spectra", "RF Time spectra", 2000, -1200, 800);
    energy_each.reset("Energy_spectra_each", "Energy spectra each", nbDet,0,nbDet, 5000,0,15000);
    rf_each.reset("RF_timing_each", "RF timing each", nbDet,0,nbDet, 2000, -1200, 800);

    energy_all_Ge_event.reset("Ge_spectra_event", "Ge spectra after event building", 20000,0,10000);
    rf_all_event.reset("Time_spectra_event", "Time spectra after event building", 2000, -1200, 800);
    energy_each_event.reset("Energy_spectra_each_event", "Energy spectra each after event building", nbDet,0,nbDet, 5000,0,15000);
    rf_each_event.reset("RF_timing_each_event", "RF timing each after event building", nbDet,0,nbDet, 2000,-1200,800);

    energy_all_Ge_trig.reset("Ge_spectra_trig", "Ge spectra after trigger", 20000,0,10000);
    rf_all_trig.reset("Time_spectra_trig", "Time spectra after trigger", 2400, -1200, 1200);
    energy_each_trig.reset("Energy_spectra_each_trig", "Energy spectra each after trigger", nbDet,0,nbDet, 5000,0,15000);
    rf_each_trig.reset("RF_timing_each_trig", "RF timing each after trigger", nbDet,0,nbDet, 2000,-1200,800);
  }
};

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, 
              Detectors const & detectors, 
              Calibration const & calibration, 
              Timeshifts const & timeshifts, 
              Path const & outPath, 
              Histos & histos,
              MTCounter & total_read_size)
{
  Timer timer;
  // Checking the lookup tables :
  if (!detectors || !timeshifts || !calibration || !reader) return;
  reader.setMaxHits(max_hits);

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  Filename filename = raw_datafile.filename();// "                               run_number_filenumber.fast"
  int filenumber = std::stoi(lastPart(filename.shortName(), '_'));
  std::string run_path = raw_datafile.path(); // "/path/to/manip/run_number.fast/"
  std::string temp = run_path;                // "/path/to/manip/run_number.fast/"
  temp.pop_back();                            // "/path/to/manip/run_number.fast"
  std::string run = rmPathAndExt(temp);       //                "run_number"
  // int run_number = std::stoi(lastPart(run,'_'));//                   number

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
  Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
  File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  total_read_size+=raw_datafile.size();
  auto dataSize = float_cast(raw_datafile.size("Mo"));

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
  tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  hit.writting(tempTree.get(), "lsEQp");

  // Loop over the TTree 
  Timer read_timer;
  ulong rawCounts = 0;
  while(reader.Read())
  {
    // Time calibration :
    hit.stamp+=timeshifts[hit.label];

    // Energy calibration : 
    hit.nrj = calibration(hit.adc,  hit.label); // Normal calibration
    hit.nrj2 = ((hit.qdc2 == 0) ? 0.f : calibration(hit.qdc2, hit.label)); // Calibrate the qdc2 if any
  
    if (isGe[hit.label] && hit.nrj>10000) continue;

    tempTree -> Fill();
    rawCounts++;
  }

  read_timer.Stop();
  
  print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

  if (rawCounts==0) {print("NO HITS IN",run); return;}

  // Realign switched hits after timeshifts :
  Alignator gindex(tempTree.get());

  // Switch the temporary TTree to reading mode :
  hit.reset();
  hit.reading(tempTree.get(), "lsEQp");

  unique_tree outTree (new TTree("Nuball2","Nuball2"));
  outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  Event event(outTree.get(), "lstEQp", "w");

  // Initialize event builder based on RF :
  RF_Manager rf;
  EventBuilder_136 eventBuilder(&event, &rf);
  eventBuilder.setNbPeriodsMore(nb_periods_more);

  // Handle the first RF downscale :
  RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
  if (!first_rf) return;
  eventBuilder.setFirstRF(hit);

  debug("first RF found at hit n°", first_rf.cursor());

  // Initialize event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Handle the first hit :
  int loop = 0;
  // The first few first hundreds of thousands of hits, in the first file of each run,
  // don't have RF downscale. So we have to skip them in order to maintain good temporal resolution :
  if (filenumber == 1) loop = first_rf.cursor(); 
  tempTree -> GetEntry(gindex[loop++]);
  eventBuilder.set_first_hit(hit);

  //Loop over the data :
  Timer convert_timer;
  auto const & nb_data = tempTree->GetEntries();
  ulong hits_count = 0;
  ulong evts_count = 0;
  ulong trig_count = 0;
  while (loop<nb_data)
  {
    tempTree -> GetEntry(gindex[loop++]);
    
    // Handle the RF data :
    if (hit.label == RF_Manager::label)
    {
      rf.setHit(hit);
      Event temp (event);
      event = hit;
      outTree -> Fill();
      event = temp;
      continue;
    }

    if (histoed)
    {
      auto const tof_hit = rf.pulse_ToF_ns(hit.stamp);
      histos.rf_all.Fill(tof_hit);
      histos.rf_each.Fill(compressedLabel[hit.label], tof_hit);
      
      if (isGe[hit.label]) histos.energy_all_Ge.Fill(hit.nrj);
      histos.energy_each.Fill(compressedLabel[hit.label], hit.nrj);
    }
    // print_precision(13);
    // Event building :
    if (eventBuilder.build(hit))
    {
      evts_count++;
      counter.count(event); 
      if ((evts_count%(int)(1.E+6)) == 0) debug(evts_count/(int)(1.E+6), "Mevts");
      if (histoed)
      {
        auto const pulse_ref = rf.pulse_ToF_ns(event.stamp);
        for (size_t trig_loop = 0; trig_loop<event.size(); trig_loop++)
        {
          auto const & label  = event.labels[trig_loop];
          auto const & time   = event.times [trig_loop];
          auto const tof_trig = pulse_ref+time/1000ll;
          auto const & nrj    = calibration(event.nrjs[trig_loop], label);

          histos.rf_all_event.Fill(tof_trig);
          histos.rf_each_event.Fill(compressedLabel[label], tof_trig);
      
          if (isGe[label]) histos.energy_all_Ge_event.Fill(nrj+gRandom->Uniform(0,1));
          histos.energy_each_event.Fill(compressedLabel[label], nrj);
        }
      }
    #ifdef TRIGGER
      if (trigger(counter))
      {
        
    #ifdef PREPROMPT
        eventBuilder.tryAddPreprompt_simple();
    #endif //PREPROMPT

        if (nb_periods_more>0) eventBuilder.tryAddNextHit_simple(tempTree.get(), hit, loop, gindex);

        hits_count+=event.size();
        trig_count++;
        outTree->Fill();

        if (histoed)
        {
          auto const pulse_ref = rf.pulse_ToF_ns(event.stamp);
          for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
          {
            auto const & label  = event.labels[trig_loop];
            auto const & nrj    = event.nrjs  [trig_loop];
            auto const & time   = event.times [trig_loop];
            auto const tof_trig = pulse_ref+time/1000ll;

            histos.rf_all_trig.Fill(tof_trig);
            histos.rf_each_trig.Fill(compressedLabel[label], tof_trig);
        
            if (isGe[label]) histos.energy_all_Ge_trig.Fill(nrj);
            histos.energy_each_trig.Fill(compressedLabel[label], nrj);
          }
        }
      }
    #else
      outTree->Fill();
    #endif
    }
  }
  convert_timer.Stop();
  print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s (",dataSize/convert_timer.TimeElapsedSec() ,"Mo/s)");
  Timer write_timer;

  // Initialize output TTree :
  unique_TFile outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  write_timer.Stop();

  auto outSize  = float_cast(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

  print_precision(4);
  print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
        "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.*double_cast(hits_count))/double_cast(rawCounts),"% hits kept");
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
      else if (command == "-H" || command == "--histograms")
      {
        histoed = true;
      }
      else if (command == "-m" || command == "--multithread")
      {// Multithreading : number of threads
        nb_threads = atoi(argv[++i]);
      }
      else if (command == "-n" || command == "--number-hits")
      {// Number of hits per file
        max_hits = atoi(argv[++i]);
      }
      else if (command == "-o" || command == "--overwrite")
      {// Overwright already existing .root files
        overwrite = true;
      }
      else if (command == "-O" || command == "--output")
      {
        output = argv[++i];
      }
      else if (command == "-t" || command == "--timeshifts")
      {
        only_timeshifts = true;
      }
      else if (command == "-Th" || command == "--Thorium")
      {
        list_runs = "Thorium.list";
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
      }
      else if (command == "-h" || command == "--help")
      {
        print("List of the commands :");
        print("(-f  || --files-number) [files-number]  : set the number of files");
        print("(       --run)          [runname]       : set only one folder to convert");
        print("(-h  || --help)                         : display this help");
        print("(-H  || --histograms)                   : Fills and writes raw histograms");
        print("(-m  || --multithread)  [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
        print("(-n  || --number-hits)  [hits_number]   : set the number of hit to read in each file.");
        print("(-o  || --overwrite)                    : overwrites the already written folders. If a folder is incomplete, you need to delete it");
        print("(-t  || --timeshifts)                   : Calculate only timeshifts, force it even if it already has been calculated");
        print("(-Th || --Thorium)                      : Treats only the thorium runs (run_nb < 75)");
        print("(-U  || --Uranium)                      : Treats only the uranium runs (run_nb >= 75)");
        print("(       --129)                          : Treats the N-SI-129 pulsed runs");
        return 0;
      }
    }
  }

  // MANDATORY : initialize the multithreading !
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(nb_files);
  MTObject::Initialize();

  // Setup the path accordingly to the machine :
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp ^^^^^^^^"); return -1;}

  Path manipPath = datapath+manip;
  Path outPath (datapath+(manip.name()+output), true);

  // Load some modules :
  Detectors detectors(IDFile);
  Calibration calibration(detectors, calibFile);
  Manip runs(File(list_runs).string());
  if (one_run) runs.setFolder(one_run_folder);

  // Checking of all the modules have been loaded correctly :
  if (!detectors || !calibration || !runs) return -1;

  // Setup some parameters :
  RF_Manager::set_offset_ns(rf_shift);

  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    MTCounter total_read_size;
    Timer timer;
    Path runpath = manipPath+run;
    auto run_name = removeExtension(run);

    Histos histos;
    if (histoed) histos.Initialize(detectors);

    print("----------------");
    print("Treating ", run_name);

    // Timeshifts loading : 
    Timeshifts timeshifts(outPath, run_name);

    // If no timeshifts data already available, calculate it :
    if (!timeshifts || (only_timeshifts && overwrite)) 
    { 
      timeshifts.setDetectors(detectors);
      timeshifts.setMult(2,3);
      timeshifts.setOutDir(outPath);

      timeshifts.calculate(runpath, nb_files_ts);
      timeshifts.verify(runpath, 10);

      timeshifts.write(run_name);
    }

    if (!only_timeshifts)
    {
      // Loop over the files in parallel :
      MTFasterReader readerMT(runpath, nb_files);
      readerMT.execute(convert, detectors, calibration, timeshifts, outPath, histos, total_read_size);

      if (histoed)
      {
        auto const name = outPath+run_name+"/histo_"+run_name+".root";
        std::unique_ptr<TFile> outFile (TFile::Open(name.c_str(), "RECREATE"));
        outFile -> cd();
        histos.energy_all_Ge_raw.Write();
        histos.rf_all_raw.Write();
        histos.energy_each_raw.Write();
        histos.rf_each_raw.Write();
        histos.energy_all_Ge.Write();
        histos.energy_each.Write();
        histos.rf_all.Write();
        histos.rf_each.Write();
        histos.energy_all_Ge_event.Write();
        histos.rf_all_event.Write();
        histos.energy_each_event.Write();
        histos.rf_each_event.Write();
        histos.energy_all_Ge_trig.Write();
        histos.energy_each_trig.Write();
        histos.rf_all_trig.Write();
        histos.rf_each_trig.Write();


        outFile -> Write();
        outFile -> Close();
        print(name, "written");
      }
    }
    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer.TimeSec());
  }
  return 1;
}
