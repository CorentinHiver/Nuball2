#ifndef SIMPLEDSSD_HPP
#define SIMPLEDSSD_HPP

#include "../Classes/Event.hpp"
#include "ExcitationEnergy.hpp"

namespace DSSD
{
  static constexpr size_t LUT_size = 1000;
  static constexpr auto is        = LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 856;});
  static constexpr auto isRing    = LUT<LUT_size> ([](Label const & label) {return 839 < label && label < 856;});
  static constexpr auto isSector  = LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 840;});
  static constexpr auto isSector1 = LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 820;});
  static constexpr auto isSector2 = LUT<LUT_size> ([](Label const & label) {return 819 < label && label < 840;});
  
  static constexpr double z  = 3.25 ; // cm : Distance DSSD-target
  static constexpr double Ri = 1.58; // cm : Inner radius 
  static constexpr double Re = 4.05; // cm : Outer radius
  static constexpr std::size_t nb_rings = 16;
  static constexpr std::size_t nb_sectors = 32;
  static constexpr std::size_t nb_strips = nb_rings+nb_sectors;

  static constexpr auto index_ring = LUT<LUT_size> ([]  (Label const & label) 
  {
    return (DSSD::isRing[label]) ? label-840 : -1;
  });

  static constexpr auto index_sector = LUT<LUT_size> ([]  (Label const & label) 
  {
         if (DSSD::isSector1[label]) return label-800;
    else if (DSSD::isSector2[label]) return label-805;
    else return -1;
  });

  static constexpr auto index = LUT<LUT_size> ([]  (Label const & label) 
  {
         if (DSSD::isSector[label]) return int(index_sector[label]);
    else if (DSSD::isRing  [label]) return int(index_ring[label]+nb_sectors);
    else return int{-1};
  });


  static constexpr double angular_size_sector = 2*3.14159/nb_sectors;  
  static constexpr double size_ring = (Re-Ri)/nb_rings; // cm : 
  static constexpr inline double ring_radius(Index const & _index) {return Ri + size_ring*(nb_rings-_index) + size_ring/2.;}
  static constexpr inline double angle_ring  (Index const & _index) {return atan( ring_radius(_index) / z);}
  static constexpr inline double angle_sector(Index const & _index) 
  {
    Index physical_index = (_index<8) ? _index+23 : _index-8;
    return physical_index*angular_size_sector;
  }
  std::pair<double, double> polar(Index const & ring_id, Index const & sector_id, TRandom * random) 
  {
    auto const & r = ring_radius(ring_id)+random->Uniform(-size_ring/2, size_ring/2);
    auto const & phi = angle_sector(sector_id)+random->Uniform(-angular_size_sector/2, angular_size_sector/2);
    return {r*cos(phi), r*sin(phi)};
  }

  static constexpr Time ring_coinc_tw = 60_ns;
};

////////////////
/// CHANNELS ///
////////////////

class CellDSSD
{
public:
  CellDSSD(float const & _Ering, float const & _Esector, Time const & _Tring, Time const & _Tsector, double const & _ring_angle, double const & _theta):
  Ering(_Ering),
  Esector(_Esector),
  Tring(_Tring),
  Tsector(_Tsector),
  ring_angle(_ring_angle),
  theta(_theta)
  {
    nrj = (Ering+Esector)/2;
  }
  float Ering = 0;
  float Esector = 0;
  Time Tring = 0;
  Time Tsector = 0;
  double ring_angle = 0;
  double theta = 0;
  float nrj = 0;
  bool random = false;
};

class CellsDSSD
{
public:
  CellsDSSD() noexcept = default;

  // Reproduce vector methods
  void clear() 
  {
    m_cells.clear();
    m_clean_size = 0;
    analyzed = true;
  }

  auto begin() {return m_cells.begin();}
  auto end() {return m_cells.end();}
  auto begin() const {return m_cells.begin();}
  auto end() const {return m_cells.end();}
  
  auto emplace_back(CellDSSD && cell) {m_cells.emplace_back(std::forward<CellDSSD>(cell));}
  
  auto size() const {return m_cells.size();}
  auto size() {return m_cells.size();}

  auto const & clean_size() const {return m_clean_size;}
  auto & clean_size() {return m_clean_size;}

  auto const & back() const {return m_cells.back();}
  auto & back() {return m_cells.back();}

private:
  bool analyzed = false;
  size_t m_clean_size = 0;
  std::vector<CellDSSD> m_cells;
};

// using CellsDSSD = std::vector<CellDSSD>;


