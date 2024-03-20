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
 * dT = (shifted_time-last_downscale) corresponds to the time separating the current hit to the reference RF
 * Therefore, N = dT/period corresponds to the number of periods separating the two hits
 * In other words, it is the number of pulses between the reference RF and the current hit
 * Then, period*N is the timestamp of the pulse relative to the current hit
 * (Remember we are doing integer arithmetic, period*dT/period != dT)
 * And dT%period is the rest of the integer division, hence the time between the hit and its relative pulse
 * Finally, one need to substract the applied offset in order to get the correct result :
 * 
 * When the data is not correctly ordered :
 * In order to get a correct answer, one need to get a positive difference, so to invert the difference : now last_downscale-shifted_time>0
 * But the result is inverted and we obtain really period-timestamp. We get the correct result by doing :
 * relative_timestamp = period - (period-timestamp)
 * 
*/
class RF_Manager
{
public:
  RF_Manager(Time const & _period = 400000, Label const & label_RF = 251) {label = label_RF; period = _period;}
  bool setHit(Hit const & hit);
#ifdef EVENT_HPP
  bool setHit(Event const & event, int const & hit_i);
  bool setEvent(Event const & evt);
  void align_to_RF(Event & evt) const;
  void align_to_RF_ns(Event & evt) const;
#endif //EVENT_HPP
  void static set_offset(Time const & offset) {m_offset = offset;}
  void static set_offset_ns(Time const & offset_ns) {m_offset = offset_ns*1000;}
  auto static const & offset() {return m_offset;}
  auto static offset_ns() {return m_offset/1000.;}
  auto period_ns() const {return period/1000.;}
  ///@brief Returns the value of the end of the time window ( = period-offset)
  auto last_time_ns() const {return (period-m_offset)/1000.;}
  void set_period_ns(Time const & _period) {period = _period*1000;}
  Timestamp refTime(Timestamp const & timestamp) const {return timestamp - pulse_ToF(timestamp);}

  Time pulse_ToF(Timestamp const & timestamp) const
  {
    // Shifts the timestamp in order to be able to get hits before the 0 :
    Timestamp const & shifted_timestamp = timestamp + m_offset; 

    if (period == 0) throw std::runtime_error("RF period = 0 !!!");

    // auto const & period_fm = Timestamp_cast(period*1000);

    if (shifted_timestamp>last_downscale)
    {// Normal case
      auto const & rf_time = (shifted_timestamp-last_downscale);
      auto const & relative_time = Time_cast((rf_time%period));
      return relative_time - m_offset;
    }
    else
    {// When the RF is found after the hit
      auto const & reversed_rf_time = (last_downscale-timestamp-m_offset);
      auto const & relative_time =Time_cast((period - (reversed_rf_time)%period));
      return relative_time - m_offset;
    }
  }
  Time pulse_ToF(Hit const & hit) const {return pulse_ToF(hit.stamp);}

  template<class... ARGS>
  float pulse_ToF_ns(ARGS... args) {return (static_cast<float>(pulse_ToF(std::forward<ARGS>(args)...))/1000.f);}

  void inline set(Timestamp new_timestamp, Timestamp _period) 
  {
    auto const & tperiod = Time_cast(new_timestamp-last_downscale);
    if (tperiod > 0 && tperiod<1.5*period*1000000) period = double_cast(tperiod)/1000.;
    else period = double_cast(_period);
    last_downscale = new_timestamp;
  }


  // Attributes :

  Timestamp last_downscale = 0;

  Time period = 0;

  thread_local static Label label;

private:
  thread_local static Time m_offset;
};

thread_local Time  RF_Manager::m_offset = 50000 ;
thread_local Label RF_Manager::label  = 251;

bool inline RF_Manager::setHit(Hit const & hit)
{
  if (hit.label == this -> label)
  {
    this -> set(hit.stamp, Timestamp_cast((hit.adc == 0) ? hit.nrj : hit.adc));
    return true;
  }
  else return false;
}

  #ifdef EVENT_HPP
/**
 * @brief Set an event containing only one hit
 */
bool inline RF_Manager::setEvent(Event const & event)
{
  if (event.labels[0] == this -> label)
  {
    this -> set(event.stamp, Timestamp_cast((event.adcs[0] == 0) ? event.nrjs[0] : event.adcs[0]));
    return true;
  }
  else return false;
}

bool inline RF_Manager::setHit(Event const & event, int const & hit_i)
{
  if (event.labels[hit_i] == RF_Manager::label)
  {
    this -> set(
      event.stamp + event.times[hit_i], 
      Timestamp_cast((event.adcs[hit_i] == 0) ? event.nrjs[hit_i] : event.adcs[hit_i])
    );
    return true;
  }
  else return false;
}

void inline RF_Manager::align_to_RF(Event & event) const
{
  event.setT0(pulse_ToF(event.stamp));
}

void inline RF_Manager::align_to_RF_ns(Event & event) const
{
  auto const & rf_Ref = pulse_ToF(event.stamp);
  event.stamp -= rf_Ref;
  for (int i = 0; i<event.mult; i++)
  {
    event.times[i] += rf_Ref;
    event.time2s[i] = Time_ns_cast(rf_Ref + event.times[i])/1000.;
  }
}
  #endif //EVENT_HPP

std::ostream& operator<<(std::ostream& cout, RF_Manager const & rf)
{
  cout << "period : " << rf.period << "ps last timestamp : " << rf.last_downscale << "ps" << std::endl;
  return cout;
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
  RF_Extractor(TTree * tree, RF_Manager & rf, Event & event, Long64_t maxEvts = Long64_cast(1.E+7));
#endif //EVENT_HPP

#ifdef FASTERREADER_HPP
  RF_Extractor(FasterReader & reader, RF_Manager & rf, Hit & hit, Long64_t maxEvts = Long64_cast(1.E+7));
#endif //FASTERREADER_HPP

  auto const & cursor() const {return m_cursor;}

  operator bool() const & {return m_ok;}

private:
  bool m_ok = false;
  int m_cursor = 0;
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
  while(!rf.setEvent(event) && m_cursor<maxEvts);
  if (m_cursor == maxEvts) {print("NO RF DATA FOUND !"); m_ok = false; return;}
  m_ok = true;
}
#endif //EVENT_HPP

#ifdef FASTERREADER_HPP // TODO
// RF_Extractor::RF_Extractor(FasterReader & reader, RF_Manager & rf, Hit & hit, Long64_t maxEvts);
// {
//   do {reader.Read();}
//   while(hit.label != RF_Manager::label && m_cursor<maxEvts );
//   if (m_cursor == maxEvts) {print("NO RF DATA FOUND !"); m_ok = false; return;}
//   rf.setHit(hit);
//   m_ok = true;
//   reader.Reset();
// }
#endif //FASTERREADER_HPP

#endif //RF_MANAGER_H
