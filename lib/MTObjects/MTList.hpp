#ifndef MTLIST_H
#define MTLIST_H

#include "MTObject.hpp"
#include "../libCo.hpp"

template<class T>
class MTVector
{
public:
  MTVector() = default;
  MTVector(std::vector<T> const & collection) : 
    m_collection(collection), 
    m_size(m_collection.size()) {}

  void set(std::vector<T> const & collection) 
  {
    lock_mutex lock(m_mutex);
    m_collection = collection; 
    m_size = m_collection.size();
    resetRead();
  }

  typename std::vector<T> const & get() const {return m_collection;}

  bool getNext(T & t);
  bool getNext(T & t, size_t & index);
  void resetRead() {m_read_index = 0;}
  void clear() {m_collection.clear();}
  void resize(size_t const & i = 0) 
  {
    lock_mutex lock(m_mutex);
    m_size = i; 
    m_collection.resize(i);
    resetRead();
  }
    
  size_t const & size() const {return m_size;}
  T const & operator[] (size_t const & i) {return m_collection[i];}

  MTVector& operator=(T const & t) 
  {
    lock_mutex lock(m_mutex);
    resize();
    this->push_back(t);
    return *this;
  }

  auto begin() {return m_collection.begin();}
  auto end() {return m_collection.end();}

  operator std::vector<T>() & {return m_collection;}

  void operator=(std::vector<T> const & collection) {this->set(collection);}

  void push_back(T const & t) {m_collection.push_back(t); m_size++;;}

  void Print() 
  {
    lock_mutex lock(m_mutex);
    print(m_collection);
  }

private:
  mutable std::mutex m_mutex;
  std::vector<T> m_collection;
  size_t m_size = 0;
  size_t m_read_index = 0;
};

using MTList = MTVector<std::string>;

template<class T>
std::ostream& operator<<(std::ostream& cout, MTVector<T> const & list)
{
  cout << list.get();
  return cout;
}

template<class T>
inline bool MTVector<T>::getNext(T & t)
{
  lock_mutex lock(m_mutex);
  if (m_read_index<m_size)
  {
    t = m_collection[m_read_index];
    ++m_read_index;
    return true;
  }
  return false;
}

template<class T>
inline bool MTVector<T>::getNext(T & t, size_t & index)
{
  lock_mutex lock(m_mutex);
  if (m_read_index<m_size)
  {
    t = m_collection[m_read_index];
    index = m_read_index++;
    return true;
  }
  return false;
}

#endif //MTLIST_H
