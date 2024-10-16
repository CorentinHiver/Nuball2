//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

// #define N_SI_129
#define N_SI_136
#define USE_RF 200

#if defined (M2G1_TRIG)
#define COUNT_EVENT
#endif //COUNT_EVENT

#include <libCo.hpp>
#include <Event.hpp>
#include <FilesManager.hpp>
#include <Detectors.hpp>
#include <Timer.hpp>
#include <RF_Manager.hpp>

#include <Clovers.hpp>
// #include <Paris.hpp>

Path dataPath;
ushort nb_threads = 2;
bool calib_BGO = false;
double prompt_time_window_ns = 50.;

void run_thread();

#ifdef N_SI_136
  Path outDir (Path::pwd()+"../136/matrices/", 1);
  File fileID ("index_129.dat");
  // File runs_list = Path::pwd()+"faster_data/N-SI-136-root/";
  File runs_list;
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
      {// List of the folders to convert
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
      else if (command == "-p" || command == "--prompt")
      {
        prompt_time_window_ns = std::stod(argv[++i]);
      }
      else if (command == "-r" || command == "--RF_period")
      {
        // USE_RF = (int)std::stoi(argv[++i]);
      }
      else if (command == "-l" || command == "--list")
      { // .list file containing the list of folders to convert
        runs_list = argv[++i];
      }
      else if (command == "-i" || command == "--index")
      {// ID file to load :
        fileID = std::string(argv[++i]);
      }
      else if (command == "-b" || command == "--BGO_cal")
      {// "Gain match" BGO is not already done
        calib_BGO = true;
      }
      else if (command == "-h" || command == "--help")
      {
        print("List commands :");
        print("(-b || --BGO_cal)                                      : roughly, for BGO, Energy[keV] ~ ADC/100. Use this command if the BGO data are still in ADC");
        print("(-d || --data_path)   [/path/to/data/]                 : path to the data");
        print("(-f || --folder)      [folder/]                        : name of the folder to convert");
        print("(-F || --folders)     [nb_folders] [[list of folders]] : list of the folders to convert");
        print("(-h || --help)                                         : displays this help");
        print("(-i || --ID)          [/path/to/ID.dat]                :                     ");
        print("(-l || --list)        [/path/to/*.list]                : read a .list file with a list of folders to convert");
        print("(-m || --multithread) [nb_threads]                     : number of threads to be used");
        print("(-o || --out_path)    [/path/to/output/]               : path to the output");
        print("(-p || --prompt)      [time gate in ns]                : two detectors are prompt with respect to each other if their time difference is lower than the time gate");
        // print("(-r || --RF_period)   [time in ns]                     : define the time period of the RF if different to 200ns");
        return 1;
      }
    }
  }

  Detectors g_listDet(fileID);
  if (runs.size() == 0) runs = listFileReader(runs_list);

#ifdef DEBUG
  print(runs);
