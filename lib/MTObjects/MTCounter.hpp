#ifndef MTCOUNTER_H
#define MTCOUNTER_H

#include "MTObject.hpp"

class MTCounter : public MTObject
{
public:
  MTCounter(){}
  operator int() { return m_counter; }
  operator size_t() { return m_counter; }

  size_t const & get() { return m_counter; }
  
  template <typename T>
  operator T() {return static_cast<T>(m_counter);}

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

  template <typename T>
  T operator/(T const & t) {return m_counter/static_cast<T>(t);}

private:
  size_t m_counter = 0;
};

#endif //MTCOUNTER_H
