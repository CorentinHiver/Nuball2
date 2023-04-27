//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

#define N_SI_129
#define PARIS
#define QDC2
#define USE_DSSD
#define FATIMA
// #define M2G1_TRIG
// #define NO_TRIG
#define DSSD_TRIG

#if defined (M2G1_TRIG)
#define COUNT_EVENT
#endif //COUNT_EVENT

// #define CORENTIN
#define DATA2

#include "../lib/utils.hpp"
#include "../lib/Classes/Event.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/MTObjects/MTTHist.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTCounter.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"
#include "Classes/QuickParameters.hpp"
#include "Modules/EachDetector.hpp"

void convertRun(quick_parameters & param);
// void run_thread(quick_parameters & param);

void test(){}

int main(int argc, char ** argv)
{
  quick_parameters qp;

  std::vector<std::thread> threads;

       if (argc == 2 && strcmp(argv[1],"-m")==0) qp.nb_threads = 1;
  else if (argc == 3 && strcmp(argv[1],"-m")==0) qp.nb_threads = atoi(argv[2]);

  // Parameters p;
  // p.readParameters();

  // Initialize arrays
  g_labelToName = arrayID(qp.fileID);
  auto m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);
  if (qp.nb_threads>1)

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

  return 1;
}

void convertRun(quick_parameters & param)
{
  TRandom rand(time(0));
  std::string run = "";
  while(param.runs.getNext(run))
  {
    Timer readTimer;
    MTObject::shared_mutex.lock();
    print("Converting",run);
    MTObject::shared_mutex.unlock();
    std::string pathRun = param.dataPath+run;
    std::string rootFiles = pathRun+"/*.root";
    TChain chain ("Nuball");
    chain.Add(rootFiles.c_str());

    Event event(&chain,"mltnN");

    int nb_evts = chain.GetEntries();
    int evt = 0;
    int file_nb = 0;

    #ifdef COUNT_EVENT
    Counters counter;
    #endif //COUNT_EVENT

    RF_Manager rf;
    chain.GetEntry(0);
    for (size_t i = 0; i<event.size(); i++) if (event.labels[i] == 251) rf.last_hit = event.times[i];

    while(evt<nb_evts)
    {// Write in files of more or less the same size
      Timer timer;
      std::unique_ptr<TTree> outTree (new TTree("Nuball", "Second conversion"));
      auto const & RF_VS_LaBr3 = new TH2F(("RF_VS_LaBr3"+run+std::to_string(file_nb)).c_str(),("RF_VS_LaBr3"+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);
      auto const & RF_VS_Ge = new TH2F(("RF_VS_Ge"+run+std::to_string(file_nb)).c_str(),("RF_VS_Ge"+run+std::to_string(file_nb)).c_str(), 1000,0,5000, 500,-100,400);

      event.writeTo(outTree.get(),"lnNTRP");

      while(evt<nb_evts)
      {
        if (evt%100000 == 0 && outTree->GetEntries() > param.nb_max_evts_in_file) break;
        chain.GetEntry(evt++);
        bool trig = false;

        for (int i = 0; i<event.mult; i++)
        {
          auto const & label = event.labels[i];
          auto const & time = event.times[i];

        #ifdef DSSD_TRIG
          if (isDSSD[label])
          {
            trig = true;
          }
        #endif //DSSD_TRIG

          if (rf.period!=0) event.Times[i] = rf.pulse_ToF(time,param.RF_shift)/_ns;
          if (label == 251)
          {
            event.RFperiod = (time+rand.Uniform(0,1)-rf.last_hit)/1000.;
            rf.period = static_cast<Time>(event.RFperiod);
            rf.last_hit = time+rand.Uniform(0,1);
          }
          else if (label == 252)
          {
            RF_VS_LaBr3->Fill(event.nrjs[i], event.Times[i]);
          }
          else if (isGe[label])
          {
            RF_VS_Ge->Fill(event.nrjs[i], event.Times[i]);
          }
        }

      #ifdef NO_TRIG
        trig = true;
      #endif //NO_TRIGa

      #ifdef COUNT_EVENT
        counter.count_event(event);
      #endif //COUNT_EVENT

      #ifdef M2G1_TRIG
        if (counter.Modules>1 && counter.rawGe>0) trig = true;
      #endif //M2G1_TRIG

        if (trig)
        {
          event.RFtime = rf.last_hit;
          outTree->Fill();
        }
      }// End events loop

      file_nb++;
      std::string outPath = param.outDir+run+"/";
      create_folder_if_none(outPath);
      std::string outName = outPath+run+"_"+std::to_string(file_nb)+".root";

      std::unique_ptr<TFile> file (TFile::Open(outName.c_str(),"recreate"));
      file    -> cd   ();
      RF_VS_LaBr3 -> Write();
      RF_VS_Ge -> Write();
      outTree -> Write();
      file    -> Write();
      file    -> Close();
      print(outName,"written, ",readTimer.TimeSec()," s");
    }// End files loop
    print(run, ":", nb_evts*1.E-6, "Mevts converted at a rate of", 1.E-6*nb_evts/readTimer.TimeSec(), "Mevts/s");
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
    //   chain.GetEntry(next_RF++);
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

// ----------------------------------------------- //
