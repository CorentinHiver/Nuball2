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

  inline auto uniform_int() noexcept
  {
    std::uniform_int_distribution<int> distribution(0, 1);
    return distribution(generator);
  }

  inline auto uniform_int(int const & min, int const & max) noexcept
  {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
  }

  template<class T>
  inline auto uniform_t() noexcept
  {
    std::uniform_real_distribution<T> distribution(0, 1);
    return distribution(generator);
  }

  template<class T>
  inline auto uniform_t(T const & min, T const & max) noexcept
  {
    std::uniform_real_distribution<T> distribution(min, max);
    return distribution(generator);
  }
  
  template<class T>
  inline double gaussian_t(T mean, T stddev) noexcept
  {
    std::normal_distribution<T> distribution(mean, stddev);
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

}

#endif //RANDOM_HPP