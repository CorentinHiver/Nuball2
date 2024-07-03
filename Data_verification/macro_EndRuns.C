#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
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

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  // Paris::InitialiseArrays();

  FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/end_runs_2/");
  MTList MTfiles(files.get());
  MTObject::Initialise(10);
  MTObject::adjustThreadsNumber(files.size());
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
      SimpleParis paris(&calibPhoswitches);

      unique_TH1F singles(new TH1F(("singles_"+thread_i_str).c_str(),"singles",10000,0,10000));
      unique_TH2F singles_VS_ring_clover(new TH2F(("singles_VS_ring_clover_"+thread_i_str).c_str(),"singles_VS_ring_clover", 10,-2,8, 10000,0,10000));
      unique_TH1F test(new TH1F(("test_"+thread_i_str).c_str(),"test",1000000,0,1.e+8));
      unique_TH1F rejected(new TH1F(("rejected_"+thread_i_str).c_str(),"rejected",10000,0,10000));
      unique_TH1F pure_singles(new TH1F(("pure_singles_"+thread_i_str).c_str(),"pure_singles",10000,0,10000));
      unique_TH2F gg(new TH2F(("gg_"+thread_i_str).c_str(),"gg",4096,0,4096, 4096,0,4096));
      unique_TH2F g_time(new TH2F(("g_time_"+thread_i_str).c_str(),"g_time;[keV];hours",4096,0,4096, 10000,0,3));
      unique_TH2F sumC2_time(new TH2F(("sumC2_time_"+thread_i_str).c_str(),"sumC2_time;[keV];hours",4096,0,4096, 10000,0,3));
      unique_TH2F ggC2(new TH2F(("ggC2_"+thread_i_str).c_str(),"ggC2",4096,0,4096, 4096,0,4096));
      unique_TH2F g_VS_sumC2(new TH2F(("g_VS_sumC2_"+thread_i_str).c_str(),"g_VS_sumC2",4096,0,2*4096, 4096,0,4096));
      unique_TH2F ggC3(new TH2F(("ggC3_"+thread_i_str).c_str(),"ggC3",4096,0,4096, 4096,0,4096));

      // unique_TH1F singles_paris(new TH1F(("singles_paris_"+thread_i_str).c_str(),"singles_paris",10000,0,10000));
      // unique_TH1F mult_paris(new TH1F(("mult_paris_"+thread_i_str).c_str(),"mult_paris",20,0,20));
      // unique_TH1F singles_paris_front(new TH1F(("singles_paris_front_"+thread_i_str).c_str(),"singles_paris_front",10000,0,10000));
      // unique_TH2F singles_paris_front_VS_angle(new TH2F(("singles_paris_front_VS_angle_"+thread_i_str).c_str(),"singles_paris_front_VS_angle",200,-5,15, 10000,0,10000));
      // unique_TH1F mult_paris_front(new TH1F(("mult_paris_front_"+thread_i_str).c_str(),"mult_paris_front",20,0,20));
      // unique_TH1F singles_paris_back(new TH1F(("singles_paris_back_"+thread_i_str).c_str(),"singles_paris_back",10000,0,10000));
      // unique_TH2F singles_paris_back_VS_angle(new TH2F(("singles_paris_back_VS_angle_"+thread_i_str).c_str(),"singles_paris_back_VS_angle",20,-5,15, 10000,0,10000));
      // unique_TH1F mult_paris_back(new TH1F(("mult_paris_back_"+thread_i_str).c_str(),"mult_paris_back",20,0,20));
      // unique_TH2F gg_paris(new TH2F(("gg_paris_"+thread_i_str).c_str(),"gg_paris",5000,0,10000, 5000,0,10000));
      // unique_TH2F gg_paris_front(new TH2F(("gg_paris_front_"+thread_i_str).c_str(),"gg_paris_front",5000,0,10000, 5000,0,10000));
      // unique_TH2F gg_paris_back(new TH2F(("gg_paris_back_"+thread_i_str).c_str(),"gg_paris_back",5000,0,10000, 5000,0,10000));
      // unique_TH2F gg_paris_front_VS_back(new TH2F(("gg_paris_front_VS_back_"+thread_i_str).c_str(),"gg_paris_front_VS_back",5000,0,10000, 5000,0,10000));
      
      // unique_TH2F Ge_VS_paris(new TH2F(("Ge_VS_paris_"+thread_i_str).c_str(),"Ge_VS_paris",5000,0,10000, 5000,0,10000));
      // unique_TH2F Ge_VS_paris_front(new TH2F(("Ge_VS_paris_front_"+thread_i_str).c_str(),"Ge_VS_paris_front",5000,0,10000, 5000,0,10000));
      // unique_TH2F Ge_VS_paris_back(new TH2F(("Ge_VS_paris_back_"+thread_i_str).c_str(),"Ge_VS_paris_back",5000,0,10000, 5000,0,10000));

      while(tree.readNext())
      {
        if (tree.cursor()%(int)(1.e+6) == 0) 
        {
          printC(nicer_double(tree.cursor(), 0), "hits");
          if (max_cursor>0 && tree.cursor() > max_cursor) break;
        }
        test->SetBinContent(tree.cursor()/1000, event.stamp);

        clovers = event;
        // paris = event;

        auto const absolute_time_h = double_cast(event.stamp)*1.e-12/3600.;

        auto const & mult = clovers.GeClean.size();
        for (size_t hit_i=0; hit_i<mult; ++hit_i)
        {
          auto const & clover_i = *(clovers.clean[hit_i]);
          auto const & nrj_i = clover_i.nrj;
          singles->Fill(nrj_i);
          g_time->Fill(nrj_i, absolute_time_h);
          singles_VS_ring_clover->Fill(clover_i.sub_ring(), nrj_i);
          if (mult == 1) pure_singles->Fill(nrj_i);
          for (size_t hit_j=hit_i+1; hit_j<clovers.clean.size(); ++hit_j)
          {
            auto const & clover_j = *(clovers.clean[hit_j]);
            auto const & nrj_j = clover_j.nrj;
            gg->Fill(nrj_i, nrj_j);
            gg->Fill(nrj_j, nrj_i);
            if (mult == 2)
            {
              sumC2_time->Fill(nrj_i+nrj_j, absolute_time_h);
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
        // mult_paris->Fill(paris.module_mult());
        // mult_paris_front->Fill(paris.front.module_mult);
        // mult_paris_back->Fill(paris.back.module_mult);

        // for (size_t fr_i = 0; fr_i<paris.front.module_mult; ++fr_i)
        // {
        //   auto const & fr_id_i = paris.front.modules_id[fr_i];
        //   auto const & fr_module_i = paris.front.modules[fr_id_i];
        //   singles_paris->Fill(fr_module_i.nrj);
        //   singles_paris_front->Fill(fr_module_i.nrj);
        //   singles_paris_front_VS_angle->Fill(fr_module_i.angle_to_beam(), fr_module_i.nrj);

        //   for (size_t fr_j = fr_i+1; fr_j<paris.front.module_mult; ++fr_j)
        //   {
        //     auto const & fr_id_j = paris.front.modules_id[fr_j];
        //     auto const & fr_module_j = paris.front.modules[fr_id_j];

        //     gg_paris->Fill(fr_module_i.nrj, fr_module_j.nrj);
        //     gg_paris->Fill(fr_module_j.nrj, fr_module_i.nrj);

        //     gg_paris_front->Fill(fr_module_i.nrj, fr_module_j.nrj);
        //     gg_paris_front->Fill(fr_module_j.nrj, fr_module_i.nrj);
        //   }

        //   for (size_t ba_j = 0; ba_j<paris.back.module_mult; ++ba_j)
        //   {
        //     auto const & ba_id_j = paris.back.modules_id[ba_j];
        //     auto const & ba_module_j = paris.back.modules[ba_id_j];

        //     gg_paris->Fill(fr_module_i.nrj, ba_module_j.nrj);
        //     gg_paris->Fill(ba_module_j.nrj, fr_module_i.nrj);

        //     gg_paris_front_VS_back->Fill(ba_module_j.nrj, fr_module_i.nrj);
        //   }
        // }
        
        // for (size_t ba_i = 0; ba_i<paris.back.module_mult; ++ba_i)
        // {
        //   auto const & ba_id_i = paris.back.modules_id[ba_i];
        //   auto const & ba_module_i = paris.back.modules[ba_id_i];

        //   singles_paris->Fill(ba_module_i.nrj);
        //   singles_paris_back->Fill(ba_module_i.nrj);
        //   singles_paris_back_VS_angle->Fill(ba_module_i.angle_to_beam(), ba_module_i.nrj);

        //   for (size_t ba_j = ba_i+1; ba_j<paris.back.module_mult; ++ba_j)
        //   {
        //     auto const & ba_id_j = paris.back.modules_id[ba_j];
        //     auto const & ba_module_j = paris.back.modules[ba_id_j];

        //     gg_paris->Fill(ba_module_i.nrj, ba_module_j.nrj);
        //     gg_paris->Fill(ba_module_j.nrj, ba_module_i.nrj);

        //     gg_paris_back->Fill(ba_module_i.nrj, ba_module_j.nrj);
        //     gg_paris_back->Fill(ba_module_j.nrj, ba_module_i.nrj);
        //   }
        // }
      
      }

      std::string out_filename = "data/end_runs/"+file_shortname+".root";
      File Filename(out_filename); Filename.makePath();
      auto output(TFile::Open(Filename.c_str(), "recreate"));
      output->cd();

        test->Write("test", TObject::kOverwrite);
        singles->Write("singles", TObject::kOverwrite);
        singles_VS_ring_clover->Write("singles_VS_ring_clover", TObject::kOverwrite);
        g_time->Write("g_time", TObject::kOverwrite);
        sumC2_time->Write("sumC2_time", TObject::kOverwrite);
        pure_singles->Write("pure_singles", TObject::kOverwrite);
        rejected->Write("rejected", TObject::kOverwrite);
        gg->Write("gg", TObject::kOverwrite);
        ggC2->Write("ggC2", TObject::kOverwrite);
        g_VS_sumC2->Write("g_VS_sumC2", TObject::kOverwrite);
        ggC3->Write("ggC3", TObject::kOverwrite);

        // singles_paris->Write("singles_paris", TObject::kOverwrite);
        // mult_paris->Write("mult_paris", TObject::kOverwrite);
        // singles_paris_front->Write("singles_paris_front", TObject::kOverwrite);
        // singles_paris_front_VS_angle->Write("singles_paris_front_VS_angle", TObject::kOverwrite);
        // mult_paris_front->Write("mult_paris_front", TObject::kOverwrite);
        // singles_paris_back->Write("singles_paris_back", TObject::kOverwrite);
        // singles_paris_back_VS_angle->Write("singles_paris_back_VS_angle", TObject::kOverwrite);
        // mult_paris_back->Write("mult_paris_back", TObject::kOverwrite);
        // gg_paris->Write("gg_paris", TObject::kOverwrite);
        // gg_paris_front->Write("gg_paris_front", TObject::kOverwrite);
        // gg_paris_back->Write("gg_paris_back", TObject::kOverwrite);
        // gg_paris_front_VS_back->Write("gg_paris_front_VS_back", TObject::kOverwrite);

        // Ge_VS_paris->Write("Ge_VS_paris", TObject::kOverwrite);
        // Ge_VS_paris_front->Write("Ge_VS_paris_front", TObject::kOverwrite);
        // Ge_VS_paris_back->Write("Ge_VS_paris_back", TObject::kOverwrite);

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
// g++ -g -o exec macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17