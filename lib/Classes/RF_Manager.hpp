#ifndef RF_MANAGER_H
#define RF_MANAGER_H

#include "../libCo.hpp"

class RF_Manager
{
public:
  RF_Manager(ushort const & label_RF = 251) {label = label_RF;}
  Bool_t setHit(Hit const & hit);
  void static set_offset(Long64_t const & offset) {m_offset = offset;}
  // Bool_t setHit(Event const & event, int const & index);

  Long64_t pulse_ToF(ULong64_t const & time, Long64_t const & offset = 0) const
  {
  #ifdef USE_RF
    // Shifts the time in order to be able to get hits before the hit :
    ULong64_t const shifted_time = time+offset;
    if (shifted_time>last_hit)
    {// Normal case : the RF reference timestamp is lower than the shifted timestamps (should also be the case of unshifted timestamp)
      // dT = (shifted_time-last_hit) corresponds to the time separating the current hit to the reference RF
      // Therefore, N = dT/period corresponds to the number of periods separating the two hits
      // In other words, it is the number of pulses between the reference RF and the current hit
      // Then, period*N is the timestamp of the pulse relative to the current hit
      // (Remember we are doing integer arithmetic, period*dT/period != dT)
      // And dT%period is the rest of the integer division, hence the time between the hit and its relative pulse
      // Finally, one need to substract the applied offset in order to get the correct result :
      return ( static_cast<Long64_t> ((time+offset-last_hit)%period)-offset );
    }
    else
    {// When the data is not correctly ordered :
      // In order to get a correct answer, one need to get a positive difference, so to invert the difference : now last_hit-shifted_time>0
      // But the result is inverted and we obtain really period-timestamp. We get the correct result by doing :
      // relative_timestamp = period - (period-timestamp)
      return ( static_cast<Long64_t> (period-(last_hit-time-offset)%period)-offset );
    }
  #else //NO USE_RF
    print("NO RF IS USED !! Please set RF_USE [period_value]");
    return static_cast<Long64_t>(time)-offset;
  #endif //USE_RF
  }

  Long64_t pulse_ToF(Hit const & hit, Long64_t const & offset = 0) const
  {
    return pulse_ToF(hit.time, offset);
  }

  Long64_t pulse_ToF(Hit const & hit) const
  {
    return pulse_ToF(hit.time, m_offset);
  }

  Bool_t isPrompt(Hit const & hit, Long64_t const & borneMin, Long64_t const & borneMax)
  {
    return (pulse_ToF(hit,-borneMin) < borneMax);
  }

  ULong64_t last_hit = 0;

#ifdef USE_RF
  ULong64_t period   = USE_RF;
#else //NO USE_RF
  ULong64_t period   = 0;
#endif //USE_RF

  static ushort label;

private:
  Long64_t static m_offset;
};

Long64_t RF_Manager::m_offset = 50 ;
ushort   RF_Manager::label  = 251;

Bool_t RF_Manager::setHit(Hit const & hit)
{
  if (hit.label == RF_Manager::label)
  {
    last_hit = hit.time;
    period = hit.nrjcal;
    return true;
  }
  else return false;
}

#endif //RF_MANAGER_H
