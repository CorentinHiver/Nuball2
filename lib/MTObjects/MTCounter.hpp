#ifndef MTCOUNTER_H
#define MTCOUNTER_H

#include "MTObject.hpp"

/**
 * @brief Atomic counter with convenient overloaded operators
 * 
 */
class MTCounter 
{
public:
  MTCounter() = default;

  template<typename T>
  MTCounter(T const & value) : m_counter(size_cast(value)) {}

  operator size_t() const { return m_counter.load(); }
  size_t get() const { return m_counter.load(); }

  void operator++() {m_counter.fetch_add(1, std::memory_order_relaxed);} // to do ++counter
  size_t operator++(int) {return m_counter.fetch_add(1, std::memory_order_relaxed);}// to do counter++

  template <typename T>
  void operator+=(T const & t) {m_counter.fetch_add(size_cast(t), std::memory_order_relaxed);}

  template <typename T>
  void operator=(T const & t) {m_counter.store(size_cast(t), std::memory_order_relaxed);}

private:
  std::atomic<size_t> m_counter{0};
};

std::ostream& operator<<(std::ostream& os, MTCounter const & counter)
{
  os << counter.get();
  return os;
}

#endif //MTCOUNTER_H
