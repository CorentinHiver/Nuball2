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

// #include "boost/array.hpp"

////////////////
//   TYPEDEF  //
////////////////

typedef unsigned short int  ushort;
typedef unsigned char       uchar ;
typedef unsigned int        uint  ;


///////////////////////
//   TEMPLATE PRINT  //
///////////////////////

// Generic print :

template <class T> void print()
{
  std::cout << std::endl;
}

template <class T> void print(T const & t)
{
  std::cout << t << std::endl;
}

template <class E> void print (std::vector<E> const & t)
{
  for (auto const & e : t) print(e);
}

template <class... T> void print(T const &... t)
{
  ((print(t)),...);
}

template <class T, class... T2> void print(T const & t, T2 const &... t2)
{
  std::cout << t << " ";
  print(t2...);
}

//Prints containers :

template <class E, class... T> void print (std::vector<E> const & v, T const &... t2)
{
  print(v);
  print(t2...);
}

template <class K, class V> void print (std::map<K,V> const & m)
{
  print();
  print("{");
  print();
  for (auto const & pair : m)
  {
    print("key : ");
    print(pair.first);
    // print();
    print("value :");
    print(pair.second);
    print();
  }
  print("}");
  print();
}

template <class K, class V, class... T> void print (std::map<K,V> const & m, T const & ... t2)
{
  print(m);
  print(t2...);
}

// Class Debug, based on template print() :
class Debug
{
public:
  template <class... T> Debug(T... t) { (*this)(t...); }

  template <class... T> void operator () (T... t)
  {
    if (sizeof...(t) == 0)
    {
      print("coucou ", i);
      i++;
      return;
    }
    print(t...);
    std::cout << std::endl;
  }

  void operator () (int _i)
  {
    std::cout << "coucou " << _i << std::endl;
    i = _i+1;
  }
private:
  int i = 0;
};

//////////////////
//   FUNCTIONS  //
//////////////////

//-----------------------------------------//
//       General string manipulations      //
//-----------------------------------------//

