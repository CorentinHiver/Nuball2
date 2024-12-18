#ifndef GATE_HPP
#define GATE_HPP

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
  Gate_t(){}
  Gate_t(std::initializer_list<T> inputs) 
  {
    if (inputs.size() == 2) 
    {
      auto it = inputs.begin();
      start = *it;
      stop = *(++it);
    }
  }

  void operator= (std::pair <T,T> const & gate) {start = gate.first; stop = gate.second;}
  void operator= (Gate_t const & timegate) {start = timegate.start; stop = timegate.stop;}

  bool operator() (T const & e) {return (e>start && e<stop);}
  bool isIn       (T const & e) {return (e>start && e<stop);}

  T start = 0.;
  T stop = 0.;

  void use(bool const & _use = true) {m_use = _use;}
  bool const & used() const {return m_use;}

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
  Gates_t(){}
  Gates_t(std::initializer_list<T> bounds) : m_size(bounds.size()/2)
  {
    check(bounds);
    bool low = true;
    for (auto const & bound : bounds)
    {
      if (low) start.push_back(bound);
      else     stop .push_back(bound);
      low = !low;
    }
  }

  bool isIn(T const & t) 
  {
    for (std::size_t i = 0; i<m_size; i++)
    {
      if (t>start[i] && t<stop[i]) return true;
    }
    return false;
  }

  bool operator() (T const & t) const {return isIn(t);}

  void check(std::initializer_list<T> bounds)
  {
    if (bounds.size()%2 == 1) throw_error(" Gates Initialiser must have an even number of bounds (one lower and one higher bound)");
  }
  std::size_t const & size() const {return m_size;}

private:
  std::vector<T> start;
  std::vector<T> stop ;
  std::size_t m_size = 0;
};

using Gates = Gates_t<float>;

#endif //GATE_HPP
