#ifndef HIT_HPP
#define HIT_HPP

#include "TTree.h"
#include "../libCo.hpp"

#ifdef COMULTITHREADING
  std::mutex mutex_hits;
#endif //COMULTITHREADING

//////////////////
/// Data types ///
//////////////////

using Label     = ushort;    // Label (ushort)
using ADC       = int;       // ADC (int)
using NRJ       = float;     // Energy in keV (float)
using Timestamp = ULong64_t; // Timestamp in ps (absolute)
using Time      = Long64_t;  // Time in ps (relative)
using Time_ns   = float;     // Time in ns (relative) !deprecated! 
using Pileup    = bool;      // Pileup bit (bool) !unused!
using Index     = uchar;     // Used in analysis structures (Clovers, Paris...). Be careful to the max value 255

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

//////////////////////
// Unit conversions //
//////////////////////

// Units of time (= relative time). The code is based on ps.
constexpr inline Time operator""_s (long double time) noexcept {return Time_cast(time*1.e+12);}
constexpr inline Time operator""_ms(long double time) noexcept {return Time_cast(time*1.e+9 );}
constexpr inline Time operator""_us(long double time) noexcept {return Time_cast(time*1.e+6 );}
constexpr inline Time operator""_ns(long double time) noexcept {return Time_cast(time*1.e+3 );}
constexpr inline Time operator""_ps(long double time) noexcept {return Time_cast(time       );}
constexpr inline Time operator""_fs(long double time) noexcept {return Time_cast(time*1.e-3 );}

constexpr inline Time operator""_s (unsigned long long time) noexcept {return Time_cast(time*1.e+12l);}
constexpr inline Time operator""_ms(unsigned long long time) noexcept {return Time_cast(time*1.e+9l );}
constexpr inline Time operator""_us(unsigned long long time) noexcept {return Time_cast(time*1.e+6l );}
constexpr inline Time operator""_ns(unsigned long long time) noexcept {return Time_cast(time*1.e+3l );}
constexpr inline Time operator""_ps(unsigned long long time) noexcept {return Time_cast(time        );}
constexpr inline Time operator""_fs(unsigned long long time) noexcept {return Time_cast(time*1.e-3l );}

// Units of particle energy. The code is based on keV.
constexpr inline double operator""_MeV(long double energy) noexcept {return double_cast(energy)*1.e+3;}
constexpr inline double operator""_keV(long double energy) noexcept {return double_cast(energy);}

constexpr inline double operator""_MeV(unsigned long long energy) noexcept {return double_cast(energy*1.e+3l);}
constexpr inline double operator""_keV(unsigned long long energy) noexcept {return double_cast(energy);}

// General units of numbers :
constexpr inline double operator""_G(long double number) noexcept {return double_cast(number*1.e+12);}
constexpr inline double operator""_M(long double number) noexcept {return double_cast(number*1.e+6 );}
constexpr inline double operator""_k(long double number) noexcept {return double_cast(number*1.e+3 );}

constexpr inline double operator""_G(unsigned long long number) noexcept {return double_cast(number*1.e+12l);}
constexpr inline double operator""_M(unsigned long long number) noexcept {return double_cast(number*1.e+6l );}
constexpr inline double operator""_k(unsigned long long number) noexcept {return double_cast(number*1.e+3l );}

constexpr inline int operator""_Gi(unsigned long long number) noexcept {return int_cast(number*1.e+12l);}
constexpr inline int operator""_Mi(unsigned long long number) noexcept {return int_cast(number*1.e+6l );}
constexpr inline int operator""_ki(unsigned long long number) noexcept {return int_cast(number*1.e+3l );}

// Angles : 
constexpr double to_rad(double const & deg){return deg*3.14159/180.;}
constexpr double to_deg(double const & rad){return rad/3.14159*180.;}
constexpr double to_rad(long double const & deg){return deg*3.14159/180.;}
constexpr double to_deg(long double const & rad){return rad/3.14159*180.;}
constexpr inline double operator""_deg(long double number) noexcept {return to_rad(number);}

/////////////////////
/// IO parameters ///
/////////////////////

/**
 * @brief ReadIO options
 * @details
 * 
 * m : mult   multiplicity (events) int
 * l : label  label                 ushort
 * t : stamp  absolute timestamp ps ULong64_t
 * T : time   relative time      ps Long64_t
 * e : adc    energy in ADC         int
 * E : nrj    energy in keV         float
 * q : qdc2   energy qdc2 in ADC    int
 * Q : nrj2   energy qdc2 in keV    float
 * 3 : qdc3   energy qdc3 in ADC    int
 * R : nrj3   energy qdc3 in keV    float
 * p : pileup pileup                bool
 */
