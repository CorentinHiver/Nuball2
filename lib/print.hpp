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
// Usage :  cout<< Colib::Color::<COLOR> <<....
//          ...
//          cout << ... << Colib::Color::RESET

namespace Colib
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

#ifndef COMULTITHREADING

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
template <class... ARGS> void debug(ARGS &&... 
#ifdef DEBUG
  args
#endif //DEBUG
) 
{
#ifdef DEBUG
  print(std::forward<ARGS>(args)...);
#endif //DEBUG
}

#else

std::mutex print_mutex;

/// @brief New line
void print() {lock_mutex lock(print_mutex); std::cout << std::endl;}

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
  lock_mutex lock(print_mutex); 
  std::cout << t << std::endl; 
}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void print(T const & t, T2 const &... t2) {{lock_mutex lock(print_mutex); std::cout << t << " ";} print(t2...);}


/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
void printC(T const & t) {lock_mutex lock(print_mutex); std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void printC(T const & t, T2 const &... t2) {{lock_mutex lock(print_mutex); std::cout << t;} printC(t2...);}


/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T> 
void println(T const & t) {lock_mutex lock(print_mutex); std::cout << t;}

/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T, class... T2> 
void println(T const & t, T2 const &... t2) {{lock_mutex lock(print_mutex); std::cout << t;} println(t2...);}

/// @brief Set the floating point precision displayed.
void print_precision(int n = 6) {lock_mutex lock(print_mutex); std::cout << std::setprecision(n);}

/// @brief Requires #define DEBUG or -D DEBUG in the compile line
template <class... ARGS> void debug(ARGS &&... args) 
{
#ifdef DEBUG
  print(std::forward<ARGS>(args)...);
#endif //DEBUG
}

#endif //COMULTITHREADING

// Specialized printing function :
/// @brief Print in bright black
template <class... T>
void warning(T const & ... t)
{
  std::cout << Colib::Color::YELLOW;
  print(t...);
  std::cout << Colib::Color::RESET;
}

/// @brief Print in red
template <class... T>
void error(T const & ... t)
{
  std::cout << Colib::Color::RED;
  print(t...);
  std::cout << Colib::Color::RESET;
}

/// @brief Print in grey
template <class... T>
void information(T const & ... t)
{
  std::cout << Colib::Color::GREY;
  print(t...);
  std::cout << Colib::Color::RESET;
}

std::string nicer_bool(bool const & some_bool)
{
  return ((some_bool) 
      ? (Colib::Color::BLUE + std::string("true") + Colib::Color::RESET) 
      : (Colib::Color::RED  + std::string("false") + Colib::Color::RESET)
  );
}

template <size_t __length__, char fillWith = '0', class T>
void printfill(T const & t){
  std::cout << std::setfill(fillWith) << std::setw(__length__) << t;
}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <size_t __length__, char fillWith = '0', class T, class... T2> 
void printfill(T const & t, T2 const &... t2) {std::cout << std::setfill(fillWith) << std::setw(__length__) << t << " "; printfill<__length__, fillWith>(t2...);}

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

  template <size_t __length__, class... T>
  static inline void printhfill0(T &&... t)
  {
    std::cout << std::hex;
    printfill<__length__>(t...);
    std::cout << std::dec << std::endl;
  }

  template <size_t __length__, char fillWith = '0', class... T>
  static inline void printhfill(T &&... t)
  {
    std::cout << std::hex;
    printfill<__length__, fillWith>(t...);
    std::cout << std::dec << std::endl;
  }
};

#endif //PRINT_HPP