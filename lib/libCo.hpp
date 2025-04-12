#ifndef LIBCO_HPP
#define LIBCO_HPP

// This is used in my PC to have better looking std errors
#ifndef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 0/1
#endif //_GLIBCXX_USE_CXX11_ABI

// This is used to generate better debug symbols for drd :
#ifdef DEBUGVALGRIND
  #include <valgrind/drd.h>
  #undef _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE
  #undef _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER
  #define _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(addr) ANNOTATE_HAPPENS_BEFORE(addr)
  #define _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(addr)  ANNOTATE_HAPPENS_AFTER(addr)
#endif //DEBUGVALGRIND

// ********** Corentin Lib ************ //
#include "print.hpp"
#include "vector_functions.hpp"
#include "randomCo.hpp"
#include "string_functions.hpp"
#include "files_functions.hpp"
#include "errors.hpp"

// *********** STD includes ********* //
#include <any>
#include <array>
#include <bitset>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <string>
#include <stack>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ********** C includes ************ //
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>

///////////////////////
// Versions aliasing //
///////////////////////

#define Cpp17 (__cplusplus >= 201702L)
#define Cpp14 (__cplusplus >= 201402L)


////////////////////////////
// Some specialized print //
////////////////////////////
// ------------------------------------------------------- //
// Useful overload of operator<< into a std::cout stream :

template <class F, class S> 
std::ostream& operator<<(std::ostream& cout, std::pair<F,S> const & p)
{
  cout << " {" << p.first << ", " << p.second << "}" << std::endl;
  return cout;
}

template <class K, class V> 
std::ostream& operator<<(std::ostream& cout, std::map<K,V> const & m)
{
  cout << "{";
  for (auto const & pair : m) cout << pair;
  cout << "}\n";
  return cout;
}

template <class K, class V> 
std::ostream& operator<<(std::ostream& cout, std::unordered_map<K,V> const & m)
{
  cout << "{";
  for (auto const & pair : m) cout << pair;
  cout << "}\n";
  return cout;
}

template<class E, size_t size> 
std::ostream& operator<<(std::ostream& cout, std::array<E,size> const & a)
{
  for (size_t i = 0; i<size; i++) print(a[i]);
  return cout;
}

template<class E, int size> 
std::ostream& operator<<(std::ostream& cout, std::array<E,size> const & a)
{
  for (size_t i = 0; i<size; i++) print(a[i]);
  return cout;
}

//////////////
//   UNITS  //
//////////////

/// @brief Returns a string in the format mm_hh_dd_mm_yy
std::string time_string()
{
  std::time_t currentTime = std::time(nullptr);
  std::tm* timeInfo = std::localtime(&currentTime);
  if (timeInfo != nullptr) {
    // Extract hours, day, and year
    int min  = timeInfo->tm_min;
    int hour = timeInfo->tm_hour;
    int day  = timeInfo->tm_mday;
    int mon  = timeInfo->tm_mon;
    int year = timeInfo->tm_year % 100; // Get last two digits of the year

    std::stringstream ss;
    // Print the time in the desired format
    ss << std::setfill('0') 
       << std::setw(2) << min  << "_" 
       << std::setw(2) << hour << "_"
       << std::setw(2) << day  << "_"
       << std::setw(2) << mon  << "_"
       << std::setw(2) << year;
    return ss.str();
  } else {
    std::cerr << "Failed to get current time." << std::endl;
    return "";
  }
}

/// @brief Returns a string in the format yy_mm_dd_hh_mm
std::string time_string_inverse()
{
  std::time_t currentTime = std::time(nullptr);
  std::tm* timeInfo = std::localtime(&currentTime);
  if (timeInfo != nullptr) {
    // Extract hours, day, and year
    int min  = timeInfo->tm_min;
    int hour = timeInfo->tm_hour;
    int day  = timeInfo->tm_mday;
    int mon  = timeInfo->tm_mon;
    int year = timeInfo->tm_year % 100; // Get last two digits of the year

    std::stringstream ss;
    // Print the time in the desired format
    ss << std::setfill('0') 
       << std::setw(2) << year << "_"
       << std::setw(2) << mon  << "_" 
       << std::setw(2) << day  << "_"
       << std::setw(2) << hour << "_"
       << std::setw(2) << min;
    return ss.str();
  } else {
    std::cerr << "Failed to get current time." << std::endl;
    return "";
  }
}

