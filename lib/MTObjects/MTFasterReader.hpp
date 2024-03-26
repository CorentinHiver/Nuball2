#ifndef MT_FASTER_READER_HPP
#define MT_FASTER_READER_HPP

#include "MTObject.hpp"
#include "MTList.hpp"
#include "../Classes/FasterReader.hpp"
#include "../Classes/Alignator.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/CoProgressBar.hpp"


/**
 * @class MTFasterReader
 * @brief Class used to read in parallel .fast files of the same run
 * @details
 * **Look at the FasterReader class for the various options**
 * 
 * # How to use this class
 * 
 * First, you'll need to activate the multithreading : 
 * 
 *        MTObject::Initialise(nb_threads)
 * 
 * Then instanciate this class :
 * 
 *        MTFasterReader reader(folder_name, first_n_files);
 * 
 * To read all the files then leave the second argument empty
 * 
 * Then, use the readRaw(function, arguments...) method to run any user defined function.
 * 
 *        reader.readRaw(function, arguments...);
 * 
 * Carefull : this function MUST have the following arguments is this EXACT order : 
 * 
 *        return_type function(Hit & hit, FasterReader & reader, arguments...)
 * 
 * This function can also be a static method of an object, but CANNOT be a member
 * 
 * The trick to use the object anyway is to pass it as an argument : 
 * 
 *        static return_type myClass::function(Hit & hit, FasterReader & reader, myClass & class, arguments...);
 * 
 * 
 * 
 * Here is a minimal code snippet :
 * 
 *      void function(Hit & hit, FasterReader & reader, Arg & some_argument) 
 *      {
 *        // This function is now setup to read one specific file
 *        while(reader.Read())
 *        {
 *          // Here, hit contains the data of the current faster data hit beeing read in the binary .fast file
 *          Do something with the hit like histo->Fill(hit.nrj) or tree -> Fill() if you connected a tree to the hit
 *        }
 *      }
 * 
 *      int main()
 *      {
 *         MTObject::Initialise(n); // n being an appropriate number of threads.
 *         MTFasterReader reader(folder);
 *         Arg some_argument;
 *         reader.readRaw(function, some_argument);
 *         // Do something with the argument like print(some_argument) or some_argument.Write()
 *      }
 * 
 * Here are two function examples. The third parameter has been instanciated before the MTFasterReader::readRaw() method call
 *      
 *        void counter(Hit & hit, FasterReader & reader, MTCounter & counterMT)
 *         {
 *            int counter = 0;
 *            while(reader.Read())
 *            {
 *               counter++;
 *            }
 *            counterMT += counter;
 *         }
 *      
 * 
 *        void scalers(Hit & hit, FasterReader & reader, MTTHist<TH1> & scaler)
 *        {
 *           while(reader.Read())
 *           {
 *              scaler.Fill(hit.label);
 *           }
 *        }
 * 
 * (see MTTHist and MTCounter documentation)
 * 
 * It is recommended to pass the variables by reference (adding the '&' character) for the parameters so that they are shared between threads.
 * Therefore this should only be done for thread safe objects, like any object in the folder MTObjects/ or stl stuff, or read-only variables.
 * If you want to pass read-only objects (i.e. lookup tables), consider adding the "const" key word (..., type const & lookup_table, ...). 
 * Another work-around is simply to declare your parameter at a global scope, although it is not recommended to have too many global variables.
 */ 
class MTFasterReader
{
public:
  /**
   * @brief Default constructor
   */
  MTFasterReader(){}
  /**
 * @brief Regular constructor
 * 
 * @param path : The path to the path containing the .fast files to read
 * @param nb_files : Default -1, takes all the files
 */
  MTFasterReader(Path path, int const & nb_files = -1) {addFolder(path, nb_files);}

  MTFasterReader(FilesManager const & files) {m_files = files;}
  
  template<class Func, class... ARGS>
  void readRaw(Func&& func, ARGS &&... args);

  template<class Func, class... ARGS>
  void readAligned(Func&& func, ARGS &&... args);

  template<class Func, class... ARGS>
  void readAligned(std::vector<Time> timeshifts, Func&& func, ARGS &&... args) 
  {
    m_timeshifts = timeshifts;
    this -> readAligned<Func, ARGS...>(func, args...);
  }

  // Files handling :
  bool addFolder(Path path, int const & nb_files = -1) 
  {
    return m_files.addFolder(path, nb_files);
  }

  bool addFile(File file) 
  {
    return m_files.addFiles(file.string());
  }

  void printMTFiles() {for (auto const & file : m_MTfiles) print(file);}
  auto & getFilesList() {return m_MTfiles;}
  auto const & files() const {return m_files;}
  auto & files() {return m_files;}

