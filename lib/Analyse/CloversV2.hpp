#ifndef CLOVERSV2_HPP
#define CLOVERSV2_HPP

#include "../Classes/Event.hpp"
#include "CloverModule.hpp"

// using Index = unsigned short;
// inline bool isGe(Index const & label) {return (label < 200 && (label+1)%6 > 1);}
/**
 * @brief A class to simplify the analysis of the Clover data
 * @details Removes the handling of the crystal of maximal energy deposit.
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

  // static constexpr inline uchar subIndex(Label const & label) noexcept {return uchar_cast((label-23)%6);}
  // static constexpr inline bool  isClover(Label const & label) noexcept {return 22 < label && label < 168;}
  // static constexpr inline bool  isGe    (Label const & label) noexcept {return (isClover(label) && subIndex(label)>1);}
  // static constexpr inline bool  isR2    (Label const & label) noexcept {return (isClover(label) && label > 94);}
  // static constexpr inline bool  isR3    (Label const & label) noexcept {return (isClover(label) && label < 95);}
  // static constexpr inline bool  isBGO   (Label const & label) noexcept {return (isClover(label) && subIndex(label)<2);}
  // static constexpr inline Index index(Label const & label)    noexcept {return Label_cast((label-23)/6);}
  
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
    if (sUseBlacklist)
    {
      // User-selected blacklisted detectors labels :
      static auto blacklistedLabel = LUT<1000>([](Label const & _label) {
        return found(sBlacklist, _label);
      });

      return blacklistedLabel[label];
    } 
    else return false;

    if (sUseOverflow)
    {
      // User-selected overflow energy value :
      static auto hasOverflow = LUT<1000>([](Label const & _label){
        return found(sOverflow  , _label);
      });
       return hasOverflow[label] && nrj > sOverflow[label];
    }
  }

private:

  std::array<CloverModule, 24> m_clovers;
  bool m_analyzed = false;

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


// std::unordered_set<Label> CloversV2::blacklist = {46, 55, 64, 69, 70, 80, 92, 97, 122, 129, 142, 163};
// std::unordered_map<Label, double> CloversV2::overflow_Ge = 
// {
//   {25, 12600 }, {26, 13600 }, {27, 10500 }, {28, 7500  }, 
//   {31, 11500 }, {32, 11400 }, {33, 8250  }, {34, 9000  }, 
//   {37, 11000 }, {38, 11100 }, {39, 11500 }, {40, 11300 }, 
//   {43, 12600 }, {44, 11900 }, {45, 11550 }, {46, 9200  }, 
//   {49, 14300 }, {50, 12800 }, {51, 13500 }, {52, 12400 }, 
//   {55, 5500  }, {56, 5500  }, 
//                 {68, 7100  }, {69, 15500 }, {70, 9500  },
//   {73, 11650 }, {74, 11600 }, {75, 11800 }, {76, 11600 }, 
//   {79, 11500 }, {80, 8000  }, {81, 18200 },
//   {85, 7700  }, {86, 12000 }, {87, 12000 }, {88, 11600 }, 
//   {91, 7900  }, {92, 10000 }, {93, 11500 }, {94, 11000 }, 
//   {97, 11400 }, {98, 11400 }, {99, 11250 }, {100, 8900 }, 
//   {103, 11400 }, {104, 11600 }, {105, 11600 }, {106, 11500 }, 
//   {109, 12800 }, {110, 1800  }, {111, 13000 }, {112, 11300 }, 
//   {115, 12800 }, {116, 11500 }, {117, 10500 }, {118, 11400 }, 
//   {121, 12400 }, {122, 20000 }, {123, 10700 }, {124, 20000 }, 
//   {127, 11600 }, {128, 11700 }, {129, 10000 }, {130, 11200 }, 
//   {133, 11200 }, {134, 9350  }, {135, 9400  }, {136, 9500  }, 
//   {139, 13200 }, {140, 12400 }, {141, 12900 }, {142, 4500  }, 
//   {145, 8200  }, {146, 9600  }, {147, 9100  }, {148, 10900 }, 
//   {151, 11900 }, {152, 12200 }, {153, 11300 }, {154, 12000 }, 
//   {157, 9110  }, {158, 9120  }, {159, 9110  }, {160, 11700 }, 
//   {163, 11000 }, {164, 11600 }, {165, 11600 }, {166, 11600 }, 
// };