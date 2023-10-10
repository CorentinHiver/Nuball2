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
#ifdef EVENT_HPP
  bool setHit(Event const & event, int const & hit_i);
  bool setEvent(Event const & evt);
  void align_to_RF(Event & evt) const;
  void align_to_RF_ns(Event & evt) const;
#endif //EVENT_HPP
  void static set_offset(Time const & offset) {m_offset = offset;}
  void static set_offset_ns(Time const & offset_ns) {m_offset = offset_ns*1000;}
  auto static offset() {return m_offset;}
  Timestamp refTime(Timestamp const & timestamp) const {return timestamp - pulse_ToF(timestamp);}

  Time pulse_ToF(Timestamp const & timestamp) const
  {
  #ifdef USE_RF

    // Shifts the timestamp in order to be able to get hits before the 0 :
    Timestamp const & shifted_timestamp = timestamp + m_offset; 

    if (period == 0) throw std::runtime_error("RF period = 0 !!!");
    auto const & period_fm = Timestamp_cast(period*1000);

    if (shifted_timestamp>last_hit)
    {// Normal case
      auto const & rf_time = (shifted_timestamp-last_hit)*1000ull;
      auto const & relative_time = Time_cast((rf_time%period_fm)/1000ull);
      // print(period_fm, rf_time, relative_time, relative_time - m_offset);
      // pauseCo();
      return relative_time - m_offset ;
    }
    else
    {// When the RF is found after the hit
      auto const & reversed_rf_time = (last_hit-timestamp-m_offset)*1000ull;
      auto const & relative_time =Time_cast((period_fm - (reversed_rf_time)%period_fm)/1000ull);
      return relative_time - m_offset;
    }

  #else //NO USE_RF
    print("NO RF IS USED !! Please set USE_RF [period_value]");
    return Time_cast(timestamp);
  #endif //USE_RF
  }
  Time pulse_ToF(Hit const & hit) const {return pulse_ToF(hit.stamp);}

  Time pulse_ToF(Hit const & hit, Time const & offset) const {return pulse_ToF(hit.stamp, offset);}
  Time pulse_ToF(Timestamp const & timestamp, Time const & offset) const {return pulse_ToF(timestamp, offset);}

  template<class... ARGS>
  float pulse_ToF_ns(ARGS... args) {return (static_cast<float>(pulse_ToF(std::forward<ARGS>(args)...))/1000.f);}

  void set(Timestamp new_timestamp, Timestamp _period) 
  {
  #ifdef USE_RF
    auto const & tperiod = Time_cast(new_timestamp-last_hit);
    if (tperiod > 0 && tperiod<1.5*USE_RF*1000000) period = double_cast(tperiod)/1000.;
    else period = double_cast(_period);
    last_hit = new_timestamp;
  #endif //USE_RF
  }


  // Attributes :

  Timestamp last_hit = 0;

#ifdef USE_RF
  double period = USE_RF*1000;
#else //NO USE_RF
  Time period = 0;
#endif //USE_RF

  static Label label;

private:
  Time static m_offset;
};

Time  RF_Manager::m_offset = 50000 ;
Label RF_Manager::label  = 251;

bool RF_Manager::setHit(Hit const & hit)
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
bool RF_Manager::setEvent(Event const & event)
{
  if (event.labels[0] == this -> label)
  {
    this -> set(event.stamp, Timestamp_cast((event.adcs[0] == 0) ? event.nrjs[0] : event.adcs[0]));
    return true;
  }
  else return false;
}

bool RF_Manager::setHit(Event const & event, int const & hit_i)
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

void RF_Manager::align_to_RF(Event & event) const
{
  auto const & rf_Ref = pulse_ToF(event.stamp);
  event.stamp -= rf_Ref;
  for (int i = 0; i<event.mult; i++) event.times[i] += rf_Ref;
}

void RF_Manager::align_to_RF_ns(Event & event) const
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
  cout << "period : " << rf.period << "ps last timestamp : " << rf.last_hit << "ps" << std::endl;
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
