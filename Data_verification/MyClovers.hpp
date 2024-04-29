#ifndef MYCLOVERS_HPP
#define MYCLOVERS_HPP

#include "../lib/Classes/Event.hpp"
#include "../lib/Analyse/CloverModule.hpp"

using Label = unsigned short;
inline bool isGe(Label const & label) {return (label < 200 && (label+1)%6 > 1);}
/**
 * @brief A class to simplify the analysis of the Clover data
 * @details Removes the handling of the crystal of maximal energy deposit.
 */
class MyClovers
{
public:
  MyClovers() noexcept
  {
    CloverModule::resetGlobalLabel(); // This allows to correctly label the CloverModules in other instances of MyClovers  
  };
  inline auto const & operator[](int const & i) const noexcept { return m_clovers[i]; }

  static inline uchar subIndex(Label const & label) noexcept {return (label+1)%6;}
  static inline bool  isClover(Label const & label) noexcept {return label<200;}
  static inline bool  isGe(Label const & label)     noexcept {return (isClover(label) && subIndex(label)>1);}
  static inline bool  isBGO(Label const & label)    noexcept {return (isClover(label) && subIndex(label)<2);}
  /// @brief label = 23 -> index = 0, label = 196 -> index = 23;
  static inline Label index(Label const & label)    noexcept {return (label-23)/6;}

  bool fill(Event const & event, int const & hit_i)
  {
    if (analyzed) throw_error("MyClovers::fill() called while already analyzed, you need to MyClovers::reset first");
    auto const & label = event.labels[hit_i];
    if (isClover(label))
    {
      auto const & nrj = event.nrjs[hit_i];
      auto const & time = event.times[hit_i];
      auto const & clover_index = MyClovers::index(label); 
      auto const & sub_index = subIndex(label);
      push_back_unique(Hits, clover_index);
      m_clovers[clover_index].addHit(nrj, time, sub_index);
      calorimetryTotal+=nrj;
      if (isGe(label))
      {
        push_back_unique(Ge, clover_index);
        calorimetryGe+=nrj;
      }
      else
      {
        push_back_unique(BGO, clover_index);
        calorimetryBGO+=nrj;
      }
      return true;
    }
    return false;
  }

  void analyze() noexcept
  {
    if (analyzed) return;
    analyzed = true;
    for (auto const & clover_index : Hits)
    {
      auto const & clover = m_clovers[clover_index];
      auto const & Ge_found = clover.nb>0;
      auto const & BGO_found = clover.nbBGO>0;

      if(Ge_found)
      {
        if (BGO_found) Rejected.push_back(clover_index);
        else           GeClean.push_back(clover_index);
      }
      else if (BGO_found) BGOClean.push_back(clover_index);
      else throw_error("c pas normal ça");
    }
  }

  void reset()
  {
    calorimetryTotal = 0.0;
    calorimetryGe = 0.0;
    calorimetryBGO = 0.0;
    analyzed = false;
    for(auto const & clover : Hits) m_clovers[clover].reset();
    Hits.clear();
    Ge.clear();
    BGO.clear();
    GeClean.clear();
    BGOClean.clear();
    Rejected.clear();
  }

  double calorimetryTotal = 0.0;
  double calorimetryGe = 0.0;
  double calorimetryBGO = 0.0;

  std::vector<Label> Hits;
  std::vector<Label> Ge;
  std::vector<Label> BGO;
  std::vector<Label> GeClean;
  std::vector<Label> BGOClean;
  std::vector<Label> Rejected;

private:
  std::array<CloverModule, 24> m_clovers;
  bool analyzed = false;
};

std::ostream & operator << (std::ostream & os, MyClovers const & clovers) noexcept
{
  os << clovers.Hits.size() << " clovers hit :" << std::endl;
  for (auto const & clover_i : clovers.Hits)
  {
    os << clovers[clover_i] << std::endl;
  }
  return os;
}


#endif //MYCLOVERS_HPP