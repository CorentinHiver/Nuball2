#include "../../lib/utils.hpp"
#include "../../lib/Classes/Event.hpp"

void readTest()
{
  TChain chain("Nuball");
  // chain.Add("129/run_25/*.root");
  // chain.Add("129/run_50/*.root");
  chain.Add("129/run_70/*.root");

  Event event(&chain,"lTnN");

  TH1F* RF_stabilite = new TH1F("jitter VS RF number","jitter VS RF number", 3000000, 0, 3000000);
  TH1F* RF_stabilite = new TH1F("jitter VS time","jitter VS time", 3000000, 0, 3000000);
  int nb = chain.GetEntries();
  int rf_count = 0;
  chain.GetEntry(0);
  Float_t first

  for(size_t evt = 0; evt<nb; evt++)
  {
    chain.GetEntry(evt);
    for (size_t hit = 0; hit<event.size(); hit++)
    {
      if (event.labels[hit] == 251)
      {
        RF_stabilite->SetBinContent(rf_count, event.Times[hit]);
        rf_count++;
      }
    }
  }
  TFile* file = new TFile("RF_stability.root","recreate");
  if (!file) {print("file not written.");return;}
  file -> cd();
  RF_stabilite->Write();
  file -> Write();
  file -> Close();
  print("file written to RF_stability.root")
}
