#ifndef EVENT_H
#define EVENT_H
#include "../utils.hpp"
//if verbosity is needed to understand the process of event building :
// #define VERBOSE_EVT_BUILDER
// #define VERBOSE_EVT_BUILDER 2 for even more details

/*
  Makes coincidence building easier
  DEPRECATED
*/

class Evt_build
{
public:
  // Constructors :
  explicit Evt_build()
  {
    m_buffer = new Buffer;
    m_buffer->resize(2);
    m_event = new Event;
    m_event -> resize(255);
  }
  explicit Evt_build(Int_t const & i)
  {
    m_buffer = new Buffer;
    m_buffer->resize(2);
    m_event = new Event;
    m_event -> resize(i);
  }
  explicit Evt_build(Event  * _event) : m_event (_event) {}
  explicit Evt_build(Buffer * _event)                            : m_buffer(_event){}
  explicit Evt_build(Buffer * _event, Int_t const & _timeWindow) : m_buffer(_event), m_time_window(_timeWindow){}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  Bool_t build(Hit const & _hit);
  Bool_t build(Hit const & _hit, RF_Manager const & rf);

  void   flush() {m_buffer -> resize(0); m_buffer -> clear(); m_status = 0;}
  Bool_t coincidence(Hit const & hit) {return ((hit.time - m_last_hit.time) < m_time_window);}

  // Getters :
  Buffer* getArray() const {return m_buffer;}
  Bool_t const & isCoincTrigged() const {return coincON;}
  UShort_t size() const {return m_buffer -> size();}
  Bool_t is_single() const {return (this->status() == 0 && m_single_hit.label>0);}
  Hit & singleHit() {return m_single_hit;}
  Hit & olderHit() {return m_older_hit;}
  unsigned char const & status() const { return m_status; }
  ULong64_t const & getShift() const {return m_shift;}

  // Setters :
  inline void set_last_hit(Hit const & hit);
  void setTimeWindow(Int_t const & _timeWindow) {m_time_window = _timeWindow;}
  void setShift(Long64_t const & shift) {m_shift = shift;}
  void setShift_ns(Long64_t const & shift) {m_shift = 1000*shift;}
  void set_RF(RF_Manager* rf) {m_rf = rf;}

  // Printers :
  void printBuffer();

  // Public members :
  unsigned char mult = 1;
  Int_t         n_evt = 0;
  Time RF_ref_time = 0;

private:
  // Attributes :
  RF_Manager* m_rf   = nullptr;
  Event*    m_event  = nullptr;
  Buffer*   m_buffer = nullptr;
  Hit       m_empty_hit ;
  Hit       m_last_hit = m_empty_hit;
  Hit       m_single_hit = m_empty_hit;
  Hit       m_older_hit = m_empty_hit;
  Time      m_time_window = 500000ull; // 500 000 ps by default (ull = unsigned long long)
  Bool_t    coincON = false;
  unsigned char m_status = 0;

  //With beam pulse
  Time m_shift = 0;
};

Bool_t Evt_build::build(Hit const & hit, RF_Manager const & /*unused*/)
{
  if (m_status == 2 || m_event->mult>254)
  {
    RF_ref_time = m_last_hit.time - (m_last_hit.time-m_rf->last_hit + m_shift)%m_rf->period;
    m_single_hit = m_empty_hit;
    m_older_hit = m_empty_hit;
    m_event -> clear(); m_status = 0;
  }
  if (!coincON)
  {
    if ( (hit.time-RF_ref_time) < 399998ull - m_shift )
    {
      m_event -> push_back(m_last_hit);
      m_event -> push_back(hit);
      m_last_hit = m_empty_hit;
      m_status = 1;
      coincON = true;
    }
    else
    {
      m_older_hit = m_single_hit;
      m_single_hit = m_last_hit;
      set_last_hit(hit);
      m_status = 0;
    }
  }
  else
  {
    if ( (hit.time-RF_ref_time) < 399998ull - m_shift )
    {
      m_event -> push_back(hit);
    }
    else
    {
      m_last_hit = hit;
      coincON = false;
      m_status = 2;
      return true;
    }
  }
  return false;
}

// Bool_t build_more(Hit const & hit)
// {
//   if (hit.time-)
// }
void Evt_build::set_last_hit(Hit const & hit)
{
  m_last_hit = hit;
  RF_ref_time = hit.time - (hit.time - m_rf->last_hit + m_shift) % m_rf->period;
}

Bool_t Evt_build::build(Hit const & hit)
{//return true when an event is ready to be processed
  if (m_status == 2 || mult>254) this->flush(); //because nb_evts is on unsigned char so < 255
  if (!coincON)
  {// No coincidence going on. If there is, go ...
    if (coincidence(hit))
    {// Bingo, coincidence detected ! Next call, go ...
      m_buffer -> push_back( m_last_hit );
      m_buffer -> push_back( hit );
      mult = 2;
      coincON = true;
      m_status = 1;
    }
    else
    {// No coincidence detected...
      m_single_hit = m_last_hit;
      m_last_hit = hit; //store the hit for next loop
      mult = 1;
      n_evt++;
      m_status = 0;
    }
  }
  else
  {// ... there ! Coincidence already detected, check if it ends
    if (coincidence(hit))
    {// Hit in coincidence with the previous hits
      mult++;
      m_buffer -> push_back(hit);
    }
    else
    {// Hit out of coincidence. Next call, go back to first condition.
      n_evt++;
      coincON = false;
      m_last_hit = hit;
      m_status = 2;
      return true;
    }
  }
  return false;
}

void Evt_build::printBuffer()
{
  std::cout << " | ";
  for (auto const & hit : *m_buffer)
  {
    std::cout << hit.label << " | " ;
  }
  std::cout << std::endl;
}
#endif
