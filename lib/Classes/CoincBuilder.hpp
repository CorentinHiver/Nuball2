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

  bool build(Hit const & _hit);

  void reset() {m_event -> clear(); m_status = 0;}
  bool coincidence(Hit const & hit) {
  //   print(hit.label, hit.stamp, m_first_hit.label, m_first_hit.stamp, Time_cast(hit.stamp - m_first_hit.stamp));  
  // std::cin.get();
    return (Time_cast(hit.stamp - m_first_hit.stamp) < m_time_window);}

  // Setters :
  void setTimeWindow(Time const & timewindow) {m_time_window = timewindow;}
  void setTimeWindow_ns(Time_ns const & timewindow_ns) {m_time_window = timewindow_ns*1000;}

  // Printers :
  void printEvent();

private:
  // Attributes :
  Time m_time_window = 500000ll; // 500 000 ps by default (ll = long long)
};

bool CoincBuilder::build(Hit const & hit)
{
  if(m_event->mult>255) reset(); // 255 is the maximum number of hits allowed inside of an event
  switch (m_status)
  {
    case 0 : case 2 :// If no coincidence has been detected in previous iteration :
    m_event -> clear();
    *m_event = m_first_hit;
    if (coincidence(hit))
    {// Case 1 :
      // The previous and current hit are in the same event.
      // In next call, we'll check if the next hits also belong to this event (situations 1' or 2)
      m_event -> push_back(hit);
      m_status = 1; // Now, the event is being filled
    }
    else
    {// Case 0 :
      // The last and current hits aren't in the same event.
      // The last hit is therefore a single hit, alone in the time window
      m_first_hit = hit; // Building next event based on the last hit
      m_status = 0; // No event detected
      if (m_keep_singles) return true;
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
      m_first_hit = hit; // Build next event based on the current hit, that is not in the event.
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
