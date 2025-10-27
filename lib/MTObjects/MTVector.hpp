#ifndef MTVECTOR_HPP
#define MTVECTOR_HPP

#include "MTObject.hpp"
#include <vector>

/**
 * @brief Thread-safe vector
 * Operator[] IS NOT thread safe
 * Range-base loop IS NOT thread safe
 * getNext IS thread-safe
 * operator>> IS thread-safe
 * 
 */
template<class T>
class MTVector
{
public:
  MTVector() = default;
  MTVector(std::vector<T> const & collection) : 
    m_collection(collection), 
    m_size(m_collection.size()) {}

  MTVector(std::initializer_list<T> const & collection) : 
    m_collection(collection), 
    m_size(m_collection.size()) {}

  auto const & get() const {return m_collection;}

  bool getNext(T & t) noexcept;
  bool getNext(T & t, size_t & index);
  bool operator>>(T & t) {return getNext(t);}
    
  size_t const & size() const {return m_size;}
  T const & operator[] (size_t const & i) {return m_collection[i];}

  void resetRead() {m_read_index = 0;}
  auto & getIndex() {return m_read_index;}
  void clear() {m_collection.clear();}
  void resize(size_t const & i = 0);
  MTVector& operator=(T const & t) 
  {
    lock_mutex lock(m_mutex);
    this -> clear();
    this -> push_back(t);
    return *this;
  }

  void operator=(std::vector<T> const & collection) {this->set(collection);}

  void push_back(T const & t) {m_collection.push_back(t); m_size++;;}

  void Print() 
  {
    lock_mutex lock(m_mutex);
    print(m_collection);
  }

  void set(std::vector<T> const & collection)
  {
    lock_mutex lock(m_mutex);
    m_collection = collection; 
    m_size = m_collection.size();
    resetRead();
  }


  operator std::vector<T>() & {return m_collection;}

  // Thread-safe begin() and end() for direct range-based for loops
  // TODO if not too difficult (chatGPT and leChat coudln't do it easily (19/10/25))
  // using LockIt = Colib::LockingIterator<T>;
  // using SharedLock = Colib::SharedLock;

  // LockIt begin() 
  // {
  //   m_iteration_lock = SharedLock::create(m_mutex);
  //   return LockIt(m_iteration_lock, m_collection.begin());
  // }

  // LockIt end() 
  // {
  //   if (!m_iteration_lock) {throw std::runtime_error("MTList::end() called before MTList::begin()");}
  //   return LockIt(m_iteration_lock, m_collection.end());
  // }

  friend std::ostream& operator<<(std::ostream& cout, MTVector<T> const & list)
  {
    cout << list.get();
    return cout;
  }

private:
  mutable std::mutex m_mutex;
  std::vector<T> m_collection;
  size_t m_size = 0;
  size_t m_read_index = 0;
  // mutable std::shared_ptr<SharedLock> m_iteration_lock; // For direct iteration locking
};

template<class T>
void MTVector<T>::resize(size_t const & i) 
{
  lock_mutex lock(m_mutex);
  m_size = i; 
  m_collection.resize(i);
  resetRead();
}

template<class T>
inline bool MTVector<T>::getNext(T & t) noexcept
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
    index = ++m_read_index;
    return true;
  }
  return false;
}

#endif //MTVECTOR_HPP