template <typename T>
std::string to_binary(T num) {
    return std::bitset<sizeof(T) * 8>(num).to_string(); // Uses full bit width of T
}

void throw_error(std::string const & message) {throw std::runtime_error(concatenate(CoLib::Color::RED, message, CoLib::Color::RESET));}

std::map<std::string, std::string> error_message = 
{
  {"DEV", "to be done"},
  {"WOW", "Ok, wow"},
  {"WTF", "What the fuck ??"}
};

auto pauseCo() 
{
#ifdef MULTITHREADING
  if (MTObject::kill) exit(MTSIGEXIT);
  lock_mutex lock(MTObject::mutex);
#endif //MULTITHREADING

  std::cout << "Programme paused, please press enter"; 
  return std::cin.get();
}

auto pauseCo(std::string const & message) 
{
#ifdef MULTITHREADING
  if (MTObject::kill) exit(MTSIGEXIT);
  lock_mutex lock(MTObject::mutex);
#endif //MULTITHREADING

  std::cout << message << std::endl;
  return std::cin.get();
}

void pauseDebug(std::string const & message = "") 
{
#ifdef DEBUG
  if (message == "") pauseCo();
  else pauseCo(message);
#endif //DEBUG
}

////////////////
//    Types   //
////////////////

/// @brief Casts a any type into an bool
template<typename T>
constexpr inline bool bool_cast(T const & t) {return static_cast<bool>(t);}

/// @brief Casts a number into an char
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline char char_cast(T const & t) {return static_cast<char>(t);}

/// @brief Casts a number into an short
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline short short_cast(T const & t) {return static_cast<short>(t);}

/// @brief Casts a number into an int
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline int int_cast(T const & t) {return static_cast<int>(t);}

/// @brief Casts a number into an long
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline long long_cast(T const & t) {return static_cast<long>(t);}

/// @brief Casts a number into a float
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline float float_cast(T const & t) {return static_cast<float>(t);}

/// @brief Casts a number into an double
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline double double_cast(T const & t) {return static_cast<double>(t);}


// Type short names :
using uchar  = unsigned char       ;
using ushort = unsigned short int  ;
using uint   = unsigned int        ;
using ulong  = unsigned long int   ;
using longlong  = long long int    ;
using ulonglong  = unsigned long long int ;
using size_t = std::size_t;
// using abs = std::abs;

// Print specialization for uchar : 
std::ostream& operator<<(std::ostream& cout, uchar const & uc)
{
  std::cout << static_cast<int>(uc);
  return cout;
}

/// @brief Casts a number into unsigned char (uchar)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline uchar uchar_cast(T const & t) {return static_cast<uchar>(t);}

/// @brief Casts a number into unsigned short (ushort)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline ushort ushort_cast(T const & t) {return static_cast<ushort>(t);}

/// @brief Casts a number into unsigned int (uint)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline uint  uint_cast(T const & t) {return static_cast<uint>(t);}

/// @brief Casts a number into unsigned long (ulong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline ulong ulong_cast(T const & t) {return static_cast<ulong>(t);}

/// @brief Casts a number into unsigned long long (ulonglong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline ulonglong ulonglong_cast(T const & t) {return static_cast<ulonglong>(t);}

/// @brief Casts a number into long long (longlong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline longlong longlong_cast(T const & t) {return static_cast<longlong>(t);}

/// @brief Casts a number into std::size_t
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
constexpr inline size_t size_cast(T const & t) {return static_cast<size_t>(t);}

///@brief Check if the given double has integer precision
bool is_int (double const & x) {return std::trunc(x) == x;}

////////////////////////
// General converters //
////////////////////////

class CastImpossible {
public:
  CastImpossible() noexcept = default;
  CastImpossible(std::string const & message) noexcept : m_message (message) {}
  std::string what() {return m_message;}
  std::string what() const {return m_message;}

private:
  std::string m_message;
};

template<class T>
T string_to(std::string const & string)
{
  T t;
  std::istringstream iss(string);

  if (!(iss>>t) || !iss.eof())
  {
    throw CastImpossible(concatenate("In string_to<T>(std::string const & string) with string = ",
    string, " the string can't be casted to ", type_of(t)));
  }
  return t;
}

////////////////
// Containers //
////////////////

