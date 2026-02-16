#ifndef PRINT_HPP
#define PRINT_HPP

// This is used in my PC to have better looking errors
#ifndef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 0/1
#endif //_GLIBCXX_USE_CXX11_ABI

#include <iostream>
#include <iomanip>
#include <vector>

/// Stream vector support
template <class T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) noexcept
{
    for (const auto& e : v)
        os << e << " ";
    return os;
}


template <class T>
std::ofstream& operator<<(std::ofstream& fout, std::vector<T> const & v) noexcept
{
  for (auto const & e : v) fout << e << " ";
  return fout;
}


namespace Colib
{
  namespace Color
  {
    constexpr char RESET[]   = "\u001b[0m";
  
    constexpr char BLACK[]   = "\u001b[30m";
    constexpr char RED[]     = "\u001b[31m";
    constexpr char GREEN[]   = "\u001b[32m";
    constexpr char YELLOW[]  = "\u001b[33m";
    constexpr char BLUE[]    = "\u001b[34m";
    constexpr char MAGENTA[] = "\u001b[35m";
    constexpr char CYAN[]    = "\u001b[36m";
    constexpr char WHITE[]   = "\u001b[37m";
  
    constexpr char BRIGHTBLACK[]   = "\u001b[30;1m";
    constexpr char BRIGHTRED[]     = "\u001b[31;1m";
    constexpr char BRIGHTGREEN[]   = "\u001b[32;1m";
    constexpr char BRIGHTYELLOW[]  = "\u001b[33;1m";
    constexpr char BRIGHTBLUE[]    = "\u001b[34;1m";
    constexpr char BRIGHTMAGENTA[] = "\u001b[35;1m";
    constexpr char BRIGHTCYAN[]    = "\u001b[36;1m";
    constexpr char BRIGHTWHITE[]   = "\u001b[37;1m";
  
    constexpr char GREY[] = "\u001b[38;5;8m";
  }

  namespace Terminal
  {
    constexpr char CLEAR_ROW[] = "\033[2K\r";
  }
}

#if defined(MT_AVAILABLE) || defined (COMULTITHREADING)
inline std::mutex print_mutex;

using lock_mutex = std::lock_guard<std::mutex>;

/// Print newline only
inline void print() noexcept
{
  lock_mutex lock(print_mutex);
  std::cout << '\n';
}

/// Print with space separation + newline
template <class... Args>
void print(const Args&... args) noexcept
{
  lock_mutex lock(print_mutex);
  ((std::cout << args << ' '), ...);
  std::cout << '\n';
}

/// Concatenated print + newline
template <class... Args>
void printC(const Args&... args) noexcept
{
  lock_mutex lock(print_mutex);
  ((std::cout << args), ...);
  std::cout << '\n';
}

/// Concatenated print, no newline
template <class... Args>
void println(const Args&... args) noexcept
{
  lock_mutex lock(print_mutex);
  ((std::cout << args), ...);
}

/// Set floating precision
inline void print_precision(int n = 6) noexcept
{
  lock_mutex lock(print_mutex);
  std::cout << std::setprecision(n);
}

/// Debug print (requires -DDEBUG)
template <class... Args>
void debug(Args&&... args) noexcept
{
#ifdef DEBUG
  print(std::forward<Args>(args)...);
#endif
}

namespace Colib
{
  static std::mutex cout_mutex;
}

/// @brief Print same line
/// @details Automatically adds space between each input. Do not terminate the output with a "\\n". 
/// Go back at the beginning of the line.
template <class... Ts> 
void printsln(Ts &&... ts)  noexcept
{
  std::lock_guard<std::mutex> lock(Colib::cout_mutex);  // Lock to ensure atomic output.
  std::cout << "\r"; 
  println(std::forward<Ts>(ts)...); 
  std::cout << std::flush;
}

