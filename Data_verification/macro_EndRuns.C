#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

int max_cursor = -1;
std::unordered_set<Label> CloversV2::blacklist = {55, 69, 70, 80, 92, 122, 142, 163};
std::unordered_map<Label, double> CloversV2::maxE_Ge = {{28, 7500 }, {33, 8250 }, {46, 9000 }, {55, 7500 }, {57 , 6000}, 
                      {68, 7000 }, {71, 9500 }, {85, 7500 }, {91, 8000 }, {134, 8500}, 
                      {135, 8500}, {136, 9000}, {142, 6000}, {145, 8000}, {146, 9000},
                      {147, 9000}, {157, 9000}, {158, 9000}, {159, 9000}};
void macro_EndRuns()
{
  Timer timer;
  FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/end_runs_2/");
  MTList MTfiles(files.get());
  MTObject::parallelise_function([&]()
  {
    std::string filename;
    auto const & thread_i_str = std::to_string(MTObject::getThreadIndex());
    while(MTfiles>>filename)
    {
      std::string file_shortname = rmPathAndExt(filename);
      Nuball2Tree tree(filename);
      Event event(tree);
      CloversV2 clovers;
      unique_TH1F singles(new TH1F(("singles_"+thread_i_str).c_str(),"singles",10000,0,10000));
      unique_TH1F test(new TH1F(("test_"+thread_i_str).c_str(),"test",1000000,0,1.e+8));
      unique_TH1F rejected(new TH1F(("rejected_"+thread_i_str).c_str(),"rejected",10000,0,10000));
      unique_TH1F pure_singles(new TH1F(("pure_singles_"+thread_i_str).c_str(),"pure_singles",10000,0,10000));
      unique_TH2F gg(new TH2F(("gg_"+thread_i_str).c_str(),"gg",4096,0,4096, 4096,0,4096));
      unique_TH2F g_time(new TH2F(("g_time_"+thread_i_str).c_str(),"g_time;[keV];hours",4096,0,4096, 10000,0,3));
      unique_TH2F ggC2(new TH2F(("ggC2_"+thread_i_str).c_str(),"ggC2",4096,0,4096, 4096,0,4096));
      unique_TH2F g_VS_sumC2(new TH2F(("g_VS_sumC2_"+thread_i_str).c_str(),"g_VS_sumC2",4096,0,2*4096, 4096,0,4096));
      unique_TH2F ggC3(new TH2F(("ggC3_"+thread_i_str).c_str(),"ggC3",4096,0,4096, 4096,0,4096));
      while(tree.readNext())
      {
        if (tree.cursor()%(int)(1.e+6) == 0) 
        {
          printC(nicer_double(tree.cursor(), 0), "hits");
          if (max_cursor>0 && tree.cursor() > max_cursor) break;
        }
        test->SetBinContent(tree.cursor()/1000, event.stamp);
        clovers = event;
        auto const & mult = clovers.GeClean.size();
        for (int hit_i=0; hit_i<mult; ++hit_i)
        {
          auto const & clover_i = *(clovers.clean[hit_i]);
          auto const & nrj_i = clover_i.nrj;
          singles->Fill(nrj_i);
          g_time->Fill(nrj_i, double_cast(event.stamp)*1.e-12/3600.);
          if (mult == 1) pure_singles->Fill(nrj_i);
          for (int hit_j=hit_i+1; hit_j<clovers.clean.size(); ++hit_j)
          {
            auto const & clover_j = *(clovers.clean[hit_j]);
            auto const & nrj_j = clover_j.nrj;
            gg->Fill(nrj_i, nrj_j);
            gg->Fill(nrj_j, nrj_i);
            if (mult == 2)
            {
              ggC2->Fill(nrj_i, nrj_j);
              ggC2->Fill(nrj_j, nrj_i);
              g_VS_sumC2->Fill(nrj_i+nrj_j, nrj_i);
              g_VS_sumC2->Fill(nrj_i+nrj_j, nrj_j);
            }
            if (mult == 3)
            {
              ggC3->Fill(nrj_i, nrj_j);
              ggC3->Fill(nrj_j, nrj_i);
            }
          }
        }
        for (int hit_i=0; hit_i<clovers.Rejected.size(); ++hit_i) rejected->Fill(clovers[clovers.Rejected[hit_i]].nrj);
      }
      std::string out_filename = "data/end_runs/"+file_shortname+".root";
      File Filename(out_filename); Filename.makePath();
      auto output(TFile::Open(Filename.c_str(), "recreate"));
      output->cd();
      test->Write("test", TObject::kOverwrite);
      singles->Write("singles", TObject::kOverwrite);
      g_time->Write("g_time", TObject::kOverwrite);
      pure_singles->Write("pure_singles", TObject::kOverwrite);
      rejected->Write("rejected", TObject::kOverwrite);
      gg->Write("gg", TObject::kOverwrite);
      ggC2->Write("ggC2", TObject::kOverwrite);
      g_VS_sumC2->Write("g_VS_sumC2", TObject::kOverwrite);
      ggC3->Write("ggC3", TObject::kOverwrite);
      output->Close();
      print(out_filename, "written");
    }
  });
  print(timer());
}

#ifndef __CINT__
int main(int argc, char** argv)
{
  if (argc == 2) max_cursor = int_cast(std::stod(argv[1]));
  macro_EndRuns();
  return 1;
}
#endif //__CINT__
// g++ -g -o macro_EndRuns macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o macro_EndRuns macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17