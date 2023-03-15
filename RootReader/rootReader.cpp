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

#include "lib/utils.hpp"
#include "lib/Event.hpp"
#include "lib/FilesManager.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"

#include "Modules/EachDetector.hpp"
#include "Modules/Matrices.hpp"
#include "Modules/RunCheck.hpp"

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
  Parameters p;
  if (!p.setParameters(argc, argv)) return (-1);
  p.readParameters();

  std::string  outRoot = outDir+"default.root";
  outRoot=p.files().getListFolders()[0];
  if(outRoot.back() == '/') outRoot.pop_back();
  outRoot=outDir+outRoot;

// Initialize arrays
  g_labelToName = arrayID(fileID);
  m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);

  // EachDetector module : one histogram of each kind for each detector
  EachDetector ed;
  ed.setParameters(p.get_ed_setup());
  // ed.Initialize(m_nb_labels);

  // Matrices ma;
  // ma.Initialize();

  RunCheck rc;
  rc.Initialize();

  // Pre-sorting code :
  Sorted_Event event_s;
  // ed.setSorted_Event(&event_s);

  // Loop over files :
  std::string rootfile;
  while(p.files().nextFileName(rootfile))
  {
    print("Treating",rootfile);
    Event event;
    RF_Manager rf;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    tree -> SetBranchAddress("mult" , &event.mult);
    tree -> SetBranchAddress("label", &event.labels);
    tree -> SetBranchAddress("nrj"  , &event.nrjs);
    tree -> SetBranchAddress("nrj2" , &event.nrj2s);
    tree -> SetBranchAddress("time" , &event.times);

    auto size = tree->GetEntries();

    // ed.SetRF(&rf);
    rc.SetRF(&rf);

    for (auto i = 0; i<size; i++)
    {
      tree->GetEntry(i);
      event_s.setEvent(event);
      // ed.Fill(event);
      // ma.FillRaw(event);
      rc.FillRaw(event);
      rc.FillSorted(event_s);
    }
  }

  // ed.Write(outRoot+"_ed.root");
  // ma.Write(outRoot+"_ma.root");
  rc.Write(outRoot+"_rc.root");
  return 1;
}
