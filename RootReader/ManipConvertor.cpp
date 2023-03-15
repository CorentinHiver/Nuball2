//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

#define N_SI_129
#define PARIS
#define QDC2
#define USE_DSSD
#define FATIMA
#define DSSD_TRIG

// #define CORENTIN
#define DATA2

#include "../lib/utils.hpp"
#include "../lib/Classes/Event.hpp"
#include "../lib/Classes/FilesManager.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"
#include "Modules/EachDetector.hpp"


struct quick_parameters
{
#ifdef N_SI_129
#if defined (CORENTIN)

    #ifdef DSSD_TRIG
  std::string const outDir = "129/DSSD_TRIG/";
    #else
  std::string const outDir = "129/";
    #endif //DSSD_TRIG
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/runs_pulsed_129.list";
  std::string const dataPath = "~/faster_data/N-SI-129-root/";
  UShort_t nb_threads = 4;
  int const nb_max_evts_in_file = 1000000; // 1 millions evts ~ 40 Mb/fichier

#elif defined (DATA2)

    #ifdef DSSD_TRIG
  std::string const outDir = "129/DSSD_TRIG/";
    #else
  std::string const outDir = "129/";
    #endif //DSSD_TRIG
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/list_runs.list";
  std::string const dataPath = "/srv/data/nuball2/N-SI-129-root/";
  UShort_t nb_threads = 10;
  int const nb_max_evts_in_file = 10000000; // 10 millions evts ~ 400 Mb/fichier

#endif

  std::vector<std::string> runs;
  size_t current_run = 0;


  std::mutex mutex;

  bool getNextRun(std::string & run)
  {
    if (nb_threads>1) mutex.lock();
    if (current_run<runs.size())
    {
      run = runs[current_run];
      current_run++;
      if (nb_threads>1) mutex.unlock();
      return true;
    }
    else
    {
      current_run++;
      if (nb_threads>1) mutex.unlock();
      return false;
    }
  }

  #endif //N_SI_129x
};

void convertRun(quick_parameters & param);
// void run_thread(quick_parameters & param);

int main(int argc, char ** argv)
{
  quick_parameters qp;
  if (argc == 2 && strcmp(argv[1],"-m")==0) qp.nb_threads = 1;
  else if (argc == 3 && strcmp(argv[1],"-m")==0) qp.nb_threads = atoi(argv[2]);

  // Parameters p;
  // p.readParameters();

  // Initialize arrays
  g_labelToName = arrayID(qp.fileID);
  auto m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);
  if (qp.nb_threads>1) TThread::Initialize();

  qp.runs = listFileReader(qp.runs_list);

  std::vector<std::thread> threads;

  checkThreadsNb(qp.nb_threads, qp.runs.size());

  if(qp.nb_threads == 1)
  {
    convertRun(qp);
  }

  else
  {
    for (int i = 0; i<qp.nb_threads; i++)
    {
      // Run in parallel this command :
      //                              vvvvvvvvvvvv
      threads.emplace_back([&qp](){convertRun(qp);});
      //                              ^^^^^^^^^^^^
      //Note : the parameter "this" in the lambda allows all instances of run_thread() to have access to the members of the main NearLine object
    }
    for(size_t i = 0; i < threads.size(); i++) threads.at(i).join(); //Closes threads
    // print("NUMBER HITS : ", m_counter);
    threads.resize(0);
    threads.clear();
    std::cout << "Multi-threading is over !" << std::endl;
  }

  return 1;
}

void convertRun(quick_parameters & param)
{
  std::string run = "";
  while(param.getNextRun(run))
  {
    print(run);
    std::string pathRun = param.dataPath+run;
    TChain chain("Nuball");
    std::string rootFiles = pathRun+"/*.root";
    chain.Add(rootFiles.c_str());

    Event event(&chain);
    // Sorted_Event event_s;

    int nb_evts = chain.GetEntries();
    int evt = 0;
    int file_nb = 0;
    while(evt<nb_evts)
    {// Write in files of more or less the same size
      auto outTree = new TTree("Nuball", "New conversion");
      event.writeTo(outTree);
      while(evt<nb_evts)
      {
        chain.GetEntry(evt);
        if (evt%100000 && outTree->GetEntries() > param.nb_max_evts_in_file) break;
        // if (i%10000) print(i);
        // event_s.sortEvent(event);
        #ifdef DSSD_TRIG
        for (int i = 0; i<event.mult; i++)
        {
          if (isDSSD[event.labels[i]])
          {
            outTree->Fill();
            break;
          }
        }
        #endif //DSSD_TRIG
        evt++;
      }// End events loop

      file_nb++;
      std::string outName = param.outDir+run+"_"+std::to_string(file_nb)+".root";
      auto file = TFile::Open(outName.c_str(),"recreate");
      outTree -> Write();
      file    -> Write();
      file    -> Close();
      delete file;
      print(outName,"written...");
    }// End files loop
  }// End runs loop
}
