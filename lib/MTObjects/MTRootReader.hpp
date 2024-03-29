#ifndef MTROOTREADER_HPP
#define MTROOTREADER_HPP

#include "../MTObjects/MTObject.hpp"
#include "../MTObjects/MTList.hpp"

#include "../Classes/Nuball2Tree.hpp"
#include "../Classes/FilesManager.hpp"

class MTRootReader
{
public:
  MTRootReader() {}
  MTRootReader(Path foldername, int const & nb_files = -1) {this -> addFolder(foldername, nb_files);}
  bool addFolder(Path path, int const & nb_files = -1) 
  {
    auto const ret = m_files.addFolder(path, nb_files);
    return ret;
  }
  template<class Func, class... ARGS>
  void read(Func&& func, ARGS &&... args);

  bool nextFilename(std::string & filename) {return m_MTfiles.getNext(filename);}

private:
  template<class Func, class... ARGS>
  static void Read(MTRootReader & MTreader, Func function, ARGS &&... args);

  FilesManager m_files;
  MTList m_MTfiles;
};

template<class Func, class... ARGS>
void MTRootReader::read(Func && func, ARGS &&... args)
{
  if (!m_files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  m_MTfiles = m_files.getListFiles();
  MTObject::parallelise_function(Read<Func, ARGS...>, *this, std::forward<Func>(func), std::forward<ARGS>(args)...);
}

template<class Func, class... ARGS>
void MTRootReader::Read(MTRootReader & MTreader, Func function, ARGS &&... args)
{ // Here we are inside each thread :
  std::string filename;
  while(MTreader.nextFilename(filename))
  {
    Event event;
    Nuball2Tree tree(filename);
    if (!tree.ok()) continue;
    event.reading(tree.get());
    function(tree, event, std::forward<ARGS>(args)...); // If issues here, check that the parallelised function has the following form : type func(Hit & hit, FasterReader & reader, ARGS... some_args)
  }
}

#endif //MTROOTREADER_HPP