class Bools 
{
private:
  bool* m_data = nullptr;
  size_t m_size = 0;
  size_t m_reserved_size = 0;

public:
  Bools() noexcept = default;
  Bools(size_t size, bool const & value = false) noexcept : m_size(size), m_reserved_size(2*size) {
    m_data = new bool[m_reserved_size];
    memset(m_data, value ? 1 : 0, m_size * sizeof(bool));
  }

  // Copy constructor
  Bools(const Bools& other) noexcept : m_size(other.m_size), m_reserved_size(2*m_size) {
    m_data = new bool[m_size];
    memcpy(m_data, other.m_data, m_size * sizeof(bool));
  }

  // Move constructor
  Bools(Bools&& other) noexcept : m_data(other.m_data), m_size(other.m_size), m_reserved_size(other.m_reserved_size) {
    // Re-initialise the original data in case it is re-used later on
    other.m_data = nullptr;
    other.m_size = 0;
  }

  // Copy assignment
  Bools& operator=(const Bools& other) noexcept {
      if (this != &other) {
        delete[] m_data;
        m_size = other.m_size;
        m_reserved_size = other.m_reserved_size;
        m_data = new bool[m_reserved_size];
        memcpy(m_data, other.m_data, m_size * sizeof(bool));
      }
      return *this;
  }

  // Move assignment
  Bools& operator=(Bools&& other) noexcept {
    if (this != &other) {
      delete[] m_data;
      m_data = other.m_data;
      m_size = other.m_size;
      other.m_data = nullptr;
      other.m_size = 0;
    }
    return *this;
  }

  void push_back(bool const & value)
  {
    this -> resize(m_size+1);
    m_data[m_size++] = value;
  }

  ~Bools() {
    delete[] m_data;
  }

  bool       & operator[](size_t const & index)       {
    return m_data[index];
  }

  bool const & operator[](size_t const & index) const {
    return m_data[index];
  }

  size_t const & size() const {
    return m_size;
  }

  void resize(size_t size) {
         if (m_size == size) return;
    else if (size>m_reserved_size)
    {
      bool* temp = new bool[(m_reserved_size = size*2)];
      if (m_data) std::memcpy(temp, m_data, m_size*sizeof(bool));
      delete[] m_data;
      m_data = temp;
    }
    for (;m_size<size;++m_size) m_data[m_size] = false;
  }

  void resize(size_t size, bool const & value) {
    if (size>m_reserved_size)
    {
      delete[] m_data;
      m_data = new bool[(m_reserved_size = size*2)];
    }
    for (m_size = 0;m_size<size;++m_size) m_data[m_size] = value;
  }

  auto begin() {return m_data;}
  auto end()   {return m_data + m_size;}

  auto begin() const {return m_data;}
  auto end()   const {return m_data + m_size;}

  // Boolean logic :
  
  bool AND() const noexcept
  {
    for(auto const & value : *this) if (!value) return false;
    return true;
  }
  
  bool OR() const noexcept
  {
    for(auto const & value : *this) if (value) return true;
    return false;
  }

  bool XOR() const noexcept
  {
    bool _found = false;
    for(auto const & value : *this) if (value)
    {
      if(_found) return false;
      else _found = true;
    }
    return _found;
  }
};

std::ostream& operator<<(std::ostream& cout, Bools const & bools)
{
  for (auto const b : bools) cout << b << " ";
  return cout;
}

using Strings = std::vector<std::string>;
using Ints = std::vector<int>;

std::string mergeStrings(Strings const & strings, std::string const & sep = "")
{
  std::string ret;
  for (auto const & str : strings) ret += str + sep;
  return removeLastPart(ret, sep); // Remove the last sep
}

template<class T>
std::string mergeStrings(std::vector<T> const & strings, std::string const & sep = "")
{
  std::string ret;
  for (auto const & str : strings) ret += std::to_string(str) + sep;
  return removeLastPart(ret, sep); // Remove the last sep
}

template<typename T>
struct is_container {
private:
    template<typename C>
    static auto test(int) -> decltype(std::begin(std::declval<C>()), std::end(std::declval<C>()), std::true_type());

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// -- unordered_set -- //

template<class T>
constexpr bool found (std::unordered_set<T> set, T const & e)
{
  return set.find(e) != set.end();
}

///////////
// MATHS //
///////////

template<class T> T positive_modulo(T const & dividend, T const & divisor)
{
  auto ret = dividend % divisor;
  if (ret<0) ret+=divisor;
  return ret;
}

namespace CoLib
{
  template<class T, class... ARGS>
  inline T sum(T i, ARGS... args) {return i+sum(args...);}

