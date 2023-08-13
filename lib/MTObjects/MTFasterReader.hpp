#ifndef MT_FASTER_READER_HPP
#define MT_FASTER_READER_HPP

#include "../Classes/Hit.hpp"
#include "../Classes/FasterReader.hpp"
#include "../Classes/FilesManager.hpp"

#include "MTObject.hpp"
#include "MTList.hpp"

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
 *    MTObject::Initialize(nb_threads)
 * 
 * Then instanciate this class :
 * 
 * 2. MTFasterReader reader(folder_name, first_n_files);
 * 
 * To read all the files then leave the second argument empty
 * 
 * Then, use the execute(function, arguments...) method to run any user defined function.
 * 
 * 3. reader.execute(function, arguments...);
 * 
 * Carefull : this function MUST have the following arguments is this EXACT order : 
 * 
 * return_type function(Hit & hit, FasterReader & reader, arguments...)
 * 
 * This function can also be a static method of an object, but CANNOT be a member
 * 
 * The trick to use the object anyway is to pass it as an argument : 
 * 
 * static return_type myClass::function(Hit & hit, FasterReader & reader, myClass & class, arguments...);
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
 *         MTObject::Initialize(n); // n being an appropriate number of threads.
 *         MTFasterReader reader(folder);
 *         Arg some_argument;
 *         reader.execute(function, some_argument);
 *         // Do something with the argument like print(some_argument) or some_argument.Write()
 *      }
 * 
 * Here are two function examples. The third parameter has been instanciated before the MTFasterReader::execute() method call
 * 
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
 * It is recommended to use the "&" symbol (reference) before the other as well parameters so that they are shared between threads,
 * therefore only with thread safe objects, but it is not mandatory. If you want to pass read-only objects (i.e. lookup tables), 
 * consider adding the "const" key word like that : (..., type const & lookup_table, ...). Another work-around is simply to declare your
 * parameter at a global scope, although it is not recommended to have too many of them.
 * 
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

  bool addFolder(Path path, int const & nb_files = -1) 
  {
    auto const ret = m_files.addFolder(path, nb_files);
    m_MTfiles = m_files.getListFiles();
    return ret;
  }
  
  /** 
   * @brief Reads many files in parallel
   * @param func: Function used on each file in parallel. Cannot be a member function, except if declared static.
   * @details
   * The declared function MUST have its two first parameters as follow : type function(Hit & hit, FasterReader & reader, ...);
   * You can add any other parameter in the ..., but then you have to call them in the execute method call
   * e.g. : 
   * 
   *        void my_function(Hit & hit, FasterReader & reader, MTCounter & counter){do something...}
   * 
   * in main
   *  
   *        MTFasterReader reader(/path/to/data/folder);
   *        MTCounter counter;
   *        reader.execute(my_function, counter);
   * 
   * That way, my_function will be executed in parallel on each file this class is currently reading.
   * 
   * 
  */
  template<class Func, class... ARGS>
  void execute(Func func, ARGS &&... args);

  void printMTFiles() {for (auto const & file : m_MTfiles) print(file);}

  /// @brief Internal method, do not use
  bool nextFilename(std::string & filename) {return m_MTfiles.getNext(filename);}

private:
  template<class Func, class... ARGS>
  static void Read(MTFasterReader & MTreader, Func function, ARGS &&... args);

  FilesManager m_files;
  MTList m_MTfiles;
  std::mutex m_mutex;
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
    function(hit, reader, std::forward<ARGS>(args)...); // If issues here, check that the parallelised function has the following form : type func(Hit & hit, FasterReader & reader, ....)
  }
}

#endif //MT_FASTER_READER_HPP