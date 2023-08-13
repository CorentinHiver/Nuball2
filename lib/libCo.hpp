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

/// @brief Casts a number into a float
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline float float_cast(T const & t) {return static_cast<float>(t);}

/// @brief Casts a number into an int
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline double double_cast(T const & t) {return static_cast<double>(t);}


// Units :
using uchar  = unsigned char       ;
using ushort = unsigned short int  ;
using uint   = unsigned int        ;
using ulong  = unsigned long int   ;
using ulonglong  = unsigned long long int ;
using longlong  = long long int ;
using size_t = std::size_t;


/// @brief Casts a number into unsigned char
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline uchar uchar_cast(T const & t) {return static_cast<uchar>(t);}

/// @brief Casts a number into unsigned short
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ushort ushort_cast(T const & t) {return static_cast<ushort>(t);}

/// @brief Casts a number into unsigned int
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline uint  uint_cast(T const & t) {return static_cast<uint>(t);}

/// @brief Casts a number into unsigned long
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ulong ulong_cast(T const & t) {return static_cast<ulong>(t);}

/// @brief Casts a number into unsigned long long
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ulonglong ulonglong_cast(T const & t) {return static_cast<ulonglong>(t);}

/// @brief Casts a number into long long
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline longlong longlong_cast(T const & t) {return static_cast<longlong>(t);}

/// @brief Casts a number into std::size_t
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline size_t size_cast(T const & t) {return static_cast<size_t>(t);}

// Containers :
using Bools = std::vector<bool>;
using Strings = std::vector<std::string>;

/////////////////////////////
//    STANDART FUNCTIONS   //
/////////////////////////////

#endif //LIB_H_CO