template <class... Ts> 
void printslnt(Ts &&... ts)  noexcept
{
  int thread_idx = Colib::MT::getThreadIndex();  // Assuming this returns 0 to Nthreads-1.
  std::lock_guard<std::mutex> lock(Colib::cout_mutex);
  std::cout << "[" << thread_idx << "] ";
  std::cout << "\033[" << (thread_idx + 1) << ";1H\r";  // Position to row (thread_idx + 1), column 1, then carriage return.
  println(std::forward<Ts>(ts)...); 
  std::cout << std::flush;
  // Optionally, reposition cursor back to a neutral spot, like the bottom: std::cout << "\033[" << (Nthreads + 1) << ";1H";
}

#else


/// @brief New line
void print() noexcept {std::cout << std::endl;}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T> 
constexpr void print(T const & t) noexcept {std::cout << t << std::endl;}

/// @brief Generic print
/// @details Automatically adds space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
constexpr void print(T const & t, T2 const &... t2) noexcept {std::cout << t << " "; print(t2...);}


/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
constexpr void printC(T const & t) noexcept {std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
constexpr void printC(T const & t, T2 const &... t2) noexcept {std::cout << t; printC(t2...);}


/// @brief Generic print separated by tabulation
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T> 
constexpr void printT(T const & t) noexcept {std::cout << t << std::endl;}

/// @brief Generic print concatenated
/// @details Concatenate the ouput, i.e. do not add space between each input. Terminate the output with a "\\n"
template <class T, class... T2> 
constexpr void printT(T const & t, T2 const &... t2) noexcept {std::cout << t << "\t"; printT(t2...);}


/// @brief Generic print in one line
/// @details Automatically adds space between each input. Do not terminate the output with a "\\n"
template <class T> 
constexpr void println(T const & t) noexcept {std::cout << t;}

/// @brief Generic print in one line
/// @details Automatically adds space between each input. Do not terminate the output with a "\\n"
template <class T, class... T2> 
constexpr void println(T const & t, T2 const &... t2) noexcept {std::cout << t << " "; println(t2...);}

/// @brief Set the floating point precision displayed.
void print_precision(int n = 6) noexcept {std::cout << std::setprecision(n);}

/// @brief Print same line
/// @details Automatically adds space between each input. Do not terminate the output with a "\\n". 
/// Go back at the beginning of the line.
template <class... Ts> 
constexpr void printsln(Ts &&... ts) noexcept 
{
  std::cout << Colib::Terminal::CLEAR_ROW; 
  println(std::forward<Ts>(ts)...); 
  std::cout << std::flush;
}

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

#endif //MT_AVAILABLE

// Specialized printing function :
/// @brief Print in bright black
template <class... T>
constexpr void warning(T const & ... t) noexcept
{
  std::cout << Colib::Color::YELLOW;
  print(t...);
  std::cout << Colib::Color::RESET;
}

/// @brief Print in red
template <class... T>
constexpr void error(T const & ... t) noexcept
{
  std::cout << Colib::Color::RED;
  print(t...);
  std::cout << Colib::Color::RESET;
}

/// @brief Print in grey
template <class... T>
constexpr void information(T const & ... t) noexcept
{
  std::cout << Colib::Color::GREY;
  print(t...);
  std::cout << Colib::Color::RESET;
}

constexpr std::string nicer_bool(bool const some_bool) noexcept
{
    using namespace Colib::Color;

    if (some_bool) return (std::string{BLUE} + "true " + RESET);
    else           return (std::string{RED } + "false" + RESET);
}

/// @brief Prints a fixed-size output filled with 0 where t is not printed
template <size_t __length__, char fillWith = '0', class T>
constexpr void printfill(T const & t) noexcept {
  std::cout << std::setfill(fillWith) << std::setw(__length__) << t;
}

/// @brief Prints a fixed-size output filled with 0 where t is not printed
template <size_t __length__, char fillWith = '0', class T, class... T2> 
constexpr void printfill(T const & t, T2 const &... t2) noexcept {std::cout << std::setfill(fillWith) << std::setw(__length__) << t << " "; printfill<__length__, fillWith>(t2...);}

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