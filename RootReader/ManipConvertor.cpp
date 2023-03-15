//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

#define N_SI_129
#define PARIS
#define QDC2
#define USE_DSSD
#define FATIMA

#include "lib/utils.hpp"
Labels g_labelToName;
#include "lib/Event.hpp"
#include "lib/FilesManager.hpp"

#include "Classes/Parameters.hpp"
#include "Modules/EachDetector.hpp"


int main(int argc, char ** argv)
{
  #ifdef N_SI_129
  std::string outDir = "../Data/129/";
  std::string fileID = "ID/index_129.dat";
  #endif //N_SI_129

  Parameters p;
  if (!p.setParameters(argc, argv)) return (-1);
  p.readParameters();

  // Initialize arrays
  g_labelToName = arrayID(fileID);
  auto m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);

  TChain chain("Nuball");
  std::string rootfile;
  while(p.files().nextFileName(rootfile)) chain.Add(rootfile.c_str());

  Event event(&chain);
  Sorted_Event event_s;

  auto outTree = new TTree("Nuball", "New conversion");
  event.writeTo(outTree);

  int nb = chain.GetEntries();
  for (int i = 0; i<nb; i++)
  {
    if (i%100000 == 0) print(i);
    chain.GetEntry(i);
    event_s.sortEvent(event);
    if (event_s.DSSDMult > 0)
    outTree->Fill();
  }

  std::unique_ptr<TFile> file(TFile::Open("run.root","recreate"));
  outTree->Write();
  file->Write();
  file -> Close();

  return 1;
}
