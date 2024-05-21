#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

int max_cursor = -1;
std::unordered_set<Label> CloversV2::blacklist = {55, 69, 70, 80, 92, 97, 122, 129, 142, 163};
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
  
  
void macro_Afterpulse()
{
  Timer timer;
  Nuball2Tree tree("after_pulse_2.root");
  Event event(tree);
  CloversV2 clovers;
  unique_TH1F singles(new TH1F("singles","singles",10000,0,10000));
  unique_TH2F singles_VS_label(new TH2F("singles_VS_label","singles_VS_label",1000,0,1000, 2000,0,20000));
  unique_TH1F test(new TH1F("test","test",1000000,0,1.e+8));
  unique_TH1F rejected(new TH1F("rejected","rejected",10000,0,10000));
  unique_TH1F pure_singles(new TH1F("pure_singles","pure_singles",10000,0,10000));
  unique_TH2F gg(new TH2F("gg","gg",4096,0,4096, 4096,0,4096));
  unique_TH2F g_time(new TH2F("g_time","g_time",4096,0,4096, 10000,0,3));
  unique_TH2F ggC2(new TH2F("ggC2","ggC2",4096,0,4096, 4096,0,4096));
  unique_TH2F g_VS_sumC2(new TH2F("g_VS_sumC2","g_VS_sumC2",4096,0,2*4096, 4096,0,4096));
  unique_TH2F ggC3(new TH2F("ggC3","ggC3",4096,0,4096, 4096,0,4096));
  while(tree.readNext())
  {
    if (tree.cursor()%(int)(1.e+6) == 0) 
    {
      printC(nicer_double(tree.cursor(), 0), "hits");
      if (max_cursor>0 && tree.cursor() > max_cursor) break;
    }
    test->SetBinContent(tree.cursor(), event.stamp);
    clovers = event;
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {
      auto const & label = event.labels[hit_i];
      auto const & nrj = event.nrjs[hit_i];
      if (CloversV2::isGe(label))
      {
        if (found(CloversV2::maxE_Ge, label) && nrj > CloversV2::maxE_Ge[label]) continue;
        singles_VS_label->Fill(label,nrj);
      }
    }
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
  auto output(TFile::Open("afterpulse_analysed.root", "recreate"));
  output->cd();
  test->Write();
  singles->Write();
  singles_VS_label->Write();
  g_time->Write();
  pure_singles->Write();
  rejected->Write();
  gg->Write();
  ggC2->Write();
  g_VS_sumC2->Write();
  ggC3->Write();
  output->Close();
  print("afterpulse_analysed.root written");
  print(timer());
}

#ifndef __CINT__
int main(int argc, char** argv)
{
  if (argc == 2) max_cursor = int_cast(std::stod(argv[1]));
  macro_Afterpulse();
  return 1;
}
#endif //__CINT__
// g++ -g -o exec macro_Afterpulse.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro_Afterpulse.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17