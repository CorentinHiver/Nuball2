
//g++ PulseConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

#define N_SI_136
// #define N_SI_129
// #define N_SI_120

#if defined(N_SI_136)
  #define USE_RF 200
#elif defined(N_SI_129) || defined(N_SI_120)
  #define USE_RF 400
#endif // MANIP

#if defined (M2G1_TRIG)
  #define COUNT_EVENT
#endif //M2G1_TRIG

#include <libCo.hpp>
#include <Event.hpp>
#include <FilesManager.hpp>
#include <MTTHist.hpp>
#include <MTList.hpp>
#include <MTCounter.hpp>
#include <Detectors.hpp>
#include <Classes/Counters.hpp>
#include <Timer.hpp>
#include <RF_Manager.hpp>
#include <Clovers.hpp>


enum trigger_modes{NO, M2G1, P, M2G1_P, DC2};
bool check_trigger(Counters & counter, Clovers & clovers, Event & event, trigger_modes trig = P)
{
  
  switch (trig)
  {
    case NO : return true;
    case M2G1 : 
      counter.countEvent(event);
      counter.clover_analyse();
      return (counter.Modules>1 && counter.RawGe>0);
    case P : 
      counter.countEvent(event);
      return (counter.DSSDMult>0);
    case M2G1_P : 
      counter.countEvent(event);
      counter.clover_analyse();
      return ((counter.Modules>1 && counter.RawGe>0) || counter.DSSDMult>0);
    case DC2 :
      clovers.SetEvent(event, 2);
      return(clovers.DelayedCleanGeMult>1);
      
    default : return false;
  }
}

trigger_modes trigger_mode = P;

Path dataPath;
size_t nb_threads = 2;
int nb_max_evts_in_file = (int)(5.E+6); // 5 millions evts/fichier
int nb_max_evts_read = -1; // 5 millions evts/fichier
bool calib_BGO = false;
bool histoed = false;
bool overwrites = false;

#ifdef N_SI_136
  Path outDir (Path::pwd()+"../136/conversion/", 1);
  File fileID ("index_129.dat");
  File runs_list = Path::pwd()+"../Parameters/runs_pulsed_Corentin.list";
#endif //N_SI_136


// Forward declaration
void convertRuns(MTList & runs);