class StripDSSD
{protected:
  StripDSSD() noexcept : m_index(gIndex++), angle(0) {}
  StripDSSD(double const & _angle) : m_index(gIndex++), angle(_angle) {}

public:
  StripDSSD* fill(Event const & event, int const & hit_i)
  {
    nrj = event.nrjs[hit_i];
    time = event.times[hit_i];
    return this;
  }

  void clear()
  {
    nrj = 0.0;
    time = 0;
  }

  auto const & index() const {return m_index;}

  bool isVoisin(StripDSSD const & other) const {return std::abs(other.m_index - m_index) == 1;}
  bool isCoincident(StripDSSD const & other) const {return std::abs(other.time - time) < DSSD::ring_coinc_tw;}
  bool isVoisin(StripDSSD * other) const {return std::abs(other->m_index - m_index) == 1;}
  bool isCoincident(StripDSSD * other) const {return std::abs(other->time - time) < DSSD::ring_coinc_tw;}

  double nrj = 0.0;
  Time time = 0;

  static uchar thread_local gIndex;
protected:
  uchar mutable m_index;

public:
  double mutable angle;
};

uchar thread_local StripDSSD::gIndex = 0;

class RingDSSD : public StripDSSD
{public:
  RingDSSD() noexcept : StripDSSD(DSSD::angle_ring(StripDSSD::gIndex)) {}
};

class SectorDSSD : public StripDSSD
{public:
  SectorDSSD() noexcept : StripDSSD(DSSD::angle_sector(StripDSSD::gIndex)) {}
};

/////////////
/// FACES ///
/////////////
/**
 * @brief Group of strips, either rings or sectors
 */
class StripsDSSD
{public:
  StripsDSSD() noexcept = default;

  void fill(StripDSSD* strip, Index const & index)
  {
    strips.push_back(strip);
    strips_id.push_back(index);
    ++mult;
  }

  void clear()
  {
    for (auto const & strip : strips) strip->clear();
    strips.clear();
    strips_id.clear();
    mult = 0;
  }

  auto size() const {return strips.size();}
  auto begin() {return strips.begin();}
  auto end() {return strips.end();}
  auto begin() const {return strips.begin();}
  auto end() const {return strips.end();}

  std::vector<Index> strips_id;
  std::vector<StripDSSD*> strips;
  int mult = 0;
};

/**
 * @brief Group of rings
 * 
 */
class RingsDSSD : public StripsDSSD
{public:
  RingsDSSD() noexcept {StripDSSD::gIndex = 0;}
  StripDSSD* fill(Event const & event, int const & hit_i)
  {
    auto const & index = DSSD::index_ring[event.labels[hit_i]];
    auto ret = m_rings[index].fill(event, hit_i);
    StripsDSSD::fill(&(m_rings[index]), index);
    return ret;
  }

  auto operator[](int const & strip_i) const {return m_rings[strips_id[strip_i]];}
  // auto operator[](int const & strip_i) const {return m_rings[strip_i];}

private:
  std::array<RingDSSD, DSSD::nb_rings> m_rings;
};

/**
 * @brief Group of sectors
 * 
 */
class SectorsDSSD : public StripsDSSD
{public:
  SectorsDSSD() noexcept {StripDSSD::gIndex = 0;}
  StripDSSD* fill(Event const & event, int const & hit_i)
  {
    auto const & id = DSSD::index_sector[event.labels[hit_i]];
    auto ret = m_sectors[id].fill(event, hit_i);
    StripsDSSD::fill(&(m_sectors[id]), id);
    return ret;
  }
  auto operator[](int const & strip_i) const {return m_sectors[strips_id[strip_i]];}
  // auto operator[](int const & strip_i) const {return m_sectors[strip_i];}
  
private:
  std::array<SectorDSSD, DSSD::nb_sectors> m_sectors;
};

////////////////
/// DETECTOR ///
////////////////

class WarsawDSSD
{public:
  WarsawDSSD() = default;
  WarsawDSSD(ExcitationEnergy* Ex) : m_Ex(Ex) {}

  bool fill(Event const & event, int const & hit_i)
  {
    if (event.nrjs[hit_i] < 100_keV) return false;
    auto const & label = event.labels[hit_i];
         if (DSSD::isRing  [label]) m_strips.push_back(m_rings.fill(event, hit_i));
    else if (DSSD::isSector[label]) m_strips.push_back(m_sectors.fill(event, hit_i));
    if (m_strips.back() == nullptr) m_strips.pop_back();
    return true;
  }

  void clear()
  {
    m_rings.clear();
    m_sectors.clear(); 
    m_strips.clear();

    nrj = 0;
    ring_index = 0;
    angle = 0;
    time = 0;
    ok = false;
  }

