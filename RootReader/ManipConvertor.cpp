//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

// #define N_SI_129
#define N_SI_120
// #define USE_PARIS
#define QDC2
// #define USE_DSSD
#define FATIMA
#define USE_LICORNE
// #define M2G1_TRIG
// #define NO_TRIG
// #define DSSD_TRIG
// #define DSSD_M2G1_TRIG
#define USE_RF 50

#if defined (M2G1_TRIG)
#define COUNT_EVENT
#endif //COUNT_EVENT

#define CORENTIN
// #define DATA2

#include <Event.hpp>
#include <FilesManager.hpp>
#include <MTTHist.hpp>
#include <MTList.hpp>
#include <MTCounter.hpp>
#include <DetectorsList.hpp>
#include <Counters.hpp>
#include <Detectors.hpp>

DetectorsList g_listDet;

#include <QuickParameters.hpp>

void convertRun(quick_parameters & param);
// void run_thread(quick_parameters & param);

void test(){}

int main(int argc, char ** argv)
{
  quick_parameters qp;

  create_folder_if_none(qp.outDir);

       if (argc == 2 && strcmp(argv[1],"-m")==0) qp.nb_threads = 1;
  else if (argc == 3 && strcmp(argv[1],"-m")==0) qp.nb_threads = atoi(argv[2]);

  // Initialize arrays
  g_listDet.load(qp.fileID);
  auto m_nb_labels = g_listDet.size();
  Detectors::Initialize();

  qp.runs = listFileReader(qp.runs_list);

  checkThreadsNb(qp.nb_threads, qp.runs.size());

  if(qp.nb_threads == 1)
  {
    convertRun(qp);
  }

  else
  {
    MTObject::Initialize(qp.nb_threads);
    MTObject::parallelise_function(convertRun,qp);
  }
  print("Conversion done");
  return 1;
}

void convertRun(quick_parameters & param)
{
  TRandom rand(time(0));
  std::string run = "";
  while(param.runs.getNext(run))
  {
    MTObject::shared_mutex.lock();
    print("Converting",run);
    MTObject::shared_mutex.unlock();
    std::string pathRun = param.dataPath+run;
    makePath(pathRun);
    if (!folder_exists(pathRun)) {print(pathRun, "doesn't exists !"); return;}
    std::string rootFiles = pathRun+"*.root";

  #ifdef DEBUG
    print("starting");
  #endif //DEBUG

    TChain chain("Nuball");
    chain.Add(rootFiles.c_str());
    // Event event(&chain,"mltnN");
    Event event;
    event.connect(&chain,"mltnN");
    // auto nb = chain->GetEntries();

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
      // event.Print();
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
    // print("RF extracted at hit n°", evt);
  #endif //USE_RF


    // Loop over all the data :
  #ifndef NO_TRIG
    Counters counter;
  #endif //NO_TRIG
    evt = 1;
    int file_nb = 0;
    Timer totalTime;

    // Counters :
    Long64_t converted_counter = 0;
    Long64_t DSSD_seul = 0;

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
       if (evt%(int)(1.E+5) == 0 && outTree->GetEntries() > param.nb_max_evts_in_file) break;
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

          counter.set_hit(event, i);

        #ifdef USE_RF

          if (rf.period!=0) event.time2s[i] = rf.pulse_ToF(time,param.RF_shift)/_ns;
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

        if (trig)
        {
          clover_mult->Fill(counter.list_clovers.size());
          paris_mult ->Fill(counter.ParisMult);
          dssd_mult  ->Fill(counter.DSSDMult);

        #ifdef USE_RF
          event.RFtime = rf.last_hit;
        #endif //USE_RF

          outTree->Fill();
        }
        counter.clear();
      }// End events loop

      file_nb++;
      std::string outPath = param.outDir+run+"/";
      create_folder_if_none(outPath);
      std::string outName = outPath+run+"_"+std::to_string(file_nb)+".root";
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
