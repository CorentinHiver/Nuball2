//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

// #define N_SI_129
#define N_SI_136

#if defined (M2G1_TRIG)
#define COUNT_EVENT
#endif //COUNT_EVENT

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

enum trigger_modes{NO, M2G1, P, M2G1_P};
bool check_trigger(Counters & counter, Event & event, trigger_modes trig = P)
{
  counter.clear();
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
      counter.clover_analyse();
      return ((counter.Modules>1 && counter.RawGe>0) || counter.DSSDMult>0);
    default: return false;
  }
}

trigger_modes trigger_mode = P;

Path dataPath;
ushort nb_threads = 2;
int nb_max_evts_in_file = (int)(1.E+7); // 1 millions evts/fichier
bool calib_BGO = false;

void run_thread();

#ifdef N_SI_136
  Path outDir (Path::pwd()+"../136/matrices/", 1);
  File fileID ("index_129.dat");
  File runs_list = Path::pwd()+"faster_data/N-SI-136-root/";
#endif //N_SI_136


int main(int argc, char ** argv)
{
  if (!fileID) return -1;

  std::vector<std::string> runs;

  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string command = argv[i];
           if (command == "-m")
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
      else if (command == "-o" || command == "--out_path")
      {// Relative path to output 
        outDir = Path(std::string(argv[++i]), 1);
      }
      else if (command == "-n" || command == "--nb_hits")
      {// number of hits in one output file
        nb_max_evts_in_file = static_cast<int>(std::stod(argv[++i]));
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
        print("(      --BGO_cal)                                     : roughly, for BGO, Energy[keV] = ADC/100. Use this command if the BGO data are still in ADC");
        print("(-d || --data_path)  [/path/to/data/]                 : path to the data");
        print("(-f || --folder)     [folder/]                        : name of the folder to convert");
        print("(-F || --folders)    [nb_folders] [[list of folders]] : list of the folders to convert");
        print("(-h || --help)                                        : displays this help");
        print("(-i || --ID)         [/path/to/ID.dat]                :                     ");
        print("(-l || --list)       [/path/to/*.list]                : read a .list file with a list of folders to convert");
        print(" -m                  [nb_threads]                     : number of threads to be used");
        print("(-o || --out_path)   [/path/to/output/]               : path to the output");
        print("(-t || --trigger)    [0;3]                            : 0 : no trigger, 1 : M2G1, 2: P, 3 : M2G1_P");
        return 1;
      }
    }
  }

  Detectors g_listDet(fileID);
  if (runs.size() == 0) runs = listFileReader(runs_list);

  MTList runsMT(runs);

#ifdef DEBUG
  print(runsMT);
#endif //DEBUG

  std::string pathRun_str = "";
  while(runs.getNext(pathRun_str))
  {
    // Handles the names :
    Path pathRun = dataPath+pathRun_str;
    std::string run = pathRun.folder().name();
    Path outPath(outDir+run,1);
    if (!pathRun) {printMT(pathRun, "doesn't exists !"); return;}

    // Load the chain :
    TChain chain("Nuball2");
    chain.Add((pathRun.string()+"run_*.root").c_str());

    // Create the event reader :
    Event event;
    event.connect(&chain,"mltnN");

    // Declare the histograms :
    auto Bidim_Ge_BGO_R3A1 = std::make_unique<TH2F>(("Bidim_Ge_BGO_R2A1"+run).c_str(), ("Bidim Ge BGO R2A1"+run+";BGO [keV];Ge [keV]").c_str(), 500,0,10000, 10000,0,10000);
    auto Bidim_Ge_Paris_LaBr3_BR2D1 = std::make_unique<TH2F>(("Bidim_Ge_BGO"+run).c_str(), ("Bidim_Ge_BGO"+run+";BGO [keV];Ge [keV]").c_str(), 500,0,10000, 10000,0,10000);
  }

  print("Conversion done");
  return 1;
}

void convertRuns(MTList<std::string> & runs)
{
  TRandom rand(time(0));

  std::string pathRun_str = "";
  while(runs.getNext(pathRun_str))
  {
    // ---------------------- //
    // Convert a whole folder //
    // ---------------------- //

    Path pathRun = dataPath+pathRun_str;
    std::string run = pathRun.folder().name();
    Path outPath(outDir+run,1);

    printMT("Converting", run);

    if (!pathRun) {printMT(pathRun, "doesn't exists !"); return;}

  #ifdef DEBUG
    printMT("starting");
  #endif //DEBUG

    // Load the chain :
    TChain chain("Nuball2");
    chain.Add((pathRun.string()+"run_*.root").c_str());

    // Create the event reader :
    Event event;
    event.connect(&chain,"mltnN");

  #ifdef DEBUG
    printMT("Chain loaded");
  #endif //DEBUG


  #ifdef USE_RF
    RF_Manager rf;
    find_first_RF(chain, event, rf);
  #endif //USE_RF

    // Counters :
    ulonglong converted_counter = 0;
    ulonglong DSSD_seul = 0;
    Counters counter;

    Timer timer; // Measure the time to convert the whole run
    Long64_t evt = 0; // Event number
    int file_nb = 0; // File number

    // Declare histograms :
    auto RF_period_histo  = std::make_unique<TH1F>(("RF_period_histo_"+run).c_str(),("RF period histo "+run).c_str(), 50, 1000*USE_RF*0.98,1000*USE_RF*1.02);

    auto ref_VS_RF        = std::make_unique<TH1F>(("ref_VS_RF_"+run).c_str(),("Timing reference "+run).c_str(), 2*USE_RF, -USE_RF/2, 3*USE_RF/2);

    auto detectors_VS_RF  = std::make_unique<TH2F>(("detectors_VS_RF_"+run).c_str(),("Timing all detectors "+run).c_str(), 
                              Detectors::number(),0,Detectors::number(), 2*USE_RF,-USE_RF/2,3*USE_RF/2);

    auto Ge_spectra       = std::make_unique<TH1F> (("Ge_spectra_"+run).c_str()   ,("Ge spectra "+run).c_str(), 1000,0,5000 );

    auto outTree  = std::make_unique<TTree>("Nuball2", "Second conversion");

    #if defined (USE_RF)
      event.writeTo(outTree.get(),"lnNTRP");
    #elif defined (USE_DSSD_REF)
      event.writeTo(outTree.get(),"lnNT");
    #else
      event.writeTo(outTree.get(),"lnNt");
    #endif

    while(evt<chain.GetEntriesFast())
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

      if (check_trigger(counter, event, trigger_mode))
      {
      // Treat event :
        for (int i = 0; i<event.mult; i++)
        {
          auto const & label = event.labels[i];

          if (calib_BGO && isBGO[label]) event.nrjs[i] /= 100.;

        }
        outTree->Fill();
      }
      loop2++;

      file_nb++;
      File outName = outPath+run+"_"+std::to_string(file_nb)+".root";
      auto const sizeOut = outTree->GetEntries();

      std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
      file -> cd();

      outTree -> Write();

      file    -> Write();
      file    -> Close();
      converted_counter += sizeOut;
    }// End files loop

    File outName = outPath+"histos_"+run+".root";
    std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
    file -> cd();

    RF_period_histo -> Write();
    ref_VS_RF       -> Write();
    detectors_VS_RF -> Write();
    Ge_spectra      -> Write();
    
    file->Write();
    file -> Close();

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