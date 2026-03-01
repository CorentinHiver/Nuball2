#pragma once
#define EVENT_HPP

#include "Hit.hpp"

#ifdef COMULTITHREADING
  std::mutex mutex_events;
#endif //COMULTITHREADING

/**
 * @brief NOT UP TO DATE !!! Event object used for reading and writing event from/to root files in Nuball2-like TTree format, 
 * perform event building and trigger for faster to root conversion and data analysis.
 * 
 * @details
 * 
 * An Event is in principle a collection of Hits, usually in a timing correlation. 
 * In order to be an efficient interface with ROOT TTree, in practice it is implemented as
 * a collections of arrays that stores the various fields of the Hit class (see documentation of the later).
 * 
 * The following are public members (i.e. you can call it directly with Event::[array_name])
 * 
 *        int mult = 0;                   Number of hits currently stored in the event.
 *        Timestamp stamp = 0ull;         Absolute timestamp of the whole event unsigned long long
 *        Label   labels  [255] = {0};    Labels of the hits
 *        Time    times   [255] = {0};    Time in ps (Long64_t = long long in x64 computers) relative either to the first hit or to the pulse 
 *        ADC     adcs    [255] = {0};    Uncalibrated ADC value of the energy
 *        NRJ     nrjs    [255] = {0};    Calibrated (or simply gain matched) energy value in keV
 *        ADC     qdc2s   [255] = {0};    Uncalibrated QDC value of the energy measured in the second time gate for QDC2 channels
 *        NRJ     nrj2s   [255] = {0};    Calibrated (or simply gain matched) energy value in keV. By default, the energy calibration is the same as the nrj so that the ratio remains the same
 *        Pileup  pileups [255] = {0};    Pileup or saturation bit
 * 
 * Format of a branch inside the root tree : 
 * 
 *        "array[mult]/type"
 * 
 * Only the two first members - the multiplicity and the timestamp - are stored in the branch as variables and not as arrays.
 * 
 * To read any Nuball2-like TTree : 
 * 
 *        // Loads the TTree or TChain ...
 *        Event event;
 *        event.reading(tree);
 * 
 * If you are only interested in a few branches, you can choose them by adding an option : 
 * 
 *        event.reading(tree, "mltT"); // Reads only the multiplicity, label, timestamp and relative time.
 * 
 * Note the multiplicity will always be activated, because it is mandatory to read the root file.
 * 
 * A list of all the options available so far :
 * 
 *        m : mult   multiplicity          int
 *        l : label  label                 ushort
 *        t : stamp  absolute timestamp ps ULong64_t
 *        T : time   relative timestamp ps Long64_t
 *        e : adc    energy in ADC         int
 *        E : nrj    energy in keV         float
 *        q : qdc2   energy qdc2 in ADC    float
 *        Q : nrj2   energy qdc2 in keV    float
 *        p : pileup pileup                bool
 * 
 * You can access the read branches via the Event::read member (see the ReadIO struct definition)
 * 
 * 
 * You can use this class to write in another root tree. To do so, use the Event::writing method : 
 * 
 *        event.writing(outTree, "mlstEQ"); Will write the multiplicity, timestamp, relative time in ps, calibrated energy and calibrated QDC2
 * 
 * You can as well access the written branches via the Event::write member (see the WriteIO struct definition)
 * 
 * 
 * Interface with Hit class : 
 * 
 * You can add Hits to an event using the Event::push_back or Event::push_front methods : 
 * 
 *        event.push_back(hit);
 * 
 *        event.push_front(hit);
 * 
 * When the event is complete (e.g. in an event builder), you can for instance write it down or analyse it.
 * 
 * You can set the reference timestamp of the Event using the Event::setT0 method.
 * It can be an external reference timestamp like the RF or a particle timestamp for instance.
 *  
 * After using it, call Event::clear() to empty the event 
 * (not necessary if read from a TTree because TTree::GetEntry overwrites all the fields).
 */

class Event
{
public:

  Event() noexcept = default;

  Event (Hit const & hit) noexcept
  {
    *this = hit;
  }

  Event(Event const & event) noexcept = default;
  Event(Event && event) noexcept = default;

  // Event(Event const & event) noexcept :
  //   mult (event.mult ),
  //   stamp(event.stamp),
  //   readOpt (event.readOpt ),
  //   writeOpt(event.writeOpt)
  // {
  //   std::copy_n(event.labels   , mult ,  labels );
  //   std::copy_n(event.times    , mult ,  times  );
  //   std::copy_n(event.adcs     , mult ,  adcs   );
  //   std::copy_n(event.qdc2s    , mult ,  qdc2s  );
  //   std::copy_n(event.nrjs     , mult ,  nrjs   );
  //   std::copy_n(event.nrj2s    , mult ,  nrj2s  );
  //   std::copy_n(event.pileups  , mult ,  pileups);
  // }

