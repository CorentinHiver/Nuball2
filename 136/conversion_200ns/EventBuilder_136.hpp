#ifndef EVENT_BUILDER_136_H
#define EVENT_BUILDER_136_H

#include <RF_Manager.hpp>
#include <Builder.hpp>
#include <Alignator.hpp>

class EventBuilder_136 : public Builder
{
public:
  // Constructors :
  EventBuilder_136(Event* _event) : Builder(_event) {}
  EventBuilder_136(Event* _event, RF_Manager* _rf) : Builder(_event), m_rf(_rf), m_period(m_rf->period) {}

  // Methods :
  // Add Hits  Outputs  0: single | 1: begin of coincidence | 2: coincidence complete
  bool build(Hit const & hit);

  bool coincidence(Hit const & hit) 
  {
    auto const & dT = Time_cast(hit.stamp-m_RF_ref_stamp);
    auto const & dT_max = Time_cast(m_period)-m_rf->offset();
    return (dT < dT_max);
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
  void tryAddPreprompt_simple();
  void tryAddNextHit_simple(TTree * tree, TTree * outtree, Hit & hit, int & loop, Alignator const & gindex);
  void setNbPeriodsMore(int const & periods) {m_nb_periods_more = periods;}

  Timestamp single_hit_VS_RF_ref = 0;
  
private:
  // Attributes :
  Timestamp m_RF_ref_stamp = 0;
  RF_Manager* m_rf = nullptr;
  std::stack<Hit> m_hit_buffer;
  int m_nb_periods_more = 0;
  Timestamp m_period = 200000ll;
};

void EventBuilder_136::tryAddNextHit_simple(TTree * tree, TTree * outtree, Hit & hit, int & loop, Alignator const & gindex)
{
  // If the next hits (for which the relative timestamp is greater than the RF period) are close enough to the event, 
  // they might come from an isomer deexcitation. Therefore, we would like to gather them in order to increase statistics.

  // The current hit is the one that "closed" the event, i.e. is outside of time window,
  // and has been set to the last_hit to serve for next EventBuilder_136::build() call
  // as the potential first hit of next event.
  // Therefore, the current m_RF_ref_stamp (pulse "t = zero") is relative to the current hit.
  // In order to check if the current hit is within the extended time window of the previous event (which is the one currently stored)
  // we need the pulse reference timestamp of the previous event :
  Timestamp const & RF_ref_stamp = m_rf->refTime(m_event->stamp);

  // First, try to add the current hit (the hit that closed the window) :
  if (Time_cast(hit.stamp-RF_ref_stamp) < Time_cast(m_period*(m_nb_periods_more+1) - m_rf->offset()))
  {
    m_event->push_back(hit);

    // If it worked, check if there is any hits left in the tree and load it :
    auto const & nb_hits = tree -> GetEntries();
    if (++loop>nb_hits-1) return;
    tree->GetEntry(gindex[loop]);

    // Then let's try to add the next hit if any :
    while(Time_cast(hit.stamp-RF_ref_stamp) < Time_cast(m_period*(m_nb_periods_more+1) - m_rf->offset()) )
    {
      if (++loop<nb_hits) 
      {// Take into  account the potential presence of RF downscale in 
        if(m_rf -> setHit(hit)) 
        {
          Event temp (*m_event);
          *m_event = hit;
          outtree -> Fill();
          *m_event = temp;
        }
        else m_event->push_back(hit);
      }
      else return;
      tree->GetEntry(gindex[loop]);
    }
    set_last_hit(hit);

    // nb: m_rf -> setHit(hit) returns true if the hit was a RF downscale
  }
}

void EventBuilder_136::tryAddPreprompt_simple()
{
  // First remove the hits that belongs to the event :
  for (int i = m_event->mult+1; i>0 && !m_hit_buffer.empty(); i--) m_hit_buffer.pop();
  if (m_hit_buffer.empty()) return;
  auto const RF_ref_stamp = m_rf->refTime(m_event->stamp);
  while (!m_hit_buffer.empty())
  {
    auto const & hit = m_hit_buffer.top();
    if (Time_cast(RF_ref_stamp-hit.stamp) < Time_cast(m_period+m_rf->offset())) m_event -> push_front(hit);
    else break;
    m_hit_buffer.pop();
  }
  if (!m_hit_buffer.empty()) m_hit_buffer = std::stack<Hit>();
}

bool EventBuilder_136::build(Hit const & hit)
{
  if(m_event->mult>255) reset(); // 255 is the maximum number of hits allowed inside of an event
  #ifdef PREPROMPT
    m_hit_buffer.emplace(hit);
  #endif //PREPROMPT
  switch (m_status)
  { 
    // If no coincidence has been detected in previous iteration :
    case 0 : case 2 : 
      if (this->coincidence(hit))
      {// Situation 1 :
        // The previous and current hit are in the same event.
        // In next call, we'll check if the next hits also belong to this event (situations 1' or 2)
        m_event -> clear();
       *m_event = m_last_hit;
        m_event->push_back(hit);
        m_last_hit.reset();
        m_status = 1; // The event can be filled with other hits
      }
      else
      {// Situation 0 :
        // The last and current hits aren't in the same event.
        // The last hit is therefore a single hit, alone in the time window around its RF.

        // The current hit is set to be the reference hit for next call :
        m_RF_ref_stamp = m_rf->refTime(hit.stamp);

        // m_last_hit is filled with the current hit :
        m_last_hit = hit;

        m_status = 0; // No event detected

        if (m_keep_singles) 
        {// We might be interested in registering single hits :
          *m_event = m_last_hit;
          return true;
        }
      }
    break;
    
    // If we are building an event (i.e. the two previous hit are in coincidence):
    case 1 :
      if (this->coincidence(hit))
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
        // Therefore, m_event is full with all the hits of the event.
        // We set the reference RF timestamp as the t0 of the event :
        m_event->setT0(m_RF_ref_stamp);
        m_status = 2; // The event is complete
        // The current hit is set to be the last hit for next call :
        this -> set_last_hit(hit);
        return true; // The event is ready to be processed
      }
    break;
    default: throw_error("event building issues, call DEV");
  }
  return false; // The event is not ready to be processed
}

