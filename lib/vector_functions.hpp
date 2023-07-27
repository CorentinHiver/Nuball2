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
  StaticVector() = default;
  StaticVector(std::size_t const & static_size) : m_static_size(static_size) {create();}
  StaticVector(T const & value, std::size_t const & static_size) : m_static_size(static_size) {create(); for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = value;}
  StaticVector(StaticVector<T> const & vector) : 
    m_static_size(vector.m_static_size),
    m_dynamic_size(vector.m_dynamic_size),
    m_deleted(vector.m_deleted)
  { create(); *m_data = *(vector.m_data); }
    
  ~StaticVector()
  {
    if(!m_deleted)
    {
      delete[] m_data;
      m_deleted = true;
    }
    else 
    {
      print("W: StaticVector double delete, be careful (this is a just a warning message)");
    }
  }

  /// @brief Deletes the underlying data
  void deallocate ()
  {
    if (m_static_size > 0) 
    {
      delete[] m_data;
      m_static_size = m_dynamic_size = 0;
    }
  }

  /// @brief Copy the values of another vector
  StaticVector& operator=(StaticVector<T> const & vector)
  {
    if (vector.m_static_size == m_static_size && m_static_size == 0) return *this;
    else
    {
      if (m_static_size > 0) delete[] m_data;
      m_static_size = vector.m_static_size;
      m_data = new T[m_static_size];
    }

    m_dynamic_size = vector.m_dynamic_size;
    *m_data = *(vector.m_data);
    return *this;
  }

  /// @brief Only reset the cursor to 0. Do not touch the data. Used for performance.
  void resize(std::size_t const & size = 0) {m_dynamic_size = size;}

  /// @brief Delete memory, reset the cursor to 0 and allocate new size memory
  void static_resize(std::size_t const & size = 0)
  {
    if (m_static_size) delete[] m_data;
    m_dynamic_size = 0;
    m_static_size = size;
    m_data = new T[m_static_size];
  }

  /// @brief Does the vector contain t ?
  /// @param t: variable in read-only mode
  virtual bool has(T const & t);

  /// @brief Does the vector contain t ?
  /// @param t: direct access to the variable
  virtual bool has(T & t);

  /// @brief Add element to the back of the vector
  void push_back(T const & e) {m_data[m_dynamic_size++] = e;}

  /// @brief Add element to the back of the vector and makes sure the memory is allocated
  void push_back_safe(T const & e)
  {
    if (m_dynamic_size++ < m_static_size) m_data[m_dynamic_size] = e;
    else std::cout << "Capacity of StaticVector<" << typeid(T).name() << "> with size " << m_static_size << " exceeded" << std::endl;
  }

  /// @brief Add element to the back of the vector only if the vector do not contain it
  void push_back_unique(T const & e);

  /// @brief Return iterator to the beginning of the vector
  virtual T* begin(){return m_data;}

  /// @brief Return iterator to the end of the vector
  virtual T* end()  {return m_data+m_dynamic_size;}

  /// @brief Return the position of the write cursor
  auto const & size() const {return m_dynamic_size;}

  /// @brief Return the ith element 
  T & operator[] (std::size_t const & i) const {return m_data[i];}

  /// @brief Return the ith element and check i do not exceed the size of the vector
  T const & at(std::size_t const & i) const {if (i < m_static_size) return m_data[i]; else return m_data[0];}

  /// @brief Return a pointer to the underlying data
  T* data() {return m_data;}

private:
  /// @brief To be used only by the constructor
  void create() { m_data = new T[m_static_size]; }

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