class IOptions
{
public:
  IOptions() noexcept {}
  IOptions(std::string const & options) noexcept {setOptions(options);}
  void reset()
  {
    m  = false; // m : mult   multiplicity (events) int
    l  = false; // l : label  label                 ushort
    t  = false; // t : stamp  absolute timestamp ps ULong64_t
    T  = false; // T : time   relative time      ps Long64_t
    e  = false; // e : adc    energy in ADC         int
    E  = false; // E : nrj    energy in keV         float
    q  = false; // q : qdc2   energy qdc2 in ADC    int
    Q  = false; // Q : nrj2   energy qdc2 in keV    float
    q3 = false; // 3 : qdc3   energy qdc3 in ADC    int
    Q3 = false; // R : nrj3   energy qdc3 in keV    float
    p  = false; // p : pileup pileup                bool
    set = false;// internal state
  }

  void setOptions(std::string const & options) noexcept
  {
    reset();
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('m') : m  = true; break;
        case ('l') : l  = true; break;
        case ('t') : t  = true; break;
        case ('T') : T  = true; break;
        case ('e') : e  = true; break;
        case ('E') : E  = true; break;
        case ('q') : q  = true; break;
        case ('Q') : Q  = true; break;
        case ('3') : q3 = true; break;
        case ('R') : Q3 = true; break;
        case ('p') : p  = true; break;
        default : error("Unkown parameter '", option, "' for io data");
      }
    }
    set = true;
  }

  std::string getOptions() const noexcept 
  {
    std::string out;

    if (m) out.push_back('m');
    if (l) out.push_back('l');
    if (t) out.push_back('t');
    if (T) out.push_back('T');
    if (e) out.push_back('e');
    if (E) out.push_back('E');
    if (q) out.push_back('q');
    if (Q) out.push_back('Q');
    if (q3) out.push_back('3');
    if (Q3) out.push_back('R');
    if (p) out.push_back('p');

    return out;
  }

  void detectLeafs(TTree * tree)
  {
    reset();
    TObjArray* branches = tree->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); ++i) 
    {
      auto branch = dynamic_cast<TBranch*>(branches->At(i));
      std::string branchNameStr(branch->GetName());

      if (branchNameStr == "mult"  ) m  = true;
      if (branchNameStr == "label" ) l  = true;
      if (branchNameStr == "stamp" ) t  = true;
      if (branchNameStr == "time"  ) T  = true;
      if (branchNameStr == "adc"   ) e  = true;
      if (branchNameStr == "nrj"   ) E  = true;
      if (branchNameStr == "qdc2"  ) q  = true;
      if (branchNameStr == "nrj2"  ) Q  = true;
      if (branchNameStr == "qdc3"  ) q3 = true;
      if (branchNameStr == "nrj3"  ) Q3 = true;
      if (branchNameStr == "pileup") p  = true;
    }
    set = true;
  }

  bool m  = false; // multiplicity (events)
  bool l  = false; // label
  bool t  = false; // timestamp in ps
  bool T  = false; // relative time in ps
  bool e  = false; // energy in ADC
  bool E  = false; // calibrated energy in keV
  bool q  = false; // qdc2 in ADC
  bool Q  = false; // calibrated qdc2 in keV
  bool q3 = false; // qdc3 in ADC
  bool Q3 = false; // calibrated qdc3 in keV
  bool p  = false; // pileup

  bool set = false;
};

/////////////////
/// Hit class ///
/////////////////

/**
 * @brief This class is used to store conveniently the data from reading the faster data. You can either treat data directly or write it in root trees
 * @details
 * 
 * This class is used as an interface between the faster data and root.
 * 
 * Connect it to a FasterReader to read data :
 * 
 *      Hit hit;
 *      FasterReader.setHit(&hit);
 *      while(reader.Read())
 *      {
 *        doSomething with the hit...
 *      }
 * 
 * Connect it to a Root Tree : 
 * 
 *    1. To convert data to a raw root tree :
 * 
 *      Hit hit;
 *      TTree * tree = new TTree("Nuball2","Nuball2");
 *      FasterReader.setHit(&hit);
 *      hit.writing(tree);
 *      while(reader.Read())
 *      {
 *        tree -> Fill();
 *      }
 * 
 *    2. To read this raw root tree :
 *      
 *      hit.reading(tree);
 *      for (int hit = 0; hit<tree->GetEntries(); hit++)
 *      {
 *        do something with the hit ...
 *      }
 * 
 * Nomenclature : 
 * The ADC are in INT because they represent a number of digitization channels
 * The energies are in float because we do not need more precision than the detectors resolution, of the order of the keV for the best ones
 */
class Hit
{
public:
  Hit(){clear();}

  Hit(Hit const & hit) :
    label  (hit.label),
    stamp  (hit.stamp),
    time   (hit.time),
    adc    (hit.adc),
    nrj    (hit.nrj),
    qdc2   (hit.qdc2),
    nrj2   (hit.nrj2),
    qdc3   (hit.qdc3),
    nrj3   (hit.nrj3),
    pileup (hit.pileup)
    {}

