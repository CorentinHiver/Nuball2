#ifndef LIB_HPP_CO
#define LIB_HPP_CO

// *********** STD includes ********* //
#include <any>
#include <array>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stack>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

// ********** C includes ************ //
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ********** Corentin Lib ************ //
#include "print.hpp"
#include "random.hpp"
#include "string_functions.hpp"
#include "files_functions.hpp"
#include "vector_functions.hpp"

//////////////
//   UTILS  //
//////////////

auto pauseCo() {std::cout << "Programe paused, please press enter"; return std::cin.get();}
auto pauseCo(std::string const & message) {std::cout << message << std::endl; return std::cin.get();}

//////////////
//   UNITS  //
//////////////

double _ns = 1000.;

////////////////
//    Types   //
////////////////

/// @brief Casts a number into an bool
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline bool bool_cast(T const & t) {return static_cast<bool>(t);}

/// @brief Casts a number into an char
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline char char_cast(T const & t) {return static_cast<char>(t);}

/// @brief Casts a number into an short
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline short short_cast(T const & t) {return static_cast<short>(t);}

/// @brief Casts a number into an int
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline int int_cast(T const & t) {return static_cast<int>(t);}

/// @brief Casts a number into an long
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline long long_cast(T const & t) {return static_cast<long>(t);}

/// @brief Casts a number into a float
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline float float_cast(T const & t) {return static_cast<float>(t);}

/// @brief Casts a number into an double
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline double double_cast(T const & t) {return static_cast<double>(t);}


// Type short names :
using uchar  = unsigned char       ;
using ushort = unsigned short int  ;
using uint   = unsigned int        ;
using ulong  = unsigned long int   ;
using longlong  = long long int ;
using ulonglong  = unsigned long long int ;
using size_t = std::size_t;


/// @brief Casts a number into unsigned char (uchar)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline uchar uchar_cast(T const & t) {return static_cast<uchar>(t);}

/// @brief Casts a number into unsigned short (ushort)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ushort ushort_cast(T const & t) {return static_cast<ushort>(t);}

/// @brief Casts a number into unsigned int (uint)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline uint  uint_cast(T const & t) {return static_cast<uint>(t);}

/// @brief Casts a number into unsigned long (ulong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ulong ulong_cast(T const & t) {return static_cast<ulong>(t);}

/// @brief Casts a number into unsigned long long (ulonglong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ulonglong ulonglong_cast(T const & t) {return static_cast<ulonglong>(t);}

/// @brief Casts a number into long long (longlong)
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline longlong longlong_cast(T const & t) {return static_cast<longlong>(t);}

/// @brief Casts a number into std::size_t
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline size_t size_cast(T const & t) {return static_cast<size_t>(t);}

// Containers :
class Bools 
{
private:
    bool* m_data = nullptr;
    size_t m_size = 0;

public:
    Bools() {}
    Bools(size_t size, bool const & value = false) : m_size(size) {
        m_data = new bool[m_size];
        memset(m_data, value ? 1 : 0, m_size * sizeof(bool));
    }

    // Copy constructor
    Bools(const Bools& other) : m_size(other.m_size) {
        m_data = new bool[m_size];
        memcpy(m_data, other.m_data, m_size * sizeof(bool));
    }

    // Move constructor
    Bools(Bools&& other) noexcept : m_data(other.m_data), m_size(other.m_size) {
        other.m_data = nullptr;
        other.m_size = 0;
    }

    // Copy assignment
    Bools& operator=(const Bools& other) {
        if (this != &other) {
            delete[] m_data;
            m_size = other.m_size;
            m_data = new bool[m_size];
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

    ~Bools() {
        delete[] m_data;
    }

    bool& operator[](size_t const & index) {
        return m_data[index];
    }

    bool const & operator[](size_t const & index) const {
        return m_data[index];
    }

    size_t const & size() const {
        return m_size;
    }

    size_t const & resize(size_t size, bool const & value = false) {
      if (size>m_size)
      {
        if (m_size==0) 
        {
          m_data = new bool[size];
          for (;m_size<size;m_size++) m_data[m_size] = value;
        }
        else 
        {
          bool* temp = new bool[size];
          std::memcpy(temp, m_data, m_size*sizeof(bool));
          delete[] m_data;
          for (;m_size<size;m_size++) temp[m_size] = value;// Fills new space with value AND set correct value to m_size
          m_data = temp;
        }
      }
      return m_size;
    }

    auto begin() {return m_data;}
    auto end()   {return m_data + m_size;}

    auto begin() const {return m_data;}
    auto end()   const {return m_data + m_size;}

};

std::ostream& operator>>(std::ostream& cout, Bools const & bools)
{
  for (auto const b : bools) cout << b << " ";
  return cout;
}

using Strings = std::vector<std::string>;
using Ints = std::vector<int>;


/////////////////////////////
//    STANDART FUNCTIONS   //
/////////////////////////////

void throw_error(std::string const & message) {throw std::runtime_error(message);}

std::map<std::string, std::string> error_message = 
{
  {"DEV", "ASK DEV or do it yourself, sry"}
};

///////////////////////////////////
//    UNORDERED MAPS FUNCTIONS   //
///////////////////////////////////

template<typename K, typename V> 
inline bool find_key(std::unordered_map<K,V> const & map, K const & key)
{
  typename std::unordered_map<K, V>::const_iterator it = map.find(key);
  return it != map.end();
}

template<typename K, typename V> 
inline bool find_value(std::unordered_map<K,V> const & map, V const & value)
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
/// @details This method is only lookinf for values, not keys
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

//////////////////////////////////
// Strings and vectors together //
//////////////////////////////////


// /// @brief Concatenate any type or class that can be turned into a string
// template<class... T>
// std::string concatenate(T... input)
// {
//   std::vector<std::string> all_strings;
//   all_strings.push_back(std::to_string(input)...);
//   std::string ret;
//   for (auto const & string : all_strings) ret+=string;
//   return ret;
// }


/////////////////////////////
//    TEMPLATE HANDELING   //
/////////////////////////////

template <class T>
#if (__cplusplus >= 201702L)
using T_is_number = std::enable_if_t<std::is_arithmetic_v<T>>;
#else 
using T_is_number = void;
#endif // __cplusplus >= 201702L

///////////////////////////
//   SLOTS AND SIGNALS   // TDB
///////////////////////////

#if (__cplusplus >= 201402L)
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

  template<class... ARGS>
  static void connect(std::function<void(ARGS...)> signal, std::function<void(ARGS...)> slot)
  {

  }

  private:


  // std::vector<std::function<void(ARGS...)>> m_slots;

};
#endif //__cplusplus >= 201402L

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

#endif //LIB_HPP_CO
