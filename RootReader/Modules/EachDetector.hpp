#ifndef EACHDETECTOR_H
#define EACHDETECTOR_H

#include "/home/corentin/Onedrive/FASTER/NearLine3/NearLine-lib/Classes/Sorted_Event.hpp"

class EachDetector
{
public:
  EachDetector(){};
  EachDetector(std::string const & parameters){setParameters(parameters);}
  EachDetector(size_t nb_labels) {this->Initialize(nb_labels);}
  ~EachDetector()
  {
    if (Ge_spectra) delete Ge_spectra;
    if (Ge_spectra_prompt) delete Ge_spectra_prompt;
    if (Ge_spectra_delayed) delete Ge_spectra_delayed;

    if (BGO_spectra) delete BGO_spectra;
    if (BGO_spectra_prompt) delete BGO_spectra_prompt;
    if (BGO_spectra_delayed) delete BGO_spectra_delayed;

    if (LaBr3_spectra) delete LaBr3_spectra;
    if (LaBr3_spectra_prompt) delete LaBr3_spectra_prompt;
    if (LaBr3_spectra_delayed) delete LaBr3_spectra_delayed;

    if (each_times) delete each_times;
    if (each_Ge_spectra) delete each_Ge_spectra;
    if (each_BGO_spectra) delete each_BGO_spectra;
    if (each_LaBr3_spectra) delete each_LaBr3_spectra;

    for (auto histo : times) if (histo) delete histo;
    for (auto histo : spectra) if (histo) delete histo;
    for (auto histo : spectra2) if (histo) delete histo;
    for (auto histo : preprompt) if (histo) delete histo;
    for (auto histo : prompt) if (histo) delete histo;
    for (auto histo : delayed) if (histo) delete histo;
    for (auto histo : times_nrj) if (histo) delete histo;
  }

  void Initialize(size_t nb_labels);
  void Fill(Event const & event);
  void SetRF(RF_Manager* rf){m_rf = rf;}
  void Write(std::string const & outRoot);
  bool setParameters(std::string const & parameters);

  void setSorted_Event(Sorted_Event* arg) {m_arg = arg;};

private:

  TH1F* Ge_spectra = nullptr;
  TH1F* Ge_spectra_prompt = nullptr;
  TH1F* Ge_spectra_delayed = nullptr;

  TH1F* BGO_spectra = nullptr;
  TH1F* BGO_spectra_prompt = nullptr;
  TH1F* BGO_spectra_delayed = nullptr;

  TH1F* LaBr3_spectra = nullptr;
  TH1F* LaBr3_spectra_prompt = nullptr;
  TH1F* LaBr3_spectra_delayed = nullptr;

  TH2F* each_times = nullptr;
  TH2F* each_BGO_spectra = nullptr;
  TH2F* each_Ge_spectra = nullptr;
  TH2F* each_LaBr3_spectra = nullptr;

  std::vector<TH1F*> times, spectra, spectra2, preprompt, prompt, delayed;
  std::vector<TH2F*> times_nrj;
  RF_Manager *m_rf = nullptr;

  Float_t preprompt_Ge = -15;Float_t preprompt_BGO = -25;  Float_t preprompt_LaBr3 = -15;
  Float_t prompt_Ge = 20; Float_t prompt_BGO = 20;  Float_t prompt_LaBr3 = 10;
  bool time_nrj_bidim = false;
  size_t m_nb_labels = 0;

  int m_bins_Ge = 0; Float_t m_min_Ge = 0; Float_t m_max_Ge = 0;
  int m_bins_BGO = 0; Float_t m_min_BGO = 0; Float_t m_max_BGO = 0;
  int m_bins_LaBr3 = 0; Float_t m_min_LaBr3 = 0; Float_t m_max_LaBr3 = 0;

  Sorted_Event* m_arg = nullptr;
};

bool EachDetector::setParameters(std::string const & parameters)
{
  std::string temp;
  std::istringstream is(parameters);
  while(is >> temp)
  {
         if (temp == "BINNING:")
    {
      is >> temp;
           if (temp == "Ge:")    is >> m_bins_Ge    >> m_min_Ge    >> m_max_Ge   ;
      else if (temp == "BGO:")   is >> m_bins_BGO   >> m_min_BGO   >> m_max_BGO  ;
      else if (temp == "LaBr3:") is >> m_bins_LaBr3 >> m_min_LaBr3 >> m_max_LaBr3;
      else {print("Parameter BINNING:", temp, "non reconnu !"); return false;}
    }
    else if (temp == "TIMING_GATES:")
    {
      is >> temp;
           if (temp == "Ge:")    is >> preprompt_Ge    >> prompt_Ge   ;
      else if (temp == "BGO:")   is >> preprompt_BGO   >> prompt_BGO  ;
      else if (temp == "LaBr3:") is >> preprompt_LaBr3 >> prompt_LaBr3;
      else {print("Parameter TIMING_GATES:", temp, "non reconnu !"); return false;}
    }
  }
  return true;
}

