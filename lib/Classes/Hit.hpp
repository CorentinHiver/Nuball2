#ifndef HIT_HPP
#define HIT_HPP

#include "../libRoot.hpp"


//////////////////
/// Data types ///
//////////////////

using Mult      = int;
using Label     = ushort;
using ADC       = int;
using NRJ       = float;
using Timestamp = ULong64_t;
using Time      = Long64_t;
using Time_ns   = float;
using Pileup    = bool;


////////////////////
/// Data vectors ///
////////////////////

using Label_vec   = std::vector<Label  >;
using ADC_vec     = std::vector<ADC    >;
using Energy_vec  = std::vector<NRJ    >;
using Time_vec    = std::vector<Time   >;
using Time_ns_vec = std::vector<Time_ns>;
using Pileup_vec  = std::vector<Pileup >;


//////////////////
/// Data casts ///
//////////////////

/// @brief Casts a number into unsigned Label
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Label Label_cast(T const & t) {return static_cast<Label>(t);}

/// @brief Casts a number into unsigned ADC
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ADC ADC_cast(T const & t) {return static_cast<ADC>(t);}

/// @brief Casts a number into unsigned Time
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Time Time_cast(T const & t) {return static_cast<Time>(t);}

/// @brief Casts a number into unsigned NRJ
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline NRJ NRJ_cast(T const & t) {return static_cast<NRJ>(t);}

/// @brief Casts a number into unsigned Time_ns
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Time_ns Time_ns_cast(T const & t) {return static_cast<Time_ns>(t);}

/// @brief Casts a number into unsigned Timestamp
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Timestamp Timestamp_cast(T const & t) {return static_cast<Timestamp>(t);}

/////////////////////
/// IO parameters ///
/////////////////////

/**
 * @brief Read options
 * @details
 * 
 * l : label  label                 ushort
 * s : stamp  absolute timestamp ps ULong64_t
 * t : time   relative timestamp ps int
 * T : time2  relative timestamp ns float
 * e : adc    energy in ADC         int
 * E : nrj    energy in keV         float
 * q : qdc2   energy qdc2 in ADC    float
 * Q : nrj2   energy qdc2 in keV    float
 * p : pileup pilepup               bool
 * 
 */
struct Read
{
  bool l = false; // label
  bool s = false; // Timestamp
  bool t = false; // relative time ps
  bool T = false; // relative time ns
  bool e = false; // energy in ADC
  bool E = false; // calibrated energy
  bool q = false; // qdc2
  bool Q = false; // calibrated qdc2
  bool p = false; // pileup

  void setOptions(std::string const & options)
  {
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('l') : l   = true;  break;
        case ('s') : s   = true;  break;
        case ('t') : t   = true;  break;
        case ('T') : T   = true;  break;
        case ('e') : e   = true;  break;
        case ('E') : E   = true;  break;
        case ('q') : q   = true;  break;
        case ('Q') : Q   = true;  break;
        case ('p') : p   = true;  break;
        default : print("Unkown parameter", option, "for io event");
      }
    }
  }
};

/**
 * @brief Write options
 * @details
 * 
 * l : label  label                 ushort
 * s : stamp  absolute timestamp ps ULong64_t
 * t : time   relative timestamp ps int
 * T : time2  relative timestamp ns float
 * e : adc    energy in ADC         int
 * E : nrj    energy in keV         float
 * q : qdc2   energy qdc2 in ADC    float
 * Q : nrj2   energy qdc2 in keV    float
 * p : pileup pilepup               bool
 * 
 */
struct Write
{
  bool l = false; // label
  bool s = false; // Timestamp
  bool t = false; // relative time ps
  bool T = false; // relative time ns
  bool e = false; // energy in ADC
  bool E = false; // calibrated energy
  bool q = false; // qdc2
  bool Q = false; // calibrated qdc2
  bool p = false; // pileup

  void setOptions(std::string const & options)
  {
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('l') : l   = true;  break;
        case ('s') : s   = true;  break;
        case ('t') : t   = true;  break;
        case ('T') : T   = true;  break;
        case ('e') : e   = true;  break;
        case ('E') : E   = true;  break;
        case ('q') : q   = true;  break;
        case ('Q') : Q   = true;  break;
        case ('p') : p   = true;  break;
        default : print("Unkown parameter", option, "for io event");
      }
    }
  }
};

/////////////////
/// Hit class ///
/////////////////

/**
 * @brief This class is used to store conviniently the data from reading the faster data. You can either treat data directly or write it in root trees
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
 *      hit.writting(tree);
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
 *        do somethign with the hit ...
 *      }
 * 
 */
