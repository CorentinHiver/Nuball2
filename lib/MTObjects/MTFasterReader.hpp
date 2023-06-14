#ifndef MT_FASTER_READER_HPP
#define MT_FASTER_READER_HPP

#include "../libCo.hpp"

#include "../Classes/Hit.h"
#include "../Classes/FasterReader.hpp"
#include "../Classes/FilesManager.hpp"

#include "MTObject.hpp"
#include "MTList.hpp"

class MTFasterReader
{
public:
  MTFasterReader(){}
  MTFasterReader(Folder folder, int const & nb_files = -1){addFolder(folder, nb_files);}

  bool addFolder(Folder folder, int const & nb_files = -1) 
  {
    auto const ret = m_files.addFolder(folder, nb_files);
    m_MTfiles = m_files.getListFiles();
    return ret;
  }
  bool nextFilename(std::string & filename) {return m_MTfiles.getNext(filename);}

  template<class Func, class... ARGS>
  static void Read(MTFasterReader & MTreader, Func function, ARGS &&... args);

  template<class Func, class... ARGS>
  void execute(Func func, ARGS &&... args);

  void printMTFiles() {for (auto const & file : m_MTfiles) print(file);}

private:
  FilesManager m_files;
  MTList<std::string> m_MTfiles;
};

template<class Func, class... ARGS>
void MTFasterReader::execute(Func func, ARGS &&... args)
{
  MTObject::parallelise_function(Read<Func, ARGS...>, *this, std::forward<Func>(func), std::forward<ARGS>(args)...);
}

template<class Func, class... ARGS>
void MTFasterReader::Read(MTFasterReader & MTreader, Func function, ARGS &&... args)
{
  std::string filename;
  while(MTreader.nextFilename(filename))
  {
    Hit hit;
    FasterReader reader(&hit, filename);
    function(hit, reader, std::forward<ARGS>(args)...);
  }
}

#endif //MT_FASTER_READER_HPP