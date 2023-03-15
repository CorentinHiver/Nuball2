//g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17

#define N_SI_129
#define PARIS
#define QDC2
#define USE_DSSD
#define FATIMA
#define DSSD_TRIG

#include "../lib/utils.hpp"
#include "../lib/Classes/Event.hpp"
#include "../lib/Classes/FilesManager.hpp"

Labels g_labelToName;

#include "Classes/Parameters.hpp"
#include "Modules/EachDetector.hpp"


int main(int argc, char ** argv)
{
  #ifdef N_SI_129
  std::string outDir = "129/";
  std::string fileID = "ID/index_129.dat";
  std::string runs_list = "Parameters/runs_pulsed_129.list";
  std::string dataPath = "~/faster_data/N-SI-129-root/";

  int nb_max_evts_in_file = 10000000; // 10 millions evts ~ 400 Mb
  #endif //N_SI_129x

  // Parameters p;
  // p.readParameters();

  // Initialize arrays
  g_labelToName = arrayID(fileID);
  auto m_nb_labels = g_labelToName.size();
  setArrays(m_nb_labels);

  auto runs = listFileReader(runs_list);
  for (auto const & run : runs)
  {
    std::string pathRun = dataPath+run;
    TChain chain("Nuball");
    std::string rootFiles = pathRun+"/*.root";
    chain.Add(rootFiles.c_str());

    Event event(&chain);
    Sorted_Event event_s;

    int nb_evts = chain.GetEntries();
    int nb_files = nb_evts/nb_max_evts_in_file+1;

    for (int file_nb = 0; file_nb<nb_files; file_nb ++)
    {// Write in files of more or less the same size
      auto outTree = new TTree("Nuball", "New conversion");
      event.writeTo(outTree);
      for (int i = 0; i<nb_max_evts_in_file; i++)
      {
        if (i>nb_evts) break; // To write the last file, we have to stop before the max evts number
        chain.GetEntry(i);
        event_s.sortEvent(event);
        #ifdef DSSD_TRIG
        if (event_s.DSSDMult > 0) outTree->Fill();
        #endif //DSSD_TRIG        
      }// End events loop
      std::string outName = outDir+run+"_"+std::to_string(file_nb+1)+".root";
      std::unique_ptr<TFile> file(TFile::Open(outName.c_str(),"recreate"));
      outTree -> Write();
      file    -> Write();
      file    -> Close();
      print(outName,"written...");
    }// End files loop
  }// End runs loop

  return 1;
}
