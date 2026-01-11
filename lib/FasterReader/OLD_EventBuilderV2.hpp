#pragma once
#include "../Classes/Event.hpp"

/**
 * @brief Performs event-building : takes a 
 */
class EventBuilderV2
{
public:
  EventBuilderV2(Event * event, std::vector<Hit*> * hits, std::vector<size_t> && m_index, Time time_window = Time(2e6)) :
    m_event(event), m_hits(hits), m_oindex(std::move(m_index)), m_timeWindow(time_window)
  {
  }

  void build()
  {
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)

    // 1. Initialize the event buffer

    std::vector<size_t> eventID;
    eventID.emplace_back(m_oindex[0]); // First hit of first event of buffer
    
    // 2. Loop through the hits buffer

    for (size_t loop_i = 1; loop_i < m_oindex.size(); ++loop_i)
    {
      
      auto const & hit_id      =  m_oindex [loop_i         ];
      auto const & hit        =  (*m_hits) [hit_id         ];
      auto const & first_hit  =  (*m_hits) [eventID.front()];

      // 3. Add new hits until one is out of time window ("closing the event")

      if (Time_cast(hit->stamp - first_hit->stamp) < m_timeWindow)
      {
        eventID.emplace_back(hit_id);
        continue;
      }

      // -- Piece of code only executed when the event is full (closed) -- //
        
      // 4. Fill the event buffer
      
      m_eventIDbuffer.emplace_back(eventID);
      
      // 5. Prepare next event : 
      eventID.clear();
      eventID.emplace_back(hit_id); // Save the current hit
    }
  }

  void loadEvent(size_t event_id)
  {
    m_event->clear();
    for (auto const & hit_id : m_eventIDbuffer[event_id]) m_event->push_back((*m_hits)[hit_id]);
  }

private:
  Event* m_event = nullptr;
  std::vector<Hit*>* m_hits = nullptr;
  std::vector<size_t> m_oindex;
  
  Time m_timeWindow = Time(2e6); // Default 2 us
};