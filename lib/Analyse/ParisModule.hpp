#ifndef PARISMODULE_H
#define PARISMODULE_H

#include "../libRoot.hpp"
#include "ParisCrystal.hpp"

class ParisModule
{
public:
  ParisModule() : m_label(glabel) {glabel++; nbModules++;}

  void static newCluster() {glabel = 0;}
  auto const & label() const {return m_label;}

  operator bool() const & {return valid;}

  void Reset();

  void Fill(Event const & event, int const & i);

  char inline test_gate_classic();

  Float_t time  = 0.;
  Float_t nrj   = 0.;
  Float_t nrj2  = 0.;
  Float_t ratio = 0.;

  ParisCrystal labr3;
  ParisCrystal   nai;

  static int nbModules;

private:
  uchar const m_label;
  static uchar glabel;
  bool valid = true;
};

uchar ParisModule::glabel = 0;
int ParisModule::nbModules = 0;

void inline ParisModule::Reset()
{
  time = 0.;
  nrj = 0.;
  nrj2 = 0.;
  ratio = 0.;
  if (labr3) labr3.Reset();
  if (nai) nai.Reset();
}

void ParisModule::Fill(Event const & event, int const & i)
{
  time = event.time2s[i];
  nrj = event.nrjs[i];
  nrj2 = event.nrj2s[i];
  switch (test_gate_classic())
  {
    case -1 : valid = false;         break;
    case  0 : labr3.set(time, nrj ); break;
    case  1 : nai  .set(time, nrj2); break;
  }
}

char inline ParisModule::test_gate_classic()
{
  ratio = (nrj2-nrj)/nrj2;
       if (ratio < -0.2) return -1;
  else if (ratio <  0.2) return  0;
  else if (ratio <  0.5) return -1;
  else if (ratio <  0.7) return  1;
  else                   return -1;
}
// 
// char inline ParisModule::test_gate_tan()
// {
//   ratio = TMath::Tan((nrj2-nrj)/nrj2);
//        if (ratio < -0.2) return -1;
//   else if (ratio <  0.2) return  0;
//   else if (ratio <  0.5) return -1;
//   else if (ratio <  0.7) return  1;
//   else                   return -1;
// }


#endif //PARISMODULE_H
