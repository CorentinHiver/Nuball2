#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "../libCo.hpp"

using ListFiles = std::vector<std::string> ;
using ListFolders = std::vector<std::string> ;

class FilesManager
{
public:

  FilesManager(){};
  FilesManager(std::string const & folder, long nb_files = -1){addFolder(folder,nb_files);};
  FilesManager(Path & folder, long nb_files = -1){addFolder(folder.string(),nb_files);};


  bool nextFileName(std::string & filename, size_t const & step = 1);

  // Adds either a single file or reads a .list containing a list of files
  virtual bool addFiles(std::string const & _filename);
  virtual bool addFile (std::string const & _filename) {return addFiles(_filename);}
  // Adds a given number of files with a .root or .fast inside the given folder (by default all the files, or the nb_files first ones)
  virtual bool addFolder    (std::string folder, long nb_files = -1, std::vector<std::string> const & extensions = {"root", "fast"});
  virtual bool addFolder    (Path const & folder, long nb_files = -1, std::vector<std::string> const & extensions = {"root", "fast"})
  {
    return this -> addFolder(folder.string(), nb_files, extensions);
  }
  virtual void flushFiles   ();
  void Print        () { for (auto const & file : m_listFiles) print(file);}
  void printFolders () { for (auto const & folder : m_listFolder) print(folder);}

  //Getters :
  Path const & path() const {return m_path;}
  auto const & get() const {return m_listFiles;}
  auto & get() {return m_listFiles;}
  ListFiles   const & getListFiles     () const { return m_listFiles ;}
  ListFolders const & getListFolders   () const { return m_listFolder;}

  ListFiles const & getFilesInFolder (std::string folder)
  {
    if (folder.back()!='/') folder.push_back('/');
    return m_listFilesInFolder[folder];
  }

  size_t const & getCursor () const { return m_filesCursor;}
  size_t size () const { return m_listFiles.size();}
  
  float diskSize() const 
  {
    float ret = 0;
    for (std::size_t i = 0; i<size(); i++)
    {
      ret+=size_file(m_listFiles[i]);
    }
    return ret;
  }

  bool isEmpty () const { return (this->size() == 0);}
  operator bool() const {return !isEmpty();}


  //Files reader :
  virtual std::string getFile (int const & n = -1)
  {
    if (n<0) return m_listFiles[m_filesCursor];
    else return m_listFiles[n];
  }

  //Setters :
  void setListFiles(ListFiles const & list) {this -> flushFiles(); for (size_t i = 0; i<list.size(); i++) this -> addFiles(list[i]);}
  void setCursor(int const & _filesCursor) {m_filesCursor = static_cast<size_t> (_filesCursor);}
  void setCursor(size_t const & _filesCursor) {m_filesCursor = _filesCursor;}
  void setVerbose(bool const & v) {verbose = v;}

  std::string operator[] (int const & n) {return getFile(n);}
  void operator=(ListFiles const & list) {setListFiles(list);}
  FilesManager const & operator=(FilesManager const & other) 
  {
    m_path = other.m_path;
    m_filesCursor = other.m_filesCursor;
    m_listFiles = other.m_listFiles;
    m_listFolder = other.m_listFolder;
    m_listFilesInFolder = other.m_listFilesInFolder;
    isReadable = other.isReadable;
    verbose = other.verbose;
    return *this;
  }

  auto begin() {return m_listFiles.begin();}
  auto end()   {return m_listFiles.end  ();}

  auto begin() const {return m_listFiles.begin();}
  auto end()   const {return m_listFiles.end  ();}

protected:
  Path m_path;
  size_t      m_filesCursor = 0;
  ListFiles   m_listFiles;
  ListFolders m_listFolder;
  std::map<std::string, ListFiles> m_listFilesInFolder;
  bool isReadable = false;
  bool verbose = false;
};

bool FilesManager::addFiles(std::string const & _filename)
{
  m_path = getPath(_filename);
  uint numberFiles = 0;
  if (extension(_filename) == "list")
  {// using the "data" file as an input containing the path to the actual data .root or .fast files
    std::ifstream inputsFile (_filename);
    if (!inputsFile.is_open() || !inputsFile.good())
    {
      std::cout << "Impossible to open or read dat file '" << _filename << "'" << std::endl;
      return false;
    }
    std::string oneline;
    while(inputsFile.good())
    {
      getline(inputsFile, oneline);
      if( oneline.size() > 1  &&  (extension(oneline) == "root" || extension(oneline) == "fast"))
      {
        m_listFiles.push_back(oneline);
        numberFiles++;
      }
    }
    inputsFile.close();
    return true;
  }
  else if (extension(_filename) == "root" || extension(_filename) == "fast")
  {// there is only one .root or .fast file
    m_listFiles.push_back(_filename);
    numberFiles = 1;
    return true;
  }
  else {std::cout << "File " << _filename << "not taken into account. Extension" << extension(_filename)
  << "unkown..." << std::endl << "Abort..." << std::endl;return false;}
}

bool FilesManager::addFolder(std::string _foldername, long _nb_files, std::vector<std::string> const & extensions)
{
  push_back_if_none(_foldername, '/');
  auto listfile = list_files_in_folder(_foldername, extensions); // Default .root OR .fast files only
  if (listfile.size() > 0)
  {
    std::sort(listfile.begin(), listfile.end());// Sorts the entries
    if (_nb_files> static_cast<long>(listfile.size()) || _nb_files == -1) _nb_files = listfile.size();//Sets the correct number of files to keep
    ListFiles cut_listfile (listfile.begin(), listfile.begin()+_nb_files);// Take the nb_files first files of the folder
    for (auto const & file : cut_listfile) m_listFilesInFolder[_foldername].emplace_back(file);
    if (m_listFiles.size() == 0) m_listFiles = cut_listfile;// Set cut_listfile to be the global list of files
    else std::copy(cut_listfile.begin(), cut_listfile.end(), std::back_inserter(m_listFiles));// Add cut_listfile to the global list of files
    if (verbose) print( cut_listfile.size(), "files added,", m_listFiles.size(), "files to process");
    return true;
  }
  else
  {
    throw std::runtime_error("Folder '"+_foldername+"' empty !!");
    return false;
  }
}

void FilesManager::flushFiles()
{
  m_listFiles.resize(0);
  m_listFiles.clear();
  m_filesCursor = 0;
}

bool FilesManager::nextFileName(std::string & filename, size_t const & step)
{
  if(m_filesCursor+step>m_listFiles.size()) return false;
  filename = m_listFiles.at(m_filesCursor);
  m_filesCursor += step;
  return true;
}

std::ostream& operator<<(std::ostream& cout, FilesManager const & files)
{
  for (auto const & f : files) cout << f << " ";
  return cout;
}
#endif
