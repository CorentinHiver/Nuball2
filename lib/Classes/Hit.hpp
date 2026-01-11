#ifndef HIT_HPP
#define HIT_HPP

#include "TTree.h"
#include "../libCo.hpp"
#include "Nuball2.hh"

/////////////////////
/// IO parameters ///
/////////////////////

/**
 * @brief 
 * ReadIO options. All branches are false by default and need to be activated, 
 * except mult that is true by default and needs to be deactivated
 * @details
 * 
 * legend 
 *    symbol : branch_name  full name  SI_unit  c++_type  default_value
 * 
 * m : mult   multiplicity (events)  N/A int       true
 * l : label  label                  N/A ushort    false
 * t : stamp  absolute timestamp     ps  ULong64_t false
 * T : time   relative time          ps  Long64_t  false
 * e : adc    energy                 ADC int       false
 * E : nrj    energy                 keV float     false
 * q : qdc2   energy qdc2            ADC int       false
 * Q : nrj2   energy qdc2            keV float     false
 * 3 : qdc3   energy qdc3            ADC int       false
 * R : nrj3   energy qdc3            keV float     false
 * p : pileup pileup                 N/A bool      false
 */
class IOptions
{
public:
  IOptions() noexcept {}
  IOptions(std::string const & options) noexcept {setOptions(options);}
  void reset()
  {
                // symbol : branch_name  full name  SI_unit  c++_type  default_value
    m  = true ; // m : mult   multiplicity (events)  N/A int       true
    l  = false; // l : label  label                  N/A ushort    false
    t  = false; // t : stamp  absolute timestamp     ps  ULong64_t false
    T  = false; // T : time   relative time          ps  Long64_t  false
    e  = false; // e : adc    energy                 ADC int       false
    E  = false; // E : nrj    energy                 keV float     false
    q  = false; // q : qdc2   energy qdc2            ADC int       false
    Q  = false; // Q : nrj2   energy qdc2            keV float     false
    q3 = false; // 3 : qdc3   energy qdc3            ADC int       false
    Q3 = false; // R : nrj3   energy qdc3            keV float     false
    p  = false; // p : pileup pileup                 N/A bool      false
    set = false;// internal state
  }

  void setOptions(std::string const & options) noexcept
  {
    reset();
    for (auto const & option : options)
    {
      switch (option)
      {
        case ('m') : m  = false; break;
        case ('l') : l  = true ; break;
        case ('t') : t  = true ; break;
        case ('T') : T  = true ; break;
        case ('e') : e  = true ; break;
        case ('E') : E  = true ; break;
        case ('q') : q  = true ; break;
        case ('Q') : Q  = true ; break;
        case ('3') : q3 = true ; break;
        case ('R') : Q3 = true ; break;
        case ('p') : p  = true ; break;
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

      if (branchNameStr == "mult"  ) m  = false;
      if (branchNameStr == "label" ) l  = true ;
      if (branchNameStr == "stamp" ) t  = true ;
      if (branchNameStr == "time"  ) T  = true ;
      if (branchNameStr == "adc"   ) e  = true ;
      if (branchNameStr == "nrj"   ) E  = true ;
      if (branchNameStr == "qdc2"  ) q  = true ;
      if (branchNameStr == "nrj2"  ) Q  = true ;
      if (branchNameStr == "qdc3"  ) q3 = true ;
      if (branchNameStr == "nrj3"  ) Q3 = true ;
      if (branchNameStr == "pileup") p  = true ;
    }
    set = true;
  }

  bool m  = true ; // multiplicity (events)
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

  Hit(
      Label     _label  ,  
      Timestamp _stamp  ,  
      Time      _time   ,   
      ADC       _adc    ,    
      NRJ       _nrj    ,    
      ADC       _qdc2   ,   
      NRJ       _nrj2   ,   
      ADC       _qdc3   ,   
      NRJ       _nrj3   ,   
      bool      _pileup ) :
    label  (_label ),
    stamp  (_stamp ),
    time   (_time  ),
    adc    (_adc   ),
    nrj    (_nrj   ),
    qdc2   (_qdc2  ),
    nrj2   (_nrj2  ),
    qdc3   (_qdc3  ),
    nrj3   (_nrj3  ),
    pileup (_pileup)
    {}

  Hit(
      Label     _label  ,  
      Timestamp _stamp  ,  
      Time      _time   ,   
      ADC       _adc    ,    
      NRJ       _nrj    ,    
      ADC       _qdc2   ,   
      NRJ       _nrj2   ,   
      bool      _pileup ) :
    label  (_label ),
    stamp  (_stamp ),
    time   (_time  ),
    adc    (_adc   ),
    nrj    (_nrj   ),
    qdc2   (_qdc2  ),
    nrj2   (_nrj2  ),
    pileup (_pileup)
    {}
  
    Hit(
      Label     _label  ,  
      Timestamp _stamp  ,  
      Time      _time   ,   
      NRJ       _nrj    ,    
      NRJ       _nrj2   ,   
      bool      _pileup ) :
    label  (_label ),
    stamp  (_stamp ),
    time   (_time  ),
    nrj    (_nrj   ),
    nrj2   (_nrj2  ),
    pileup (_pileup)
    {}
    
  Hit(
      Label     _label  ,  
      Timestamp _stamp  ,  
      Time      _time   ,   
      ADC       _adc    ,    
      ADC       _qdc2   ,   
      bool      _pileup ) :
    label  (_label ),
    stamp  (_stamp ),
    time   (_time  ),
    adc    (_adc   ),
    qdc2   (_qdc2  ),
    pileup (_pileup)
    {}

  Hit(Hit const & hit) :
    label  (hit.label ),
    stamp  (hit.stamp ),
    time   (hit.time  ),
    adc    (hit.adc   ),
    nrj    (hit.nrj   ),
    qdc2   (hit.qdc2  ),
    nrj2   (hit.nrj2  ),
    qdc3   (hit.qdc3  ),
    nrj3   (hit.nrj3  ),
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
  Time      time   = 0ll;   // Relative time ('ull' stands for long long)
  ADC       adc    = 0;     // Energy in ADC or QDC1
  NRJ       nrj    = 0.f;   // Calibrated energy in keV
  ADC       qdc2   = 0;     // Energy in qdc2
  NRJ       nrj2   = 0.f;   // Calibrated energy in qdc2 in keV
  ADC       qdc3   = 0;     // Energy in qdc3
  NRJ       nrj3   = 0.f;   // Calibrated energy in qdc3 in keV
  bool      pileup = false; // Pile-up (and saturation in QDC) tag

  void reading(TTree * tree, std::string const & options = "");
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
  if (read.T ) tree -> SetBranchAddress("time"   , & time   );
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


using HitTrigger = std::function<bool(const Hit&)>;


#endif //HIT_HPP
