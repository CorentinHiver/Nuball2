#ifndef MTCOUNTER_H
#define MTCOUNTER_H

#include "MTObject.hpp"

/**
 * @brief Atomic counter with convenient overloaded operators
 * @details
 * All the setter overloaded operators are std::memory_order_relaxed
 * All the setter methods are std::memory_order_seq_cst 
 */
class MTCounter 
{
public:
  MTCounter() = default;

  template<typename T, typename check_T_is_number = T_is_number<T>>
  MTCounter(T const & value) : m_counter(size_cast(value)) {}

  operator size_t() const { return m_counter.load(); }
  operator float() const { return float_cast(m_counter.load()); }
  size_t get() const { return m_counter.load(); }

  size_t operator++()     {return m_counter.fetch_add(1, std::memory_order_relaxed);} // to do counter++
  size_t operator++(int)  {return (m_counter.fetch_add(1, std::memory_order_relaxed)+1);}// to do ++counter
  size_t incr()           {return m_counter.fetch_add(1, std::memory_order_seq_cst)+1;}
  size_t increment()      {return m_counter.fetch_add(1, std::memory_order_seq_cst)+1;}
  
  size_t operator--()     {return m_counter.fetch_sub(-1, std::memory_order_relaxed);} // to do counter--
  size_t operator--(int)  {return m_counter.fetch_sub(-1, std::memory_order_relaxed)-1;}// to do --counter
  size_t decr()           {return m_counter.fetch_sub(-1, std::memory_order_seq_cst)-1;}
  size_t decrement()      {return m_counter.fetch_sub(-1, std::memory_order_seq_cst)-1;}

  template <typename T, typename check_T_is_number = T_is_number<T>>
  void add(T const & t) {return m_counter.fetch_add(size_cast(t), std::memory_order_seq_cst);}

  template <typename T, typename check_T_is_number = T_is_number<T>>
  void operator+=(T const & t) {m_counter.fetch_add(size_cast(t), std::memory_order_relaxed);}

  template <typename T, typename check_T_is_number = T_is_number<T>>
  void operator=(T const & t) {m_counter.store(size_cast(t), std::memory_order_relaxed);}

private:
  std::atomic<ulonglong> m_counter{0};
};

std::ostream& operator<<(std::ostream& os, MTCounter const & counter)
{
  os << counter.get();
  return os;
}

#endif //MTCOUNTER_H
