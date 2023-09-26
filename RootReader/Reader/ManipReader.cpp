/*
 *
 * The goal of this piece of code is to perform analysis on the data
 * It analyse all the runs together
 *
*/
#define SAFE
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


#include <Detectors.hpp>
// #include "../../lib/Classes/Detectors.hpp"

#include <libRoot.hpp>
#include <Parameters.hpp>
// #include <AnalyseRaw.hpp>

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
#include <AnalyseIsomer_136.hpp>
#include <CalibrationReader.hpp>
// #include <Calibrator.hpp>
// #include <TimewalkDSSD.hpp>
// #include <Analyse511.hpp>
// #include <NoPulse.hpp>


int main(int argc, char** argv)
{
  
#if defined (N_SI_129) || defined (N_SI_136)
  detectors.load("index_129.list");
#elif defined (N_SI_120)
  detectors.load("index_120.list");
#endif

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

  // Sorted_Event::Initialize();
  MTObject::Initialize(p.threadsNb());

  #if defined (N_SI_120)
  std::string fileID = "ID/index_120.dat";

  #elif defined (N_SI_129) || defined (N_SI_136)
  std::string fileID = "ID/index_129.dat";

  #endif

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

  // for (int run_nb = 25; run_nb<102; run_nb++)
  // {
  //   p.addRun("/home/corentin/faster_data/N-SI-129-root_P_conversion1/run_"+std::to_string(run_nb)+"/");
  // }
  AnalyseIsomer ai;
  ai.launch(p);

  // CalibrationReader cr;
  // cr.launch(p);

  // AnalyseRaw ar;
  // ar.launch(p);

  // debug("Starting here");
  // MTTHist<TH1F> dssd_nrj("dssd_nrj", "dssd_nrj", 1000,0,20000);

  // debug("Histo created");

  // // FilesManager files("/home/corentin/faster_data/N-SI-136-root_P2_conversion1/run_75/");
  // FilesManager files("/home/corentin/faster_data/N-SI-136-root_test2/run_75/");
  // debug("files loaded");
  // std::string filename;
  // while (files.nextFileName(filename))
  // {
  //   Nuball2Tree tree(filename);
  //   if (!tree) continue;
  //   Event event(tree);
  //   DSSD dssd;
  //   auto const & nb_events = tree -> GetEntries();
  //   for (int event_i = 0; event_i<nb_events; event_i++)
  //   {
  //     for (int i = 0; i<event.mult; i++)
  //     {
  //       print(event.mult);
  //       dssd.SetEvent(event);
  //       for (auto const sector : dssd.Sectors) 
  //       {
  //         print(sector.nrj);
  //         dssd_nrj->Fill(sector.nrj);
  //       }
  //     }
  //   }
  // }

  // TFile *outTree(TFile::Open("test.root", "recreate"));
  // outTree -> cd();
  // dssd_nrj->Write();
  // outTree->Write();
  // outTree->Close();
  // delete outTree;

  // p.printPerformances();

  return 1;
}
