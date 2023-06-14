#ifndef HIT_H
#define HIT_H

#include <libRoot.hpp>
#include <libCo.hpp>

using Label  = ushort;
using Label_vec = std::vector<Label>;
using ADC    = int;
using ADC_vec = std::vector<ADC>;
using NRJ = float;
using Energy_vec = std::vector<NRJ>;
using Time   = ULong64_t;
using Time_vec = std::vector<Time>;
using Pileup = bool;
using Pileup_vec = std::vector<bool>;

class Hit
{
public:
  Label label  = 0;     // Label
  float nrj    = 0.f;   // Energy
  float nrj2   = 0.f;   // used if QDC2
  NRJ   nrjcal = 0;     // Calibrated energy
  Time  time   = 0;     // Time
  bool  pileup = false; // Pile-up

  void reset()
  {
    label  = 0;
    nrj    = 0;
    nrj2   = 0;
    nrjcal = 0;
    time   = 0;
    pileup = 0;
  }

  float gate_ratio(){if (nrj2 != 0) return ( static_cast<float>((nrj2-nrj)/nrj2) ); else return 0.f;}
};

template <class... T> void print(Hit const & hit, T const & ... t2)
{
  print(hit);
  print(t2...);
}

void print(Hit hit)
{
  std::cout << "l : " << hit.label;
  if (hit.nrj >0)   std::cout << " nrj :  "   << hit.nrj ;
  if (hit.nrj2>0)   std::cout << " nrj2 : "   << hit.nrj2;
  if (hit.nrjcal>0) std::cout << " nrjcal : " << hit.nrjcal;
  std::cout << " time : " << hit.time;
  if (hit.pileup) std::cout << " pileup";
  std::cout << std::endl;
}

class Hit_ptr
{// Deprecated for now
public:

  Label * label  = nullptr; // Label
  float * nrj    = nullptr; // Eenergy
  float * nrj2   = nullptr; // used if QDC2
  NRJ   * nrjcal = nullptr; // Calibrated energy
  Time  * time   = nullptr; // Time
  float * time2  = nullptr; // Relative time
  bool  * pileup = nullptr; // Pile-up

  void reset()
  {
    label  = nullptr;
    nrj    = nullptr;
    nrj2   = nullptr;
    nrjcal = nullptr;
    time   = nullptr;
    time2  = nullptr;
    pileup = nullptr;
  }

  float gate_ratio(){if ((*nrj2) != 0) return ( static_cast<float>(((*nrj2)-(*nrj))/(*nrj2)) ); else return 0.f;}

};

#endif //HIT_H
