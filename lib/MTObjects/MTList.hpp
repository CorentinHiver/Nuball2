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

  MTVector(std::initializer_list<T> const & collection) : 
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

  bool getNext(T & t) noexcept;
  bool getNext(T & t, size_t & index);
  bool operator>>(T & t) {return getNext(t);}
  void resetRead() {m_read_index = 0;}
  auto & getIndex() {return m_read_index;}
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
    this -> clear();
    this -> push_back(t);
    return *this;
  }

  // auto end() {return m_collection.end();} // Not thread safe
  // auto begin() {return m_collection.begin();} // Not thread safe

  operator std::vector<T>() & {return m_collection;}

  void operator=(std::vector<T> const & collection) {this->set(collection);}

  void push_back(T const & t) {m_collection.push_back(t); m_size++;;}

  void Print() 
  {
    lock_mutex lock(m_mutex);
    print(m_collection);
  }
// Shared lock object to manage the mutex for direct range-based for loops
    struct SharedLock {
        SharedLock(std::mutex& mtx) : m_mutex(mtx) {
            m_mutex.lock();
        }
        ~SharedLock() {
            m_mutex.unlock();
        }
        std::mutex& m_mutex;
    };

    // Custom iterator for thread-safe direct iteration
    class LockingIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        LockingIterator(std::shared_ptr<SharedLock> lock, typename std::vector<T>::iterator it)
            : m_lock(lock), m_it(it) {}

        reference operator*() const { return *m_it; }
        pointer operator->() const { return &(*m_it); }

        LockingIterator& operator++() {
            ++m_it;
            return *this;
        }

        LockingIterator operator++(int) {
            LockingIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const LockingIterator& other) const {
            return m_it == other.m_it;
        }

        bool operator!=(const LockingIterator& other) const {
            return !(*this == other);
        }

    private:
        std::shared_ptr<SharedLock> m_lock;
        typename std::vector<T>::iterator m_it;
    };

    // Thread-safe begin() and end() for direct range-based for loops
    LockingIterator begin() {
        m_iteration_lock = std::make_shared<SharedLock>(m_mutex);
        return LockingIterator(m_iteration_lock, m_collection.begin());
    }

    LockingIterator end() {
        if (!m_iteration_lock) {
            throw std::runtime_error("end() called before begin()");
        }
        return LockingIterator(m_iteration_lock, m_collection.end());
    }

private:
  mutable std::mutex m_mutex;
  std::vector<T> m_collection;
  size_t m_size = 0;
  size_t m_read_index = 0;
  mutable std::shared_ptr<SharedLock> m_iteration_lock; // For direct iteration locking
};

using MTList = MTVector<std::string>;

template<class T>
std::ostream& operator<<(std::ostream& cout, MTVector<T> const & list)
{
  cout << list.get();
  return cout;
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

#endif //MTLIST_H
