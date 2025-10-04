#ifndef VECTOR_FUNCTIONS_HPP
#define VECTOR_FUNCTIONS_HPP

#include "print.hpp"
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

template<class T>
using vector2D =  std::vector<std::vector<T>>;

template<class T>
T sum(std::vector<T> const & source)
{
  T sum = 0;
  for (auto const & value : source) sum += value;
  return sum;
}

template<class T>
T mean(std::vector<T> const & source)
{
  T mean = 0;
  for (auto const & value : source) mean += value;
  return mean/static_cast<T>(source.size());
}

template<class T>
std::vector<T> push_back(std::vector<T> const & target, T const & value)
{
  target.push_back(value);
}

template<class T>
std::vector<T> push_back(std::vector<T> const & target, std::vector<T> const & source)
{
  for (auto const & value : source) target.push_back(value);
}

template<class T>
std::istream& operator>>(std::istream& input, std::vector<T> vector)
{
  T t;
  input >> t;
  vector.push_back(t);
  return input;
}

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
  else return false;
}

/**
 * @brief Pushes back the new value only if it is superior to the last one (requires a number or a class with < operator)
 * 
 * @tparam T 
 * @param vector 
 * @param t 
 * @return true 
 * @return false 
 */
template <typename T>
bool push_back_increase(std::vector<T> & vector, T const & t)
{
  auto const & size = vector.size();
  if ((size == 0) || vector[size-1] < t)
  {
    vector.push_back(t);
    return true;
  }
  else return false;
}


template<class T>
void fill2D(vector2D<T> & vec2, size_t const & size_x, size_t const & size_y, T const & obj)
{
  vec2.reserve(size_x);
  for (size_t i = 0; i<size_x; i++) vec2.emplace_back(std::vector<T>(size_y, obj));
}



template <typename T>
bool found(std::vector<T> const & vec, T const & t)
{
  return (std::find(vec.begin(), vec.end(), t) != vec.end());
}

template <typename T>
bool found(std::vector<T> & vec, T const & t)
{
  return (std::find(vec.begin(), vec.end(), t) != vec.end());
}

template <typename T>
bool found(std::vector<T> const & vec, T & t)
{
  return (std::find(vec.begin(), vec.end(), t) != vec.end());
}

template <typename T>
bool found(std::vector<T> & vec, T & t)
{
  return (std::find(vec.begin(), vec.end(), t) != vec.end());
}



template <typename T>
int first_index_in(std::vector<T> & vec, T & t)
{
  return (std::distance(vec.begin(), std::find(vec.begin(), vec.end(), t) != vec.end()));
}

template <typename T>
int first_index_in(std::vector<T> & vec, T const & t)
{
  return (std::distance(vec.begin(), std::find(vec.begin(), vec.end(), t) != vec.end()));
}

template <typename T>
int first_index_in(std::vector<T> const & vec, T & t)
{
  return (std::distance(vec.begin(), std::find(vec.begin(), vec.end(), t) != vec.end()));
}

template <typename T>
int first_index_in(std::vector<T> const & vec, T const & t)
{
  return (std::distance(vec.begin(), std::find(vec.begin(), vec.end(), t) != vec.end()));
}

//////////////
// LINSPACE //
//////////////

template<class T>
std::vector<T>& linspace(std::vector<T> & vec, size_t size, int begin = 0, int spacing = 1)
{
  vec.clear(); vec.reserve(size);
  for (size_t it = 0; it<size; ++it) vec.push_back(static_cast<T>(it * spacing + begin));
  return vec;
}

std::vector<size_t>& linspace(std::vector<size_t> & vec, size_t size, size_t begin = 0, size_t spacing = 1)
{
  vec.clear(); vec.reserve(size);
  for (size_t it = begin; it<size; ++it) vec.push_back(it*spacing);
  return vec;
}

template<class T>
std::vector<T> linspace(size_t size, T begin = 0, T spacing = 1)
{
  std::vector<T> ret;
  linspace(ret, size, begin, spacing);
  return ret;
}

template<class T>
std::vector<T> linspace_for(std::vector<T> const & values, T begin = 0, T spacing = 1)
{
  std::vector<T> ret;
  linspace(ret, values.size(), begin, spacing);
  return ret;
}


