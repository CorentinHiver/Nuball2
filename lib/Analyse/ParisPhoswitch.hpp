#ifndef PARIS_PHOSWITCH_HPP
#define PARIS_PHOSWITCH_HPP

#include "../libRoot.hpp"

class ParisPhoswitch
{
public:
  ParisPhoswitch() {}

  auto setLabel(uchar const & label) {m_label = label;}
  auto const & label() const {return m_label;}

  operator bool() {return (m_state>-1);}

  void Reset();

  void Fill(Event const & event, int const & i);

  char inline test_gate_classic();
  char inline test_gate_tan();

  Time_ns time  = 0.;
  NRJ nrj   = 0.;
  NRJ nrj2  = 0.;
  NRJ ratio = 0.;

private:
  uchar m_label = -1;
  char m_state = -1;
};


void inline ParisPhoswitch::Reset()
{
  time = 0.f;
  nrj = 0.f;
  nrj2 = 0.f;
  ratio = 0.f;
  m_state = -1;
}

void ParisPhoswitch::Fill(Event const & event, int const & i)
{
  time = event.times[i];
  nrj = NRJ_cast(event.nrjs[i]);
  nrj2 = NRJ_cast(event.nrj2s[i]);
  m_state = test_gate_classic();
}

char inline ParisPhoswitch::test_gate_classic()
{
  ratio = (nrj2-nrj)/nrj2;
       if (ratio < -0.2) return -1;
  else if (ratio <  0.2) return  0;
  else if (ratio <  0.5) return -1;
  else if (ratio <  0.7) return  1;
  else                   return -1;
}

/// @brief 
/// @todo 
char inline ParisPhoswitch::test_gate_tan()
{
  ratio = TMath::Tan((nrj2-nrj)/nrj2);
       if (ratio < -0.2) return -1;
  else if (ratio <  0.2) return  0;
  else if (ratio <  0.5) return -1;
  else if (ratio <  0.7) return  1;
  else                   return -1;
}


#endif //PARIS_PHOSWITCH_HPP
