// // 1. Parameters
//   // RF : 
#define USE_RF 200 //ns
//   // Detectors :
// #define USE_DSSD
// #define USE_PARIS
//   // Triggers :
// #define TRIGGER
// // #define KEEP_ALL

#include <Convertor.hpp>
#include "EventBuilder_136.hpp"

struct Histos
{
  // Vector_MTTHist<TH1F> rf;
  MTTHist<TH1F> energy_all;
  MTTHist<TH1F> rf_all;

  MTTHist<TH2F> energy_each;
  MTTHist<TH2F> rf_each;

  MTTHist<TH1F> energy_all_event;
  MTTHist<TH1F> rf_all_event;

  MTTHist<TH2F> energy_each_event;
  MTTHist<TH2F> rf_each_event;

  MTTHist<TH1F> energy_all_trig;
  MTTHist<TH1F> rf_all_trig;

  MTTHist<TH2F> energy_each_trig;
  MTTHist<TH2F> rf_each_trig;

  void Initialize(Detectors const & detectors)
  {
    auto const & nbDet = detectors.number();

    energy_all.reset("Ge_spectra", "Ge spectra", 8000,0,4000);
    rf_all.reset("RF_Time_spectra", "RF Time spectra", USE_RF*2, -USE_RF/4, 3*USE_RF/4);

    energy_each.reset("Energy_spectra_each", "Energy spectra each", nbDet,0,nbDet, 5000,0,15000);
    rf_each.reset("RF_timing_each", "RF timing each", nbDet,0,nbDet, USE_RF*2, -USE_RF/4, 3*USE_RF/4);

    energy_all_event.reset("Ge_spectra_event", "Ge spectra after event building", 8000,0,4000);
    rf_all_event.reset("Time_spectra_event", "Time spectra after event building", 1000, -100, 400);

    energy_each_event.reset("Energy_spectra_each_event", "Energy spectra each after event building", nbDet,0,nbDet, 5000,0,15000);
    rf_each_event.reset("RF_timing_each_event", "RF timing each after event building", nbDet,0,nbDet, USE_RF*2,-USE_RF/4,3*USE_RF/4);

    energy_all_trig.reset("Ge_spectra_trig", "Ge spectra after trigger", 8000,0,4000);
    rf_all_trig.reset("Time_spectra_trig", "Time spectra after trigger", 1000, -100, 400);

    energy_each_trig.reset("Energy_spectra_each_trig", "Energy spectra each after trigger", nbDet,0,nbDet, 5000,0,15000);
    rf_each_trig.reset("RF_timing_each_trig", "RF timing each after trigger", nbDet,0,nbDet, USE_RF*2,-USE_RF/4,3*USE_RF/4);
  }
};

class Convertor136 : public Convertor
{
  // General wrapper class
template <class... ARGS>
  Convertor136(ARGS &&... args) : Convertor(std::forward<ARGS>(args)...) 
  {

  }

  // Terminal parameter mode
  Convertor136(int argc, char** argv) : Convertor(argc, argv) 
  {
    for (int i = 3; i<argc; i++)
    {
      std::string command = argv[i];
           if (command == "-H") m_histoed = true;
      else if (command == "-o") m_overwrite = true;
    }
  }

  void convertFile(Hit & hit, FasterReader & reader, Path const & outPath) override;
  void printParameters() const override;

private:
  bool m_histoed = false;
  bool m_overwrite = false;
  Histos histo;
};

void Convertor136::printParameters() const
{
  Convertor::printParameters();
  print("For Convertor136 :");
  print("-H : Write down histograms");
}

