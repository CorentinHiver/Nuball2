#ifndef PRINT_HPP
#define PRINT_HPP

// This is used in my PC to have better looking errors
#ifndef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 0/1
#endif //_GLIBCXX_USE_CXX11_ABI

#include <iostream>
#include <iomanip>
#include <vector>
// #include <map>
// #include <array>

// Defining the different colors possible of the terminal
// Usage :  cout<< CoLib::Color::<COLOR> <<....
//          ...
//          cout << ... << CoLib::Color::RESET

namespace CoLib
{
  namespace Color
  {
    const char* RESET   = "\u001b[0m";
  
    const char* BLACK   = "\u001b[30m";
    const char* RED     = "\u001b[31m";
    const char* GREEN   = "\u001b[32m";
    const char* YELLOW  = "\u001b[33m";
    const char* BLUE    = "\u001b[34m";
    const char* MAGENTA = "\u001b[35m";
    const char* CYAN    = "\u001b[36m";
    const char* WHITE   = "\u001b[37m";
  
    const char* BRIGHTBLACK   = "\u001b[30;1m";
    const char* BRIGHTRED     = "\u001b[31;1m";
    const char* BRIGHTGREEN   = "\u001b[32;1m";
    const char* BRIGHTYELLOW  = "\u001b[33;1m";
    const char* BRIGHTBLUE    = "\u001b[34;1m";
    const char* BRIGHTMAGENTA = "\u001b[35;1m";
    const char* BRIGHTCYAN    = "\u001b[36;1m";
    const char* BRIGHTWHITE   = "\u001b[37;1m";
  
    const char* GREY = "\u001b[38;5;8m";
  }
}

#ifndef MULTITHREADING

/// @brief New line
void print() {std::cout << std::endl;}

template <class T>
std::ostream& operator<<(std::ostream& cout, std::vector<T> const & v)
{
  for (auto const & e : v) cout << e << " ";
  return cout;
}


/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T> 
void print(T const & t) {std::cout << t << std::endl;}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void print(T const & t, T2 const &... t2) {std::cout << t << " "; print(t2...);}


/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
void printC(T const & t) {std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void printC(T const & t, T2 const &... t2) {std::cout << t; printC(t2...);}


/// @brief Generic print separated by tabulation
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
void printT(T const & t) {std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void printT(T const & t, T2 const &... t2) {std::cout << t << "\t"; printT(t2...);}


/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T> 
void println(T const & t) {std::cout << t;}

/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T, class... T2> 
void println(T const & t, T2 const &... t2) {std::cout << t; println(t2...);}

/// @brief Set the floating point precision displayed.
void print_precision(int n = 6) {std::cout << std::setprecision(n);}

/// @brief Requires #define DEBUG or -D DEBUG in the compile line
template <class... ARGS> void debug(ARGS &&... args) 
{
#ifdef DEBUG
  print(std::forward<ARGS>(args)...);
#endif //DEBUG
}


#else

std::mutex print_mutex;

/// @brief New line
void print() {print_mutex.lock(); std::cout << std::endl; print_mutex.unlock();}

template <class T>
std::ostream& operator<<(std::ostream& cout, std::vector<T> const & v)
{
  for (auto const & e : v) cout << e << " ";
  return cout;
}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T> 
void print(T const & t)
{
  print_mutex.lock(); 
  std::cout << t << std::endl; 
  print_mutex.unlock();
}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void print(T const & t, T2 const &... t2) {print_mutex.lock(); std::cout << t << " "; print_mutex.unlock(); print(t2...);}


/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
void printC(T const & t) {print_mutex.lock(); std::cout << t << std::endl; print_mutex.unlock();}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void printC(T const & t, T2 const &... t2) {print_mutex.lock(); std::cout << t;  print_mutex.unlock(); printC(t2...);}


/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T> 
void println(T const & t) {print_mutex.lock(); std::cout << t; print_mutex.unlock();}

/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T, class... T2> 
void println(T const & t, T2 const &... t2) {print_mutex.lock(); std::cout << t; print_mutex.unlock();  println(t2...);}

/// @brief Set the floating point precision displayed.
void print_precision(int n = 6) {print_mutex.lock(); std::cout << std::setprecision(n); print_mutex.unlock();}

/// @brief Requires #define DEBUG or -D DEBUG in the compile line
template <class... ARGS> void debug(ARGS &&... args) 
{
#ifdef DEBUG
  print(std::forward<ARGS>(args)...);
#endif //DEBUG
}


/// @brief legacy
template<class... ARGS>
void printMT(ARGS &&... args) 
{
  print(std::forward<ARGS>(args)...);
}

#endif //MULTITHREADING


// Extracts the name of the types (overloaded for user defined objects):

template<class T>
std::string type_of(T const & t)
{
  return typeid(t).name();
}

// Specialized printing function :
/// @brief Print in bright black
template <class... T>
void warning(T const & ... t)
{
  std::cout << CoLib::Color::YELLOW;
  print(t...);
  std::cout << CoLib::Color::RESET;
}

/// @brief Print in red
template <class... T>
void error(T const & ... t)
{
  std::cout << CoLib::Color::RED;
  print(t...);
  std::cout << CoLib::Color::RESET;
}

/// @brief Print in grey
template <class... T>
void information(T const & ... t)
{
  std::cout << CoLib::Color::GREY;
  print(t...);
  std::cout << CoLib::Color::RESET;
}

std::string nicer_bool(bool const & some_bool)
{
  return ((some_bool) 
      ? (CoLib::Color::BLUE + std::string("true") + CoLib::Color::RESET) 
      : (CoLib::Color::RED  + std::string("false") + CoLib::Color::RESET)
  );
}

// template <class... T>
// void printHex(T &&... t) 
// {
//   print(std::hex, std::forward<T>(t)..., std::dec);
// }

class Hextech
{
public:
  template <class... T>
  static inline void printh(T &&... t)
  {
    std::cout << std::hex;
    print(t...);
    std::cout << std::dec;
  }
};

#endif //PRINT_HPP