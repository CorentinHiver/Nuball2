#pragma once

#include "../Classes/Event.hpp"

/**
 * @brief Describes a Clover module : One clover with its 4 Germanium leafs and 2 BGO shields.
 * @details
 * Each Clover has its own index, between 0 and (nb_labels-1)
 * In Nuball2, they range from 0 to 23
 * 
 * This class manages the number of Ge crystals and BGO crystals that fired in the event.
 */

class Clover
{
public:

  // Member variables :
  
  uint8_t  nb      = {};  // Number of Ge and BGO crystals in the clover
  uint8_t  nbBGO   = {};  // Number of BGO crystals in the clover
  Energy_t nrj     = {}; // Add-backed energy of Ge  Clovers
  Energy_t nrjBGO  = {}; // Add-backed energy of BGO Clovers
  Time     time    = {}; // Time of the crystal with most energy deposit of the clover, ps
  Time     timeBGO = {}; // Time of the latest BGO, ps

  std::array<Energy_t, 4> GeCrystals{{}};  // The four cristals
  std::vector<Index>      GeCrystalsId  ;  // The index of the four cristals

  Energy_t maxE_Ge = {};
  Index    maxE_Ge_cristal = {}; // Index of the Ge crystal with the most energy deposit in the clover. [0;4]
  
  Energy_t calo = {};     // Total energy deposit in the clover

  // Constructors : 
  Clover() : m_index(gIndex) {++gIndex;}
  Clover(Index const & index) : m_index(index)
  {
  #ifdef VERBOSE_CLOVER
    print("creating clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  // Methods :
  constexpr void clear() noexcept
  {
    nrj              = {};
    nrjBGO           = {};
    nb               = {};
    nbBGO            = {};
    time             = {};
    timeBGO          = {};
    maxE_Ge          = {};
    maxE_Ge_cristal  = {};
    calo             = {};

    GeCrystals      = {{}};

    GeCrystalsId.clear();
  }
  auto const & index() const noexcept {return m_index;}
  auto         label() const noexcept {return 23 + 6*m_index* + maxE_Ge_cristal;}

  static constexpr inline void resetGlobalLabel() noexcept {gIndex = 0;}

  void addHit(float const & _nrj, Time const & _time, Index const & sub_index, bool grey_listed = false) noexcept
  {
    calo += _nrj;
    if (sub_index<2 || grey_listed) addBGO(_nrj, _time);
    else                            addGe (_nrj, _time, sub_index);
  }

  constexpr inline bool hasGe      () const noexcept {return nb    >  0 ;}
  constexpr inline bool hasBGO     () const noexcept {return nbBGO >  0 ;}
  constexpr inline bool isCleanGe  () const noexcept {return nbBGO == 0 ;}
  constexpr inline bool isCleanBGO () const noexcept {return nb    == 0 ;}
  constexpr inline bool isRejected () const noexcept {return nbBGO > 0 && nb > 0;}
  constexpr inline uint8_t nbCrystals() const noexcept {return nb + nbBGO;}

private:
  void addGe(float const & _nrj, Time const & _time, Index const & sub_index) 
  {
    auto const & Ge_sub_index = sub_index-2;
    // if (grey_listed) 
    nrj += _nrj;
    // The highest energy deposit most likely corresponds to the hit before scattering
    if (_nrj > maxE_Ge)
    {
      maxE_Ge = _nrj;
      time = _time;
      maxE_Ge_cristal = Ge_sub_index;
    }
    ++nb;

    GeCrystalsId.push_back(Ge_sub_index);
    GeCrystals[Ge_sub_index]=_nrj;
  }

  void addBGO(float const & _nrj, Time const & _time) 
  {
    nrjBGO += _nrj ;
    timeBGO = _time;
    ++nbBGO;
  }
  // Copy constructors and operators are private, i.e. not usable -> Forbiden for users
  Clover(Clover const & other) noexcept = default; 
  Clover(Clover && other) noexcept = default; 
  Clover& operator=(Clover const & other) noexcept = default;
  Clover& operator=(Clover && other) noexcept = default;

private:
  Index mutable m_index;
  static inline thread_local Index gIndex = 0;
};

std::ostream& operator<<(std::ostream& out, Clover const & cloverModule)
{
  out << 

    "Clover n°" << " " << int_cast(cloverModule.index()) << " : " << 
    "nb Ge " <<  int_cast(cloverModule.nb) << " " << 
    "nb BGO " <<  int_cast(cloverModule.nbBGO) << " ";

    if (cloverModule.nb>0)
    {out << 

    "nrj : " <<  cloverModule.nrj << " " << 
    "time:  " <<  cloverModule.time/1000. << " ns " << 
    "id max E : " <<  int_cast(cloverModule.maxE_Ge_cristal) << " ";

    }
    if (cloverModule.nbBGO>0)
    { out << 

    "nrj BGO : " <<  cloverModule.nrjBGO << " " << 
    "time BGO :  " <<  cloverModule.timeBGO/1000. << " ns ";

    }
    if (cloverModule.nb>0 && cloverModule.nbBGO>0)
    { out << 

    "calo : " << cloverModule.calo << "keV";

    }
  return out;
}

// /**
//  * @brief Represents a collection of hits in the same clover
//  * 
//  */
// class CloverModules
// {
// public:
//   CloverModules() noexcept : m_index(Clover::gIndex++) {}
//   CloverModules(CloverModules const & other) :
//     m_modules (other.m_modules),
//     m_index (other.m_index)
//     {}

//   CloverModules& operator=(CloverModules const & other) //= delete; // Copy this class is forbidden so far
//   {
//     m_modules = other.m_modules;
//     m_index = other.m_index;
//     return *this;
//   }

//   void addHit(float const & _nrj, Time const & _time, uint8_t const & sub_index)
//   {
//     auto const & nb_modules = m_modules.size();
//     bool createModule = false;
//     if (nb_modules == 0) createModule = true;
//     else
//     {
//       auto const & prev_clover = m_modules[nb_modules-1];
//       auto const & dT = std::abs(_time - ((prev_clover.hasGe()) ? prev_clover.time : prev_clover.timeBGO));
//       if (dT>sTimeWindow) createModule = true;
//     }
//     if(createModule)
//     {
//       Clover _module(m_index);
//       _module.addHit(_nrj, _time, sub_index);
//       m_modules.emplace_back(std::move(_module));
//     }
//     else m_modules.back().addHit(_nrj, _time, sub_index);
//   }

//   void clear() {m_modules.clear();}

//   auto begin() {return m_modules.begin();}
//   auto end() {return m_modules.end();}
//   auto begin() const {return m_modules.begin();}
//   auto end() const {return m_modules.end();}
//   auto size() const {return m_modules.size();}

// static void setTimeWindow(Time const & ts){sTimeWindow = ts;} 
// static inline Time sTimeWindow = 50_ns;
  
// private:
//   std::vector<Clover> m_modules;
//   uint8_t mutable m_index;
//   static thread_local uint8_t gIndex;
// };

// std::ostream& operator<<(std::ostream& out, CloverModules const & clovers)
// {
//   for (auto const & clover : clovers) out << clover;
//   return out;
// }
