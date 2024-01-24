#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> uniform_random_generator_double(0, 1);

// double random_uniform() {return uniform_random_generator_double(gen);}


int intRand(const int & min, const int & max) 
{
  static thread_local std::mt19937 generator;
  std::uniform_int_distribution<int> distribution(min,max);
  return distribution(generator);
}

double random_uniform() 
{
  static thread_local std::mt19937 generator;
  std::uniform_real_distribution<double> distribution(0,1);
  return distribution(generator);
}

#endif //RANDOM_HPP