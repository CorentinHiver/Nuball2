#ifndef CLOVERMODULE_H
#define CLOVERMODULE_H
#include "Module.hpp"
/**
 * @brief Describes a Clover module
 * @details
 * Each Clover has its own index, between 0 and (nb_labels-1)
 * In Nuball2, they range from 0 to 23
 * 
 * This simple class manages the number of Ge crystals and BGO crystals that fired in the event,
 */

static constexpr Time CloversTimeWindow = 50_ns;

/**
 * @brief Represent one clover with its 4 germaniums and 2 BGO shields.
 * @details The sub-indexes are : [0,1] BGO, [2,4] Ge. Can be calculated as (label-23)%6.
 * 
 */
class CloverModule
{
public:
  // CloverModule() {}
  CloverModule() : m_index(gIndex) {++gIndex;}
  CloverModule(Index const & index) : m_index(index)
  {
  #ifdef VERBOSE_CLOVER
    print("creating clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  void reset();
  void clear() {this -> reset();}
  void clean() {this -> reset();}
  auto const & index() const {return m_index;}
  static void resetGlobalLabel() {gIndex = 0;}

  void addHit(float const & _nrj, Time const & _time, uchar const & sub_index)
  {
    calo += _nrj;
    if (sub_index<2) addBGO(_nrj, _time);
    else             addGe (_nrj, _time, sub_index);
  }

  /// @brief Deprecated, use addHit instead
  void addGe(float const & _nrj, Time const & _time, uchar const & sub_index) 
  {
    auto const & Ge_sub_index = sub_index-2;
    nrj += _nrj;
    // The highest energy deposit most likely corresponds to the hit before scattering
    if (_nrj>maxE_Ge)
    {
      maxE_Ge = _nrj;
      time = _time;
      maxE_Ge_cristal = Ge_sub_index;
    }
    ++nb;
    GeCrystalsId.push_back(Ge_sub_index);
    GeCrystals[Ge_sub_index]=_nrj;
  }

  /// @brief Deprecated, use addHit instead
  void addBGO(float const & _nrj, Time const & _time) 
  {
    nrjBGO += _nrj; 
    timeBGO = _time;
    ++nbBGO;
  }

  bool hasGe() const {return nb>0;}
  bool hasBGO() const {return nbBGO>0;}
  bool isCleanGe() const {return nbBGO == 0;}
  bool isCleanBGO() const {return nb == 0;}
  bool isRejected() const {return nbBGO > 0 && nb > 0;}

  uchar nbCrystals() const {return nb+nbBGO;}

  // In the following, if nothing is specified then it refers to the Germanium

  uchar nb = 0;       // Number of Ge crystals in the clover
  uchar nbBGO = 0;    // Number of BGO crystals in the clover
  float nrj = 0.;     // Add-backed energy of Ge  Clovers
  float nrjBGO = 0.;  // Add-backed energy of BGO Clovers
  Time time = 0.;     // Time of the crystal with most energy deposit of the clover, ps
  Time timeBGO = 0.;  // Time of the latest BGO, ps
  double GeCrystals[4] = {0};
  std::vector<Index> GeCrystalsId;

  float maxE_Ge = 0.0;
  uchar maxE_Ge_cristal = 0u; // Index of the Ge crystal with the most energy deposit in the clover. [0;4]
  
  float calo = 0;     // Total energy deposit in the clover

  uchar ring() const
  {
    return m_index%2;
  }
  uchar sub_ring() const
  {
    if (m_index == 5) 
    { // -pi/2 rotation
      switch (maxE_Ge_cristal)
      {
        case 0: case 3 : return 2*ring()+1;
        case 1: case 2: return 2*ring();
      }
    }
    else if (m_index == 14 || m_index == 15) 
    { // +pi/2 rotation
      switch (maxE_Ge_cristal)
      {
        case 1: case 2 : return 2*ring()+1;
        case 0: case 3: return 2*ring();
      }
    }
    else if (m_index == 17) 
    { // pi rotation
      switch (maxE_Ge_cristal)
      {
        case 2: case 3 : return 2*ring()+1;
        case 0: case 1: return 2*ring();
      }
    }
    else if (maxE_Ge_cristal < 2) return 2*ring()+1; // red and green
    else                     return 2*ring();   // black and blue
    return 0;
  }

  CloverModule(CloverModule const & other) :
    nb (other.nb),
    nbBGO (other.nbBGO),
    nrj (other.nrj),
    nrjBGO (other.nrjBGO),
    time (other.time),
    timeBGO (other.timeBGO),
    maxE_Ge (other.maxE_Ge),
    maxE_Ge_cristal (other.maxE_Ge_cristal),
    calo (other.calo),
    m_index (other.m_index)
  {
  #ifdef VERBOSE_CLOVER
    print("copying clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  CloverModule(CloverModule && other) :
    nb (std::move(other.nb)),
    nbBGO (std::move(other.nbBGO)),
    nrj (std::move(other.nrj)),
    nrjBGO (std::move(other.nrjBGO)),
    time (std::move(other.time)),
    timeBGO (std::move(other.timeBGO)),
    maxE_Ge (std::move(other.maxE_Ge)),
    maxE_Ge_cristal (std::move(other.maxE_Ge_cristal)),
    calo (std::move(other.calo)),
    m_index (std::move(other.m_index))
  {
  #ifdef VERBOSE_CLOVER
    print("moving clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  CloverModule& operator=(CloverModule const & other)
  {
    nb = other.nb;
    nbBGO = other.nbBGO;
    nrj = other.nrj;
    nrjBGO = other.nrjBGO;
    time = other.time;
    timeBGO = other.timeBGO;
    maxE_Ge = other.maxE_Ge;
    maxE_Ge_cristal = other.maxE_Ge_cristal;
    calo = other.calo;
    return *this;
  }

  CloverModule& operator=(CloverModule && other)
  {
    nb = std::move(other.nb);
    nbBGO = std::move(other.nbBGO);
    nrj = std::move(other.nrj);
    nrjBGO = std::move(other.nrjBGO);
    time = std::move(other.time);
    timeBGO = std::move(other.timeBGO);
    maxE_Ge = std::move(other.maxE_Ge);
    maxE_Ge_cristal = std::move(other.maxE_Ge_cristal);
    calo = std::move(other.calo);
    return *this;
  }

  friend class CloverModules;

private:
  uchar mutable m_index;
  static thread_local uchar gIndex;
};

uchar thread_local CloverModule::gIndex = 0;

void CloverModule::reset()
{
  nrj = 0.f;
  nrjBGO = 0.f;
  nb = 0;
  nbBGO = 0;
  time = 0.0;
  timeBGO = 0.0;

  maxE_Ge = 0.0;
  maxE_Ge_cristal = '\0';

  calo=0.0;

  for (auto const & id : GeCrystalsId) GeCrystals[id] = 0;
  GeCrystalsId.clear();
}

std::ostream& operator<<(std::ostream& cout, CloverModule const & cloverModule)
{
  cout << 
    "Clover n°" << " " << int_cast(cloverModule.index()) << " : " << 
    "nb Ge " <<  int_cast(cloverModule.nb) << " " << 
    "nb BGO " <<  int_cast(cloverModule.nbBGO) << " ";
    if (cloverModule.nb>0)
    {
cout << "nrj : " <<  cloverModule.nrj << " " << 
        "time:  " <<  cloverModule.time/1000. << " ns " << 
        "id max E : " <<  int_cast(cloverModule.maxE_Ge_cristal) << " ";
    }
    if (cloverModule.nbBGO>0)
    {
cout << "nrj : " <<  cloverModule.nrjBGO << " " << 
        "time:  " <<  cloverModule.timeBGO/1000. << " ns ";
    }
    if (cloverModule.nb>0 && cloverModule.nbBGO>0)
    {
cout << "calo : " << cloverModule.calo << "keV";
    }
  return cout;
}

class CloverModules
{
public:
  CloverModules() noexcept : m_index(CloverModule::gIndex++) {}
  CloverModules(CloverModules const & other) :
    m_modules (other.m_modules),
    m_index (other.m_index)
    {}

  CloverModules& operator=(CloverModules const & other) //= delete; // Copy this class is forbidden so far
  {
    m_modules = other.m_modules;
    m_index = other.m_index;
    return *this;
  }

  void addHit(float const & _nrj, Time const & _time, uchar const & sub_index)
  {
    auto const & nb_modules = m_modules.size();
    bool createModule = false;
    if (nb_modules == 0) createModule = true;
    else
    {
      auto const & prev_clover = m_modules[nb_modules-1];
      auto const & dT = std::abs(_time - ((prev_clover.hasGe()) ? prev_clover.time : prev_clover.timeBGO));
      if (dT>CloversTimeWindow) createModule = true;
    }
    if(createModule)
    {
      CloverModule _module(m_index);
      _module.addHit(_nrj, _time, sub_index);
      m_modules.emplace_back(std::move(_module));
    }
    else m_modules.back().addHit(_nrj, _time, sub_index);
  }

  void clear() {m_modules.clear();}

  auto begin() {return m_modules.begin();}
  auto end() {return m_modules.end();}
  auto begin() const {return m_modules.begin();}
  auto end() const {return m_modules.end();}
  auto size() const {return m_modules.size();}
  
private:
  std::vector<CloverModule> m_modules;
  uchar mutable m_index;
  static thread_local uchar gIndex;
};

std::ostream& operator<<(std::ostream& out, CloverModules const & clovers)
{
  for (auto const & clover : clovers) out << clover;
  return out;
}
#endif //CLOVERMODULE_H
