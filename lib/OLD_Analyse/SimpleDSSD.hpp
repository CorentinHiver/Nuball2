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

      if (!DSSD::is[label]) return;

      auto const & index = DSSD::index[label];
      if(DSSD::isRing[label]) 
      {
        m_ringStrips[index].fill(event, hit_i);
        rings.push_back(&m_ringStrips[index]);
      }
      else 
      {
        m_sectStrips[index].fill(event, hit_i);
        sectors.push_back(&m_sectStrips[index]);
      }
      
      ++m_mult;
    }

    void clear() noexcept
    {
      for (auto & ring   : rings)   ring   -> clear();
      for (auto & sector : sectors) sector -> clear();
      rings  .clear();
      sectors.clear();
      m_mult = 0;
    }

    void analyze() noexcept 
    {
    }

    auto const & mult() const noexcept {return m_mult;}

    std::vector<Strip*> rings;
    std::vector<Strip*> sectors;
    
  private:
    std::array<Strip, nb_rings> m_ringStrips;
    std::array<Strip, nb_sectors> m_sectStrips;
    int m_mult = 0;
  };

}

#endif //SIMPLEDSSD_HPP