template<class T>
auto c_linspace(std::vector<T> & vec, size_t size, int begin = 0, int spacing = 1)
{
  vec.clear(); vec.reserve(size);
  for (size_t it = 0; it<size; ++it) vec.push_back(static_cast<T>(it * spacing + begin));
  return vec.data();
}

auto c_linspace(std::vector<size_t> & vec, size_t size, size_t begin = 0, size_t spacing = 1)
{
  vec.clear(); vec.reserve(size);
  for (size_t it = begin; it<size; ++it) vec.push_back(it*spacing);
  return vec.data();
}

template<class T>
auto c_linspace(size_t size, T begin = 0, T spacing = 1)
{
  std::vector<T> ret;
  c_linspace(ret, size, begin, spacing);
  return ret.data();
}

template<class T>
auto c_linspace_for(std::vector<T> const & values, T begin = 0, T spacing = 1)
{
  std::vector<T> ret;
  c_linspace(ret, values.size(), begin, spacing);
  return ret.data();
}


// TODO : logspace, like linspace but logarithmic 
// template<class T>
// std::vector<T> logspace(std::vector<double> & vec, size_t size, T begin = 0, double power = 10)
// {
//   if (min <= 0) throw std::invalid_argument("min and max must be positive.");
//   double factor = min; // Start with the initial minimum value
  
//   for (size_t i = 0; i < size; ++i) vec[i] = factor * std::pow(power, i);

//   return vec;
// }

// template<class T>
// std::vector<T> logspace(size_t size, T begin = 0, double power = 10)
// {
//   if (min <= 0) throw std::invalid_argument("min and max must be positive.");
//   std::vector<double> ret(size);
//   double factor = min; // Start with the initial minimum value
  
//   for (size_t i = 0; i < size; ++i) ret[i] = factor * std::pow(power, i);
  
//   return ret;
// }

// std::vector<double> log2space(size_t nb_bins, int min) {return logspace(nb_bins, min, 2);}
// std::vector<double> log10space(size_t nb_bins, int min) {return logspace(nb_bins, min, 10);}



template <typename T, size_t n>
constexpr T maximum(std::array<T, n> const & array)
{
  static_assert(n>0, "array size is 0 !!");
  auto value = array[0];
  for (auto const & e : array) if (e>value) value = e;
  return value;
}

template <typename T>
T maximum(std::vector<T> const & vector)
{
  if (vector.size() < 1 ) throw std::runtime_error("vector size is 0 !!");
  auto value = vector[0];
  for (auto const & e : vector) if (e>value) value = e;
  return value;
}

template <typename T>
T minimum(std::vector<T> const & vector)
{
  if (vector.size() < 1 ) throw std::runtime_error("vector size is 0 !!");
  auto value = vector[0];
  for (auto const & e : vector) if (e<value) value = e;
  return value;
}

template <typename T>
size_t maximum_index(std::vector<T> const & vector)
{
  size_t index = 0;
  T value = vector[index];
  for (size_t i = 0; i<vector.size(); i++) if (vector[i]>value) 
  {
    value = vector[i];
    index = i;
  }
  return index;
}

template <typename T>
T minimum_index(std::vector<T> const & vector)
{
  int index = 0;
  T value = vector[index];
  for (int i = 0; i<vector.size(); i++) if (vector[i]<value) 
  {
    value = vector[i];
    index = i;
  }
  return value;
}

template <typename T>
bool is_good(std::vector<T> const & vector)
{
  static_assert(std::is_arithmetic<T>::value, "in is_good(std::vector<T> vector) : T must be an arithmetic type (a number)");
  auto const & size = vector.size();
  if (size == 0) return false;
  for (auto const & v : vector) if (std::isnan(v) || std::isinf(v)) return false;
  return true;
}

