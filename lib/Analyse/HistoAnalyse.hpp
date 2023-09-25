#ifndef HISTOANALYSE_H
#define HISTOANALYSE_H

#include <Gate.hpp>
#include <MTTHist.hpp>

template<class THist>
class HistoAnalyse
{
public:

  HistoAnalyse(THist * histo) { reset(histo); }
  HistoAnalyse(MTTHist<THist> & histo) { reset(histo); }

  void reset() {m_exists = false;}

  void reset(MTTHist<THist> & histo)
  {
    if (MTObject::isMasterThread())
    {
      histo.Merge();
      m_histo = histo.get();
    }
    else
    {
      m_histo = histo[MTObject::getThreadIndex()];
    }
    m_exists = bool_cast(m_histo);
  }

  void reset(THist * histo)
  {
    if (histo->InheritsFrom("TH1")) { m_histo = histo; m_exists = true;}
    else {print("Object does not inherit from TH1 !!"); m_exists = false;}
  }

  THist * operator-> () {return m_histo;}

  ~HistoAnalyse();

  Float_t peaksOverBackground(std::vector<Gate> gates);

  void normalizeX(Float_t const & factor = 1);
  void normalizeY(Float_t const & factor = 1);
  void normalizeXY(Float_t const & factor = 1, int const & min_x = 0, int const & min_y = 0);

  void removeBackground();

private:
  THist * m_histo = nullptr;
  bool m_exists  = false;
};

template<class THist>
Float_t HistoAnalyse<THist>::peaksOverBackground(std::vector<Gate> gates)
{
  return 0.;
}

template<class THist>
void HistoAnalyse<THist>::normalizeY(Float_t const & factor)
{
  if (!m_exists) return;
  int const & bins_x = m_histo->GetNbinsX();
  if (bins_x<1) { print("Not enough binsx !!"); return; }
  if (m_histo->InheritsFrom("TH1") && !m_histo->InheritsFrom("TH2"))
  {
    // --- Normalize 1D --- //
    Float_t maxRow = 0.;
    // 1 : Get the maximum
    for (int x = 0; x<bins_x; x++) if(m_histo->GetBinContent(x) > maxRow) maxRow = m_histo->GetBinContent(x);
    // 2 : Normalize to set maximum = factor
    if (maxRow>0) for (int x = 0; x<bins_x+1; x++) m_histo -> SetBinContent(x, factor*m_histo->GetBinContent(x)/maxRow);
  }
  else if (m_histo->InheritsFrom("TH2"))
  {
    // --- Normalize 2D --- //
    int const & bins_y = m_histo->GetNbinsY();
    for (int x = 0; x<bins_x+1; x++)
    {
      Float_t maxRow = 0.;
      // 1 : Get the maximum
      for (int y = 0; y<bins_y; y++) if(m_histo->GetBinContent(x, y) > maxRow) maxRow = m_histo->GetBinContent(x, y);
      // 2 : Normalize to set maximum = factor
      if (maxRow>0) for (int y = 0; y<bins_y+1; y++) m_histo -> SetBinContent(x, y, factor*m_histo->GetBinContent(x, y)/maxRow);
    }
  }
}

template<class THist>
void HistoAnalyse<THist>::normalizeX(Float_t const & factor)
{

  if (m_histo->InheritsFrom("TH1") && !m_histo->InheritsFrom("TH2"))
  {
    print("Can't normalize the X axis of a TH1 !!");
    return;
  }
  else if (m_histo->InheritsFrom("TH2"))
  {
    // --- Normalize 2D --- //
    int const & bins_x = m_histo->GetNbinsX();
    int const & bins_y = m_histo->GetNbinsY();
    for (int y = 0; y<bins_y+1; y++)
    {
      Float_t maxRow = 0.;
      // 1 : Get the maximum
      for (int x = 0; x<bins_x-1; x++) if(m_histo->GetBinContent(x, y) > maxRow) maxRow = m_histo->GetBinContent(x, y);
      // 2 : Normalize to set maximum = factor
      for (int x = 0; x<bins_x+1; x++) m_histo -> SetBinContent(x, y, factor*m_histo->GetBinContent(x, y)/maxRow);
    }
  }
}

template<class THist>
void HistoAnalyse<THist>::normalizeXY(Float_t const & factor, int const & min_x, int const & min_y)
{

}

template<class THist>
HistoAnalyse<THist>::~HistoAnalyse()
{
  // if (m_histo) delete m_histo;
}

template<class THist>
void NormalizeX(MTTHist<THist> & histo, Float_t const & factor = 1)
{
  HistoAnalyse<THist> histo_a (histo);
  histo_a.normalizeX(factor);
}

template<class THist>
void NormalizeY(MTTHist<THist> & histo, Float_t const & factor = 1)
{
  HistoAnalyse<THist> histo_a (histo);
  histo_a.normalizeY(factor);
}

#endif //HISTOANALYSE_H
