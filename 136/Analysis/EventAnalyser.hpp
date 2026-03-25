#pragma once

#include "../../lib/Classes/Event.hpp"
#include "../../lib/Classes/Gate.hpp"
#include "../../lib/Nuball2.hh"

static constexpr inline std::array<Gate_t<Time>, NSI136::Detector::nbTypes> make_prompt_time_gates()
{
  using namespace NSI136::Detector;
  std::array<Gate_t<Time>, nbTypes> g{};
  g[Ge]    = {-15_ns,  15_ns};
  g[BGO]   = {-15_ns,  15_ns};
  g[Paris] = {-10_ns,  10_ns};
  g[DSSD]  = {-20_ns,  50_ns};
  return g;
}

static constexpr inline std::array<Gate_t<Time>, NSI136::Detector::nbTypes> make_delayed_time_gates()
{
  using namespace NSI136::Detector;
  std::array<Gate_t<Time>, nbTypes> g{};

  g[Ge]    = { 40_ns, 160_ns};
  g[BGO]   = { 40_ns, 160_ns};
  g[Paris] = { 40_ns, 160_ns};
  return g;
}

class EventAnalyser
{
public:
  EventAnalyser(Event const & event) : m_event(event) 
  {
    for (int hit_i = 0; hit_i<m_event.mult; ++hit_i)
    {
      auto const & label = m_event.labels[hit_i];
      auto const & time = m_event.times[hit_i];

      auto const & type = NSI136::detectorType[label];

           if (prompt_time_gate [type].isIn(time)) promptHits .push_back(hit_i);
      else if (delayed_time_gate[type].isIn(time)) delayedHits.push_back(hit_i);
    }
  }

  std::vector<int> promptHits;
  std::vector<int> delayedHits;
  
private:
  static constexpr std::array<Gate_t<Time>, NSI136::Detector::nbTypes> prompt_time_gate  = make_prompt_time_gates();
  static constexpr std::array<Gate_t<Time>, NSI136::Detector::nbTypes> delayed_time_gate = make_delayed_time_gates();
 
  Event const & m_event;
};