  Hit& operator=(Hit const & hit)
  {
    label  = hit.label;
    stamp  = hit.stamp;
    time   = hit.time;
    adc    = hit.adc;
    nrj    = hit.nrj;
    qdc2   = hit.qdc2;
    nrj2   = hit.nrj2,
    qdc3   = hit.qdc3;
    nrj3   = hit.nrj3,
    pileup = hit.pileup;
    return *this;
  }

  void clear()
  {
    label  = 0;
    stamp  = 0ull;
    time   = 0ll;
    adc    = 0;
    nrj    = 0.f;
    qdc2   = 0;
    nrj2   = 0.f;
    qdc3   = 0;
    nrj3   = 0.f;
    pileup = false;
  }

  Label     label  = 0;     // Label
  Timestamp stamp  = 0ull;  // Timestamp ('ull' stands for unsigned long long)
  Time      time   = 0ll;   // Relative time
  ADC       adc    = 0;     // Energy in ADC or QDC1
  NRJ       nrj    = 0.f;   // Calibrated energy in keV
  ADC       qdc2   = 0;     // Energy in qdc2
  NRJ       nrj2   = 0.f;   // Calibrated energy in qdc2 in keV
  ADC       qdc3   = 0;     // Energy in qdc3
  NRJ       nrj3   = 0.f;   // Calibrated energy in qdc3 in keV
  bool      pileup = false; // Pile-up (and saturation in QDC) tag

  void reading (TTree * tree, std::string const & options = "");
  void writing(TTree * tree, std::string const & options = "lteqp");

  IOptions read;
  IOptions write;

private:
};

void Hit::reading(TTree * tree, std::string const & options)
{
#ifdef COMULTITHREADING
  lock_mutex lock(mutex_hits);
#endif //COMULTITHREADING

  if (!tree) {print("Input tree at address 0x00 !"); return;}

  this -> clear();

  if (options == "") read.detectLeafs(tree);
  else read.setOptions(options);

  tree -> ResetBranchAddresses();

  if (read.l ) tree -> SetBranchAddress("label"  , & label  );
  if (read.t ) tree -> SetBranchAddress("stamp"  , & stamp  );
  if (read.T ) tree -> SetBranchAddress("time"   , & time  );
  if (read.e ) tree -> SetBranchAddress("adc"    , & adc    );
  if (read.E ) tree -> SetBranchAddress("nrj"    , & nrj    );
  if (read.q ) tree -> SetBranchAddress("qdc2"   , & qdc2   );
  if (read.Q ) tree -> SetBranchAddress("nrj2"   , & nrj2   );
  if (read.q3) tree -> SetBranchAddress("qdc3"   , & qdc3   );
  if (read.Q3) tree -> SetBranchAddress("nrj3"   , & nrj3   );
  if (read.p ) tree -> SetBranchAddress("pileup" , & pileup );
}

void Hit::writing(TTree * tree, std::string const & options)
{
#ifdef COMULTITHREADING
  lock_mutex lock(mutex_hits);
#endif //COMULTITHREADING

  if (!tree) {print("Input tree at address 0x00 !"); return;}

  write.setOptions(options);

  tree -> ResetBranchAddresses();

  if (write.l ) tree -> Branch("label"  , & label  );
  if (write.t ) tree -> Branch("stamp"  , & stamp  );
  if (write.T ) tree -> Branch("time"   , & time   );
  if (write.e ) tree -> Branch("adc"    , & adc    );
  if (write.E ) tree -> Branch("nrj"    , & nrj    );
  if (write.q ) tree -> Branch("qdc2"   , & qdc2   );
  if (write.Q ) tree -> Branch("nrj2"   , & nrj2   );
  if (write.q3) tree -> Branch("qdc3"   , & qdc3   );
  if (write.Q3) tree -> Branch("nrj3"   , & nrj3   );
  if (write.p ) tree -> Branch("pileup" , & pileup );
}

std::ostream& operator<<(std::ostream& cout, Hit const & hit)
{
  cout << "l : " << std::setw(3) << hit.label;
  if (hit.stamp != 0) cout << std::setprecision(7) << " timestamp : " << double(hit.stamp*1e-12) << " s ";
  if (hit.time  != 0) cout << " rel time : "  << hit.time;
  if (hit.adc   != 0) cout << " adc : " << std::setw(8) << hit.adc;
  if (hit.qdc2  != 0) cout << " qdc2 : "<< std::setw(8) << hit.qdc2;
  if (hit.qdc3  != 0) cout << " qdc3 : "<< std::setw(8) << hit.qdc3;
  if (hit.nrj   != 0) cout << " nrj : "       << hit.nrj ;
  if (hit.nrj2  != 0) cout << " nrj2 : "      << hit.nrj2;
  if (hit.nrj3  != 0) cout << " nrj3 : "      << hit.nrj3;
  if (hit.pileup)     cout << "\u001b[31m pileup \u001b[0m";
  return cout;
}


//////////////////////////
/// Trigger definition ///
//////////////////////////

using TriggerHit = std::function<bool(const Hit&)>;


#endif //HIT_HPP