  template<class... ARGS>
  inline double mean(ARGS... args) {return double_cast(sum(args...) / sizeof...(args));}

  using Point = std::pair<double, double>;
  constexpr static Point rotate(double const & x, double const & y, double const & angle) 
  {
    return Point(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
  }
  constexpr static Point rotate(Point const & point, double const & angle) 
  {
    return Point(point.first * cos(angle) - point.second * sin(angle), point.first * sin(angle) + point.second * cos(angle));
  }
}


///////////////////////////////////
//    UNORDERED MAPS FUNCTIONS   //
///////////////////////////////////

template<typename K, typename V> 
constexpr inline bool find_key(std::unordered_map<K,V> const & map, K const & key)
{
  typename std::unordered_map<K, V>::const_iterator it = map.find(key);
  return it != map.end();
}

template<typename K, typename V> 
constexpr inline bool find_value(std::unordered_map<K,V> const & map, V const & value)
{
  return (std::find_if(map.begin(), map.end(), [&](const auto& pair) {
        return pair.second == value;
    }));
}

/////////////////////////
//    MAPS FUNCTIONS   //
/////////////////////////

/// @brief Returns yes if the key is found in the map
/// @details This method is only looking in the keys, not in the values
template<typename K, typename V> 
inline bool find_key(std::map<K,V> const & map, K const & key)
{
  typename std::map<K, V>::const_iterator it = map.find(key);
  return it != map.end();
}

/// @brief Returns the list of keys in a map
/// @details This method is only looking in the keys, not the values
template<typename K, typename V> 
inline std::vector<K> list_of_keys(std::map<K,V> const & map)
{
  std::vector<K> ret;
  for (auto const & it : map) ret.push_back(it.first);
  return ret;
}

/// @brief Returns yes if the value is found in the map
/// @details This method is only looking in the values, not in the keys
template<typename K, typename V> 
inline bool find_value(std::map<K,V> const & map, V const & value)
{
  return (std::find_if(map.begin(), map.end(), [&](const auto& pair) {
        return pair.second == value;
    }));
}

/// @brief Returns the element with the maximum value
/// @details This method is only comparing values, not keys
template<typename K, typename V> 
inline std::pair<K,V> get_max_element(std::map<K,V> const & map) 
{
  return *std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
    return p1.second < p2.second;
  }); 
}

/// @brief Returns the maximum value stored in the map
/// @details This method is only looking for values, not keys
template<typename K, typename V> 
inline V get_max_value(std::map<K,V> const & map) 
{
  return (std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
    return p1.second < p2.second;
  })->second); 
}

/// @brief Returns the maximum key stored in the map
/// @details This method is only looking for values, not keys
template<typename K, typename V> 
inline K get_max_key(std::map<K,V> const & map) 
{
  return (*std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.first < p2.first;
  })->first); 
}

template<typename K, typename V> 
inline std::pair<K,V> get_min(std::map<K,V> const & map) 
{
  return *std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second > p2.second;
  }); 
}

template<typename K, typename V> 
inline V get_min_value(std::map<K,V> const & map) 
{
  return (std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second > p2.second;
  })->second); 
}

template<typename K, typename V> 
inline K get_min_key(std::map<K,V> const & map) 
{
  return (*std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.first > p2.first;
  })->first); 
}

// template<typename K, typename V, typename NewK>
// inline auto convert(std::map<K,V> const & map) 
// {
//   std::map<NewK, V> ret;
//   if (map.empty()) return ret;
//   for (auto const & e : map) 
//   {
//     try{
//        ret.emplace(static_cast<NewK>(e.first), e.second);
//     } catch (...){
//       throw std::runtime_error("in convert(map) : types not compatibles");
//     }
//   }
//   return ret;
// } 

template<typename NewK, typename K, typename V>
inline auto convert(std::map<K,V> const & map) 
{
    std::map<NewK, V> ret;
    if (map.empty()) return ret;
  
    for (auto const & e : map) {
        // Assuming K is 'unsigned short' and NewK is 'int'
        ret.emplace(static_cast<NewK>(e.first), e.second); // Convert K (unsigned short) to NewK (int)
    }
  
    return ret;
}


