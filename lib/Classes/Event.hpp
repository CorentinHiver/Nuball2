#ifndef EVENT_HPP
#define EVENT_HPP

#include "Hit.hpp"

#ifdef MULTITHREADING
  std::mutex mutex_events;
#endif //MULTITHREADING

/**
 * @brief Event object used for reading and writing event from/to root files in Nuball2-like TTree format, 
 * perform event building and trigger for faster to root conversion and data analysis.
 * 
 * @version 2.0 Removed the time2s handling
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
 *        event.reading(tree, "lst"); // Reads only the multiplicity, label, timestamp and relative time.
 * 
 * Note the multiplicity will always be activated, because it is mandatory to read the root file.
 * 
 * A list of all the options available so far :
 * 
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
 *        event.writing(outTree, "lstEQ"); Will write the multiplicity, timestamp, relative time in ps, calibrated energy and calibrated QDC2
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
 * Especially useful, you can set the reference timestamp of the Event using the Event::setT0 method.
 * It can be an external reference timestamp like the RF or a particle timestamp for instance.
 *  
 * After using it, call Event::clear() to empty the event 
 * (not necessary if readd from a TTree because TTree::GetEntry overwrites all the fields).
 */
class Event
{
  
public:

  Event() 
  {

  }

  Event (Hit const & hit)
  {
    *this = hit;
  }

  Event(Event const & event) :
    mult (event.mult ),
    stamp(event.stamp),
    read (event.read ),
    write(event.write)
  {
    std::copy_n(event.labels   , mult ,  labels );
    std::copy_n(event.times    , mult ,  times  );
    std::copy_n(event.adcs     , mult ,  adcs   );
    std::copy_n(event.qdc2s    , mult ,  qdc2s  );
    std::copy_n(event.nrjs     , mult ,  nrjs   );
    std::copy_n(event.nrj2s    , mult ,  nrj2s  );
    std::copy_n(event.pileups  , mult ,  pileups);
  }

  Event (TTree * tree) {this -> reading(tree);}
  Event (TTree * tree, std::string const & options) {this -> reading(tree, options);}

  // Interface with TTree class
  void reading(TTree * tree);
  void reading(TTree * tree, std::string const & options);
  void writing(TTree * tree, std::string const & options = "ltTeEqQ");

  // Interface with Hit class
  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  Event& operator=(Hit const & hit);
  Event& operator=(Event const & evt);

  // Timing management :
  void setT0(Timestamp const & timestamp) noexcept
  {
    auto const & shift = Time_cast(this->stamp - timestamp);
    this -> timeShift(shift);
  }

  void timeShift(Time const & shift) noexcept
  {
    for (int hit_i = 0; hit_i<mult; hit_i++) {timeShift(shift, hit_i);}
  }

  void timeShift(Time const & shift, int const & hit_i) noexcept {times[hit_i]+=shift;}

  // Usual methods :
  void Print() const noexcept;
  void clear() noexcept { mult = 0; }
  size_t size() const noexcept { return size_cast(mult); }

  // State accessors : 
  bool isSingle() const noexcept {return (mult == 1);}
  bool isEmpty()  const noexcept {return (mult == 0);}

  // Public members :
  int mult = 0;
  Timestamp stamp = 0ull;
  static constexpr size_t maxSize = 255;
  Label     labels  [maxSize] = {0};
  Time      times   [maxSize] = {0};
  ADC       adcs    [maxSize] = {0};
  NRJ       nrjs    [maxSize] = {0};
  ADC       qdc2s   [maxSize] = {0};
  NRJ       nrj2s   [maxSize] = {0};
  Pileup    pileups [maxSize] = {0};

  void check_safe()
  {
#ifndef UNSAFE
       if (mult>254) {print("Event mult > 254 !!"); return;}
  else if (mult<0  ) throw_error("Event mult < 0 !!");
#endif //UNSAFE
  }
      
