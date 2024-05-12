#ifndef EVENT_BUILDER_136_HPP
#define EVENT_BUILDER_136_HPP

#include "Builder.hpp"
#include "RF_Manager.hpp"
#include "Alignator.hpp"

/**
 * @brief Event builder based on RF
 * @details
 * 
 * You need to create an instance of RF_Manager before instantiating this class 
 */
class EventBuilderRF : public Builder
{
public:
  // Constructors :
  EventBuilderRF(Event* _event, RF_Manager* _rf) : Builder(_event), m_rf(_rf) {}

  /// @brief Try to add the hit to the current event
  /// @return true if the current hit is outside of the time window
  bool build(Hit const & hit) override;

  bool coincidence(Hit const & hit) override
  {
    auto const & dT = Time_cast(hit.stamp-m_RF_ref_stamp);
    auto const & dT_max = Time_cast(m_rf->period)-m_rf->offset();
    return (dT < dT_max);
  }
  
  void clear() override
  {
    m_event -> clear(); 
    m_status = 0;
  }

  // Getters :
  auto const & getRefTime() const {return m_RF_ref_stamp;}
  auto const & getLastRefTime() const {return m_RF_ref_stamp;}

  // Setters :
  void setRF(RF_Manager* rf) {m_rf = rf;}
  void setFirstRF(Hit const & hit);
  void setRFRefTime(Timestamp const & _RF_ref_stamp) {m_RF_ref_stamp = _RF_ref_stamp;}
  inline void setFirstHit (Hit const & hit);

  /// @brief Experimental : add more period after a trigger (not working now I think)
  void tryAddPreprompt_simple();
  void tryAddNextHit_simple(TTree * tree, TTree * outTree, Hit & hit, int & loop, Alignator const & gindex);
  void setNbPeriodsMore(int const & periods) {m_nb_periods_more = periods;}

private:
  // Attributes :
  RF_Manager* m_rf = nullptr;
  Timestamp m_RF_ref_stamp = 0;
#ifdef PREPROMPT
  std::stack<Hit> m_hit_buffer;
#endif //PREPROMPT
  int m_nb_periods_more = 0;
};

bool EventBuilderRF::build(Hit const & hit)
{
  if(m_event->mult>255) clear(); // 255 is the maximum number of hits allowed inside of an event
  #ifdef PREPROMPT
    m_hit_buffer.emplace(hit);
  #endif //PREPROMPT
  switch (m_status)
  {
    // If no coincidence has been detected in previous iteration :
    case 0 : case 2 : 
     *m_event = m_first_hit; // Filling the event with this first hit
      m_event -> setT0(m_RF_ref_stamp);// We set the reference RF timestamp as the t0 of the event

      if (this->coincidence(hit))
      {// Situation 1 :
        // The previous and current hit are in the same event.
        // In next call, we'll check if the next hits also belong to this event (situations 1' or 2)
        m_event -> push_back(hit);
        m_first_hit.clear(); // Prepare first_hit for next event
        m_status = 1; // The event can be filled with other hits
        return false; // The event is not ready to be processed
      }
      else
      {// Situation 0 :
        // The last and current hits aren't in the same event.
        // The last hit is therefore a single hit, alone in its time window.
        // And the current hit is set to be the reference hit for next call :
        this -> setFirstHit(hit);
        m_status = 0; // Single event detected
        // If we want to write the singles then we must return true in this situation :
        if (Builder::m_keep_singles) return true;
      }
    break;
    
    // If we are building an event (i.e. the two previous hits were in coincidence):
    case 1 :
      if (this->coincidence(hit))
      {
        // Situation 1' :
        // The current hit also belongs to the event
        m_event->push_back(hit);
        // NB: m_status still equals 1
        return false; // The event is still not ready to be processed
      }
      else
      {
        // Situation 2 :
        // The current hit is outside of the event : this hit closes the event 
        // Therefore, m_event is full with all the hits of the event.
        m_status = 2; // The event is complete
        // The current hit is set to be the first hit for next event building :
        this -> setFirstHit(hit);
        return true; // The event is ready to be processed
      }
    break;
    default: throw_error("event builder issues...");
  }
  return false;
}

void EventBuilderRF::setFirstRF(Hit const & rf_hit)
{
  if (rf_hit.label != RF_Manager::label) throw std::runtime_error("First RF hit of the file is not really a RF");
  m_rf -> setHit(rf_hit);
}

void EventBuilderRF::setFirstHit(Hit const & hit)
{
  // The closest RF to this hit is taken as reference to build the event :
  m_RF_ref_stamp = m_rf->refTime(hit.stamp);

  // m_first_hit is filled with the current hit :
  m_first_hit = hit;
}


void EventBuilderRF::tryAddNextHit_simple(TTree * tree, TTree * outTree, Hit & hit, int & loop, Alignator const & gindex)
{
  // If the next hits (for which the relative timestamp is greater than the RF period) are close enough to the event, 
  // they might come from an isomer de-excitation. Therefore, we would like to gather them in order to increase statistics.

  // The current hit is the one that "closed" the event, i.e. is outside of time window,
  // and has been set to the last_hit to serve for next EventBuilderRF::build() call
  // as the potential first hit of next event.
  // Therefore, the current m_RF_ref_stamp (pulse "t = zero") is relative to the current hit.
  // In order to check if the current hit is within the extended time window of the previous event (which is the one currently stored)
  // we need the pulse reference timestamp of the previous event :
  Timestamp const & RF_ref_stamp = m_rf->refTime(m_event->stamp);

  // First, try to add the current hit (the hit that closed the window) :
  auto const & dT = Time_cast(hit.stamp-RF_ref_stamp) % Time_cast(m_rf->period);
  auto const & dT_max = Time_cast(m_rf->period*(m_nb_periods_more+1))-m_rf->offset();
  if (dT < dT_max)
  {
    m_event->push_back(hit);

    // If it worked, check if there are any hits left in the tree and load it :
    auto const & nb_hits = tree -> GetEntries();
    if (++loop>nb_hits-1) return;
    tree->GetEntry(gindex[loop]);

    // Then let's try to add the next hit if any :
    while(Time_cast(hit.stamp-RF_ref_stamp) < Time_cast(m_rf->period*(m_nb_periods_more+1) - m_rf->offset()) )
    {
      if (++loop<nb_hits) 
      {// Take into  account the potential presence of RF downscale in 
        if(m_rf -> setHit(hit)) 
        {
          Event temp (*m_event);
          *m_event = hit;
          outTree -> Fill();
          *m_event = temp;
        }
        else m_event->push_back(hit);
      }
      else return;
      tree->GetEntry(gindex[loop]);
    }
    setFirstHit(hit);

    // nb: m_rf -> setHit(hit) returns true if the hit was a RF downscale
  }
}

void EventBuilderRF::tryAddPreprompt_simple()
{
#ifdef PREPROMPT
  // First remove the hits that belongs to the event :
  for (int i = m_event->mult+1; i>0 && !m_hit_buffer.empty(); i--) m_hit_buffer.pop();
  if (m_hit_buffer.empty()) return;
  auto const RF_ref_stamp = m_rf->refTime(m_event->stamp);
  while (!m_hit_buffer.empty())
  {
    auto const & hit = m_hit_buffer.top();
    if (Time_cast(RF_ref_stamp-hit.stamp) < Time_cast(m_rf->period+m_rf->offset())) m_event -> push_front(hit);
    else break;
    m_hit_buffer.pop();
  }
  if (!m_hit_buffer.empty()) m_hit_buffer = std::stack<Hit>();
#endif //PREPROMPT
}


#endif //EVENT_BUILDER_136_HPP