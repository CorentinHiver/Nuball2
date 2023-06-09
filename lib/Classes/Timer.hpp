#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using hr_clock_t = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<hr_clock_t>;
using duration_milli_t = std::chrono::duration<double, std::milli>;

class Timer
{
public:
  Timer(){Restart();}

  time_point_t const & Restart()
  {
    return (m_start = m_clock.now());
  }

  time_point_t const & Now()
  {
    return (m_now = m_clock.now());
  }

  time_point_t const & Stop()
  {
    d_milli = duration_milli_t(Now() - m_start);
    return (m_stop = m_clock.now());
  }

  float Time()
  {
    Now();
    return(static_cast<float>(duration_milli_t(m_now - m_start).count()));
  }

  float TimeSec()
  {
    Now();
    return(duration_milli_t(m_now - m_start).count()/1000.);
  }

  float TimeElapsed()
  {
    return d_milli.count();
  }

  float TimeElapsedSec()
  {
    return d_milli.count()/1000.;
  }

  float operator() ()
  {
    float time = Time();
    m_unit = "ms";

    if (time>1000.) { time/=1000.; m_unit = "s";}
    else { return time; }

    if (time>120.) { time/=60.; m_unit = "min";  }
    else {  return time; }

    if (time>120.) { time/=60.; m_unit = "h"; return time;}
    else { return time; }
  }

  std::string unit() {(*this)(); return m_unit;}
  
  float Unit()
  {
         if (m_unit == "ms") return 1.;
    else if (m_unit ==  "s") return 1000.;
    else if (m_unit =="min") return 60000.;
    else if (m_unit ==  "h") return 3600000.;
    else return 0.;
  }

private:

  hr_clock_t m_clock;

  time_point_t m_start;
  time_point_t m_now;
  time_point_t m_stop;

  duration_milli_t d_milli;
  std::string m_unit = "ms";
};

#endif //TIMER_H
