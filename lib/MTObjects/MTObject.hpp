#ifndef MTOBJECT_HPP
#define MTOBJECT_HPP

#include <csignal>
#include <future>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include "TROOT.h"
#include "TThread.h"

#define MULTITHREADING
#define MTSIGEXIT 42

using lock_mutex = const std::lock_guard<std::mutex>;

/**
 * @brief Class handling an easy multi threading.
 * @attention Include this class before any other in order to activate multithreading additions
 * @details
 * 
 * POSIX-based multithreading.
 * If you want to use the power of multithreading, this class can be convenient.
 * Please include this class before any other from this library in order to activate multithreading additions.
 * 
 * First thing to do is to setup the number of threads then to Initialise them :
 * 
 *        MTObject::setThreadsNb(nb_threads);
 *        MTObject::Initialise();
 * 
 * Or more concisely : 
 * 
 *        MTObject::Initialise(nb_threads);
 * 
 * Then you can multithread any function or static method : 
 * 
 *        MTObject::parallelise_function(function, param1, param2, ....);
 * 
 * If you want to multithread a non-static method (regular methods in an object), you need to pass
 * through a static function first. 
 * 
 * 
 * Example 1 : simplest example : parallelise a lambda.
 * 
 *        main()
 *        {
 *          MTObject::Initialise(2); // Using two concurrent threads
 *          int a = 0;
 *          int b = 0;
 *          // Here, the [&] allows the lambda to have access at everything that has been declared above. 
 *          // Therefore, in this case the lambda don't need any argument and the '()' is empty
 *          MTObject::parallelise_function([&](){
 *            if (random_uniform(0, 1)>0.5) ++a; // the random_uniform() is thread safe if '#include<MTObject>' is included first
 *            else                          ++b;
 *          });
 *          print(a, b);
 * 
 * Example 2 : parallelise a lambda and fill a histogram using MultiHist
 * 
 *        main()
 *        {
 *          MTObject::Initialise(2); // Using two concurrent threads
 *          MultiHist<TH1F> test("test", "test", 1000,0,1000); // MultiHist holds a vector of TH1F to be filled using its own Fill method
 *          
 *          MTObject::parallelise_function([&]()
 *          { // Here starts the parallelized portion of code
 *            print("thread id :", MTObject::getThreadIndex()); // You can access the current thread index (0 or 1 in this case)
 *            // This method automatically fills the copy of the histogram that corresponds to its thread index :
 *            for(int i = 0; i<10000000; i++) test.Fill(random_gaussian(500, 100)); 
 *          });
 *          // Now, you may want to perform some operations on the MultiHist. 
 *          // But before, you need to merge the copies into one single spectra using MultiHist::Merge
 *          test.Merge();
 *          // Now, you can access the fused histogram using '->' operator.
 *          // It calls the TH1F itself, so you have access at all the 
 *          print(test->GetMean());
 *          auto outfile(TFile::Open("test.root", "RECREATE"));
 *          outfile->cd();
 *          // Note : you can use the MultiHist::Write method without already merged it
 *          test.Write();
 *          outfile->Write();
 *          outfile->Close();
 *          return 0;
 *        }
 * 
 * Example 3 : parallelize a non-static method of a class :
 * 
 *        class MyClass
 *        {
 *        public:
 *          MyClass() {}
 *          void function_to_multithread(argument_1, argument_2, ...){....}
 *
 *          static void helper_function(MyClass & myClass, argument_1, argument_2, ...) {return myClass.function_to_multithread(argument_1, argument_2, ...);}
 *        };
 *
 *        int main()
 *        {
 *         ...
 *          MTObject::parallelise_function(myClass.helper_function, argument_1, argument_2, ....);
 *         ...
 *        }
 * 
 * @todo 
 * Trying to make this work :
 * template<class... ARGS>
 * static ret_type helper_function(MyClass & myClass, ARGS... args) {return myClass.function_to_multithread(std::forward<ARGS>(args)...);}
 */


class MTObject
{
public:
  MTObject() {}
  MTObject(size_t & _nb_threads ) {Initialise(_nb_threads);}

  static void Initialise(size_t const & _nb_threads, bool force = false)
  {
    setThreadsNb(_nb_threads, force);
    Initialise();
  }

  /** @brief Sets the number of threads.
   * 
   * @details Check the number of threads. Usually, over 75% of cores is the optimal.
   * Set force parameter to true if you want to use all the cores
   */
  static void setThreadsNb(int const & n, bool force = false) noexcept {setThreadsNb(static_cast<size_t>(n), force);}

  /** @brief Sets the number of threads.
   * 
   * @details Check the number of threads. Usually, over 75% of cores is the optimal.
   * Set force parameter to true if you want to use all the cores
   */
  static void setThreadsNb(size_t const & n, bool force = false) noexcept
  {
    auto const maxThreads = static_cast<size_t>(std::thread::hardware_concurrency()*((force) ? 1 : 0.75));

    if(n > maxThreads)
    {
      nb_threads = maxThreads;
      std::cout << "Number of threads too large (hardware) -> reset to " << nb_threads << std::endl;
    }
    else nb_threads = n;

    // nbThreadsChanged(nb_threads);// Signal
  }

