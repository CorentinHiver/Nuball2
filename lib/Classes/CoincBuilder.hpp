#ifndef COINC_BUILDER2_H
#define COINC_BUILDER2_H
#include "Builder.hpp"

/*
  Makes coincidence building easier
*/

class CoincBuilder : public Builder
{
public:
  // Constructors :
  explicit CoincBuilder() { }
  explicit CoincBuilder(Event * _event)                                                         {m_event = _event;}
  explicit CoincBuilder(Event * _event, Int_t const & _timeWindow) : m_time_window(_timeWindow) {m_event = _event;}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  Bool_t build(Hit const & _hit);

  void   reset() {m_event -> clear(); m_status = 0;}
  Bool_t coincidence(Hit const & hit) {return ((hit.time - m_last_hit.time) < m_time_window);}

  // Setters :
  void setTimeWindow(Int_t const & _timeWindow) {m_time_window = _timeWindow;}

  // Printers :
  void printEvent();

private:
  // Attributes :
  Time m_time_window = 500000ull; // 500 000 ps by default (ull = unsigned long long)
};

Bool_t CoincBuilder::build(Hit const & hit)
{//return true when a coincidence is ready to be processed
  if(m_event->mult>255) reset();
  if (!coincON)
  {// If no coincidence has been detected in previous iteration :
    if (coincidence(hit))
    {// Case 1 :
      // The previous and current hit are in the same event.
      // In next call, we'll check if the next hits are in the event or not (cases 3 or 4)
      m_event -> clear();
      m_event -> push_back(m_last_hit);
      m_event -> push_back(hit);
      coincON = true; // Open the event
      m_status = 1; // Now, the event is being filled
    }
    else
    {// Case 2 :
      // The last and current hits aren't in the same event.
      // This means either the last hit is filled with an empty hit if last hit closed an event,
      // or a single hit : a lonely hit alone in the time window around its RF.
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

void CoincBuilder::printEvent()
{
  m_event -> Print();
}

#endif //COINC_BUILDER2_H
