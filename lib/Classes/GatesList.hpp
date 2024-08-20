#ifndef GATESLIST_HPP
#define GATESLIST_HPP

template<size_t n>
class GatesList
{
public:
  explicit GatesList(int gate_bin_size = 2, std::array<int, n> const & array) 
  {
    static_assert(are_all_arithmetic<T...>::value, "All types must be arithmetic.");
    get_size<T...>();
  }
};

#endif //GATESLIST_HPP