#endif //DEBUG

  for (auto const & run_str : runs)
  {
    // Handles the names :
    Path pathRun = dataPath+run_str;
    std::string run = pathRun.folder().name();
    auto run_nb_str = lastPart(run,'_');
    Path outPath(outDir+run,1);

    if (!pathRun) {print(pathRun, "doesn't exists !"); continue;}

    // Load the chain :
    TChain chain("Nuball2");
    chain.Add((pathRun.string()+"run_*.root").c_str());
    auto const nb_evts = chain.GetEntries();

    // Create the event reader :
    Event event;
    event.connect(&chain,"mltnN");

    // --- Histograms : --- //
    // RF time spectra :
    auto Clover_time_spectra_RF = std::make_unique<TH1F>(("Clover_time_spectra_RF_"+run).c_str(), ("Ge clover time spectra run "+run_nb_str+";RF time [ns]").c_str(), 4000,-100,300);
    auto BGO_time_specta = std::make_unique<TH1F>(("BGO_time_spectra_RF_"+run).c_str(), ("Ge clover time spectra run "+run_nb_str+";RF time [ns]").c_str(), 400,-100,300);
    
    // Bidims with RF
    auto Clover_time_spectra_RF__VS__Ge_mult = std::make_unique<TH2F>(("Clover_time_spectra_RF_VS_mult_"+run).c_str(), ("Ge clover time spectra run "+run_nb_str+";RF time [ns]; mult").c_str(), 4000,-100,300, 50,0,50);
    auto Clover_time_spectra_RF__VS__timeClover_VS_BGO= std::make_unique<TH2F>(("Clover_time_spectra_RF__VS__timeClover_VS_BGO_"+run).c_str(), 
            ("RF VS (Ge-BGO) time spectra run "+run_nb_str+";RF time [ns]; Ge time - BGO time [ns]").c_str(), 400,-100,300, 1000,-210000,210000);

    // Bidim Clover Ge / Clover Ge
    auto Bidim_Clover_clean = std::make_unique<TH2F>(("Bidim_Clover_clean_"+run).c_str(), ("Bidim clover clean run "+run_nb_str+";Ge clover [keV];Ge crystal [keV]").c_str(), 1000,0,10000, 5000,0,10000);
    auto Bidim_Clover_clean_prompt_prompt   = std::make_unique<TH2F>(("Bidim_Clover_clean_pp"+run).c_str(), ("Bidim clover clean prompt prompt run "+run_nb_str+";Energy [keV];Energy [keV]").c_str(), 1000,0,10000, 5000,0,10000);
    auto Bidim_Clover_clean_delayed_delayed = std::make_unique<TH2F>(("Bidim_Clover_clean_dd"+run).c_str(), ("Bidim clover clean delayed delayed run "+run_nb_str+";Energy [keV];Energy [keV]").c_str(), 1000,0,10000, 5000,0,10000);

    // Bidim Clover/Ge crystal nrj 
    auto Bidim_Ge_BGO_R3A1_blue = std::make_unique<TH2F>(("Bidim_Ge_R3A1_blue_VS_clover_Ge_"+run).c_str(), ("Bidim Ge BGO R3A1 BGO1 run "+run_nb_str+";Ge clover [keV];Ge crystal [keV]").c_str(), 1000,0,10000, 5000,0,10000);

    // Bidim Ge/BGO nrj
    auto Bidim_Ge_BGO_R3A1_BGO1 = std::make_unique<TH2F>(("Bidim_Ge_BGO1_R3A1_"+run).c_str(), ("Bidim Ge BGO R3A1 BGO1 run "+run_nb_str+";BGO [keV];Ge [keV]").c_str(), 1000,0,10000, 5000,0,10000);
    
    // Bidim BGO_index/BGO_nrj :
    auto BGO_spectra_gated_511 = std::make_unique<TH2F>(("BGO_spectra_511_"+run).c_str(), (" BGO spectra gated 511 keV run "+run_nb_str+";E [keV]").c_str(), 49,0,48, 1000,0,10000);
    auto BGO_spectra_gated_511_prompt = std::make_unique<TH2F>(("BGO_spectra_511_prompt_"+run).c_str(), ("Prompt BGO spectra gated 511 keV run "+run_nb_str+";E [keV]").c_str(), 49,0,48, 1000,0,10000);
    auto BGO_spectra_gated_511_not_prompt = std::make_unique<TH2F>(("BGO_spectra_511_not_prompt_"+run).c_str(), ("not prompt BGO spectra gated 511 keV run "+run_nb_str+";E [keV]").c_str(), 49,0,48, 1000,0,10000);
    
    // Time spectra Ge_time-BGO_time:
    auto diff_tempo_Ge_VS_BGO = std::make_unique<TH1F>(("diff_tempo_Ge-BGO_"+run).c_str(), ("diff_tempo_Ge-BGO_"+run+";timeGe-time_BGO [ns]").c_str(), 1000,-210000,210000);
    auto diff_tempo_Ge_VS_BGO_prompt = std::make_unique<TH1F>(("prompt_diff_tempo_Ge-BGO_"+run).c_str(), ("prompt_diff_tempo_Ge-BGO_"+run+";timeGe-time_BGO [ns]").c_str(), 1000,-210000,210000);
    auto diff_tempo_Ge_VS_BGO_511 = std::make_unique<TH1F>(("diff_tempo_Ge-BGO_511_"+run).c_str(), ("diff_tempo_Ge-BGO_511_"+run+";timeGe-time_BGO [ns]").c_str(), 1000,-210000,210000);
    auto diff_tempo_Ge_VS_BGO_prompt_511 = std::make_unique<TH1F>(("prompt_diff_tempo_Ge-BGO_511_"+run).c_str(), ("prompt_diff_tempo_Ge-BGO_511_"+run+";timeGe-time_BGO [ns]").c_str(), 1000,-210000,210000);

    // Bidim Time spectra Ge_time-BGO_time and nrj :
    auto diff_tempo_Ge_VS_BGO__VS_Ge_nrj = std::make_unique<TH2F>(("diff_tempo_Ge_VS_BGO__VS_Ge_nrj_"+run).c_str(), 
                (" diff_tempo_Ge-BGO__VS_Ge_nrj "+run_nb_str+";timeGe-time_BGO [ns];E [keV]").c_str(), 200,-200000,200000, 10000,0,10000);
    auto diff_tempo_Ge_VS_BGO__VS_BGO_nrj = std::make_unique<TH2F>(("diff_tempo_Ge_VS_BGO__VS_BGO_nrj_"+run).c_str(), 
                (" diff_tempo_Ge-BGO__VS_BGO_nrj "+run_nb_str+";timeGe-time_BGO [ns];E [keV]").c_str(), 800,-200000,200000, 500,0,10000);
    
    // Same, but with additionnal energy gate on the other detector :
    auto diff_tempo_Ge_VS_BGO__VS_Ge_nrj_511 = std::make_unique<TH2F>(("diff_tempo_Ge_VS_BGO__VS_Ge_nrj_511_"+run).c_str(), 
                (" diff_tempo_Ge-BGO__VS_Ge_nrj gate 511 "+run_nb_str+";timeGe-time_BGO [ns];E [keV]").c_str(), 200,-200000,200000, 10000,0,10000);
    auto diff_tempo_Ge_VS_BGO__VS_BGO_nrj_511 = std::make_unique<TH2F>(("diff_tempo_Ge_VS_BGO__VS_BGO_nrj_511_"+run).c_str(), 
                (" diff_tempo_Ge-BGO__VS_BGO_nrj_gate 511 "+run_nb_str+";timeGe-time_BGO [ns];E [keV]").c_str(), 800,-200000,200000, 500,0,10000);

    // auto Bidim_Ge_Paris_LaBr3_BR2D1 = std::make_unique<TH2F>(("Bidim_Ge_BGO"+run).c_str(),  ("Bidim_Ge_BGO"+run+";BGO [keV];Ge [keV]").c_str(), 500,0,10000, 10000,0,10000);

  #ifdef USE_RF
    RF_Manager rf;
    rf.set_offset_ns(35);
    RF_Extractor first_rf(&chain, rf, event);
    if (!first_rf) throw std::runtime_error(("NO RF IN FILE"+ run_str).c_str());
  #endif //USE_RF

    Clovers clovers;
    // Paris paris;

    Long64_t evt = 0;
    while(evt<chain.GetEntriesFast())
    {
      if (evt%(int)(1.E+6) == 0) print((int)((100.*evt)/nb_evts),"%");
      
      chain.GetEntry(evt++);

      if (event.labels[0] == RF_Manager::label)
      {
        rf.setHit(event[0]);
        continue;
      }

      clovers.SetEvent(event);
      clovers.Analyse();

      for (uint loop_i = 0; loop_i<clovers.Clean_Ge.size(); loop_i++)
      {
        auto const & hit_i = clovers.Clean_Ge[loop_i];

        auto const & clover_i = clovers[hit_i];
        auto const & nrj_Ge = clover_i.nrj;
        auto const & time_Ge = clover_i.time;

        // RF timing :
        auto const Time_Ge = rf.pulse_ToF_ns(clover_i.time);
        Clover_time_spectra_RF -> Fill(Time_Ge);
        auto const Ge_isPrompt = ((Time_Ge>-30) && (Time_Ge<30));
        Clover_time_spectra_RF__VS__Ge_mult -> Fill(Time_Ge, clovers.Clean_Ge.size());

        for (uint loop_j = loop_i; loop_j<clovers.Clean_Ge.size(); loop_j++)
        {
          auto const & hit_j = clovers.Clean_Ge[loop_j];
          auto const & clover_j = clovers[hit_j];
          auto const & nrj_Ge_j = clover_j.nrj;
          Bidim_Clover_clean -> Fill(nrj_Ge, nrj_Ge_j);
        }

        for (auto const & Ge_crystal : clovers.cristaux)
        {
          auto const & nrj_crystal = clovers.cristaux_nrj[Ge_crystal];
          // print(nrj_Ge, Ge_crystal);
          // sleep(1);
          if (Ge_crystal == 0 && clover_i.label() != 0) Bidim_Ge_BGO_R3A1_blue -> Fill(nrj_Ge, nrj_crystal);
        }

        for (auto const & BGO_crystal : clovers.cristaux_BGO)
        {
          auto nrj_BGO = clovers.cristaux_nrj_BGO[BGO_crystal];
          if (calib_BGO) nrj_BGO /= 76.;

          // Bidim Ge/R3A1_BGO1 to look for coincidences
          if (BGO_crystal == 0) Bidim_Ge_BGO_R3A1_BGO1 -> Fill(nrj_BGO, nrj_Ge);

          // Coincidences with 511 keV in Ge :
          if (nrj_Ge>505 && nrj_Ge<515) BGO_spectra_gated_511 -> Fill(BGO_crystal, nrj_BGO);

          // Timing between Ge and BGO :
          auto const & time_BGO = clovers.cristaux_time_BGO[BGO_crystal];
          auto const diff_time = time_Ge-time_BGO; 

          diff_tempo_Ge_VS_BGO -> Fill(diff_time);
          Clover_time_spectra_RF__VS__timeClover_VS_BGO -> Fill(Time_Ge, diff_time);
          diff_tempo_Ge_VS_BGO__VS_Ge_nrj  -> Fill(diff_time, nrj_Ge );
          diff_tempo_Ge_VS_BGO__VS_BGO_nrj -> Fill(diff_time, nrj_BGO);
          
          // Gate on BGO energy :
          if (nrj_BGO>300 && nrj_BGO<600) diff_tempo_Ge_VS_BGO__VS_Ge_nrj_511 -> Fill(diff_time, nrj_Ge);

          // Gate on Ge energy :
          if (nrj_Ge>505 && nrj_Ge<515) 
          {
            diff_tempo_Ge_VS_BGO_511->Fill(diff_time);
            diff_tempo_Ge_VS_BGO__VS_BGO_nrj_511 -> Fill(diff_time, nrj_BGO);
          }

          // Gating on diff_time :
          if (abs(diff_time) < (prompt_time_window_ns*1000.))
          {
            diff_tempo_Ge_VS_BGO_prompt->Fill(diff_time);// Check the time gate
            if (nrj_Ge>505 && nrj_Ge<515) 
            {
              BGO_spectra_gated_511_prompt -> Fill(BGO_crystal, nrj_BGO);
              diff_tempo_Ge_VS_BGO_prompt_511->Fill(diff_time); // Check the time and energy gate
            }
          }
          else BGO_spectra_gated_511_not_prompt -> Fill(BGO_crystal, nrj_BGO);
        }
      }
    }

    std::string const outRoot = "test_bidim.root";
    std::unique_ptr<TFile> outFile (TFile::Open(outRoot.c_str(),"recreate"));
    outFile -> cd();

      Clover_time_spectra_RF -> Write();
      Clover_time_spectra_RF__VS__timeClover_VS_BGO -> Write();
      Clover_time_spectra_RF__VS__Ge_mult -> Write();

      Bidim_Ge_BGO_R3A1_blue -> Write();
      Bidim_Ge_BGO_R3A1_BGO1 -> Write();

      BGO_spectra_gated_511 -> Write();
      BGO_spectra_gated_511_prompt -> Write();
      BGO_spectra_gated_511_not_prompt -> Write();

      diff_tempo_Ge_VS_BGO->Write();
      diff_tempo_Ge_VS_BGO_prompt->Write();
      diff_tempo_Ge_VS_BGO_511->Write();
      diff_tempo_Ge_VS_BGO_prompt_511->Write();

      diff_tempo_Ge_VS_BGO__VS_Ge_nrj  -> Write();
      diff_tempo_Ge_VS_BGO__VS_BGO_nrj -> Write();
      diff_tempo_Ge_VS_BGO__VS_Ge_nrj_511  -> Write();
      diff_tempo_Ge_VS_BGO__VS_BGO_nrj_511 -> Write();

      outFile -> Write();      
    outFile -> Close();
    print(outRoot, "written");

  }

  return 1;
}