namespace Colib
{
  /// @brief Order the vector from lower to higher value
  /// @tparam T must have operator< overloaded if user defined class 
  template <typename T>
  std::vector<size_t> & bubble_sort(std::vector<T> const & vector, std::vector<size_t> & ordered_indexes)
  {
    // Verifications :
    if (vector.size() == 0) {printC(Colib::Color::RED, "In bubble_sort(vector, ordered_indexes) : vector size is zero !", Colib::Color::RESET); return ordered_indexes;}
    if (vector.size() != ordered_indexes.size()) ordered_indexes.resize(vector.size());
  
    // Initializations :
    T v = vector[0];
    ordered_indexes[0] = 0;
    size_t j = 0;
  
    // Loop through the vector :
    for (size_t i = 0;i<vector.size(); i++)
    {
      // Initial guess : the ith ordered_indexes's index corresponds to the vector's ith index 
      // (e.g. the 5th bin has initial value 5 (ordered_indexes[5] = 5))
      ordered_indexes[i] = i;
  
      // Check this assumption : v holds the value of ith value of vector
      v = vector[i];
  
      // Find the true jth index of the ith vector's index
      j = i;
  
      // Loop goes on until the (j-1)th vector's value is lower than the ith value
      while((j>0) && vector[ordered_indexes[j-1]] > v)
      {
        // If the (j-1)th value is higher than the ith value then switch the indexes.
        ordered_indexes[j] = ordered_indexes[j-1];
        --j;
      }
  
      // Save the correct position of the index
      ordered_indexes[j] = i;
    }
  
    // Note that this method is iterative : if the 1st value is higher than the 2nd,
    // ordered_index[0] = 1 and ordered_index[1] = 0. If now the 3rd value is higher than 
    // the 2nd but lower than the 1st (for i = 2), vector[ordered_index[1]] > v
    // but vector[ordered_index[0]] < v : the result is indeed {1, 2, 0}
  
    return ordered_indexes;
  }
  
  /// @brief Order the vector from lower to higher value
  /// @tparam T must have operator< overloaded if user defined class 
  template <class T>
  std::vector<size_t> bubble_sort(std::vector<T> const & vector)
  {
    std::vector<size_t> ordered_indexes(vector.size());
    bubble_sort(vector, ordered_indexes);
    return ordered_indexes;
  }

  /// @brief Order the vector from lower to higher value using various methods
  /// @param method: so far, only default "bubble" method is implemented, using Colib::bubble_sort
  /// @tparam T must have operator< overloaded if user defined class 
  template <class T>
  std::vector<T>& sort(std::vector<T> & vector, std::string method = "bubble")
  {
    if (method == "bubble")
    {
      auto const & sorted_indices = Colib::bubble_sort(vector);
      std::vector<T> buff = vector;
      vector.clear();
      for (auto const & id : sorted_indices) vector.push_back(buff[id]);
      return vector;
    }
    else error("sorting method", method, "unkown...");
    return vector;
  }

  /// @brief Fills vector to_fill with ordered elements from vector fill_from
  /// @tparam T must have operator< overloaded if user defined class 
  template <class T>
  std::vector<T>& fill_sorted(std::vector<T> & to_fill, std::vector<T> const & fill_from)
  {
    auto const & sorted_indices = Colib::bubble_sort(fill_from);
    int to_fill_it = 0;
    for (auto const & id : sorted_indices)
    {
      auto & e = fill_from[id];
      if (to_fill_it<to_fill.size()) for (;to_fill_it < to_fill.size(); ++to_fill_it)
      {
        if (e > to_fill[to_fill_it]) to_fill.insert(to_fill.begin()+to_fill_it, e);
        break;
      }
      else to_fill.push_back(e);
    }
    return to_fill;
  }

  // Coder la memem chose pour un deque car c'est plus efficace !
}

template <class T>
void invert(std::vector<T> & vector)
{
  std::reverse(vector.begin(), vector.end());
}

template<class K, class V>
void unpack(std::vector<std::pair<K,V>> const & pairs, std::vector<K> & keys, std::vector<V> & values)
{
  keys.reserve(pairs.size());
  values.reserve(pairs.size());
  for (auto const & pair : pairs)
  {
    keys.push_back(pair.first);
    values.push_back(pair.second);
  }
}

/// @brief Returns the vector in the range [start, start+length[.
/// @details E.g. vec = {1,2,3,4,5} ; sub_vec(vec, 1, 3) -> {2, 3, 4};
template<class T>
auto sub_vec(std::vector<T> const & vec, int const & start, int const & length)
{
  return std::vector<T>(vec.begin() + start, vec.begin() + start + length);
}

