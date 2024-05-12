#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> uniform_random_generator_double(0, 1);

// double random_uniform() {return uniform_random_generator_double(gen);}

namespace randomCo
{
  static thread_local std::mt19937 generator;

  void setSeed(int const & _seed) {generator.seed(_seed);}

  template<class T>
  inline double uniform_t() noexcept
  {
    std::uniform_real_distribution<T> distribution(0, 1);
    return distribution(generator);
  }

  template<class T>
  inline double uniform_t(T const & min, T const & max) noexcept
  {
    std::uniform_real_distribution<T> distribution(min, max);
    return distribution(generator);
  }

  inline double uniform() noexcept
  {
    std::uniform_real_distribution<double> distribution(0, 1);
    return distribution(generator);
  }

  inline double uniform(const double & min, const double & max) noexcept
  {
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
  }

  inline double gaussian(double mean, double stddev) noexcept
  {
    std::normal_distribution<double> distribution(mean, stddev);
    return distribution(generator);
  }

  
  
  /// @brief Same as uniform_t, but always generates the same sequence of random numbers (faster, less random)
  template<class T>
  inline double uniform_t_fast() noexcept
  {
    thread_local static std::uniform_real_distribution<T> distribution(0, 1);
    return distribution(generator);
  }

    /// @brief Same as uniform_t, but always generates the same sequence of random numbers (faster, less random)
  template<class T>
  inline double uniform_t_fast(T const & min, T const & max) noexcept
  {
    thread_local static std::uniform_real_distribution<T> distribution(min, max);
    return distribution(generator);
  }

  /// @brief Same as uniform, but always generates the same sequence of random numbers (faster, less random)
  inline double uniform_fast() noexcept
  {
    thread_local static std::uniform_real_distribution<double> distribution(0, 1);
    return distribution(generator);
  }

  /// @brief Same as uniform, but always generates the same sequence of random numbers (faster, less random)
  inline double uniform_fast(const double & min, const double & max) noexcept
  {
    thread_local static std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
  }

  /// @brief Same as gaussian, but always generates the same sequence of random numbers (faster, less random)
  inline double gaussian_fast(double mean, double stddev) noexcept
  {
    thread_local static std::normal_distribution<double> distribution(mean, stddev);
    return distribution(generator);
  }
}

#endif //RANDOM_HPP