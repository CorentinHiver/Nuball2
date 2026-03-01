#ifndef CLOVERMODULE_H
#define CLOVERMODULE_H

#include "../Classes/Hit.hpp"

/**
 * @brief Describes a Clover module
 * @details
 * Each Clover has its own index, between 0 and (nb_labels-1)
 * In Nuball2, they range from 0 to 23
 * 
 * This class manages the number of Ge crystals and BGO crystals that fired in the event,
 */

/**
 * @brief Represents one clover with its 4 Germaniums and 2 BGO shields.
 * @details The sub-indexes are : [0,1] BGO, [2,4] Ge | sub_index = (label-23)%6.
 */
class CloverModule
{
public:

  // Member variables :
  
  uchar nb      = 0;  // Number of Ge crystals in the clover
  uchar nbBGO   = 0;  // Number of BGO crystals in the clover
  float nrj     = 0.; // Add-backed energy of Ge  Clovers
  float nrjBGO  = 0.; // Add-backed energy of BGO Clovers
  Time  time    = 0.; // Time of the crystal with most energy deposit of the clover, ps
  Time  timeBGO = 0.; // Time of the latest BGO, ps

  double GeCrystals[4] = {0};       // The four cristals
  std::vector<Index> GeCrystalsId;  // The index of the four cristals

  float maxE_Ge = 0.0;
  uchar maxE_Ge_cristal = 0u; // Index of the Ge crystal with the most energy deposit in the clover. [0;4]
  
  float calo = 0;     // Total energy deposit in the clover

