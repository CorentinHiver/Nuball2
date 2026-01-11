#pragma once
#define Nuball2

#ifdef COMULTITHREADING
  std::mutex mutex_hits;
#endif //COMULTITHREADING

#include "units.hh"

//////////////////
/// Data types ///
//////////////////

using Label     = ushort;    // Label (ushort)
using ADC       = int;       // ADC (int)
using NRJ       = float;     // Energy in keV (float)
using Timestamp = ULong64_t; // Timestamp in ps (absolute)
using Time      = Time_t;    // Time in ps (relative)
using Time_ns   = float;     // Time in ns (relative) !deprecated! 
using Pileup    = bool;      // Pileup bit (bool) !unused!
using Index     = uchar;     // Used in analysis structures (Clovers, Paris...). Be careful to the max value of 255 !

////////////////////
/// Data vectors ///
////////////////////

using Label_vec   = std::vector<Label  >; // Vector of Label (ushort)
using ADC_vec     = std::vector<ADC    >; // Vector of ADC (int)
using Energy_vec  = std::vector<NRJ    >; // Vector of Energy in keV (float)
using Time_vec    = std::vector<Time_t >; // Vector of Time in ps (relative)
using Time_ns_vec = std::vector<Time_ns>; // Vector of Time in ns (relative) !deprecated!  
using Pileup_vec  = std::vector<Pileup >; // Vector of Pileup bit (bool) !unused!

//////////////////
/// Data casts ///
//////////////////

/// @brief Casts a number into unsigned Label
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Label Index_cast(T const & t) {return static_cast<Index>(t);}

/// @brief Casts a number into unsigned Label
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Label Label_cast(T const & t) {return static_cast<Label>(t);}

/// @brief Casts a number into unsigned ADC
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline ADC ADC_cast(T const & t) {return static_cast<ADC>(t);}

/// @brief Casts a number into unsigned Time
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Time Time_cast(T const & t) {return static_cast<Time>(t);}

/// @brief Casts a number into unsigned NRJ
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline NRJ NRJ_cast(T const & t) {return static_cast<NRJ>(t);}

/// @brief Casts a number into unsigned Time_ns
/// @deprecated
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Time_ns Time_ns_cast(T const & t) {return static_cast<Time_ns>(t);}

/// @brief Casts a number into unsigned Timestamp
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Timestamp Timestamp_cast(T const & t) {return static_cast<Timestamp>(t);}
