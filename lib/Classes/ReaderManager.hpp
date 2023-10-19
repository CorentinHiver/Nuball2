#ifndef DATAMULTIREADERMANAGER_H_CO
#define DATAMULTIREADERMANAGER_H_CO

#include "DataMultiReader.h"
// ********** C includes ************ //
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

typedef std::vector <Hit> Buffer;
typedef std::vector<std::string> Listfiles;

class ReaderManager
{
public:
  ReaderManager();

  //Files managing
  Bool_t addFiles   (std::string const & _filename        );
  Bool_t addFolder  (std::string folder, int nb_files = -1);
  void   flushFiles ();
  void   printFiles ();
  Int_t  nbFiles    (){return m_listFiles.size();};

  //Multi-reader managing
  Bool_t readNextFile();
  Bool_t readNextHit() { return (m_Reader.Read()) ;};
  Bool_t readNextHit(Hit & _hit) { return (m_Reader.Read(_hit)) ;};
  Hit const & getHit() const { return m_Reader.getHit() ;};

private:
  Bool_t setReader(std::string const & _filename);
  Bool_t setReader(Int_t const & _filecursor);
  Int_t m_filesCursor = 0;
  std::string m_currentFile = "";
  Listfiles m_listFiles;
  DataMultiReader m_Reader;
};

ReaderManager::ReaderManager()
{

}

Bool_t ReaderManager::readNextFile()
{
  if(m_filesCursor+1>m_listFiles.size()) return false;
  m_currentFile = m_listFiles.at(m_filesCursor);
  Bool_t is_file = m_Reader.Initialize(m_currentFile);
  m_filesCursor++;
  if (!is_file) m_filesCursor = 0;// Place the file cursor back to 0, ready to read again from the first file
  return is_file;
}

Bool_t ReaderManager::setReader(std::string const & _filename)
{
  m_currentFile = _filename;
  return  m_Reader.Initialize(_filename);
}

Bool_t ReaderManager::setReader(Int_t const & _filecursor)
{
  m_currentFile = m_listFiles[_filecursor];
  return m_Reader.Initialize(m_currentFile);
}

Bool_t ReaderManager::addFiles(std::string const & _filename)
{
  UInt_t numberFiles = 0;
  if (extension(_filename) == "dat")
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

Bool_t ReaderManager::addFolder(std::string _foldername, int _nb_files)
{
  if (_foldername.back() != '/') _foldername.push_back('/');
  struct dirent *file = nullptr;
  DIR *dp = nullptr;
  dp = opendir(_foldername.c_str());
  if (dp == nullptr) {std::cout << "Folder " << _foldername << " not found..." << std::endl; return false;}
  std::string name = "";
  Listfiles listfile;
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
    Listfiles cut_listfile (listfile.begin(), listfile.begin()+_nb_files);// Take the nb_files first files
    if (m_listFiles.size() == 0) m_listFiles = cut_listfile;// Set cut_listfile to be the global list of files
    else std::copy(m_listFiles.begin(), m_listFiles.end(), std::back_inserter(cut_listfile));// Add cut_listfile to the global list of files
    return true;
  }
  else
  {
    return false;
  }
}

inline void ReaderManager::printFiles()
{
  for (auto const & file : m_listFiles) std::cout << file << std::endl;
}
#endif
