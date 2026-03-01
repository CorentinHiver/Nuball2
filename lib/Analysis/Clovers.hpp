#pragma once

#include "Clover.hpp"

/**
 * @brief A class to simplify the analysis of the Clover data
 * @todo Create a "greylist" for HPGe to reject because of poor resolution, but still usable for calorimetry 
 * and Compton rejection of other HPGe of the same clover (i.e., they may have bad energy resolution, 
 * if their timing is good they should be considered "grey")
 */
class Clovers
{
  static constexpr inline size_t m_size = 24;
public:
  Clovers() noexcept {
    Clover::resetGlobalLabel(); // This allows to correctly label the CloverModules in other instances of Clovers  
  }
  ~Clovers() noexcept = default;
  
  Clovers& operator=(Clovers const & other) noexcept = default;
  
  bool fill   (Event const & event, int const & hit_i);
  bool fillRaw(Event const & event, int const & hit_i);
  
  Clovers& load      (Event const & event);
  Clovers& loadRaw   (Event const & event);
  Clovers& operator= (Event const & event);

  void analyze   () noexcept;
  void analyzeRaw() noexcept;

  void clear() noexcept
  {
    calorimetryTotal  = {};
    calorimetryGe     = {};
    calorimetryBGO    = {};
    m_analyzed        = {};
    for(auto const & clover_id : hits) 
      m_clovers[clover_id].clear();
    hits    .clear();
    Ge      .clear();
    BGO     .clear();
    clean   .clear();
    cleanBGO.clear();
    rejected.clear();
  }

  NRJ calorimetryTotal = {};
  NRJ calorimetryGe    = {};
  NRJ calorimetryBGO   = {};

  // Indices :

  std::vector<Index> hits    ;
  std::vector<Index> Ge      ;
  std::vector<Index> BGO     ;
  std::vector<Index> clean   ;
  std::vector<Index> cleanBGO;
  std::vector<Index> rejected;

  inline constexpr auto const & operator[] (int const & i) const noexcept { return m_clovers[i]; }

  // Static parameters :

  using Blacklist = std::unordered_set<Label>;
  using Overflow  = std::unordered_map<Label, NRJ>;

  static inline Blacklist sBlacklist;
  static inline Overflow  sOverflow ;

  static void setEnergyThreshold (NRJ threshold  ) {sThreshold    = threshold   ;}
  static void setSmearGeCaloR    (NRJ resolution ) {sSmearedCaloR = resolution  ; sSmearGeCalo = true;}

  static void setBlacklist (Blacklist const & blacklist   ) {sBlacklist = blacklist  ; sUseBlacklist = true;}
  static void setOverflow  (Overflow  const & overflow_Ge ) {sOverflow  = overflow_Ge; sUseOverflow  = true;}
  
  static bool rejectDetector(Label const & label, NRJ const & nrj)
  {
    bool ret = false;
    if (sUseBlacklist)
    {
      // User-selected blacklisted detectors labels :
      static thread_local auto const blacklistedLabel = Colib::LUT<1000>([](Label const & _label) {
        return Colib::found(sBlacklist, _label);
      });

      ret = blacklistedLabel[label];
    } 

    if (sUseOverflow)
    {
      // User-selected overflow energy value :
      static auto hasOverflow = Colib::LUT<1000>([](Label const & _label){
        return Colib::key_found(sOverflow, _label);
      });
       ret = hasOverflow[label] && nrj > sOverflow[label];
    }
    return ret;
  }

private:

  // Data
  std::array<Clover, m_size> m_clovers;

  // Parameters :
  // Member :
  bool m_analyzed = {};
  // Static :
  static inline NRJ  sThreshold    = {5_keV  };
  static inline NRJ  sSmearedCaloR = {400_keV};
  static inline bool sSmearGeCalo  = {};
  static inline bool sUseBlacklist = {};
  static inline bool sUseOverflow  = {};

public:

  static inline std::array<std::size_t, m_size> s_rotations = {{}};

  static bool setRotation(size_t const index, size_t const rotation) 
  {
    if (m_size <= index) {error("In Clover::setRotationsFile : label is", index ,"! There are only", m_size, "Clovers..."); return false;}
    if (3 < rotation) {error("In Clover::setRotationsFile : for label", index, "the rotation is >3. What do you mean ?"); return false;}
    s_rotations[index] = rotation;
    return true;
  }
  
  static bool setRotationsFile(std::ifstream & file)
  {
    std::string line;
    while(std::getline(file, line))
    {
      std::istringstream iss(line);
      size_t index = 0;
      size_t rotation = 0;
      iss >> index >> rotation;
      if (!setRotation(index, rotation)) return false;
    }
    return true;
  }

  
  static bool setRotationsFile(std::string const & filename)
  {
    std::ifstream file(filename);
    if (!file.is_open()) {error("In Clover::setRotationsFile : file", filename, "can't be opened"); return false;}
    auto const & ret = setRotationsFile(file);
    file.close();
    return ret;
  }

