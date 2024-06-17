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

  static constexpr double z  = -3.25 ; // cm : Distance DSSD-target
  static constexpr double Ri = 1.58; // cm : Inner radius 
  static constexpr double Re = 4.05; // cm : Outer radius
  static constexpr std::size_t nb_rings = 16;
  static constexpr std::size_t nb_sectors = 32;

  static constexpr double size_ring = (Re-Ri)/nb_rings; // cm : 
  static constexpr inline double angle_ring  (Index const & index) {return atan( (Ri + size_ring*index + size_ring/2.) / z);}
  static constexpr inline double angle_sector(Index const & index) {return index;}// TODO

  static constexpr Time ring_coinc_tw = 60_ns;
};

////////////////
/// CHANNELS ///
////////////////

class StripDSSD
{protected:
  StripDSSD() : m_index(gIndex++), angle(0) {}
  StripDSSD(double const & _angle) : m_index(gIndex++), angle(_angle) {}

public:
  void fill(Event const & event, int const & hit_i)
  {
    nrj = event.nrjs[hit_i];
    time = event.times[hit_i];
  }

  void clear()
  {
    nrj = 0.0;
    time = 0;
  }

  auto const & index() const {return m_index;}

  bool isVoisin(StripDSSD const & other) const {return abs(other.m_index - m_index) == 1;}
  bool isCoincident(StripDSSD const & other) const {return abs(other.time - time) < DSSD::ring_coinc_tw;}
  bool isVoisin(StripDSSD * other) const {return abs(other->m_index - m_index) == 1;}
  bool isCoincident(StripDSSD * other) const {return abs(other->time - time) < DSSD::ring_coinc_tw;}

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
  RingDSSD() : StripDSSD(DSSD::angle_ring(StripDSSD::gIndex)) {}
};

class SectorDSSD : public StripDSSD
{public:
  SectorDSSD() : StripDSSD(DSSD::angle_sector(StripDSSD::gIndex)) {}
};

/////////////
/// FACES ///
/////////////

class StripsDSSD
{public:
  StripsDSSD() noexcept = default;

  void fill(StripDSSD* strip, Index const & index)
  {
    all_id.push_back(index);
    all.push_back(strip);
    ++mult;
  }

  void clear()
  {
    for (auto const & strip : all) strip->clear();
    all_id.clear();
    all.clear();
    mult = 0;
  }

  auto size() const {return all.size();}

  std::vector<Index> all_id;
  std::vector<StripDSSD*> all;
  int mult = 0;
};

class RingsDSSD : public StripsDSSD
{public:
  RingsDSSD() {StripDSSD::gIndex = 0;}
  void fill(Event const & event, int const & hit_i)
  {
    auto const & index = DSSD::index_ring[event.labels[hit_i]];
    m_rings[index].fill(event, hit_i);
    StripsDSSD::fill(&(m_rings[index]), index);
  }

  auto operator[](int const & strip_i) const {return m_rings[strip_i];}

private:
  std::array<RingDSSD, DSSD::nb_rings> m_rings;
};

class SectorsDSSD : public StripsDSSD
{public:
  SectorsDSSD() {StripDSSD::gIndex = 0;}
  void fill(Event const & event, int const & hit_i)
  {
    auto const & id = DSSD::index_sector[event.labels[hit_i]];
    m_sectors[id].fill(event, hit_i);
    StripsDSSD::fill(&(m_sectors[id]), id);
  }
  auto operator[](int const & strip_i) const {return m_sectors[strip_i];}

private:
  std::array<SectorDSSD, DSSD::nb_sectors> m_sectors;
};

////////////////
/// DETECTOR ///
////////////////

class WarsawDSSD
{public:
  WarsawDSSD() noexcept = default;
  WarsawDSSD(ExcitationEnergy* Ex) : m_Ex_p(Ex) {}

  bool fill(Event const & event, int const & hit_i)
  {
    if (event.nrjs[hit_i] < 100_keV) return false;
    auto const & label = event.labels[hit_i];
         if (DSSD::isRing  [label]) m_rings.fill(event, hit_i);
    else if (DSSD::isSector[label]) m_sectors.fill(event, hit_i);
    return true;
  }

  void clear()
  {
    Smult[m_sectors.mult] = false;
    Rmult[m_rings.mult] = false;

    m_rings.clear();
    m_sectors.clear(); 
    m_strips.clear();

    Ex_p = ExcitationEnergy::bad_value;
    dssd_energy = 0;
    ring_index = 0;
    // angle = 0;
    time = 0;
    ok = false;
  }

