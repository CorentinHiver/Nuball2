#ifndef FILES_HPP
#define FILES_HPP

#include "print.hpp"
#include "string_functions.hpp"

//----------------------------------------------------//
//       General files and folders manipulations      //
//----------------------------------------------------//

std::string removeExtension (const std::string string) { return (string.substr(0, string.find_last_of(".")  ));  }
std::string extension       (const std::string string) { return (string.substr(   string.find_last_of(".")+1));  }
std::string getExtension    (const std::string string) { return (string.substr(   string.find_last_of(".")+1));  }
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
  Path(){}
  Path(Path const & path) : m_exists(path.m_exists), m_path(path.m_path) {}
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

  static Path get(std::string const & file) {return Path(getPath(file));}

  std::string const & path() const {return m_path;}
  
  operator std::string() const & {if (!m_exists) print("Carefull, you manipulate not existing folder"); return m_path;}
  auto c_str() {return m_path.c_str();}

  Path & operator+(std::string const & addString)
  {
    std::string temp_path = m_path+addString;
    return ((*this) = temp_path);
  }

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

  Path & operator=(Path & path) 
  {
    m_path = path.m_path;
    m_exists = path.m_exists;
    return *this;
  }

  void operator+=(std::string const & addString)
  {
    if (addString.back() == '/') print("You have to add a path to", m_path, ",not a file.");
    else m_path+=addString;
  }

  bool operator==(std::string const & comprString)
  {
    return (comprString == m_path);
  }

private:
  bool m_exists = false;
  std::string m_path;
};

Path operator+(std::string const & string, Path const & _path)
{
  return (string+_path.path());
}

using Folder = Path;

template <class... T> void print (Path const & p)
{
  std::cout << p.path();
}

template <class... T> void print (Path const & f, T const &... t2)
{
  print(f);
  print(t2...);
}

class Filename
{
public:
  Filename(){}
  Filename(std::string const & _filename) : m_filename(_filename) 
  {
    this -> fill(_filename);
  }

  Filename(Filename const & _filename) : m_filename(_filename.m_filename), m_name(_filename.m_name), m_extension(_filename.m_extension) {}

  Filename & operator=(Filename const & _filename)
  {
    m_filename = _filename.filename();
    m_name = _filename.name();
    m_extension = _filename.extension();
    return *this;
  }

  Filename & operator=(std::string const & _filename)
  {
    m_filename = _filename;
    this -> fill(_filename);
    return *this;
  }

  std::string const & filename() const {return m_filename;}
  std::string const & name() const {return m_name;}
  std::string const & extension() const {return m_extension;}

private:

  void fill(std::string const & filename)
  {
    m_name = removeExtension(filename);
    m_extension = getExtension(filename);
  }
  std::string m_filename;
  std::string m_name;
  std::string m_extension;
};

class File
{
public:
  File(std::string const & file) : m_file(file)
  {
    m_path = getPath(file);
    if (m_path.exists())
    {
      m_name = file;
      if (file_exists(m_file)) m_exists = true;
      else print("The file", file, "is unreachable in folder", m_path);
    }
    else
    {
      print("The path of the file", file, "is unreachable");
    }
  }
  
  File(Path const & path, Filename const & filename)
  {
    m_path = path;
    m_name = filename;
  }

  auto c_str() {return m_path.c_str();}

  File & operator=(std::string const & file) 
  {
    m_path = Path(file);
    return *this;
  }

  std::string const & file() const {return m_file;}
  Filename    const & name() const {return m_name;}
  Path        const & path() const {return m_path;}

private:
  bool m_exists = false;
  std::string m_file; // Full path + name + extension
  Filename m_name; // Name + extension
  Path m_path; // Path
};

template <class... T> void print (File const & f)
{
  print(f.file());
}

template <class... T> void print (File const & f, T const &... t2)
{
  print(f);
  print(t2...);
}



#endif //FILES_HPP