#pragma once
#include "../libCo.hpp"

/**
 * @brief Convenient gate class
 * 
 * @tparam T: Type of the data
 */
template<typename T>
class Gate_t
{
public:
  constexpr Gate_t() noexcept = default;
  constexpr Gate_t(T s, T e) noexcept : start(s), stop(e) {}
  constexpr Gate_t(std::initializer_list<T> inputs) 
  {
    if (inputs.size() == 2) 
    {
      auto it = inputs.begin();
      start = *it;
      stop = *(++it);
    }
    // else Colib::throw_error("in Gate(", inputs, ") : input size > 2 !!");
  }
  // void operator= (std::pair <T,T> const & gate) {start = gate.first; stop = gate.second;}
  // void operator= (Gate_t const & timegate) {start = timegate.start; stop = timegate.stop;}
  constexpr Gate_t& operator= (Gate_t const & timegate) noexcept = default;

  [[nodiscard]] constexpr bool operator() (T const & e) const {return (e>start && e<stop);}
  [[nodiscard]] constexpr bool isIn      (T const & e) const {return (e>start && e<stop);}
  T start = 0.;
  T stop = 0.;
  constexpr void use(bool const & _use = true) {m_use = _use;}
  [[nodiscard]] constexpr bool const & used() const {return m_use;}
  friend std::ostream& operator<<(std::ostream& out, Gate_t const & gate)
  {
    out << gate.start << " " << gate.stop;
    return out;
  }
private:
  bool m_use = false;
};

/**
 * @brief Gate with mostly used type of data
 */
using Gate = Gate_t<float>;

template<typename T>
class Gates_t
{
public:
  constexpr Gates_t() noexcept = default;
  constexpr Gates_t(std::initializer_list<T> bounds) : m_size(bounds.size()/2)
  {
    check(bounds);
    m_gates.reserve(m_size); // Efficiency: reserve space
    
    bool low = true;
    T s;
    for (auto const & bound : bounds)
    {
      if (low) s = bound;
      else     m_gates.push_back({s, bound});
      low = !low;
    }
  }
  // C++17: Marked const so it can be used in read-only contexts
  [[nodiscard]] constexpr bool isIn(T const & t) const
  {
    for (std::size_t i = 0; i<m_size; i++)
    {
      if (t>m_gates[i].first && t<m_gates[i].second) return true;
    }
    return false;
  }
  [[nodiscard]] constexpr bool operator() (T const & t) const {return isIn(t);}
  void check(std::initializer_list<T> bounds)
  {
    if (bounds.size()%2 == 1) Colib::throw_error(" Gates Initialiser must have an even number of bounds");
  }
  
  [[nodiscard]] std::size_t const & size() const {return m_size;}
private:
  std::vector<std::pair<T,T>> m_gates;
  std::size_t m_size = 0;
};

using Gates = Gates_t<float>;