int main(int argc, char ** argv)
{
  if (!fileID) return -1;

  if (!USE_RF)
  {
    print("The goal of this code is to make more convinient root files only for pulsed beam...");
    return -1;
  }

  std::vector<std::string> runs;

  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string command = argv[i];
           if (command == "-m" || command == "--multithread")
      {// Multithreading 
        nb_threads = atoi(argv[++i]);
      }
      else if (command == "-f" || command == "--folder")
      {// To convert only one folder
        runs.resize(1);
        runs[0] = (argv[++i]);
      }
      else if (command == "-F" || command == "--folders")
      {// list of the folders to convert
        int nb = atoi(argv[++i]);
        runs.resize(nb);
        for (int folder = 0; folder<nb; folder++) runs[folder] = std::string(argv[++i]);
      }
      else if (command == "-d" || command == "--data_path")
      {// Path to the data
        dataPath = argv[++i];
      }
      else if (command == "-O" || command == "--out_path")
      {// Relative path to output 
        outDir = Path(std::string(argv[++i]), 1);
      }
      else if (command == "-o" || command == "--overwrites")
      {
        overwrites = true;
      }
      else if (command == "-H" || command == "--histograms")
      {// Enables histograms
        histoed = true;
      }
      else if (command == "-n" || command == "--nb_hits_input")
      {
        nb_max_evts_read = int_cast(std::stod(argv[++i])); // stod allows scientific format (e.g. 1.e6)
      }
      else if (command == "-N" || command == "--nb_hits_output")
      {// number of hits in one output file
        nb_max_evts_in_file = int_cast(std::stod(argv[++i]));// stod allows scientific format (e.g. 1.e6)
      }
      else if (command == "-l" || command == "--list")
      { // .list file containing the list of folders to convert
        runs_list = argv[++i];
      }
      else if (command == "-i" || command == "--index")
      {// ID file to load :
        fileID = std::string(argv[++i]);
      }
      else if (command == "--BGO_cal")
      {// "Gain match" BGO is not already done
        calib_BGO = true;
      }
      else if (command == "-s" || command == "--shift_rf")
      {
        RF_Manager::set_offset(std::stoi(argv[++i]));
      }
      else if (command == "-t" || command == "--trigger")
      {
        trigger_mode = static_cast<trigger_modes>(std::stoi(argv[++i]));
      }
      else if (command == "-h" || command == "--help")
      {
        print("List commands :");
        print("(      --BGO_cal)                                        : Roughly, for BGO, Energy[keV] = ADC/100. Use this command if the BGO data are still in ADC");
        print("(-d || --data_path)     [/path/to/data/]                 : Path to the data (=experiment path)");
        print("(-f || --folder)        [folder/]                        : Name of the folder (=run) to convert");
        print("(-F || --folders)       [nb_folders] [[list of folders]] : List of folders (=runs) to convert");
        print("(-h || --help)                                           : Displays this help");
        print("(-H || --histograms)                                     : Fills and writes histograms"); 
        print("(-i || --ID)            [/path/to/ID.dat]                : Index file");
        print("(-l || --list)          [/path/to/*.list]                : Read a .list file with a list of folders to convert");
        print("(-m || --multithread)   [nb_threads]                     : Number of threads to be used");
        print("(-n || --nb-hits-input) [nb_hits_in_intput]              : Number of hits to read in each input file (scientific notation accepted (e.g. 1.e+6)");
        print("(-N || --nb-hits-ouput) [nb_hits_in_output]              : Number of hits to writes in each output file (scientific notation accepted (e.g. 1.e+6))");
        print("(-O || --out_path)      [/path/to/output/]               : Path to the output");
        print("(-o || --overwrite)                                      : Overwrites the output folders");
        print("(-t || --trigger)       [0;3]                            : 0 : no trigger, 1 : M2G1, 2: P, 3 : M2G1_P");
        return 1;
      }
    }
  }

  RF_Manager::set_offset_ns(40);

  detectors.load(fileID);
  if (runs.size() == 0) runs = listFileReader(runs_list);

  MTList runsMT(runs);

  debug(runsMT);

  if (runsMT.size() == 0)
  {
    print("NO DATA IN", runs_list);
    return -1;
  }

  else if(runsMT.size() == 1 || nb_threads == 1) convertRuns(runsMT);

  else
  {
    MTObject::setThreadsNb(nb_threads);
    MTObject::adjustThreadsNumber(runsMT.size(), "number of runs too small");
    MTObject::Initialize();
    MTObject::parallelise_function(convertRuns, runsMT);
  }
  print("Conversion done");
  return 1;
}

bool find_first_RF(TTree & tree, Event & event, RF_Manager & rf, int const & maximum_RF_location = int_cast(1.E+7))
{
    // Get first RF of file :
    bool stop = false;
    Long64_t evt = 0;
    while(!stop && evt < maximum_RF_location)
    {
      tree.GetEntry(evt++);
      for (size_t i = 0; i<event.size(); i++)
      {
        if (event.labels[i] == 251)
        {
          rf.last_hit = event.times[i];
          rf.period = event.nrjs[i];
          stop = true;
          break;
        }
      }
    }
    if (evt == maximum_RF_location) {print("NO RF found !!"); return false;}
#ifdef DEBUG
    print("RF extracted at hit n°", evt);
#endif //DEBUG
  return true;
}

