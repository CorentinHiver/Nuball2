#ifndef HIT_HPP
#define HIT_HPP

#include "../Nuball2.hh"

/////////////////
/// Hit class ///
/////////////////

/**
 * @brief DOCUMENTATION NOT UP TO DATE !!! This class is used to store conveniently the data from reading the faster data. You can either treat data directly or write it in root trees
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
  Hit() noexcept = default;

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
      bool      _pileup ) noexcept :
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
      bool      _pileup ) noexcept :
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
      bool      _pileup ) noexcept :
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
      bool      _pileup ) noexcept :
    label  (_label ),
    stamp  (_stamp ),
    time   (_time  ),
    adc    (_adc   ),
    qdc2   (_qdc2  ),
    pileup (_pileup)
    {}

  Hit(Hit const & hit) noexcept = default;
  Hit& operator=(Hit const & hit) noexcept = default;
  Hit& operator=(Hit && hit) noexcept = default;

  inline constexpr void clear() noexcept
  {
    label  = {};
    stamp  = {};
    time   = {};
    adc    = {};
    nrj    = {};
    qdc2   = {};
    nrj2   = {};
    qdc3   = {};
    nrj3   = {};
    pileup = {};
  }

  Label     label  = {}; // Label (identification number)
  Timestamp stamp  = {}; // Timestamp ('ull' stands for unsigned long long)
  Time      time   = {}; // Relative time ('ull' stands for long long)
  ADC       adc    = {}; // Energy in ADC or QDC1
  NRJ       nrj    = {}; // Calibrated energy in keV
  ADC       qdc2   = {}; // Energy in qdc2
  NRJ       nrj2   = {}; // Calibrated energy in qdc2 in keV
  ADC       qdc3   = {}; // Energy in qdc3
  NRJ       nrj3   = {}; // Calibrated energy in qdc3 in keV
  bool      pileup = {}; // Pile-up or saturation tag

  constexpr inline bool operator<(Hit const & other) const noexcept {return stamp < other.stamp;}
  constexpr inline bool operator>(Hit const & other) const noexcept {return stamp > other.stamp;}

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

using HitTrigger = std::function<bool(const Hit&)>;


#endif //HIT_HPP
