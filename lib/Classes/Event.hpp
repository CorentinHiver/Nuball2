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
      labels  [i] = event.labels  [i];
      times   [i] = event.times   [i];
      nrjs    [i] = event.nrjs    [i];
      nrj2s   [i] = event.nrj2s   [i];
      nrjcals [i] = event.nrjcals [i];
      nrj2cals[i] = event.nrj2cals[i];
      time2s  [i] = event.time2s  [i];
      pileups [i] = event.pileups [i];
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

  Event& operator=(Hit const & hit);
  Event& operator=(Event const & evt);

  void Print() const;

  void clear() { mult = 0; }

  std::size_t size() const { return static_cast<std::size_t>(mult); }

  std::size_t const & maxSize() const { return m_maxSize; }

  bool isSingle() const {return (mult == 1);}
  bool isEmpty()  const {return (mult == 0);}

  // Hit getHit(uchar const & i) const {return Hit(labels[i], 0, times[i], 0., nrjs[i], nrj2s[i], pileups[i]);}
  // Hit operator[](uchar const & i) const {return getHit(i);}

  // Public members :
  int mult = 0;

#ifdef USE_RF
  Time   RFtime   = 0ull;
  float  RFperiod = static_cast<float> (USE_RF);
#endif //USE_RF

  ushort labels  [255] = {0};
  Time   times   [255] = {0};
  double time2s  [255] = {0};
  float  nrjs    [255] = {0};
  float  nrjcals [255] = {0};
  float  nrj2s   [255] = {0};
  float  nrj2cals[255] = {0};
  bool   pileups [255] = {0};

  Read read;
  Write write;

private:
  std::size_t m_maxSize = 255;
};

inline Event& Event::operator=(Hit const & hit)
{
  labels  [0] = hit.label;
  nrjs    [0] = hit.nrj;
  nrj2s   [0] = hit.nrj2;
  nrjcals [0] = hit.nrjcal;
  nrj2cals[0] = hit.nrj2cal;
  times   [0] = hit.time;
  time2s  [0] = hit.time2;
  pileups [0] = hit.pileup;
  mult = 1;
  return *this;
}

inline Event& Event::operator=(Event const & evt)
{
  read = evt.read;
  write = evt.write;
  mult = evt.mult;
  for (int i = 0; i<mult; i++)
  {
    if (write.l) labels  [i] = evt.labels  [i];
    if (write.e) nrjs    [i] = evt.nrjs    [i];
    if (write.q) nrj2s   [i] = evt.nrj2s   [i];
    if (write.E) nrjcals [i] = evt.nrjcals [i];
    if (write.Q) nrj2cals[i] = evt.nrj2cals[i];
    if (write.t) times   [i] = evt.times   [i];
    if (write.T) time2s  [i] = evt.time2s  [i];
    if (write.p) pileups [i] = evt.pileups [i];
  }
#ifdef USE_RF
  if (write.RFp) RFtime   = evt.RFtime;
  if (write.RFp) RFperiod = evt.RFperiod;
#endif //USE_RF
  return *this;
}

