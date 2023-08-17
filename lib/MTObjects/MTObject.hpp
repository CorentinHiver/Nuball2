#ifndef MTOBJECT_HPP
#define MTOBJECT_HPP
/*
  * This object is meant to be a base class for any other multithreaded object
  * It's only point is to manage this nb_threads static member variable
  * That is, all classes inheriting from MTObject will share this variable
*/
#include <thread>
#include <mutex>

using lock_mutex = std::lock_guard<std::mutex>;

#include "../libRoot.hpp"

class MTObject
{
public:
  MTObject() {}
  MTObject(size_t & _nb_threads ) {Initialize(_nb_threads);}

  static void Initialize(size_t const & _nb_threads, bool force = false)
  {
    // Check the number of threads. Usually, over 75% of cores is the optimal.
    // Set force parameter to true if you want to use all the cores
    size_t maxThreads = size_cast(std::thread::hardware_concurrency()*((force) ? 1 : 0.75));

    if(nb_threads > maxThreads)
    {
      nb_threads = maxThreads;
      std::cout << "Number of threads too large (hardware) -> reset to " << nb_threads << std::endl;
    }
    else nb_threads = _nb_threads;
    
    master_thread_id = std::this_thread::get_id();

    if (nb_threads>1)
    {
      print("MTObject initialized with", nb_threads, "threads");
      TThread::Initialize();
      ROOT::EnableThreadSafety();
      ON = true;
    }
  }

  static void adjustThreadsNumber(uint const & limiting_number, std::string const & print_if_limit_reached = "") 
  {
    if (limiting_number<nb_threads) 
    {
      nb_threads = limiting_number;
      print(print_if_limit_reached, "thread number reduced to ", nb_threads);
    }
  }

  template <class Func, class... ARGS>
  static void parallelise_function(Func && func, ARGS &&... args)
  {
    if (ON)
    {
      m_threads.reserve(nb_threads); // Memory pre-allocation (used for performances reasons)
      for (uint i = 0; i<nb_threads; i++) m_threads.emplace_back( [i, &func, &args...] ()
      {// Inside this lambda function, we already are inside the threads, so the parallelised section starts NOW :
        // threads_ID[std::this_thread::get_id()] = i; // Old indexing system
        m_thread_index = i; // Index the thread
        func(std::forward<ARGS>(args)...); // Run the function inside the thread
      });
      for(size_t i = 0; i < m_threads.size(); i++) m_threads.at(i).join(); //Closes threads, waiting fot everyone to finish
      m_threads.clear(); // Flushes memory
      print("Multi-threading is over !");
    }

    else
    {
      print("Running without multithreading ...");
      m_thread_index = 0;
      func(std::forward<ARGS>(args)...);
      return;
    }
  }

  static bool isMasterThread() {return (master_thread_id == std::this_thread::get_id());}

  static size_t nb_threads;
  static std::mutex mutex; // A global mutex for everyone to use
  static bool ON; // State boolean
  operator bool() {return ON;} // Can be used only if the class has been instanciated

  static int const & getThreadIndex() {return m_thread_index;}
  static int const & index() {return m_thread_index;}
  // static int const & getThreadIndex() {return threads_ID[std::this_thread::get_id()];} // Old indexing system

  static auto const & getThreadsNb() {return nb_threads;}
  static auto const & getThreadsNumber() {return nb_threads;}
  static void setThreadsNb(int const & n) {nb_threads = size_cast(n);}
  static void setThreadsNb(size_t const & n) {nb_threads = n;}

private:
  // static std::map<std::thread::id, int> threads_ID; // Old indexing system
  static std::thread::id master_thread_id; 
  static thread_local int m_thread_index; // thread_local variable, meaning it will hold different values for each thread it is in
  static std::vector<std::thread> m_threads;
};

// Declaration of static variables :
size_t MTObject::nb_threads = 1;
bool MTObject::ON = false;
std::mutex MTObject::mutex;

// std::map<std::thread::id, int> MTObject::threads_ID;
std::thread::id MTObject::master_thread_id;
thread_local int MTObject::m_thread_index = 0;
std::vector<std::thread> MTObject::m_threads;


template<class... ARGS>
void printMT(ARGS &&... args) 
{
  MTObject::mutex.lock();
  print(std::forward<ARGS>(args)...);
  MTObject::mutex.unlock();
}

#endif //MTOBJECT_HPP