  static void adjustThreadsNumber(size_t const & limiting_number, std::string const & print_if_limit_reached = "") 
  {
    if (limiting_number<nb_threads) 
    {
      setThreadsNb(limiting_number);
      std::cout << print_if_limit_reached << " thread number reduced to " << nb_threads << std::endl;
    }
    if (nb_threads == 1) MTObject::ON = false;
  }

  static bool kill;

  static void signalHandler(int signal)
  {
    if (signal == SIGINT)
    {
      if (!activated) exit(MTSIGEXIT); // If there is not multithreading going on, no need to wait for anything ...
      else if (MTObject::kill)
      {
        std::cout << "\nCtrl+C pressed twice, killing violently the program... (but should be fine, maybe, hopefully...)" << std::endl;
      }
      else
      {
        {
          lock_mutex lock(mutex);
          std::cout << "\nCtrl+C pressed, quitting the multithreaded environnement safely" << std::endl;
          std::cout << "Waiting for the current threads to finish...." << std::endl;
          std::cout << "If the threads do not stop are still created, you'll have to close the terminal (nothing will happens by default)" << std::endl;
          MTObject::kill = true;
        }
        for (auto & thread : m_threads) thread.join();
        m_threads.clear();
        std::cout << "All threads terminated properly" << std::endl;
      }
      m_exit_function();// User-defined function that can be set using setExitFunction
      exit(MTSIGEXIT);
    }
  }

  static void Initialise()
  {
    // Safety to Initialise threads only once :
    if (m_Initialised) return;
    m_Initialised = true;

    // signal(SIGINT, signalHandler);

    // Initialising :
    master_thread_id = std::this_thread::get_id();
    if (nb_threads>1)
    {
      // Initialise the root thread management
    #ifdef DEBUG
      std::cout << "Initialise ROOT thread management..." << std::endl;
    #endif //DEBUG
      // TThread::Initialise();
      ROOT::EnableThreadSafety();
      MTObject::ON = true;
      std::cout << "MTObject Initialised with " << nb_threads << " threads " << std::endl;
    }
    else MTObject::ON = false;
  }

  /// @brief 
  /// @todo check if I can replace Func by std::function
  template <class Func, class... ARGS>
  static void parallelise_function(Func && func, ARGS &&... args)
  {
    if (nb_threads == 1) MTObject::ON = false;
    if (MTObject::ON)
    {
      activated = true;
      m_threads.reserve(nb_threads); // Memory pre-allocation (used for performances reasons)
      for (size_t i = 0; i<nb_threads; i++) m_threads.emplace_back( [i, &func, &args...] ()
      {// Inside this lambda function, we already are inside the threads, so the parallelized section starts NOW :
        m_thread_index = i; // Index the thread
        func(std::forward<ARGS>(args)...); // Run the function inside thread
      });
      for(auto & thread : m_threads) thread.join(); //Closes threads, waiting fot everyone to finish
      m_threads.clear(); // Flushes memory
      std::cout << "Multi-threading is over !" << std::endl;
      activated = false;
    }

    else
    {
      std::cout << "Running without multithreading ..." << std::endl;
      m_thread_index = 0;
      func(std::forward<ARGS>(args)...);
      return;
    }
  }

  static auto const & getThreadsNb() {return nb_threads;}
  static auto const & getThreadsNumber() {return nb_threads;}

  static bool isMasterThread() {return (master_thread_id == std::this_thread::get_id());}

  static size_t nb_threads;
  static std::mutex mutex; // A global mutex for everyone to use
  static bool ON; // State boolean : is multithreading activated ?
  static bool activated; // State boolean : is multithreading activated ?

  static auto getThreadStdIndex() {return std::this_thread::get_id();}
  static auto getThreadStdIndexInt() {return std::hash<std::thread::id>{}(std::this_thread::get_id());}
  static auto getThreadIndex() {return m_thread_index;}
  static auto const & index() {return m_thread_index;}

  ///@brief A function that is used when ctrl+C is pressed in order to clean some stuff
  ///@details Tip : function can be a lambda (of the form [&](){...do something...}) in it, 
  /// so you can capture all the variables at the location you use this method
  static void setExitFunction(std::function<void(void)> const & function) {m_exit_function = function;}

private:
  static std::thread::id master_thread_id; 
  static thread_local size_t m_thread_index; // thread_local variable, meaning it will hold different values for each thread it is in
  static std::vector<std::thread> m_threads;
  static bool m_Initialised;
  static std::function<void(void)> m_exit_function;

};

bool MTObject::m_Initialised = false;
bool MTObject::kill = false;
bool MTObject::activated = false;
std::function<void(void)> MTObject::m_exit_function = [](){};

// Declaration of static variables :
size_t MTObject::nb_threads = 1;
bool MTObject::ON = false;
std::mutex MTObject::mutex;
std::mutex MTmutex;

std::thread::id MTObject::master_thread_id;
thread_local size_t MTObject::m_thread_index = 0;
std::vector<std::thread> MTObject::m_threads;


#endif //MTOBJECT_HPP

