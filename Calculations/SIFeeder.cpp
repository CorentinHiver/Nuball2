#include "../lib/libCo.hpp"

std::unordered_map<uint, std::string> elements = {
  {90,"Th"},
  {91,"Pa"},
  {92,"U"},
  {93,"Np"},
  {94,"Pu"},
  {95,"Am"},
  {96,"Cm"},
  {97,"Bk"},
  };

int main()
{
  for (uint Z = 90; Z<98: ++Z) for (uint N = 120; N<140: ++N)
  {
    auto const & element = elements[Z];
    auto const & isotope = element+std::to_string(Z+N);
  }
};