  /// @brief Simple analysis for single particle interaction. For multiple particles handling capabilities, use buildCells instead
  void analyze()
  {
    if (m_sectors.mult > 2 || m_rings.mult > 2) 
    {
      ok = false;
      return;
    }

    else if (m_rings.mult == 0)
    {
      ok = false;
    }
    else if (m_rings.mult == 1)
    {
      ring_index = m_rings.strips[0]->index();
      nrj = m_rings.strips[0]->nrj;
      ok = true;
    }

    else if (m_rings.mult == 2)
    {
      if (m_rings.strips[0]->isVoisin(m_rings.strips[1]) && m_rings.strips[0]->isCoincident(m_rings.strips[1]))
      {
        ring_index = (m_rings.strips[0]->time > m_rings.strips[1]->time) ? m_rings.strips[0]->index() : m_rings.strips[1]->index();
        nrj = m_rings.strips[0]->nrj + m_rings.strips[1]->nrj;
        ok = true;
      }
      else 
      {
        ok = false;
        return;
      }
    }

    if (ok)
    {
      if (m_sectors.mult == 0)
      {        
        if (found(good_rings, ring_index)) ok = true;
        else ok = false;
      }

      else if (m_sectors.mult == 1)
      {
        nrj = m_sectors.strips[0]->nrj;
        time = m_sectors.strips[0]->time;
        ok = (m_rings.mult > 0); // if no ring, clean energy measure but no angle, so no precise excitation energy measurement possible :
      }
      else if (m_sectors.mult == 2)
      {
        if (m_sectors.strips[0]->isVoisin(m_sectors.strips[1]) && m_sectors.strips[0]->isCoincident(m_sectors.strips[1]))
        {
          nrj = m_sectors.strips[0]->nrj + m_sectors.strips[1]->nrj;
          time = (m_sectors.strips[0]->nrj > m_sectors.strips[1]->nrj) ? m_sectors.strips[0]->time : m_sectors.strips[1]->time;
          ok = true;
        }
        else ok = false;
      }
    }

    if (ok) angle = m_rings[ring_index].angle;
  }

  /// @brief Can handle up to four simultaneous particles. 
  void buildCells(bool addback_sectors = true, bool addback_rings = true)
  {
    if (m_sectors.mult > 4 || m_rings.mult > 8) 
    {
      ok = false;
      return;
    }

    m_cells.clear();

    // DSSD add-back procedure for hits with charge-sharing 
    std::vector<bool> sector_rejected(m_sectors.mult, false); // Neighbor and coincident sectors
    std::vector<bool> ring_rejected(m_rings.mult, false); // Neighbor and coincident rings

    std::vector<float> sector_nrjs;
    std::vector<float> ring_nrjs;
    std::vector<float> sector_times;
    std::vector<float> ring_times;
    std::vector<float> sector_angles;
    std::vector<float> ring_angles;

    // Addback for sectors
    for (int sector_i = 0; sector_i<m_sectors.mult; ++sector_i)
    {
      if (sector_rejected[sector_i]) continue; // If already used in add-back, do not treat twice
      auto const & sector = m_sectors[sector_i];
      sector_nrjs.push_back(sector.nrj);
      sector_times.push_back(sector.time);
      sector_angles.push_back(sector.time);
      auto & nrj = sector_nrjs.back();
      auto & time = sector_times.back();
      // auto & angle = sector_angles.back();
      if (addback_sectors) for (int sector_j = sector_i+1; sector_j<m_sectors.mult; ++sector_j)
      { // Loop over the other sectors of the event :
        auto const & sector_bis = m_sectors[sector_j];
        if (sector.isVoisin(sector_bis)) // If the two sectors i and j are neighbors :
        {
          sector_rejected[sector_j] = true; // Do not treat later
          if (sector.nrj<sector_bis.nrj) time = sector_bis.time; // Bigger pulse means better timing
          nrj += sector_bis.nrj; // Add-back procedure
        }
      }
      // print("sector", sector.nrj, sector.time, sector.angle);
      // pauseCo();
    }

    // Addback for rings
    for (size_t ring_i = 0; ring_i<m_rings.size(); ++ring_i)
    {
      if (ring_rejected[ring_i]) continue; // If already used in add-back, do not treat twice
      auto const & ring = m_rings[ring_i];
      ring_nrjs.push_back(ring.nrj);
      ring_times.push_back(ring.time);
      ring_angles.push_back(ring.angle);
      auto & nrj = ring_nrjs.back();
      auto & time = ring_times.back();
      auto & angle = ring_angles.back();
      if (addback_rings) for (size_t ring_j = ring_i+1; ring_j<m_rings.size(); ++ring_j)
      { // Loop over the other rings of the event :
        auto const & ring_bis = m_rings[ring_j];
        if (ring.isVoisin(ring_bis)) // If the two rings i and j are neighbors :
        {
          ring_rejected[ring_j] = true; // Do not treat later
          if (ring.nrj<ring_bis.nrj) time = ring_bis.time; // Bigger pulse means better timing
          nrj += ring_bis.nrj; // Add-back procedure
          angle = (angle+ring_bis.angle)/2;
        }
      }
      // print("ring", nrj, time, angle);
      // pauseCo();
    }

    std::vector<bool> sector_matched(m_sectors.mult, false); // Sector found its ring match
    std::vector<bool> ring_matched(m_rings.mult, false); // Ring found its sector match

    for (size_t sector_i = 0; sector_i<sector_nrjs.size(); ++sector_i)
    {
      auto const & sector_nrj = sector_nrjs[sector_i];
      for (size_t ring_i = 0; ring_i<ring_nrjs.size(); ++ring_i)
      {
        auto const & ring_nrj = ring_nrjs[ring_i];
        if (abs(sector_nrj-ring_nrj)<200_keV)
        {
          m_cells.emplace_back(CellDSSD(ring_nrj,sector_nrj,ring_times[sector_i],sector_times[ring_i],ring_angles[ring_i],sector_angles[ring_i]));
          sector_matched[sector_i] = true;
          ring_matched[ring_i] = true;
        }
      }
    }

    m_cells.clean_size() = m_cells.size();

    // Random match making for remaining sectors and hits
    for (size_t sector_i = 0; sector_i<sector_nrjs.size(); ++sector_i)
    {
      if (sector_matched[sector_i]) continue;
      for (size_t ring_i = 0; ring_i<ring_nrjs.size(); ++ring_i)
      {
        if (ring_matched[ring_i]) continue;
        m_cells.emplace_back(CellDSSD(ring_nrjs[ring_i],sector_nrjs[sector_i],ring_times[sector_i],sector_times[ring_i],ring_angles[ring_i],sector_angles[ring_i]));
        m_cells.back().random = true;
      }
    }
  }