void Convertor136::convertFile(Hit & hit, FasterReader & reader, Path const & outPath) 
{
  Timer timer;
  // Checking the lookup tables :
  if (!m_detectors || !m_timeshifts || !m_calibration || !reader) return;

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
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
  if ( !m_overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
  tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  hit.writting(tempTree.get(), "lsEQp");

  // Loop over the TTree 
  Timer read_timer;
  ulong rawCounts = 0;
  print(MTObject::getThreadIndex());
  while(reader.Read())
  {
    // Time calibration :
    hit.stamp+=m_timeshifts[hit.label];

    // Energy calibration :
    hit.nrj = m_calibration(hit.adc,  hit.label); // Normal calibration
    hit.nrj2 = ((hit.nrj2 == 0) ? 0.f : m_calibration(hit.qdc2, hit.label)); // Calibrate the nrj2 if any

    tempTree -> Fill();

    rawCounts++;
  }
  read_timer.Stop();

#ifdef DEBUG
  print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s");
#endif //DEBUG

  if (rawCounts==0) {print("NO HITS IN",run); return;}

  // Realign switched hits after timeshifts :
  Alignator gindex(tempTree.get());

  // Switch the temporary TTree to reading mode :
  hit.reset();
  hit.reading(tempTree.get(), "lsEQp");

  TTree* outTree = new TTree("Nuball2","Nuball2");
  outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  Event event(outTree, "lstEQp", "w");

  // Initialize event builder based on RF :
  RF_Manager rf;
  EventBuilder_136 eventBuilder(&event, &rf);

  // Initialize event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Handle the first RF downscale :
  RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
  if (!first_rf) return;
  eventBuilder.setFirstRF(hit);

  // Handle the first hit :
  int loop = 0;
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
      Event temp (event);
      event = hit;
      outTree -> Fill();
      event = temp;
      rf.setHit(hit);
      continue;
    }

    if (m_histoed)
    {
      auto const tof_hit = rf.pulse_ToF_ns(hit.time);
      histos.rf_all.Fill(tof_hit);
      histos.rf_each.Fill(compressedLabel[hit.label], tof_hit);
      
      if (isGe[hit.label]) histos.energy_all.Fill(hit.nrjcal);
      histos.energy_each.Fill(compressedLabel[hit.label], hit.nrjcal);
    }

    // Event building :
    if (eventBuilder.build(hit))
    {
      evts_count++;
      if (m_histoed)
      {
        for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
        {
          auto const & label = event.labels[trig_loop];
          auto const & nrjcal = event.nrjcals[trig_loop];
          auto const & time = event.times[trig_loop];
          auto const tof_trig = rf.pulse_ToF_ns(time);

          histos.rf_all_event.Fill(tof_trig);
          histos.rf_each_event.Fill(compressedLabel[label], tof_trig);
      
          if (isGe[label]) histos.energy_all_event.Fill(nrjcal);
          histos.energy_each_event.Fill(compressedLabel[label], nrjcal);
        }
      }
      counter.count(event); 

    #ifdef TRIGGER
      if (trigger(counter))
      {
        hits_count+=event.size();
        
        trig_count++;

        if (m_histoed)
        {
          for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
          {
            auto const & label = event.labels[trig_loop];
            auto const & nrjcal = event.nrjcals[trig_loop];
            auto const & time = event.times[trig_loop];
            auto const tof_trig = rf.pulse_ToF_ns(time);

            histos.rf_all_trig.Fill(tof_trig);
            histos.rf_each_trig.Fill(compressedLabel[label], tof_trig);
        
            if (isGe[label]) histos.energy_all_trig.Fill(nrjcal);
            histos.energy_each_trig.Fill(compressedLabel[label], nrjcal);
          }
        }

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

  // Initialize output TTree :
  std::unique_ptr<TFile> outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  write_timer.Stop();

  auto dataSize = static_cast<float>(raw_datafile.size("Mo"));
  auto outSize  = static_cast<float>(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

  print_precision(4);
  print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
        "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.*double_cast(hits_count))/double_cast(rawCounts),"% hits kept");

}

int main(int argc, char** argv)
{
  MySimpleConvertor(argc, argv);
  return 1;
}

/*
// // 2. Include library
// #include <libCo.hpp>
// #include <FasterReader.hpp>   // This class is the base for mono  threaded code
// #include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
// #include <MTFasterReader.hpp> // This class is the base for multi threaded code
// #include <MTCounter.hpp>      // Use this to thread safely count what you want²
// #include <MTTHist.hpp>        // Use this to thread safely fill histograms
// #include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
// #include <Calibration.hpp>    // Either loads or calculate calibration coefficients
// #include <Detectors.hpp>      // Eases the manipulation of detector's labels
// #include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
// #include <RF_Manager.hpp>     // Eases manipulation of RF information


// #include "EventBuilder_136.hpp" // Event builder for this experiment

// // 3. Declare some global variables :
// std::string IDFile = "index_129.list";
// std::string calibFile = "136.calib";
// Folder manip = "N-SI-136";
// std::string list_runs = "list_runs.list";
// int  nb_files_ts = 50;
// int nb_files = -1;
// bool only_timeshifts = false; // No conversion : only calculate the timeshifts
// bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
// bool histoed = false;
// bool one_run = false;
// std::string one_run_folder = "";

// bool extend_periods = false; // To take more than one period after a event trigger
// uint nb_periods_more = 0; // Number of periods to extend after an event that triggered

// bool trigger(Counter136 const & counter)
// {
//   return (counter.nb_dssd>0);
//   // return ((counter.nb_modules>2 && counter.nb_clovers>0) || counter.nb_dssd>0);
// }

// 

// // 4. Declare the function to run on each file in parallel :
// void convert(Hit & hit, FasterReader & reader, 
//               Detectors const & detectors, 
//               Calibration const & calibration, 
//               Timeshifts const & timeshifts, 
//               Path const & outPath, 
//               Histos & histos)
// {
//   Timer timer;
//   // Checking the lookup tables :
//   if (!detectors || !timeshifts || !calibration || !reader) return;

//   // Extracting the run name :
//   File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
//   std::string run_path = raw_datafile.path(); // "/path/to/manip/run_number.fast/"
//   std::string temp = run_path;                // "/path/to/manip/run_number.fast/"
//   temp.pop_back();                            // "/path/to/manip/run_number.fast"
//   std::string run = rmPathAndExt(temp);       //                "run_number"
//   // int run_number = std::stoi(lastPart(run,'_'));//                   number

//   // Setting the name of the output file :
//   Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
//   Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
//   File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

//   // Important : if the output file already exists, then do not overwrite it !
//   if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

//   // Initialize the temporary TTree :
//   std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
//   tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
//   hit.writting(tempTree.get(), "ltEQp");

//   // Loop over the TTree 
//   Timer read_timer;
//   ulong rawCounts = 0;
//   print(MTObject::getThreadIndex());
//   while(reader.Read())
//   {
//     // Time calibration :
//     hit.time+=timeshifts[hit.label];

//     // Energy calibration :
//     hit.nrjcal = calibration(hit.nrj,  hit.label); // Normal calibraiton
//     hit.nrj2cal = ((hit.nrj2 == 0) ? 0.f : calibration(hit.nrj2, hit.label)); // Calibrate the nrj2 if any
//     if (isBGO[hit.label]) hit.nrjcal = NRJ_cast(hit.nrj); // "Proto calibration" of BGO

//     tempTree -> Fill();

//     rawCounts++;
//   }
//   read_timer.Stop();

// #ifdef DEBUG
//   print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s");
// #endif //DEBUG

//   if (rawCounts==0) {print("NO HITS IN",run); return;}

//   // Realign switched hits after timeshifts :
//   Alignator gindex(tempTree.get());

//   // Switch the temporary TTree to reading mode :
//   hit.reset();
//   hit.reading(tempTree.get(), "ltEQp");

//   TTree* outTree = new TTree("Nuball2","Nuball2");
//   outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
//   Event event(outTree, "ltEQp", "w");

//   // Initialize event builder based on RF :
//   RF_Manager rf;
//   EventBuilder_136 eventBuilder(&event, &rf);

//   // Initialize event analyser : simple modules and DSSD counter
//   Counter136 counter;

//   // Handle the first RF downscale :
//   RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
//   if (!first_rf) return;
//   eventBuilder.setFirstRF(hit);

//   // Handle the first hit :
//   int loop = 0;
//   tempTree -> GetEntry(gindex[loop++]);
//   eventBuilder.set_first_hit(hit);

//   //Loop over the data :
//   Timer convert_timer;
//   auto const & nb_data = tempTree->GetEntries();
//   ulong hits_count = 0;
//   ulong evts_count = 0;
//   ulong trig_count = 0;
//   while (loop<nb_data)
//   {
//     tempTree -> GetEntry(gindex[loop++]);

//     // Handle the RF data :
//     if (hit.label == RF_Manager::label)
//     {
//       Event temp (event);
//       event = hit;
//       outTree -> Fill();
//       event = temp;
//       rf.setHit(hit);
//       continue;
//     }

//     if (histoed)
//     {
//       auto const tof_hit = rf.pulse_ToF_ns(hit.time);
//       histos.rf_all.Fill(tof_hit);
//       histos.rf_each.Fill(compressedLabel[hit.label], tof_hit);
      
//       if (isGe[hit.label]) histos.energy_all.Fill(hit.nrjcal);
//       histos.energy_each.Fill(compressedLabel[hit.label], hit.nrjcal);
//     }

//     // Event building :
//     if (eventBuilder.build(hit))
//     {
//       evts_count++;
//       if (histoed)
//       {
//         for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
//         {
//           auto const & label = event.labels[trig_loop];
//           auto const & nrjcal = event.nrjcals[trig_loop];
//           auto const & time = event.times[trig_loop];
//           auto const tof_trig = rf.pulse_ToF_ns(time);

//           histos.rf_all_event.Fill(tof_trig);
//           histos.rf_each_event.Fill(compressedLabel[label], tof_trig);
      
//           if (isGe[label]) histos.energy_all_event.Fill(nrjcal);
//           histos.energy_each_event.Fill(compressedLabel[label], nrjcal);
//         }
//       }
//       counter.count(event); 

//     #ifdef TRIGGER
//       if (trigger(counter))
//       {
//         hits_count+=event.size();
        
//         trig_count++;

//         if (histoed)
//         {
//           for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
//           {
//             auto const & label = event.labels[trig_loop];
//             auto const & nrjcal = event.nrjcals[trig_loop];
//             auto const & time = event.times[trig_loop];
//             auto const tof_trig = rf.pulse_ToF_ns(time);

//             histos.rf_all_trig.Fill(tof_trig);
//             histos.rf_each_trig.Fill(compressedLabel[label], tof_trig);
        
//             if (isGe[label]) histos.energy_all_trig.Fill(nrjcal);
//             histos.energy_each_trig.Fill(compressedLabel[label], nrjcal);
//           }
//         }

//         outTree->Fill();
//       }
//     #else
//       outTree->Fill();
//     #endif
//     }
//   #ifdef KEEP_ALL
//     if (eventBuilder.isSingle())
//     {
//       auto temp = event;
//       event = eventBuilder.singleHit();
//       outTree -> Fill();
//       event = temp;
//       continue;
//     }
//   #endif
//   }
//   convert_timer.Stop();
// #ifdef DEBUG
//   print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s");
// #endif //DEBUG
//   Timer write_timer;

//   // Initialize output TTree :
//   std::unique_ptr<TFile> outFile (TFile::Open(outfile.c_str(), "RECREATE"));
//   outFile -> cd();
//   outTree -> Write();
//   outFile -> Write();
//   outFile -> Close();

//   write_timer.Stop();

//   auto dataSize = static_cast<float>(raw_datafile.size("Mo"));
//   auto outSize  = static_cast<float>(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

//   print_precision(4);
//   print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
//         "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.*double_cast(hits_count))/double_cast(rawCounts),"% hits kept");
// }

// // 5. Main
// int main(int argc, char** argv)
// {
//   int nb_threads = 2;
//   if (argc > 1)
//   {

//     for(int i = 1; i < argc; i++)
//     {
//       std::string command = argv[i];
//            if (command == "-e" || command == "--extend-period")
//       {// To get more than 1 period after pulse if trigger activated 
//         extend_periods = true;
//         nb_periods_more = atoi(argv[++i]);
//       }
//       else if (command == "-f" || command == "--files-number")
//       {
//         nb_files = atoi(argv[++i]);
//       }
//       else if (command == "--file")
//       {
//         one_run = true;
//         one_run_folder = argv[++i];
//       }
//       else if (command == "-H" || command == "--histograms")
//       {
//         histoed = true;
//       }
//       else if (command == "-m" || command == "--multithread")
//       {// Multithreading : number of threads
//         nb_threads = atoi(argv[++i]);
//       }
//       else if (command == "-o" || command == "--overwrite")
//       {// Overwright already existing .root files
//         overwrite = true;
//       }
//       else if (command == "-t" || command == "--timeshifts")
//       {
//         only_timeshifts = true;
//       }
//       else if (command == "-Th" || command == "--Thorium")
//       {
//         list_runs = "list_Th.list";
//       }
//       else if (command == "-U" || command == "--Uranium")
//       {
//         list_runs = "list_U.list";
//       }
//       else if (command == "-h" || command == "--help")
//       {
//         print("List of the commands :");
//         print("(-f  || --files_number) [files_number]  : set the number of files");
//         print("(-h  || --help)                         : display this help");
//         print("(-H  || --histograms)                   : Fills and writes raw histograms");
//         print("(-m  || --multithread)  [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
//         print("(-o  || --overwrite)                    : overwrites the already written folders. If a folder is incomplete, you need to delete it");
//         print("(-t  || --timeshifts)                   : Calculate only timeshifts, force it even if it already has been calculated");
//         print("(-Th || --Thorium)                      : Treats only the thorium runs (run_nb < 75)");
//         print("(-U  || --Uranium)                      : Treats only the uranium runs (run_nb >= 75)");
//         return 0;
//       }
//     }
//   }

//   // MANDATORY : initialize the multithreading !
//   if (nb_threads>1) MTObject::Initialize(nb_threads);
//   print(MTObject::getThreadIndex());

//   // Setup the path accordingly to the machine :
//   Path datapath = Path::home();
//        if ( datapath == "/home/corentin/") datapath+="faster_data/";
//   else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
//   else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp ^^^^^^^^"); return -1;}

//   Path manipPath = datapath+manip;
//   Path outPath (datapath+(manip.name()+"-root_test2"), true);

//   // Load some modules :
//   Detectors detectors(IDFile);
//   Calibration calibration(detectors, calibFile);
//   // Manip runs(File(manipPath+list_runs));

//   // Checking of all the modules have been loaded correctly :
//   // if (!detectors || !calibration || !runs) return -1;

//   // Setup some parameters :
//   RF_Manager::set_offset(40000);

//   // Loop sequentially through the runs and treat their files in parallel :
//   std::string run = "run_75.fast";
//   // std::string run;
//   // while(runs.getNext(run))
//   // {
//     Path runpath = manipPath+run;
//     auto run_name = removeExtension(run);

//     Histos histos;
//     if (histoed) histos.Initialize(detectors);

//     print("----------------");
//     print("Treating ", run_name);

//     // Timeshifts loading : 
//     Timeshifts timeshifts(outPath, run_name);

//     // If no timeshifts data already available, calculate it :
//     if (!timeshifts || (only_timeshifts && overwrite)) 
//     { 
//       timeshifts.setDetectors(detectors);
//       timeshifts.setMult(2,3);
//       timeshifts.setOutDir(outPath);

//       timeshifts.calculate(runpath, nb_files_ts);
//       timeshifts.verify(runpath, 10);

//       timeshifts.write(run_name);
//     }

//     if (!only_timeshifts)
//     {
//       // Loop over the files in parallel :
//       MTFasterReader readerMT(runpath, nb_files);
//       readerMT.execute(convert, detectors, calibration, timeshifts, outPath, histos);

//       if (histoed)
//       {
//         std::unique_ptr<TFile> outFile (TFile::Open((outPath+run_name+"/histo_"+run_name+".root").c_str(), "RECREATE"));
//         outFile -> cd();
//         histos.energy_all.Write();
//         histos.energy_each.Write();
//         histos.rf_all.Write();
//         histos.rf_each.Write();
//         histos.energy_all_event.Write();
//         histos.rf_all_event.Write();
//         histos.energy_each_event.Write();
//         histos.rf_each_event.Write();
//         histos.energy_all_trig.Write();
//         histos.energy_each_trig.Write();
//         histos.rf_all_trig.Write();
//         histos.rf_each_trig.Write();
//         outFile -> Write();
//         outFile -> Close();
//         print(outPath+run_name+"/"+run_name+"_histo.root written");
//       }
//     }
//   // }

//   return 1;
// }
*/