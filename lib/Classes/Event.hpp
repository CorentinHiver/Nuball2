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
    mult   (event.mult   ),
    read   (event.read   ),
    write  (event.write  ),
    m_maxSize(event.m_maxSize)
  {
    std::copy_n(event.labels   , mult ,  labels   );
    std::copy_n(event.times    , mult ,  times    );
    std::copy_n(event.nrjs     , mult ,  nrjs     );
    std::copy_n(event.nrj2s    , mult ,  nrj2s    );
    std::copy_n(event.nrjcals  , mult ,  nrjcals  );
    std::copy_n(event.nrj2cals , mult ,  nrj2cals );
    std::copy_n(event.time2s   , mult ,  time2s   );
    std::copy_n(event.pileups  , mult ,  pileups  );
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
  void* hits[8] = {labels, times, time2s, nrjs, nrjcals, nrj2s, nrj2cals, pileups};
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

inline Event& Event::operator=(Event const & event)
{
  read  = event.read;
  write = event.write;
  mult  = event.mult;
  if (write.l) std::copy_n(event.labels   , mult ,  labels   );
  if (write.t) std::copy_n(event.times    , mult ,  times    );
  if (write.T) std::copy_n(event.nrjs     , mult ,  nrjs     );
  if (write.e) std::copy_n(event.nrj2s    , mult ,  nrj2s    );
  if (write.E) std::copy_n(event.nrjcals  , mult ,  nrjcals  );
  if (write.q) std::copy_n(event.nrj2cals , mult ,  nrj2cals );
  if (write.Q) std::copy_n(event.time2s   , mult ,  time2s   );
  if (write.p) std::copy_n(event.pileups  , mult ,  pileups  );
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
  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
#ifdef SAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif
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
#ifdef SAFE
  if (mult>254) {print("Event mult > 255 !!"); return;}
#endif
  for (int i = 0; i<mult; i++)
  {
                 labels  [i+1] = labels  [i];
    if (write.t) times   [i+1] = times   [i];
    if (write.T) time2s  [i+1] = time2s  [i];
    if (write.e) nrjs    [i+1] = nrjs    [i];
    if (write.E) nrjcals [i+1] = nrjcals [i];
    if (write.q) nrj2s   [i+1] = nrj2s   [i];
    if (write.Q) nrj2cals[i+1] = nrj2cals[i];
    if (write.p) pileups [i+1] = pileups [i];
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

inline void Event::Print() const
{
  print(*this);
}

std::ostream& operator<<(std::ostream& cout, Event const & event)
{
  cout << std::endl << "---" << std::endl;
  cout << event.mult << " hits : " << std::endl;
  for (uchar i = 0; i<event.mult;i++)
  {
    cout << "label : " << event.labels[i];
    if(event.times    [i]) cout << "time : "     + std::to_string(event.times    [i])      ;
    if(event.time2s   [i]) cout << "time : "     + std::to_string(event.time2s   [i])+" ns";
    if(event.nrjs     [i]) cout << "ADC : "      + std::to_string(event.nrjs     [i])      ;
    if(event.nrj2s    [i]) cout << "QDC2 : "     + std::to_string(event.nrj2s    [i])      ;
    if(event.nrjcals  [i]) cout << "energy : "   + std::to_string(event.nrjcals  [i])      ;
    if(event.nrj2cals [i]) cout << "energy 2 : " + std::to_string(event.nrj2cals [i])      ;
    if(event.pileups  [i]) cout << "pileup"                                                ;
    cout << std::endl;
  }
  return cout;
}

#endif //EVENT_HPP