  auto mult() const {return m_rings.mult + m_sectors.mult;}

  auto const & rings() const {return m_rings;}
  auto const & sectors() const {return m_sectors;}
  auto const & cells() const {return m_cells;}

  std::vector<Index> good_rings = {0,1,2,6,7,10,11,12,13,14};

  double nrj = 0;
  double angle = 0;
  double time = 0;
  Index  ring_index = 0;
  bool   ok = false;

private:
  RingsDSSD   m_rings;
  SectorsDSSD m_sectors;
  CellsDSSD   m_cells;
  std::vector<StripDSSD*> m_strips;
  ExcitationEnergy* m_Ex = nullptr;
};

#endif //SIMPLEDSSD_HPP

/*
    // As many rings and sectors fired
    if (m_sectors.mult == m_rings.mult)
    {
      if (m_sectors.mult == 1 && abs(sector_labels[0].time - ring_labels[0].time))
      {
        ring_index = ring_labels[0];
        nrj = sector_energy[0];
        Ex_p = (*m_Ex)(nrj, ring_index);
      }
      // else; // Maybe to be improved
    }

    if (m_sectors.mult == 2*m_rings.mult)
    {
      if (m_sectors.mult == 1 && abs(int_cast(ring_labels[0]-ring_labels[1])))
      {
        ring_index = ring_labels[0]; // To be improved maybe
        nrj = sector_energy[0];
        Ex_p = ((*m_Ex)(nrj, ring_labels[0]) + (*m_Ex)(nrj, ring_labels[1]))/2;
        // Might want to reject cases when the sum energy of both rings isn't equal to the one of the sector
      }
      // else; // Maybe to be improved
    }

    if (m_sectors.mult == 0)
    { // If the sector is dead then only rings fired
      if (m_rings.mult == 1)
      {
        nrj = ring_energy[0];
        ring_index = ring_labels[0];
        Ex_p = (*m_Ex)(nrj, ring_index);
      }
      else if (m_rings.mult == 2)
      {
        nrj = ring_energy[0]+ring_energy[1];
        ring_index = (ring_energy[0] > ring_energy[1]) ? ring_labels[0] : ring_labels[1];
        Ex_p = (*m_Ex)(nrj, ring_index);
      }
    }
    */