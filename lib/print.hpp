#ifndef PRINT_HPP
#define PRINT_HPP

// This is used in my PC to have better looking errors
#ifndef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 0/1
#endif //_GLIBCXX_USE_CXX11_ABI

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

// Defining the different colors possible of the terminal
// Usage :  cout<< <COLOR> <<....
//          ...
//          cout << ... << RESET

#define RESET   "\u001b[0m"

#define BLACK   "\u001b[30m"
#define RED     "\u001b[31m"
#define GREEN   "\u001b[32m"
#define YELLOW  "\u001b[33m"
#define BLUE    "\u001b[34m"
#define MAGENTA "\u001b[35m"
#define CYAN    "\u001b[36m"
#define WHITE   "\u001b[37m"

#define BRIGHTBLACK   "\u001b[30;1m"
#define BRIGHTRED     "\u001b[31;1m"
#define BRIGHTGREEN   "\u001b[32;1m"
#define BRIGHTYELLOW  "\u001b[33;1m"
#define BRIGHTBLUE    "\u001b[34;1m"
#define BRIGHTMAGENTA "\u001b[35;1m"
#define BRIGHTCYAN    "\u001b[36;1m"
#define BRIGHTWHITE   "\u001b[37;1m"

#define GREY "\u001b[38;5;8m"

#ifndef MULTITHREADING

/// @brief New line
void print() {std::cout << std::endl;}

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

/// @brief Requires #define DEBUG or -DDEBUG in the compile line
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
void printC(T const & t) {std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
void printC(T const & t, T2 const &... t2) {print_mutex.lock(); std::cout << t;  print_mutex.unlock(); printC(t2...);}


/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T> 
void println(T const & t) {std::cout << t;}

/// @brief Generic print in one line
/// @details Concatenate the ouput, i.e. do not add space between each input. Do not terminate the output with a "\\n"
template <class T, class... T2> 
void println(T const & t, T2 const &... t2) {print_mutex.lock(); std::cout << t; print_mutex.unlock();  println(t2...);}

/// @brief Set the floating point precision displayed.
void print_precision(int n = 6) {std::cout << std::setprecision(n);}

/// @brief Requires #define DEBUG or -DDEBUG in the compile line
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

// Specialised printing function :
/// @brief Print in bright black
template <class... T>
void warning(T const & ... t)
{
  std::cout << YELLOW;
  print(t...);
  std::cout << RESET;
}

/// @brief Print in red
template <class... T>
void error(T const & ... t)
{
  std::cout << RED;
  print(t...);
  std::cout << RESET;
}

/// @brief Print in grey
template <class... T>
void information(T const & ... t)
{
  std::cout << GREY;
  print(t...);
  std::cout << RESET;
}

#endif //PRINT_HPP