#ifndef PARICRYSTAL_H
#define PARICRYSTAL_H

#include "../libRoot.hpp"

class ParisCrystal
{
public:
  ParisCrystal(){}

  operator bool() const & {return exists;}

  void set(Float_t const & time, Float_t const & nrj);

  void Reset()
  {
    exists = false;
    if (nrj>1.)
    {
      nrj  = 0.;
      time = 0.;
    }
  }

  Float_t time   = 0;
  Float_t nrj = 0;
  bool exists = false;
};

void ParisCrystal::set(Float_t const & _time, Float_t const & _energy)
{
  time   = _time  ;
  nrj = _energy;
  exists = true;
}


#endif //PARICRYSTAL_H
