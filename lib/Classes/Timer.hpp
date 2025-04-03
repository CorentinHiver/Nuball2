#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using hr_clock_t = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<hr_clock_t>;
using duration_milli_t = std::chrono::duration<double, std::milli>;

template <typename T>
std::string nicer_seconds(T const & time, int nb_decimals = 3)
{
  T _time = time;
  std::string unit;
        if (_time<120.  )  {             ; unit = " s"  ;}
  else if (_time<3600. )  {_time/=60.   ; unit = " min";}
  else if (_time<86400.)  {_time/=3600. ; unit = " h"  ;}
  else                    {_time/=86400.; unit = " j"  ;}

  if (unit == "s") nb_decimals = 0;
  std::stringstream ss;
  ss << std::fixed << std::setprecision(nb_decimals) << _time << unit;
  return ss.str();
}

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

  auto Time()
  {
    Now();
    return(duration_milli_t(m_now - m_start).count());
  }

  auto TimeSec()
  {
    Now();
    return(duration_milli_t(m_now - m_start).count()/1000.);
  }

  auto Time(std::string const & unit)
  {
    if (!found(m_units, unit)) {print("in Timer::Time(string unit) : unit", unit, "unkown... ms by default"); return Time();}
    return Time()/m_units[unit];
  }

  auto TimeElapsed()
  {
    return d_milli.count();
  }

  auto TimeElapsedSec()
  {
    return d_milli.count()/1000.;
  }

  auto operator() (int const & precision = 6)
  {
    double time = Time();
    m_unit = "ms";

    if (time>3600000.) {time/=3600000.; m_unit = "h";}
    else if (time>120000.){time/=60000.; m_unit = "min";}
    else if (time>1000.){time/=1000.; m_unit = "s";}

    std::stringstream ss;
    ss << std::setprecision(precision) << time << " " << m_unit;

    return ss.str();
  }

  std::string unit() {(*this)(); return m_unit;}
  
  double Unit()
  {
    return m_units[m_unit];
  }

private:

  hr_clock_t m_clock;

  time_point_t m_start;
  time_point_t m_now;
  time_point_t m_stop;

  duration_milli_t d_milli;
  std::string m_unit = "ms";
  std::map<std::string, double> m_units = 
  {
    {"ms" , 1.},
    {"s"  , 1000.},
    {"min", 60000.},
    {"h"  , 3600000.}
  };
};

std::ostream& operator<<(std::ostream& out, Timer & timer)
{
  out << timer();
  return out;
}

#endif //TIMER_H