  // auto const & label  const (int const & hit_i) {return labels [hit_i];}
  auto       & label        (int const & hit_i) {return labels [hit_i];}
  // auto const & time   const (int const & hit_i) {return times  [hit_i];}
  auto       & time         (int const & hit_i) {return times  [hit_i];}
  // auto const & adc    const (int const & hit_i) {return adcs   [hit_i];}
  auto       & adc          (int const & hit_i) {return adcs   [hit_i];}
  // auto const & nrj    const (int const & hit_i) {return nrjs   [hit_i];}
  auto       & nrj          (int const & hit_i) {return nrjs   [hit_i];}
  // auto const & qdc2   const (int const & hit_i) {return qdc2s  [hit_i];}
  auto       & qdc2         (int const & hit_i) {return qdc2s  [hit_i];}
  // auto const & nrj2   const (int const & hit_i) {return nrj2s  [hit_i];}
  auto       & nrj2         (int const & hit_i) {return nrj2s  [hit_i];}
  // auto const & pileup const (int const & hit_i) {return pileups[hit_i];}
  auto       & pileup       (int const & hit_i) {return pileups[hit_i];}

  Hit operator[](int const & hit_i);

  // I/O status :
  IOptions read;
  IOptions write;

private:
};

Hit Event::operator[](int const & hit_i) 
{// Problem about the time here ...
  Hit hit;
  hit.stamp = stamp + times[hit_i];
  hit.label = labels[hit_i];
  hit.adc   = adcs  [hit_i];
  hit.qdc2  = qdc2s [hit_i];
  hit.nrj   = nrjs  [hit_i];
  hit.nrj2  = nrj2s [hit_i];
  return hit;
}


inline Event& Event::operator=(Hit const & hit)
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

inline Event& Event::operator=(Event const & event)
{
  read  = event.read;
  write = event.write;
  mult  = event.mult;
  if (write.t || read.t) stamp = event.stamp;
  if (write.l || read.l) std::copy_n(event.labels   , mult ,  labels );
  if (write.T || read.T) std::copy_n(event.times    , mult ,  times  );
  if (write.e || read.e) std::copy_n(event.adcs     , mult ,  adcs   );
  if (write.q || read.q) std::copy_n(event.qdc2s    , mult ,  qdc2s  );
  if (write.E || read.E) std::copy_n(event.nrjs     , mult ,  nrjs   );
  if (write.Q || read.Q) std::copy_n(event.nrj2s    , mult ,  nrj2s  );
  if (write.p || read.p) std::copy_n(event.pileups  , mult ,  pileups);
  return *this;
}

/**
 * @brief Automatically set branches based on the presence or not of branches in the root tree.
 * Reserved for "Nuball2" type of trees
 */
void inline Event::reading(TTree * tree)
{
#ifdef MULTITHREADING
  lock_mutex lock(mutex_events);
#endif //MULTITHREADING

  if (!tree) {print("Input tree at address 0x00 !"); return;}

  tree -> ResetBranchAddresses();

  auto branches = tree->GetListOfBranches();
  for (int i = 0; i < branches->GetEntries(); ++i) 
  {
    auto branch = dynamic_cast<TBranch*>(branches->At(i));
    std::string const branchName = branch->GetName();
    
         if (branchName == "mult"  ) {                tree -> SetBranchAddress("mult"   , &mult);}
    else if (branchName == "label" ) {read.l = true;  tree -> SetBranchAddress("label"  , &labels );}
    else if (branchName == "stamp" ) {read.t = true;  tree -> SetBranchAddress("stamp"  , &stamp  );}
    else if (branchName == "time"  ) {read.T = true;  tree -> SetBranchAddress("time"   , &times  );}
    else if (branchName == "adc"   ) {read.e = true;  tree -> SetBranchAddress("adc"    , &adcs   );}
    else if (branchName == "nrj"   ) {read.E = true;  tree -> SetBranchAddress("nrj"    , &nrjs   );}
    else if (branchName == "qdc2"  ) {read.q = true;  tree -> SetBranchAddress("qdc2"   , &qdc2s  );}
    else if (branchName == "nrj2"  ) {read.Q = true;  tree -> SetBranchAddress("nrj2"   , &nrj2s  );}
    else if (branchName == "pileup") {read.p = true;  tree -> SetBranchAddress("pileup" , &pileups);}
    else
    {
      warning("branch", branchName, "unkown ... Event class can't read it !");
    }
  }
  tree -> SetBranchStatus("*",true);
}

