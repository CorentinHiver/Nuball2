#ifndef RF_MANAGER_H
#define RF_MANAGER_H

#include "../libCo.hpp"
#include "../libRoot.hpp"
#include "Event.hpp"

/**
 * @brief Class used to manage the RF
 * 
 * @details
 * 
 * Normal case : the RF reference timestamp is lower than the shifted timestamps (should also be the case of unshifted timestamp)
 * dT = (shifted_time-last_hit) corresponds to the time separating the current hit to the reference RF
 * Therefore, N = dT/period corresponds to the number of periods separating the two hits
 * In other words, it is the number of pulses between the reference RF and the current hit
 * Then, period*N is the timestamp of the pulse relative to the current hit
 * (Remember we are doing integer arithmetic, period*dT/period != dT)
 * And dT%period is the rest of the integer division, hence the time between the hit and its relative pulse
 * Finally, one need to substract the applied offset in order to get the correct result :
 * 
 * When the data is not correctly ordered :
 * In order to get a correct answer, one need to get a positive difference, so to invert the difference : now last_hit-shifted_time>0
 * But the result is inverted and we obtain really period-timestamp. We get the correct result by doing :
 * relative_timestamp = period - (period-timestamp)
 * 
*/
class RF_Manager
{
public:
  RF_Manager(Label const & label_RF = 251) {label = label_RF;}
  bool setHit(Hit const & hit);
  bool setHit(Event const & event, uint const & pos = 0);
  void static set_offset(Long64_t const & offset) {m_offset = offset;}
  void static set_offset_ns(Long64_t const & offset_ns) {m_offset = offset_ns*1000;}
  auto static offset() {return m_offset;}

  Long64_t pulse_ToF(ULong64_t const & time, Long64_t const & offset) const
  {
  #ifdef USE_RF

    // Shifts the time in order to be able to get hits before the 0 :
    ULong64_t const shifted_time = time+offset;

    if (period == 0) throw std::runtime_error("RF period = 0 !!!");

    if (shifted_time>last_hit)
    {// Normal case
      return ( static_cast<Long64_t>((shifted_time-last_hit)%period) - offset );
    }
    else
    {// When the RF is found after the hit (rf not in correct order)
      return ( static_cast<Long64_t> (period-(last_hit-time-offset)%period)-offset );
    }
  #else //NO USE_RF
    print("NO RF IS USED !! Please set USE_RF [period_value]");
    return static_cast<Long64_t>(time);
  #endif //USE_RF
  }

  Long64_t pulse_ToF(Hit const & hit, Long64_t const & offset) const {return pulse_ToF(hit.time,   offset);}
  Long64_t pulse_ToF(Hit const & hit                         ) const {return pulse_ToF(hit.time, m_offset);}
  Long64_t pulse_ToF(ULong64_t const & time                  ) const {return pulse_ToF(    time, m_offset);}

  template<class... ARGS>
  float pulse_ToF_ns(ARGS... args) {return (static_cast<float>(pulse_ToF(std::forward<ARGS>(args)...))/1000.f);}

  bool isPrompt(Hit const & hit, Long64_t const & borneMin, Long64_t const & borneMax)
  {
    return (pulse_ToF(hit,-borneMin) < borneMax);
  }

  void set(ULong64_t _last_hit, ULong64_t _period) {last_hit = _last_hit; period = _period;}

  ULong64_t last_hit = 0;

#ifdef USE_RF
  ULong64_t period = USE_RF;
#else //NO USE_RF
  ULong64_t period = 0;
#endif //USE_RF

  static Label label;

private:
  Long64_t static m_offset;
};

Long64_t RF_Manager::m_offset = 50000 ;
Label    RF_Manager::label  = 251;

bool RF_Manager::setHit(Hit const & hit)
{
  if (hit.label == RF_Manager::label)
  {
    last_hit = hit.time;
    period = hit.nrj;
    if (hit.nrj == 0) period = hit.nrjcal;
    return true;
  }
  else return false;
}

bool RF_Manager::setHit(Event const & event, uint const & i)
{
  if (event.labels[i] == RF_Manager::label)
  {
    last_hit = event.times[i];
    period = event.nrjs[i];
    return true;
  }
  else return false;
}


//--------------------//
//--- Helper class ---//
//--------------------//

class RF_Extractor
{
public:

#ifdef ALIGNATOR_HPP
  RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator const & gindex);
#endif //ALIGNATOR_HPP

#ifdef EVENT_HPP
  RF_Extractor(TTree * tree, RF_Manager & rf, Event & event, Long64_t maxEvts = (Long64_t)(1.E+7));
#endif //EVENT_HPP

  auto const & cursor() const {return m_cursor;}

  operator bool() const & {return m_ok;}

private:
  bool m_ok = false;
  Long64_t m_cursor = 0;
};

#ifdef ALIGNATOR_HPP
RF_Extractor::RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator const & gindex)
{
  auto const & nb_data = tree->GetEntries();
  do {tree -> GetEntry(gindex[m_cursor++]);}
  while(hit.label != RF_Manager::label && m_cursor<nb_data);
  if (m_cursor == nb_data) {print("NO RF DATA FOUND !"); m_ok = false; return;}
  rf.setHit(hit);
  m_ok = true;
}
#endif //ALIGNATOR_HPP

#ifdef EVENT_HPP
RF_Extractor::RF_Extractor(TTree * tree, RF_Manager & rf, Event & event, Long64_t maxEvts)
{
  do {tree -> GetEntry(m_cursor++);}
  while(event.labels[0] != RF_Manager::label && m_cursor<maxEvts);
  if (m_cursor == maxEvts) {print("NO RF DATA FOUND !"); m_ok = false; return;}
  rf.setHit(event);
  m_ok = true;
}
#endif //EVENT_HPP

#endif //RF_MANAGER_H
