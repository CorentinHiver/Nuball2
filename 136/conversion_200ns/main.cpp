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
std::string IDFile = "index_129.list";
std::string calibFile = "136.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string time_ref = "301";
std::string timewindow = "1500";
int  nb_files_ts = 20;
int nb_files = -1;
bool overwrite = false; // Overwrite already existing converted root filesstruct histos

struct Histos
{
  Vector_MTTHist<TH1F> rf;
  MTTHist<TH2F> rf_all;
  void Initialize(Detectors const & detectors)
  {
    auto const & nbDet = detectors.number();

    rf_all.reset("RF_timing", "RF_timing", nbDet,0,nbDet, 1000,-100,400);

    rf.resize(nbDet);
    for (uint label = 0; label<nbDet; label++)
    {
      if (!detectors.exists[label]) continue;
      auto const & name = detectors[label];
      rf[label].reset((name+"_RF_timing").c_str(), (name+"_RF_timing").c_str(), 1000, -100, 400);
    }
  }
};

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, 
              Detectors const & detectors, 
              Calibration const & calibration, 
              Timeshifts const & timeshifts, 
              Path const & outPath, 
              Histos & histos)
{
  Timer timer;
  // Checking the lookup tables :
  if (!detectors || !timeshifts || !calibration || !reader) return;

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

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> readTree (new TTree("temp","temp"));
  readTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  readTree -> Branch("label"  , &hit.label );
  readTree -> Branch("time"   , &hit.time  );
  readTree -> Branch("nrjcal" , &hit.nrjcal);
  readTree -> Branch("nrj2"   , &hit.nrj2  );
  readTree -> Branch("pileup" , &hit.pileup);

  // Loop over the TTree 
  Timer read_timer;
  ulong rawCounts = 0;
  int RF_counter_raw = 0;
  while(reader.Read())
  {
    hit.time+=timeshifts[hit.label];
    hit.nrjcal = calibration(hit.nrj,  hit.label);
    hit.nrj2   = calibration(hit.nrj2, hit.label);
    readTree -> Fill();
    rawCounts++;
    if (hit.label == RF_Manager::label) RF_counter_raw++;
  }
  read_timer.Stop();

#ifdef DEBUG
  print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts and", RF_counter_raw, "RF counts, in", read_timer.TimeElapsedSec(),"s");
#endif //DEBUG

if (rawCounts==0) return;

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

  // Initialize output TTree :
  std::unique_ptr<TFile> outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  TTree* outTree = new TTree("Nuball2","Nuball2");
  Event event(outTree, "ltnNp", "w");

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
  Timer convert_timer;
  auto const & nb_data = readTree->GetEntries();
  ulong hits_count = 0;
  ulong evts_count = 0;
  ulong RF_counter_written = 0;
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

    auto const tof = rf.pulse_ToF(hit.time);
    histos.rf[hit.label].Fill(tof/_ns);
    histos.rf_all.Fill(hit.label, tof/_ns);

    // Event building :
    if (eventBuilder.build(hit))
    {
      counter.count(event); 
    #ifdef TRIGGER
      if ((counter.nb_modules>1 && counter.nb_Ge>0) || counter.nb_dssd>0)
      {
        hits_count+=event.size();
        evts_count++;
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
  convert_timer.Stop();
#ifdef DEBUG
  print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s");
#endif //DEBUG
  Timer write_timer;
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  write_timer.Stop();

  auto dataSize = static_cast<int>(raw_datafile.size("Mo"));
  auto outSize  = static_cast<int>(size_file_conversion(outFile->GetSize(), "o", "Mo"));

  timer();
  print(RF_counter_raw, RF_counter_written);
  print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
        "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", 100*hits_count/rawCounts,"% hits kept");
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
           if (command == "-m" || command == "--multithread")
      {// Multithreading : number of threads
        nb_threads = atoi(argv[++i]);
      }
      else if (command == "-o" || command == "--overwrite")
      {// Overwright already existing .root files
        overwrite = true;
      }
      else if (command == "-f" || command == "--files-number")
      {
        nb_files = atoi(argv[++i]);
      }
      else if (command == "-U" || command == "--Uranium")
      {
        list_runs = "list_U.list";
      }
      else if (command == "-Th" || command == "--Thorium")
      {
        list_runs = "list_Th.list";
      }
    }
  }

  // MANDATORY : initialize the multithreading !
  MTObject::Initialize(nb_threads);

  // Setup the path accordingly to the machine :
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main ^^^^^^^^"); return -1;}

  Path manipPath = datapath+manip;
  Path outPath (datapath+(manip.name()+"-root"), true);

  // Load some modules :
  Detectors detectors(IDFile);
  Calibration calibration(detectors, calibFile);
  Manip runs(File(manipPath+list_runs));

  // Checking of all the modules have been loaded correctly :
  if (!detectors || !calibration || !runs) return -1;

  // Setup some parameters :
  RF_Manager::set_offset(30000);

  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    Path runpath = manipPath+run;
    auto run_name = removeExtension(run);

    Histos histos;
    histos.Initialize(detectors);

    print("----------------");
    print("Treating ", run_name);

    // Timeshifts loading : 
    Timeshifts timeshifts(outPath,run_name);

    // If no timeshifts data already available, calculate it :
    if (!timeshifts) 
    { 
      timeshifts.setDetectors(detectors);
      timeshifts.setMult(2,3);
      // timeshifts.setMaxHits(10000);
      timeshifts.setOutDir(outPath);
      
      timeshifts.calculate(runpath, nb_files_ts);
      timeshifts.verify(runpath, 10);

      timeshifts.write(run_name);
    }

    // Loop over the files in parallel :
    MTFasterReader readerMT(runpath, nb_files);
    readerMT.execute(convert, detectors, calibration, timeshifts, outPath, histos);

    std::unique_ptr<TFile> outFile (TFile::Open((outPath+run_name+"/histo_"+run_name+".root").c_str(), "RECREATE"));
    outFile -> cd();
    for (auto & histo : histos.rf) histo.Write();
    histos.rf_all.Write();
    outFile -> Write();
    outFile -> Close();
    print(outPath+run_name+"/"+run_name+"_histo.root written");
  }

  return 1;
}
