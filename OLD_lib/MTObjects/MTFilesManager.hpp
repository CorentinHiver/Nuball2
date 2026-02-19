#ifndef MTFILESMANAGER_H
#define MTFILESMANAGER_H

#include "MTList.hpp"
#include "../Classes/FilesManager.hpp"

//WORK IN PROGRESS

/**
 * @brief Work in progress
 * 
 */
class MTFilesManager : public FilesManager
{
  MTList<std::string> m_listFiles;
}

#endif //MTFILESMANAGER_H