// Cross-containers found function :
template<typename K, typename V> 
inline bool found(std::map<K,V> const & map, K const & key) {return find_key(map, key);}

template<typename K, typename V> 
inline bool found(std::unordered_map<K,V> const & map, K const & key) {return find_key(map, key);}

////////////////////////////
//    TEMPLATE HANDLING   //
////////////////////////////

template <class T>
#if Cpp17
using T_is_number = std::enable_if_t<std::is_arithmetic_v<T>>;
#else 
using T_is_number = void;
#endif // Cpp17

template<class... T>
constexpr std::size_t get_size() {
    return sizeof...(T);
}

#if Cpp17

template <typename... Args>
struct are_all_arithmetic;

template <>
struct are_all_arithmetic<> : std::true_type {};

template <typename First, typename... Rest>
struct are_all_arithmetic<First, Rest...>
    : std::conjunction<std::is_arithmetic<First>, are_all_arithmetic<Rest...>> {};

#endif //Cpp17

///////////////////////////
//   SLOTS AND SIGNALS   // TDB
///////////////////////////

#if Cpp14
template<class... ARGS>
class Signal
{
  public:

  Signal() = default;
  // Signal(std::function<void(ARGS...)> && func) {}
  // Signal(std::function<void(ARGS...)> & func) {m_signals.emplace_back(func);}

  void operator()(ARGS &&... args){for (auto const & signal : m_signals) signal(std::forward<ARGS>(args)...);}

  void connect(std::function<void(ARGS...)> func)
  {
    m_signals.push_back(func);
  }

  private:

  std::vector<std::function<void(ARGS...)>> m_signals;
  // std::vector<std::function<void(ARGS...)>> m_signals;

};

class Slots
{
  public:
  Slots() = default;

  // template<class... ARGS>
  // static void connect(std::function<void(ARGS...)> signal, std::function<void(ARGS...)> slot)
  // {

  // }

  private:


  // std::vector<std::function<void(ARGS...)>> m_slots;

};
#endif //Cpp14

/////////////////////////
//   SOME COOL STUFF   //
/////////////////////////

namespace CoLib
{
  void progress_bar(float const & progress_percent, int width = 50)
  {
    auto const & nb_chars = int_cast(progress_percent/100.*width);

    std::cout << "|";
    for (int i = 0; i<width; i++) 
    {
      if (i<nb_chars) std::cout << ".";
      else            std::cout << " ";
    }
    std::cout << "| : " << int_cast(progress_percent) << "%" << std::endl << "\033[F";
    std::cout.flush();
  }

  void short_progress_bar(float const & progress_percent) {progress_bar(progress_percent, 10 );}
  void long_progress_bar (float const & progress_percent) {progress_bar(progress_percent, 100);}

}

template <class T>
std::string nicer_double(T const & t, int const & nb_decimals = 6)
{
  auto value = double_cast(t);
  std::string s;
       if (value<1.e-9)  {value*=1.e+12; s = " f";}
  else if (value<1.e-6)  {value*=1.e+9 ; s = " n";}
  else if (value<1.e-3)  {value*=1.e+6 ; s = " µ";}
  else if (value<1.e+0)  {value*=1.e+3 ; s = " m";}
  else if (value<1.e+3)  {value*=1.e+0 ; s = " " ;}
  else if (value<1.e+6)  {value*=1.e-3 ; s = " k";}
  else if (value<1.e+9)  {value*=1.e-6 ; s = " M";}
  else if (value<1.e+12) {value*=1.e-9 ; s = " G";}

  std::stringstream ss;
  ss << std::fixed << std::setprecision(nb_decimals) << value << s;
  return ss.str();
}

//////////////////////////////////////
// COMPILE-TIME LOOK UP TABLE (LUT) //
//////////////////////////////////////

/**
 * @brief Lookup table that can be generated at compile time.
 * @details 
 * Instanciation :
 * constexpr auto myLut = LUT<10> ([](int i) { return i*i; }); 
 * 
 */