void inline Event::reading(TTree * tree, std::string const & options)
{
#ifdef MULTITHREADING
  lock_mutex lock(mutex_events);
#endif //MULTITHREADING

  if (!tree) {print("Input tree at address 0x00 !"); return;}

  read.setOptions(options);

  tree -> ResetBranchAddresses();

              tree -> SetBranchAddress("mult"   , &mult   );
  if (read.l) tree -> SetBranchAddress("label"  , &labels );
  if (read.t) tree -> SetBranchAddress("stamp"  , &stamp  );
  if (read.T) tree -> SetBranchAddress("time"   , &times  );
  if (read.e) tree -> SetBranchAddress("adc"    , &adcs   );
  if (read.E) tree -> SetBranchAddress("nrj"    , &nrjs   );
  if (read.q) tree -> SetBranchAddress("qdc2"   , &qdc2s  );
  if (read.Q) tree -> SetBranchAddress("nrj2"   , &nrj2s  );
  if (read.p) tree -> SetBranchAddress("pileup" , &pileups);

  tree -> SetBranchStatus("*",true);
}

void inline Event::writing(TTree * tree, std::string const & options)
{
#ifdef MULTITHREADING
  lock_mutex lock(mutex_events);
#endif //MULTITHREADING

  if (!tree) {print("Output tree at address 0x00 !"); return;}

  write.setOptions(options);  

  tree -> ResetBranchAddresses();

               createBranch     (tree, &mult    , "mult"  );
  if (write.t) createBranch     (tree, &stamp   , "stamp" );
  if (write.T) createBranchArray(tree, &times   , "time"  , "mult");
  if (write.E) createBranchArray(tree, &nrjs    , "nrj"   , "mult");
  if (write.Q) createBranchArray(tree, &nrj2s   , "nrj2"  , "mult");
  if (write.e) createBranchArray(tree, &adcs    , "adc"   , "mult");
  if (write.q) createBranchArray(tree, &qdc2s   , "qdc2"  , "mult");
  if (write.l) createBranchArray(tree, &labels  , "label" , "mult");
  if (write.p) createBranchArray(tree, &pileups , "pileup", "mult");
  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
  check_safe();
  labels  [mult] = hit.label;
  times   [mult] = Time_cast(hit.stamp-stamp);
  adcs    [mult] = hit.adc;
  nrjs    [mult] = hit.nrj;
  qdc2s   [mult] = hit.qdc2;
  nrj2s   [mult] = hit.nrj2;
  pileups [mult] = hit.pileup;
  ++mult;
}

/**
 * @brief Sometimes, we want to select pre-prompt events. 
 * In such case, we have to put in in front of the others.
 * 
 * @details
 * About the timestamp of the event, we keep the same as this additional event is located
 * before the first hit that really represents the "0" of the event
 * 
 */
inline void Event::push_front(Hit const & hit)
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
#ifdef MULTITHREADING
  lock_mutex lock(mutex_events);
#endif //MULTITHREADING

  cout << std::endl << "---" << std::endl;
  cout << event.mult << " hits : ";
  if (event.stamp != 0) cout << "Timestamp : " << event.stamp << " ps";
  std::cout << std::endl;
  for (int i = 0; i<event.mult;i++)
  {
    cout << "label : " << event.labels[i] << " ";
    if(event.times  [i] != 0) cout << "time : "     + std::to_string(event.times [i])+" ps  " ;
    if(event.adcs   [i] != 0) cout << "adc : "      + std::to_string(event.adcs  [i])+" ";
    if(event.qdc2s  [i] != 0) cout << "qdc2 : "     + std::to_string(event.qdc2s [i])+" ";
    if(event.nrjs   [i] != 0) cout << "energy : "   + std::to_string(event.nrjs  [i])+" keV  ";
    if(event.nrj2s  [i] != 0) cout << "energy 2 : " + std::to_string(event.nrj2s [i])+" keV  ";
    if(event.pileups[i] != 0) cout << "pileup"                                                 ;
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

using TriggerEvent = std::function<bool(const Event&)>;

#endif //EVENT_HPP
