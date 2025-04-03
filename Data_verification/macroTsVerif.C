#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"

int max_cursor = -1;
void macroTsVerif()
{
  std::string trigger = "P";
  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), -1);
  MTList MTfiles(files.get());
  MTObject::Initialise(10);
  MTObject::adjustThreadsNumber(files.size());
  std::string outfile = "data/"+trigger+"_ts_verif.root";
  auto outFile(TFile::Open(outfile.c_str(), "recreate"));
  std::mutex write_mutex;
  MTObject::parallelise_function([&](){
    std::string file;
    while(MTfiles.getNext(file))
    {
      auto const & filename = removePath(file);
      auto const & run_name = removeExtension(filename);
      // auto const & run_number = std::stoi(split(run_name, '_')[1]);
      Nuball2Tree tree(file);
      Event event(tree, "lTE");
      unique_TH2F dT_label (new TH2F(("dT_label_"+run_name).c_str(),( "dT VS label clean "+run_name).c_str(), 1000,0,1000, 600,-100_ns,200_ns));
      unique_TH2F dT_label_5110 (new TH2F(("dT_label_5110_"+run_name).c_str(),( "dT 5110keV VS label clean "+run_name).c_str(), 1000,0,1000, 600,-100_ns,200_ns));
      dT_label->SetDirectory(nullptr);
      
      while(tree.readNext())
      {
        if (max_cursor>0 && tree.cursor() > max_cursor) break;
        if (tree.cursor()%(int)(1.e+7) == 0) printC(nicer_double(tree.cursor(), 0), "hits");
        for (int hit_i = 0; hit_i<event.mult; ++hit_i)
        {
          auto const & label = event.labels[hit_i];
          auto const & time = event.times[hit_i];
          dT_label->Fill(label, time);
          auto const & nrj = event.nrjs[hit_i];
          if (5090<nrj && nrj<5120) dT_label_5110->Fill(label, time);
        }
      }
      outFile->cd();
      dT_label_5110->Write();
    }
  });
  outFile->Close();
  print(outfile, "written");
}

#ifndef __CINT__
int main(int argc, char** argv)
{
  if (argc == 2) max_cursor = int_cast(std::stod(argv[1]));
  macroTsVerif();
  return 1;
}
#endif //__CINT__
// g++ -g -o exec macroTsVerif.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macroTsVerif.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17