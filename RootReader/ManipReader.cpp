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
#include "../lib/Classes/Timewalk.hpp"

#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTCounter.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"

#include "Modules/EachDetector.hpp"
#include "Modules/Matrices.hpp"
#include "Modules/RunCheck.hpp"
#include "Modules/AnalyseDSSD.hpp"
#include "Modules/AnalyseIsomer.hpp"
#include "Modules/TimewalkDSSD.hpp"

#include "../lib/Analyse/HistoAnalyse.hpp"


int main(int argc, char** argv)
{
  // Parameters :
  Parameters p;
  if (!p.setParameters(argc,argv)) return -1;
  if (!p.readParameters("Parameters/analysis.setup")) return -1;
  if (!p.checkParameters()) return -1;

  if (MTObject::getThreadsNb()>p.getRunsList().size())
  {// Set one thread per run, and not per file : 
    MTObject::setThreadsNb(p.getRunsList().size());
    print("Number of threads too large (too few runs to process) ->",
    MTObject::getThreadsNb(), "threads");
  }

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

  RunCheck runs;
  runs.launch(p);

  // TimewalkDSSD td;
  // td.launch(p);

  // AnalyseDSSD ad;
  // ad.launch(p);

  // Matrices ma;
  // ma.launch(p);

  // AnalyseIsomer ai;
  // ai.launch(p);

  p.printPerformances();

  return 1;
}
