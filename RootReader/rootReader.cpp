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

int main(int argc, char** argv)
{
  //Initialization :
  size_t m_nb_labels = 0;


#if defined (N_SI_120)
  std::string outDir = "120/";
  std::string fileID = "ID/index_120.dat";

#elif defined (N_SI_129)
  std::string outDir = "../Data/129/";
  std::string fileID = "ID/index_129.dat";

#endif

  // Parameters :
  // Parameters p;
  // if (!p.setParameters(argc, argv)) return (-1);
  // p.readParameters();

  std::string  outRoot = "129/Analyse/Isomer/";
  std::string  dataPath = "../Data_129/DSSDFinal/";
  // std::string  outRoot = outDir+"default.root";
  // outRoot=p.files().getListFolders()[0];
  // if(outRoot.back() == '/') outRoot.pop_back();
  // outRoot="129/Analyse/Isomer/"+outRoot;

// Initialise arrays
  g_labelToName = arrayID(fileID);
  m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);

  // EachDetector module : one histogram of each kind for each detector
  // EachDetector ed;
  // ed.setParameters(p.get_ed_setup());
  // ed.Initialise(m_nb_labels);

  AnalyseIsomer ai;
  ai.Initialise();

  FilesManager files;
  // std::string list = "Parameters/list_runs.list";
  std::string list = "Parameters/list_runs_good_pulse.list";
  std::vector<std::string> listRuns = listFileReader(list);
  for (auto const & run : listRuns) files.addFolder(dataPath+run);
  // files.addFolder(dataPath+"run_70");

  // Matrices ma;
  // ma.Initialise();

  // RunCheck rc;
  // rc.Initialise();

  // Pre-sorting code :
  Sorted_Event event_s;
  // ed.setSorted_Event(&event_s);

  Timer totalTime;
  size_t totalEvts;

  // Loop over files :
  std::string rootfile;
  // while(p.files().nextFileName(rootfile))
  while(files.nextFileName(rootfile))
  {
    Timer timer;
    RF_Manager rf;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "lTn");

    auto size = tree->GetEntries();
    totalEvts+=size;
    // ed.SetRF(&rf);
    // rc.SetRF(&rf);

    for (auto i = 0; i<size; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      // ed.Fill(event);
      // ma.FillRaw(event);
      // rc.FillRaw(event);
      // rc.FillSorted(event_s);
      ai.FillSorted(event_s,event);
      ai.FillRaw(event);
    } // End event loop
    auto const & time = timer()/1000; // en s
    print(rmPathAndExt(rootfile), ":", size/1000000., "Mevt treated in", time, "s -> ", size/time/1000000, "Mevts/s");
  } // End files loop

  // ed.Write(outRoot+"_ed.root");
  // ma.Write(outRoot+"_ma.root");
  // rc.Write(outRoot+"_rc.root");
  ai.Write(outRoot+"test_ai.root");
  print("Analysis of",totalEvts,"Mevts performed in", totalTime(), "s ->", totalEvts/totalTime(),"Mevts/s");
  return 1;
}