  // Constructors : 
  CloverModule() : m_index(gIndex) {++gIndex;}
  CloverModule(Index const & index) : m_index(index)
  {
  #ifdef VERBOSE_CLOVER
    print("creating clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  // Methods :
  void         reset();
  void         clear()       {this -> reset();}
  auto const & index() const {return m_index;}
  auto         label() const {return 23+m_index*6+maxE_Ge_cristal;}

  static void resetGlobalLabel() {gIndex = 0;}

  void addHit(float const & _nrj, Time const & _time, uchar const & sub_index, bool grey_listed = false)
  {
    calo += _nrj;
    if (sub_index<2 || grey_listed) addBGO(_nrj, _time);
    else                            addGe (_nrj, _time, sub_index);
  }

  /// @brief Deprecated, use addHit instead
  void addGe(float const & _nrj, Time const & _time, uchar const & sub_index) 
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

  /// @brief Deprecated, use addHit instead
  void addBGO(float const & _nrj, Time const & _time) 
  {
    nrjBGO += _nrj ;
    timeBGO = _time;
    ++nbBGO;
  }

  constexpr bool hasGe      () const {return nb    >  0 ;}
  constexpr bool hasBGO     () const {return nbBGO >  0 ;}
  constexpr bool isCleanGe  () const {return nbBGO == 0 ;}
  constexpr bool isCleanBGO () const {return nb    == 0 ;}
  constexpr bool isRejected () const {return nbBGO > 0 && nb > 0;}
  constexpr uchar nbCrystals() const {return nb + nbBGO;}

  /// @brief Returns 0 for R3 and 1 for R2
  constexpr uchar ring() const {return m_index % 2;}

  // Rotation 1 : m_index == 5
  // Rotation 2 : m_index == 14 || m_index == 15
  // Rotation 3 : m_index == 17

  static inline std::array<std::size_t, 24> s_rotations = {0};

  static bool setRotation(size_t const & index, size_t const & rotation) 
  {
    if (index>23) {error("In CloverModule::setRotationsFile : label is", index ,"! There are only 24 Clovers..."); return false;}
    if (rotation > 3) {error("In CloverModule::setRotationsFile : for label", index, "the rotation is >3. What do you mean ?"); return false;}
    s_rotations[index] = rotation;
    return true;
  }
  
  bool setRotation(size_t const & rotation) {return setRotation(m_index, rotation);}

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
    if (!file.is_open()) {error("In CloverModule::setRotationsFile : file", filename, "can't be opened"); return false;}
    auto const & ret = setRotationsFile(file);
    file.close();
    return ret;
  }

  /// @brief returns 0 for R3 upstream, 1 for R3 downstream, 2 for R2 upstream, 3 for R2 downstream
  uchar sub_ring() const
  {
    auto const & rotation = s_rotations[m_index];
    switch (rotation)
    {
      case 0 : // No rotation 
      {
        switch (maxE_Ge_cristal)
        {
          case 0: case 1: return 2*ring()+1;
          case 2: case 3: return 2*ring();
        }
      }
      case 1 : // -pi/2 rotation
      {
        switch (maxE_Ge_cristal)
        {
          case 0: case 3 : return 2*ring()+1;
          case 1: case 2 : return 2*ring();
        }
      }
      case 2 : // pi/2 rotation
      {
        switch (maxE_Ge_cristal)
        {
          case 1: case 2 : return 2*ring()+1;
          case 0: case 3 : return 2*ring();
        }
      }
      case 3 : // pi rotation
      {
        switch (maxE_Ge_cristal)
        {
          case 2: case 3 : return 2*ring()+1;
          case 0: case 1 : return 2*ring();
        }
      }
      default : return -1;
    }
  }


  // Copy constructors and operators :

  CloverModule(CloverModule const & other) :
    nb                (other.nb),
    nbBGO             (other.nbBGO),
    nrj               (other.nrj),
    nrjBGO            (other.nrjBGO),
    time              (other.time),
    timeBGO           (other.timeBGO),
    maxE_Ge           (other.maxE_Ge),
    maxE_Ge_cristal   (other.maxE_Ge_cristal),
    calo              (other.calo),
    m_index           (other.m_index)
  {
  #ifdef VERBOSE_CLOVER
    print("copying clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  CloverModule(CloverModule && other) :
    nb              (std::move(other.nb             )),
    nbBGO           (std::move(other.nbBGO          )),
    nrj             (std::move(other.nrj            )),
    nrjBGO          (std::move(other.nrjBGO         )),
    time            (std::move(other.time           )),
    timeBGO         (std::move(other.timeBGO        )),
    maxE_Ge         (std::move(other.maxE_Ge        )),
    maxE_Ge_cristal (std::move(other.maxE_Ge_cristal)),
    calo            (std::move(other.calo           )),
    m_index         (std::move(other.m_index        ))
  {
  #ifdef VERBOSE_CLOVER
    print("moving clover n°", int_cast(m_index));
  #endif //VERBOSE_CLOVER
  }

  CloverModule& operator=(CloverModule const & other)
  {
    nb              = other.nb;
    nbBGO           = other.nbBGO;
    nrj             = other.nrj;
    nrjBGO          = other.nrjBGO;
    time            = other.time;
    timeBGO         = other.timeBGO;
    maxE_Ge         = other.maxE_Ge;
    maxE_Ge_cristal = other.maxE_Ge_cristal;
    calo            = other.calo;
    return *this;
  }

  CloverModule& operator=(CloverModule && other)
  {
    nb              = std::move(other.nb);
    nbBGO           = std::move(other.nbBGO);
    nrj             = std::move(other.nrj);
    nrjBGO          = std::move(other.nrjBGO);
    time            = std::move(other.time);
    timeBGO         = std::move(other.timeBGO);
    maxE_Ge         = std::move(other.maxE_Ge);
    maxE_Ge_cristal = std::move(other.maxE_Ge_cristal);
    calo            = std::move(other.calo);
    return *this;
  }

private:
  uchar mutable m_index;
  static inline thread_local uchar gIndex = 0;
};

void CloverModule::reset()
{
  nrj              = 0.f ;
  nrjBGO           = 0.f ;
  nb               = 0   ;
  nbBGO            = 0   ;
  time             = 0.0 ;
  timeBGO          = 0.0 ;
  maxE_Ge          = 0.0 ;
  maxE_Ge_cristal  = '\0';
  calo             = 0.0 ;
  for (auto const & id : GeCrystalsId) GeCrystals[id] = 0;
  GeCrystalsId.clear();
}

std::ostream& operator<<(std::ostream& out, CloverModule const & cloverModule)
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
//   CloverModules() noexcept : m_index(CloverModule::gIndex++) {}
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

//   void addHit(float const & _nrj, Time const & _time, uchar const & sub_index)
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
//       CloverModule _module(m_index);
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
//   std::vector<CloverModule> m_modules;
//   uchar mutable m_index;
//   static thread_local uchar gIndex;
// };

// std::ostream& operator<<(std::ostream& out, CloverModules const & clovers)
// {
//   for (auto const & clover : clovers) out << clover;
//   return out;
// }

#endif //CLOVERMODULE_H
