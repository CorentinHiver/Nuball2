#ifndef UTILS_H
#define UTILS_H

inline constexpr bool gate(double const & low, double const & energy, double const & high) noexcept {return (low < energy && energy < high);}
inline constexpr bool gate(Time const & low, Time const & time, Time const & high) noexcept {return (low < time && time < high);}
inline constexpr bool gate(Label const & low, Label const & label, Label const & high) noexcept {return (low < label && label < high);}
inline constexpr bool gate(size_t const & low, size_t const & label, size_t const & high) noexcept {return (low < label && label < high);}

inline constexpr bool gate_proton(Time const & time) {return gate(-5_ns, time, 5_ns);}
inline constexpr bool gate_deuton(Time const & time) {return gate(0_ns, time, 10_ns);}
inline constexpr bool gate_alpha(Time const & time) {return gate(0_ns, time, 10_ns);}



float smearGe(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

float smearParis(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

#endif //UTILS_H