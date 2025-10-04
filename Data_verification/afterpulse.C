#include "../lib/libRoot.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Analyse/CloversV2.hpp"

void afterpulse()
{
  Nuball2Tree tree("~/nuball2/after_pulse_2/after_pulse_2.root");
  if (!tree) {error("No tree"); return;}

  unique_TH1F singles(new TH1F("singles", "singles", 10000,0,10000));
  unique_TH2F gg(new TH2F("gg", "gamma-gamma", 10000,0,10000, 10000,0,10000));
  unique_TH2F ggC2(new TH2F("ggC2", "gamma-gamma C2", 10000,0,10000, 10000,0,10000));
  unique_TH2F gg_VS_sum_C2(new TH2F("ggC2", "gamma-gamma C2", 10000,0,20000, 10000,0,10000));

  Event event(tree, "ltEQ");
  CloversV2 clovers;
  while(tree.readNext())
  {
    if (tree.cursor()%(int)(1.e7) == 0) print(Colib::nicer_double(tree.cursor(), 0));
    clovers = event;
    auto const & multC = clovers.clean.size();
    if (multC == 1) singles -> Fill(clovers.clean[0]->nrj);
    else
    {
      for (size_t loop_i = 0; loop_i<clovers.clean.size(); ++loop_i)
      {
        auto const & clover_i = clovers.clean[loop_i];
        if (clover_i.nrj<5) continue;
        for (size_t loop_j = loop_i+1; loop_j<clovers.clean.size(); ++loop_j)
        {
          if (clover_j.nrj<5) continue;
          auto const & clover_j = clovers.clean[loop_j];
          gg->Fill(clover_i->nrj, clover_j->nrj);
          gg->Fill(clover_j->nrj, clover_i->nrj);
          if (multC == 2)
          {
            ggC2->Fill(clover_i->nrj, clover_j->nrj);
            ggC2->Fill(clover_j->nrj, clover_i->nrj);
            
            gg_VS_sum_C2->Fill()
          }
        }
      }
    }
  }

  auto file(TFile::Open("data/afterpulse.root", "recreate"));
  if(!file) {error("data/afterpulse.root can't be open"); return;}
  file->cd();
  singles->Write();
  gg->Write();
  ggC2->Write();
  file->Close();
  print("data/afterpulse.root written");
}

#ifndef __CINT__
int main()
{
  afterpulse();
  return 1;
}
#endif //__CINT__

// g++ -g -o afterpulse afterpulse.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o afterpulse afterpulse.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17