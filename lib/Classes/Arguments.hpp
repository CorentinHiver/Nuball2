#pragma once

#include "../string_functions.hpp"
#include "../vector_functions.hpp"
#include "../print.hpp"

class Arguments
{
public:
  Arguments() noexcept = default;
  Arguments(int argc, char** argv) :
    m_argc(argc),
    m_argv(argv),
    m_args(Colib::argv_to_string(argv)),
    m_iss(m_args.data())
  {
    next(); // Skipping the first argument because it is the name of the executable
  }

  void set(int argc, char** argv)
  {
    m_iss.clear();
    m_argc = argc;
    m_argv = argv;
    m_args = Colib::argv_to_string(argv);
    m_iss.str(m_args.data());
    next(); // Skipping the first argument because it is the name of the executable
  }

  bool next()
  {
    if (m_argCursor++ < m_argc)
    {
      m_iss >> m_argument;
      return true;
    }
    else return false;
  }

  template<class T>
  T& load(T & t) 
  {
    if (m_argCursor++ < m_argc)
    {
      m_iss >> t;
      return t;
    }
    else throw MissingArg(m_argument);
  }

  template<class T>
  T load()
  {
    if (m_argCursor++ < m_argc)
    {
      T t;
      m_iss >> t;
      return t;
    }
    else throw MissingArg(m_argument);
  }

  template<class T>
  Arguments& operator>>(T & t) 
  {
    if (m_argCursor++ < m_argc)
    {
      m_iss >> t;
      return *this;
    }
    else throw MissingArg(m_argument);
  }

  bool operator==(std::string const & argument) {return m_argument == argument;}
  operator std::string() const & {return m_argument;}
  std::string const & getArg() const {return m_argument;}
  auto const & getArgc() const {return m_argc;}
  auto const & getArgv() const {return m_argv;}
  int nbRemainingArgs() const {return m_argc - m_argCursor;}
  friend std::ostream& operator<<(std::ostream& out, Arguments const & arg) {out << static_cast<std::string>(arg); return out;}
  int size() const {return m_argc-1;}
  
private:
  // Arguments parameters:
  int m_argc;
  char** m_argv = nullptr;
  std::string m_args;
  std::istringstream m_iss;
  std::string m_argument;

  // State attributes:
  int m_argCursor = 0;

public:
  struct MissingArg
  {
    // MissingArg() noexcept = default;
    MissingArg(std::string const & argument, bool _print = false) : 
      message(argument+" needs at least another parameter !") 
    {
      if (_print) print();
    }
    void print() {error(message);}
    std::string message;
  };
};



// #include <any>
  // auto const & get() {return m_argument;}
  // template<typename T>
  // void setArg(std::string const & argument, T & value) 
  // {
  //   m_arguments.push_back(argument);
  //   m_parameters.push_back(default_value);
  // }

  // void print()
  // {
  //   for (size_t i = 0; i<m_arguments.size(); ++i) 
  //   // print(m_arguments[i], m_parameters[i]);
  //   std::cout 
  //   << m_arguments[i] 
  //   // << " " << m_parameters[i]
  //   ;
  // }

  // void process()
  // {
  //   while(next())
  //   {
  //     auto arg_place = Colib::findIndex(m_arguments, m_argument);
  //     if (arg_place < m_arguments.size()) 
  //       for (int i = 0; i<m_nbParams[arg_place]; ++i) 
  //         load(m_parameters[arg_place]);
  //   }
  // }

  
  // std::vector<std::string> m_arguments;
  // std::vector<std::any> m_parameters;
  // std::vector<int> m_nbParams;
