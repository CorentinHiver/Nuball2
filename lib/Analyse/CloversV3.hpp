#ifndef CLOVERSV3_HPP
#define CLOVERSV3_HPP

#include "../Classes/Event.hpp"
#include "CloverModule.hpp"

// class Calorimetry : public Module
// {
// public:
//   Calorimetry() noexcept = default;
//   Calorimetry(Time const & _time, float const & _nrj) noexcept : time(_time), nrj(_nrj) {}
//   void reset()
//   {
//     time = 0;
//     nrj = 0;
//   }
// };

/**
 * @brief A class to simplify the analysis of the Clovers data
 * @details 
 *  Contains 24 vector of clovers. Usually only one clover per vector is created.
 */
class CloversV3
{
public:
  CloversV3() noexcept
  {
    CloverModule::resetGlobalLabel(); // This allows to correctly label the CloverModules in other instances of CloversV3  
  }
  inline constexpr auto const & operator[](int const & i) const noexcept { return m_clovers[i]; }
  CloversV3& operator=(CloversV3 const & other) //= delete; // Can't copy a CloversV3
  {
    analyzed = other.analyzed;

    for(auto const & clover : all_id) m_clovers[clover].clear();
    all_id = other.all_id;
    for(auto const & clover : all_id) m_clovers[clover] = other.m_clovers[clover];

    return *this;
  }

  static constexpr inline uchar subIndex(Label const & label) noexcept {return uchar_cast((label-23)%6);}
  static constexpr inline bool  isClover(Label const & label) noexcept {return 22 < label && label < 200;}
  static constexpr inline bool  isGe(Label const & label)     noexcept {return (isClover(label) && subIndex(label)>1);}
  static constexpr inline bool  isR2(Label const & label)     noexcept {return (isClover(label) && label > 94);}
  static constexpr inline bool  isR3(Label const & label)     noexcept {return (isClover(label) && label < 95);}
  static constexpr inline bool  isBGO(Label const & label)    noexcept {return (isClover(label) && subIndex(label)<2);}
  /// @brief label = 23 -> index = 0, label = 196 -> index = 23;
  static constexpr inline Label index(Label const & label)    noexcept {return Label_cast((label-23)/6);}

  void operator=(Event const & event)
  {
    this->clear();
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {
      if (event.nrjs[hit_i] < threshold) continue;// Energy threshold
      auto const & label = event.labels[hit_i];
      if (found(blacklist, label)) continue;// Bad detectors
      if (found(maxE_Ge, label) && event.nrjs[hit_i] > maxE_Ge[label]) continue; // Overflow
      this->fill(event, hit_i);
    }
    this->analyze();
  }

  bool fill(Event const & event, int const & hit_i)
  {
    if (analyzed) throw_error("CloversV3::fill() called while already analyzed, you need to CloversV3::reset() first");
    auto const & label = event.labels[hit_i];
    if (isClover(label))
    {
      auto const & nrj = event.nrjs[hit_i];
      auto const & time = event.times[hit_i];
      auto const & clover_index = CloversV3::index(label); 
      auto const & sub_index = subIndex(label);
      push_back_unique(all_id, clover_index);
      m_clovers[clover_index].addHit(nrj, time, sub_index);
      return true;
    }
    return false;
  }

  void analyze() noexcept
  {
    if (analyzed) return;
    analyzed = true;

    // Perform add-back :
    for (auto const & clover_index : all_id)
    {
      for (auto & clover : m_clovers[clover_index])
      {// There is more than one loop if two separate gamma hit the same clover at more than CloversTimeWindow ns (usually 50ns)
        // auto const & Ge_found = clover.nb>0;
        // auto const & BGO_found = clover.nbBGO>0;
        all.push_back(&clover);
             if (clover.isCleanGe ()) cleanGe .push_back(&clover);
        else if (clover.isCleanBGO()) cleanBGO.push_back(&clover);
        else rejected.push_back(&clover);
      }
    }
  }

  void reset()
  {
    analyzed = false;
    for(auto const & clover : all_id) m_clovers[clover].clear();
    all_id.clear();
    cleanGe.clear();
    cleanBGO.clear();
    rejected.clear();
    all.clear();
  }

  void clear(){reset();}

  std::vector<Label> all_id;
  std::vector<CloverModule*> all;
  std::vector<CloverModule*> cleanGe;
  std::vector<CloverModule*> cleanBGO;
  std::vector<CloverModule*> rejected;
  // std::vector<Calorimetry> calorimetry;

  static std::unordered_set<Label> blacklist;
  static std::unordered_map<Label, double> maxE_Ge;

private:
  std::array<CloverModules, 24> m_clovers;
  bool analyzed = false;
  constexpr static double threshold = 5; // 5 keV
};

std::ostream & operator<<(std::ostream & os, CloversV3 const & clovers) noexcept
{
  os << clovers.all.size() << " clovers hit :" << std::endl;
  for (auto const & clover : clovers.all)
  {
    os << *clover << std::endl;
  }
  return os;
}

#endif //CLOVERSV3_HPP