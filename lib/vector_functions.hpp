#ifndef VECTOR_FUNCTIONS_HPP
#define VECTOR_FUNCTIONS_HPP

#include "print.hpp"

template < class T >
std::istringstream & operator >> (std::istringstream & is, std::vector<T>& v)
{
  T t;
  is >> t;
  v.push_back(t);
  return is;
}

template <typename T>
bool push_back_unique(std::vector<T> & vector, T const & t)
{
  if (std::find(std::begin(vector), std::end(vector), t) == std::end(vector))
  {
    vector.push_back(t);
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////
//   CLASS STATIC VECTOR  //
////////////////////////////

/**
 * @brief An efficient container for dynamic arrays with a known and fixed maximum size
 * @attention Prototype, have some memory management issues in some cases ...
*/
template<class T>
class StaticVector
{
public:
  StaticVector() {m_data = new T[m_static_size];}
  StaticVector(std::size_t const & static_size) : m_static_size(static_size) {}
  StaticVector(T const & value, std::size_t const & static_size) {for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = value;}
  StaticVector(StaticVector<T> const & vector) : m_static_size(vector.m_static_size) {*m_data = *(vector.m_data);}
  ~StaticVector()
  {
    if(!m_deleted)
    {
      delete[] m_data;
      m_deleted = true;
    }
    else 
    {
      print("W Static vector double delete, be careful (this is a just a warning message)");
    }
  }

  StaticVector& operator=(StaticVector<T> const & vector)
  {
    if (m_static_size > 0) delete[] m_data;
    m_static_size = vector.m_static_size;
    m_dynamic_size = vector.m_dynamic_size;
    m_data = new T[m_static_size];
    *m_data = *(vector.m_data);
    return *this;
  }

  void resize(std::size_t const & size = 0) {m_dynamic_size = size;}
  void static_resize(std::size_t const & size = 0)
  {
    if (m_static_size) delete[] m_data;
    m_dynamic_size = 0;
    m_static_size = size;
    m_data = new T[m_static_size];
  }

  virtual bool has(T const & t);
  virtual bool has(T & t);

  void push_back(T const & e) {m_data[m_dynamic_size++] = e;}
  void push_back_safe(T const & e)
  {
    if (m_dynamic_size++ < m_static_size) m_data[m_dynamic_size] = e;
    else std::cout << "Capacity StaticVector<" << typeid(T).name() << "," << m_static_size << "> exceeded" << std::endl;
  }
  void push_back_unique(T const & e);

  virtual T* begin(){return m_data;}
  virtual T* end()  {return m_data+m_dynamic_size;}

  auto const & size() const {return m_dynamic_size;}
  T & operator[] (std::size_t const & i) const {return m_data[i];}
  T const & at(std::size_t const & i) const {if (m_dynamic_size < m_static_size) return m_data[i]; else return m_data[0];}
  T* data() {return m_data;}

private:
  T* m_data;
  std::size_t m_dynamic_size = 0;
  std::size_t m_static_size = 0;
  bool m_deleted = false;
};

template<class T>
bool inline StaticVector<T>::has(T const & t)
{
  return (std::find(this -> begin(), this -> end(), t) != this -> end());
}

template<class T>
bool inline StaticVector<T>::has(T & t)
{
  return (std::find(this -> begin(), this -> end(), t) != this -> end());
}

template<class T>
void StaticVector<T>::push_back_unique(T const & t)
{
#ifdef SAFE
  if (!this->has(t)) this -> push_back_safe(t);
#else
  if (!this->has(t)) this -> push_back(t);
#endif //SAFE
}

// /**
//  * @brief TBD
//  * 
//  */
// template<class T, std::size_t  = 0>
// class StaticOrderVector : public StaticVector<T>
// {// Binary search works only with
// public:
//   bool has(T const & t)
//   {
//     return std::binary_search(this -> begin(), this -> end(), t);
//   }
// };

#endif //VECTOR_FUNCTIONS_HPP