class Hit
{
public:
  Hit(){reset();}
  // Hit(Label _label, Timestamp _stamp, Time _time =0, ADC _adc = 0, ADC _qdc2 = 0, float _time2 = 0, NRJ _nrj = 0, NRJ _nrj2 = 0, bool _pileup = false) : 
  Hit(Label _label, Timestamp _stamp, ADC _adc = 0, ADC _qdc2 = 0, float _time2 = 0, NRJ _nrj = 0, NRJ _nrj2 = 0, bool _pileup = false) : 
    label  (_label),
    stamp  (_stamp),
    // time   (_time),
    time2  (_time2),
    adc    (_adc),
    qdc2   (_qdc2),
    nrj    (_nrj),
    nrj2   (_nrj2),
    pileup (_pileup)
    {}

  Hit(Hit const & hit) :
    label  (hit.label),
    stamp  (hit.stamp),
    // time   (hit.time),
    time2  (hit.time2),
    adc    (hit.adc),
    qdc2   (hit.qdc2),
    nrj    (hit.nrj),
    nrj2   (hit.nrj2),
    pileup (hit.pileup)
    {}

  Hit& operator=(Hit const & hit)
  {
    label  = hit.label;
    stamp  = hit.stamp;
    // time   = hit.time;
    time2  = hit.time2;
    adc    = hit.adc;
    qdc2   = hit.qdc2;
    nrj    = hit.nrj;
    nrj2   = hit.nrj2;
    pileup = hit.pileup;
    return *this;
  }

  Label     label  = 0;     // Label
  Timestamp stamp  = 0ull;  // Timestamp ('ull' stands for unsigned long long)
  // Time      time   = 0;     // Relative time in ps
  Time_ns   time2  = 0.f;   // Relative time in ns
  ADC       adc    = 0;     // Energy in ADC or QDC1
  ADC       qdc2   = 0;     // Energy in qdc2
  NRJ       nrj = 0.f;      // Calibrated energy in keV
  NRJ       nrj2= 0.f;      // Calibrated energy in qdc2 in keV
  bool      pileup = false; // Pile-up (and saturation in QDC) tag

  void reset()
  {
    label  = 0;
    stamp = 0ull;
    // time   = 0;
    time2  = 0.f;
    adc    = 0;
    qdc2   = 0;
    nrj = 0.f;
    nrj2= 0.f;
    pileup = false;
  }

  void reading (TTree * tree, std::string const & options);
  void writting(TTree * tree, std::string const & options);

  Read read;
  Write write;
};

void Hit::reading(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  this->reset();

  read.setOptions(options);

  tree -> ResetBranchAddresses();
  
  if (read.l) tree -> SetBranchAddress("label"  , & label  );
  if (read.s) tree -> SetBranchAddress("stamp"  , & stamp  );
  // if (read.t) tree -> SetBranchAddress("time"   , & time   );
  if (read.T) tree -> SetBranchAddress("time2"  , & time2  );
  if (read.e) tree -> SetBranchAddress("adc"    , & adc    );
  if (read.E) tree -> SetBranchAddress("nrj"    , & nrj    );
  if (read.q) tree -> SetBranchAddress("qdc2"   , & qdc2   );
  if (read.Q) tree -> SetBranchAddress("nrj2"   , & nrj2   );
  if (read.p) tree -> SetBranchAddress("pileup" , & pileup );
} 

void Hit::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  write.setOptions(options);

  tree -> ResetBranchAddresses();

  if (write.l) tree -> Branch("label"  , & label  );
  if (write.s) tree -> Branch("stamp"  , & stamp  );
  // if (write.t) tree -> Branch("time"   , & time   );
  if (write.T) tree -> Branch("time2"  , & time2  );
  if (write.e) tree -> Branch("adc"    , & adc    );
  if (write.E) tree -> Branch("nrj"    , & nrj    );
  if (write.q) tree -> Branch("qdc2"   , & qdc2   );
  if (write.Q) tree -> Branch("nrj2"   , & nrj2   );
  if (write.p) tree -> Branch("pileup" , & pileup );
}

std::ostream& operator<<(std::ostream& cout, Hit const & hit)
{
  cout << "l : " << hit.label;
  if (hit.stamp  != 0) cout << " timestamp : "    << hit.stamp;
  // if (hit.time  != 0) cout << " time : "    << hit.time;
  if (hit.time2 != 0) cout << " rel time : "<< hit.time2;
  if (hit.adc   != 0) cout << " adc :  "    << hit.adc    ;
  if (hit.qdc2  != 0) cout << " qdc2 : "    << hit.qdc2   ;
  if (hit.nrj   != 0) cout << " nrj : "     << hit.nrj ;
  if (hit.nrj2  != 0) cout << " nrj2 : "    << hit.nrj2;
  if (hit.pileup)     cout << " pileup";
  return cout;
}


#endif //HIT_HPP