void EachDetector::Initialize(size_t nb_labels)
{
  m_nb_labels = nb_labels;

  times.resize(nb_labels);
  spectra.resize(nb_labels, nullptr);
  spectra2.resize(nb_labels, nullptr);
  preprompt.resize(nb_labels, nullptr);
  prompt.resize(nb_labels, nullptr);
  delayed.resize(nb_labels, nullptr);
  if (time_nrj_bidim) times_nrj.resize(nb_labels, nullptr);

  Ge_spectra = new TH1F("Raw Ge Spectra", "Raw Ge Spectra", 12000,0,6000);
  Ge_spectra_prompt = new TH1F("Prompt Ge Spectra", "Prompt Ge Spectra", 12000,0,6000);
  Ge_spectra_delayed = new TH1F("Delayed Ge Spectra", "Delayed Ge Spectra", 12000,0,6000);

  BGO_spectra = new TH1F("Raw BGO Spectra", "Raw BGO Spectra", 600,0,6000);
  BGO_spectra_prompt = new TH1F("Prompt BGO Spectra", "Prompt BGO Spectra", 600,0,6000);
  BGO_spectra_delayed = new TH1F("Delayed BGO Spectra", "Delayed BGO Spectra", 600,0,6000);

  LaBr3_spectra = new TH1F("Raw LaBr3 Spectra", "Raw LaBr3 Spectra", 3000,0,6000);
  LaBr3_spectra_prompt = new TH1F("Prompt LaBr3 Spectra", "Prompt LaBr3 Spectra", 3000,0,6000);
  LaBr3_spectra_delayed = new TH1F("Delayed LaBr3 Spectra", "Delayed LaBr3 Spectra", 3000,0,6000);

  each_times = new TH2F("Times", "Times", nb_labels,0.5,nb_labels+0.5, 500, -100, 400);
  each_Ge_spectra = new TH2F("each Ge spectra", "each Ge spectra", 166,0.5,166.5, 12000,0,6000);
  each_BGO_spectra = new TH2F("each BGO spectra", "each BGO spectra", 166,0.5,166.5, 600,0,6000);

#if defined (N_SI_120)
  each_LaBr3_spectra = new TH2F("each LaBr3 spectra", "each LaBr3 spectra", 19,200,219, 3000,0,6000);
#elif defined (N_SI_129)
  each_LaBr3_spectra = new TH2F("each Paris spectra", "each Paris spectra", 412,300.5,712.5, 3000,0,6000);
#endif

  for (size_t label = 0; label<nb_labels; label++)
  {
    std::string det =  g_labelToName[label];
    std::string name;
    if (det=="") continue;
    name = "Relative timestamp for  " + det;
    if (isLaBr3[label] || isParis[label]) times[label]=new TH1F(name.c_str(), name.c_str(), 5000, -100, 400);
    else times[label]=new TH1F(name.c_str(), name.c_str(), 500, -100, 400);
    if (time_nrj_bidim) {
      name = "E VS dT for "+ det;
      times_nrj[label]=new TH2F(name.c_str(), name.c_str(), 500,-100,400, 4990,10,10000);
    }
    if (isDSSD[label])
    {
      name = "Energy spectra for " + det;
      spectra[label]=new TH1F(name.c_str(), name.c_str(), 1999,10,20000);
      name = "Energy spectra QDC2 for " + det;
      spectra2[label]=new TH1F(name.c_str(), name.c_str(), 1999,10,20000);
      name = "Preprompt energy spectra for  " + det;
      preprompt[label]=new TH1F(name.c_str(), name.c_str(), 1999,10,20000);
      name = "Prompt energy spectra for  " + det;
      prompt[label]=new TH1F(name.c_str(), name.c_str(), 1999,10,20000);
      name = "Delayed energy spectra for  " + det;
      delayed[label]=new TH1F(name.c_str(), name.c_str(), 1999,10,20000);
    }
    else
    {
      name = "Energy spectra for  " + det;
      spectra[label]=new TH1F(name.c_str(), name.c_str(), 4990,10,5000);
      name = "Energy spectra QD2 for  " + det;
      spectra2[label]=new TH1F(name.c_str(), name.c_str(), 4990,10,5000);
      name = "Preprompt energy spectra for  " + det;
      preprompt[label]=new TH1F(name.c_str(), name.c_str(), 4990,10,5000);
      name = "Prompt energy spectra for  " + det;
      prompt[label]=new TH1F(name.c_str(), name.c_str(), 4990,10,5000);
      name = "Delayed energy spectra for  " + det;
      delayed[label]=new TH1F(name.c_str(), name.c_str(), 4990,10,5000);

    }
  }
}

