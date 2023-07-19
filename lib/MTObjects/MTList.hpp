#ifndef MTLIST_H
#define MTLIST_H

#include "MTObject.hpp"
#include "../libCo.hpp"

template<class T>
class MTList
{
public:
  MTList(){}
  MTList(std::vector<T> const & collection) : m_collection(collection) {m_size = m_collection.size();}

  void set(std::vector<T> const & collection) {m_collection = collection; m_size = m_collection.size();}
  typename std::vector<T> const & get() const {return m_collection;}

  bool getNext(T & t);
  bool getNext(T & t, size_t & index);
  void reset() {m_i = 0;}
  void resize(int const & i = 0) {m_i = m_size = i; m_collection.resize(i);}

  size_t const & size() const {return m_size;}
  T const & operator[] (size_t const & i) {return m_collection[i];}

  MTList& operator=(T const & t) 
  {
    resize();
    this->push_back(t);
    return *this;
  }

  auto begin() {return m_collection.begin();}
  auto end() {return m_collection.end();}

  operator std::vector<T>() & {return m_collection;}

  void operator=(std::vector<T> const & collection) {this->set(collection);}

  void push_back(T const & t) {;m_collection.push_back(t); m_size++;;}

  void Print() {print(m_collection);}

private:
  std::vector<T> m_collection;
  size_t m_size = 0;
  size_t m_i = 0;
};

template<class T>
std::ostream& operator<<(std::ostream& cout, MTList<T> const & list)
{
  cout << list.get();
  return cout;
}

template<class T>
inline bool MTList<T>::getNext(T & t)
{
  MTObject::mutex.lock();
  if (m_i<m_size)
  {
    t = m_collection[m_i];
    m_i++;
    MTObject::mutex.unlock();
    return true;
  }
  else
  {
    MTObject::mutex.unlock();
    return false;
  }
}

template<class T>
inline bool MTList<T>::getNext(T & t, size_t & index)
{
  MTObject::mutex.lock();
  if (m_i<m_size)
  {
    t = m_collection[m_i];
    m_i++;
    index = m_i;
    MTObject::mutex.unlock();
    return true;
  }
  else
  {
    MTObject::mutex.unlock();
    return false;
  }
}

#endif //MTLIST_H
