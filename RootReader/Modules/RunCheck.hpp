#ifndef RUNCHECK_HD
#define RUNCHECK_HD

class RunCheck
{
public:
  RunCheck(){}
  void Initialize();
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event);
  void Write(std::string const & outRoot);

  void SetRF(RF_Manager* rf) {m_rf = rf;}

private:

  // Multiplicity :
    // 1D
  TH1F* m_raw_mult;
  TH1F* m_Ge_crystals_mult;
  TH1F* m_BGO_crystals_mult;
  TH1F* m_LaBr3_crystals_mult;
  TH1F* m_Paris_crystals_mult;
  TH1F* m_Clover_Ge_mult;
  TH1F* m_Clover_BGO_mult;
  TH1F* m_Clover_mult;
  TH1F* m_Clean_Clover_Ge_mult;
  TH1F* m_Veto_Clover_Ge_mult;
  TH1F* m_module_mult;
    // 2D
  TH2F* m_Clean_Ge_VS_Paris_mult;
  TH2F* m_Clean_Ge_VS_Modules_mult;

  // Pulsation :
  RF_Manager *m_rf;
  TH2F* m_each_detector_VS_pulsation;
  TH2F* m_ref_LaBr3_nrj_VS_time;

  // Energy
  TH2F* m_each_detector_energy;
};

void RunCheck::Initialize()
{
  m_raw_mult = new TH1F("Crystals Multiplicity", "Crystals Multiplicity", 50, 0, 50);
  m_Ge_crystals_mult = new TH1F("Ge crystals Multiplicity", "Ge crystals Multiplicity", 50, 0, 50);
  m_BGO_crystals_mult = new TH1F("BGO crystals Multiplicity", "BGO crystals Multiplicity", 50, 0, 50);
  m_LaBr3_crystals_mult = new TH1F("LaBr3 crystals Multiplicity", "LaBr3 crystals Multiplicity", 50, 0, 50);
  m_Paris_crystals_mult = new TH1F("Paris crystals Multiplicity", "Paris crystals Multiplicity", 50, 0, 50);
  m_Clover_Ge_mult = new TH1F("Ge Clover Multiplicity", "Ge Clover Multiplicity", 50, 0, 50);
  m_Clover_BGO_mult = new TH1F("BGO Clover Multiplicity", "BGO Clover Multiplicity", 50, 0, 50);
  m_Clover_mult = new TH1F("Clover Multiplicity", "Clover Multiplicity", 50, 0, 50);
  m_Clean_Clover_Ge_mult = new TH1F("Clean Ge Clover Multiplicity", "Clean Ge Clover Multiplicity", 50, 0, 50);
  m_Veto_Clover_Ge_mult = new TH1F("Veto Ge Clover (garbage) Multiplicity", "Veto Ge Clover (garbage) Multiplicity", 50, 0, 50);
  m_module_mult = new TH1F("Modules Multiplicity", "Modules Multiplicity", 50, 0, 50);
    // 2D
  m_Clean_Ge_VS_Paris_mult = new TH2F("Ge Clover mult VS Paris mult", "Ge Clover mult VS Paris mult",
      50,0,50, 50,0,50);
  m_Clean_Ge_VS_Modules_mult = new TH2F("Ge Clover mult VS Modules mult", "Ge Clover mult VS Modules mult",
      50,0,50, 50,0,50);

  // Pulsation :
  m_each_detector_VS_pulsation = new TH2F("Each detector pulse timing","Each detector pulse timing",
      g_labelToName.size(),0.5,g_labelToName.size()+0.5, 500,-100, 400);
  m_ref_LaBr3_nrj_VS_time = new TH2F("Reference LaBr3 nrj VS time","Reference LaBr3 nrj VS time",
      5000,-100,400, 1000,0,5000);

  // Energy :
  m_each_detector_energy = new TH2F("Each detector nrj","Each detector nrj",
      g_labelToName.size(),0.5,g_labelToName.size()+0.5, 5000,0,2500);

}

