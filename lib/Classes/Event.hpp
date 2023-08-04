#ifndef EVENT_HPP
#define EVENT_HPP

#include "Hit.hpp"
#include "../libCo.hpp"
#include "../libRoot.hpp"

/**
 * @brief Event used for reading and writting event, event building and trigger
 * @attention When used with Hits, only reads nrjcal (and nrj2cal when QDC2 used)
 * 
 */
class Event
{
  
public:

  Event() {}

  Event (Hit const & hit)
  {
    *this = hit;
  }

  Event(Event const & event) :
    mult   (event.mult   ),
    read   (event.read   ),
    write  (event.write  ),
    m_maxSize(event.m_maxSize)
  {
    for (int i = 0; i<mult; i++)
    {
      labels [i] = event.labels [i];
      times  [i] = event.times  [i];
      nrjs   [i] = event.nrjs   [i];
      nrj2s  [i] = event.nrj2s  [i];
      time2s [i] = event.time2s [i];
      pileups[i] = event.pileups[i];
    }
    
  #ifdef USE_RF
    RFtime   = event.RFtime;
    RFperiod = event.RFperiod;
  #endif //USE_RF
  }

  Event(TTree * tree, std::string const & options = "ltEQ", std::string const & io = "r")
  {
    this -> connect(tree, options, io);
  }

  void operator=(Hit const & hit);
  void operator=(Event const & evt);

  void connect(TTree * tree, std::string const & options = "ltEQ", std::string const & io = "r")
  {
    if (io == "r" || io == "read" || io == "R" || io == "Read")  reading(tree, options);
    else if (io == "w" || io == "write" || io == "W" || io == "Write")  writting(tree, options);
    else { throw std::runtime_error("EVENT : NO KNOWN I/O PARAMETER");}
  }
  void reading(TTree * tree, std::string const & options = "ltEQ");
  void writting(TTree * tree, std::string const & options = "ltEQ");

  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  void Print();
  void clear() { mult = 0; }
  std::size_t size() const { return static_cast<std::size_t>(mult); }
  std::size_t const & maxSize() const { return m_maxSize; }
  bool isSingle() const {return (mult == 1);}

  bool const & readTime() const {return read.T;}
  bool const & readtime() const {return read.t;}

  Hit getHit(uchar const & i) const {return Hit(labels[i], 0, times[i], 0., nrjs[i], nrj2s[i], pileups[i]);}
  Hit operator[](uchar const & i) const {return getHit(i);}

  // Public members :
  int mult = 0;

#ifdef USE_RF
  Time   RFtime   = 0ull;
  float  RFperiod = static_cast<float> (USE_RF);
#endif //USE_RF

  ushort labels [255] = {0};
  float  nrjs   [255] = {0};
  Time   times  [255] = {0};
  float  nrj2s  [255] = {0};
  double time2s [255] = {0};
  bool   pileups[255] = {0};

  Read read;
  Write write;

private:

  std::size_t m_maxSize = 255;
};

inline void Event::operator=(Hit const & hit)
{
  labels  [0] = hit.label;
  nrjs    [0] = hit.nrjcal;
  nrj2s   [0] = hit.nrj2;
  times   [0] = hit.time;
  pileups [0] = hit.pileup;
  mult = 1;
}

inline void Event::operator=(Event const & evt)
{
  mult = evt.mult;
  for (int i = 0; i<mult; i++)
  {
    labels [i] = evt.labels [i];
    nrjs   [i] = evt.nrjs   [i];
    nrj2s  [i] = evt.nrj2s  [i];
    times  [i] = evt.times  [i];
    pileups[i] = evt.pileups[i];
  }
#ifdef USE_RF
  RFtime   = evt.RFtime;
  RFperiod = evt.RFperiod;
#endif //USE_RF
  read = evt.read;
  write = evt.write;
}

void Event::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}

  write.setOptions(options);  

  tree -> ResetBranchAddresses();

                  tree -> Branch("mult"  , &mult);
  if ( write.l )  tree -> Branch("label" , &labels , "label[mult]/s" );
  if ( write.t )  tree -> Branch("time"  , &times  , "time[mult]/l"  );
  if ( write.T )  tree -> Branch("Time"  , &time2s , "Time[mult]/F"  );
  if ( write.E )  tree -> Branch("nrj"   , &nrjs   , "nrj[mult]/F"   );
  if ( write.Q )  tree -> Branch("nrj2"  , &nrj2s  , "nrj2[mult]/F"  );
  if ( write.p )  tree -> Branch("pileup", &pileups, "pileup[mult]/O");
#ifdef USE_RF
  if ( write.RFt )  tree -> Branch("RFtime"   , &RFtime  );
  if ( write.RFp )  tree -> Branch("RFperiod" , &RFperiod);
#endif //USE_RF

  tree -> SetBranchStatus("*",true);
}

void Event::reading(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  read.setOptions(options);

  tree -> ResetBranchAddresses();
               tree -> SetBranchAddress("mult"  , &mult   );
  if ( read.l) tree -> SetBranchAddress("label" , &labels );
  if ( read.t) tree -> SetBranchAddress("time"  , &times  );
  if ( read.T) tree -> SetBranchAddress("Time"  , &time2s );
  if ( read.E) tree -> SetBranchAddress("nrj"   , &nrjs   );
  if ( read.Q) tree -> SetBranchAddress("nrj2"  , &nrj2s  );
  if ( read.p) tree -> SetBranchAddress("pileup", &pileups);
#ifdef USE_RF
  if ( read.p ) tree -> SetBranchAddress("RFtime"  , &RFtime  );
  if ( read.p ) tree -> SetBranchAddress("RFperiod", &RFperiod);
#endif //USE_RF
  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
  labels[mult]  = hit.label;
  nrjs[mult]    = hit.nrjcal;
  nrj2s[mult]   = hit.nrj2cal;
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
  nrj2s[0]   = hit.nrj2cal;
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

#endif //EVENT_HPP
