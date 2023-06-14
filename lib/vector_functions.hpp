#ifndef VECTOR_FUNCTIONS_HPP
#define VECTOR_FUNCTIONS_HPP



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

template<class T, std::size_t __size__ = 0>
class StaticVector
{
public:
  StaticVector() : m_static_size(__size__) {m_data = new T[m_static_size];}
  StaticVector(T const & value);
  StaticVector(StaticVector<T, __size__> const & vector);
  ~StaticVector(){delete[] m_data;}

  void resize(std::size_t const & size = 0) {m_dynamic_size = size;}
  void static_resize(std::size_t const & size = 0)
  {
    delete[] m_data;
    m_dynamic_size = 0;
    m_static_size = size;
    m_data = new T[m_static_size];
  }

  virtual bool has(T const & t);

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
  T *m_data;
  std::size_t m_dynamic_size = 0;
  std::size_t m_static_size = 0;
};

template<class T, std::size_t __size__>
StaticVector<T,__size__>::StaticVector(T const & value) : m_static_size(__size__)
{
  m_data = new T[m_static_size];
  for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = value;
}

template<class T, std::size_t __size__>
StaticVector<T,__size__>::StaticVector(StaticVector<T, __size__> const & vector) : m_static_size(__size__)
{
  m_data = new T[m_static_size];
  for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = vector.at(i);
}

template<class T, std::size_t __size__>
bool inline StaticVector<T,__size__>::has(T const & t)
{
  return (std::find(this -> begin(), this -> end(), t) != this -> end());
}

template<class T, std::size_t __size__>
void StaticVector<T,__size__>::push_back_unique(T const & t)
{
#ifdef SAFE
  if (!this->has(t)) this -> push_back_safe(t);
#else
  if (!this->has(t)) this -> push_back(t);
#endif //SAFE
}

template<class T, std::size_t __size__ = 0>
class StaticOrderVector : public StaticVector<T, __size__>
{// Binary search works only with
public:
  bool has(T const & t)
  {
    return std::binary_search(this -> begin(), this -> end(), t);
  }
};

#endif //VECTOR_FUNCTIONS_HPP