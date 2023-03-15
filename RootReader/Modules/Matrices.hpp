#ifndef MATRICES_H
#define MATRICES_H

class Matrices
{
public:
  Matrices() {}
  ~Matrices()
  {
    if (m_BGO_VS_Clover) delete m_BGO_VS_Clover;
    if (m_LaBr3_VS_Clover) delete m_LaBr3_VS_Clover;
    if (m_Clean_Ge_bidim) delete m_Clean_Ge_bidim;
    for (auto histo : m_each_BGO_VS_all_Clover) if (histo) delete histo;
    for (auto histo : m_each_LaBr3_VS_all_Clover) if (histo) delete histo;

  }
  void Initialize();
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event);
  void Write(std::string const & outRoot);
  bool setParameters(std::string const & parameters);

  void setSorted_Event(Sorted_Event * evt) {m_s_event = evt;}

  void SetBinsGe(int const & bins, Float_t const & min, Float_t const & max)
  {
    m_Ge_bins = bins; m_Ge_min = min; m_Ge_max = max;
  }

  void SetBinsBGO(int const & bins, Float_t const & min, Float_t const & max)
  {
    m_BGO_bins = bins; m_BGO_min = min; m_BGO_max = max;
  }

  void SetBinsLaBr3(int const & bins, Float_t const & min, Float_t const & max)
  {
    m_LaBr3_bins = bins; m_LaBr3_min = min; m_LaBr3_max = max;
  }

private:

  // ---- Variables ---- //
  int m_Ge_bins = 6000; Float_t m_Ge_min = 0; Float_t m_Ge_max = 6000;
  int m_BGO_bins = 600; Float_t m_BGO_min = 0; Float_t m_BGO_max = 6000;
  int m_LaBr3_bins = 1000; Float_t m_LaBr3_min = 0; Float_t m_LaBr3_max = 6000;

  // ---- Histograms ---- //
  TH2F* m_BGO_VS_Clover = nullptr;
  TH2F* m_LaBr3_VS_Clover = nullptr;
  std::vector<TH2F*> m_each_BGO_VS_all_Clover;
  std::vector<TH2F*> m_each_LaBr3_VS_all_Clover;

  TH2F* m_Clean_Ge_bidim = nullptr;

  Sorted_Event *m_s_event = nullptr;
};

bool Matrices::setParameters(std::string const & parameters)
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
  }
  return true;
}

void Matrices::Initialize()
{
  m_Clean_Ge_bidim = new TH2F("Clean Ge bidim", "Clean Ge bidim",
      m_Ge_bins,m_Ge_min,m_Ge_max, m_Ge_bins,m_Ge_min,m_Ge_max);
  m_BGO_VS_Clover = new TH2F("BGO VS Clover", "BGO VS Clover",
      m_Ge_bins,m_Ge_min,m_Ge_max, m_BGO_bins,m_BGO_min,m_BGO_max);
  m_LaBr3_VS_Clover = new TH2F("Paris VS Clover", "Paris VS Clover",
      m_Ge_bins,m_Ge_min,m_Ge_max, m_LaBr3_bins,m_LaBr3_min,m_LaBr3_max);

  m_each_BGO_VS_all_Clover.resize(48, nullptr);
  m_each_LaBr3_VS_all_Clover.resize(74, nullptr);

  for (size_t label = 0; label<g_labelToName.size(); label++)
  {
    std::string name = g_labelToName[label]+" VS CLover";
    if (isBGO[label])
    {
      m_each_BGO_VS_all_Clover[labelToBGOcrystal[label]] = new TH2F(name.c_str(), name.c_str(),
          m_Ge_bins,m_Ge_min,m_Ge_max, m_BGO_bins,m_BGO_min,m_BGO_max);
    }
    else if (isParis[label])
    {
      m_each_LaBr3_VS_all_Clover[labelToPariscrystal[label]] = new TH2F(name.c_str(), name.c_str(),
          m_Ge_bins,m_Ge_min,m_Ge_max, m_LaBr3_bins,m_LaBr3_min,m_LaBr3_max);
    }
  }
}

void Matrices::FillSorted(Sorted_Event const & event)
{
  for (size_t loop_i = 0; loop_i<event.clover_hits.size(); loop_i++)
  {
    if (!event.BGO[loop_i])
    {
      auto const & nrj_i = event.nrj_clover[loop_i];
      for (size_t loop_j = loop_i+1; loop_j<event.clover_hits.size(); loop_j++)
      {
        if (!event.BGO[loop_j])
        {
          auto const & nrj_j = event.nrj_clover[loop_j];
          m_Clean_Ge_bidim -> Fill(nrj_i,nrj_j);
          m_Clean_Ge_bidim -> Fill(nrj_j,nrj_i);
        }
      }
    }
  }
}

void Matrices::FillRaw(Event const & event)
{
  if (event.size()<2) return;
  for (size_t loop_i = 0; loop_i<event.size(); loop_i++)
  {
    auto const & label_i = event.labels[loop_i];
    if (isGe[label_i])
    {
      auto const & nrj_i = event.nrjs[loop_i];
      for (size_t loop_j = loop_i+1; loop_j<event.size(); loop_j ++)
      {
        auto const & label_j = event.labels[loop_j];
        if (isBGO[label_j])
        {
          auto const & nrj_j = event.nrjs[loop_j];
          m_BGO_VS_Clover->Fill(nrj_i, nrj_j);
          m_each_BGO_VS_all_Clover[labelToBGOcrystal[label_j]] -> Fill(nrj_i, nrj_j);
        }
        else if (isParis[label_j])
        {
          Paris_Label label(label_j);
          auto const & nrj_j = event.nrjs[loop_j];
          auto const & nrj2_j = event.nrj2s[loop_j];
          if (nrj2_j!=0 && (nrj2_j-nrj_j)/nrj2_j < 0.4)
          {
            m_LaBr3_VS_Clover->Fill(nrj_i, nrj_j);
            m_each_LaBr3_VS_all_Clover[labelToPariscrystal[label_j]] -> Fill(nrj_i, nrj_j);
          }
        }
      }
    }
  }
}

void Matrices::Write(std::string const & outRoot)
{
  std::unique_ptr<TFile> outfile(TFile::Open(outRoot.c_str(),"RECREATE"));
  outfile->cd();

  if (m_Clean_Ge_bidim && m_Clean_Ge_bidim->Integral()>1) m_Clean_Ge_bidim -> Write();
  if (m_BGO_VS_Clover && m_BGO_VS_Clover->Integral()>1) m_BGO_VS_Clover -> Write();
  if (m_LaBr3_VS_Clover && m_LaBr3_VS_Clover->Integral()>1) m_LaBr3_VS_Clover -> Write();

  for (auto & hist : m_each_BGO_VS_all_Clover) if (hist && hist->Integral()>1) hist->Write();
  for (auto & hist : m_each_LaBr3_VS_all_Clover) if (hist && hist->Integral()>1) hist->Write();

  outfile->Write();
  outfile->Close();
  print("Matrices file written to", outRoot);
}

#endif //MATRICES_H
