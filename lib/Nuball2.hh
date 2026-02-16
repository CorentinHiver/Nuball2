#pragma once
#define Nuball2

#ifdef COMULTITHREADING
  std::mutex mutex_hits;
#endif //COMULTITHREADING

#include "units.hh"
#include "libCo.hpp"
#include "RtypesCore.h"
#include "TObject.h"

//////////////////
/// Data types ///
//////////////////

using Label     = ushort;    // Label (ushort)
using ADC       = int;       // ADC (int)
using NRJ       = float;     // Energy in keV (float)
using Timestamp = ULong64_t; // Timestamp in ps (absolute)
// using Time      = Time;    // Time in ps (relative)
using Time_ns   = float;     // Time in ns (relative) !deprecated! 
using Pileup    = bool;      // Pileup bit (bool) !unused!
using Index     = uchar;     // Used in analysis structures (Clovers, Paris...). Be careful to the max value of 255 !

////////////////////
/// Data vectors ///
////////////////////

using Label_vec   = std::vector<Label  >; // Vector of Label (ushort)
using ADC_vec     = std::vector<ADC    >; // Vector of ADC (int)
using Energy_vec  = std::vector<NRJ    >; // Vector of Energy in keV (float)
using Time_vec    = std::vector<Time   >; // Vector of Time in ps (relative)
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

/// @brief Casts a number into unsigned Timestamp
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline ULong64_t ULong64_cast(T const & t) {return static_cast<ULong64_t>(t);}

/// @brief Casts a number into unsigned Timestamp
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline Long64_t Long64_cast(T const & t) {return static_cast<Long64_t>(t);}

class RootHeader : public TObject
{
  int status = 0;
  inline constexpr static int calibrated = 0b00001;
  inline constexpr static int timeshift = 0b00010;
  inline constexpr static int corrected = 0b00100;

public:
  std::string comments;
  bool isCalibrated() const {return status & calibrated;}
  void setCalibrated() {status |= calibrated;}
  bool isTimeshifted() const {return status & timeshift;}
  void setTimeshifted() {status |= timeshift;}
};

namespace NSI136
{
  static constexpr size_t LUT_s = 1000; // Look-up-table size

  static constexpr auto cloverIndex  = Colib::LUT<LUT_s> ([](Label const & label) -> Index {return Index_cast((label-23)/6);});
  static constexpr auto crystalIndex = Colib::LUT<LUT_s> ([](Label const & label) -> Index {return Index_cast((label-23)%6);});
  static constexpr auto isClover     = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 22 < label && label < 168;});
  static constexpr auto isGe         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && crystalIndex[label] > 1;});
  static constexpr auto isBGO        = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && crystalIndex[label] < 2;});
  static constexpr auto isR2         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && cloverIndex [label] > 12;});
  static constexpr auto isR3         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && cloverIndex [label] < 13;});

  static constexpr auto isRF         = Colib::LUT<LUT_s> ([](Label const & label) -> bool {return label == 251;});

  static constexpr auto isRefLaBr    = Colib::LUT<LUT_s> ([](Label const & label) -> bool {return label == 252;});

  // static constexpr auto isParisBR1   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 200 < label && label < 209;});
  static constexpr auto isBR2   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 300 < label && label < 317;});
  static constexpr auto isBR3   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 400 < label && label < 413;});
  static constexpr auto isFR1   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 500 < label && label < 509;});
  static constexpr auto isFR2   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 600 < label && label < 617;});
  static constexpr auto isFR3   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 700 < label && label < 713;});
  static constexpr auto isParis = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return (isBR2[label] || isBR3[label] || isFR1[label] || isFR2[label] || isFR3[label]);});
  
  static constexpr auto isRing    = Colib::LUT<LUT_s> ([](Label const & label) {return 839 < label && label < 856;});
  static constexpr auto isSector1 = Colib::LUT<LUT_s> ([](Label const & label) {return 799 < label && label < 820;});
  static constexpr auto isSector2 = Colib::LUT<LUT_s> ([](Label const & label) {return 819 < label && label < 840;});
  static constexpr auto isSector  = Colib::LUT<LUT_s> ([](Label const & label) {return isSector1[label] || isSector2[label];});
  static constexpr auto isDSSD    = Colib::LUT<LUT_s> ([](Label const & label) {return isRing[label] || isSector[label];});

  static constexpr Label maxLabel = Label_cast(LUT_s);
}