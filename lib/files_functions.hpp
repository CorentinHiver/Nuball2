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
  return size * (size_file_unit[unit_i]/size_file_unit[unit_o]);
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

std::string & push_back_if_none(std::string & string, char const & character)
{
  if (string.back() != character) string.push_back(character);
  return string;
}

bool folder_exists(std::string folderName)
{
  push_back_if_none(folderName, '/');
  DIR *dp = nullptr;
  dp = opendir(folderName.c_str());
  bool ret = (dp != nullptr);
  std::string str = ((dp!=nullptr) ? "oui" : "non");
  if (dp) closedir(dp);
  return ret;
}

bool folder_exists(std::string folderName, bool const & verbose)
{
  push_back_if_none(folderName, '/');
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
  push_back_if_none(folderName, '/');
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
  push_back_if_none(folderName, '/');
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
  push_back_if_none(folderName, '/');
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


// ----------------------------------- //
// ----------------------------------- //
//        PATH FOLDERS AND FILES
// ----------------------------------- //
// ----------------------------------- //


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
   * @details
   * Basically, it is simply ensures that the name ends with a '/'
   * 
   * Also, it is the base class of Path class
  */
  Folder(std::string const & folder) : m_folder (folder)
  {
    make();
  }

  Folder(const char * folder) : m_folder (std::string(folder))
  {
    make();
  }

  Folder & operator=(std::string const & folder)
  {
    m_folder = folder;
    make();
    return *this;
  }

  Folder & operator=(const char * folder)
  {
    return (*this = std::string(folder));
  }

  Folder & operator+=(std::string const & folder)
  {
    m_folder += std::string(folder);
    return *this;
  }

  Folder & operator+=(const char * folder)
  {
    m_folder += std::string(folder);
    return *this;
  }

  operator std::string() const & {return m_folder;}
  operator bool()        const & {return m_ok    ;}
  
  std::string const & string() const {return m_folder;}
  std::string const & get()    const {return m_folder;}
  std::string name() const {auto ret = m_folder; ret.pop_back(); return ret;}

  void make()
  {
    if (m_folder.size() == 0) m_ok = false;
    else
    {
      if (m_folder.back() != '/') m_folder.push_back('/');
      m_ok = true;
    }
  }
  
private:
  bool m_ok = false;
  std::string m_folder;
};

std::ostream& operator<<(std::ostream& cout, Folder const & folder)
{
  cout << folder.string();
  return cout;
}

Folder operator+(std::string const & string, Folder const & folder) { return (string + folder.get());}
Folder operator+(const char * string, Folder const & folder) { return (std::string(string) + folder.get());}

/**
 * @brief Object used to hold a list of folders
 */
class Folders
{
public:

  Folders(){}

  Folders(std::vector<std::string> const & folders)
  {
    for (auto const & folder : folders) {m_folders.push_back(Folder(folder));}
  }

  Folders(std::vector<Folder> const & folders) {m_folders = folders;}

  Folders(Folders const & folders) { m_folders = folders.m_folders; }

  Folders& operator=(std::vector<std::string> const & folders)
  {
    for (auto const & folder : folders) {m_folders.push_back(Folder(folder));} return *this;
  }
  Folders& operator=(std::vector<Folder> const & folders) {m_folders = folders;return *this;}
  Folders& operator=(Folders const & folders) { m_folders = folders.m_folders; return *this;}

  operator std::string() 
  {
    Folder ret;
    for (auto const & folder : m_folders) ret+=(folder);
    return ret;
  }

  operator std::vector<Folder>() const &  {return m_folders;}

  auto begin() const {return m_folders.begin();}
  auto end  () const {return m_folders.end();}

  auto const & list() const {return m_folders;}
  auto const & get()  const {return m_folders;}

  void push_back(Folder const & folder) {m_folders.push_back(folder);}

private:

  std::vector<Folder> m_folders;
};

std::ostream& operator<<(std::ostream& cout, Folders const & folders)
{
  cout << folders.get();
  return cout;
}

/**
 * @brief Object used to hold the complete path of a giver folder
 * 
 * @details
 * You can use either a full path from the root ("/.../.../") or from the home directory ("~/.../.../")
 * 
 * So far, relative paths are not supported (yet hopefully)
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
  Path(std::string const & path, bool const & create = false) : m_path(path) {load(create);}

    /**
   * @brief Turns a C string to a path, creating it if create = true and it doesn't already exists
  */
  Path(const char* c_str, bool const & create = false) : m_path(std::string(c_str)) {load(create);}

  void load(bool const & create = false)
  {
    if (m_path[0]=='/')
    {// Absolute path
    }
    else if (m_path[0]=='~')
    {// Home path
      m_path = home()+m_path;
    }
    else
    {// Relative path
      m_path = pwd()+m_path;
      // m_exists = false;
    }

    // To ensure it finishes with a '/' :
    push_back_if_none(m_path, '/');

  #ifdef MTOBJECT_HPP
    if (MTObject::ON) MTObject::shared_mutex.lock();
  #endif //MTOBJECT_HPP
  
    // To ensure it exists :
    m_exists = folder_exists(m_path);

    // Create the folder if it doesn't exist yet :
    if (!m_exists && create) this -> make();
    if (!folder_exists(m_path))
    {
      m_exists = false;
      throw std::runtime_error(m_path+" doesn't exist !!");
    }
  #ifdef MTOBJECT_HPP
    if (MTObject::ON) MTObject::shared_mutex.unlock();  
  #endif //MTOBJECT_HPP

    makeFolderList(m_path);
  }

  Folders makeFolderList(std::string const & path)
  {
    auto folders = getList(path,'/');
    return Folders(folders);
  }

  int  nbFiles() {return nb_files_in_folder(m_path);}
  bool exists() {return folder_exists(m_path);}
  operator bool() const {return folder_exists(m_path);}
  bool make() { create_folder_if_none(m_path); return this -> exists();}

  Path & addFolder(Folder const & folder)
  {
    if (file_exists(m_path+=folder.get()))
    {
      m_recursive_folders.push_back(folder.name());
    }
    else 
    {
      throw std::runtime_error(m_path + " don't exist !");
    }
    return *this;
  }

  std::string const & path() const {return m_path;}
  
  std::string const & get() const {if (!m_exists) print("Carefull, you manipulate not existing folder"); return m_path;}
  std::string const & string() const {return(get());}
  operator std::string() const & {return (get());}
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
    push_back_if_none(m_path, '/');
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
    push_back_if_none(str, '/');
    m_path+=str;
    return *this;
  }

  bool operator==(std::string const & cmprStr)
  {
    return (cmprStr == m_path);
  }

  static Path home() {return Path(std::string(std::getenv("HOME")));}
  static Path pwd() {return Path(std::string(std::getenv("PWD")));}
  static Path current_directory() {return Path(std::string(std::getenv("PWD")));}

