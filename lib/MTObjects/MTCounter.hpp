#ifndef MTCOUNTER_H
#define MTCOUNTER_H

#include "MTObject.hpp"

class MTCounter 
{
public:
  MTCounter(){}

  operator int() { return static_cast<int>(m_counter); }
  operator float() {return static_cast<float>(m_counter);}
  operator std::size_t() const & { return m_counter; }
  std::size_t const & get() const { return m_counter; }
  std::size_t const & operator() () const {return m_counter;}

  void operator++() {m_mutex.lock(); m_counter++; m_mutex.unlock();}
  std::size_t& operator++(int) {m_mutex.lock(); m_counter++; m_mutex.unlock();return m_counter;}

  template <typename T>
  void operator+=(T const & t)
  {
    m_mutex.lock();
    m_counter+=static_cast<std::size_t>(t);
    m_mutex.unlock();
  }

  template <typename T>
  void operator=(T const & t)
  {
    m_mutex.lock();
    m_counter = static_cast<std::size_t>(t);
    m_mutex.unlock();
  }

  float operator/(float const & v) {return static_cast<float>(m_counter)/v;}
  float operator*(float const & v) {return static_cast<float>(m_counter)/v;}
  double  operator*(double  const & v) {return static_cast<double >(m_counter)/v;}
  std::size_t operator*(std::size_t  const & v) {return static_cast<std::size_t >(m_counter)/v;}

private:
  std::mutex m_mutex;
  std::size_t m_counter = 0;
};

std::ostream& operator<<(std::ostream& os, MTCounter const & counter)
{
  os << counter.get();
  return os;
}

#endif //MTCOUNTER_H
