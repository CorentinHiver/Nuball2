/*
 *
 * The goal of this piece of code is to perform analysis on the data
 * It analyse all the runs together
 *
*/

// Declare the manip name :
// #define N_SI_129
// #define N_SI_120
#define N_SI_136

// Automatic setup of the detectors
#if defined (N_SI_129) || defined (N_SI_136)
  #define PARIS
  #define FATIMA
  #define USD_DSSD
#elif defined (N_SI_120)
  #define FATIMA 
#endif //N_SI_120

// Automatic setup of the RF :
#if defined (N_SI_136)
  #define USE_RF 200
#elif defined (N_SI_129) || defined (N_SI_120)
  #define USE_RF 400
#endif


#include <libRoot.hpp>
#include <Detectors.hpp>

Detectors g_detectors("index_129.list");

#include <Parameters.hpp>
#include <AnalyseRaw.hpp>

// #include <Event.hpp>
// #include <FilesManager.hpp>
// #include <Timewalk.hpp>

// #include <MTList.hpp>
// #include <MTCounter.hpp>
// #include <HistoAnalyse.hpp>

// Include modules
// #include <EachDetector.hpp>
// #include <Matrices.hpp>
// #include <RunCheck.hpp>
// #include <AnalyseDSSD.hpp>
// #include <AnalyseIsomer.hpp>
// #include <TimewalkDSSD.hpp>
// #include <Analyse511.hpp>
// #include <NoPulse.hpp>


int main(int argc, char** argv)
{
  // Parameters :
  Parameters p;
  if (!p.setParameters(argc,argv)) return -1;
  if (!p.readParameters("../Parameters/analysis.setup")) return -1;
  if (!p.checkParameters()) return -1;

  if (MTObject::getThreadsNb()>p.getRunsList().size())
  {// Set one thread per run, and not per file :
    MTObject::setThreadsNb(p.getRunsList().size());
    print("Number of threads too large (too few runs to process) ->",
    MTObject::getThreadsNb(), "threads");
  }

  Sorted_Event::Initialize();
  MTObject::Initialize(p.threadsNb());

  #if defined (N_SI_120)
  std::string fileID = "ID/index_120.dat";

  #elif defined (N_SI_129)
  std::string fileID = "ID/index_129.dat";

  #endif

  // Initialization :

    // Initialize arrays
  print("Initialization of arrays...");

  // NoPulse np;
  // np.launch(p);

  // RunCheck runs;
  // runs.launch(p);

  // TimewalkDSSD td;
  // td.launch(p);

  // AnalyseDSSD ad;
  // ad.launch(p);

  // Analyse511 a511;
  // a511.launch(p);

  // Matrices ma;
  // ma.launch(p);

  // AnalyseIsomer ai;
  // ai.launch(p);

  AnalyseRaw ar;
  ar.launch(p);

  p.printPerformances();

  return 1;
}