  static Index constexpr rotate(Index const sub_index) noexcept
  {
    if (sub_index < 2) return sub_index; // do not rotate BGO
    auto const Ge_sub_index = sub_index - 2;
    auto const & rotation = s_rotations[Ge_sub_index];
    constexpr Index ROTATION_LUT[4][4] = {
      {0, 1, 2, 3}, // Case 0: No rotation
      {2, 0, 3, 1}, // Case 1: -pi/2 rotation
      {1, 3, 0, 2}, // Case 2: pi/2 rotation
      {3, 2, 1, 0}  // Case 3: pi rotation
    };
    return ROTATION_LUT[rotation][Ge_sub_index];
  }
  
  /// @brief Returns 0 for R3 and 1 for R2
  static constexpr Index ring(Index clover_index) const {return clover_index % 2;}
};

/**
 * @brief 
 * @attention Extracts energy from the nrj field, i.e., calibrated energy in keV, hence requires calibration
 */
bool Clovers::fill(Event const & event, int const & hit_i)
{
  if (m_analyzed) Colib::throw_error("Clovers::fill() called while already analyzed, you need to Clovers::reset() (or clear()) first");
  auto const & label = event.labels[hit_i];
  auto const & nrj   = event.nrjs  [hit_i];
  
  if (Clovers::rejectDetector(label, nrj)) return false;
  
  if (NSI136::isClover[label])
  {
    auto const & nrj          = event.nrjs          [hit_i];
    auto const & time         = event.times         [hit_i];
    auto const & clover_index = NSI136::cloverIndex [label];

    auto const & sub_index = rotate(NSI136::crystalIndex[label]);

    Colib::pushBackUnique(hits, clover_index);

    m_clovers[clover_index].addHit(nrj, time, sub_index);
    
    NRJ calo;
    if (NSI136::isGe[label])
    {
      calo = (sSmearGeCalo) ? randomCo::gaussian( nrj, nrj * ((sSmearedCaloR / sqrt(nrj)) / NRJ{100}) / 2.35 ) : nrj;
      Colib::pushBackUnique(Ge, clover_index);
      calorimetryGe += calo;
    }
    else
    {
      Colib::pushBackUnique(BGO, clover_index);
      calo = nrj;
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
bool Clovers::fillRaw(Event const & event, int const & hit_i)
{
  if (m_analyzed) Colib::throw_error("Clovers::fillRaw() called while already analyzed, you need to Clovers::reset() (or clear()) first");
  auto const & label = event.labels[hit_i];
  auto const & nrj   = event.adcs  [hit_i];

  if (rejectDetector(label, nrj)) return false;

  if (NSI136::isGe[label])
  {
    auto const & nrj          = event.adcs          [hit_i];
    auto const & time         = event.times         [hit_i];
    auto const & clover_index = NSI136::cloverIndex [label];
    auto const & sub_index    = rotate(NSI136::crystalIndex[label]);

    Colib::pushBackUnique(hits, clover_index);

    m_clovers[clover_index].addHit(nrj, time, sub_index);

    return true;
  }
  return false;
}

Clovers& Clovers::load(Event const & event)
{
  clear();
  for (int hit_i = 0; hit_i<event.mult; hit_i++) fill(event, hit_i);
  analyze();
  return *this;
}

Clovers& Clovers::loadRaw(Event const & event)
{
  clear();
  for (int hit_i = 0; hit_i<event.mult; hit_i++) fillRaw(event, hit_i);
  analyze();
  return *this;
}

Clovers& Clovers::operator=(Event const & event)
{
  return load(event);
}

void Clovers::analyze() noexcept
{
  if (m_analyzed) return;
  m_analyzed = true;
  for (auto const & clover_index : hits)
  {
    auto const & clover    = m_clovers[clover_index];
    auto const & Ge_found  = clover.nb    > 0;
    auto const & BGO_found = clover.nbBGO > 0;

    if(Ge_found)
    {
      Ge.push_back(clover_index);
      if (BGO_found) 
      {
        rejected.push_back(clover_index);
        BGO     .push_back(clover_index);
      }
      else
      {
        clean.push_back(clover_index);
      }
    }
    else if (BGO_found) 
    {
      cleanBGO.push_back(clover_index);
      BGO     .push_back(clover_index);
    }
    else Colib::throw_error("Clovers::analyze() : aille aille aille");
  }
}

std::ostream & operator<<(std::ostream & os, Clovers const & clovers) noexcept
{
  os << clovers.hits.size() << " clovers hit :" << std::endl;
  for (auto const & clover_i : clovers.hits)
  {
    os << clovers[clover_i] << std::endl;
  }
  return os;
}