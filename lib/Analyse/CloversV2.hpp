#ifndef CLOVERSV2_HPP
#define CLOVERSV2_HPP

#include "../Classes/Event.hpp"
#include "CloverModule.hpp"

// using Index = unsigned short;
// inline bool isGe(Index const & label) {return (label < 200 && (label+1)%6 > 1);}
/**
 * @brief A class to simplify the analysis of the Clover data
 * @details Removes the handling of the crystal of maximal energy deposit.
 * @todo Create a "greylist" for HPGe to reject because of poor resolution, but still usable for calorimetry 
 * and Compton rejection of other HPGe of the same clover (i.e., they may have bad energy resolution, 
 * if their timing is good they should be considered "grey")
 */
class CloversV2
{
public:
  CloversV2() noexcept
  {
    CloverModule::resetGlobalLabel(); // This allows to correctly label the CloverModules in other instances of CloversV2  
  };
  ~CloversV2() {}
  
  CloversV2& operator=(CloversV2 const & other)
  {
    calorimetryTotal  = other.calorimetryTotal;
    calorimetryGe     = other.calorimetryGe;
    calorimetryBGO    = other.calorimetryBGO;
    m_analyzed        = other.m_analyzed;

    for(auto const & clover : Hits_id) m_clovers[clover].clear();
    Hits_id = other.Hits_id;
    for(auto const & clover : Hits_id) m_clovers[clover] = other.m_clovers[clover];

    Ge_id       = other.Ge_id;
    BGO_id      = other.BGO_id;
    GeClean_id  = other.GeClean_id;
    BGOClean_id = other.BGOClean_id;
    Rejected_id = other.Rejected_id;
    clean       = other.clean;
    cleanBGO    = other.cleanBGO;
    rejected    = other.rejected;
    all         = other.all;
    Ge          = other.Ge ;
    BGO         = other.BGO;

    return *this;
  }

  /// @brief The clover index (labels [23;29] -> 0, labels [190;196] -> 23)
  static constexpr auto index        = LUT<1000> ([](Label const & label) -> Index {return Index_cast((label-23)/6);});
  static constexpr auto subIndex     = LUT<1000> ([](Label const & label) -> Index {return Index_cast((label-23)%6);});
  static constexpr auto crystalIndex = subIndex; // Aliasing
  static constexpr auto is           = LUT<1000> ([](Label const & label) -> bool  {return 22 < label && label < 168;});
  static constexpr auto isGe         = LUT<1000> ([](Label const & label) -> bool  {return is[label] && subIndex[label] > 1;});
  static constexpr auto isBGO        = LUT<1000> ([](Label const & label) -> bool  {return is[label] && subIndex[label] < 2;});
  static constexpr auto isR2         = LUT<1000> ([](Label const & label) -> bool  {return is[label] && label > 94;});
  static constexpr auto isR3         = LUT<1000> ([](Label const & label) -> bool  {return is[label] && label < 95;});
  
  bool fill   (Event const & event, int const & hit_i);
  bool fillRaw(Event const & event, int const & hit_i);
  
  CloversV2& load      (Event const & event);
  CloversV2& loadRaw   (Event const & event);
  CloversV2& operator= (Event const & event);

  void analyze   () noexcept;
  void analyzeRaw() noexcept;

  void reset()
  {
    calorimetryTotal  = 0.0;
    calorimetryGe     = 0.0;
    calorimetryBGO    = 0.0;
    m_analyzed        = false;
    for(auto const & clover : Hits_id) m_clovers[clover].reset();
    Hits_id    .clear();
    Ge_id      .clear();
    BGO_id     .clear();
    GeClean_id .clear();
    BGOClean_id.clear();
    Rejected_id.clear();
    clean      .clear();
    cleanBGO   .clear();
    rejected   .clear();
    all        .clear();
    Ge         .clear();
    BGO        .clear();
  }

  void clear(){reset();}

  double calorimetryTotal = 0.0;
  double calorimetryGe    = 0.0;
  double calorimetryBGO   = 0.0;

  // Indices

