#ifndef DSSD_H
#define DSSD_H

#include "../../lib/MTObjects/MTTHist.hpp"

class AnalyseDSSD
{
public:
  AnalyseDSSD(){}

  bool setParameters(std::string const & parameters);
  void Initialize();
  void InitializeRun();
  void FillRaw(Event const & event){}
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void WriteRun();
  void WriteManip();

  void setBinningSpectra(int const & bins, Float_t const & min, Float_t const & max)
  {
    m_bins_spectra = bins;
    m_min_spectra  = min ;
    m_max_spectra  = max ;
  }

private:

  // ---- Parameters ---- //
  std::string m_outDir = "test_ds.root";
  std::string m_outRoot = "";

  // ---- Variables ---- //
  int m_bins_spectra = 10000;
  Float_t m_min_spectra = 0;
  Float_t m_max_spectra = 20000;

  // ---- Histograms ---- //
  MTTHist<TH2F> m_each_Sector_spectra;
  MTTHist<TH2F> m_each_Ring_spectra;

  // nRnS : same number of rings and sectors
  MTTHist<TH2F> m_each_Sector_spectra_nRnS;
  MTTHist<TH2F> m_each_Ring_spectra_nRnS;

  Vector_MTTHist<TH2F> m_each_DSSD_VS_Time;
};

bool AnalyseDSSD::setParameters(std::string const & parameters)
{
  std::istringstream pa(parameters);
  std::string line;
  while(getline(pa,line))
  {
    std::istringstream is(line);
    std::string temp;
    while(is>>temp)
    {
           if (temp == "outDir:")  is >> m_outDir ;
      else if (temp == "outRoot:") is >> m_outRoot;
    }
  }
  return true;
}

void AnalyseDSSD::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  for (size_t loop_i = 0; loop_i<event_s.DSSD_hits.size(); loop_i++)
  {
    // auto const & dssd_i = event_s.DSSD_hits[loop_i];
    //
    // auto const & label = event.labels[dssd_i];
    // auto const & Time = event_s.times[dssd_i];
    // auto const & nrj = event.nrjs[dssd_i];
  }
}

void AnalyseDSSD::Initialize()
{
  m_each_Sector_spectra.reset("Each sector spectra","Each sector spectra",
      36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra.reset("Each ring spectra","Each ring spectra",
      16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Sector_spectra_nRnS.reset("Each sector spectra nRnS","Each sector spectra same number of sectors and rings",
      36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra_nRnS.reset("Each ring spectra nRnS","Each ring spectra same number of sectors and rings",
      16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);
}

void AnalyseDSSD::WriteRun()
{

}

void AnalyseDSSD::WriteManip()
{

}

#endif //DSSD_H
