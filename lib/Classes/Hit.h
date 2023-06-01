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
  Label   label  = 0; //Hit label
  float nrj    = 0; //energy
  float nrj2   = 0; //used if QDC2
  NRJ     nrjcal = 0; // Calibrated energy
  Time    time   = 0; //time
  bool    pileup = 0; //pile-up

  void reset()
  {
    label  = 0;
    nrj    = 0;
    nrj2   = 0;
    nrjcal = 0;
    time   = 0;
    pileup = 0;
  }

  float gate_ratio(){if (nrj2 != 0) return ((float)(nrj2-nrj)/nrj2); else return 0.f;}
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
  // if (hit.nrj3>0)   std::cout << " nrj3 : "   << hit.nrj3;
  // if (hit.nrj4>0)   std::cout << " nrj4 : "   << hit.nrj4;
  if (hit.nrjcal>0) std::cout << " nrjcal : " << hit.nrjcal;
  std::cout << " time : " << hit.time;
  if (hit.pileup) std::cout << " pileup";
  std::cout << std::endl;
}

class Hit_ptr
{
public:

  Label   * label  = nullptr; //Hit label
  float * nrj    = nullptr; //energy
  float * nrj2   = nullptr; //used if QDC2
  NRJ     * nrjcal = nullptr; // Calibrated energy
  Time    * time   = nullptr; //time
  float   * time2  = nullptr; //time
  bool    * pileup = nullptr; //pile-up

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

  float gate_ratio(){if ((*nrj2) != 0) return ((float)((*nrj2)-(*nrj))/(*nrj2)); else return 0.f;}

};

#endif //HIT_H