  std::vector<Index> Hits_id    ;
  std::vector<Index> Ge_id      ;
  std::vector<Index> BGO_id     ;
  std::vector<Index> GeClean_id ;
  std::vector<Index> BGOClean_id;
  std::vector<Index> Rejected_id;

  // Views :

  std::vector<const CloverModule*> all     ;
  std::vector<const CloverModule*> Ge      ;
  std::vector<const CloverModule*> BGO     ;
  std::vector<const CloverModule*> clean   ;
  std::vector<const CloverModule*> cleanBGO;
  std::vector<const CloverModule*> rejected;

  // Getters :

  auto begin()       {return all.begin();}
  auto end  ()       {return all.end  ();}
  auto begin() const {return all.begin();}
  auto end  () const {return all.end  ();}

  inline constexpr auto const & operator[](int const & i) const noexcept { return m_clovers[i]; }

  // Static parameters :

  using Blacklist = std::unordered_set<Label>;
  using Overflow  = std::unordered_map<Label, double>;
  static inline Blacklist sBlacklist  = {};
  static inline Overflow sOverflow   = {{}};

  static void setEnergyThreshold   (double const & threshold            ) {sThreshold          = threshold         ;}
  static void setSmearGeCalorimetry(double const & smearGeCalorimetry   ) {sSmearGeCalorimetry = smearGeCalorimetry;}
  static void setSmearGeCalorimetryResolution(double const & resolution ) {sSmearedCaloRes     = resolution        ;}
  static void setBlacklist         (Blacklist const & blacklist         ) {sBlacklist          = blacklist         ;}
  static void setOverflow          (Overflow  const & overflow_Ge       ) {sOverflow           = overflow_Ge       ;}
  
  static void useBlacklist  (bool const & b = true) {sUseBlacklist = b;}
  static void rejectOverflow(bool const & b = true) {sUseOverflow  = b;}

  static bool rejectDetector(Label const & label, NRJ const & nrj)
  {
    bool ret = false;
    if (sUseBlacklist)
    {
      // User-selected blacklisted detectors labels :
      static auto blacklistedLabel = LUT<1000>([](Label const & _label) {
        return found(sBlacklist, _label);
      });

      ret = blacklistedLabel[label];
    } 

    if (sUseOverflow)
    {
      // User-selected overflow energy value :
      static auto hasOverflow = LUT<1000>([](Label const & _label){
        return found(sOverflow  , _label);
      });
       ret = hasOverflow[label] && nrj > sOverflow[label];
    }
    return ret;
  }

private:

  // Data
  std::array<CloverModule, 24> m_clovers;

  // Parameters :
  // Member :
  bool m_analyzed = false;
  // Static :
  static inline double sThreshold = 5_keV;
  static inline double sSmearedCaloRes = 400_keV;
  static inline bool sSmearGeCalorimetry = false;
  static inline bool sUseBlacklist = false;
  static inline bool sUseOverflow  = false;
};

/**
 * @brief 
 * @attention Extracts energy from the nrj field, i.e., calibrated energy in keV, hence requires calibration
 */
bool CloversV2::fill(Event const & event, int const & hit_i)
{
  if (m_analyzed) throw_error("CloversV2::fill() called while already analyzed, you need to CloversV2::reset() (or clear()) first");
  auto const & label = event.labels[hit_i];
  auto const & nrj   = event.nrjs  [hit_i];
  
  if (CloversV2::rejectDetector(label, nrj)) return false;
  
  if (CloversV2::is[label])
  {
    auto const & nrj          = event.nrjs         [hit_i];
    auto const & time         = event.times        [hit_i];
    auto const & clover_index = CloversV2::index   [label];
    auto const & sub_index    = CloversV2::subIndex[label];

    push_back_unique(Hits_id, clover_index);

    m_clovers[clover_index].addHit(nrj, time, sub_index);
    
    auto calo = nrj;
    if (isGe[label])
    {
      if (sSmearGeCalorimetry)
        calo = randomCo::gaussian(nrj, nrj*((sSmearedCaloRes/sqrt(nrj))/100_keV)/2.35);
      push_back_unique(Ge_id, clover_index);
      calorimetryGe += calo;
    }
    else
    {
      push_back_unique(BGO_id, clover_index);
      calorimetryBGO += calo;
    }

    calorimetryTotal += calo;
    
    return true;
  }
  return false;
}

