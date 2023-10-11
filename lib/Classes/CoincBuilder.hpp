#ifndef COINC_BUILDER2_H
#define COINC_BUILDER2_H
#include "Builder.hpp"

/*
  Makes coincidence building easier
*/

/**
 * @brief Coincidence builder
 * @details
 * Use this class when you don't have any particular time reference
 * Add one hit after the other using CoincBuilder::build()
 * If two consecutive hits are within a time window, add the following hits
 * This class will return true when the current hit is out of the time window
 * 
 * Using Builder::keepSingles(bool) method will make CoincBuilder::build() return
 * true for lonely hits as well
 * 
 */
class CoincBuilder : public Builder 
{
public:
  // Constructors :
  explicit CoincBuilder() { }
  explicit CoincBuilder(Event * _event)                           : Builder(_event)                             {}
  explicit CoincBuilder(Event * _event, Time const & _timeWindow) : Builder(_event), m_time_window(_timeWindow) {}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  bool build(Hit const & _hit);

  void reset() {m_event -> clear(); m_status = 0;}
  bool coincidence(Hit const & hit) {
  //   print(hit.label, hit.stamp, m_last_hit.label, m_last_hit.stamp, Time_cast(hit.stamp - m_last_hit.stamp));  
  // std::cin.get();
    return (Time_cast(hit.stamp - m_last_hit.stamp) < m_time_window);}

  // Setters :
  void setTimeWindow(Time const & _timeWindow) {m_time_window = _timeWindow;}

  // Printers :
  void printEvent();

private:
  // Attributes :
  Time m_time_window = 500000; // 500 000 ps by default (ull = unsigned long long)
};

bool CoincBuilder::build(Hit const & hit)
{//return true when a coincidence is ready to be processed
  if(m_event->mult>255) reset();
  switch (m_status)
  {
    case 0 : case 2 :// If no coincidence has been detected in previous iteration :
    if (coincidence(hit))
    {// Case 1 :
      // The previous and current hit are in the same event.
      // In next call, we'll check if the next hits are in the event or not (cases 3 or 4)
      *m_event = m_last_hit;
      m_event -> push_back(hit);
      coincON = true; // Open the event
      m_status = 1; // Now, the event is being filled
    }
    else
    {// Case 0 :
      // The last and current hits aren't in the same event.
      // This means either the last hit is filled with an empty hit if last hit closed an event,
      // or a single hit : a lonely hit alone in the time window around its RF.
      m_single_hit = m_last_hit; // If last hit is not in time window of last hit, then
      m_last_hit = hit; // Building next event based on the last hit
      m_status = 0; // The current event is empty !
      if (m_keep_singles && m_single_hit.label != 0)
      {
        *m_event = m_single_hit;
        return true;
      }
    }
    break;

    case 1 :
  // ... there ! Coincidence already detected, check if current hit is out of currently building event
    if (coincidence(hit))
    {// Case 1' :
      // Hit in coincidence with the previous hits
      m_event -> push_back(hit);
    }
    else
    {// Case 2 : 
      // Hit out of coincidence. Next call, go back to first condition.
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
