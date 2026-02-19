#ifndef HIT_HPP
#define HIT_HPP

#include "TTree.h"
// #include "../libCo.hpp"
#include "../Nuball2.hh"

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
struct IOptions
{
  IOptions() noexcept { reset(); }
  IOptions(std::string const & options) { setOptions(options); }

  void reset() noexcept
  {
    unset(m) ;
    unset(l) ;
    unset(t) ;
    unset(T) ;
    unset(e) ;
    unset(E) ;
    unset(q) ;
    unset(Q) ;
    unset(q3);
    unset(Q3);
    unset(p) ;
  }

  void setOptions(std::string const & options)
  {
    reset();
    for (char option : options)
    {
      switch (option)
      {
        case 'm': set(m) ; break;
        case 'l': set(l) ; break;
        case 't': set(t) ; break;
        case 'T': set(T) ; break;
        case 'e': set(e) ; break;
        case 'E': set(E) ; break;
        case 'q': set(q) ; break;
        case 'Q': set(Q) ; break;
        case '3': set(q3); break;
        case 'R': set(Q3); break;
        case 'p': set(p) ; break;
        default : 
          throw std::invalid_argument("Unknown parameter '" + std::string(1, option) + "' for io data");
      }
    }
    set(s);
  }

  std::string getOptions() const noexcept 
  {
    std::string out;

    if (is_set())
    {
      if (test(m) ) out.push_back('m');
      if (test(l) ) out.push_back('l');
      if (test(t) ) out.push_back('t');
      if (test(T) ) out.push_back('T');
      if (test(e) ) out.push_back('e');
      if (test(E) ) out.push_back('E');
      if (test(q) ) out.push_back('q');
      if (test(Q) ) out.push_back('Q');
      if (test(q3)) out.push_back('3');
      if (test(Q3)) out.push_back('R');
      if (test(p) ) out.push_back('p');
    }

    return out;
  }

  void detectLeafs(TTree * tree)
  {
    if (!tree) return; // Safety check

    reset();

    TObjArray* branches = tree->GetListOfBranches();
    if (!branches) return;

    for (int i = 0; i < branches->GetEntries(); ++i) 
    {
      auto branch = dynamic_cast<TBranch*>(branches->At(i));
      if (!branch) continue;

      std::string branchNameStr(branch->GetName());

      if (branchNameStr == "mult"  ) set (m );
      if (branchNameStr == "label" ) set (l );
      if (branchNameStr == "stamp" ) set (t );
      if (branchNameStr == "time"  ) set (T );
      if (branchNameStr == "adc"   ) set (e );
      if (branchNameStr == "nrj"   ) set (E );
      if (branchNameStr == "qdc2"  ) set (q );
      if (branchNameStr == "nrj2"  ) set (Q );
      if (branchNameStr == "qdc3"  ) set (q3);
      if (branchNameStr == "nrj3"  ) set (Q3);
      if (branchNameStr == "pileup") set (p );
    }
    set(s);
    set(r);
  }

  friend std::ostream& operator<<(std::ostream& out, IOptions const & options)
  {
    if (options.is_set())  
    {
      if(options.test (options.r )) out << "read ";
      if(options.test (options.w )) out << "write ";
      if(options.test (options.m )) out << "m ";
      if(options.test (options.l )) out << "l ";
      if(options.test (options.t )) out << "t ";
      if(options.test (options.T )) out << "T ";
      if(options.test (options.e )) out << "e ";
      if(options.test (options.E )) out << "E ";
      if(options.test (options.q )) out << "q ";
      if(options.test (options.Q )) out << "Q ";
      if(options.test (options.q3)) out << "q3 ";
      if(options.test (options.Q3)) out << "Q3 ";
      if(options.test (options.p )) out << "p ";
      out << std::endl;
    }
    else 
    {
      out << "IOption not set" << std::endl;
    }
    return out;
  }

  bool test (uchar const bit) const {return m_state.test(bit);}
  bool is_set() const {return test(m_state.test(s));}

  enum fields
  {
    m, // multiplicity (events)
    l, // label
    t, // timestamp in ps
    T, // relative time in ps
    e, // energy in ADC
    E, // calibrated energy in keV
    q, // qdc2 in ADC
    Q, // calibrated qdc2 in keV
    q3, // qdc3 in ADC
    Q3, // calibrated qdc3 in keV
    p, // pileup
    s, // is the option set or not
    r, // readOpt mode 
    w // write mode
  };
  
private:
  void set  (uchar const bit) {m_state.set  (bit);}
  void unset(uchar const bit) {m_state.reset(bit);}

