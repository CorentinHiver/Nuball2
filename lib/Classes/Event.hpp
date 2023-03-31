#ifndef EVENT_H
#define EVENT_H

#include "Hit.h"

/*

READ/WRITE OPTIONS :

l : label               UShort_t
t : absolute timestamp  ULong64_t
T : relative timestamp  Float_t
n : energy              Float_t
N : energy QDC2         Float_t
p : pileup              Bool_t

R : RFtime              ULong64_t
P : RFperiod            Float_t

*/

class Event
{
public:

  Event(){initialise();}

  Event (Hit const & hit)
  {
    initialise(1);
    *this = hit;
  }

  Event(TTree * tree, std::string const & options = "ltnN")
  {
    initialise();
    connect(tree, options);
  }

  void initialise(int i_max = 255);

  Hit operator[] (Int_t const & i);
  void operator= (Hit const & hit);

  void connect(TTree * tree, std::string const & options = "ltnN");
  void writeTo(TTree * tree, std::string const & options = "ltnN");

  void push_back(Hit const & hit);
  void push_front(Hit const & hit);

  void Print();
  void clear() { mult = 0; }
  size_t size() const { return static_cast<size_t>(mult); }
  bool isSingle() const {return (mult == 1);}

  bool const & readTime() const {return read_T;}
  bool const & readtime() const {return read_t;}

  int      mult     = 0;
  Time     RFtime   = 0;
  Float_t  RFperiod = 399998.;

  UShort_t labels [255];
  Float_t  nrjs   [255];
  Float_t  nrj2s  [255];
  Time     times  [255];
  Float_t  Times  [255];
  Bool_t   pileups[255];
  Detector types  [255];

private:

  bool read_l = false;
  bool read_t = false;
  bool read_T = false;
  bool read_E = false;
  bool read_E2 = false;
  bool read_p = false;
  bool read_RFt = false;
  bool read_RFp = false;

  bool write_l = false;
  bool write_t = false;
  bool write_T = false;
  bool write_E = false;
  bool write_E2 = false;
  bool write_p = false;
  bool write_RFt = false;
  bool write_RFp = false;
};

inline void Event::initialise(int i_max )
{
  for (int i = 0; i<i_max; i++)
  {
    labels  [i] = 0;
    nrjs    [i] = 0;
    nrj2s   [i] = 0;
    times   [i] = 0;
    pileups [i] = 0;
    types   [i] = null;
  }
}

inline Hit Event::operator[] (Int_t const & i)
{
  Hit ret;
  ret.nrjcal = nrjs[i];
  ret.nrj2   = nrj2s[i];
  ret.label  = labels[i];
  ret.time   = times[i];
  if (read_T || write_T) ret.time   = Times[i];
  ret.pileup = pileups[i];
  ret.type   = types[i];
  return ret;
}

inline void Event::operator= (Hit const & hit)
{
  labels[0]  = hit.label;
  nrjs[0]    = hit.nrjcal;
  nrj2s[0]   = hit.nrj2;
  times[0]   = hit.time;
  pileups[0] = hit.pileup;
  types[0]   = type_det(hit.label);
  mult = 1;
}

void Event::writeTo(TTree * tree, std::string const & options)
{
  if (!tree) {print("Output tree at address 0x00 !"); return;}
  for (auto const & e : options)
  {
    switch (e)
    {
      case ('l') : write_l    =  true;  break;
      case ('t') : write_t    =  true;  break;
      case ('T') : write_T    =  true;  break;
      case ('n') : write_E    =  true;  break;
      case ('N') : write_E2   =  true;  break;
      case ('p') : write_p    =  true;  break;
      case ('R') : write_RFt  =  true;  break;
      case ('P') : write_RFp  =  true;  break;
    }
  }

  tree -> ResetBranchAddresses();

                   tree -> Branch("mult"     , &mult);
  if (write_l   )  tree -> Branch("label"    , &labels , "label[mult]/s" );
  if (write_t   )  tree -> Branch("time"     , &times  , "time[mult]/l"  );
  if (write_T   )  tree -> Branch("Time"     , &Times  , "Time[mult]/F"  );
  if (write_E   )  tree -> Branch("nrj"      , &nrjs   , "nrj[mult]/F"   );
  if (write_E2  )  tree -> Branch("nrj2"     , &nrj2s  , "nrj2[mult]/F"  );
  if (write_p   )  tree -> Branch("pileup"   , &pileups, "pileup[mult]/O");
  if (write_RFt )  tree -> Branch("RFtime"   , &RFtime  );
  if (write_RFp )  tree -> Branch("RFperiod" , &RFperiod);

  tree -> SetBranchStatus("*",true);
}

void Event::connect(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}
  for (auto const & e : options)
  {
    switch (e)
    {
      case ('l') : read_l    =  true;  break;
      case ('t') : read_t    =  true;  break;
      case ('T') : read_T    =  true;  break;
      case ('n') : read_E    =  true;  break;
      case ('N') : read_E2   =  true;  break;
      case ('p') : read_p    =  true;  break;
      case ('R') : read_RFt  =  true;  break;
      case ('P') : read_RFp  =  true;  break;
    }
  }
  tree -> ResetBranchAddresses();
               tree -> SetBranchAddress("mult"    , &mult    );
  if (read_l ) tree -> SetBranchAddress("label"   , &labels  );
  if (read_t ) tree -> SetBranchAddress("time"    , &times   );
  if (read_T ) tree -> SetBranchAddress("Time"    , &Times   );
  if (read_E ) tree -> SetBranchAddress("nrj"     , &nrjs    );
  if (read_E2) tree -> SetBranchAddress("nrj2"    , &nrj2s   );
  if (read_p ) tree -> SetBranchAddress("pileup"  , &pileups );
  if (read_p ) tree -> SetBranchAddress("RFtime"  , &RFtime  );
  if (read_p ) tree -> SetBranchAddress("RFperiod", &RFperiod);
  tree -> SetBranchStatus("*",true);
}

inline void Event::push_back(Hit const & hit)
{
  labels[mult]  = hit.label;
  nrjs[mult]    = hit.nrjcal;
  nrj2s[mult]   = hit.nrj2;
  times[mult]   = hit.time;
  pileups[mult] = hit.pileup;
  types[mult]   = static_cast<Detector> (hit.type);
  mult++;
}

inline void Event::push_front(Hit const & hit)
{
  for (uchar i = 0; i<mult; i++)
  {
    labels[mult+1]  = labels[mult];
    nrjs[mult+1]    = nrjs[mult];
    nrj2s[mult+1]   = nrj2s[mult];
    times[mult+1]   = times[mult];
    pileups[mult+1] = pileups[mult];
    types[mult+1]   = types[mult];
  }
  labels[0]  = hit.label;
  nrjs[0]    = hit.nrjcal;
  nrj2s[0]   = hit.nrjcal;
  times[0]   = hit.time;
  pileups[0] = hit.pileup;
  types[0]   = type_det(hit.label);
  mult++;
}

inline void Event::Print()
{
  for (uchar i = 0; i<mult;i++)
  {
    print(
      "label :",labels[i],
      "time :" ,times[i],
      (types[i]) ? "type : "+type_str[types[i]] : "",
      (nrjs[i]) ? "energy :"+std::to_string(nrjs[i]) : "",
      (nrj2s[i]) ? "energy2 :"+std::to_string(nrj2s[i]) : "",
      (pileups[i]) ? "pileup" : ""
    );
  }
}

// Include useful classes
#include "Counters.hpp"

#endif //EVENT_H
