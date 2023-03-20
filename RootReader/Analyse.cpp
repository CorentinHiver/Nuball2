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
#include "Modules/AnalyseIsomer.hpp"

/*
* USAGE:
*
* rootfile
* -l rootfiles.list
* -d rootDir/ nb_files(default all)
*
*/

void analyse(AnalyseIsomer & ai, Parameters & p);

int main(int argc, char** argv)
{
  // Parameters :
  Parameters p;
  if (!p.setParameters(argc,argv)) return -1;
  if (!p.readParameters("Parameters/analysis.setup")) return -1;

  #if defined (N_SI_120)
  std::string fileID = "ID/index_120.dat";

  #elif defined (N_SI_129)
  std::string fileID = "ID/index_129.dat";

  #endif

  // Initialization :
  size_t m_nb_labels = 0;
    // Initialize Multithreading
  MTObject::Initialize(p.threadsNb());

    // Initialize arrays
  print("Initialization of arrays...");
  g_labelToName = arrayID(fileID);
  m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);

    // Initialization of modules
  AnalyseIsomer ai;
  ai.setParameters(p.get_parameters_ai());
  ai.Initialize();


  Timer totalTime;

  MTObject::parallelise_function(analyse,ai,p);
  ai.Write();
  int totalCounts = p.totalCounter;
  print("Analysis of", totalCounts/1000000.,"Mevts performed in", totalTime()/1000, "s ->", totalCounts/totalTime()/1000, "Mevts/s");
  return 1;
}

void analyse(AnalyseIsomer & ai, Parameters & p)
{
  MTObject::shared_mutex.lock();
  print("Thread n°", MTObject::getThreadIndex(), "begin");
  MTObject::shared_mutex.unlock();
  Sorted_Event event_s;
  std::string rootfile;
  while(p.getNextFile(rootfile))
  {
    print(rootfile);
    Timer timer;
    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "lTn");

    size_t size = tree->GetEntries();
    p.totalCounter+=size;

    for (size_t i = 0; i<size; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      ai.FillSorted(event_s,event);
      ai.FillRaw(event);
    } // End event loop
  } // End files loop
}
