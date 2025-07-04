#ifndef MTROOTREADER_HPP
#define MTROOTREADER_HPP

#include "../MTObjects/MTObject.hpp"
#include "../MTObjects/MTList.hpp"

#include "../Classes/Nuball2Tree.hpp"
#include "../Classes/FilesManager.hpp"

#include "../Classes/Alignator.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/Timeshifts.hpp"

#ifdef COMULTITHREADING
  std::mutex rootReaderMutex;
#endif //COMULTITHREADING

void rootReaderLockMutex()
{
#ifdef COMULTITHREADING
  rootReaderMutex.lock();
#endif //COMULTITHREADING
}

void rootReaderUnlockMutex()
{
#ifdef COMULTITHREADING
  rootReaderMutex.unlock();
#endif //COMULTITHREADING
}

/**
 * @brief 
 * @details
 * MTRootReader reader(path, nb);
 * reader.read([&](Nuball2Tree & tree, Event & event){
 * ....
 * };);
 */
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
  template<class Func, class... ARGS>
  void read_raw(Calibration const & calibration, Timeshifts const & timeshifts, Func&& func, ARGS &&... args);

  bool nextFilename(std::string & filename) {return m_MTfiles.getNext(filename);}

private:
  FilesManager m_files;
  MTList m_MTfiles;
};

template<class Func, class... ARGS>
void MTRootReader::read(Func && func, ARGS &&... args)
{
  if (!m_files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  m_MTfiles = m_files.getListFiles();
  MTObject::parallelise_function([&](){
    std::string filename;
    while(nextFilename(filename))
    {
      if (MTObject::kill) break;
      Event event;
      Nuball2Tree tree(filename, event);
      if (!tree.ok()) continue;
      func(tree, event, std::forward<ARGS>(args)...); // If issues here, check that the parallelised function (or lambda) has the following form : type func(Nuball2Tree & tree, Event & event, ARGS... some_args)
    }
  });
}

template<class Func, class... ARGS>
void MTRootReader::read_raw(Calibration const & calibration, Timeshifts const & timeshifts, Func&& func, ARGS &&... args)
{ // Here we are inside each thread :
  if (!m_files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  if (!timeshifts) throw_error("NO TIMESHIFT DATA PROVIDED !!");
  m_MTfiles = m_files.getListFiles();
  MTObject::parallelise_function([&](){ // Here we are inside each thread :
    std::string filename;
    while(nextFilename(filename))
    {
      if (MTObject::kill) {print("Killing thread", MTObject::getThreadIndex()); break;}

    rootReaderLockMutex();
      Hit hit;
      Nuball2Tree tree(filename, hit);
      // tree.loadRAM();
      TString name = "temp"+std::to_string(MTObject::getThreadIndex());
      unique_tree tempTree(new TTree(name, "temp"));
      tempTree->SetDirectory(nullptr);
      hit.writing(tempTree.get(), "ltQE");
    rootReaderUnlockMutex();

      while (tree.readNext())
      {
        hit.stamp+=timeshifts[hit.label];
        calibration(hit);
        tempTree->Fill();
      }

    rootReaderLockMutex();
      Alignator alignedTree(tempTree.get());
      hit.clear();
      hit.reading(tempTree.get());
    rootReaderUnlockMutex();
    
      func(alignedTree, hit, std::forward<ARGS>(args)...);
    }
  });
}


#endif //MTROOTREADER_HPP