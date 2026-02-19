#pragma once

#include <cstdint>
#include <chrono>
#include <random>
#include <thread>

#ifndef cpp20
#define cpp20 (__cplusplus >= 202002L)
#endif //pp20

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

#ifdef CPP20
  /// @brief This is NOT true pseudo-random generation, but works like a charm for ADC to float convertion
  /// @tparam n_pow_size MUST be < 20 to keep pleasant compilation time 
  template<size_t n_pow_size = 16>
  requires(8 <= n_pow_size && n_pow_size < 20)
  inline float ultra_fast_uniform() noexcept 
  {
    // 1. Compile-time constants
    static constexpr size_t size = 1ULL << n_pow_size;
    static constexpr size_t mask = size - 1;
    static constexpr float step = 1.0f / static_cast<float>(size);

    // 2. Compile-time Table Generation
    static constexpr auto lut = []() 
    {
      std::array<float, size> table{};
      
      // Fill with perfectly granular steps: 0, 1/size, 2/size...
      for (size_t i = 0; i < size; ++i) table[i] = i * step;

      // Shuffle using a constexpr-friendly PRNG (Fisher-Yates)
      uint32_t state = 0x12345;
      for (size_t i = size - 1; i > 0; --i) 
      {
          // Simple LCG for shuffle
          state = state * 1664525u + 1013904223u;
          size_t j = state % (i + 1);
          
          float temp = table[i];
          table[i] = table[j];
          table[j] = temp;
      }
      return table;
    }();

    // 3. Runtime Access (Initialized once, modified only in following line with index++
    static thread_local size_t index = static_cast<size_t>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());

    return lut[(++index) & mask];
  }
#endif //CPP20
}