void Event::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}

  write.setOptions(options);  

  tree -> ResetBranchAddresses();

                  tree -> Branch("mult"    , &mult);
  if ( write.l )  tree -> Branch("label"   , &labels  , "label[mult]/s"  );
  if ( write.t )  tree -> Branch("time"    , &times   , "time[mult]/l"   );
  if ( write.T )  tree -> Branch("time2"   , &time2s  , "time2[mult]/D"  );
  if ( write.e )  tree -> Branch("nrj"     , &nrjs    , "nrj[mult]/F"    );
  if ( write.E )  tree -> Branch("nrjcal"  , &nrjcals , "nrjcal[mult]/F" );
  if ( write.q )  tree -> Branch("nrj2"    , &nrj2s   , "nrj2[mult]/F"   );
  if ( write.Q )  tree -> Branch("nrj2cal" , &nrj2cals, "nrj2cal[mult]/F");
  if ( write.p )  tree -> Branch("pileup"  , &pileups , "pileup[mult]/O" );
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
               tree -> SetBranchAddress("mult"   , &mult    );
  if ( read.l) tree -> SetBranchAddress("label"  , &labels  );
  if ( read.t) tree -> SetBranchAddress("time"   , &times   );
  if ( read.T) tree -> SetBranchAddress("time2"  , &time2s  );
  if ( read.e) tree -> SetBranchAddress("nrj"    , &nrjs    );
  if ( read.E) tree -> SetBranchAddress("nrjcal" , &nrjcals );
  if ( read.q) tree -> SetBranchAddress("nrj2"   , &nrj2s   );
  if ( read.Q) tree -> SetBranchAddress("nrj2cal", &nrj2cals);
  if ( read.p) tree -> SetBranchAddress("pileup" , &pileups );
#ifdef USE_RF
  if ( read.p ) tree -> SetBranchAddress("RFtime"  , &RFtime  );
  if ( read.p ) tree -> SetBranchAddress("RFperiod", &RFperiod);
#endif //USE_RF
  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
               labels  [mult] = hit.label;
  if (write.t) times   [mult] = hit.time;
  if (write.T) time2s  [mult] = hit.time2;
  if (write.e) nrjs    [mult] = hit.nrj;
  if (write.E) nrjcals [mult] = hit.nrjcal;
  if (write.q) nrj2s   [mult] = hit.nrj2;
  if (write.Q) nrj2cals[mult] = hit.nrj2cal;
  if (write.p) pileups [mult] = hit.pileup;
  mult++;
}

inline void Event::push_front(Hit const & hit)
{
  for (uchar i = 0; i<mult; i++)
  {
                 labels  [mult+1] = labels  [mult];
    if (write.t) times   [mult+1] = times   [mult];
    if (write.T) time2s  [mult+1] = time2s  [mult];
    if (write.e) nrjs    [mult+1] = nrjs    [mult];
    if (write.E) nrjcals [mult+1] = nrjcals [mult];
    if (write.q) nrj2s   [mult+1] = nrj2s   [mult];
    if (write.Q) nrj2cals[mult+1] = nrj2cals[mult];
    if (write.p) pileups [mult+1] = pileups [mult];
  }
               labels  [0] = hit.label;
  if (write.t) times   [0] = hit.time;
  if (write.T) time2s  [0] = hit.time2;
  if (write.e) nrjs    [0] = hit.nrj;
  if (write.E) nrjcals [0] = hit.nrjcal;
  if (write.q) nrj2s   [0] = hit.nrj2;
  if (write.Q) nrj2cals[0] = hit.nrj2cal;
  if (write.p) pileups [0] = hit.pileup;
  mult++;
}

// template <class... T> void print(Event const & evt, T const & ... t2)
// {
//   evt.Print();
//   print(t2...);
// }

inline void Event::Print() const
{
  print("---");
  print(mult, "hits :");
  for (uchar i = 0; i<mult;i++)
  {
    print(
      "label :",labels[i],
      (times[i]) ? "time :" +std::to_string(times[i]) : "",
      (time2s[i]) ? "time :" +std::to_string(time2s[i])+" ns" : "",
      (nrjs[i]) ? "ADC :"+std::to_string(nrjs[i]) : "",
      (nrj2s[i]) ? "QDC2 :"+std::to_string(nrj2s[i]) : "",
      (nrjcals[i]) ? "energy :"+std::to_string(nrjcals[i]) : "",
      (nrj2cals[i]) ? "energy 2 :"+std::to_string(nrj2cals[i]) : "",
      (pileups[i]) ? "pileup" : ""
    );
  }
  print("---");
}

std::ostream& operator<<(std::ostream& cout, Event const & event)
{
  event.Print();
  return cout;
}

#endif //EVENT_HPP
