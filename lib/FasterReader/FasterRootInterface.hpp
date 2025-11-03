#ifndef FASTERROOTINTERFACEV2_HPP
#define FASTERROOTINTERFACEV2_HPP

#include "FasterReaderV2.hpp"
#include "../Classes/Hit.hpp"
#include "TFile.hpp"

template <class Config = FasterReaderConfig>
class FasterRootInterface_t : public FasterReaderV2_t<FasterReaderConfig>
{
public:

  FasterRootInterface_t() : FasterReaderV2_t<FasterReaderConfig>() {}
  FasterRootInterface_t(std::string const & filename, Hit * hit = nullptr) : 
    FasterReaderV2_t<FasterReaderConfig>(filename), 
    m_hit(hit) {}

  void initializeOutput(std::string rootname)
  {
    if (!m_hit) 
    file
    tree
  }

  void setHit(Hit * hit) {m_hit = hit;}

  bool readNextRootHit()
  {
    cleanQDCs();
    loadLabel();
    switch(FasterReaderV2_t<FasterReaderConfig>::readNextHit())
    {
      case Alias::TRAPEZ_SPECTRO : loadTrapez(); break;
      case Alias::RF_DATA        : loadRF    (); break;
      case Alias::CRRC4_SPECTRO  : loadCRRC4 (); break;
      case Alias::QDC_TDC_X1     : loadQDC<1>(); break;
      case Alias::QDC_TDC_X2     : loadQDC<2>(); break;
      case Alias::QDC_TDC_X3     : loadQDC<3>(); break;
      case Alias::QDC_TDC_X4     : loadQDC<4>(); break;
      case Alias::EOF_FASTER     : return false;
      default: ;
    }
    loadTimestamp();
    return m_cursor < m_cursor_max;
  }

  void setMaxHits(ulonglong max) {m_cursor_max = max;}


  auto       & getHit()       {return *m_hit;}
  auto const & getHit() const {return *m_hit;}

  auto const & getMaxHits() const {return m_cursor_max;}

private:

  void cleanQDCs()
  {
    m_hit -> qdc2 = 0;
    m_hit -> qdc3 = 0;
  }

  void loadLabel()
  {
    m_hit -> label = m_header.label;
  }

  void loadTimestamp() {m_hit->stamp = m_timestamp;}
  
  void loadTrapez()
  {
    m_hit -> adc    = m_trapez_spectro.measure;
    m_hit -> pileup = (m_trapez_spectro.pileup == 1 || m_trapez_spectro.saturated == 1 || m_trapez_spectro.sat_cpz == 1);
  }

  void loadRF()
  {
    m_hit -> adc = ADC_cast(m_rf_data.period*1000);
    m_hit -> pileup = m_rf_data.saturated; 
  }

  void loadCRRC4()
  {
    m_hit -> adc = m_crrc4_spectro.measure;
    m_hit -> pileup = (m_crrc4_spectro.pileup == 1 || m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  void loadQDC()
  {
    auto qdc_data = getQDC<n>();
    m_hit -> pileup = false;
    if constexpr (n>0)  {m_hit -> adc  = qdc_data[0].measure; m_hit -> pileup |= qdc_data[0].saturated;}
    if constexpr (n>1)  {m_hit -> qdc2 = qdc_data[1].measure; m_hit -> pileup |= qdc_data[1].saturated;}
    if constexpr (n>2)  {m_hit -> qdc3 = qdc_data[2].measure; m_hit -> pileup |= qdc_data[2].saturated;}
  }

  Hit* m_hit = nullptr;
  TFile * file = nullptr;
  TTree * tree = nullptr;
  ulonglong m_cursor_max = Colib::big<ulonglong>();

};

using FasterRootInterface = FasterRootInterface_t<>;

#endif //FASTERROOTINTERFACEV2_HPP
