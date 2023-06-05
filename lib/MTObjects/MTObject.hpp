#ifndef MTOBJECT_H
#define MTOBJECT_H
/*
  * This object is meant to be a base class for any other multithreaded object
  * It's only point is to manage this nb_threads static member variable
  * That is, all classes inheriting from MTObject will share this variable
*/
#include <thread>
#include <mutex>

#include <libRoot.hpp>

class MTObject
{
public:
  MTObject() {}
  MTObject(ushort & _nb_threads ) {Initialize(_nb_threads);}

  static void Initialize(ushort const & _nb_threads, bool force = false)
  {
    // Check the number of threads. Usually, over 75% of cores is the optimal.
    // Set force parameter to true if you want to use all the cores
    int maxThreads = static_cast<int>(std::thread::hardware_concurrency()*((force) ? 1 : 0.75));
    nb_threads = _nb_threads;
    if(nb_threads > maxThreads)
    {
      nb_threads = maxThreads;
      std::cout << "Number of threads too large (hardware) -> reset to " << nb_threads << std::endl;
    }
    
    master_thread = std::this_thread::get_id();

    if (nb_threads>1)
    {
      TThread::Initialize();
      ROOT::EnableThreadSafety();
    }

    ON = true;
  }

  template <class Func, class... ARGS>
  static void parallelise_function(Func func, ARGS &&... args)
  {
    if (!ON)
    {
      print("The multithreading has not been initialized.");
      print("Please use MTObject::Initialize(int number_threads) for initialisation.");
      return;
    }
    for (ushort i = 0; i<nb_threads; i++)
    {
      m_threads.emplace_back(
        [i, &func, &args...]()
        {
          shared_mutex.lock();
          threads_ID[std::this_thread::get_id()] = i;
          shared_mutex.unlock();
          sleep(1);
          func(std::forward<ARGS>(args)...);
        });
    }
    for(size_t i = 0; i < m_threads.size(); i++) m_threads.at(i).join(); //Closes threads
    m_threads.resize(0);
    m_threads.clear();
    std::cout << "Multi-threading is over !" << std::endl;
  }

  static bool isMasterThread()
  {
    return (master_thread == std::this_thread::get_id());
  }

  static ushort nb_threads;
  static std::mutex shared_mutex;
  static std::map<std::thread::id, int> threads_ID;
  static bool ON;
  operator bool() {return ON;}
  static std::thread::id master_thread;

  static int getThreadIndex()
  {
    return threads_ID[std::this_thread::get_id()];
  }

  static auto const & getThreadsNb() {return nb_threads;}
  static void setThreadsNb(int const & n) {nb_threads = static_cast<ushort>(n);}
protected:
  std::mutex m_mutex;
private:
  static std::vector<std::thread> m_threads;
};
// As it is a static variable we have to declare it outside of the class
// Also, I think it is better to initialise it at 1, in order to avoid unitialisation issue
ushort MTObject::nb_threads = 1;
bool MTObject::ON = false;
std::map<std::thread::id, int> MTObject::threads_ID;
std::mutex MTObject::shared_mutex;
std::thread::id MTObject::master_thread;
std::vector<std::thread> MTObject::m_threads;
#endif //MTOBJECT_H
