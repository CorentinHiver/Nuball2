#ifndef COINC_BUILDER2_H
#define COINC_BUILDER2_H
#include "Builder.hpp"

/*
  Makes coincidence building easier
*/

class CoincBuilder2 : public Builder
{
public:
  // Constructors :
  explicit CoincBuilder2()
  {
    m_event = new Event;
  }
  explicit CoincBuilder2(Event * _event)                                                         {m_event = _event;}
  explicit CoincBuilder2(Event * _event, Int_t const & _timeWindow) : m_time_window(_timeWindow) {m_event = _event;}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  Bool_t build(Hit const & _hit);

  void   reset() {m_event -> clear(); m_status = 0;}
  Bool_t coincidence(Hit const & hit) {return ((hit.time - m_last_hit.time) < m_time_window);}

  // Setters :
  void setTimeWindow(Int_t const & _timeWindow) {m_time_window = _timeWindow;}

  // Printers :
  void printEvent();

  // Public members :
  Int_t         n_evt = 0;

private:
  // Attributes :
  Time m_time_window = 500000ull; // 500 000 ps by default (ull = unsigned long long)
};

Bool_t CoincBuilder2::build(Hit const & hit)
{//return true when a coincidence is ready to be processed

  if (m_status == 2) this->reset();

  if (!coincON)
  {// No coincidence going on. If there is, go ...
    if (coincidence(hit))
    {// Bingo, coincidence detected ! Next call, go ...
      m_event -> clear();
      m_event -> push_back(m_last_hit);
      m_event -> push_back(hit);
      coincON = true; // Open the event
      m_status = 1; // Now, the event is being filled
    }
    else
    {// No coincidence detected
      m_single_hit = m_last_hit; // If last hit is not in time window of last hit, then
      m_last_hit = hit; // Building next event based on the last hit
      m_status = 0; // The current event is empty !
    }
  }
  else
  {// ... there ! Coincidence already detected, check if current hit is out of currently building event
    if (coincidence(hit))
    {// Hit in coincidence with the previous hits
      m_event -> push_back(hit);
    }
    else
    {// Hit out of coincidence. Next call, go back to first condition.
      m_last_hit = hit; // Build next event based on the current hit, that is not in the event.
      coincON = false;
      m_status = 2; // The current event is full
      return true; // Now the event is complete and can be treated
    }
  }
  return false;
}

void CoincBuilder2::printEvent()
{
  m_event -> Print();
}

#endif //COINC_BUILDER2_H
