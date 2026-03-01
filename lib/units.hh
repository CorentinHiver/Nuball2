#pragma once
#include <cstdint>
#define Nuball2Units

//////////////////////
// Units definition //
//////////////////////

using Time   = int64_t;
using Energy_t = float ;

///////////////
// Constants //
///////////////

constexpr inline Time   seconds = 1.e+12; // unit base is ps
constexpr inline Energy_t MeVs    = 1.e+03; // unit base is keV
constexpr inline Energy_t Joules  = 6.2415091e+18;

//////////////////////
// Unit conversions //
//////////////////////

constexpr inline Time operator""_s (long double time) noexcept {return static_cast<Time>(time*seconds*1e+00);}
constexpr inline Time operator""_ms(long double time) noexcept {return static_cast<Time>(time*seconds*1e-03);}
constexpr inline Time operator""_us(long double time) noexcept {return static_cast<Time>(time*seconds*1e-06);}
constexpr inline Time operator""_ns(long double time) noexcept {return static_cast<Time>(time*seconds*1e-09);}
constexpr inline Time operator""_ps(long double time) noexcept {return static_cast<Time>(time*seconds*1e-12);}
constexpr inline Time operator""_fs(long double time) noexcept {return static_cast<Time>(time*seconds*1e-15);}

constexpr inline Time operator""_s (unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-00l);}
constexpr inline Time operator""_ms(unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-03l);}
constexpr inline Time operator""_us(unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-06l);}
constexpr inline Time operator""_ns(unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-09l);}
constexpr inline Time operator""_ps(unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-12l);}
constexpr inline Time operator""_fs(unsigned long long time) noexcept {return static_cast<Time>(time*seconds*1e-15l);}

// This code is based on keV.
constexpr inline Energy_t operator""_J  (long double energy) noexcept {return static_cast<Energy_t>(energy*Joules);}
constexpr inline Energy_t operator""_MeV(long double energy) noexcept {return static_cast<Energy_t>(energy*MeVs*1.0e+00);}
constexpr inline Energy_t operator""_keV(long double energy) noexcept {return static_cast<Energy_t>(energy*MeVs*1.0e-03);}

constexpr inline Energy_t operator""_J  (unsigned long long energy) noexcept {return static_cast<Energy_t>(energy*Joules);}
constexpr inline Energy_t operator""_MeV(unsigned long long energy) noexcept {return static_cast<Energy_t>(energy*MeVs*1.0e+00);}
constexpr inline Energy_t operator""_keV(unsigned long long energy) noexcept {return static_cast<Energy_t>(energy*MeVs*1.0e-03);}

// General units of numbers : Giga Mega kilo
constexpr inline long double operator""_G(long double number) noexcept {return static_cast<double>(number*1.e+09);}
constexpr inline long double operator""_M(long double number) noexcept {return static_cast<double>(number*1.e+06);}
constexpr inline long double operator""_k(long double number) noexcept {return static_cast<double>(number*1.e+03);}

constexpr inline long double operator""_G(unsigned long long number) noexcept {return static_cast<double>(number*1.e+09l);}
constexpr inline long double operator""_M(unsigned long long number) noexcept {return static_cast<double>(number*1.e+06l);}
constexpr inline long double operator""_k(unsigned long long number) noexcept {return static_cast<double>(number*1.e+03l);}

// General units of integer numbers : Giga Mega kilo
constexpr inline uint64_t operator""_Gi(         long double number) noexcept {return static_cast<uint64_t>(number*1.e+9);}
constexpr inline uint64_t operator""_Mi(         long double number) noexcept {return static_cast<uint64_t>(number*1.e+6);}
constexpr inline uint64_t operator""_ki(         long double number) noexcept {return static_cast<uint64_t>(number*1.e+3);}
constexpr inline uint64_t operator""_Gi(unsigned long long   number) noexcept {return number*1.e+9l;}
constexpr inline uint64_t operator""_Mi(unsigned long long   number) noexcept {return number*1.e+6l;}
constexpr inline uint64_t operator""_ki(unsigned long long   number) noexcept {return number*1.e+3l;}

// Angles :
constexpr inline double to_rad(double const & deg){return deg*3.14159/180.;}
constexpr inline double to_deg(double const & rad){return rad/3.14159*180.;}
constexpr inline double to_rad(long double const & deg){return deg*3.14159/180.;}
constexpr inline double to_deg(long double const & rad){return rad/3.14159*180.;}
constexpr inline double operator""_deg(long double number) noexcept {return to_rad(number);}