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
 * If two consecutive hits are within a time window, they are added to the same event
 * This class will return true when the current hit is out of the time window,
 * i.e. is not in coincidence with the hits of the current event.
 * 
 * Using Builder::keepSingles(bool) method will make CoincBuilder::build() return true 
 * for alone hits without any other hits in coincidence. 
 * 
 */
class CoincBuilder : public Builder 
{
public:
  // Constructors :
  explicit CoincBuilder() noexcept { }

  explicit CoincBuilder(Event * _event) noexcept : 
    Builder(_event)
  {}

  explicit CoincBuilder(Event * _event, Time const & _timeWindow) noexcept : 
    Builder(_event), 
    m_time_window(_timeWindow) 
  {}

  bool build(Hit const & _hit) noexcept;

  void reset() noexcept
  {
    m_event -> clear(); 
    m_status = 0;
  }
  
  bool coincidence(Hit const & start_hit, Hit const & stop_hit) const noexcept 
  {
    return (std::abs(Time_cast(stop_hit.stamp - start_hit.stamp)) < m_time_window);
  }

  // Setters :
  void setTimeWindow(Time const & timewindow) noexcept {m_time_window = timewindow;}

  void setTimeWindow_ns(Time_ns const & timewindow_ns) noexcept {m_time_window = timewindow_ns*1000ll;}

  // Getters :
  auto getEvent() const noexcept {return Builder::m_event;}

private:
  // CoincBuilder's attributes :
  Time m_time_window = 500_ns; // 500 000 ps by default (ll = long long)
};

bool CoincBuilder::build(Hit const & hit) noexcept
{
  if(m_event->mult>255) reset(); // 255 is the maximum number of hits allowed inside of an event
  switch (m_status)
  {
    case 0 : case 2 :// If no coincidence has been detected in previous iteration :
      // 1 : last event has been processed already, clear it for new iteration
      m_event -> clear();
      // 2 : last hit was not in coincidence with last event. We now attempt to create a new event based on this hit :
      *m_event = m_first_hit;
      // 3 : test coincidence bewteen current hit and last hit, i.e. current hit and current event
      if (this -> coincidence(m_first_hit, hit))
      {// Case 1 :
        // The previous and current hit are in coincidence and belong to the same event.
        // In next call, we'll check if the next hits also belong to this event (situations 1' or 2 in case 1)
        m_event -> push_back(hit);
        m_status = 1; // Now, the event is being filled
        return false;
      }
      else
      {// Case 0 :
        // Case 0 means the current hit is not in coincidence with the hit of last iteration.
        // This means that the m_event created in this iteration, filled with m_first_hit 
        // (i.e. hit of last iteration), contains a single hit, alone in its time window

        m_first_hit = hit; // Building next event based on current hit
        m_status = 0; // No coincidence detected

        // If interested in keeping single hits, return true, else return false
        return Builder::m_keep_singles;
      }
    break;

    case 1 :
    // Coincidence already detected, check if current hit is out of the event currently being build
      if (this -> coincidence(m_first_hit, hit))
      {// Case 1' :
        // Hit in coincidence with the event. Next call, still try to add hits
        m_event -> push_back(hit);
        return false;
      }
      else
      {// Case 2 : 
        // Hit out of coincidence. Next call, go back to first condition.
        m_first_hit = hit; // Build next event based on the current hit, that is not in the event.
        m_status = 2; // The current event is full
        return true; // Now the event is complete and can be processed
      }
    break;

    default: error("event builder issues..."); return false;
  }
  // return false;
}

std::ostream& operator<< (std::ostream& out, CoincBuilder const & builder)
{
  out << *(builder.getEvent());
  return out;
}

#endif //COINC_BUILDER2_H
