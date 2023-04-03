#ifndef PARIS_H
#define PARIS_H
#include "../NearLine.hpp"

class ParisBidim
{
public:
  ParisBidim(NearLine* nearline) : n(nearline){}

  operator bool() {return m_activated;}
  void operator=(bool activate) {m_activated = activate;}

  Bool_t Initialize();
  Bool_t SetConfig(std::istringstream & is);
  void Fill(Hit & hit, RF_Manager const & rf, int const & thread_nb = 0);
  void Write();

private:

  Int_t m_bins = 1000;
  Int_t m_nb_histo = 1;

  //Ratio
  Vector_MTTHist<TH2F> m_long_gate_vs_ratio;
  Vector_MTTHist<TH2F> m_short_gate_vs_ratio;
  Vector_MTTHist<TH2F> m_short_vs_tan_ratio;
  //Long VS short
  Vector_MTTHist<TH2F> m_long_vs_short;
  // Long/short or short/long
  Vector_MTTHist<TH2F> m_short_vs_long_div_short;
  Vector_MTTHist<TH2F> m_short_vs_short_div_long;
  Vector_MTTHist<TH2F> m_short_vs_tan_long_div_short;
  Vector_MTTHist<TH2F> m_short_vs_tan_short_div_long;
  //Calibrated energy vs tof (needs RF and timeshifts data and calibration coefficients)
  Vector_MTTHist<TH2F> m_E_VS_ToF;

  Bool_t m_activated = true;
  NearLine* n = nullptr;

  std::string m_outroot = "paris_bidim.root";
  std::string m_outdir = "";
  Bool_t m_calibrated = false;
};

Bool_t ParisBidim::SetConfig(std::istringstream & is)
{
  std::string temp = "NULL";
  while (is>>temp)
  {
         if (temp == "outroot:" || temp == "outRoot:"|| temp == "OutRoot:") is >> m_outroot;
    else if (temp == "calibrated")
    {
      m_calibrated = true;
      if(!n->m_calib){print("NO CALIBRATION DATA FOR PARIS BIDIM !! remove \"calibration\" parameter"); return false;}
      print("Output calibrated data");
    }
    else if (temp == "bins:")
    {
      is >> m_bins;
    }
  }
  return true;
}

