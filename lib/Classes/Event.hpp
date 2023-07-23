#ifndef EVENT_H
#define EVENT_H

#include "Hit.hpp"
#include "../libCo.hpp"
#include "../libRoot.hpp"

/*

READ/WRITE OPTIONS :

l : label   label               ushort
t : time    absolute timestamp  ULong64_t
T : time2   relative timestamp  float
n : nrj     energy              float
N : nrj2    energy QDC2         float
p : pileup  pilepup             bool

R :         RFtime              ULong64_t
P :         RFperiod            float

*/

class Event
{
  
public:

  Event() {}

  Event (Hit const & hit)
  {
    *this = hit;
  }

  Event(Event const & event)
  {
    *this = event;
  }

  Event(TTree * tree, std::string const & options = "ltnN", std::string const & io = "r")
  {
    if (io == "r" || io == "read" || io == "R" || io == "Read")  connect(tree, options);
    else if (io == "w" || io == "write" || io == "W" || io == "Write")  writeTo(tree, options);
    else { print("EVENT : NO KNOWN I/O PARAMETER"); exit(0);}
  }

  void fillHit_ptr(Hit_ptr & hit, int const & i);

  Hit_ptr const & operator[] (Int_t const & i);
  void operator= (Hit const & hit);
  void operator= (Event const & evt);

  void connect(TTree * tree, std::string const & options = "ltnN");
  void writeTo(TTree * tree, std::string const & options = "ltnN");

  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  void Print();
  void clear() { mult = 0; }
  std::size_t size() const { return static_cast<size_t>(mult); }
  std::size_t const & maxSize() const { return m_maxSize; }
  bool isSingle() const {return (mult == 1);}

  bool const & readTime() const {return read.T;}
  bool const & readtime() const {return read.t;}

  Hit getHit(uchar const & i) const {return Hit(labels[i], 0, times[i], 0., nrjs[i], nrj2s[i], pileups[i]);}

  // Public members :

  int      mult     = 0;

#ifdef USE_RF
  Time   RFtime   = 0ull;
  float  RFperiod = static_cast<float> (USE_RF);
#endif //USE_RF

  ushort labels [255] = {0};
  float  nrjs   [255] = {0};
  Time   times  [255] = {0};
  float  nrj2s  [255] = {0};
  float  time2s [255] = {0};
  bool   pileups[255] = {0};

  struct read
  {
    bool l = false;
    bool t = false;
    bool T = false;
    bool E = false;
    bool E2 = false;
    bool p = false;
    bool RFt = false;
    bool RFp = false;
  } read;

  struct write
  {
    bool l = false;
    bool t = false;
    bool T = false;
    bool E = false;
    bool E2 = false;
    bool p = false;
    bool RFt = false;
    bool RFp = false;
  } write;

private:

  std::size_t m_maxSize = 255;
  Hit_ptr m_hit;
};

inline void Event::fillHit_ptr(Hit_ptr & hit, int const & i)
{
  hit.nrjcal = &nrjs  [i];
  hit.nrj2   = &nrj2s [i];
  hit.label  = &labels[i];
  hit.time   = &times [i];
  if (read.T || write.T) hit.time2 = &time2s[i];
  hit.pileup = &pileups[i];
}

inline Hit_ptr const & Event::operator[] (int const & i)
{
  fillHit_ptr(m_hit, i);
  return m_hit;
}

inline void Event::operator= (Hit const & hit)
{
  labels  [0] = hit.label;
  nrjs    [0] = hit.nrjcal;
  nrj2s   [0] = hit.nrj2;
  times   [0] = hit.time;
  pileups [0] = hit.pileup;
  mult = 1;
}

inline void Event::operator= (Event const & evt)
{
  mult = evt.mult;
  for (int i = 0; i<mult; i++)
  {
    labels [i] = evt.labels[i];
    nrjs   [i] = evt.nrjs[i];
    nrj2s  [i] = evt.nrj2s[i];
    times  [i] = evt.times[i];
    pileups[i] = evt.pileups[i];
  }
#ifdef USE_RF
  RFtime   = evt.RFtime;
  RFperiod = evt.RFperiod;
#endif //USE_RF
  read = evt.read;
  write = evt.write;
}

