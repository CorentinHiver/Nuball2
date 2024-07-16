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
  inline constexpr auto const & operator[](int const & i) const noexcept { return m_clovers[i]; }
  CloversV2& operator=(CloversV2 const & other)
  {
    calorimetryTotal = other.calorimetryTotal;
    calorimetryGe = other.calorimetryGe;
    calorimetryBGO = other.calorimetryBGO;
    analyzed = other.analyzed;

    for(auto const & clover : Hits_id) m_clovers[clover].clear();
    Hits_id = other.Hits_id;
    for(auto const & clover : Hits_id) m_clovers[clover] = other.m_clovers[clover];

    Ge_id = other.Ge_id;
    BGO_id = other.BGO_id;
    GeClean_id = other.GeClean_id;
    BGOClean_id = other.BGOClean_id;
    Rejected_id = other.Rejected_id;
    clean = other.clean;
    rejected = other.rejected;
    all = other.all;
    return *this;
  }

  static constexpr inline uchar subIndex(Label const & label) noexcept {return uchar_cast((label-23)%6);}
  static constexpr inline bool  isClover(Label const & label) noexcept {return 22 < label && label < 200;}
  static constexpr inline bool  isGe(Label const & label)     noexcept {return (isClover(label) && subIndex(label)>1);}
  static constexpr inline bool  isR2(Label const & label)     noexcept {return (isClover(label) && label > 94);}
  static constexpr inline bool  isR3(Label const & label)     noexcept {return (isClover(label) && label < 95);}
  static constexpr inline bool  isBGO(Label const & label)    noexcept {return (isClover(label) && subIndex(label)<2);}
  /// @brief label = 23 -> index = 0, label = 196 -> index = 23;
  static constexpr inline Index index(Label const & label)    noexcept {return Label_cast((label-23)/6);}

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
    if (analyzed) throw_error("CloversV2::fill() called while already analyzed, you need to CloversV2::reset() first");
    auto const & label = event.labels[hit_i];
    if (isClover(label))
    {
      auto const & nrj = event.nrjs[hit_i];
      auto const & time = event.times[hit_i];
      auto const & clover_index = CloversV2::index(label); 
      auto const & sub_index = subIndex(label);
      push_back_unique(Hits_id, clover_index);
      m_clovers[clover_index].addHit(nrj, time, sub_index);
      if (isGe(label))
      {
        push_back_unique(Ge_id, clover_index);
        auto const & smeared_nrj = random->Gaus(nrj, nrj*((400_keV/sqrt(nrj))/100_keV)/2.35);
        calorimetryTotal+=smeared_nrj;
        calorimetryGe+=smeared_nrj;
      }
      else
      {
        push_back_unique(BGO_id, clover_index);
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
    for (auto const & clover_index : Hits_id)
    {
      auto const & clover = m_clovers[clover_index];
      auto const & Ge_found = clover.nb>0;
      auto const & BGO_found = clover.nbBGO>0;

      all.push_back(&clover);

      if(Ge_found)
      {
        if (BGO_found) 
        {
          Rejected_id.push_back(clover_index);
          rejected.push_back(&clover);
        }
        else           
        {
          GeClean_id.push_back(clover_index);
          clean.push_back(&clover);
        }
      }
      else if (BGO_found) BGOClean_id.push_back(clover_index);
      else throw_error("c pas normal ça");
    }
  }

  void reset()
  {
    calorimetryTotal = 0.0;
    calorimetryGe = 0.0;
    calorimetryBGO = 0.0;
    analyzed = false;
    for(auto const & clover : Hits_id) m_clovers[clover].reset();
    Hits_id.clear();
    Ge_id.clear();
    BGO_id.clear();
    GeClean_id.clear();
    BGOClean_id.clear();
    Rejected_id.clear();
    clean.clear();
    rejected.clear();
    all.clear();
  }

  void clear(){reset();}

  double calorimetryTotal = 0.0;
  double calorimetryGe = 0.0;
  double calorimetryBGO = 0.0;

  std::vector<Index> Hits_id;
  std::vector<Index> Ge_id;
  std::vector<Index> BGO_id;
  std::vector<Index> GeClean_id;
  std::vector<Index> BGOClean_id;
  std::vector<Index> Rejected_id;

  // Views :
  std::vector<const CloverModule*> all;
  std::vector<const CloverModule*> clean;
  std::vector<const CloverModule*> rejected;

  auto begin() {return all.begin();}
  auto end() {return all.end();}
  auto begin() const {return all.begin();}
  auto end() const {return all.end();}

  static std::unordered_set<Label> blacklist;
  static std::unordered_map<Label, double> maxE_Ge;

private:
  std::array<CloverModule, 24> m_clovers;
  bool analyzed = false;
  constexpr static double threshold = 5; // 5 keV
  TRandom* random = new TRandom(time(0));
};

std::ostream & operator << (std::ostream & os, CloversV2 const & clovers) noexcept
{
  os << clovers.Hits_id.size() << " clovers hit :" << std::endl;
  for (auto const & clover_i : clovers.Hits_id)
  {
    os << clovers[clover_i] << std::endl;
  }
  return os;
}

#endif //CLOVERSV2_HPP