/**
 * @brief Returns the first order unit derivative of the given vector.
 * 
 * @param vec 
 * @param smooth 
 * @return std::vector<T> 
 */
template<class T>
std::vector<T> derivate(std::vector<T> const & vec, int const & smooth = 1)
{
  auto const & N = vec.size();
  std::vector<T> ret; ret.reserve(N);

  auto const & smooth_range = 2*smooth;
  int lower_bin = 0;
  int upper_bin = 0;
  double low_sum = 0.0;
  double up_sum = 0.0;
  for (int bin = 0; bin<N; bin++)
  {
    lower_bin = bin-smooth;
    upper_bin = bin+smooth;

    // Before all, handle side effects : 
    // At the beginning and the end of the spectra, there are not enough bins on both sides to smooth correctly
    // Therefore, we have to set a correct number of bins
    if (lower_bin<0)
    {// For the first bins of the histogram
      lower_bin = 0;
      upper_bin = 2*bin;
    }
    else if (upper_bin > N-1)
    {// For the last bins of the histogram
      lower_bin = 2*bin-N;
      upper_bin = N;
    }

    low_sum = 0.0;
    up_sum = 0.0;

    // First, sum the content of all the bins on the left :
    for (int bin_low = lower_bin; bin_low<bin; bin_low++) {low_sum+=vec[bin_low];}

    // Second, sum the content of all the bins on the right :
    for (int bin_up = bin+1; bin_up<upper_bin; bin_up++) {up_sum+=vec[bin_up];}

    // Calculate the derivative : (sum_right - sum_left) / (x_right - x_left)
    auto const & derivative = (up_sum - low_sum) / smooth_range;
    
    ret.push_back(derivative);

    return ret;
  }
}

/**
 * @brief Returns the first order derivative of the given y values based on the x axis
 * 
 * @tparam T 
 * @param x 
 * @param y 
 * @param smooth 
 * @return std::vector<T> 
 */
template<class T>
std::vector<T> derivate(std::vector<T> const & x, std::vector<T> const & y, int const & smooth = 1)
{
  auto const & N = x.size();
  if (N != y.size()) error("derivate(X, Y) : X and Y vectors size mismatch");
  std::vector<T> ret; ret.reserve(N);

  auto const & smooth_range = 2*smooth;

  int    lower_bin = 0  ;
  int    upper_bin = 0  ;
  double low_sum   = 0.0;
  double up_sum    = 0.0;

  for (int bin = 0; bin<N; bin++)
  {
    lower_bin = bin - smooth;
    upper_bin = bin + smooth;

    // Before all, handle side effects : 
    // At the beginning and the end of the spectra, there are not enough bins on both sides to smooth correctly
    // Therefore, we have to set a correct number of bins
    if (lower_bin<0)
    {// For the first bins of the histogram
      lower_bin = 0;
      upper_bin = 2*bin;
    }
    else if (upper_bin > N-1)
    {// For the last bins of the histogram
      lower_bin = 2*bin-N;
      upper_bin = N;
    }

    low_sum = 0.0;
    up_sum  = 0.0;

    // First, sum the content of all the bins on the left :
    for (int bin_low = lower_bin; bin_low<bin; bin_low++) {low_sum+=y[bin_low];}

    // Second, sum the content of all the bins on the right :
    for (int bin_up = bin+1; bin_up<upper_bin; bin_up++) {up_sum+=y[bin_up];}

    // Calculate the derivative : (sum_right - sum_left) / (x_right - x_left)
    auto const & derivative = (up_sum - low_sum) / (x[upper_bin] - y[lower_bin]);

    ret.push_back(derivative);

    return ret;
  }
}

template <class T>
std::vector<T> scale(std::vector<T> const & vec, double const & scaling)
{
  std::vector<T> ret; ret.reserve(vec.size());
  for (auto const & v : vec) ret.push_back(v*scaling);
  return ret;
}

template <class T>
std::vector<T> & scale(std::vector<T> * vec, double const & scaling)
{
  for (auto & v : *vec) v *= scaling;
  return *vec;
}

////////////////////////////
//   CLASS STATIC VECTOR  //
////////////////////////////

