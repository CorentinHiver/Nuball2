#ifndef FASTER2ROOTV2_HPP
#define FASTER2ROOTV2_HPP

#include "FasterRootInterface.hpp"

class Faster2RootV2
{
public:
  FasterRunReader() noexcept = default;

  void addFiles(std::string files)
  {
    auto const & filenames = Colib::findFilesWildcard(files);
    for (auto const & filename : filenames) m_files.push_back(filename);
  }

  void addFolder(std::string folder)
  {
    auto const & filenames = Colib::findFilesWildcard(folder+"*");
    for (auto const & filename : filenames) m_files.push_back(filename);
  }

  // void setHit(Hit * hit) {reader.setHit(hit);}

  bool readNextFile()
  {
    if (reader.)
  }


private:
  std::vector<std::string> m_files;
  thread_local FasterRootInterface reader;
};

#endif //FASTER2ROOTV2_HPP