std::string firstPart       (const std::string string, const char sep) { return (string.substr(0, string.find_first_of(sep) ));  }
std::string lastPart        (const std::string string, const char sep) { return (string.substr(   string.find_last_of(sep)+1));  }
std::string removeFirstPart (const std::string string, const char sep) { return (string.substr(   string.find_first_of(sep) ));  }
std::string removeLastPart  (const std::string string, const char sep) { return (string.substr(0, string.find_last_of(sep)  ));  }
std::string removeBlankSpace(std::string str)
{ //  In a std::string, removes blank spaces
	int pos = 0;
	while ( (pos = (int)str.find(" ")) != -1)
	{
		str = str.substr(0,pos) + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}

std::string rpCommaWDots(std::string str)
{//  In a std::string, replaces all commas with dots
	int pos = 0;
	while ( (pos = (int)str.find(",")) != -1)
	{
		str = str.substr(0,pos) + "." + str.substr(pos+1,str.size()-pos-1);
	}
	return str;
}

bool isNumber(std::string const & string)
{
  for (auto const & c : string)
  {
    if (!isdigit(c)) return false;
  }
  return true;
}


//----------------------------------------------------//
//       General files and folders manipulations      //
//----------------------------------------------------//

std::string removeExtension (const std::string string) { return (string.substr(0, string.find_last_of(".")  ));  }
std::string extension       (const std::string string) { return (string.substr(   string.find_last_of(".")+1));  }
std::string getPath         (const std::string string) { return (string.substr(0, string.find_last_of("/")+1));  }
std::string removePath      (const std::string string) { return (string.substr(   string.find_last_of("/")+1));  }
std::string rmPathAndExt    (const std::string string)
                            {return (string.substr     ( string.find_last_of("/")+1, string.find_last_of(".")-string.find_last_of("/")-1)); }

bool file_is_empty(std::ifstream& file)                { return file.peek() == std::ifstream::traits_type::eof();}

std::map<std::string, float> size_file_unit =
{
  {"o",  1.},
  {"ko", 1024.},
  {"Mo", 1048576.},
  {"Go", 1073741824.},
  {"B",  1.},
  {"kB", 1024.},
  {"MB", 1048576.},
  {"GB", 1073741824.}
};

float size_file_conversion(float const & size, std::string const & unit_i, std::string const & unit_o)
{
  return size * (size_file_unit[unit_o]/size_file_unit[unit_i]);
}

float size_file(std::ifstream& file, std::string const & unit = "o")
{
  int const init = file.tellg();
  file.seekg(0, std::ios::end);
  int const ret = file.tellg();
  file.seekg(init);// Go back to inital place in the file
  return ret/size_file_unit[unit];
}

int size_file(std::string filename, std::string const & unit = "o")
{
  std::ifstream f (filename, std::ios::binary);
  return size_file(f, unit);
}

bool file_exists(std::string fileName)
{
  std::string path = getPath(fileName);
  std::string name = removePath(fileName);
  struct dirent *file = nullptr;
  DIR *dp = nullptr;
  dp = opendir(path.c_str());
  if(dp == nullptr) return false;
  else
  {
    while ((file = readdir(dp)))
    {
      if (!strcmp(file -> d_name, name.c_str()))
      {
        closedir(dp);
        return true;
      }
    }
  }
  closedir(dp);
  return false;
}

std::string & makePath(std::string & folderName)
{
  if (folderName.back() != '/') folderName.push_back('/');
  return folderName;
}

bool folder_exists(std::string folderName)
{
  makePath(folderName);
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  bool ret = (dp != nullptr);
  std::string str = ((dp!=nullptr) ? "oui" : "non");
  closedir(dp);
  return ret;
}

bool folder_exists(std::string folderName, bool const & verbose)
{
  makePath(folderName);
  if (folder_exists(folderName)) return true;
  if (verbose) std::cout << "Folder " << folderName << " not found..." << std::endl;
  return false;
}

void create_folder_if_none(std::string const & folderName)
{
  if (folderName=="")
  {
    print("No folder asked for !");
    return;
  }
  if(!folder_exists(folderName))
  {
    print("Creating folder", folderName);
    system(("mkdir "+folderName).c_str());
  }
}

int nb_files_in_folder(std::string & folderName)
{
  int ret = -1;
  makePath(folderName);
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  if(dp == nullptr) ret = -1;
  else
  {
    int i = 0;
    while ((readdir(dp))) i++;
    ret = i;
  }
  closedir(dp);
  return ret;
}

std::string get_filename_at(std::string & folderName, int pos)
{
  std::string ret;
  makePath(folderName);
  struct dirent *file = nullptr;
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  if(dp == nullptr) ret = "";
  else
  {
    int i = 0;
    while ((file = readdir(dp)) && i<pos) i++;
  }
  ret = file -> d_name;
  closedir(dp);
  return ret;
}

int check_new_file(std::string & folderName, std::string & lastFile)
{
  int ret = -1;
  makePath(folderName);
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  struct dirent *file = nullptr;
  if(dp == nullptr) ret = -1;
  else
  {
    int i = 0;
    while ((file = readdir(dp)))
    {
      lastFile = file -> d_name;
      i++;
    }
    ret = i;
  }
  closedir(dp);
  return ret;
}

std::vector<std::string> listFileReader(std::string const & filename)
{
  std::vector<std::string> list;

  std::ifstream listfile(filename,std::ios::in);
  if(!listfile.good())
  {
    print("List file", filename, "not found !");
  }
  else
  {
    std::string line;
    while(getline(listfile,line))
    {
      list.push_back(line);
    }
  }
  return list;
}

std::vector<std::string> findFilesWildcard(std::string const & expression)
{
  std::vector<std::string> ret;

  glob_t result;
  if (glob(expression.c_str(), GLOB_TILDE, NULL, &result) == 0)
  {
    for (size_t i = 0; i < result.gl_pathc; ++i)
    {
      ret.push_back(result.gl_pathv[i]);
    }
    globfree(&result);
  }
  return ret;
}

void findFilesWildcard(std::string const & expression, std::vector<std::string> & vec)
{
  auto const files = findFilesWildcard(expression);
  for (auto const & file : files) vec.push_back(file);
}

template <class N, class D> std::string procent(N const & n, D const & d)
{
  return (std::to_string(100*static_cast<double>(n)/static_cast<double>(d))+"%");
}


class Path
{
public:
  Path(std::string const & inputString, bool create = false) : m_path(inputString)
  {
    makePath(m_path);
    m_exists = folder_exists(m_path);
    if (!m_exists && create) create_folder_if_none(m_path);
    if (!folder_exists(m_path))
    {
      print(m_path,"doesn't exist !!");
      m_exists = false;
    }
  }

  int nbFiles() {return nb_files_in_folder(m_path);}
  bool exists() {return folder_exists(m_path);}
  bool create() {create_folder_if_none(m_path); return this -> exists();}
  
  operator std::string() const & {return m_path;}

  Path & operator=(std::string const & inputString) 
  {
    m_path = inputString;
    makePath(m_path);
    m_exists = folder_exists(m_path);
    if (!folder_exists(m_path))
    {
      print(m_path,"doesn't exist !!");
      m_exists = false;
    }
    return *this;
  }

private:
  bool m_exists = false;
  std::string m_path;
};

using Folder = Path;


//////////////////////////////
//   VECTORS MANIPULATIONS  //
//////////////////////////////

template <typename T>
bool push_back_unique(std::vector<T> & vector, T const & t)
{
  if (std::find(std::begin(vector), std::end(vector), t) == std::end(vector))
  {
    vector.push_back(t);
    return true;
  }
  else
  {
    return false;
  }
}


////////////////////////////
//   CLASS STATIC VECTOR  //
////////////////////////////

template<class T, std::size_t __size__ = 0>
class StaticVector
{
public:
  StaticVector() : m_static_size(__size__) {m_data = new T[m_static_size];}
  StaticVector(T const & value);
  StaticVector(StaticVector<T, __size__> const & vector);
  ~StaticVector(){delete[] m_data;}

  void resize(std::size_t const & size = 0) {m_dynamic_size = size;}
  void static_resize(std::size_t const & size = 0)
  {
    delete[] m_data;
    m_dynamic_size = 0;
    m_static_size = size;
    m_data = new T[m_static_size];
  }

  virtual bool has(T const & t);

  void push_back(T const & e) {m_data[m_dynamic_size++] = e;}
  void push_back_safe(T const & e)
  {
    if (m_dynamic_size++ < m_static_size) m_data[m_dynamic_size] = e;
    else std::cout << "Capacity StaticVector<" << typeid(T).name() << "," << m_static_size << "> exceeded" << std::endl;
  }
  void push_back_unique(T const & e);

  virtual T* begin(){return m_data;}
  virtual T* end()  {return m_data+m_dynamic_size;}

  auto const & size() const {return m_dynamic_size;}
  T & operator[] (std::size_t const & i) const {return m_data[i];}
  T const & at(std::size_t const & i) const {if (m_dynamic_size < m_static_size) return m_data[i]; else return m_data[0];}
  T* data() {return m_data;}

private:
  T *m_data;
  std::size_t m_dynamic_size = 0;
  std::size_t m_static_size = 0;
};

template<class T, std::size_t __size__>
StaticVector<T,__size__>::StaticVector(T const & value) : m_static_size(__size__)
{
  m_data = new T[m_static_size];
  for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = value;
}

template<class T, std::size_t __size__>
StaticVector<T,__size__>::StaticVector(StaticVector<T, __size__> const & vector) : m_static_size(__size__)
{
  m_data = new T[m_static_size];
  for (std::size_t i = 0; i<m_static_size; i++) m_data[i] = vector.at(i);
}

template<class T, std::size_t __size__>
bool inline StaticVector<T,__size__>::has(T const & t)
{
  return (std::find(this -> begin(), this -> end(), t) != this -> end());
}

template<class T, std::size_t __size__>
void StaticVector<T,__size__>::push_back_unique(T const & t)
{
#ifdef SAFE
  if (!this->has(t)) this -> push_back_safe(t);
#else
  if (!this->has(t)) this -> push_back(t);
#endif //SAFE
}

template<class T, std::size_t __size__ = 0>
class StaticOrderVector : public StaticVector<T, __size__>
{// Binary search works only with
public:
  bool has(T const & t)
  {
    return std::binary_search(this -> begin(), this -> end(), t);
  }
};

#endif //LIB_H_CO