  std::bitset<14> m_state;
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
 * Connect it to a FasterReader to readOpt data :
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
 *    2. To readOpt this raw root tree :
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

  Label     label  = 0;     // Label (identification number)
  Timestamp stamp  = 0ull;  // Timestamp ('ull' stands for unsigned long long)
  Time      time   = 0ll;   // Relative time ('ull' stands for long long)
  ADC       adc    = 0;     // Energy in ADC or QDC1
  NRJ       nrj    = 0.f;   // Calibrated energy in keV
  ADC       qdc2   = 0;     // Energy in qdc2
  NRJ       nrj2   = 0.f;   // Calibrated energy in qdc2 in keV
  ADC       qdc3   = 0;     // Energy in qdc3
  NRJ       nrj3   = 0.f;   // Calibrated energy in qdc3 in keV
  bool      pileup = false; // Pile-up (and saturation in QDC) tag

  inline bool operator<(Hit const & other) const noexcept {return stamp < other.stamp;}
  inline bool operator>(Hit const & other) const noexcept {return stamp > other.stamp;}

  auto constexpr getTimestamp_s() const {return stamp/double(1_s);}

private:
};

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

class RootHit : public Hit
{
#ifdef MULTITHREAD
  inline static std::mutex mutex_hits;
#endif //MULTITHREAD

public:
  template<class... ARGS>
  RootHit(ARGS &&... args) : Hit(std::forward<ARGS>(args)...) {}

  void reading(TTree * tree, std::string const & options)
  {
  #ifdef MULTITHREAD
    lock_mutex lock(mutex_hits);
  #endif //MULTITHREAD

    if (!tree) {print("Input tree at address 0x00 !"); return;}

    this -> clear();

    if (options == "") readOpt.detectLeafs(tree);
    else readOpt.setOptions(options);

    tree -> ResetBranchAddresses();

    if (readOpt.test(readOpt.l) ) tree -> SetBranchAddress("label"  , & label  );
    if (readOpt.test(readOpt.t) ) tree -> SetBranchAddress("stamp"  , & stamp  );
    if (readOpt.test(readOpt.T) ) tree -> SetBranchAddress("time"   , & time   );
    if (readOpt.test(readOpt.e) ) tree -> SetBranchAddress("adc"    , & adc    );
    if (readOpt.test(readOpt.E) ) tree -> SetBranchAddress("nrj"    , & nrj    );
    if (readOpt.test(readOpt.q) ) tree -> SetBranchAddress("qdc2"   , & qdc2   );
    if (readOpt.test(readOpt.Q) ) tree -> SetBranchAddress("nrj2"   , & nrj2   );
    if (readOpt.test(readOpt.q3)) tree -> SetBranchAddress("qdc3"   , & qdc3   );
    if (readOpt.test(readOpt.Q3)) tree -> SetBranchAddress("nrj3"   , & nrj3   );
    if (readOpt.test(readOpt.p) ) tree -> SetBranchAddress("pileup" , & pileup );
  }

  void writing(TTree * tree, std::string const & options = "lteqp")
  {
  #ifdef MULTITHREAD
    lock_mutex lock(mutex_hits);
  #endif //MULTITHREAD

    if (!tree) {print("Input tree at address 0x00 !"); return;}

    writeOpt.setOptions(options);

    tree -> ResetBranchAddresses();

    if (writeOpt.test(writeOpt.l )) tree -> Branch("label"  , & label  );
    if (writeOpt.test(writeOpt.t )) tree -> Branch("stamp"  , & stamp  );
    if (writeOpt.test(writeOpt.T )) tree -> Branch("time"   , & time   );
    if (writeOpt.test(writeOpt.e )) tree -> Branch("adc"    , & adc    );
    if (writeOpt.test(writeOpt.E )) tree -> Branch("nrj"    , & nrj    );
    if (writeOpt.test(writeOpt.q )) tree -> Branch("qdc2"   , & qdc2   );
    if (writeOpt.test(writeOpt.Q )) tree -> Branch("nrj2"   , & nrj2   );
    if (writeOpt.test(writeOpt.q3)) tree -> Branch("qdc3"   , & qdc3   );
    if (writeOpt.test(writeOpt.Q3)) tree -> Branch("nrj3"   , & nrj3   );
    if (writeOpt.test(writeOpt.p )) tree -> Branch("pileup" , & pileup );
  }
private:
  IOptions readOpt;
  IOptions writeOpt;
};


using HitTrigger = std::function<bool(const Hit&)>;


#endif //HIT_HPP