/**
 * @brief In fillRaw, the adc value is read instead of nrj. 
 * @attention No calibration, therefore calorimetry and add-back values mean nothing and are not calculated
 * 
 */
bool CloversV2::fillRaw(Event const & event, int const & hit_i)
{
  if (m_analyzed) throw_error("CloversV2::fillRaw() called while already analyzed, you need to CloversV2::reset() (or clear()) first");
  auto const & label = event.labels[hit_i];
  auto const & nrj   = event.adcs  [hit_i];

  if (rejectDetector(label, nrj)) return false;

  if (CloversV2::is[label])
  {
    auto const & nrj          = event.adcs          [hit_i];
    auto const & time         = event.times         [hit_i];
    auto const & clover_index = CloversV2::index    [label];
    auto const & sub_index    = CloversV2::subIndex [label];

    push_back_unique(Hits_id, clover_index);

    m_clovers[clover_index].addHit(nrj, time, sub_index);

    return true;
  }
  return false;
}

CloversV2& CloversV2::load(Event const & event)
{
  this->clear();
  for (int hit_i = 0; hit_i<event.mult; hit_i++) this->fill(event, hit_i);
  this->analyze();
  return *this;
}

CloversV2& CloversV2::loadRaw(Event const & event)
{
  this->clear();
  for (int hit_i = 0; hit_i<event.mult; hit_i++) this->fillRaw(event, hit_i);
  this->analyze();
  return *this;
}

CloversV2& CloversV2::operator=(Event const & event)
{
  return this -> load(event);
}

void CloversV2::analyze() noexcept
{
  if (m_analyzed) return;
  m_analyzed = true;
  for (auto const & clover_index : Hits_id)
  {
    auto const & clover    = m_clovers[clover_index];
    auto const & Ge_found  = clover.nb    > 0;
    auto const & BGO_found = clover.nbBGO > 0;

    auto clover_ptr = &clover;

    all.push_back(clover_ptr);

    if(Ge_found)
    {
      Ge.push_back(clover_ptr);
      if (BGO_found) 
      {
        Rejected_id.push_back(clover_index);
        BGO        .push_back(clover_ptr);
        rejected   .push_back(clover_ptr);
      }
      else
      {
        GeClean_id.push_back(clover_index);
        clean     .push_back(clover_ptr);
      }
    }
    else if (BGO_found) 
    {
      BGOClean_id.push_back(clover_index);
      BGO        .push_back(clover_ptr);
      cleanBGO   .push_back(clover_ptr);
    }
    else throw_error("CloversV2::analyze() : aille aille aille");
  }
}

void CloversV2::analyzeRaw() noexcept
{
  if (m_analyzed) return;
  m_analyzed = true;
  for (auto const & clover_index : Hits_id)
  {
    auto const & clover = m_clovers[clover_index];
    auto const & Ge_found = clover.nb>0;
    auto const & BGO_found = clover.nbBGO>0;

    auto clover_ptr = &clover;

    all.push_back(clover_ptr);

    if(Ge_found)
    {
      Ge.push_back(clover_ptr);
      if (BGO_found) 
      {
        BGO.push_back(clover_ptr);
        Rejected_id.push_back(clover_index);
        rejected.push_back(clover_ptr);
      }
      else           
      {
        GeClean_id.push_back(clover_index);
        clean.push_back(clover_ptr);
      }
    }
    else if (BGO_found) 
    {
      BGO.push_back(clover_ptr);
      BGOClean_id.push_back(clover_index);
      cleanBGO.push_back(clover_ptr);
    }
    else throw_error("c pas normal ça");
  }
}

std::ostream & operator<<(std::ostream & os, CloversV2 const & clovers) noexcept
{
  os << clovers.Hits_id.size() << " clovers hit :" << std::endl;
  for (auto const & clover_i : clovers.Hits_id)
  {
    os << clovers[clover_i] << std::endl;
  }
  return os;
}

#endif //CLOVERSV2_HPP