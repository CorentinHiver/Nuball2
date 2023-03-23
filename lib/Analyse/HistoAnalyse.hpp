#ifndef HISTOANALYSE_H
#define HISTOANALYSE_H

#include "../utils.hpp"
#include "../Classes/Gate.hpp"

template<class THist>
class HistoAnalyse
{
public:

  HistoAnalyse(THist * histo) { if (histo->InheritsFrom("TH1")) m_histo = histo; else print("Object does not inherit from TH1 !!"); }

  ~HistoAnalyse();

  template <class... ARGS>
  void reset() {m_exists = false;}

  template <class... ARGS>
  void reset(THist * histo) {m_histo = histo;}

  Float_t peaksOverBackground(std::vector<Gate> gates);
  // Float_t


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
HistoAnalyse<THist>::~HistoAnalyse()
{
  if (m_histo) delete m_histo;
}

#endif //HISTOANALYSE_H