  // Interface with Hit class
  void push_back (Hit const & hit) noexcept;
  void push_front(Hit const & hit) noexcept;

  inline bool operator==(Event const & other) const noexcept;
  inline Event& operator=(Hit const & hit) noexcept ;
  inline Event& operator=(Event const & evt) noexcept = default ;  
  inline Event& operator=(Event && event) noexcept = default;

  // Timing management :

  void setT0(Timestamp const & newTimestamp) noexcept
  {
    auto const shift = Time_cast(this->stamp - newTimestamp);
    for (int hit_i = 0; hit_i < mult; hit_i++) times[hit_i] += shift;
    this->stamp = newTimestamp;
  }

  void setT0(Hit const & hit) noexcept
  {
    setT0(hit.stamp);
  }

  // Usual methods :
  void Print() const noexcept;
  void clear() noexcept { mult = 0; }

  // State accessors : 
  bool isSingle() const noexcept {return (mult == 1);}
  bool isEmpty()  const noexcept {return (mult == 0);}

  // Public members :
  inline static constexpr size_t maxSize = 255u;

  int mult = 0;
  Timestamp stamp = 0ull;
  Label  labels  [maxSize] = {0};
  Time   times   [maxSize] = {0};
  ADC    adcs    [maxSize] = {0};
  NRJ    nrjs    [maxSize] = {0};
  ADC    qdc2s   [maxSize] = {0};
  NRJ    nrj2s   [maxSize] = {0};
  Pileup pileups [maxSize] = {0};

  void fullClear() noexcept 
  { // Set all the fields to 0
    std::fill(labels , labels  + maxSize, 0);
    std::fill(times  , times   + maxSize, 0);
    std::fill(adcs   , adcs    + maxSize, 0);
    std::fill(nrjs   , nrjs    + maxSize, 0);
    std::fill(qdc2s  , qdc2s   + maxSize, 0);
    std::fill(nrj2s  , nrj2s   + maxSize, 0);
    std::fill(pileups, pileups + maxSize, 0);
    mult = 0;
  }

  void check_safe()
  {
#ifndef UNSAFE
       if (mult > int(maxSize-2)) {print("Event mult > 255 !!"); return;}
#endif //UNSAFE
  }
  
  auto const & label  (int const & hit_i) const {return labels [hit_i];}
  auto       & label  (int const & hit_i)       {return labels [hit_i];}
  auto const & time   (int const & hit_i) const {return times  [hit_i];}
  auto       & time   (int const & hit_i)       {return times  [hit_i];}
  auto const & adc    (int const & hit_i) const {return adcs   [hit_i];}
  auto       & adc    (int const & hit_i)       {return adcs   [hit_i];}
  auto const & nrj    (int const & hit_i) const {return nrjs   [hit_i];}
  auto       & nrj    (int const & hit_i)       {return nrjs   [hit_i];}
  auto const & qdc2   (int const & hit_i) const {return qdc2s  [hit_i];}
  auto       & qdc2   (int const & hit_i)       {return qdc2s  [hit_i];}
  auto const & nrj2   (int const & hit_i) const {return nrj2s  [hit_i];}
  auto       & nrj2   (int const & hit_i)       {return nrj2s  [hit_i];}
  auto const & pileup (int const & hit_i) const {return pileups[hit_i];}
  auto       & pileup (int const & hit_i)       {return pileups[hit_i];}

  // std::vector-like interface
  inline Hit operator[](int const & hit_i) const noexcept;
  size_t size() const noexcept {return size_cast(mult);}

private:
};

inline Hit Event::operator[](int const & hit_i) const noexcept
{// Problem about the time here ... // ENCORE LE CAS ??
  return Hit(
    labels [hit_i], 
    stamp + times[hit_i], 
    times  [hit_i], 
    adcs   [hit_i], 
    nrjs   [hit_i], 
    qdc2s  [hit_i], 
    nrj2s  [hit_i], 
    pileups[hit_i]
  );
}

inline bool Event::operator==(Event const & other) const noexcept
{
  if (mult != other.mult) return false;
  if (stamp != other.stamp) return false;
  for (int i = 0; i<mult; ++i)
  {
    if (labels  [i] != other.labels  [i]) return false;
    if (times   [i] != other.times   [i]) return false;
    if (adcs    [i] != other.adcs    [i]) return false;
    if (nrjs    [i] != other.nrjs    [i]) return false;
    if (qdc2s   [i] != other.qdc2s   [i]) return false;
    if (nrj2s   [i] != other.nrj2s   [i]) return false;
    if (pileups [i] != other.pileups [i]) return false;
  }
  return true;
}

inline Event& Event::operator=(Hit const & hit) noexcept
{
  mult = 1;
  stamp = hit.stamp;
  times   [0] = 0;
  labels  [0] = hit.label;
  adcs    [0] = hit.adc;
  qdc2s   [0] = hit.qdc2;
  nrjs    [0] = hit.nrj;
  nrj2s   [0] = hit.nrj2;
  pileups [0] = hit.pileup;
  return *this;
}