template<std::size_t size, class Generator>
constexpr auto LUT(Generator&& g)
{
  // Deduce the return type of the lookup table :
  using type = std::decay_t<decltype(g(std::size_t{0}))>;
  // Instanciate the lookup table :
  std::array<type, size> lut{};
  // Fill the lookup table using the generator :
  for (std::size_t i = 0; i<size; ++i) lut[i] = std::forward<Generator>(g)(i);
  return lut;
}

/// @brief 
/// @attention Works only in ordered arrays
template <typename T, std::size_t N>
constexpr bool binary_search (std::array<T, N> const & array, T const & value)
{
  int low = 0;
  int high = N - 1;

  while (low <= high) {
    int mid = (low + high) / 2;
         if (array[mid] < value) low = mid + 1;
    else if (array[mid] > value) high = mid - 1;
    else return true;
  }
  return false;  // Value not found
}

template <typename T, std::size_t N>
constexpr int find_index(const std::array<T, N>& array, const T& value) 
{
  for (std::size_t i = 0; i < N; ++i) if (array[i] == value) return i;
  return -1;  // Value not found
} 

template <typename T, std::size_t N>
constexpr T found(const std::array<T, N>& array, const T& value) 
{
  for (std::size_t i = 0; i < N; ++i) if (array[i] == value) return true;
  return false;  // Value not found
} 

//////////////////////////////////
// MAKE SOME STD FUNC CONSTEXPR //
//////////////////////////////////

#if Cpp17

template <typename T>
typename std::enable_if_t<std::is_arithmetic_v<T>, T>
constexpr abs_const(T const & t) {return (t>=0) ? t : -t;}

#endif //Cpp17

////////////////////
// User interface //
////////////////////

std::string askUser(std::string message)
{
  print(message);
  std::string ret;
  std::getline(std::cin, ret);
  return ret;
}

bool askUserYN(std::string message)
{
  return (askUser(message) == "y");
}

////////////////
// Algorithms //
////////////////


/// @brief Calculate the slope for two extremal points, then correct it with the residues of the intermediat points.
template<class T> double quickSlope(std::vector<T> x, std::vector<T> y)
{
  if (x.size()!=y.size()) throw_error("quickSlope : x and y don't have the same size !!");
  auto const & N = x.size()-1;
  double slope = (y[N]-y[0])/(x[N]-x[0]);
  double correctionSum = 0;
  for (size_t i = 1; i<N-1; ++i)
  {
    auto const & dx = x[i]-x[0];
    auto const & residues = y[i] - (y[0] + slope*dx);
    correctionSum += residues/dx;
  }

  double correction = ((N-1) > 0) ? (correctionSum / (N-1)) : 0.0;
  return slope+correction;
}

// #if (__cplusplus >= 201703L)

// /**
//  * @brief This class allows one to read one specific format of csv file (see details)
//  * 
//  * @details
//  * 
//  * The format of the data MUST be the following : 
//  * 
//  * [[Name of the columns]]
//  * [[First row data]]
//  * [[....]]
//  * [[Last row data]]
//  * 
//  * Then, declare the reader in two steps : 
//  * 
//  * first construct the reader using the constructor.
//  * 
//  * 
//  */
// template<class... T>
// class CSVReader
// {
// public:
//   CSVReader(std::string const & filename, char const & delim = ';') {this -> open(filename, delim);}

//   bool open(std::string const & filename, char const & delim = ';');

//   operator bool() const & {return m_ok;}

// private:
//   std::vector<std::string> m_header;
//   std::vector<std::tuple<T...>> m_data;
//   bool m_ok = false;
// };

// template<class... T>
// bool CSVReader<T...>::open(std::string const & filename, char const & delim)
// {
//   // Open file :
//   std::ifstream file(filename, std::ios::in);
//   if (!file) {print(filename, "not found"); return (m_ok = false);}

//   // Read names header : 
//   std::string reader;
//   std::getline(file, reader);
//   m_header = getList(reader, delim);
//   print(m_header);

//   // Read the types header :
//   while (std::getline(file, reader))
//   {
//     std::vector<std::string> typeNames = getList(reader, delim);
//      if (typeNames.size() != sizeof...(T))
//     {
//         // Handle the case where the number of types in the header
//         // doesn't match the number of template arguments.
//         print("Error: Number of types in the header doesn't match the template arguments.");
//         return (m_ok = false);
//     }
//   }


//   return true;
// }

// #endif //(__cplusplus >= 201703L)

#endif //LIBCO_HPP
