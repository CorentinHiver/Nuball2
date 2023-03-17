#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}

  bool setParameters(std::string const & parameters);
  void Initialize();
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write(std::string const & outRoot);

private:

  Sorted_Event *m_s_event = nullptr;

  TH1F* Ge_spectra;
  TH1F* Ge_spectra_prompt;
  TH1F* Ge_spectra_delayed;
  TH2F* GeDelayed_VS_GeDelayed;
  TH2F* Ge_spectra_VS_Time;
  std::vector<TH2F> Ge_VS_each_channel_DSSD;
};

bool AnalyseIsomer::setParameters(std::string const & parameters)
{
  return true;
}

void AnalyseIsomer::Initialize()
{
  Ge_spectra = new TH1F("Ge spectra","Ge spectra", 14000,0,7000);
  Ge_spectra_delayed = new TH1F("Ge spectra delayed","Ge spectra delayed", 14000,0,7000);
  Ge_spectra_prompt = new TH1F("Ge spectra delayed","Ge spectra delayed", 14000,0,7000);
  Ge_spectra_VS_Time = new TH1F("Ge spectra VS Time","Ge spectra VS Time", 1000,-100,500, 14000,0,7000);
  GeDelayed_VS_GeDelayed = new TH2F("Ge spectra bidim delayed","Ge spectra bidim delayed", 14000,0,7000, 3500,0,7000);
  for (int i = 0; i<55; i++)
  {
    std::string name = "Ge VS "+g_labelToName[i];
    Ge_VS_each_channel_DSSD[i] = new TH1F(name.c_str(), name.c_str(), 14000,0,7000, 1000,0,20000);
  }
}

void AnalyseIsomer::FillRaw(Event const & event)
{

}

void AnalyseIsomer::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
  {
    auto const & label_i = event_s.clover_hits[loop_i];
    auto const & nrj_i = event_s.nrj_clover[loop_i];
    if (event_s.BGO[label_i] || nrj_i<5) continue;
    Ge_spectra->Fill(nrj_i);
    for (size_t loop_j = loop_i+1; loop_j<event_s.clover_hits.size(); loop_j++)
    {
      auto const & label_j = event_s.clover_hits[loop_j];
      auto const & nrj_j = event_s.nrj_clover[loop_j];
      if (event_s.BGO[label_j] || nrj_j<5) continue;
      if (event_s.delayed_Ge[label_i] && event_s.delayed_Ge[label_j])
      {
        GeDelayed_VS_GeDelayed -> Fill(nrj_i,nrj_j);
        GeDelayed_VS_GeDelayed -> Fill(nrj_j,nrj_i);
      }
    }
    for (auto const & dssd : event_s.DSSD_hits)
    {
      const & dssd_nrj = event.nrjs[dssd];
      const & dssd_nrj = event.nrjs[dssd];
      Ge_VS_each_channel_DSSD[]
    }
  }
}

void AnalyseIsomer::Write(std::string const & outRoot)
{
  std::unique_ptr<TFile> oufile(TFile::Open(outRoot.c_str(),"recreate"));
  Ge_spectra->Write();
  GeDelayed_VS_GeDelayed->Write();
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outRoot);
  if(Ge_spectra) delete Ge_spectra;
  if (GeDelayed_VS_GeDelayed) delete GeDelayed_VS_GeDelayed;
}
#endif //ANALYSEISOMER_H
