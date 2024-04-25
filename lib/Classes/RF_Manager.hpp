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
 * Normal case : the RF reference timestamp is lower than the shifted timestamps (should also be the case of un-shifted timestamp)
 * dT = (shifted_time-last_downscale_timestamp) corresponds to the time separating the current hit to the reference RF
 * Therefore, N = dT/period corresponds to the number of periods separating the two hits
 * In other words, it is the number of pulses between the reference RF and the current hit
 * Then, period*N is the timestamp of the pulse relative to the current hit
 * (Remember we are doing integer arithmetic, period*dT/period != dT)
 * And dT%period is the rest of the integer division, hence the time between the hit and its relative pulse
 * Finally, one need to subtract the applied offset in order to get the correct result :
 * 
 * When the data is not correctly ordered :
 * In order to get a correct answer, one need to get a positive difference, so to invert the difference : now last_downscale_timestamp-shifted_time>0
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
  /// Use this when using the relative times are in ns (time2s)
  void align_to_RF_ns(Event & evt) const;
#endif //EVENT_HPP
  void static setOffset(Time const & offset) {m_offset = offset;}
  void static setOffset_ns(Time const & offset_ns) {m_offset = offset_ns*1000;}
  auto static offset_fs() {return Time_cast(m_offset*1000);}
  auto static const & offset() {return m_offset;}
  auto static offset_ns() {return double_cast(m_offset)/1000.;}
  auto period_ns() const {return double_cast(period)/1000.;}
  ///@brief Returns the value of the end of the time window ( = period-offset)
  auto lastTime_ns() const {return double_cast(period-m_offset)/1000.;}
  void setPeriod(Time const & _period) {period = _period*1000;}
  void setPeriod_ns(Time const & _period) {period = _period*1000;}
  /// @brief Returns the timestamp of the reference RF
  Timestamp refTime(Timestamp const & timestamp) const {return timestamp - relTime(timestamp);}

  Time relTime(Timestamp const & timestamp) const
  {
    if (period == 0) throw std::runtime_error("RF period = 0 !!!");

    // Shifts the timestamp in order to be able to get hits before the 0. Indeed, we calculate the relative time using a modulo. 
    // With no shift, the relative time is always positive (there cannot be negative result out of the modulo operation).
    // Therefore, the time is first shifted of m_offset picoseconds and then the modulo operation is performed.
    // Then the output is shifted back in order to obtain the correct relative time (-> the minimum relative time is -m_offset).
    Timestamp const & shifted_timestamp = timestamp + m_offset;
    // This is the time separating the shifted timestamp to the last downscaled RF
    // It is casted to Time (long long signed int) because it is supposed to fit in the range [-9.2e18, 9.2e18] for x64 architectures (should be ok for 32 bits but be careful)
    // In the frame of this code, Time is used to represent the relative times in picoseconds
    auto const & rf_time = Time_cast(shifted_timestamp-last_downscale_timestamp); 

    // Now, there are two possibilities. Normally, the reference timestamp is lower than the shifted timestamp.
    // But in some cases, the reference timestamp is higher than the shifted timestamp (when looking "in the past").
    // The following allows one to have valid relative times for both situations :
    if (shifted_timestamp>=last_downscale_timestamp)
    {// Normal case
      auto const & relative_time = Time_cast((rf_time%period)); // This is the time separating the shifted timestamp to the t0 of the current pulse.
      return relative_time - m_offset; // Shifts back to obtain the correct relative time
    }
    else if (rf_time<0)
    {// When the RF timestamp is larger than the hit's
      auto const & relative_time = Time_cast((period - (-rf_time)%period)); 
      return relative_time - m_offset;
    }
    else 
    {
      error ("rf_time ")
      throw_error("FATAL : inconsistency in the timestamps !!! in RF_Manager::relTime()");
      return 0;
    }
  }
  Time relTime(Hit const & hit) const {return relTime(hit.stamp);}

  template<class... ARGS>
  float relTime_ns(ARGS... args) {return (static_cast<float>(relTime(std::forward<ARGS>(args)...))/1000.f);}

  void inline set(Timestamp new_timestamp, Timestamp _period) 
  {
    auto const & tPeriod = Time_cast(new_timestamp-last_downscale_timestamp);

    // The RF is downscaled every 1000 RF pulses. 
    // There are two cases : either the RF is indeed downscaled as expected 
    // and the difference of timestamps should be in the order of 1000 RF pulses:
    if (tPeriod > 0 && tPeriod<Time_cast(1.5*double_cast(period))*1000000) period = Time_cast(double_cast(tPeriod)/1000.);

    // If it is not, then use the period provided by the user :
    else period = double_cast(_period);

    // Either way, we need to update the timestamp of the last downscale RF :
    last_downscale_timestamp = new_timestamp;
  }


  // Attributes :

  Timestamp last_downscale_timestamp = 0;

  Time period = 0;

  static Label label;

private:
  static Time m_offset;
};

Time  RF_Manager::m_offset = 50000;
Label RF_Manager::label = 251;

std::ostream& operator<<(std::ostream& cout, RF_Manager const & rf)
{
  cout << "period : " << rf.period << "ps last timestamp : " << rf.last_downscale_timestamp << "ps" << std::endl;
  return cout;
}

bool inline RF_Manager::setHit(Hit const & hit)
{
  if (hit.label == this -> label)
  {
    if (hit.adc == 0) this -> set(hit.stamp,Timestamp_cast(hit.nrj));
    else              this -> set(hit.stamp,Timestamp_cast(hit.adc));
    return true;
  }
  else return false;
}

#ifdef EVENT_HPP
  /// @brief Set an event containing the downscale hit of RF
  bool inline RF_Manager::setEvent(Event const & event)
  {
    if (event.labels[0] == this -> label)
    {
      
      if (event.adcs[0] == 0) this -> set(event.stamp,Timestamp_cast(event.nrjs[0]));
      else                    this -> set(event.stamp,Timestamp_cast(event.adcs[0]));
      return true;
    }
    else return false;
  }

  bool inline RF_Manager::setHit(Event const & event, int const & hit_i)
  {
    if (event.labels[hit_i] == RF_Manager::label)
    {
      auto const & new_timestamp = event.stamp + event.times[hit_i];
      if (event.adcs[0] == 0) this -> set(new_timestamp, Timestamp_cast(event.nrjs[0]));
      else                    this -> set(new_timestamp, Timestamp_cast(event.adcs[0]));
      return true;
    }
    else return false;
  }

  void inline RF_Manager::align_to_RF(Event & event) const
  {
    event.setT0(relTime(event.stamp));
  }
#endif //EVENT_HPP

//--------------------//
//--- Helper class ---//
//--------------------//

/// @brief Helper class for applying RF_Manager to various interfaces
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
