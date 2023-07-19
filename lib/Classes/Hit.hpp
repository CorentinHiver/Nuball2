#ifndef HIT_H
#define HIT_H

#include "../libRoot.hpp"

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
  Hit(){reset();}
  Hit(Label _label, float _nrj, Time _time, float _nrj2 = 0, NRJ _nrjcal = 0, NRJ _nrj2cal = 0, bool _pileup = false) : 
    label(_label),
    nrj(_nrj),
    time(_time),
    nrj2(_nrj2),
    nrjcal(_nrjcal),
    nrj2cal(_nrj2cal),
    pileup(_pileup)
    {}

  Label label  = 0;     // Label
  float nrj    = 0.f;   // Energy
  Time  time   = 0ull;  // Timestamp ('ull' stands for unsigned long long)
  float nrj2   = 0.f;   // used if QDC2
  NRJ   nrjcal = 0.f;   // Calibrated energy
  NRJ   nrj2cal= 0.f;   // Calibrated QDC2
  bool  pileup = false; // Pile-up

  void reset()
  {
    label  = 0;
    nrj    = 0.f;
    nrj2   = 0.f;
    nrj2cal= 0.f;
    nrjcal = 0.f;
    time   = 0ull;
    pileup = false;
  }

  void connect(TTree * tree);
};

void Hit::connect(TTree * tree)
{
  tree -> Branch("label"  , & label  );
  tree -> Branch("nrj"    , & nrj    );
  tree -> Branch("nrj2"   , & nrj2   );
  tree -> Branch("time"   , & time   );
  tree -> Branch("pileup" , & pileup );
}

std::ostream& operator<<(std::ostream& cout, Hit const & hit)
{
  cout << "l : " << hit.label;
  if (hit.nrj >0)   cout << " nrj :  "   << hit.nrj ;
  if (hit.nrj2>0)   cout << " nrj2 : "   << hit.nrj2;
  if (hit.nrjcal>0) cout << " nrjcal : " << hit.nrjcal;
  cout << " time : " << hit.time;
  if (hit.pileup) cout << " pileup";
  return cout;
}

/**
 * @brief @deprecated
 * 
 */
class Hit_ptr
{
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