void convertRuns(MTList & runs)
{
  TRandom rand(time(0));

  std::string pathRun_str = "";
  while(runs.getNext(pathRun_str))
  {
    // ---------------------- //
    // Convert a whole folder //
    // ---------------------- //
    Timer timer;

    Path pathRun = dataPath+pathRun_str;
    std::string run = pathRun.folder().name();

    // If the output folder already exists (empty or not), then overwrites only if parameter "-O" has been chosen :
    if (!overwrites && folder_exists(outDir+run)) {print(outDir+run,"already exist !!"); continue;}

    printMT("Converting", run);

    if (!pathRun) {printMT(pathRun, "doesn't exists !"); return;}

  #ifdef DEBUG
    printMT("starting");
  #endif //DEBUG

    // Load the chain :
    TChain chain("Nuball2");
    auto nb_files = chain.Add((pathRun.string()+"run_*.root").c_str());

    if(nb_files == 0) {print("NO FILES IN MATCHING", pathRun.string()+"run_*.root", "READING NEXT FOLDER"); continue;}

    // Create the event reader :
    Event event;
    event.reading(&chain);
    // print(pathRun_str);
    // chain.Print();
    // event.reading(&chain,"lstEQp");

  #ifdef DEBUG
    printMT("Chain loaded");
  #endif //DEBUG

    RF_Manager rf;
    RF_Extractor first_rf(&chain, rf, event);
    if (!first_rf) {print("Next run"); continue;}
    debug("First RF found at", first_rf.cursor(), "and period is", rf.period);

    // Counters :
    ulonglong converted_counter = 0;
    Counters counter;
    Clovers clovers;


    // Create the output folder :
    Path outPath(outDir+run,1);

    Long64_t evt = 0; // Event number
    int file_nb = 0; // File number

    // Declare histograms :
    auto RF_period_histo  = (histoed) ? std::make_unique<TH1F>(("RF_period_histo_"+run).c_str(), ("RF period histo "+run).c_str(), 50, 1000*USE_RF*0.98, 1000*USE_RF*1.02) : 0;

    auto ref_VS_RF        = (histoed) ? std::make_unique<TH1F>(("ref_VS_RF_"+run).c_str(), ("Timing reference "+run).c_str(), 8*USE_RF, -USE_RF/2, 3*USE_RF/2) : 0;

    auto detectors_VS_RF  = (histoed) ? std::make_unique<TH2F>(("detectors_VS_RF_"+run).c_str(), ("Timing all detectors "+run).c_str(), 
                              Detectors::number(),0,Detectors::number(), 2*USE_RF,-USE_RF/2,3*USE_RF/2) : 0;
    
    auto detectors_time  = (histoed) ? std::make_unique<TH2F>(("detectors_time_"+run).c_str(), ("Rel timing all detectors "+run).c_str(), 
                              Detectors::number(),0,Detectors::number(), 2*USE_RF,-USE_RF/2,3*USE_RF/2) : 0;
    
    // auto detectors_time  = (histoed) ? std::make_unique<TH2F>(("detectors_time_"+run).c_str(), ("Rel timing all detectors "+run).c_str(), 
    //                           Detectors::number(),0,Detectors::number(), 2*USE_RF,-USE_RF/2,3*USE_RF/2) : 0;

    auto Ge_spectra       = (histoed) ? std::make_unique<TH1F> (("Ge_spectra_"+run).c_str(), ("Ge spectra "+run).c_str(), 20000,0,10000 ) : 0;

    auto Ge_spectra_VS_T  = (histoed) ? std::make_unique<TH2F> (("Ge_spectra_VS_time_"+run).c_str(), ("Ge spectra VS time "+run).c_str(), 20000,0,10000, 2*USE_RF,-USE_RF/2,3*USE_RF/2) : 0;

    auto ref_VS_RF_after  = (histoed) ? std::make_unique<TH1F>(("ref_VS_RF_after_"+run).c_str(), ("Timing reference after "+run).c_str(), 2*USE_RF, -USE_RF/2, 3*USE_RF/2) : 0;
    
    auto BR2D1_labr3  = (histoed) ? std::make_unique<TH1F>(("BR2D1_labr3_"+run).c_str(), ("BR2D1 labr3 "+run).c_str(), 1500, 0, 15000) : 0;
    auto BR2D1_labr3_delayed  = (histoed) ? std::make_unique<TH1F>(("BR2D1_labr3_delayed_"+run).c_str(), ("BR2D1 labr3 "+run).c_str(), 1500, 0, 15000) : 0;
    auto BR2D1_nai  = (histoed) ? std::make_unique<TH1F>(("BR2D1_nai_"+run).c_str(), ("BR2D1 nai "+run).c_str(), 1500, 0, 15000) : 0;
    auto BR2D1_nai_delayed  = (histoed) ? std::make_unique<TH1F>(("BR2D1_nai_delayed_"+run).c_str(), ("BR2D1 nai "+run).c_str(), 1500, 0, 15000) : 0;

    auto const & prompt_LaBr3_VS_Ge = (histoed) ? std::make_unique<TH2F>(("prompt_LaBr3_VS_Ge_"+run).c_str(), ("Prompt LaBr3 VS Clover Ge "+run).c_str(), 5000,0,10000, 750,0,15000) : 0;
    auto const & delayed_LaBr3_VS_Ge = (histoed) ? std::make_unique<TH2F>(("delayed_LaBr3_VS_Ge_"+run).c_str(), ("Delayed LaBr3 VS Clover Ge "+run).c_str(), 5000,0,10000, 750,0,15000) : 0;
    auto const & prompt_NaI_VS_Ge = (histoed) ? std::make_unique<TH2F>(("prompt_NaI_VS_Ge_"+run).c_str(), ("Prompt NaI VS Clover Ge "+run).c_str(), 5000,0,10000, 500,0,15000) : 0;
    auto const & delayed_NaI_VS_Ge = (histoed) ? std::make_unique<TH2F>(("delayed_NaI_VS_Ge_"+run).c_str(), ("Delayed NaI VS Clover Ge "+run).c_str(), 5000,0,10000, 500,0,15000) : 0;

    // Loop over the whole folder :
    while(evt<chain.GetEntriesFast() && (nb_max_evts_read==-1) ? true : evt<nb_max_evts_read)
    {
      auto outTree  = std::make_unique<TTree>("Nuball2", "2 delayed clean Ge");
      outTree -> SetDirectory(nullptr);
      Timer timerFile;

    #if defined (USE_RF)
      event.writting(outTree.get(),"lTEQp");
    #elif defined (USE_DSSD_REF)
      event.writting(outTree.get(),"lTEQp");
    #else
      event.writting(outTree.get(),"ltEQp");
    #endif

      // Loop over the folder until the output tree reaches the maximum size, or the end of data is reached :
      ulong loop2 = 0;
      while(evt<chain.GetEntriesFast() && (nb_max_evts_read==-1) ? true : evt<nb_max_evts_read)
      {
        // Write in files of more or less the same size :
        if (evt%(int)(1.E+5) == 0 && outTree->GetEntries() > nb_max_evts_in_file) break;
      #ifdef DEBUG
        if (evt%(int)(1.E+6) == 0)
        {
          printMT(evt*1.E-6, "Mevts");
        }
      #endif //DEBUG

        // Read event :
        chain.GetEntry(evt++);

        if (event.mult>40) continue;

        // Extract the RF information and calculate the relative timestamp : 
        auto const & stamp = event.stamp;
        auto const rf_stamp = rf.pulse_ToF(stamp);
        for (int i = 0; i<event.mult; i++)
        {
          auto const & label = event.labels[i];
          auto const & time  = event.times [i];
          auto const & nrj  = event.nrjs [i];
          auto const & nrj2  = event.nrj2s [i];

          event.time2s[i] = Time_ns_cast(rf_stamp+time)/1000.f;
          auto const & time2 = event.time2s[i];

          if (label == 252) 
          {
            if (histoed) ref_VS_RF->Fill(event.time2s[i]);
          }
          else if (label == RF_Manager::label)
          {
            if (histoed) RF_period_histo->Fill(event.nrjs[i]);
            rf.period = Timestamp_cast(event.nrjs[i]);
            rf.last_hit = stamp;
          } 
          else if (label == 301) 
          {
            auto const & ratio = (nrj2-nrj)/nrj2;
            auto const & prompt = time2>-10 && time2 < 5;
            auto const & delayed = (time2>40 && time2 < 180) || (time2>240 && time2<380);
            clovers.SetEvent(event);
            if (ratio>-0.2 && ratio<0.2) 
            {
              if (prompt) 
              {
                BR2D1_labr3->Fill(nrj);
                for (auto const & index : clovers.promptClean) prompt_LaBr3_VS_Ge->Fill(clovers[index].nrj, nrj);
              }
              else if (delayed) 
              {
                BR2D1_labr3_delayed->Fill(nrj);
                for (auto const & index : clovers.delayedClean) delayed_LaBr3_VS_Ge->Fill(clovers[index].nrj, nrj);
              }
            }
            else if (ratio>0.5 && ratio < 0.8) 
            {
              if (prompt) 
              {
                BR2D1_nai->Fill(nrj2);
                for (auto const & index : clovers.promptClean) prompt_NaI_VS_Ge->Fill(clovers[index].nrj, nrj);
              }
              else if (delayed) 
              {
                BR2D1_nai_delayed->Fill(nrj2);
                for (auto const & index : clovers.promptClean) delayed_NaI_VS_Ge->Fill(clovers[index].nrj, nrj);
              }
            }
          }
        }

        if (check_trigger(counter, clovers, event, trigger_mode))
        {
          // Treat event :
          for (int i = 0; i<event.mult; i++)
          {
            auto const & label = event.labels[i];
            auto const & nrj = event.nrjs[i];
            auto const & time = event.times[i];
            auto const & time2 = event.time2s[i];

            if (histoed)
            {
              if (isGe[label]) 
              {
                Ge_spectra->Fill(nrj);
                Ge_spectra_VS_T -> Fill(nrj, time2);
                detectors_time -> Fill(label, time/1000.);
              }
              detectors_VS_RF -> Fill(compressedLabel[label], time2);
            }
          }
          outTree->Fill();
        }
        loop2++;
      }// End events loop

      file_nb++;
      File outName = outPath+run+"_"+std::to_string(file_nb)+".root";
      auto const sizeOut = outTree->GetEntries();

      std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
      file -> cd();
      outTree -> Write();
      file    -> Write();
      file    -> Close();
      printMT(outName,"written, ",sizeOut*1.E-6, "Mevts in", timerFile.TimeSec()," s (",100*sizeOut/loop2,"% kept)");
      converted_counter += sizeOut;
    }// End files loop

    if (histoed)
    {
      File outName = outPath+"histos_"+run+".root";
      std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
      file -> cd();

      RF_period_histo -> Write();
      ref_VS_RF       -> Write();
      detectors_VS_RF -> Write();
      detectors_time  -> Write();
      Ge_spectra      -> Write();
      Ge_spectra_VS_T -> Write();

      BR2D1_labr3 -> Write();
      BR2D1_labr3_delayed -> Write();
      BR2D1_nai -> Write();
      BR2D1_nai_delayed -> Write();

      prompt_LaBr3_VS_Ge -> Write();
      delayed_LaBr3_VS_Ge -> Write();
      prompt_NaI_VS_Ge -> Write();
      delayed_NaI_VS_Ge -> Write();
      
      file -> Write();
      file -> Close();
      print(outName, "written");
    }

    print(run, ":", evt*1.E-6, "->", converted_counter*1.E-6, "Mevts (",100*converted_counter/evt,"%) converted at a rate of", 1.E-3*evt/timer.TimeSec(), "kEvts/s");
    chain.Reset();
  }// End runs loop
}

