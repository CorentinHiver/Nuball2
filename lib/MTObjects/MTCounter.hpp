#ifndef MTCOUNTER_H
#define MTCOUNTER_H

#include "MTObject.hpp"

class MTCounter : public MTObject
{
public:
  MTCounter(){}
  operator int() { return m_counter; }
  operator size_t() { return m_counter; }

  operator float() {return static_cast<float>(m_counter);}
  size_t const & get() const { return m_counter; }
  size_t const & operator() () const  {return m_counter;}

  void operator++() {m_mutex.lock(); m_counter++; m_mutex.unlock();}
  size_t& operator++(int) {m_mutex.lock(); m_counter++; m_mutex.unlock();return m_counter;}

  template <typename T>
  void operator+=(T const & t)
  {
    m_mutex.lock();
    m_counter+=static_cast<size_t>(t);
    m_mutex.unlock();
  }

  template <typename T>
  void operator=(T const & t)
  {
    m_mutex.lock();
    m_counter = static_cast<size_t>(t);
    m_mutex.unlock();
  }

  Float_t operator/(Float_t const & v) {return static_cast<Float_t>(m_counter)/v;}
  Float_t operator*(Float_t const & v) {return static_cast<Float_t>(m_counter)/v;}
  double  operator*(double  const & v) {return static_cast<double >(m_counter)/v;}
  size_t  operator*(size_t  const & v) {return static_cast<size_t >(m_counter)/v;}

private:
  size_t m_counter = 0;
};

std::ostream& operator<<(std::ostream& os, MTCounter const & counter)
{
  os << counter.get();
  return os;
}

#endif //MTCOUNTER_H