void Event::writeTo(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}
  for (auto const & e : options)
  {
    switch (e)
    {
      case ('l') : write.l    =  true;  break;
      case ('t') : write.t    =  true;  break;
      case ('T') : write.T    =  true;  break;
      case ('n') : write.E    =  true;  break;
      case ('N') : write.E2   =  true;  break;
      case ('p') : write.p    =  true;  break;
      case ('R') : write.RFt  =  true;  break;
      case ('P') : write.RFp  =  true;  break;
    }
  }

  tree -> ResetBranchAddresses();

                    tree -> Branch("mult"     , &mult);
  if ( write.l   )  tree -> Branch("label"    , &labels , "label[mult]/s" );
  if ( write.t   )  tree -> Branch("time"     , &times  , "time[mult]/l"  );
  if ( write.T   )  tree -> Branch("Time"     , &time2s , "Time[mult]/F");
  if ( write.E   )  tree -> Branch("nrj"      , &nrjs   , "nrj[mult]/F"   );
  if ( write.E2  )  tree -> Branch("nrj2"     , &nrj2s  , "nrj2[mult]/F"  );
  if ( write.p   )  tree -> Branch("pileup"   , &pileups, "pileup[mult]/O");
  if ( write.RFt )  tree -> Branch("RFtime"   , &RFtime  );
  if ( write.RFp )  tree -> Branch("RFperiod" , &RFperiod);

  tree -> SetBranchStatus("*",true);
}

void Event::connect(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}
  for (auto const & e : options)
  {
    switch (e)
    {
      case ('l') : read.l    =  true;  break;
      case ('t') : read.t    =  true;  break;
      case ('T') : read.T    =  true;  break;
      case ('n') : read.E    =  true;  break;
      case ('N') : read.E2   =  true;  break;
      case ('p') : read.p    =  true;  break;
      case ('R') : read.RFt  =  true;  break;
      case ('P') : read.RFp  =  true;  break;
    }
  }
  tree -> ResetBranchAddresses();
                tree -> SetBranchAddress("mult"    , &mult    );
  if ( read.l ) tree -> SetBranchAddress("label"   , &labels  );
  if ( read.t ) tree -> SetBranchAddress("time"    , &times   );
  if ( read.T ) tree -> SetBranchAddress("Time"    , &time2s  );
  if ( read.E ) tree -> SetBranchAddress("nrj"     , &nrjs    );
  if ( read.E2) tree -> SetBranchAddress("nrj2"    , &nrj2s   );
  if ( read.p ) tree -> SetBranchAddress("pileup"  , &pileups );
  if ( read.p ) tree -> SetBranchAddress("RFtime"  , &RFtime  );
  if ( read.p ) tree -> SetBranchAddress("RFperiod", &RFperiod);

  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
  labels[mult]  = hit.label;
  nrjs[mult]    = hit.nrjcal;
  nrj2s[mult]   = hit.nrj2;
  times[mult]   = hit.time;
  pileups[mult] = hit.pileup;
  mult++;
}

inline void Event::push_front(Hit const & hit)
{
  for (uchar i = 0; i<mult; i++)
  {
    labels [mult+1] = labels [mult];
    nrjs   [mult+1] = nrjs   [mult];
    nrj2s  [mult+1] = nrj2s  [mult];
    times  [mult+1] = times  [mult];
    pileups[mult+1] = pileups[mult];
  }
  labels[0]  = hit.label;
  nrjs[0]    = hit.nrjcal;
  nrj2s[0]   = hit.nrjcal;
  times[0]   = hit.time;
  pileups[0] = hit.pileup;
  mult++;
}

// template <class... T> void print(Event const & evt, T const & ... t2)
// {
//   evt.Print();
//   print(t2...);
// }

inline void Event::Print()
{
  print("---");
  print(mult, "hits :");
  for (uchar i = 0; i<mult;i++)
  {
    print(
      "label :",labels[i],
      "time :" ,times[i],
      (nrjs[i]) ? "energy :"+std::to_string(nrjs[i]) : "",
      (nrj2s[i]) ? "energy2 :"+std::to_string(nrj2s[i]) : "",
      (pileups[i]) ? "pileup" : ""
    );
  }
  print("---");
}

#endif //EVENT_H
