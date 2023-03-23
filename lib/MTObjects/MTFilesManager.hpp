#ifndef MTFILESMANAGER_H
#define MTFILESMANAGER_H
#include "MTObject.hpp"
#include "FilesManager.hpp"

//WORK IN PROGRESS

class MTFilesManager : public MTObject, MTFilesManager
{
  MTFilesManager(){}
  Bool_t FilesManager::nextFileName(std::mutex & mutex, std::string & filename, size_t const & step)

}
Bool_t FilesManager::nextFileName(std::mutex & mutex, std::string & filename, size_t const & step)
{
  if(m_filesCursor+step>m_listFiles.size()) return false;
  filename = m_listFiles.at(m_filesCursor);
  m_filesCursor += step;
  return true;
}

#endif //MTFILESMANAGER_H
