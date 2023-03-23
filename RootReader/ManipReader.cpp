/*
 *
 * The goal of this piece of code is to perform analysis on the data
 * It analyse all the runs together
 *
*/

#define N_SI_129
// #define N_SI_120

#ifdef N_SI_129
#define PARIS
#define FATIMA
#define USD_DSSD
#endif //N_SI_129

#ifdef N_SI_120
#define FATIMA
#endif //N_SI_120

// #define MTTHIST_MONO
#include "../lib/utils.hpp"
#include "../lib/Classes/Event.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTCounter.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"

#include "Modules/EachDetector.hpp"
#include "Modules/Matrices.hpp"
#include "Modules/RunCheck.hpp"
#include "Modules/AnalyseDSSD.hpp"
#include "Modules/AnalyseIsomer.hpp"

#include "../lib/Analyse/HistoAnalyse.hpp"

// void reading(AnalyseDSSD & ad, Parameters & p);
// void reading(RunCheck & rc, Parameters & p);
// void analyse(AnalyseIsomer & ai, Parameters & p);

int main(int argc, char** argv)
{
  // Parameters :
  Parameters p;
  if (!p.setParameters(argc,argv)) return -1;
  if (!p.readParameters("Parameters/analysis.setup")) return -1;
  MTObject::Initialize(p.threadsNb());

  #if defined (N_SI_120)
  std::string fileID = "ID/index_120.dat";

  #elif defined (N_SI_129)
  std::string fileID = "ID/index_129.dat";

  #endif

  // Initialization :
  size_t m_nb_labels = 0;
    // Initialize Multithreading

    // Initialize arrays
  print("Initialization of arrays...");
  g_labelToName = arrayID(fileID);
  m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);


  // Initialize modules

  // AnalyseDSSD ad;
  // ad.setParameters(p.get_parameters_ai());
  // ad.Initialize();
  // MTObject::parallelise_function(reading,ad,p);
  // ad.WriteManip();

  // this -> Write();

  AnalyseIsomer ai;
  // ai.setParameters(p.getParameters("ai"));
  // ai.InitializeManip();
  //
  // MTObject::parallelise_function(ai.run, p);
  ai.launch(p);
  p.printPerformances();

  // ai.setParameters(p.get_parameters_ai());
  // ai.Initialize();
  // MTObject::parallelise_function(analyse,ai,p);
  // ai.Write();
  //
  // RunCheck rc;
  // rc.setParameters(p.getParameters("rc"));
  // rc.Initialize();
  // MTObject::parallelise_function(reading,rc,p);
  // rc.Write();

  p.printPerformances();

  return 1;
}
//
// void AnalyseDSSD::reading(AnalyseDSSD & ad, Parameters & p)
// {
//
//   Sorted_Event event_s;
//   std::string run;
//   while(p.getNextRun(run))
//   {
//     FilesManager files(p.getDataPath()+run+"/");
//     MTList<std::string> list_files = files.getListFiles();
//     std::string rootfile;
//     Timer timer;
//     while(list_files.getNext(rootfile))
//     {
//       print(rootfile);
//       p.totalFilesSize+=size_file(rootfile, "Mo");
//       std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
//       if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
//       std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
//       Event event(tree.get(), "lTn");
//
//       size_t entries = tree->GetEntries();
//       p.totalCounter+=entries;
//
//       for (size_t i = 0; i<entries; i++)
//       {
//         tree->GetEntry(i);
//         ad.FillRaw(event);
//         event_s.sortEvent(event);
//         ad.FillSorted(event_s,event);
//       } // End event loop
//     } // End files loop
//     auto time = timer();
//     print(run, "treated in", time, timer.unit());
//   }
// }

// void reading(RunCheck & rc, Parameters & p)
// {
//   MTObject::shared_mutex.lock();
//   print("Thread n°", MTObject::getThreadIndex(), "begin");
//   MTObject::shared_mutex.unlock();
//   Sorted_Event event_s;
//   std::string run;
//
//   RunCheck rc;
//   rc.setParameters(p);
//   rc.Initialize();
//   while(p.getNextRun(run))
//   {
//     FilesManager files(p.getDataPath()+run+"/");
//     MTList<std::string> list_files = files.getListFiles();
//     std::string rootfile;
//     Timer timer;
//   }
// }

// void analyse(AnalyseIsomer & ai, Parameters & p)
// {
//
//   std::string rootfile;
//   while(p.getNextFile(rootfile))
//   {
//     Timer timer;
//
//     std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
//     if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
//     std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
//     Event event(tree.get(), "lTn");
//
//     size_t events = tree->GetEntries();
//     p.totalCounter+=events;
//
//     auto const & filesize = size_file(rootfile, "Mo");
//     p.totalFilesSize+=filesize;
//
//     for (size_t i = 0; i<events; i++)
//     {
//       tree->GetEntry(i);
//       event_s.sortEvent(event);
//       ai.FillSorted(event_s,event);
//       ai.FillRaw(event);
//     } // End event loop
//     auto const & time = timer();
//     print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
//   } // End files loop
// }
