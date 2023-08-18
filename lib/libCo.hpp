#ifndef LIB_H_CO
#define LIB_H_CO

// *********** STD includes ********* //
#include <any>
#include <array>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <numeric>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

// ********** C includes ************ //
#include <cstdlib>
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
      m_data = new bool[(m_size = size)];
      memset(m_data, value ? 1 : 0, m_size * sizeof(bool));
      return m_size;
    }

};

using Strings = std::vector<std::string>;

/////////////////////////////
//    STANDART FUNCTIONS   //
/////////////////////////////

void throw_error(std::string const & message) {throw std::runtime_error(message);}

std::map<std::string, std::string> error_message = 
{
  {"DEV", "ASK DEV or do it yourself, sry"}
};

/////////////////////////
//    MAPS FUNCTIONS   //
/////////////////////////

template<typename K, typename V> 
bool find_key(std::map<K,V> const & map, K const & key)
{
  typename std::map<K, V>::const_iterator it = map.find(key);
  return it != map.end();
}

template<typename K, typename V> 
bool find_value(std::map<K,V> const & map, V const & value)
{
  return (std::find_if(map.begin(), map.end(), [&](const auto& pair) {
        return pair.second == value;
    }));
}

template<typename K, typename V> 
std::pair<K,V> get_max(std::map<K,V> const & map) 
{
  return *std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second < p2.second;
  }); 
}

template<typename K, typename V> 
V get_max_value(std::map<K,V> const & map) 
{
  return (std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second < p2.second;
  })->second); 
}

template<typename K, typename V> 
K get_max_key(std::map<K,V> const & map) 
{
  return (*std::max_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.first < p2.first;
  })->first); 
}

template<typename K, typename V> 
std::pair<K,V> get_min(std::map<K,V> const & map) 
{
  return *std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second > p2.second;
  }); 
}

template<typename K, typename V> 
V get_min_value(std::map<K,V> const & map) 
{
  return (std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.second > p2.second;
  })->second); 
}

template<typename K, typename V> 
K get_min_key(std::map<K,V> const & map) 
{
  return (*std::min_element(map.begin(), map.end(), [] (const std::pair<K,V> & p1, const std::pair<K,V> & p2) 
  {
        return p1.first > p2.first;
  })->first); 
}

/////////////////////////////
//    TEMPLATE HANDELING   //
/////////////////////////////

template <typename T>
using T_is_number = std::enable_if_t<std::is_arithmetic_v<T>>;

#endif //LIB_H_CO
