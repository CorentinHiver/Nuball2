#ifndef FILES_HPP
#define FILES_HPP

#include "print.hpp"
#include "string_functions.hpp"

//----------------------------------------------------//
//       General files and folders manipulations      //
//----------------------------------------------------//

std::string removeExtension (std::string const & string) { return (string.substr(0, string.find_last_of(".")  ));  }
std::string extension       (std::string const & string) { return (string.substr(   string.find_last_of(".")+1));  }
std::string getExtension    (std::string const & string) { return (string.substr(   string.find_last_of(".")+1));  }
std::string getPath         (std::string const & string) { return (string.substr(0, string.find_last_of("/")+1));  }
std::string removePath      (std::string const & string) { return (string.substr(   string.find_last_of("/")+1));  }

std::string rmPathAndExt(std::string const & string)
{
  return (string.substr( string.find_last_of("/")+1, string.find_last_of(".")-string.find_last_of("/")-1)); 
}

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

std::string & makeFolder(std::string & folderName)
{
  if (folderName.back() != '/') folderName.push_back('/');
  return folderName;
}

bool folder_exists(std::string folderName)
{
  makeFolder(folderName);
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  bool ret = (dp != nullptr);
  std::string str = ((dp!=nullptr) ? "oui" : "non");
  closedir(dp);
  return ret;
}

bool folder_exists(std::string folderName, bool const & verbose)
{
  makeFolder(folderName);
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
  makeFolder(folderName);
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
  makeFolder(folderName);
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
  makeFolder(folderName);
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

/**
 * @brief Object used to hold a folder's name
 */
class Folder
{
public: 

  Folder(){}

  /**
   * @brief Turns a string to a folder's name.
   * 
   * Basically, it is simply ensured that the name ends with a '/'
   * 
   * Also, it is the base class of Path class
  */
  Folder(std::string const & folder) : m_folder (folder)
  {
    make(m_folder);
  }

  Folder(const char * folder) : m_folder (std::string(folder))
  {
    make(m_folder);
  }

  Folder & operator=(std::string const & folder)
  {
    m_folder = folder;
    make(m_folder);
    return *this;
  }

  Folder & operator=(const char * folder)
  {
    m_folder = folder;
    make(m_folder);
    return *this;
  }

  operator std::string() const & {return m_folder;}
  operator bool() const & {return m_ok;}
  std::string const & get() const {return m_folder;}
  std::string name() const {auto ret = m_folder; ret.pop_back(); return ret;}

  static void make(std::string & folder)
  {
    if (folder.size() == 0) m_ok = false;
    else
    {
      if (folder.back() != '/') folder.push_back('/');
      m_ok = true;
    }
  }
  

private:
  bool m_ok = false;
  std::string m_folder;
};

Folder operator+(std::string string, Folder const & folder)
{
  return Folder(string + folder.get());
}

template <class... T> void print (Folder const & f)
{
  std::cout << f.get() << std::endl;
}

template <class... T> void print (Folder const & f, T const &... t2)
{
  print(f);
  print(t2...);
}

/**
 * @brief Object used to hold a list of folders
 */
class Folders
{
public:

  Folders(){}

  Folders(std::vector<std::string> const & folders)
  {
    for (auto const & folder : folders) m_folders.push_back(Folder(folder));
  }

  operator std::vector<Folder>() const &  {return m_folders;}
  std::vector<Folder> const & get() const {return m_folders;}

private:
  std::vector<Folder> m_folders;
};

template <class... T> void print (Folders const & f)
{
  print(f.get());
}


/**
 * @brief Object used to hold the complete path of a giver folder
 * 
 * Absolute path only
 * 
 * You can use either a full path from the root ("/.../.../") or from the home directory ("~/.../.../")
 * 
 */
class Path
{
public:
  Path(){}
  Path(Path const & path) : m_exists(path.m_exists), m_path(path.m_path) {}

  /**
   * @brief Turns a string to a path, creating it if create = true and it doesn't already exists
  */
  Path(std::string const & path, bool create = false) : m_path(path)
  {
    if (m_path[0]=='/')
    {// Absolute path
    }
    else if (m_path[0]=='~')
    {// Home path
      m_path = std::string(std::getenv("HOME"))+m_path;
    }
    else
    {// Relative path
      print("Relative paths aren't supported, sorry !");
      m_exists = false;
    }

    makeFolder(m_path);
    m_exists = folder_exists(m_path);

    // getList

    if (!m_exists && create) create_folder_if_none(m_path);
    if (!folder_exists(m_path))
    {
      throw std::runtime_error(m_path+" doesn't exist !!");
      m_exists = false;
    }
  }

    /**
   * @brief Turns a C string to a path, creating it if create = true and it doesn't already exists
  */
  Path(const char* c_str, bool create = false) : m_path(std::string(c_str))
  {
    if (m_path[0]=='/')
    {// Absolute path
    }
    else if (m_path[0]=='~')
    {// Home path
      m_path = std::string(std::getenv("HOME"))+m_path;
    }
    else
    {// Relative path
      print("Relative paths aren't supported, sorry !");
      m_exists = false;
    }

    makeFolder(m_path);
    m_exists = folder_exists(m_path);

    // getList

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

  static Folders extractFolders(Path & path)
  {
    return getList(path.m_path,'/');
  }

  static Path make(std::string const & file) {return Path(getPath(file));}

  std::string const & path() const {return m_path;}
  
  operator std::string() const & {if (!m_exists) print("Carefull, you manipulate not existing folder"); return m_path;}
  auto c_str() {return m_path.c_str();}

  std::string operator+(std::string const & addString)
  {
    return (m_path+addString);
  }

  std::string operator+(const char* addString)
  {
    return (m_path+static_cast<std::string>(addString));
  }

  Path operator+(Folder const & folder)
  {
    return Path(m_path+folder);
  }

  Path & operator=(std::string const & inputString) 
  {
    m_path = inputString;
    makeFolder(m_path);
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

  Path & operator=(const char* path) 
  {
    m_path = path;
    m_exists = folder_exists(m_path);
    return *this;
  }

  Path & operator+=(std::string const & addString)
  {
    auto str = addString;
    makeFolder(str);
    m_path+=str;
    return *this;
  }

  bool operator==(std::string const & comprString)
  {
    return (comprString == m_path);
  }

private:
  bool m_exists = false;
  std::string m_path;
  Folders m_recursive_folders;
};

template <class... T> void print (Path const & p)
{
  std::cout << p.path() << std::endl;
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

  operator std::string() const & {return m_filename;}

  std::string const & filename() const {return m_filename;}
  std::string const & name() const {return m_name;}
  std::string const & extension() const {return m_extension;}
  Path const & path() const {return m_path;}

private:

  void fill(std::string const & filename)
  {
    m_name = rmPathAndExt(filename);
    m_extension = getExtension(filename);
    m_path = getPath(filename);
  }
  std::string m_filename;
  std::string m_name;
  std::string m_extension;
  Path m_path;
};

std::ostream& operator<<(std::ostream& os, const Filename& filename)
{
  os << filename.filename();
  return os;
}

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