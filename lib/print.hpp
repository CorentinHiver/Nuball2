#ifndef PRINT_HPP
#define PRINT_HPP

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

// Defining the different colors possible of the terminal
// Usage :  cout<<COLOR<<....
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

// Generic print :

template <class T> void print()
{
  std::cout << std::endl;
}

#ifndef __CINT__
template <class T> void print(T const & t)
{
  std::cout << t << std::endl;
}
#endif //__CINT__

template <class... T> void print(T const &... t)
{
  ((print(t)),...);
}

template <class T, class... T2> void print(T const & t, T2 const &... t2)
{
  std::cout << t << " ";
  print(t2...);
}


// Useful overload of operator<< into a std::cout stream :

template <class E>
std::ostream& operator<<(std::ostream& cout, std::vector<E> const & v)
{
  for (auto const & e : v) cout << e << " ";
  return cout;
}

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

template<class E, size_t size> 
std::ostream& operator<<(std::ostream& cout, std::array<E,size> const & a)
{
  for (size_t i = 0; i<size; i++) print(a[i]);
  return cout;
}

void print_precision(int n = 6) {std::cout << std::setprecision(n);}

template <class... ARGS> void debug(ARGS &&... args) 
{
#ifdef DEBUG
  print(std::forward<ARGS>(args)...);
#endif //DEBUG
}

// Print specialization for char and uchar : 

std::ostream& operator<<(std::ostream& cout, unsigned char const & uc)
{
  std::cout << static_cast<int>(uc);
  return cout;
}

// Printing the name of the types :

template<class T>
std::string type_of(T const & t)
{
  return typeid(t).name();
}

// Specialised printing function :

template <class... T>
void warning(T const & ... t)
{
  print(RED, t..., RESET);
}

template <class... T>
void information(T const & ... t)
{
  print(GREY, t..., RESET);
}

#endif //PRINT_HPP