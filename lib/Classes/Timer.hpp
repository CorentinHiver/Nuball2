#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using hr_clock_t = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<hr_clock_t>;
// using NOW = std::chrono::high_resolution_clock::now();
using duration_milli_t = std::chrono::duration<double, std::milli>;

class Timer
{
public:
  Timer(){Restart();}

  time_point_t const & Restart()
  {
    return (start = clock.now());
  }

  time_point_t const & Stop()
  {
    d_milli = duration_milli_t(now - start);
    return (stop = clock.now());
  }

  time_point_t const & Now()
  {
    return (now = clock.now());
  }

  double const Time()
  {
    Now();
    return(duration_milli_t(now - start).count());
  }

  double const TimeSec()
  {
    Now();
    return(duration_milli_t(now - start).count()/1000);
  }

  double const TimeElapsed()
  {
    return d_milli.count();
  }

  double const TimeElapsedSec()
  {
    return d_milli.count()/1000;
  }

  double const operator() () {return Time();}

private:

  hr_clock_t clock;

  time_point_t start;
  time_point_t now;
  time_point_t stop;

  duration_milli_t d_milli;

};

#endif //TIMER_H
