#ifndef SIMPLEDSSD_HPP
#define SIMPLEDSSD_HPP

#include "DSSD.hpp"

namespace DSSD
{
  class Strip
  {
  public:

    Strip() noexcept {}
    double angle = 0;
    double nrj = 0; 
    double time = 0; 

    void fill(Event const & event, int const & hit_i) noexcept
    {
      nrj = event.nrjs[hit_i];
      time = event.times[hit_i];
    }
    
    void clear() noexcept
    {
      angle = 0;
      nrj = 0; 
      time = 0; 
    }
  };

  class Simple
  {
  public:
    Simple(){}
    void fill(Event const & event, int const & hit_i) noexcept
    {
      auto const & label = event[hit_i].label;
      if (!DSSD::is[hit_i]) return;
      auto const & index = DSSD::index[label];
      ((DSSD::isRing[label]) ? rings[index] : sectors[index]).fill(event, hit_i);
    }

    void clear() noexcept
    {
      for (auto & ring : rings) ring.clear();
      for (auto & sector : sectors) sector.clear();
    }

    void analyze() noexcept 
    {
      m_mult = rings.size() + sectors.size();
    }

    auto const & mult() const {return m_mult;}

    std::array<Strip, nb_rings> rings;
    std::array<Strip, nb_sectors> sectors;

  private:
    int m_mult = 0;
  };

}

#endif //SIMPLEDSSD_HPP