  void analyze()
  {
    if (m_sectors.mult == 2 || m_rings.mult == 2) 
    {
      ok = false;
      return;
    }

    Smult[m_sectors.mult] = true;
    Rmult[m_rings.mult] = true;

    if (m_rings.mult == 0)
    {
      ok = false;
    }
    if (m_rings.mult == 1)
    {
      ring_index = m_rings.all[0]->index();
      dssd_energy = m_rings.all[0]->nrj;
      ok = true;
    }

    else if (m_rings.mult == 2)
    {
      if (m_rings.all[0]->isVoisin(m_rings.all[1]) && m_rings.all[0]->isCoincident(m_rings.all[1]))
      {
        ring_index = (m_rings.all[0]->time > m_rings.all[1]->time) ? m_rings.all[0]->index() : m_rings.all[1]->index();
        dssd_energy = m_rings.all[0]->nrj + m_rings.all[1]->nrj;
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
        if (m_rings.mult == 0) ok = false;
        else
        {
          Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
          if (m_rings.mult > 0 && ring_index > 4) ok = false; // If only ring(s), very bad energy for all detectors with id > 4 (may be fixed ?)
          else ok = true;
          time = 0;
        }
      }

      else if (m_sectors.mult == 1)
      {
        dssd_energy = m_sectors.all[0]->nrj;
        time = m_sectors.all[0]->time;
        if (m_rings.mult == 0)
        {
          // clean energy measure but no angle, so no precise excitation energy measurement possible
          ok = false;
        }
        else ok = true;
        Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
      }
      else if (m_sectors.mult == 2)
      {
        if (m_sectors.all[0]->isVoisin(m_sectors.all[1]) && m_sectors.all[0]->isCoincident(m_sectors.all[1]))
        {
          dssd_energy = m_sectors.all[0]->nrj + m_sectors.all[1]->nrj;
          Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
          time = (m_sectors.all[0]->nrj > m_sectors.all[1]->nrj) ? m_sectors.all[0]->time : m_sectors.all[1]->time;
          ok = true;
        }
        else ok = false;
      }
    }

    if (ok)
    {
      // angle = m_rings[ring_index].angle;
    }
  }

  auto mult() const {return m_rings.mult + m_sectors.mult;}

  auto const & rings() const {return m_rings;}
  auto const & sectors() const {return m_sectors;}

  double Ex_p = ExcitationEnergy::bad_value;
  double dssd_energy = 0;
  // double angle = 0;
  double time = 0;
  Index  ring_index = 0;
  bool   ok = false;
  bool Smult[32];
  bool Rmult[16];

private:
  RingsDSSD   m_rings;
  SectorsDSSD m_sectors;
  std::vector<StripDSSD*> m_strips;
  ExcitationEnergy* m_Ex_p = nullptr;
};

#endif //SIMPLEDSSD_HPP

/*
    // As many rings and sectors fired
    if (m_sectors.mult == m_rings.mult)
    {
      if (m_sectors.mult == 1 && abs(sector_labels[0].time - ring_labels[0].time))
      {
        ring_index = ring_labels[0];
        dssd_energy = sector_energy[0];
        Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
      }
      // else; // Maybe to be improved
    }

    if (m_sectors.mult == 2*m_rings.mult)
    {
      if (m_sectors.mult == 1 && abs(int_cast(ring_labels[0]-ring_labels[1])))
      {
        ring_index = ring_labels[0]; // To be improved maybe
        dssd_energy = sector_energy[0];
        Ex_p = ((*m_Ex_p)(dssd_energy, ring_labels[0]) + (*m_Ex_p)(dssd_energy, ring_labels[1]))/2;
        // Might want to reject cases when the sum energy of both rings isn't equal to the one of the sector
      }
      // else; // Maybe to be improved
    }

    if (m_sectors.mult == 0)
    { // If the sector is dead then only rings fired
      if (m_rings.mult == 1)
      {
        dssd_energy = ring_energy[0];
        ring_index = ring_labels[0];
        Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
      }
      else if (m_rings.mult == 2)
      {
        dssd_energy = ring_energy[0]+ring_energy[1];
        ring_index = (ring_energy[0] > ring_energy[1]) ? ring_labels[0] : ring_labels[1];
        Ex_p = (*m_Ex_p)(dssd_energy, ring_index);
      }
    }
    */