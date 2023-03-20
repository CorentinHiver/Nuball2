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

  void operator++() {local_mutex.lock(); m_counter++; local_mutex.unlock();}
  size_t& operator++(int) {local_mutex.lock(); m_counter++; local_mutex.unlock();return m_counter;}
  void operator+=(size_t const & t) {local_mutex.lock(); m_counter+=t; local_mutex.unlock();}
  void operator+=(int const & t) {local_mutex.lock(); m_counter+=t; local_mutex.unlock();}

  size_t operator/(size_t const & t) {return m_counter/t;}

private:
  size_t m_counter = 0;
};

#endif //MTCOUNTER_H