void RunCheck::FillRaw(Event const & event)
{
  for (size_t loop = 0; loop<event.size(); loop++)
  {
    auto const & label = event.labels[loop];
    auto const & time = event.times[loop];
    auto const & nrj = event.nrjs[loop];

    if (label == 251)
    {
      m_rf->period = (time-m_rf->last_hit)/1000.;
      m_rf->last_hit = time;
      continue;
    }
    auto const time_pulse = m_rf -> pulse_ToF(time,50000ll)/_ns;
    m_each_detector_VS_pulsation -> Fill(label, time_pulse);

    if (label == 252) m_ref_LaBr3_nrj_VS_time -> Fill(time_pulse, nrj);
  }
}

void RunCheck::FillSorted(Sorted_Event const & event)
{
  m_raw_mult -> Fill(event.RawMult);
  m_Ge_crystals_mult -> Fill(event.RawGeMult);
  // m_BGO_crystals_mult -> Fill(event.RawBGOMult);
  m_LaBr3_crystals_mult -> Fill(event.LaBr3Mult);
  #ifdef PARIS
  m_Paris_crystals_mult -> Fill(event.ParisMult);
  #endif //PARIS
  m_Clover_Ge_mult -> Fill(event.CloverGeMult);
  m_Clover_BGO_mult -> Fill(event.BGOMult);
  m_Clover_mult -> Fill(event.CloverMult);
  m_Clean_Clover_Ge_mult -> Fill(event.CleanGeMult);
  m_Veto_Clover_Ge_mult -> Fill(event.ComptonVetoMult);
  m_module_mult -> Fill(event.ModulesMult);

  m_Clean_Ge_VS_Paris_mult -> Fill(event.ParisMult, event.CleanGeMult);
  m_Clean_Ge_VS_Modules_mult -> Fill(event.ModulesMult, event.CleanGeMult);
}

void RunCheck::Write(std::string const & outRoot)
{
  std::unique_ptr<TFile> outfile(TFile::Open(outRoot.c_str(),"RECREATE"));
  outfile->cd();

  if(m_raw_mult && m_raw_mult->Integral()>1) m_raw_mult -> Write();
  if(m_Ge_crystals_mult && m_Ge_crystals_mult->Integral()>1) m_Ge_crystals_mult -> Write();
  // m_BGO_crystals_mult -> Fill(event.RawBGOMult);
  if(m_LaBr3_crystals_mult && m_LaBr3_crystals_mult->Integral()>1) m_LaBr3_crystals_mult -> Write();
  #ifdef PARIS
  if(m_Paris_crystals_mult && m_Paris_crystals_mult->Integral()>1) m_Paris_crystals_mult -> Write();
  #endif //PARIS
  if(m_Clover_Ge_mult && m_Clover_Ge_mult->Integral()>1) m_Clover_Ge_mult -> Write();
  if(m_Clover_BGO_mult && m_Clover_BGO_mult->Integral()>1) m_Clover_BGO_mult -> Write();
  if(m_Clover_mult && m_Clover_mult->Integral()>1) m_Clover_mult -> Write();
  if(m_Clean_Clover_Ge_mult && m_Clean_Clover_Ge_mult->Integral()>1) m_Clean_Clover_Ge_mult -> Write();
  if(m_Veto_Clover_Ge_mult && m_Veto_Clover_Ge_mult->Integral()>1) m_Veto_Clover_Ge_mult -> Write();
  if(m_module_mult && m_module_mult->Integral()>1) m_module_mult -> Write();

  if(m_Clean_Ge_VS_Paris_mult && m_Clean_Ge_VS_Paris_mult->Integral()>1) m_Clean_Ge_VS_Paris_mult -> Write();
  if(m_Clean_Ge_VS_Modules_mult && m_Clean_Ge_VS_Modules_mult->Integral()>1) m_Clean_Ge_VS_Modules_mult -> Write();

  if(m_each_detector_VS_pulsation && m_each_detector_VS_pulsation->Integral()>1) m_each_detector_VS_pulsation -> Write();
  if(m_ref_LaBr3_nrj_VS_time && m_ref_LaBr3_nrj_VS_time->Integral()>1) m_ref_LaBr3_nrj_VS_time -> Write();



  outfile->Write();
  outfile->Close();
  print("Run check written to ", outRoot);
}


#endif //RUNCHECK_HD
