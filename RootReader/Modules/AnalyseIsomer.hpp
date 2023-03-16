#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}

  bool setParameters(std::string const & parameters);
  void Initialize();
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event);
  void Write(std::string const & outRoot);

private:

  Sorted_Event *m_s_event = nullptr;

  TH1F* Ge_spectra;
  TH2F* GeDelayed_VS_GeDelayed;
  std::vector<TH2F> Ge_VS_each_channel_DSSD;
};

bool AnalyseIsomer::setParameters(std::string const & parameters)
{
  return true;
}

void AnalyseIsomer::Initialize()
{
  Ge_spectra = new TH1F("Ge spectra","Ge spectra", 14000,0,7000);
  GeDelayed_VS_GeDelayed = new TH2F("Ge spectra bidim delayed","Ge spectra bidim delayed", 14000,0,7000, 3500,0,7000);
}

void AnalyseIsomer::FillRaw(Event const & event)
{

}

void AnalyseIsomer::FillSorted(Sorted_Event const & event)
{
  for (size_t loop_i = 0; loop_i<event.clover_hits.size(); loop_i++)
  {
    auto const & label_i = event.clover_hits[loop_i];
    if (event.BGO[label_i]) continue;
    auto const & nrj_i = event.nrj_clover[loop_i];
    Ge_spectra->Fill(nrj_i);
    for (size_t loop_j = loop_i+1; loop_j<event.clover_hits.size(); loop_j++)
    {
      auto const & label_j = event.clover_hits[loop_j];
      if (event.BGO[label_j] && !event.DSSDPrompt) continue;
      auto const & nrj_j = event.nrj_clover[loop_j];
      if (event.delayed_Ge[label_i] && event.delayed_Ge[label_j])
      {
        GeDelayed_VS_GeDelayed -> Fill(nrj_i,nrj_j);
        GeDelayed_VS_GeDelayed -> Fill(nrj_j,nrj_i);
      }
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
  if(Ge_spectra) delete Ge_spectra;
  if (GeDelayed_VS_GeDelayed) delete GeDelayed_VS_GeDelayed;
}
#endif //ANALYSEISOMER_H
