#ifndef UTILS_H
#define UTILS_H

inline constexpr bool gate(double const & low, double const & energy, double const & high) noexcept {return (low < energy && energy < high);}
inline constexpr bool gate(Time const & low, Time const & time, Time const & high) noexcept {return (low < time && time < high);}
inline constexpr bool gate(Label const & low, Label const & label, Label const & high) noexcept {return (low < label && label < high);}
inline constexpr bool gate(size_t const & low, size_t const & label, size_t const & high) noexcept {return (low < label && label < high);}

std::unordered_set<Label> CloversV2::blacklist = {46, 55, 69, 70, 80, 92, 97, 122, 129, 142, 163};
std::unordered_map<Label, double> CloversV2::maxE_Ge = 
{
  {25, 12600 }, {26, 13600 }, {27, 10500 }, {28, 7500  }, 
  {31, 11500 }, {32, 11400 }, {33, 8250  }, {34, 9000  }, 
  {37, 11000 }, {38, 11100 }, {39, 11500 }, {40, 11300 }, 
  {43, 12600 }, {44, 11900 }, {45, 11550 }, {46, 9200  }, 
  {49, 14300 }, {50, 12800 }, {51, 13500 }, {52, 12400 }, 
  {55, 5500  }, {56, 5800  }, 
                {68, 7100  }, {69, 15500 }, {70, 9500  },
  {73, 11650 }, {74, 11600 }, {75, 11800 }, {76, 11600 }, 
  {79, 11500 }, {80, 8000  }, {81, 18200 },
  {85, 7700  }, {86, 12000 }, {87, 12000 }, {88, 11600 }, 
  {91, 7900  }, {92, 10000 }, {93, 11500 }, {94, 11000 }, 
  {97, 11400 }, {98, 11400 }, {99, 11250 }, {100, 8900 }, 
  {103, 11400 }, {104, 11600 }, {105, 11600 }, {106, 11500 }, 
  {109, 12800 }, {110, 1800  }, {111, 13000 }, {112, 11300 }, 
  {115, 12800 }, {116, 11500 }, {117, 10500 }, {118, 11400 }, 
  {121, 12400 }, {122, 20000 }, {123, 10700 }, {124, 20000 }, 
  {127, 11600 }, {128, 11700 }, {129, 10000 }, {130, 11200 }, 
  {133, 11200 }, {134, 9350  }, {135, 9400  }, {136, 9500  }, 
  {139, 13200 }, {140, 12400 }, {141, 12900 }, {142, 4500  }, 
  {145, 8200  }, {146, 9600  }, {147, 9100  }, {148, 10900 }, 
  {151, 11900 }, {152, 12200 }, {153, 11300 }, {154, 12000 }, 
  {157, 9110  }, {158, 9120  }, {159, 9110  }, {160, 11700 }, 
  {163, 11000 }, {164, 11600 }, {165, 11600 }, {166, 11600 }, 
};


#endif //UTILS_H