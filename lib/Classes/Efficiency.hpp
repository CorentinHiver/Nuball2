#ifndef EFFICIENCY_HPP
#define EFFICIENCY_HPP
#include "../libRoot.hpp"

class Efficiency
{
public:
  Efficiency(std::string const & filename, std::string options = "ascii")
  {
    if (options == "ascii")
    {
      std::ifstream file(filename, std::ios::in);
      int energy = 0; double value = 0;
      while (file >> energy >> value) m_data.push_back(value);
      m_max = maximum(m_data);
    }
  }
  auto const & operator[](double const & energy) {return m_data[int_cast(energy)];}
  auto const & normalizedValue(double const & energy) {return m_data[int_cast(energy)]/m_max;}
  auto const & normalizedCounts(int const & counts, double const & energy) {return counts*normalizedValue(energy);}

private:
  std::vector<double> m_data;
  double m_max = 0.0;
};

#endif //EFFICIENCY_HPP