private:
  bool m_exists = false;
  std::string m_path;
  Folders m_recursive_folders;
};

std::ostream& operator<<(std::ostream& cout, Path const & p)
{
  cout << p.path();
  return cout;
}

/**
 * @brief Contains the short name and the extension of a given file, without any knowledge of its path
 */
class Filename
{
public:
  Filename(){}
  Filename(std::string const & _filename) : m_fullName(_filename) 
  {
    this -> fill(_filename);
  }

  Filename(Filename const & _filename) : m_fullName(_filename.m_fullName), m_shortName(_filename.m_shortName), m_extension(_filename.m_extension) {}

  Filename & operator=(Filename const & _filename)
  {
    m_fullName = _filename.m_fullName;
    m_shortName = _filename.m_shortName;
    m_extension = _filename.m_extension;
    return *this;
  }

  Filename & operator=(std::string const & _filename)
  {
    this -> fill(_filename);
    return *this;
  }

  operator std::string() const & {return m_fullName;}
  std::string const & get() const {return m_fullName;}

  std::string const & fullName() const {return m_fullName;}
  std::string const & shortName() const {return m_shortName;}
  std::string const & extension() const {return m_extension;}

private:

  void fill(std::string const & filename)
  {
    if (getPath(filename).size() > 1) {print(filename, "is not a filename...");}
    else
    {
      m_shortName = rmPathAndExt(filename);
      m_extension = getExtension(filename);
    }
  }
  std::string m_fullName;
  std::string m_shortName;
  std::string m_extension;
};

std::ostream& operator<<(std::ostream& os, Filename const & filename)
{
  os << filename.fullName();
  return os;
}

std::string operator+(Path const & path, Filename const & filename) {return path.get() + filename.get();} 

/**
 * @brief Composed of a Path and a Filename
 * @details
 * A File object is composed of a Path and a Filename object, which are composed of :
 *  - A list of folder that forms the Path to the file
 *  - A short name and an extension for the Filename
 */
class File
{
public:
  File(File const & file) : 
          m_ok(file.m_ok)    ,
          m_file(file.m_file), 
          m_path(file.m_path), 
          m_filename(file.m_filename) 
  { }

  File(std::string const & file) 
  {
    fill(file);
    check();
  }
  File(const char * file) 
  {
    fill(file);
    check();
  }
  
  File(Path const & path, Filename const & filename) : m_file(path+filename), m_path(path), m_filename(filename) {}

  auto c_str() {return m_file.c_str();}

  File & operator=(std::string const & file) 
  {
    m_path = getPath(file);
    return *this;
  }
  File & operator=(const char * file) 
  {
    m_path = Path(file);
    return *this;
  }
  File & operator=(File const & file) 
  {
    m_ok   = file.m_ok  ;
    m_file = file.m_file;
    m_path = file.m_path;
    m_filename = file.m_filename;
    return *this;
  }

  std::string const & string() const {return m_file;}
  std::string const & get   () const {return m_file;}
  std::string const & file  () const {return m_file;}
  operator std::string() const &     {return m_file;}
  Path        const & path  () const {return m_path;}
  Filename    const & name  () const {return m_filename;}
  Filename    const & filename  () const {return m_filename;}

  std::string const & shortName() const {return m_filename.shortName();}
  std::string const & extension() const {return m_filename.extension();}

  operator bool() const & {return m_ok;}
  bool const & ok()       {return m_ok;}

  auto size(std::string const & unit = "o") const {return size_file(m_file, unit);}

private:

  void check()
  {
    if (m_path.exists())
    {
      if (file_exists(m_file)) m_ok = true;
      else print("The file", m_file, "is unreachable in folder", m_path);
    }
    else
    {
      print("The path of the file", m_file, "is unreachable");
    }
  }

  void fill(std::string const & file)
  {
    m_file = file;
    m_path = getPath(file);
    m_filename = removePath(file);
  }

  bool m_ok = false;
  std::string m_file; // Full path + short name + extension
  Path m_path;        // Path
  Filename m_filename;// Name + extension
};

std::ostream& operator<<(std::ostream& cout, File const & file)
{
  cout << file.get();
  return cout;
}

#endif //FILES_HPP