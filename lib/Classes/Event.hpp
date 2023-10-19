#ifndef EVENT_HPP
#define EVENT_HPP

#include "Hit.hpp"
#include "../libCo.hpp"
#include "../libRoot.hpp"

/**
 * @brief Event used for reading and writting event, event building and trigger
 * @details
 * 
 * An Event is in principle a collection of Hits. However, in order to be an efficient interface with 
 * ROOT TTree, in practice it consist of a collections of arrays that stores values of a specific branch.
 * 
 * The following are public members (i.e. you can call it directly with Event::[array_name])
 * 
 *        int mult = 0;                  Number of hits currently stored in the event.
 *        Timestamp stamp = 0ull;         Absolute timestamp of the whole event
 *        Label   labels  [255] = {0};    Labels of the hits
 *        Time    times   [255] = {0};    Time in ps (Long64_t) relative to the first hit
 *        Time_ns time2s  [255] = {0};    Time in ns (float) relative either to the first hit or to the pulse
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
 * Only the two first members are not arrays because they are true for the entire event.
 * 
 * To read any Nuball2-like TTree : 
 * 
 *        // Loads the TTree or TChain ...
 *        Event event;
 *        event.reading(ttree);
 * 
 * If you are only interested in a few branches, you can choose them by adding an option : 
 * 
 *        event.reading(ttree, "lst"); // Reads only the multiplicity, label, timestamp and relatative time.
 * 
 * Note the multiplicity will always be activated, because it is mandatory to read the root file.
 * 
 * A list of all the options available so far :
 * 
 *        l : label  label                 ushort
 *        s : stamp  absolute timestamp ps ULong64_t
 *        t : time   relative timestamp ps Long64_t
 *        T : time2  relative timestamp ns float
 *        e : adc    energy in ADC         int
 *        E : nrj    energy in keV         float
 *        q : qdc2   energy qdc2 in ADC    float
 *        Q : nrj2   energy qdc2 in keV    float
 *        p : pileup pilepup               bool
 * 
 * You can access the readed branches via the Event::read member (see the ReadIO struct definition)
 * 
 * 
 * You can use this class to write in another root tree. To do so, use the Event::writting method : 
 * 
 *        event.writting(outTree, "lstEQ"); Will write the multiplicity, timestamp, relative time and calibrated energy and QDC2
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
 * When the event is complete (e.g. in an event builder), you can for instance write it down or analyse it, then call Event::clear() to empty it.
 * 
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
    write(event.write),
    isReading  (event.isReading),
    isWritting (event.isWritting)
  {
    std::copy_n(event.labels   , mult ,  labels );
    std::copy_n(event.times    , mult ,  times  );
    std::copy_n(event.time2s   , mult ,  time2s );
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
  void writting(TTree * tree, std::string const & options = "lstTeEqQ");

  // Interface with Hit class
  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  Event& operator=(Hit const & hit);
  Event& operator=(Event const & evt);

  // Timing management :
  void setT0(Timestamp const & timestamp)
  {
    auto const & shift = Time_cast(this->stamp - timestamp);
    (read.T) ? this -> timeShift_ns(shift/1000.) : this -> timeShift(shift);
  }

  void timeShift(Time const & shift)
  {
    for (int hit_i = 0; hit_i<mult; hit_i++)
    {
      times[hit_i]+=shift;
    }
  }

  void timeShift_ns(double const & shift)
  {
    for (int hit_i = 0; hit_i<mult; hit_i++)
    {
      time2s[hit_i]+=shift;
    }
  }

  // Usual methods :
  void Print() const;
  void clear() { mult = 0; }
  size_t size() const { return size_cast(mult); }

  // Specific methods :
  size_t const & maxSize() const { return m_maxSize; }

  // Accessors :
  Time_ns time_ns(int const & i) const {return (read.T) ? time2s[i] : Time_ns_cast(times[i])/1000.f;}

  // State accessors : 
  bool isSingle() const {return (mult == 1);}
  bool isEmpty()  const {return (mult == 0);}
  bool isCalibrated() const 
  {
         if ( isReading && !isWritting) return read .e && !read .E;
    else if (!isReading &&  isWritting) return write.e && !write.E;
    else 
    {
      print("Event not connected to any tree yet !");
      return false;
    }
  }

  // Public members :
  int mult = 0;
  Timestamp stamp = 0ull;
  Label   labels  [255] = {0};
  Time    times   [255] = {0};
  Time_ns time2s  [255] = {0};
  ADC     adcs    [255] = {0};
  NRJ     nrjs    [255] = {0};
  ADC     qdc2s   [255] = {0};
  NRJ     nrj2s   [255] = {0};
  Pileup  pileups [255] = {0};

  // I/O status :
  IOptions read;
  IOptions write;

private:
  size_t m_maxSize = 255;
  bool isReading = false;
  bool isWritting = false;
};

inline Event& Event::operator=(Hit const & hit)
{
  mult = 1;
  stamp = hit.stamp;
  times   [0] = 0;
  time2s  [0] = 0;
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
  isReading  = event.isReading;
  isWritting = event.isWritting;
  if (write.s || read.s) stamp = event.stamp;
  if (write.l || read.l) std::copy_n(event.labels   , mult ,  labels );
  if (write.t || read.t) std::copy_n(event.times    , mult ,  times  );
  if (write.T || read.T) std::copy_n(event.time2s   , mult ,  time2s );
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
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  isReading  = true;
  isWritting = false;

  tree -> ResetBranchAddresses();

  tree -> SetBranchAddress("mult", &mult);

  auto branches = tree->GetListOfBranches();
  for (int i = 0; i < branches->GetEntries(); ++i) 
  {
    auto branch = dynamic_cast<TBranch*>(branches->At(i));
    std::string const branchName = branch->GetName();
    
         if (branchName == "label" ) {read.l = true;  tree -> SetBranchAddress("label"  , &labels );}
    else if (branchName == "stamp" ) {read.s = true;  tree -> SetBranchAddress("stamp"  , &stamp  );}
    else if (branchName == "time"  ) {read.t = true;  tree -> SetBranchAddress("time"   , &times  );}
    else if (branchName == "time2" ) {read.T = true;  tree -> SetBranchAddress("time2"  , &time2s );}
    else if (branchName == "adc"   ) {read.e = true;  tree -> SetBranchAddress("adc"    , &adcs   );}
    else if (branchName == "nrj"   ) {read.E = true;  tree -> SetBranchAddress("nrj"    , &nrjs   );}
    else if (branchName == "qdc2"  ) {read.q = true;  tree -> SetBranchAddress("qdc2"   , &qdc2s  );}
    else if (branchName == "nrj2"  ) {read.Q = true;  tree -> SetBranchAddress("nrj2"   , &nrj2s  );}
    else if (branchName == "pileup") {read.p = true;  tree -> SetBranchAddress("pileup" , &pileups);}
  }
  tree -> SetBranchStatus("*",true);
}

void inline Event::reading(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  isReading  = true;
  isWritting = false;

  read.setOptions(options);

  tree -> ResetBranchAddresses();

              tree -> SetBranchAddress("mult"   , &mult   );
  if (read.l) tree -> SetBranchAddress("label"  , &labels );
  if (read.s) tree -> SetBranchAddress("stamp"  , &stamp  );
  if (read.t) tree -> SetBranchAddress("time"   , &times  );
  if (read.T) tree -> SetBranchAddress("time2"  , &time2s );
  if (read.e) tree -> SetBranchAddress("adc"    , &adcs   );
  if (read.E) tree -> SetBranchAddress("nrj"    , &nrjs   );
  if (read.q) tree -> SetBranchAddress("qdc2"   , &qdc2s  );
  if (read.Q) tree -> SetBranchAddress("nrj2"   , &nrj2s  );
  if (read.p) tree -> SetBranchAddress("pileup" , &pileups);

  tree -> SetBranchStatus("*",true);
}

void inline Event::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}

  isReading  = false;
  isWritting = true;

  write.setOptions(options);  

  tree -> ResetBranchAddresses();

               createBranch     (tree, &mult    , "mult"  );
  if (write.s) createBranch     (tree, &stamp   , "stamp" );
  if (write.t) createBranchArray(tree, &times   , "time"  , "mult");
  if (write.T) createBranchArray(tree, &time2s  , "time2" , "mult");
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
#ifndef UNSAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif //UNSAFE

  labels  [mult] = hit.label;
  times   [mult] = Time_cast(hit.stamp-stamp);
  // times   [mult] = (hit.isExternalTime()) ? hit.time : Time_cast(hit.stamp-stamp);
  adcs    [mult] = hit.adc;
  nrjs    [mult] = hit.nrj;
  qdc2s   [mult] = hit.qdc2;
  nrj2s   [mult] = hit.nrj2;
  pileups [mult] = hit.pileup;
  mult++;
}

/**
 * @brief Sometimes, we want to select pre-prompt events. 
 * In such case, we have to put in in front of the others.
 * 
 * @details
 * About the timestamp of the event, we keep the same as this additionnal event is located
 * before the first hit that really represents the "0" of the event
 * 
 */
