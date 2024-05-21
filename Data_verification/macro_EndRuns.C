#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

int max_cursor = -1;
std::unordered_set<Label> CloversV2::blacklist = {55, 69, 70, 80, 92, 122, 129, 142, 163};
std::unordered_map<Label, double> CloversV2::maxE_Ge = 
{
  {25, 12600 }, {26, 13600 }, {27, 10500 }, {28, 7500  }, 
  {31, 11500 }, {32, 11400 }, {33, 8250  }, {34, 9000  }, 
  {37, 11000 }, {38, 11100 }, {39, 11500 }, {40, 11300 }, 
  {43, 12600 }, {44, 11900 }, {45, 11550 }, {46, 9200  }, 
  {49, 14300 }, {50, 12800 }, {51, 13500 }, {52, 12400 }, 
  {55, 5500  }, {56, 5800  }, 
                {68, 7100  }, {69, 15500 }, {70, 9500  },
  {73, 11650 }, {74, 11600 }, {75, 11800 }, {76, 11600 }, 
  {79, 11500 }, {80, 8000  }, {81, 18200 },
  {85, 7700  }, {86, 12000 }, {87, 12000 }, {88, 11600 }, 
  {91, 7900  }, {92, 10000 }, {93, 11500 }, {94, 11000 }, 
  {97, 11400 }, {98, 11400 }, {99, 11250 }, {100, 8900 }, 
  {103, 11400 }, {104, 11600 }, {105, 11600 }, {106, 11500 }, 
  {109, 12800 }, {110, 1800  }, {111, 13000 }, {112, 11300 }, 
  {115, 12800 }, {116, 11500 }, {117, 10500 }, {118, 11400 }, 
  {121, 12400 }, {122, 20000 }, {123, 10700 }, {124, 20000 }, 
  {127, 11600 }, {128, 11700 }, {129, 10000 }, {130, 11200 }, 
  {133, 11200 }, {134, 9350  }, {135, 9400  }, {136, 9500  }, 
  {139, 13200 }, {140, 12400 }, {141, 12900 }, {142, 4500  }, 
  {145, 8200  }, {146, 9600  }, {147, 9100  }, {148, 10900 }, 
  {151, 11900 }, {152, 12200 }, {153, 11300 }, {154, 12000 }, 
  {157, 9110  }, {158, 9120  }, {159, 9110  }, {160, 11700 }, 
  {163, 11000 }, {164, 11600 }, {165, 11600 }, {166, 11600 }, 
};
void macro_EndRuns()
{
  Timer timer;
  FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/end_runs_2/");
  MTList MTfiles(files.get());
  MTObject::Initialise(5);
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
      unique_TH1F singles_VS_ring_clover(new TH1F(("singles_VS_ring_clover_"+thread_i_str).c_str(),"singles_VS_ring_clover",10000,0,10000));
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
        for (size_t hit_i=0; hit_i<mult; ++hit_i)
        {
          auto const & clover_i = *(clovers.clean[hit_i]);
          auto const & nrj_i = clover_i.nrj;
          singles->Fill(nrj_i);
          g_time->Fill(nrj_i, double_cast(event.stamp)*1.e-12/3600.);
          if (mult == 1) pure_singles->Fill(nrj_i);
          for (size_t hit_j=hit_i+1; hit_j<clovers.clean.size(); ++hit_j)
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
        for (size_t hit_i=0; hit_i<clovers.Rejected.size(); ++hit_i) rejected->Fill(clovers[clovers.Rejected[hit_i]].nrj);
      }
      std::string out_filename = "data/end_runs/"+file_shortname+".root";
      File Filename(out_filename); Filename.makePath();
      auto output(TFile::Open(Filename.c_str(), "recreate"));
      output->cd();
      test->Write("test", TObject::kOverwrite);
      singles->Write("singles", TObject::kOverwrite);
      singles_VS_ring_clover->Write("singles_VS_ring_clover", TObject::kOverwrite);
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