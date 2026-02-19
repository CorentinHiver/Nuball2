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
  void reset();

  // Getters :
  auto const & getShift() const {return m_shift;}
  auto const & get_RF_ref_time() const {return RF_ref_time;}

  // Setters :
  void setShift(Long64_t const & shift) {m_shift = shift;}
  void setShift_ns(Long64_t const & shift) {m_shift = 1000*shift;}
  void setRF(RF_Manager* rf) {m_rf = rf;}
  void setFirstRF(Hit const & hit);
  void setRFTime(Time const & _RF_ref_time) {RF_ref_time = _RF_ref_time;}
  inline void set_last_hit (Hit const & hit);
  inline void set_first_hit(Hit const & hit){set_last_hit(hit);}

private:
  // Attributes :
  Time RF_ref_time = 0;
  RF_Manager* m_rf = nullptr;
  Time m_shift = 0;
};

bool EventBuilder_136::build(Hit const & hit)
{
  if(m_event->mult>255) reset(); // 255 is the maximum number of hits allowed inside of an event
  if (!coincON)
  { // If no coincidence has been detected in previous iteration :
    if ( (hit.time-RF_ref_time) < static_cast<ULong64_t>(m_rf->period) )
    {// Case 1 :
      // The previous and current hit are in the same event.
      // In next call, we'll check if the next hits are in the event or not (cases 3 or 4)
      m_event -> clear();
    #ifdef PREPROMPT
      if ( (RF_ref_time-m_single_hit.time) < static_cast<ULong64_t>(m_rf->period))
          m_event -> push_back(m_single_hit);
    #endif //PREPROMPT
      m_event -> push_back(m_last_hit);
      m_event -> push_back(hit);
      m_last_hit.reset();
      coincON = true; // Open the event
      m_status = 1; // The event can be filled with potential additionnal hits
    }
    else
    {// Case 2 :
      // The last and current hits aren't in the same event.
      // This means either the last hit is filled with an empty hit if last hit closed an event,
      // or a single hit : a lonely hit, alone in the time window around its RF.
      m_single_hit = m_last_hit;
      // The current hit is set to be the last hit for next call :
      this -> set_last_hit(hit);
      m_status = 0; // No event detected
    }
  }

  else
  { // If the two previous hits are in the same event, checking if the current hit is :
    if ( (hit.time-RF_ref_time) < static_cast<ULong64_t>(m_rf->period) )
    {// Case 3 :
      // print(hit.time, hit.label, (hit.time-RF_ref_time)/1000., m_rf->period/1000.);
      // The current hit belongs to the event
      m_event -> push_back(hit);
      // NB: m_status still equals to 1
    }
    else
    {// Case 4 :
      // The current hit is outside of the event : this hit closes the event :
      coincON = false; // The coincidence is closed
      // Therefore, m_event is full with all the hits of the event :
      m_status = 2; // The event is complete
      // The current hit is set to be the last hit for next call :
      m_last_hit = hit;
      return true; // The event is ready to be processed
    }
  }
  return false; // No processing of the data
}

void EventBuilder_136::setFirstRF(Hit const & rf_hit)
{
  if (rf_hit.label != 251) throw std::runtime_error("First RF hit of the file is not really a RF");
  m_rf -> setHit(rf_hit);
  RF_ref_time = rf_hit.time;
}

void EventBuilder_136::set_last_hit(Hit const & hit)
{
  // The closest RF to this hit is taken as reference to build the event :
  RF_ref_time = hit.time - (hit.time - m_rf->last_hit + m_shift) % m_rf->period;
  // m_last_hit is filled with the current hit :
  m_last_hit = hit;
}

void EventBuilder_136::reset()
{
  // Sets the reference RF timestamp to the last hit :
  RF_ref_time = m_last_hit.time - (m_last_hit.time-m_rf->last_hit + m_shift)%m_rf->period ;
  // Then
  m_event -> clear(); m_status = 0;
}


class Counter136
{
public:
  int modules = 0;
  int nb_Ge = 0;
  std::vector<Label> clovers;

  void reset() 
  {
    modules = 0; 
    nb_Ge = 0; 
    clovers.resize(0);
  }
  
  void count(Event const & event)
  {
    reset();
    for (size_t hit = 0; hit<event.size(); hit++)
    {
      auto const & label = event.labels[hit];
      if (isGe[label]) {nb_Ge++; push_back_unique(clovers, labelToClover[label]);}
      else if(isBGO[label])      push_back_unique(clovers, labelToClover[label]);
      else if (isLaBr3[label] || isParis[label]) modules++;
    }
    modules+=clovers.size();
  }
};


#endif //EVENT_BUILDER_136_H