Bool_t ParisBidim::Initialize()
{
  m_nb_histo = n->m_labelToName.size();

  m_long_gate_vs_ratio.resize(m_nb_histo);
  m_short_gate_vs_ratio.resize(m_nb_histo);
  m_short_vs_tan_ratio.resize(m_nb_histo);

  m_long_vs_short.resize(m_nb_histo);

  m_short_vs_long_div_short.resize(m_nb_histo);
  m_short_vs_short_div_long.resize(m_nb_histo);
  m_short_vs_short_div_long.resize(m_nb_histo);
  m_short_vs_tan_long_div_short.resize(m_nb_histo);
  m_short_vs_tan_short_div_long.resize(m_nb_histo);

  m_E_VS_ToF.resize(m_nb_histo);

  std::string name;
  for (auto l : paris_labels)
  {
    name = n->m_labelToName[l];
    if (name!="")
    {
      // Ratio
      m_long_gate_vs_ratio[l].reset((name+"_long_VS_ratio").c_str(), (name+" long gate VS gates ratio").c_str(),
          100,-0.5,1.5, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      m_short_gate_vs_ratio[l].reset((name+"_short_VS_ratio").c_str(), (name+" short gate VS gates ratio").c_str(),
          100,-0.5,1.5, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      m_short_vs_tan_ratio[l].reset((name+"_short_VS_tan(ratio)").c_str(), (name+" short VS #tan(ratio)").c_str(),
          500,-10,10, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);

      //Long VS short
      m_long_vs_short[l].reset((name+"_long_VS_short").c_str(), (name+" long VS short").c_str(),
          n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris], n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);

      // Long/short OR short/long
      m_short_vs_long_div_short[l].reset((name+"_short_VS_long/short").c_str(), (name+" short VS #frac{long}{short}").c_str(),
          500,-10,10, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      m_short_vs_short_div_long[l].reset((name+"_short_VS_short/long").c_str(), (name+" short VS #frac{short}{long}").c_str(),
          500,-10,10, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      m_short_vs_tan_long_div_short[l].reset((name+"_short_VS_tan(long/short)").c_str(), (name+" short VS tan(long/short)").c_str(),
          500,-10,10, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      m_short_vs_tan_short_div_long[l].reset((name+"_short_VS_tan(short/long)").c_str(), (name+" short VS tan(short/long").c_str(),
          500,-10,10, n->m_bins_raw[Paris],n->m_min_raw[Paris],n->m_max_raw[Paris]);
      if(m_calibrated)
        m_E_VS_ToF[l].reset((name+"_E_VS_ToF").c_str(), (name+" E VS ToF each detector").c_str(),
            500,-50-n->m_RF_shift/_ns,400-n->m_RF_shift/_ns, n->m_bins_raw[Paris],n->m_min_bidim[Paris],n->m_max_bidim[Paris]);
    }
    else
    {
      m_long_gate_vs_ratio[l].reset(nullptr);
      m_short_gate_vs_ratio[l].reset(nullptr);
      m_short_vs_tan_ratio[l].reset(nullptr);
      m_long_vs_short[l].reset(nullptr);
      m_short_vs_long_div_short[l].reset(nullptr);
      m_short_vs_short_div_long[l].reset(nullptr);
      m_short_vs_tan_long_div_short[l].reset(nullptr);
      m_short_vs_tan_short_div_long[l].reset(nullptr);
      if(m_calibrated)
        m_E_VS_ToF[l].reset(nullptr);
    }
  }
  return true;
}

void ParisBidim::Fill(Hit & hit, RF_Manager const & rf, int const & thread_nb)
{
  if (isParis[hit.label])
  {
    m_long_gate_vs_ratio[hit.label][thread_nb] -> Fill(hit.gate_ratio(), hit.nrj2);
    m_short_gate_vs_ratio[hit.label][thread_nb] -> Fill(hit.gate_ratio(), hit.nrj);
    m_short_vs_tan_ratio[hit.label][thread_nb] -> Fill((hit.nrj2!=0) ? TMath::Tan((Float_t)(hit.nrj2-hit.nrj)/hit.nrj2) : NAN, hit.nrj);

    m_long_vs_short[hit.label][thread_nb] -> Fill(hit.nrj, hit.nrj2);

    m_short_vs_long_div_short[hit.label][thread_nb] -> Fill((hit.nrj!=0) ? (Float_t)hit.nrj2/hit.nrj : NAN, hit.nrj);
    m_short_vs_short_div_long[hit.label][thread_nb] -> Fill((hit.nrj2!=0) ? (Float_t)hit.nrj/hit.nrj2 : NAN, hit.nrj);

    m_short_vs_tan_long_div_short[hit.label][thread_nb] -> Fill((hit.nrj!=0) ? TMath::Tan((Float_t)hit.nrj2/hit.nrj) : NAN, hit.nrj);
    m_short_vs_tan_short_div_long[hit.label][thread_nb] -> Fill((hit.nrj2!=0) ? TMath::Tan((Float_t)hit.nrj/hit.nrj2) : NAN, hit.nrj);


    if(m_calibrated) m_E_VS_ToF[hit.label][thread_nb] -> Fill(rf.pulse_ToF(hit.time,n -> m_RF_shift)/_ns, hit.nrjcal);
  }
}

void ParisBidim::Write()
{
  TFile* file = (TFile*)TFile::Open((n->m_outdir+m_outroot).c_str(),"recreate");
  if (!file) return;
  for (int i = 0; i<m_nb_histo; i++)
  {
    m_short_vs_tan_ratio[i].Merge();
    m_short_vs_tan_ratio[i] -> GetXaxis() -> SetTitle("tan(#frac{long-short}{long})");
    m_short_vs_tan_ratio[i] -> GetYaxis() -> SetTitle("short gate [ADC]");

    m_long_vs_short[i].Merge();
    m_long_vs_short[i] -> GetXaxis() -> SetTitle("short gate [ADC]");
    m_long_vs_short[i] -> GetYaxis() -> SetTitle("long gate [ADC]");

    m_short_vs_long_div_short[i].Merge();
    m_short_vs_long_div_short[i] -> GetXaxis() -> SetTitle("#frac{long}{short}");
    m_short_vs_long_div_short[i] -> GetYaxis() -> SetTitle("short gate [ADC]");

    m_short_vs_short_div_long[i].Merge();
    m_short_vs_short_div_long[i] -> GetXaxis() -> SetTitle("#frac{short}{long}");
    m_short_vs_short_div_long[i] -> GetYaxis() -> SetTitle("short gate [ADC]");

    m_short_vs_tan_long_div_short[i].Merge();
    m_short_vs_tan_long_div_short[i] -> GetXaxis() -> SetTitle("tan #frac{long}{short}");
    m_short_vs_tan_long_div_short[i] -> GetYaxis() -> SetTitle("short gate [ADC]");

    m_short_vs_tan_short_div_long[i].Merge();
    m_short_vs_tan_short_div_long[i] -> GetXaxis() -> SetTitle("tan #frac{short}{long}");
    m_short_vs_tan_short_div_long[i] -> GetYaxis() -> SetTitle("short gate [ADC]");

    m_long_gate_vs_ratio[i].Write();
    m_short_gate_vs_ratio[i].Write();
    m_short_vs_tan_ratio[i].Write();
    m_long_vs_short[i].Write();
    m_short_vs_long_div_short[i].Write();
    m_short_vs_short_div_long[i].Write();
    m_short_vs_tan_long_div_short[i].Write();
    m_short_vs_tan_short_div_long[i].Write();
    if(m_calibrated) m_E_VS_ToF[i].Write();
  }
  delete file;
  print("Root file written to ",n->m_outdir+m_outroot);
}


#endif //PARIS_H
