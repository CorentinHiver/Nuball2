#ifndef PARIS_PHOSWITCH_HPP
#define PARIS_PHOSWITCH_HPP

#include "../libRoot.hpp"

class ParisPhoswitch
{
public:
  ParisPhoswitch() {}

  auto setLabel(uchar const & label) {m_label = label;}
  auto const & label() const {return m_label;}

  operator bool() const noexcept {return (cristal>=0);}

  void inline reset();

  char inline fill(Event const & event, int const & hit_i);

  static char inline test_gate_classic(float const & _nrj, float const & _nrj2);
  static char inline test_gate_tan(float const & _nrj, float const & _nrj2);
  static char inline simple_pid(float const & _nrj, float const & _nrj2);

  auto isLaBr3() const {return cristal == 0;}
  auto isNaI() const {return cristal == 1;}
  auto isMixed() const {return cristal == 2;}

  Time time  = 0ll;
  float nrj   = 0.f;
  char cristal = -1; // -1 : rejected, 0 : LaBr3, 1 : NaI, 2 : mixed event

private:
  uchar m_label = -1;
};

std::vector<std::string> ParisPhoswitch_cristals = {"LaBr3", "NaI", "Mix"};

std::ostream& operator<<(std::ostream& out, ParisPhoswitch const & phoswitch)
{
  out << "cristal : ";
  if (0 <= phoswitch.cristal) out << ParisPhoswitch_cristals[phoswitch.cristal]; 
  else                        out << "rejected"; 
  out << " time : " << phoswitch.time << " ps nrj : " << phoswitch.nrj << " keV ";
  return out;
}

void inline ParisPhoswitch::reset()
{
  time = 0ll;
  nrj = 0.f;
  cristal = -1;
}

char inline ParisPhoswitch::fill(Event const & event, int const & hit_i)
{
  if (event.nrjs[hit_i] < 10_keV || event.nrj2s[hit_i] < 10_keV) return -1;
  
  cristal = simple_pid(event.nrjs[hit_i], event.nrj2s[hit_i]);
  time = event.times[hit_i];
  if (cristal == 0) nrj = float_cast(event.nrj2s[hit_i]);
  if (cristal == 1) nrj = float_cast(event.nrjs[hit_i]);
  return cristal;
}

char inline ParisPhoswitch::simple_pid(float const & _nrj, float const & _nrj2)
{
  if (_nrj2==0.f) return -1;
  auto const & ratio = _nrj/_nrj2;
       if (ratio < 0.3) return -1;
  else if (ratio < 0.5) return  1;
  else if (ratio < 0.8) return  2;
  else if (ratio < 1.2) return  0;
  else                  return -1;
}

char inline ParisPhoswitch::test_gate_classic(float const & _nrj, float const & _nrj2)
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
char inline ParisPhoswitch::test_gate_tan(float const & _nrj, float const & _nrj2)
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
