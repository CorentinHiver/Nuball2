//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

// #define N_SI_129
#define N_SI_136
// #define N_SI_120
// #define USE_PARIS
// #define USE_DSSD
// #define FATIMA
// #define USE_LICORNE
// #define M2G1_TRIG
#define NO_TRIG
// #define DSSD_TRIG
// #define DSSD_M2G1_TRIG

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

#define USE_RF 200
#define RF_SHIFT 50

// std:: = "/home/corentin/faster_data/N-SI-129-root/M2G1_D1_TRIG/";
// std:: = "../Data_129/M2G1/";
Path dataPath = "/home/corentin/faster_data/N-SI-136-root/";
ushort nb_threads = 2;
int nb_max_evts_in_file = (int)(1.E+6); // 1 millions evts/fichier

void run_thread();

#ifdef N_SI_136
  Path outDir (Path::pwd()+"../136/conversion/", 1);
  File fileID ("index_129.dat");
  File runs_list = Path::pwd()+"../Parameters/runs_pulsed_Corentin.list";
#endif //N_SI_136

MTList runs (listFileReader(runs_list));
Detectors g_listDet(fileID);

// Forward declaration
void convertRun();

int main(int argc, char ** argv)
{
  if (!fileID) return -1;
  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string command = argv[i];
           if (command == "-m")
      {// Multithreading 
        nb_threads = atoi(argv[++i]);
      }
    }
  }

  if(nb_threads == 1) convertRun();

  else
  {
    MTObject::Initialize(nb_threads);
    MTObject::adjustThreadsNumber(runs.size(), "number of runs too small");
    MTObject::parallelise_function(convertRun);
  }
  print("Conversion done");
  return 1;
}