inline void Event::push_back(Hit const & hit) noexcept
{
  check_safe();
  if (mult == 0) *this = hit; // Use of Event::operator=(Hit)
  else
  {
    labels  [mult] = hit.label;
    times   [mult] = Time_cast(hit.stamp-stamp);
    adcs    [mult] = hit.adc   ;
    nrjs    [mult] = hit.nrj   ;
    qdc2s   [mult] = hit.qdc2  ;
    nrj2s   [mult] = hit.nrj2  ;
    pileups [mult] = hit.pileup;
    ++mult;
  }
}

/**
 * @brief Sometimes, we want add events before the first hit. 
 * In such case, we have to push in in front of the others.
 * 
 * @details
 * About the timestamp of the event, we keep the same as this additional event is located
 * before the first hit that really represents the "0" of the event. Use Event::setT0 if
 * you want to modify the event's timestamp
 * 
 * @attention Not efficient !! Use if no other choice or not called often. 
 * 
 */
inline void Event::push_front(Hit const & hit) noexcept
{
  check_safe();
  for (auto i = mult; i>0; i--)
  {
    labels  [i] = labels  [i-1];
    times   [i] = times   [i-1];
    adcs    [i] = adcs    [i-1];
    nrjs    [i] = nrjs    [i-1];
    qdc2s   [i] = qdc2s   [i-1];
    nrj2s   [i] = nrj2s   [i-1];
    pileups [i] = pileups [i-1];
  }
  labels  [0] = hit.label;
  times   [0] = Time_cast(hit.stamp-stamp); // Here, times[0]<0 because the stamp corresponds to the 0 of the first hit
  adcs    [0] = hit.adc;
  nrjs    [0] = hit.nrj;
  qdc2s   [0] = hit.qdc2;
  nrj2s   [0] = hit.nrj2;
  pileups [0] = hit.pileup;
  ++mult;
}

inline std::ostream& operator<<(std::ostream& cout, Event const & event)
{
#ifdef COMULTITHREADING
  lock_mutex lock(mutex_events);
#endif //COMULTITHREADING

  cout << std::endl << "---" << std::endl;
  cout << event.mult << " hits. ";
  if (event.stamp != 0) cout << "Timestamp : " << event.stamp << " ps(" << std::fixed << std::setw(10) << Colib::nicer_seconds(event.stamp*1e-12) << ")";
  std::cout << std::endl;
  for (int i = 0; i<event.mult;i++)
  {
    cout << "label : " << std::fixed << std::setw(3)  << event.labels[i] << " ";
    cout << "time : "     << std::fixed << std::setw(10) << std::setprecision(10) << std::right << event.times [i] << +" ps  " ;
    if(event.adcs   [i] != 0) cout << "adc : "      << std::fixed << std::setw( 6) << std::setprecision( 6) << std::right << event.adcs  [i] << +" "     ;
    if(event.qdc2s  [i] != 0) cout << "qdc2 : "     << std::fixed << std::setw( 6) << std::setprecision( 6) << std::right << event.qdc2s [i] << +" "     ;
    if(event.nrjs   [i] != 0) cout << "energy : "   << std::fixed << std::setw( 5) << std::setprecision( 4) << std::right << event.nrjs  [i] << +" keV  ";
    if(event.nrj2s  [i] != 0) cout << "energy 2 : " << std::fixed << std::setw( 5) << std::setprecision( 4) << std::right << event.nrj2s [i] << +" keV  ";
    if(event.pileups[i] != 0) cout << "\u001b[31m pileup \u001b[0m";
    cout << std::endl;
  }
  cout << "--- " << std::endl;
  return cout;
}

inline void Event::Print() const noexcept
{
  print(*this);
}

//////////////////////////
/// Trigger definition ///
//////////////////////////

using EventTrigger = std::function<bool(const Event&)>;

// inline Event& Event::operator=(Event const & event) noexcept 
// {
//   read  = event.read;
//   write = event.write;
//   mult  = event.mult;
//   if (write.t || read.t) stamp = event.stamp;
//   if (write.l || read.l) std::copy_n(event.labels   , mult ,  labels );
//   if (write.T || read.T) std::copy_n(event.times    , mult ,  times  );
//   if (write.e || read.e) std::copy_n(event.adcs     , mult ,  adcs   );
//   if (write.q || read.q) std::copy_n(event.qdc2s    , mult ,  qdc2s  );
//   if (write.E || read.E) std::copy_n(event.nrjs     , mult ,  nrjs   );
//   if (write.Q || read.Q) std::copy_n(event.nrj2s    , mult ,  nrj2s  );
//   if (write.p || read.p) std::copy_n(event.pileups  , mult ,  pileups);
//   return *this;
// }