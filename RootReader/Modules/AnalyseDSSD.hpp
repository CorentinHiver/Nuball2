#ifndef DSSD_H
#define DSSD_H

class AnalyseDSSD
{
public:
  AnalyseDSSD(){this->Initialize();}
  void Initialize();

  void setBinningSpectra(int const & bins, Float_t const & min, Float_t const & max)
  {
    m_bins_spectra = bins;
    m_min_spectra  = min ;
    m_max_spectra  = max ;
  }

private:

  // ---- Variables ---- //
  int m_bins_spectra = 10000;
  Float_t m_min_spectra = 0;
  Float_t m_max_spectra = 20000;

  // ---- Histograms ---- //
  TH2F* m_each_Sector_spectra;
  TH2F* m_each_Ring_spectra;

  // nRnS : same number of rings and sectors
  TH2F* m_each_Sector_spectra_nRnS;
  TH2F* m_each_Ring_spectra_nRnS;

};

void AnalyseDSSD::Initialize()
{
  m_each_Sector_spectra = new TH2F("Each sector spectra","Each sector spectra", 36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra = new TH2F("Each ring spectra","Each ring spectra", 16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Sector_spectra_nRnS = new TH2F("Each sector spectra nRnS","Each sector spectra nRnS", 36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra_nRnS = new TH2F("Each sector spectra nRnS","Each sector spectra nRnS", 16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);
}

#endif //DSSD_H
