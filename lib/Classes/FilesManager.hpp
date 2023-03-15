#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "utils.hpp"

using ListFiles = std::vector<std::string> ;
using ListFolders = std::vector<std::string> ;

class FilesManager
{
public:

  FilesManager(){};

  Bool_t nextFileName(std::string & filename, size_t const & step = 1);

  Bool_t addFiles     (std::string const & _filename        );// Adds either a single file or reads a .list containing a list of files
  Bool_t addFolder    (std::string folder, int nb_files = -1);
  void   flushFiles   ();
  void   Print   ();
  void   printFolders ();

  //Getters :
  Int_t       size           () { return m_listFiles.size() ;}
  Bool_t      isEmpty        () { return (this->size() == 0);}
  ListFiles   getListFiles   () { return m_listFiles        ;}
  ListFolders getListFolders () { return m_listFolder       ;}
  Int_t       getCursor      () { return m_filesCursor      ;}

  //Files reader :
  std::string getFile    (int const & n) {return m_listFiles[n];}

  //Setters :
  void setListFiles(ListFiles const & list) {this -> flushFiles(); for (size_t i = 0; i<list.size(); i++) this -> addFiles(list[i]);}
  void setCursor(Int_t _filesCursor) {m_filesCursor = _filesCursor;}

  // Operator overloading :
  std::string operator[] (int const & n) {return getFile(n)        ;}
  void operator=(ListFiles const & list) {return setListFiles(list);}

private:
  size_t      m_filesCursor = 0;
  std::string m_currentFile = "";
  ListFiles   m_listFiles;
  ListFolders m_listFolder;
  bool isReadable = false;
};

Bool_t FilesManager::addFiles(std::string const & _filename)
{
  UInt_t numberFiles = 0;
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

Bool_t FilesManager::addFolder(std::string _foldername, int _nb_files)
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
    if (m_listFiles.size() == 0) m_listFiles = cut_listfile;// Set cut_listfile to be the global list of files
    else std::copy(cut_listfile.begin(), cut_listfile.end(), back_inserter(m_listFiles));// Add cut_listfile to the global list of files
    std::cout << cut_listfile.size() << " files added, " << m_listFiles.size() << " files to process" << std::endl;
    return true;
  }
  else
  {
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

Bool_t FilesManager::nextFileName(std::string & filename, size_t const & step)
{
  if(m_filesCursor+step>m_listFiles.size()) return false;
  filename = m_listFiles.at(m_filesCursor);
  m_filesCursor += step;
  return true;
}
#endif
