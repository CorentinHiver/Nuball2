#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <cstdint>
#include <bit>
#include <chrono>

#if __cplusplus >= 202002L
  #define CPP20
#endif //__cplusplus >= 202002L

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

  inline double fast_uniform() noexcept {
    static thread_local std::minstd_rand generator(std::random_device{}()); // Shadowing the generator previously defined for the other functions
    static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
  }

#ifdef CPP20
  inline float fast_dirty_uniform() noexcept {
    static thread_local uint32_t counter = 1;
  
    // Combine time with counter to vary each call
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint32_t state = static_cast<uint32_t>(now) ^ (counter++);
  
    // Xorshift32
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
  
    uint32_t ieee_float = 0x3F800000 | (state >> 9);
    return std::bit_cast<float>(ieee_float) - 1.0f;
  }
#else // CPP < 20
  inline float fast_dirty_uniform() noexcept {return fast_uniform();}
#endif //CPP20

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