void EventBuilder_136::setFirstRF(Hit const & rf_hit)
{
  if (rf_hit.label != RF_Manager::label) throw std::runtime_error("First RF hit of the file is not really a RF");
  m_rf -> setHit(rf_hit);
}

void EventBuilder_136::set_last_hit(Hit const & hit)
{
  // The closest RF to this hit is taken as reference to build the event :
  m_RF_ref_stamp = m_rf->refTime(hit.stamp);

  // m_last_hit is filled with the current hit :
  m_last_hit = hit;
}

void EventBuilder_136::reset()
{
  m_event -> clear(); 
  m_status = 0;
}

class Counter136
{
public:
  bool counted = false; 
  size_t nb_modules = 0;
  size_t nb_dssd = 0;
  size_t nb_Ge = 0;
  size_t nb_BGO = 0;
  size_t nb_clovers = 0;
  size_t nb_clovers_clean = 0;
  std::vector<int> clovers;
  std::vector<int> clovers_ge;
  std::vector<int> clovers_bgo;
  std::vector<int> clovers_clean_Ge;

  Counter136(Event* event) : m_event(event) {}

  void reset() noexcept  
  {
    counted = false;
    nb_modules = 0; 
    nb_dssd = 0; 
    nb_Ge = 0; 
    nb_BGO = 0;
    nb_clovers = 0;
    nb_clovers_clean = 0;
    clovers.clear();
    clovers_ge.clear();
    clovers_bgo.clear();
    clovers_clean_Ge.clear();
  }
  
  void count()
  {
    reset();
    for (int hit = 0; hit<m_event->mult; hit++)
    {
      auto const & label = m_event->labels[hit];
      if (isClover[label])
      {
        auto const & clover_label = int_cast(labelToClover[label]);
             if(isGe[label] ) {push_back_unique(clovers, clover_label); push_back_unique(clovers_ge , clover_label);}
        else if(isBGO[label]) {push_back_unique(clovers, clover_label); push_back_unique(clovers_bgo, clover_label);}
      }
      else if (isLaBr3[label]) ++nb_modules;
      else if (isParis[label]) ++nb_modules;
      else if (isDSSD[label])  ++nb_dssd;
    }
    nb_Ge  = clovers_ge .size();
    nb_BGO = clovers_bgo.size();
    nb_modules += (nb_clovers = clovers.size());
    counted = true;
  }

  void analyse()
  {
    if (!counted) this->count();
    for (auto & clover_i : clovers)
    {
      if (found(clovers_ge, clover_i) && !found(clovers_bgo, clover_i)) 
        clovers_clean_Ge.push_back(clover_i);
    }
    nb_clovers_clean = clovers_clean_Ge.size();
  }

private:
  Event * m_event = nullptr;
};


#endif //EVENT_BUILDER_136_H