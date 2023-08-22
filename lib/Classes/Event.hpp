#ifndef EVENT_HPP
#define EVENT_HPP

#include "Hit.hpp"
#include "../libCo.hpp"
#include "../libRoot.hpp"

/**
 * @brief Event used for reading and writting event, event building and trigger
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
    std::copy_n(event.time2s   , mult ,  time2s );
    std::copy_n(event.adcs     , mult ,  adcs   );
    std::copy_n(event.qdc2s    , mult ,  qdc2s  );
    std::copy_n(event.nrjs     , mult ,  nrjs   );
    std::copy_n(event.nrj2s    , mult ,  nrj2s  );
    std::copy_n(event.pileups  , mult ,  pileups);
  }

  Event(TTree * tree, std::string const & options = "lstEQ", std::string const & io = "r")
  {
    this -> connect(tree, options, io);
  }

  void connect(TTree * tree, std::string const & options = "lstEQ", std::string const & io = "r")
  {
    if (io == "r" || io == "read" || io == "R" || io == "Read")  reading(tree, options);
    else if (io == "w" || io == "write" || io == "W" || io == "Write")  writting(tree, options);
    else { throw std::runtime_error("EVENT : NO KNOWN I/O PARAMETER");}
  }
  void reading(TTree * tree, std::string const & options = "lstEQ");
  void writting(TTree * tree, std::string const & options = "lstEQ");

  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  Event& operator=(Hit const & hit);
  Event& operator=(Event const & evt);

  void Print() const;

  void clear() { mult = 0; }

  size_t size() const { return size_cast(mult); }

  size_t const & maxSize() const { return m_maxSize; }

  bool isSingle() const {return (mult == 1);}
  bool isEmpty()  const {return (mult == 0);}

  // Hit getHit(uchar const & i) const {return Hit(labels[i], 0, times[i], 0., adcs[i], qdc2s[i], pileups[i]);}
  // Hit operator[](uchar const & i) const {return getHit(i);}

  // Public members :
  Mult mult = 0;
  Timestamp stamp = 0ull;
  Label   labels  [255] = {0};
  Time    times   [255] = {0};
  Time_ns time2s  [255] = {0};
  ADC     adcs    [255] = {0};
  NRJ     nrjs    [255] = {0};
  ADC     qdc2s   [255] = {0};
  NRJ     nrj2s   [255] = {0};
  Pileup  pileups [255] = {0};

  Read read;
  Write write;

private:
  size_t m_maxSize = 255;
  void* hits[8] = {labels, times, time2s, adcs, nrjs, qdc2s, nrj2s, pileups};
};

inline Event& Event::operator=(Hit const & hit)
{
  mult = 1;
  stamp = hit.stamp;
  times   [0] = 0;
  time2s  [0] = hit.time2;
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

void Event::reading(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

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

void Event::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}

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

              //  tree -> Branch("mult"   , &mult    , "mult/I"        );
  // if (write.s) tree -> Branch("stamp"  , &stamp   , "stamp/l"       );
  // if (write.T) tree -> Branch("time2"  , &time2s  , "time2[mult]/D" );
  // if (write.e) tree -> Branch("adc"    , &adcs    , "adc[mult]/D"   );
  // if (write.E) tree -> Branch("nrj"    , &nrjs    , "nrj[mult]/D"   );
  // if (write.Q) tree -> Branch("nrj2"   , &nrj2s   , "nrj2[mult]/D"  );
  // if (write.q) tree -> Branch("qdc2"   , &qdc2s   , "qdc2[mult]/D"  );
  // if (write.t) tree -> Branch("time"   , &times   , "time[mult]/I"  );
  // if (write.l) tree -> Branch("label"  , &labels  , "label[mult]/s" );
  // if (write.p) tree -> Branch("pileup" , &pileups , "pileup[mult]/O");

  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
#ifdef SAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif

               labels  [mult] = hit.label;
  if (write.t) times   [mult] = Time_cast(hit.stamp-stamp);
  if (write.e) adcs    [mult] = hit.adc;
  if (write.E) nrjs    [mult] = hit.nrj;
  if (write.q) qdc2s   [mult] = hit.qdc2;
  if (write.Q) nrj2s   [mult] = hit.nrj2;
  if (write.p) pileups [mult] = hit.pileup;
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
#ifdef SAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif
  for (Mult i = 0; i<mult; i++)
  {
                 labels  [i+1] = labels  [i];
    if (write.t) times   [i+1] = times   [i];
    if (write.e) adcs    [i+1] = adcs    [i];
    if (write.E) nrjs    [i+1] = nrjs    [i];
    if (write.q) qdc2s   [i+1] = qdc2s   [i];
    if (write.Q) nrj2s   [i+1] = nrj2s   [i];
    if (write.p) pileups [i+1] = pileups [i];
  }
               labels  [0] = hit.label;
  if (write.t) times   [0] = Time_cast(hit.time-stamp); // Here, times[0]<0 because the stamp corresponds to the 0 of the first hit that is logically located after
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

std::ostream& operator<<(std::ostream& cout, Event const & event)
{
  cout << std::endl << "---" << std::endl;
  cout << event.mult << " hits : ";
  if (event.stamp != 0) cout << "Timestamp : " << event.stamp << " ps";
  std::cout << std::endl;
  for (Mult i = 0; i<event.mult;i++)
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