/*
// void convertRuns(MTList<std::string> & runs)
// {
//   TRandom rand(time(0));

//   std::string pathRun_str = "";
//   while(runs.getNext(pathRun_str))
//   {
//     // ---------------------- //
//     // Convert a whole folder //
//     // ---------------------- //

//     Path pathRun = dataPath+pathRun_str;
//     std::string run = pathRun.folder().name();
//     Path outPath(outDir+run,1);

//     printMT("Converting", run);

//     if (!pathRun) {printMT(pathRun, "doesn't exists !"); return;}

//   #ifdef DEBUG
//     printMT("starting");
//   #endif //DEBUG

//     // Load the chain :
//     TChain chain("Nuball2");
//     chain.Add((pathRun.string()+"run_*.root").c_str());

//     // Create the event reader :
//     Event event;
//     event.connect(&chain,"mltnN");

//   #ifdef DEBUG
//     printMT("Chain loaded");
//   #endif //DEBUG


//   #ifdef USE_RF
//     RF_Manager rf;
//     find_first_RF(chain, event, rf);
//   #endif //USE_RF

//     // Counters :
//     ulonglong converted_counter = 0;
//     ulonglong DSSD_seul = 0;
//     Counters counter;

//     Timer timer; // Measure the time to convert the whole run
//     Long64_t evt = 0; // Event number
//     int file_nb = 0; // File number

//     // Declare histograms :

//     auto outTree  = std::make_unique<TTree>("Nuball2", "Second conversion");

//     #if defined (USE_RF)
//       event.writeTo(outTree.get(),"lnNTRP");
//     #elif defined (USE_DSSD_REF)
//       event.writeTo(outTree.get(),"lnNT");
//     #else
//       event.writeTo(outTree.get(),"lnNt");
//     #endif

//     while(evt<chain.GetEntriesFast())
//     {
//       // Write in files of more or less the same size :
//       if (evt%(int)(1.E+5) == 0 && outTree->GetEntries() > nb_max_evts_in_file) break;
//     #ifdef DEBUG
//       if (evt%(int)(1.E+6) == 0)
//       {
//         printMT(evt*1.E-6, "Mevts");
//       }
//     #endif //DEBUG

//       // Read event :
//       chain.GetEntry(evt++);

//       if (check_trigger(counter, event, trigger_mode))
//       {
//       // Treat event :
//         for (int i = 0; i<event.mult; i++)
//         {
//           auto const & label = event.labels[i];

//           if (calib_BGO && isBGO[label]) event.nrjs[i] /= 100.;

//         }
//         outTree->Fill();
//       }
//       loop2++;

//       file_nb++;
//       File outName = outPath+run+"_"+std::to_string(file_nb)+".root";
//       auto const sizeOut = outTree->GetEntries();

//       std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
//       file -> cd();

//       outTree -> Write();

//       file    -> Write();
//       file    -> Close();
//       converted_counter += sizeOut;
//     }// End files loop

//     File outName = outPath+"histos_"+run+".root";
//     std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
//     file -> cd();

//     RF_period_histo -> Write();
//     ref_VS_RF       -> Write();
//     detectors_VS_RF -> Write();
//     Ge_spectra      -> Write();
    
//     file->Write();
//     file -> Close();

//     print(run, ":", evt*1.E-6, "->", converted_counter*1.E-6, "Mevts (",100*converted_counter/evt,"%) converted at a rate of", 1.E-3*evt/timer.TimeSec(), "kEvts/s");
//     chain.Reset();
//   }// End runs loop
// }

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