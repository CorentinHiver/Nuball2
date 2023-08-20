#ifndef EVENT_BUILDER_136_H
#define EVENT_BUILDER_136_H

#include <Builder.hpp>
#include <RF_Manager.hpp>

class EventBuilder_136 : public Builder
{
public:
  // Constructors :
  EventBuilder_136(Event* _event) {m_event  = _event;}
  EventBuilder_136(Event* _event, RF_Manager* _rf) : m_rf(_rf) {m_event = _event;}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  bool build(Hit const & _hit);
  bool inTimeRange(Hit const & hit) 
  {
    return (Time_cast(hit.stamp-m_RF_ref_stamp) < Time_cast(m_rf->period) - m_rf->offset());
  }
  void reset();

  // Getters :
  // auto const & getShift() const {return m_shift;}
  auto const & get_RF_ref_time() const {return m_RF_ref_stamp;}

  // Setters :
  void setRF(RF_Manager* rf) {m_rf = rf;}
  void setFirstRF(Hit const & hit);
  void setRFRefTime(Timestamp const & _RF_ref_stamp) {m_RF_ref_stamp = _RF_ref_stamp;}
  inline void set_last_hit (Hit const & hit);
  inline void set_first_hit(Hit const & hit){set_last_hit(hit);}

  /// @brief Experimental : add more period after a trigger 
  bool tryAddNextHit_simple(uchar const & nb_periods_more);

  Timestamp single_hit_VS_RF_ref = 0;
private:
  // Attributes :
  Timestamp m_RF_ref_stamp = 0;
  RF_Manager* m_rf = nullptr;
  // Time m_shift = 0;
};

bool EventBuilder_136::tryAddNextHit_simple(uchar const & nb_periods_more)
{
  // If the next hit (for which the relative timestamp is greater than the RF period) is close enough to the event, it might be an isomer deexcitation.
  // Therefore, we would like to have a look at it in order to increase statistics.
  // If the next hit is in the time region of the next pulse then one should definitely remove it.
  // Here we keep it because it might still contain some information, it depends on the probability of 
  // 2 consecutive pulses to produce parasitic reaction
  if (Time_cast(m_last_hit.time-m_RF_ref_stamp) < Time_cast(m_rf->period*nb_periods_more))
  {
  }
  return false;
}

// void push_back_136(Event * event, Hit const & hit)
// {
//   auto & mult = event->mult;
//   mult++;
//   event->labels  [mult] = hit.label;
//   event->times   [mult] = hit.stamp;
//   event->nrjs    [mult] = hit.nrj;
//   event->nrj2s   [mult] = hit.nrj2;
//   event->pileups [mult] = hit.pileup;
// }

bool EventBuilder_136::build(Hit const & hit)
{
  if(m_event->mult>255) reset(); // 255 is the maximum number of hits allowed inside of an event
    // debug(Time_cast(hit.stamp-m_RF_ref_stamp));
  switch (m_status)
  { 
    case 0 : case 2 : // If no coincidence has been detected in previous iteration :
      if (this->inTimeRange(hit))
      {// Situation 1 :
        // The previous and current hit are in the same event.
        // In next call, we'll check if the next hits also belong to this event (cases 3 or 4)
        m_event -> clear();
      #ifdef PREPROMPT
        if ( Time_cast(m_RF_ref_stamp-m_single_hit.time) < Time_cast(m_rf->period))
            m_event -> push_front(m_single_hit);
      #endif //PREPROMPT
        *m_event = m_last_hit;
        m_event->push_back(hit);
        m_last_hit.reset();
        m_status = 1; // The event can be filled with potential additionnal hits
      }
      else
      {// Situation 0 :
        // The last and current hits aren't in the same event.
        // The last hit is therefore a single hit, alone in the time window around its RF.
        m_single_hit = m_last_hit;
        // The current hit is set to be the last hit for next call :
        this -> set_last_hit(hit);
        m_status = 0; // No event detected
      }
    break;
    case 1: // If we are building an event (i.e. the two previous hit are in coincidence):
      if (this->inTimeRange(hit))
      {
        // Situation 1' :
        // The current hit also belongs to the event
        m_event->push_back(hit);
        // NB: m_status still equals to 1
      }
      else
      {
        // Situation 2 :
        // The current hit is outside of the event : this hit closes the event 
        // Therefore, m_event is full with all the hits of the event :
        m_status = 2; // The event is complete
        // The current hit is set to be the last hit for next call :
        this -> set_last_hit(hit);
        return true; // The event is ready to be processed
      }
    break;
  }
  return false; // No processing of the data
}

void EventBuilder_136::setFirstRF(Hit const & rf_hit)
{
  if (rf_hit.label != RF_Manager::label) throw std::runtime_error("First RF hit of the file is not really a RF");
  m_rf -> setHit(rf_hit);
  m_RF_ref_stamp = rf_hit.stamp;
}

void EventBuilder_136::set_last_hit(Hit const & hit)
{
  // The closest RF to this hit is taken as reference to build the event :
  m_RF_ref_stamp = hit.stamp - m_rf->pulse_ToF(hit.stamp);
  // We substract the offset because we want the hits to be in [-offset, period-offset].
  // We achieve this by shifting back the reference by -offset
  // Therefore the comparison (hit_stamp-ref)<period gives in effect (relative_time+offset)<period

  // m_last_hit is filled with the current hit :
  m_last_hit = hit;
}

void EventBuilder_136::reset()
{
  m_event -> clear(); m_status = 0;
}


class Counter136
{
public:
  size_t nb_modules = 0;
  size_t nb_dssd = 0;
  size_t nb_Ge = 0;
  size_t nb_clovers = 0;
  std::vector<Label> clovers;

  void reset() 
  {
    nb_modules = 0; 
    nb_dssd = 0; 
    nb_Ge = 0; 
    nb_clovers = 0;
    clovers.resize(0);
  }
  
  void count(Event const & event)
  {
    reset();
    for (Mult hit = 0; hit<event.mult; hit++)
    {
      auto const & label = event.labels[hit];
      if (isGe[label]) {nb_Ge++; push_back_unique(clovers, labelToClover[label]);}
      else if(isBGO[label])      push_back_unique(clovers, labelToClover[label]);
      else if (isLaBr3[label] || isParis[label]) nb_modules++;
      else if (isDSSD[label])                    nb_dssd++;
    }
    nb_clovers = clovers.size();
    nb_modules+=nb_clovers;
  }
};


#endif //EVENT_BUILDER_136_H
