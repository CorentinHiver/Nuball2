#ifndef HIT_HPP
#define HIT_HPP

#include "../libRoot.hpp"

using Label  = ushort;
using Label_vec = std::vector<Label>;
using ADC    = int;
using ADC_vec = std::vector<ADC>;
using NRJ = float;
using Energy_vec = std::vector<NRJ>;
using Time   = ULong64_t;
using Time_vec = std::vector<Time>;
using Pileup = bool;
using Pileup_vec = std::vector<bool>;


/// @brief Casts a number into unsigned int
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Label Label_cast(T const & t) {return static_cast<Label>(t);}

/**
 * @brief Read options
 * @details
 * 
 * l : label   label               ushort
 * t : time    absolute timestamp  ULong64_t
 * T : time2   relative timestamp  float
 * e : nrj     energy in ADC       float
 * E : nrjcal  energy in keV       float
 * q : nrj2    energy QDC2 in ADC  float
 * Q : nrj2cal energy QDC2 in keV  float
 * p : pileup  pilepup             bool
 * 
 * R :         RF last timestamp   ULong64_t
 * P :         RF period           float
 * 
 */
struct Read
{
  bool l   = false; // label
  bool t   = false; // time
  bool T   = false; // relative Time
  bool e   = false; // ADC
  bool E   = false; // calibrated energy
  bool q   = false; // QDC2
  bool Q   = false; // calibrated QDC2
  bool p   = false; // pileup
  bool RFt = false; // RF downscale timestamp
  bool RFp = false; // RF period
  
  void setOptions(std::string const & options)
  {
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('l') : l   = true;  break;
        case ('t') : t   = true;  break;
        case ('T') : T   = true;  break;
        case ('e') : e   = true;  break;
        case ('E') : E   = true;  break;
        case ('q') : q   = true;  break;
        case ('Q') : Q   = true;  break;
        case ('p') : p   = true;  break;
        case ('R') : RFt = true;  break;
        case ('P') : RFp = true;  break;
        default : print("Unkown parameter", option, "for io event");
      }    
    }
  }
};

/**
 * @brief Write options
 * @details
 * 
 * l : label   label               ushort
 * t : time    absolute timestamp  ULong64_t
 * T : time2   relative timestamp  float
 * e : nrj     energy in ADC       float
 * E : nrjcal  energy in keV       float
 * q : nrj2    energy QDC2 in ADC  float
 * Q : nrj2cal energy QDC2 in keV  float
 * p : pileup  pilepup             bool
 * 
 * R :         RF last timestamp   ULong64_t
 * P :         RF period           float
 * 
 */
struct Write
{
  bool l   = false; // label
  bool t   = false; // time
  bool T   = false; // relative Time
  bool e   = false; // energy in ADC
  bool E   = false; // calibrated energy
  bool q   = false; // QDC2
  bool Q   = false; // calibrated QDC2
  bool p   = false; // pileup
  bool RFt = false; // time last RF downscale
  bool RFp = false; // RF period

  void setOptions(std::string const & options)
  {
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('l') : l   = true;  break;
        case ('t') : t   = true;  break;
        case ('T') : T   = true;  break;
        case ('e') : e   = true;  break;
        case ('E') : E   = true;  break;
        case ('q') : q   = true;  break;
        case ('Q') : Q   = true;  break;
        case ('p') : p   = true;  break;
        case ('R') : RFt = true;  break;
        case ('P') : RFp = true;  break;
        default : print("Unkown parameter", option, "for io event");
      }
    }
  }
};

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
  Hit(Label _label, Time _time, float _time2, float _nrj, float _nrj2 = 0, NRJ _nrjcal = 0, NRJ _nrj2cal = 0, bool _pileup = false) : 
    label  (_label),
    time   (_time),
    time2  (_time2),
    nrj    (_nrj),
    nrj2   (_nrj2),
    nrjcal (_nrjcal),
    nrj2cal(_nrj2cal),
    pileup (_pileup)
    {}

  Label label  = 0;     // Label
  Time  time   = 0ull;  // Timestamp ('ull' stands for unsigned long long)
  float time2  = 0.f;   // Relative time
  float nrj    = 0.f;   // Energy
  float nrj2   = 0.f;   // used if QDC2
  NRJ   nrjcal = 0.f;   // Calibrated energy
  NRJ   nrj2cal= 0.f;   // Calibrated QDC2
  bool  pileup = false; // Pile-up

  void reset()
  {
    label  = 0;
    time   = 0ull;
    nrj    = 0.f;
    nrj2   = 0.f;
    nrjcal = 0.f;
    nrj2cal= 0.f;
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

  read.setOptions(options);

  tree -> ResetBranchAddresses();
  
  if (read.l) tree -> SetBranchAddress("label"  , & label  );
  if (read.e) tree -> SetBranchAddress("nrj"    , & nrj    );
  if (read.E) tree -> SetBranchAddress("nrjcal" , & nrjcal );
  if (read.q) tree -> SetBranchAddress("nrj2"   , & nrj2   );
  if (read.Q) tree -> SetBranchAddress("nrj2cal", & nrj2cal);
  if (read.t) tree -> SetBranchAddress("time"   , & time   );
  if (read.T) tree -> SetBranchAddress("time2"  , & time2  );
  if (read.p) tree -> SetBranchAddress("pileup" , & pileup );
}

void Hit::writting(TTree * tree, std::string const & options)
{
  if (!tree) {print("Input tree at address 0x00 !"); return;}

  write.setOptions(options);

  tree -> ResetBranchAddresses();

  if (write.l) tree -> Branch("label"  , & label  );
  if (write.e) tree -> Branch("nrj"    , & nrj    );
  if (write.E) tree -> Branch("nrjcal" , & nrjcal );
  if (write.q) tree -> Branch("nrj2"   , & nrj2   );
  if (write.Q) tree -> Branch("nrj2cal", & nrj2cal);
  if (write.t) tree -> Branch("time"   , & time   );
  if (write.T) tree -> Branch("time2"  , & time2  );
  if (write.p) tree -> Branch("pileup" , & pileup );
}

std::ostream& operator<<(std::ostream& cout, Hit const & hit)
{
  cout << "l : " << hit.label;
  if (hit.nrj    >0) cout << " nrj :  "    << hit.nrj    ;
  if (hit.nrj2   >0) cout << " nrj2 : "    << hit.nrj2   ;
  if (hit.nrjcal >0) cout << " nrjcal : "  << hit.nrjcal ;
  if (hit.nrj2cal>0) cout << " nrj2cal : " << hit.nrj2cal;
  cout << " time : " << hit.time;
  cout << " rel time : " << hit.time2;
  if (hit.pileup) cout << " pileup";
  return cout;
}


#endif //HIT_HPP