void convertRun()
{
  TRandom rand(time(0));
  std::string run = "";
  while(runs.getNext(run))
  {
    MTObject::shared_mutex.lock();
    print("Converting",run);
    MTObject::shared_mutex.unlock();
    Path pathRun = dataPath+run;
    if (!pathRun) {print(pathRun, "doesn't exists !"); return;}
    std::string rootFiles = pathRun+"*.root";

  #ifdef DEBUG
    print("starting");
  #endif //DEBUG

    TChain chain("Nuball");
    chain.Add(rootFiles.c_str());
    Event event;
    event.connect(&chain,"mltnN");

  #ifdef DEBUG
    print("Chain loaded");
  #endif //DEBUG

    Timer readTimer;

    Long64_t evt = 0;


  #ifdef USE_RF
    RF_Manager rf;
    bool stop = false;
    Long64_t maximum_RF_location = 1.E+7;
    while(!stop && evt < maximum_RF_location)
    {
      chain.GetEntry(evt++);
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
    if (evt == maximum_RF_location) {print("NO RF found !!"); return;}
  #ifdef DEBUG
    // print("RF extracted at hit n°", evt);
  #endif //DEBUG
  #endif //USE_RF


    // Loop over all the data :
  #ifndef NO_TRIG
    Counters counter;
  #endif //NO_TRIG
    evt = 1;
    int file_nb = 0;
    Timer totalTime;

    // Counters :
    ulonglong converted_counter = 0;
    ulonglong DSSD_seul = 0;

    while(evt<chain.GetEntriesFast())
    {
      Timer timer;

      auto outTree  = std::make_unique<TTree>("Nuball", "Second conversion");

      auto RF_period_histo = std::make_unique<TH1F>(("RF_period_histo"+run+std::to_string(file_nb)).c_str(),("RF period histo"+run+std::to_string(file_nb)).c_str(), 40000, 0,4000);

      auto RF_VS_LaBr3 = std::make_unique<TH2F>(("RF_VS_LaBr3_"+run+std::to_string(file_nb)).c_str(),("RF VS LaBr3 "+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);
      auto RF_VS_DSSD = std::make_unique<TH2F>(("RF_VS_DSSD_"+run+std::to_string(file_nb)).c_str(),("RF VS DSSD "+run+std::to_string(file_nb)).c_str(), 1000,0,20000, 500,-100,400);
      auto RF_VS_Ge = std::make_unique<TH2F>(("RF_VS_Ge_"+run+std::to_string(file_nb)).c_str(),("RF VS Ge "+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);
      auto clover_mult = std::make_unique<TH1F>(("clover_mult_"+run+std::to_string(file_nb)).c_str(),("clover mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);
      auto paris_mult = std::make_unique<TH1F>(("paris_mult_"+run+std::to_string(file_nb)).c_str(),("paris mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);
      auto dssd_mult = std::make_unique<TH1F>(("dssd_mult_"+run+std::to_string(file_nb)).c_str(),("dssd mult "+run+std::to_string(file_nb)).c_str(), 20,0,20);

    #if defined (USE_RF)
      event.writeTo(outTree.get(),"lnNTRP");
    #elif defined (USE_DSSD_REF)
      event.writeTo(outTree.get(),"lnNT");
    #else
      event.writeTo(outTree.get(),"lnNt");
    #endif

      // Loop over the data until the output tree reaches the maximum size, or the end of data is reached :
      while(evt<chain.GetEntriesFast())
      {
        // Write in files of more or less the same size :
       if (evt%(int)(1.E+5) == 0 && outTree->GetEntries() > nb_max_evts_in_file) break;
       if (evt%(int)(1.E+6) == 0)
       {
         print(evt*1.E-6, "Mevts");
       }

        // Read event :
        chain.GetEntry(evt++);

        #ifndef USE_RF
          auto const & time_ref = event.times[0];
        #endif //NO USE_RF

        // Treat event :
        for (int i = 0; i<event.mult; i++)
        {
          auto const & label = event.labels[i];
          auto const & time  = event.times [i];

        #ifndef NO_TRIG
          counter.set_hit(event, i);
        #endif //NO_TRIG

        #ifdef USE_RF

          if (rf.period!=0) event.time2s[i] = rf.pulse_ToF(time)/_ns;
          if (label == 251)
          {
            event.RFperiod = (time+rand.Uniform(0,1)-rf.last_hit)/1000.;
            RF_period_histo->Fill(event.RFperiod);
            rf.period = static_cast<Time>(event.RFperiod);
            rf.last_hit = time+rand.Uniform(0,1);
          }
        #else
          event.time2s[i] = (time - time_ref)/_ns;
        #endif //USE_RF

          if (label == 252)
          {
            RF_VS_LaBr3->Fill(event.nrjs[i], event.time2s[i]);
          }
          else if (isGe[label])
          {
            RF_VS_Ge->Fill(event.nrjs[i], event.time2s[i]);
          }
          else if (isDSSD[label])
          {
            RF_VS_DSSD->Fill(event.nrjs[i], event.time2s[i]);
          }
        }

      #ifdef M2G1_TRIG
        counter.clover_analyse();
        bool trig = (counter.Modules>1 && counter.rawGe>0);
      #endif //M2G1_TRIG

      #ifdef DSSD_TRIG
        bool trig = (counter.DSSDMult>0);
        if (counter.DSSDMult == counter.mult) DSSD_seul++;
      #endif //DSSD_TRIG

      #ifdef DSSD_M2G1_TRIG
        bool trig = (counter.DSSDMult>0 && counter.Modules>1 && counter.rawGe>0);
      #endif //DSSD_TRIG

      #ifdef NO_TRIG
        bool trig = true;
      #endif //NO_TRIG

        if (trig)
        {
        #ifndef NO_TRIG
          clover_mult->Fill(counter.list_clovers.size());
          paris_mult ->Fill(counter.ParisMult);
          dssd_mult  ->Fill(counter.DSSDMult);
        #endif //NO_TRIG

        #ifdef USE_RF
          event.RFtime = rf.last_hit;
        #endif //USE_RF

          outTree->Fill();
        }
      #ifndef NO_TRIG
        counter.clear();       
      #endif //NO_TRIG

      }// End events loop

      file_nb++;
      Path outPath = outDir+run;
      create_folder_if_none(outPath);
      File outName = outPath+run+"_"+std::to_string(file_nb)+".root";
      auto const sizeOut = outTree->GetEntries();

      std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
      file -> cd();

      RF_VS_LaBr3 -> Write();
      RF_VS_DSSD -> Write();
      RF_VS_Ge -> Write();
      clover_mult -> Write();
      paris_mult -> Write();
      dssd_mult -> Write();

      outTree -> Write();
      file    -> Write();
      file    -> Close();
      print(outName,"written, ",sizeOut*1.E-6, "Mevts in", timer.TimeSec()," s (",100*sizeOut/evt,"% kept)");
      converted_counter += sizeOut;
    }// End files loop
    print(run, ":", evt*1.E-6, "->", converted_counter*1.E-6, "Mevts (",100*converted_counter/evt,"%) converted at a rate of", 1.E-3*evt/readTimer.TimeSec(), "kEvts/s");
    chain.Reset();
  }// End runs loop
}


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
