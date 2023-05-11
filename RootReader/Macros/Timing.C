#include "../../lib/utils.hpp"
#include "../../lib/Classes/Event.hpp"

void Timing()
{
  TChain chain("Nuball");
  // chain.Add("129/run_25/*.root");
  // chain.Add("129/run_50/*.root");
  // chain.Add("129/run_62/*.root");
  // chain.Add("129/run_70/*.root");
  chain.Add("129/run_70/*.root");

  Event event(&chain,"lTn");
  Time RFtime = 0;
  outTree->SetBranchAdress("RFtime",&RFtime);
  Long64_t RFperiod = 0;
  outTree->SetBranchAdress("RFperiod",&RFperiod);


  TRandom rand(time(0));

  int bins = 200000;

  TH1F* RF_stabilite_graph = new TH1F("jitter VS RF number histo","jitter VS RF number histo", bins, 0, bins);
  TH1F* RF_period = new TH1F("RF period VS RF number","RF period VS RF number", bins, 0, bins);
  TH1F* RF_period_mes = new TH1F("RF period mes VS RF number","RF period VS RF number", bins, 0, bins);
  TH1F* RF_period_mes_calc_graph = new TH1F("RF period mesuree - calculee graph","RF period mesuree - calculee", bins, 0, bins);
  TH1F* RF_period_mes_calc_last_graph = new TH1F("RF period mesuree - calculee last graph","RF period mesuree - calculee last", bins, 0, bins);

  TH1F* RF_stabilite_histo = new TH1F("jitter","jitter", 200, -5, 5);
  TH1F* RF_period_mes_calc_histo = new TH1F("RF period mesuree - calculee histo ","RF period mesuree - calculee", 500, -5, 5);
  TH1F* RF_period_mes_calc_last_histo = new TH1F("RF period mesuree - calculee last histo ","RF period mesuree - calculee last", 500, -5, 5);

  TH1F* refDet = new TH1F("ref det","ref det", 5000, -100, 400);
  TH1F* allSpectroDet = new TH1F("all Spectro det","all Spectro det", 5000, -100, 400);
  TH1F* allDSSD = new TH1F("all DSSD","all DSSD", 5000, -100, 400);
  auto nb = chain.GetEntries();
  int rf_count = 0;
  chain.GetEntry(0);
  ULong64_t last_hit = event.times[0];
  ULong64_t last_period = event.nrjs[0];
  for(size_t evt = 1; evt<nb; evt++)
  {
    chain.GetEntry(evt);
    for (size_t hit = 0; hit<event.size(); hit++)
    {
      if (event.labels[hit] == 251)
      {
        RF_stabilite_graph->SetBinContent(rf_count, event.Times[hit]);
        RF_period->SetBinContent(rf_count, (event.times[hit]-last_hit));
        RF_period_mes->SetBinContent(rf_count, event.nrjs[hit]);
        RF_period_mes_calc_graph->SetBinContent(rf_count, event.nrjs[hit] - (event.times[hit]-last_hit)/1.E3);
        RF_period_mes_calc_last_graph->SetBinContent(rf_count, last_period - (event.times[hit]-last_hit)/1.E3);

        RF_stabilite_histo->Fill(event.Times[hit]);
        RF_period_mes_calc_histo -> Fill(event.nrjs[hit] - (event.times[hit]-last_hit+rand.Uniform(0,1))/1.E3);
        RF_period_mes_calc_last_histo -> Fill(last_period - (event.times[hit]-last_hit+rand.Uniform(0,1))/1.E3);
        last_hit = event.times[hit];
        last_period = event.nrjs[hit];
        rf_count++;
      }
      else if (event.labels[hit] == 252)
      {
        refDet->Fill(event.Times[hit]);
      }
      else if (event.labels[hit] > 799)
      {
        allDSSD->Fill(event.Times[hit]);
      }
      else
      {
        allSpectroDet->Fill(event.Times[hit]);
      }
    }
  }
  TFile* file = new TFile("RF_stability.root","recreate");
  if (!file) {print("file not written.");return;}
  file -> cd();
  RF_stabilite_graph->Write();
  RF_period->Write();
  RF_period_mes->Write();
  RF_period_mes_calc_graph->Write();
  RF_period_mes_calc_last_graph->Write();

  RF_stabilite_histo->Write();
  RF_period_mes_calc_histo->Write();
  RF_period_mes_calc_last_histo->Write();

  refDet->Write();
  allDSSD->Write();
  allSpectroDet->Write();

  file -> Write();
  file -> Close();
  print("file written to RF_stability.root");
}
