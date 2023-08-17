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
 * @details
 * This class is meant to handle a vector of data that needs to be resized a lot.
 * To do so, declare it this way :
 * 
 *      static_vector<T> my_vec = static_vector<T>(maximum_size);
 * 
 * If not in an object prototype, simply :
 *      
 *      auto my_vec = static_vector<T>(maximum_size);
 * 
 * You can fill the whole vector with some value :
 *  
 *      auto my_vec = static_vector<T>(maximum_size, fill_value);
 *      // or :
 *      my_vec.fill(fill_value);
 * 
 * Now, you can use this vector just like a regular std::vector : 
 * 
 *      my_vec.push_back(t);
 *      my_vec.push_back(t2);
 *      my_vec.push_back(t3);
 *      // Do some stuff
 *      my_vec.resize(0);
 * 
 * @attention keep in mind you cannot exceed the capacity of the vector.
 * 
 * If you want not to crash you application if the capacity is reached, use push_back_safe instead.
 * 
 * An interesting feature is push_back_unique(t). This allows one to push_back t only if it has 
 * not been found in the vector. It may require t to have a comparison operator (not tested yet).
 * 
 * Now, if for some reason you want to modify the capacity of the vector, you can use static_resize(new_size).
*/
template<class T>
class StaticVector
{
public:
  StaticVector() = default;

  /// @brief Create a new Static_vector with size static_size
  StaticVector(std::size_t const & static_size) : m_static_size(static_size) {reserve();}

  /// @brief Create a new Static_vector with size static_size and fill it with element e
  StaticVector(std::size_t const & static_size, T const & e) : m_static_size(static_size) 
  {
    reserve(); 
    fill_static(e);
  }

  /// @brief Create a new Static_vector by copy (duplicate)
  StaticVector(StaticVector<T> const & vector) : 
    m_static_size(vector.m_static_size),
    m_dynamic_size(vector.m_dynamic_size),
    m_deleted(vector.m_deleted)
  { reserve(); *m_data = *(vector.m_data); }

  /// @brief Move contructor
  StaticVector(StaticVector<T>&& other)
  {
    *this = std::move(other);
  }

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

  /// @brief Only reset the user size to new_size (default 0). Do not touch the data. Use for performances.
  void resize(std::size_t const & new_size = 0) {m_dynamic_size = new_size;}
  void clear () {m_dynamic_size = 0;}

  /// @brief Delete memory, reset the user size to 0 and allocate new_size memory
  void static_resize(std::size_t const & new_size = 0)
  {
    if (m_static_size) delete[] m_data;
    m_dynamic_size = 0;
    m_static_size = new_size;
    reserve();
  }
  void static_resize(std::size_t const & new_size, T const & t)
  {
    static_resize(new_size);
    fill_static(t);
  }

  /// @brief Does the vector contain element e ?
  /// @param t: variable in read-only mode
  virtual bool has(T const & t) const {return (std::find(this -> begin(), this -> end(), t) != this -> end());}

  /// @brief Does the vector contain element e ?
  /// @param t: direct access to the variable
  virtual bool has(T & t) const {return (std::find(this -> begin(), this -> end(), t) != this -> end());}

  /// @brief Add element to the back of the vector. Use for performances. Unsafe. define SAFE for less performance but size checking
  void push_back(T const & e) 
  {
  #ifdef SAFE
    if (m_dynamic_size < m_static_size) m_data[m_dynamic_size++] = e;
    else std::cout << "Capacity of StaticVector<" << typeid(T).name() << "> with size " << m_static_size << " exceeded" << std::endl;
  #else 
    m_data[m_dynamic_size++] = e;
  #endif //SAFE
  }

  /// @brief Move the element to the back of the vector. Use for performances. Unsafe. define SAFE for less performance but size checking
  void move_back(T && e) 
  {
  #ifdef SAFE
    if (m_dynamic_size < m_static_size) m_data[m_dynamic_size++] = e;
    else std::cout << "Capacity of StaticVector<" << typeid(T).name() << "> with size " << m_static_size << " exceeded" << std::endl;
  #else 
    m_data[m_dynamic_size++] = std::move(e);
  #endif //SAFE
  }

  /// @brief Add element to the back of the vector only if the vector do not contain it
  void push_back_unique(T const & t) {if (!this->has(t)) this -> push_back(t);}

  /// @brief Return iterator to the beginning of the vector
  virtual T* begin() {return m_data;}

  /// @brief Return iterator to the end of the vector
  virtual T* end()  {return m_data+m_dynamic_size;}

  /// @brief Return iterator to the beginning of the vector
  virtual T* begin() const {return m_data;}

  /// @brief Return iterator to the end of the vector
  virtual T* end() const {return m_data+m_dynamic_size;}

  /// @brief Return the position of the write cursor
  auto const & size() const {return m_dynamic_size;}

  /// @brief Return the ith element 
  T & operator[] (std::size_t const & i) const {return m_data[i];}

  /// @brief Return the ith element and check i do not exceed the size of the vector
  T const & at(std::size_t const & i) const {if (i < m_static_size) return m_data[i]; else return m_data[0];}

  /// @brief Return a pointer to the underlying data
  T* data() {return m_data;}

  /// @brief Fills the vector with element e within user size
  void fill(T const & e) {memset(m_data, e, m_dynamic_size * sizeof(e));}

  /// @brief Fills the vector with element e within static size
  void fill_static(T const & e) {memset(m_data, e, m_static_size * sizeof(e));}

  void reserve() {m_data = new T[m_static_size];}

private:
  T* m_data; // Underlying data dynamic array
  std::size_t m_dynamic_size = 0; // User size
  std::size_t m_static_size = 0;  // Maximum size
  bool m_deleted = false; // Is the class deleted or not
};

template<class T>
std::ostream& operator<<(std::ostream& cout, StaticVector<T> const & vector)
{
  for (auto const & e : vector) cout << e << " ";
  return cout;
}

#endif //VECTOR_FUNCTIONS_HPP