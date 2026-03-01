#pragma once

#define Nuball2

#ifdef CoMT
  std::mutex Nuball2mutex;
#endif // CoMT

#include "units.hh"
#include "libCo.hpp"
#include "RtypesCore.h"

//////////////////
/// Data types ///
//////////////////

using Label     = ushort;    // Label (ushort)
using ADC       = int;       // ADC (int)
using NRJ       = Energy_t;  // Energy in keV (float)
using Timestamp = ULong64_t; // Timestamp in ps (absolute)
using Time_ns   = float;     // Time in ns (relative) !deprecated! 
using Pileup    = bool;      // Pileup bit (bool) !unused!
using Index     = uint8_t;   // Used in analysis structures (Clovers, Paris...). Be careful to the max value of 255 !
// using Time      = int64_t;    // Time in ps (relative) Already declared in units.hh
// using Time      = int64_t;    // Time in ps (relative) Already declared in units.hh

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

inline std::string runNumberStr(std::string const & runFilename) {return Colib::split(runFilename, '_')[1];}
inline int         runNumber   (std::string const & runFilename) {return std::stoi(runNumberStr(runFilename));}

////////////////
// MAX values //
////////////////

constexpr inline auto LabelMax = std::numeric_limits<Label>::max();

namespace NSI136
{
  static constexpr size_t LUT_s = 1000; // Look-up-table size

  // CLOVERS //

  static constexpr auto cloverIndex  = Colib::LUT<LUT_s> ([](Label const & label) -> Index {return Index_cast((label-23)/6);});
  static constexpr auto crystalIndex = Colib::LUT<LUT_s> ([](Label const & label) -> Index {return Index_cast((label-23)%6);});
  static constexpr auto isClover     = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 22 < label && label < 168;});
  static constexpr auto isGe         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && crystalIndex[label] > 1;});
  static constexpr auto isBGO        = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && crystalIndex[label] < 2;});
  static constexpr auto isR2         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && cloverIndex [label] > 12;});
  static constexpr auto isR3         = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return isClover[label] && cloverIndex [label] < 13;});
  
  //   RF   //

  static constexpr auto isRF         = Colib::LUT<LUT_s> ([](Label const & label) -> bool {return label == 251;});
  
  //   Ref   //

  static constexpr auto isRefLaBr    = Colib::LUT<LUT_s> ([](Label const & label) -> bool {return label == 252;});

    
  //  PARIS  //

  // static constexpr auto isBR1   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 200 < label && label < 209;});
  static constexpr auto isBR2   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 300 < label && label < 317;});
  static constexpr auto isBR3   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 400 < label && label < 413;});
  static constexpr auto isFR1   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 500 < label && label < 509;});
  static constexpr auto isFR2   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 600 < label && label < 617;});
  static constexpr auto isFR3   = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return 700 < label && label < 713;});
  static constexpr auto isBR    = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return Colib::lut_OR(label, isBR2, isBR3);});
  static constexpr auto isFR    = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return Colib::lut_OR(label, isFR1, isFR2, isFR3);});
  static constexpr auto isParis = Colib::LUT<LUT_s> ([](Label const & label) -> bool  {return Colib::lut_OR(label, isBR2, isBR3, isFR1, isFR2, isFR3);});
  static constexpr auto ParisRingIndex = Colib::LUT<LUT_s> ([](Label const label) -> Label  
  {
      if (!isParis[label]) return LabelMax;
         if (isBR2[label]) return label - 301;
    else if (isBR3[label]) return label - 401;
    else if (isFR1[label]) return label - 501;
    else if (isFR2[label]) return label - 601;
    else if (isFR3[label]) return label - 701;
    else return LabelMax;
  });
  static constexpr auto ParisIndex = Colib::LUT<LUT_s> ([](Label const label) -> Label  
  {
      if (!isParis[label]) return LabelMax;
         if (isBR2[label]) return ParisRingIndex[label];
    else if (isBR3[label]) return ParisRingIndex[label] + Colib::lutEntries(isBR2);
    else if (isFR1[label]) return ParisRingIndex[label] + Colib::lutsEntries_sum(isBR2, isBR3);
    else if (isFR2[label]) return ParisRingIndex[label] + Colib::lutsEntries_sum(isBR2, isBR3, isFR1);
    else if (isFR3[label]) return ParisRingIndex[label] + Colib::lutsEntries_sum(isBR2, isBR3, isFR1, isFR2);
    else return LabelMax;
  });
  static constexpr auto ParisClusterIndex = Colib::LUT<LUT_s> ([](Label const label) -> Label  
  {
     if (!isParis[label]) return LabelMax;
    else if (isBR[label]) return ParisIndex[label];
    else if (isFR[label]) return ParisIndex[label] - Colib::lutEntries(isBR);
    else return LabelMax;
  });

  //  DSSD  //
  
  static constexpr auto isRing    = Colib::LUT<LUT_s> ([](Label const & label) {return 839 < label && label < 856;});
  static constexpr auto isSector1 = Colib::LUT<LUT_s> ([](Label const & label) {return 799 < label && label < 820;});
  static constexpr auto isSector2 = Colib::LUT<LUT_s> ([](Label const & label) {return 819 < label && label < 840;});
  static constexpr auto isSector  = Colib::LUT<LUT_s> ([](Label const & label) {return Colib::lut_OR(label, isSector1, isSector2);});
  static constexpr auto isDSSD    = Colib::LUT<LUT_s> ([](Label const & label) {return Colib::lut_OR(label, isRing, isSector);});

  static constexpr Label maxLabel = Label_cast(LUT_s);
}

// #include "TObject.h"
// class RootHeader : public TObject
// {
//   int8_t status = 0;
//   inline constexpr static int calibrated = 0b00001;
//   inline constexpr static int timeshift = 0b00010;
//   inline constexpr static int corrected = 0b00100;
//   inline constexpr static int finished = 0b01000;

// public:
//   RootHeader() = default;
//   virtual ~RootHeader() = default;
//   ClassDef(RootHeader, 1);

//   bool isCalibrated () const {return status & calibrated;}
//   bool isTimeshifted() const {return status & timeshift ;}
//   bool isFinished   () const {return status & finished  ;}
//   void setCalibrated () {status |= calibrated;}
//   void setTimeshifted() {status |= timeshift ;}
//   void setFinished   () {status |= finished  ;}
  
// };
