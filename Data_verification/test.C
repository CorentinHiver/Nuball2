#include "../lib/libRoot.hpp"
#include "../lib/Classes/GatesList.hpp"

void test(){

  static constexpr size_t gate_bin_size = 2; // Take 2 keV
  static constexpr std::array<int, 19> ddd_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991, 1750, 1836, 2115, 1846, 2125}; // keV
  static constexpr std::array<int, 21> dpp_gates = {205, 222, 244, 279, 301, 309, 352, 642, 688, 699, 903, 921, 942, 966, 991, 1279, 1750, 1836, 1846, 2115, 2125}; // keV
  static constexpr std::array<int, 14> ppp_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991}; // keV

  static auto constexpr ddd_gate_bin_max = maximum(ddd_gates)+gate_bin_size+1;
  static auto constexpr ddd_gate_lookup = Colib::LUT<ddd_gate_bin_max> ([](int bin){
    for (auto const & gate : ddd_gates) if (Colib::abs_const(gate-bin)<3) return true;
    return false;
  });
  static auto constexpr ddd_id_gate_lkp = Colib::LUT<ddd_gate_bin_max> ([&](int bin){
    if (ddd_gate_lookup[bin])
    {
      for (int i = 0; i<ddd_gates.size(); ++i) if (Colib::abs_const(ddd_gates[i] - bin)<3) return i;
      print("wierd....");
      return 0;
    }
    else return 0;
  });
for (int i = 0; i<ddd_gate_bin_max; ++i) if (ddd_gate_lookup[i]) print(i, ddd_id_gate_lkp[i]);
GatesList gates(2, 202,300,500);
}