inline void Event::push_front(Hit const & hit)
{
#ifndef UNSAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif //UNSAFE
  for (auto i = mult; i>0; i--)
  {
                 labels  [i] = labels  [i-1];
    if (write.t) times   [i] = times   [i-1];
    if (write.e) adcs    [i] = adcs    [i-1];
    if (write.E) nrjs    [i] = nrjs    [i-1];
    if (write.q) qdc2s   [i] = qdc2s   [i-1];
    if (write.Q) nrj2s   [i] = nrj2s   [i-1];
    if (write.p) pileups [i] = pileups [i-1];
  }
               labels  [0] = hit.label;
  if (write.t) times   [0] = Time_cast(hit.stamp-stamp); // Here, times[0]<0 because the stamp corresponds to the 0 of the first hit
  if (write.e) adcs    [0] = hit.adc;
  if (write.E) nrjs    [0] = hit.nrj;
  if (write.q) qdc2s   [0] = hit.qdc2;
  if (write.Q) nrj2s   [0] = hit.nrj2;
  if (write.p) pileups [0] = hit.pileup;
  mult++;
}

inline void Event::Print() const
{
  print(*this);
}

inline std::ostream& operator<<(std::ostream& cout, Event const & event)
{
  cout << std::endl << "---" << std::endl;
  cout << event.mult << " hits : ";
  if (event.stamp != 0) cout << "Timestamp : " << event.stamp << " ps";
  std::cout << std::endl;
  for (int i = 0; i<event.mult;i++)
  {
    cout << "label : " << event.labels[i] << " ";
    if(event.times  [i] != 0) cout << "time : "     + std::to_string(event.times [i])+" ps  " ;
    if(event.time2s [i] != 0) cout << "rel time : " + std::to_string(event.time2s[i])+" ns  " ;
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

#endif //EVENT_HPP
