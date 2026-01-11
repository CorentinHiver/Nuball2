#ifndef TIMER_HPP
#define TIMER_HPP

#include <iomanip>
#include <map>
#include <chrono>
#include <unordered_map>

using       hr_clock_t = std::chrono::high_resolution_clock;
using     time_point_t = std::chrono::time_point<hr_clock_t>;
using duration_milli_t = std::chrono::duration<double, std::milli>;

namespace Colib
{
  template <typename T>
  std::string nicer_seconds(T const & time, int nb_decimals = 3)
  {
    T _time = time;
    std::string unit;
    
    if      (time < 1.e-6 ) { _time *= 1.e9 ;  unit = " ns" ;}
    else if (time < 1.e-3 ) { _time *= 1.e6 ;  unit = " us" ;}
    else if (time < 1.    ) { _time *= 1.e3 ;  unit = " ms" ;}
    else if (time < 60.   ) {                  unit = " s"  ;}
    else if (time < 3600. ) { _time /= 60.  ;  unit = " min";}
    else if (time < 86400.) { _time /= 3600.;  unit = " h"  ;}
    else                    { _time /= 86400.; unit = " j"  ;}

    std::stringstream ss;
    ss << std::fixed << std::setprecision(nb_decimals) << _time << unit;
    return ss.str();
  }

  template <typename T>
  std::string nicer_milliseconds(T const & time, int nb_decimals = 3)
  {
    return nicer_seconds(static_cast<T>(time*1e-3), nb_decimals);
  }
};

/**
 * @brief Timer class. Two usages : time elapsed since creation(or last Restart), or time elapsed between each Start() and Stop(). Non thread-safe.
 */
class Timer
{
public:
  Timer() noexcept { Start(); }

  /// @brief Gets the absolute timestamp
  time_point_t Now() const noexcept
  {
    return m_clock.now();
  }

  /// @brief Starts counting the time elapsed until next Stop() call.
  time_point_t const & Start()
  {
    return (m_start = Now());
  }

  /// @brief Starts counting the time elapsed until next Stop() call. Resets the time elapsed counting.
  time_point_t const & Restart()
  {
    d_milli = duration_milli_t::zero();
    return Start();
  }

  /// @brief Stops counting the time, increments m_stop accordingly
  time_point_t const & Stop()
  {
    m_stop = Now();
    d_milli += duration_milli_t(m_stop - m_start);
    return (m_stop);
  }

  ///@brief Print the time since last Restart, in milliseconds
  double Time() const noexcept
  {
    return(duration_milli_t(Now() - m_start).count());
  }

  ///@brief Print the time since last Restart, in the required time unit (ms, s, min, h, j)
  double Time(std::string const & unit) const noexcept
  {
    return Time()/m_units.at(unit);
  }

  ///@brief Print the time since last Restart, in seconds
  double TimeSec() const noexcept
  {
    Now();
    return(duration_milli_t(Now() - m_start).count()/1000.);
  }

  ///@brief Print the time measured between each Start and Stop, in milliseconds
  double TimeElapsed() const noexcept
  {
    return d_milli.count();
  }

  ///@brief Print the time measured between each Start and Stop, in seconds
  double TimeElapsedSec() const noexcept
  {
    return d_milli.count()/1000.;
  }

  ///@brief Print the time since creation or last Restart()
  std::string operator() (int const & precision = 6) const noexcept
  {
    double time = Time();
    std::string unit = "ms";
    
         if (time>86400000.) {time /= 86400000.; unit =   "j";}
    else if (time>3600000. ) {time /=  3600000.; unit =   "h";}
    else if (time>120000.  ) {time /=    60000.; unit = "min";}
    else if (time>1000.    ) {time /=     1000.; unit =   "s";}
    else if (time>0.001    ) {time *=        1.; unit =  "ms";}
    else if (time>1.e-6    ) {time *=     1000.; unit =  "us";}

    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << time << " " << unit;
    return ss.str();
  }

  ///@brief Print the time measured between each Start and Stop
  std::string timeElapsed() const noexcept
  {
    return Colib::nicer_milliseconds(d_milli.count());
  }

private:

  hr_clock_t m_clock;

  time_point_t m_start;
  time_point_t m_stop;

  duration_milli_t d_milli;
  std::unordered_map<std::string, double> m_units = 
  {
    {"ms" , 1.},
    {"s"  , 1000.},
    {"min", 60000.},
    {"h"  , 3600000.},
    {"j"  , 86400000.},
  };
};

std::ostream& operator<<(std::ostream& out, Timer & timer)
{
  out << timer();
  return out;
}

#endif //TIMER_HPP