  // Other parameters :
  void setTimeshifts(std::vector<Time> const & timeshifts) {m_timeshifts = timeshifts;}
  auto const & timeshift(Label const & label) const {return m_timeshifts[label];}
  static void showProgressBar(bool const & choice = true) {s_progressBar = choice;}

private:

  // Private methods that handles the multi-threading
  template<class Func, class... ARGS>
  static void Read(MTFasterReader & MTreader, Func function, ARGS &&... args);

  template<class Func, class... ARGS>
  static void Realign(MTFasterReader & MTreader, Func function, ARGS &&... args);

  bool nextFilename(std::string & filename) {return m_MTfiles.getNext(filename);}

  FilesManager m_files;
  MTList m_MTfiles;

  std::vector<Time> m_timeshifts;
  static bool s_progressBar;
};
bool MTFasterReader::s_progressBar = false;
//////////////////////////////
//   Read raw faster data   //
//////////////////////////////
/** 
 * @brief Reads many faster files in parallel
 * @param func: Function used on each file in parallel. CAREFULL : must be a function or a static method
 * @details
 * The declared function MUST have its two first parameters as follow : type function(Hit & hit, FasterReader & reader, ...);
 * You can add any other parameter in the ..., but then you have to call them in the readRaw method call
 * e.g. : 
 * 
 *        void my_function(Hit & hit, FasterReader & reader, MTCounter & counter){do something...}
 * 
 * in main
 *  
 *        MTFasterReader readerMT(/path/to/data/folder/, wanted_number_of_files);
 *        MTCounter counterMT;
 *        readerMT.readRaw(my_function, counterMT);
 * 
 * That way, my_function will be executed in parallel on each file in /path/to/data/folder/
*/
template<class Func, class... ARGS>
inline void MTFasterReader::readRaw(Func && func, ARGS &&... args)
{
  if (!m_files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  m_MTfiles = m_files.getListFiles();
  MTObject::parallelise_function(Read<Func, ARGS...>, *this, std::forward<Func>(func), std::forward<ARGS>(args)...);
}

template<class Func, class... ARGS>
inline void MTFasterReader::Read(MTFasterReader & MTReader, Func function, ARGS &&... args)
{ // Here we are inside each thread :
  std::string filename;
  CoProgressBar progress(&MTReader.getFilesList().getIndex(), MTReader.getFilesList().size());
  while(MTReader.nextFilename(filename))
  {
    if (MTObject::kill) {print("Killing thread", MTObject::getThreadIndex()); break;}
  fasterReaderMutex.lock();
    Hit hit;
    FasterReader reader(&hit, filename);
    if (s_progressBar) progress.show();
  fasterReaderMutex.unlock();
    function(hit, reader, std::forward<ARGS>(args)...); // If issues here, check that the parallelised function has the following form : type func(Hit & hit, FasterReader & reader, ARGS... some_args)
  }
}


///////////////////////////////////////
//   Read time aligned faster data   //
///////////////////////////////////////
/**
 * @brief Reads many faster files in parallel, providing a time-aligned 
 * 
 * @details
 * Use this function in the same way as readRaw, with a function like this : 
 *  func(Hit & hit, Alignator & alignedTree, args...)
 * 
 * @param func : Must be of the form func(Hit & hit, Alignator & alignedTree, args...). 
 * Alignator is a simple wrapper around a tree. Use alignedTree::GetEntry
 */
template<class Func, class... ARGS>
inline void MTFasterReader::readAligned(Func&& func, ARGS &&... args)
{
  if (!m_files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  if (m_timeshifts.size() == 0) print("CAREFULL, NO TIMESHIFT DATA PROVIDED !!");
  m_MTfiles = m_files.getListFiles();
  MTObject::parallelise_function(Realign<Func, ARGS...>, *this, std::forward<Func>(func), std::forward<ARGS>(args)...);
}

template <class Func, class... ARGS>
inline void MTFasterReader::Realign(MTFasterReader &MTreader, Func function, ARGS &&...args)
{// Here we are inside each thread :
  std::string filename;
  while(MTreader.nextFilename(filename))
  {
    Hit hit;
    FasterReader reader(&hit, filename);
    unique_tree tempTree(new TTree("temp", "temp"));
    hit.writting(tempTree.get());
    while (reader.Read())
    {
      hit.stamp+=MTreader.timeshift(hit.label);
      tempTree->Fill();
    }
    Alignator alignedTree(tempTree.get());
    hit.reset();
    hit.reading(tempTree.get());
    function(hit, alignedTree, std::forward<ARGS>(args)...);
  }
}


#endif //MT_FASTER_READER_HPP