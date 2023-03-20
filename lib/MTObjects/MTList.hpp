#ifndef MTLIST_H
#define MTLIST_H

#include "MTObject.hpp"

template<class T>
class MTList : public MTObject
{
public:
  MTList(){}
  MTList(std::vector<T> const & collection) : m_collection(collection) {m_size = m_collection.size();}

  void set(std::vector<T> const & collection) {m_collection = collection; m_size = m_collection.size();}
  typename std::vector<T> & get() {return m_collection;}

  bool getNext(T & t);
  void Reset() {i = 0;}

  size_t const & size() const {return m_size;}
  T const & operator[] (size_t const & i) {return m_collection[i];}

  auto begin() {return m_collection.begin();}
  auto end() {return m_collection.end();}

  operator std::vector<T>() & {return m_collection;}
  void operator=(std::vector<T> const & collection) {this->set(collection);}

  void push_back(T const & t) {m_collection.push_back(t);}

private:
  std::vector<T> m_collection;
  size_t m_size = 0;
  size_t i = 0;
};

template<class T>
inline bool MTList<T>::getNext(T & t)
{
  local_mutex.lock();
  if (i<m_size)
  {
    t = m_collection[i];
    i++;
    local_mutex.unlock();
    return true;
  }
  else
  {
    local_mutex.unlock();
    return false;
  }
}

#endif //MTLIST_H