void EachDetector::Fill(Event const & event)
{
  for (auto loop = 0; loop<event.mult; loop++)
  {
    auto const & label = event.labels[loop];
    if (label == 251)
    {
      m_rf->last_hit = event.times[loop];
      m_rf->period = event.nrjs[loop];
      continue;
    }
    else if (label>m_nb_labels) continue;

    auto const time = m_rf->pulse_ToF(event.times[loop],50000ull)/_ns;
    auto nrj = event.nrjs[loop];
    auto nrj2 = event.nrj2s[loop];

    each_times -> Fill(label, time);
    times[label] -> Fill(time);
    if(time_nrj_bidim)times_nrj[label] -> Fill(time,nrj);


    spectra[label] -> Fill(nrj);
    if (nrj2) spectra2[label] -> Fill(nrj2);
    if (isBGO[label])
    {
      BGO_spectra->Fill(nrj);
      each_BGO_spectra->Fill(label,nrj);
      if (time<preprompt_BGO)
      {
        preprompt[label] -> Fill(nrj);
      }
      else if (time<prompt_BGO)
      {
        BGO_spectra_prompt->Fill(nrj);
        prompt[label] -> Fill(nrj);
      }
      else
      {
        BGO_spectra_delayed->Fill(nrj);
        delayed[label] -> Fill(nrj);
      }
    }
    else if (isGe[label])
    {
      // Analyse :
      Ge_spectra->Fill(nrj);
      each_Ge_spectra->Fill(  label,nrj);
      if (time<prompt_Ge)
      {
        Ge_spectra_prompt -> Fill(nrj);
        prompt[label] -> Fill(nrj);
      }
      else
      {
        Ge_spectra_delayed -> Fill(nrj);
        delayed[label] -> Fill(nrj);
      }
    }
    else if (isLaBr3[label] || (isParis[label] && (nrj2-nrj)/nrj2 < 0.4 ))
    {
      LaBr3_spectra->Fill(nrj);
      each_LaBr3_spectra->Fill(label,nrj);
      if (time<preprompt_LaBr3)
      {
        preprompt[label] -> Fill(nrj);
      }
      else if (time<prompt_LaBr3)
      {
        LaBr3_spectra_prompt->Fill(nrj);
        prompt[label] -> Fill(nrj);
      }
      else
      {
        LaBr3_spectra_delayed->Fill(nrj);
        delayed[label] -> Fill(nrj);
      }
    }
  }
}

void EachDetector::Write(std::string const & outRoot)
{
  // Write the histograms :
  std::unique_ptr<TFile> outfile(TFile::Open(outRoot.c_str(),"RECREATE"));
  outfile->cd();
  if (Ge_spectra && Ge_spectra->Integral()>1) Ge_spectra -> Write();
  if (Ge_spectra_prompt && Ge_spectra_prompt->Integral()>1) Ge_spectra_prompt -> Write();
  if (Ge_spectra_delayed && Ge_spectra_delayed->Integral()>1) Ge_spectra_delayed -> Write();

  if (BGO_spectra && BGO_spectra->Integral()>1) BGO_spectra -> Write();
  if (BGO_spectra_prompt && BGO_spectra_prompt->Integral()>1) BGO_spectra_prompt -> Write();
  if (BGO_spectra_delayed && BGO_spectra_delayed->Integral()>1) BGO_spectra_delayed -> Write();

  if (LaBr3_spectra && LaBr3_spectra->Integral()>1) LaBr3_spectra -> Write();
  if (LaBr3_spectra_prompt && LaBr3_spectra_prompt->Integral()>1) LaBr3_spectra_prompt -> Write();
  if (LaBr3_spectra_delayed && LaBr3_spectra_delayed->Integral()>1) LaBr3_spectra_delayed -> Write();

  if (each_times && each_times->Integral()>1) each_times -> Write();
  if ( each_Ge_spectra && each_Ge_spectra->Integral()>1) each_Ge_spectra -> Write();
  if ( each_BGO_spectra && each_BGO_spectra->Integral()>1) each_BGO_spectra -> Write();
  if ( each_LaBr3_spectra && each_LaBr3_spectra->Integral()>1) each_LaBr3_spectra -> Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (times[i] && times[i]->Integral()>1) times[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if(time_nrj_bidim && times_nrj[i] && times_nrj[i]->Integral()>1) times_nrj[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (spectra[i] && spectra[i]->Integral()>1) spectra[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (spectra2[i] && spectra2[i]->Integral()>1) spectra2[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (preprompt[i] && preprompt[i]->Integral()>1) preprompt[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (prompt[i] && prompt[i]->Integral()>1) prompt[i]->Write();
  for (size_t i = 0; i<m_nb_labels; i++) if (delayed[i] && delayed[i]->Integral()>1) delayed[i]->Write();
  outfile->Write();
  outfile->Close();
  print("Each detector spectra file written to", outRoot);
}

#endif //EACHDETECTOR_H
