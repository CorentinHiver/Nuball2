#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> uniform_random_generator_double(0, 1);

// double random_uniform() {return uniform_random_generator_double(gen);}


static thread_local std::mt19937 generator;

template<class T>
double random_uniform_t()
{
  std::uniform_real_distribution<T> distribution(0, 1);
  return distribution(generator);
}

template<class T>
double random_uniform_t(T const & min, T const & max)
{
  std::uniform_real_distribution<T> distribution(0, 1);
  return distribution(generator);
}

double random_uniform()
{
  std::uniform_real_distribution<double> distribution(0, 1);
  return distribution(generator);
}

double random_uniform(const double & min, const double & max)
{
  std::uniform_real_distribution<double> distribution(min, max);
  return distribution(generator);
}

double random_gaussian(double mean, double stddev) 
{
  std::normal_distribution<double> distribution(mean, stddev);
  return distribution(generator);
}

#endif //RANDOM_HPP