/*

// auto RF_period_histo = std::make_unique<TH1F>(("RF_period_histo"+run+std::to_string(file_nb)).c_str(),("RF period histo"+run+std::to_string(file_nb)).c_str(), 40000, 0,4000);

      // auto LaBr3_spectra = std::make_unique<TH2F>(("LaBr3__spectra"+run+std::to_string(file_nb)).c_str(),("RF VS LaBr3 "+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);
      // auto DSSD_spectra = std::make_unique<TH2F>(("DSSD__spectra"+run+std::to_string(file_nb)).c_str(),("RF VS DSSD "+run+std::to_string(file_nb)).c_str(), 1000,0,20000, 500,-100,400);
      // auto Ge_spectra = std::make_unique<TH2F>(("Ge__spectra"+run+std::to_string(file_nb)).c_str(),("RF VS Ge "+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);
      // auto clover_mult = std::make_unique<TH1F>(("clover_mult_"+run+std::to_string(file_nb)).c_str(),("clover mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);
      // auto paris_mult = std::make_unique<TH1F>(("paris_mult_"+run+std::to_string(file_nb)).c_str(),("paris mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);
      // auto dssd_mult = std::make_unique<TH1F>(("dssd_mult_"+run+std::to_string(file_nb)).c_str(),("dssd mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);

          // if (label == 252)
          // {
          //   LaBr3_spectra->Fill(event.nrjs[i], event.time2s[i]);
          // }
          // else if (isGe[label])
          // {
          //   Ge_spectra->Fill(event.nrjs[i], event.time2s[i]);
          // }
          // else if (isDSSD[label])
          // {
          //   DSSD_spectra->Fill(event.nrjs[i], event.time2s[i]);
          // }

          #ifndef NO_TRIG
          clover_mult->Fill(counter.list_clovers.size());
          paris_mult ->Fill(counter.ParisMult);
          dssd_mult  ->Fill(counter.DSSDMult);
        #endif //NO_TRIG
*/

/*
// ----------------------------------------------- //
// --------------------- //
//     IDEE TIMING       //
// --------------------- //
// if (label == 251)
// {
    //Instead of  :

    // rf.period = (time+rand.Uniform(0,1)-rf.last_hit)/1000;

    //Go seek for the next RF Hit and calculate the mean
    // bool seek = true;
    // auto next_RF = evt;
    // while(seek)
    // {
    //   chain->GetEntry(next_RF++);
    //   for (size_t i = 0; i<event.size(); i++)
    //   {
    //     if (event.labels[i] == 251)
    //     {
    //       rf.period = ((event.times[i]-time+rand.Uniform(0,1))/1000 + (time-rf.last_hit+rand.Uniform(0,1))/1000)/2;
    //       seek = false;
    //     }
    //   }
    // }

    // rf.last_hit = time+rand.Uniform(0,1);
// }


// else if (evt%(int)(1.E+7) == 0)
// {
// auto const avancement = 100.*evt/nb_evts;
// auto const rate = avancement/totalTime.TimeSec();
// print(run, ":", (int)(avancement),"% -", (int)rate, "%/s");
// }

// ----------------------------------------------- //
*/