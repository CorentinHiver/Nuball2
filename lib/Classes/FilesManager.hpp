#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <libCo.hpp>

using ListFiles = std::vector<std::string> ;
using ListFolders = std::vector<std::string> ;

class FilesManager
{
public:

  FilesManager(){};
  FilesManager(std::string const & folder, int nb_files = -1){addFolder(folder,nb_files);};

  bool nextFileName(std::string & filename, size_t const & step = 1);

  // Adds either a single file or reads a .list containing a list of files
  bool addFiles     (std::string const & _filename        );
  // Adds a given number of files with a .root or .fast inside the given folder (by default all the files, or the nb_files first ones)
  bool addFolder    (std::string folder, int nb_files = -1);
  void   flushFiles   ();
  void   Print        ();
  void   printFolders ();

  //Getters :
  ListFiles   const & getListFiles     () const { return m_listFiles ;}
  ListFolders const & getListFolders   () const { return m_listFolder;}

  ListFiles const & getFilesInFolder (std::string folder)
  {
    if (folder.back()!='/') folder.push_back('/');
    return m_listFilesInFolder[folder];
  }

  size_t const & getCursor () const { return m_filesCursor;}
  size_t size () const { return m_listFiles.size();}
  bool isEmpty () { return (this->size() == 0);}

  //Files reader :
  virtual std::string getFile    (int const & n = -1)
  {
    if (n<0) return m_listFiles[m_filesCursor];
    else return m_listFiles[n];
  }

  //Setters :
  void setListFiles(ListFiles const & list) {this -> flushFiles(); for (size_t i = 0; i<list.size(); i++) this -> addFiles(list[i]);}
  void setCursor(int const & _filesCursor) {m_filesCursor = static_cast<size_t> (_filesCursor);}
  void setCursor(size_t const & _filesCursor) {m_filesCursor = _filesCursor;}
  void setVerbose(bool const & v) {verbose = v;}

  // Operator overloading :
  std::string operator[] (int const & n) {return getFile(n)        ;}
  void operator=(ListFiles const & list) {return setListFiles(list);}

protected:
  size_t      m_filesCursor = 0;
  ListFiles   m_listFiles;
  ListFolders m_listFolder;
  std::map<std::string, ListFiles> m_listFilesInFolder;
  bool isReadable = false;
  bool verbose = false;
};

bool FilesManager::addFiles(std::string const & _filename)
{
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

bool FilesManager::addFolder(std::string _foldername, int _nb_files)
{
  if (extension(_foldername) == "list")
  {
    //to code
  }
  if (_foldername.back() != '/')
  {
    m_listFolder.push_back(removePath(_foldername)+"/");
    _foldername.push_back('/');
  }
  else
  {
    _foldername.pop_back();
    m_listFolder.push_back(removePath(_foldername)+"/");
    _foldername.push_back('/');
  }
  struct dirent *file = nullptr;
  DIR *dp = nullptr;
  dp = opendir(_foldername.c_str());
  if (dp == nullptr) {std::cout << "Folder " << _foldername << " not found..." << std::endl; return false;}
  std::string name = "";
  ListFiles listfile;
  while ( (file = readdir(dp)))
  {
    name = file->d_name;
    if (extension(name) == "root" || extension(name) == "fast")
      listfile.push_back(_foldername+name);
  }
  closedir(dp);
  delete file;
  if (listfile.size() > 0)
  {
    std::sort(listfile.begin(), listfile.end());// Sorts the entries
    if (_nb_files>(int)listfile.size() || _nb_files == -1) _nb_files = listfile.size();//Sets the correct number of files to keep
    ListFiles cut_listfile (listfile.begin(), listfile.begin()+_nb_files);// Take the nb_files first files of the folder
    for (auto const & file : cut_listfile) m_listFilesInFolder[_foldername].emplace_back(file);
    if (m_listFiles.size() == 0) m_listFiles = cut_listfile;// Set cut_listfile to be the global list of files
    else std::copy(cut_listfile.begin(), cut_listfile.end(), back_inserter(m_listFiles));// Add cut_listfile to the global list of files
    if (verbose) print( cut_listfile.size(), "files added,", m_listFiles.size(), "files to process");
    return true;
  }
  else
  {
    print("NO data found !");
    return false;
  }
}

void FilesManager::Print()
{
  std::cout << "Loaded files : ";
  for (auto const & file : m_listFiles) std::cout << file << std::endl;
  std::cout << std::endl;
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
#endif
