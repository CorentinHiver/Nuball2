#ifndef TIMER_HPP
#define TIMER_HPP

#include <iomanip>
#include <map>
#include <chrono>
#include <unordered_map>

class Timer
{
public:
  using clock_t     = std::chrono::steady_clock;
  using time_point  = clock_t::time_point;
  using duration_ms = std::chrono::duration<double, std::milli>;

  Timer() noexcept
  {
    restart();
  }

  /// Current timestamp
  static time_point now() noexcept
  {
    return clock_t::now();
  }

  /// Start (or resume) timing
  void start() noexcept
  {
    m_start = now();
    m_running = true;
  }

  /// Restart and clear accumulated time
  void restart() noexcept
  {
    m_elapsed = duration_ms::zero();
    m_start   = now();
    m_running = true;
  }

  /// Stop timing and accumulate elapsed duration
  void stop() noexcept
  {
    if (m_running)
    {
      m_elapsed += duration_ms(now() - m_start);
      m_running = false;
    }
  }

  /// Elapsed time since last start/restart (milliseconds)
  double elapsed_ms() const noexcept
  {
    if (m_running)
      return (m_elapsed + duration_ms(now() - m_start)).count();
    return m_elapsed.count();
  }

  /// Elapsed time in seconds
  double elapsed_sec() const noexcept
  {
    return elapsed_ms() * 1e-3;
  }

  /// Elapsed time with unit conversion
  double elapsed(std::string_view unit) const
  {
    return elapsed_ms() / unit_scale(unit);
  }

  /// Human-readable formatting
  std::string format(int precision = 6) const
  {
    double value = elapsed_ms();
    std::string_view unit = "ms";

    if      (value >= 86'400'000.0) { value /= 86'400'000.0; unit = "d";   }
    else if (value >= 3'600'000.0)  { value /= 3'600'000.0;  unit = "h";   }
    else if (value >= 60'000.0)     { value /= 60'000.0;     unit = "min"; }
    else if (value >= 1'000.0)      { value /= 1'000.0;      unit = "s";   }
    else if (value < 1.0)           { value *= 1'000.0;      unit = "µs";  }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << value << ' ' << unit;
    return oss.str();
  }

  /// @brief Deprecated, remove the () where it is used
  std::string operator()() const {return format();}

  friend inline std::ostream& operator<<(std::ostream& os, const Timer& timer)
  {
    return os << timer.format();
  }

private:
  static double unit_scale(std::string_view unit)
  {
    if (unit == "ms")  return 1.0;
    if (unit == "s")   return 1'000.0;
    if (unit == "min") return 60'000.0;
    if (unit == "h")   return 3'600'000.0;
    if (unit == "d")   return 86'400'000.0;

    throw std::invalid_argument("Timer: unknown time unit");
  }

private:
  time_point  m_start{};
  duration_ms m_elapsed{0};
  bool        m_running{false};
};

#endif // TIMER_HPP