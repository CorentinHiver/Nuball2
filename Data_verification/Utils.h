#ifndef UTILS_H
#define UTILS_H

// Gates : for floats, the gates are inclusive ; for integers, the gates are exclusive

inline constexpr bool gate(double const & low, double const & energy, double const & high) noexcept {return (low <= energy && energy <= high);}
inline constexpr bool gate(float const & low, float const & energy, float const & high) noexcept {return (low <= energy && energy <= high);}
inline constexpr bool gate(double const & low, float const & energy, double const & high) noexcept {return (float(low) <= energy && energy <= float(high));}
inline constexpr bool gate(int const & low, int const & label, int const & high) noexcept {return (low < label && label < high);}
inline constexpr bool gate(size_t const & low, size_t const & label, size_t const & high) noexcept {return (low < label && label < high);}
inline constexpr bool gate(Time const & low, Time const & time, Time const & high) noexcept {return (low < time && time < high);}
inline constexpr bool gate(Label const & low, Label const & label, Label const & high) noexcept {return (low < label && label < high);}

inline constexpr bool gate_proton(Time const & time) {return gate(-5_ns, time, 5_ns);}
inline constexpr bool gate_deuton(Time const & time) {return gate(0_ns, time, 10_ns);}
inline constexpr bool gate_alpha(Time const & time) {return gate(10_ns, time, 50_ns);}

float smearGe(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj, nrj*((300.0/sqrt(nrj))/100.0));
}

float smearParis(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj, nrj*((300.0/sqrt(nrj))/100.0));
}

template<class T>
constexpr inline T sqrt_c(T const & number) {return sqrt(number);}

std::map<Label, double> calibLaBr = {
{301, 1.00232},
{302, 1.00355},
{303, 0.998705},
{304, 0.998661},
{305, 1.00534},
{306, 1.00412},
{307, 1.00604},
{308, 0.995267},
{309, 1.00071},
{310, 0.999144},
{311, 0.995776},
{312, 0.999043},
{313, 0.999239},
{314, 0.989351},
{315, 1.00009},
{316, 0.995293},
{401, 1.00433},
{402, 0.977221},
{403, 0.980751},
{404, 0.972085},
{405, 0.988895},
{406, 0.967432},
{407, 0.975061},
{408, 0.999042},
{409, 0.989256},
{410, 0.967872},
{411, 0.979239},
{412, 0.959348},
{501, 1.01598},
{502, 1.01273},
{503, 0.997414},
{504, 0.9995},
{505, 1.0099},
{506, 1.01118},
{507, 1.01875},
{508, 1.01079},
{601, 1.09881},
{602, 0.796353},
{603, 0.972897},
{604, 0.822098},
{605, 1.00082},
{606, 0.761883},
{607, 1.01488},
{608, 0.995224},
{609, 0.996967},
{610, 0.981561},
{611, 0.980286},
{612, 0.879495},
{613, 0.990487},
{614, 0.955601},
{615, 0.989463},
{616, 0.749016},
{701, 0.994545},
{702, 1.00487},
{703, 0.997289},
{704, 0.942364},
{705, 0.980697},
{706, 0.992492},
{707, 0.960328},
{708, 0.985715},
{709, 0.970808},
{710, 0.997369},
{711, 0.969776},
{712, 1.00585}
};

#endif //UTILS_H