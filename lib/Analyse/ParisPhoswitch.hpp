#ifndef PARIS_PHOSWITCH_HPP
#define PARIS_PHOSWITCH_HPP

#include "../libRoot.hpp"

class ParisPhoswitch
{
public:
  ParisPhoswitch() {}

  auto setLabel(uchar const & label) {m_label = label;}
  auto const & label() const {return m_label;}

  operator bool() {return (state>-1);}

  void inline reset();

  void inline fill(Event const & event, int const & i);

  static char inline test_gate_classic(NRJ const & _nrj, NRJ const & _nrj2);
  static char inline test_gate_tan(NRJ const & _nrj, NRJ const & _nrj2);
  static char inline test_gate_simple(NRJ const & _nrj, NRJ const & _nrj2);

  Time time  = 0ll;
  NRJ nrj   = 0.f;
  NRJ nrj2  = 0.f;
  char state = -1;

private:
  uchar m_label = -1;
};

void inline ParisPhoswitch::reset()
{
  time = 0ll;
  nrj = 0.f;
  nrj2 = 0.f;
  state = -1;
}

void inline ParisPhoswitch::fill(Event const & event, int const & i)
{
  time = event.times[i];
  nrj = NRJ_cast(event.nrjs[i]);
  nrj2 = NRJ_cast(event.nrj2s[i]);
  state = test_gate_simple(nrj, nrj2);
}

char inline ParisPhoswitch::test_gate_simple(NRJ const & _nrj, NRJ const & _nrj2)
{
  if (_nrj2==0) return -1;
  auto const & ratio = _nrj/_nrj2;
       if (ratio < 0.3) return -1;
  else if (ratio < 0.5) return  0;
  else if (ratio < 0.8) return  2;
  else if (ratio < 1.2) return  1;
  else                  return -1;
}

char inline ParisPhoswitch::test_gate_classic(NRJ const & _nrj, NRJ const & _nrj2)
{
  if (_nrj2==0) return -1;
  auto const & ratio = (_nrj2-_nrj)/_nrj2;
       if (ratio < -0.2) return -1;
  else if (ratio <  0.2) return  0;
  else if (ratio <  0.5) return  2;
  else if (ratio <  0.7) return  1;
  else                   return -1;
}

/// @brief 
/// @todo 
char inline ParisPhoswitch::test_gate_tan(NRJ const & _nrj, NRJ const & _nrj2)
{
  if (_nrj2==0) return -1;
  auto const & ratio = TMath::Tan((_nrj2-_nrj)/_nrj2);
       if (ratio < -0.2) return -1;
  else if (ratio <  0.2) return  0;
  else if (ratio <  0.5) return  2;
  else if (ratio <  0.7) return  1;
  else                   return -1;
}


#endif //PARIS_PHOSWITCH_HPP