/**
 * @brief An efficient container for dynamic arrays with a known and fixed maximum size
 * @attention Prototype, has some memory management issues in some cases ...
 * @deprecated With optimisation option, std::vector is almost as efficient as this class ...
 * @todo clear without destroying objects may save time for large objects
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
 
   /// @brief Move constructor
   StaticVector(StaticVector<T>&& other)
   {
     *this = std::move(other);
   }
 
   ~StaticVector()
   {
     if(!m_deleted)
     {
       if (m_static_size > 0)
       {
         delete[] m_data;
         m_deleted = true;
       }
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
 
   void inline checkCapacity() const
   {
   #ifndef UNSAFE
     if (m_dynamic_size > m_static_size) std::cout << "Capacity of StaticVector<" << typeid(T).name() << "> with size " << m_static_size << " exceeded" << std::endl;
   #endif //UNSAFE
   }
 
   void inline checkCapacity() 
   {
   #ifndef UNSAFE
     if (m_dynamic_size > m_static_size) std::cout << "Capacity of StaticVector<" << typeid(T).name() << "> with size " << m_static_size << " exceeded" << std::endl;
   #endif //UNSAFE
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
     checkCapacity();
     m_data[m_dynamic_size++] = e;
   }
 
   /// @brief Move the element to the back of the vector. Use for performances. Unsafe. define SAFE for less performance but size checking
   void move_back(T && e) 
   {
     checkCapacity();
     m_data[m_dynamic_size++] = std::move(e);
   }
 
   /// @brief Add element to the back of the vector only if the vector do not contain it
   bool push_back_unique(T const & t) 
   {
     if (!this->has(t)) 
     {
       this -> push_back(t);
       return true;
     }
     else return false;
   }
 
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
   T & operator[] (std::size_t const & i) const 
   {
     checkCapacity();
     return m_data[i];
   }
 
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
   size_t m_dynamic_size = 0; // User size
   size_t m_static_size = 0;  // Maximum size
   bool m_deleted = false; // Is the class deleted or not
 };
 
 template<class T>
 std::ostream& operator<<(std::ostream& cout, StaticVector<T> const & vector)
 {
   for (auto const & e : vector) cout << e << " ";
   return cout;
 }
*/

/*
template<T>
class SmartVector
{
public:
  SmartVector() = default;
  SmartVector(std::vector<T> const & vector) : m_vector(vector) {}
  SmartVector(std::vector<T>       & vector) : m_vector(vector) {}
  SmartVector(std::vector<T>         vector) : m_vector(vector) {}
  SmartVector(std::Initializer_list<T> const & init_list) : m_vector(init_list) {}
  SmartVector(std::Initializer_list<T>       & init_list) : m_vector(init_list) {}
  SmartVector(std::Initializer_list<T>         init_list) : m_vector(init_list) {}

  auto operator=(std::vector<T> const & vector) {m_vector = vector;}
  auto operator=(std::vector<T>       & vector) {m_vector = vector;}
  auto operator=(std::vector<T>         vector) {m_vector = vector;}
  auto operator=(std::Initializer_list<T> const & init_list) {m_vector = vector;}
  auto operator=(std::Initializer_list<T>       & init_list) {m_vector = vector;}
  auto operator=(std::Initializer_list<T>         init_list) {m_vector = vector;}

  void bubble_sort() {bubble_sort(m_vector, m_index);}
  /// @brief @attention you need to check bounds
  auto const & getNextOrder() const {return m_vector[m_index[m_iterator++]];}
  /// @brief @attention you need to check bounds
  auto       & getNextOrder()       {return m_vector[m_index[m_iterator++]];}
  bool readNextOrder(T & value) 
  {
    if (m_iterator>m_vector.size()) 
    {
      value = m_vector[m_index[m_iterator++]]; 
      return true
    } 
    else return false;
  }
  void   setIterator(int const & i = 0) {m_iterator = i;}
  void resetIterator()                  {m_iterator = 0;}
  

private:
  std::vector<T> m_vector;

  // For bubble sort :
  std::vector<int> m_index;
  int m_iterator = 0;
};
*/
#endif //VECTOR_FUNCTIONS_HPP