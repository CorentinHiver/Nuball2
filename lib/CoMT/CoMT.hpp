#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include <csignal>
#include <cstring>
#include <algorithm> // For std::clamp

// Portability check for write/unistd
#ifdef _WIN32
    #include <io.h>
    #define WRITE_FUNC _write
    #define STDOUT_FD 1
#else
    #include <unistd.h>
    #define WRITE_FUNC ::write
    #define STDOUT_FD STDOUT_FILENO
#endif

// #ifdef ROOT_TObject 
  #include "TROOT.h"
  #include "TThread.h"
// #endif

// Defined simply for user convenience if they want to define their own locks
using lock_mutex = std::lock_guard<std::mutex>; 

namespace Colib
{
  /**
   * @brief Multithreading framework for c++ and ROOT classes.
   * @attention MT::isKilled() allows the user to handle cases when the user used ctrl+C to kill the programm. If not implemented, a second ctrl+C will violently kill the programm
   * @details
   * 
   * Automatically paralellise a given function - or lambda - and its parameters.
   * 
   * Call like this :
   * 
   *        MT::parallelise_function_n<4>([](){
   *         std::cout << MT::getThreadIndex() << std::endl;
   *        });
   * 
   * will result in (with a possibly shuffled output): 
   * 
   *        0
   *        1
   *        2
   *        3
   * 
   * You can also set a unique thread number for the whole programm - modifiable at runtime !:
   * 
   *        MT::Initialize(4)
   * 
   *        MT::parallelise_function([](){
   *         std::cout << MT::getThreadIndex() << std::endl;
   *        });
   * 
   * How to use the killing logic ?
   * 
   * You can do the following : all threads print i from 0 to 100000. The first one who finished wins and kills concurency :
   * 
   *        MT::parallelise_function_n<4>([]()
   *        {
   *          for (int i = 0; i<10000; ++i) 
   *          {
   *            print(i); 
   *            if(MT::isKilled()) break;
   *          }
   *          MT::kill();
   *        });
   * 
   */
  namespace MT
  {

    inline static std::atomic<bool> m_killed   {false};
    inline static std::atomic<bool> m_activated{false};

    inline static thread_local size_t local_thread_index = 0;
    inline static std::atomic<size_t> s_nbThreads{1};

    inline static std::mutex mutex;
    inline std::mutex cout_mutex;

    enum class ThreadRole : uint8_t {Master, Worker};
    inline static thread_local ThreadRole local_thread_role = ThreadRole::Master;
    bool isMasterThread() noexcept {return local_thread_role == ThreadRole::Master;}
    bool isWorkerThread() noexcept {return local_thread_role == ThreadRole::Worker;}

    #ifdef ROOT_TObject
    inline static std::atomic<bool> root_safety_enabled {false};
    #endif
    
    void signalHandler(int signal) noexcept
    {
      if (signal == SIGINT)
      {
        if (m_killed.load()) ::_exit(42); // Second Ctrl+C: Force exit immediately

        m_killed.store(true);
        const char* msg = "\n[MT] Ctrl+C caught. Waiting for threads to finish... Press again to force quit.\n";
        // Async-signal-safe write
        WRITE_FUNC(STDOUT_FD, msg, (unsigned int)strlen(msg));
      }
    }

    void installSignalHandler()
    {
      static bool installed = false;
      if (installed) return;

      struct sigaction sa{};
      sa.sa_handler = signalHandler;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0; 

      if (sigaction(SIGINT, &sa, nullptr) == -1) 
      {
        std::cerr << "[MT] Failed to set signal handler.\n";
        ::_exit(44);
      }

      installed = true;
    }

    void enableRootSafety()
    {
    // #ifdef ROOT_TObject
      if (!root_safety_enabled.load())
      {
        ROOT::EnableThreadSafety();
        root_safety_enabled.store(true);
      }
    // #endif
    }

    void Initialise(size_t n_threads)
    {
      lock_mutex lock(mutex);

      size_t hw = std::thread::hardware_concurrency();
      if (hw == 0) hw = 1;

      s_nbThreads.store(std::clamp(n_threads, 1ul, hw));

      if (1 < s_nbThreads.load())
      {
        enableRootSafety();
        installSignalHandler();
      }

      std::cout << "[MT] Initialised with " << s_nbThreads.load() << " threads.\n";
    }

    size_t getThreadsNb  () noexcept { return s_nbThreads; }
    size_t getThreadIndex() noexcept { return local_thread_index; }

    void kill() noexcept { m_killed.store(true); }

    bool isKilled() noexcept { return m_killed.load(); }
    bool isActivated() noexcept { return m_activated.load(); }


    void reserve_thread_lines(size_t nb_threads) 
    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      // 1. Create the space by printing newlines
      for (size_t i = 0; i < nb_threads; ++i) std::cout << '\n';
      std::cout << "\033[" << nb_threads << "A" << std::flush;
    }

    template <typename... Ts>
    void printsln(Ts&&... args) noexcept 
    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      size_t idx = getThreadIndex();

      if (idx > 0) std::cout << "\033[" << idx << "B";

      std::cout << "\033[2K\r";

      if (!isMasterThread()) std::cout << "[" << idx << "] ";
      (std::cout << ... << std::forward<Ts>(args));

      if (idx > 0) std::cout << "\r\033[" << idx << "A";
      else std::cout << "\r";

      std::cout << std::flush;
    }

    void reset_after_threads(size_t nb_threads) 
    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "\033[" << nb_threads << "B" << std::endl;
    }

    /// @brief Run a function in parallel.
    template <class Func, class... Args>
    void parallelise_function(Func&& func, Args&&... args)
    {
      reserve_thread_lines(s_nbThreads);
      // 1. Serial execution optimization
      if (s_nbThreads <= 1)
      {
        local_thread_index = 0;
        local_thread_role  = ThreadRole::Worker;
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return;
      }

      m_activated = true;
      std::vector<std::thread> workers;
      workers.reserve(s_nbThreads);

      // 2. Launch Threads
      for (size_t id = 0; id < s_nbThreads; ++id)
      {
        workers.emplace_back([&, id]() 
        {
          local_thread_index = id;          
          if(m_killed.load()) return; 
          printsln("Worker started");          
          local_thread_role  = ThreadRole::Worker;
          std::invoke(func, args...);
          printsln("Worker go to sleep");          
        });
      }

      // 3. Wait for Threads
      for (auto& t : workers) if (t.joinable()) t.join();

      local_thread_role  = ThreadRole::Master;
      m_activated = false;

      // 4. Handle Exit Request
      if (isKilled())
      {
        const char* msg = "\n[MT] Exiting after thread completion.\n";
        WRITE_FUNC(STDOUT_FD, msg, (unsigned int)strlen(msg));
        ::_exit(42);
      }
      // reset_after_threads(s_nbThreads);
    }

    /**
     * @brief Helper to initialize and run in one go.
     * @warning Changes the global thread count setting.
     */
    template <size_t nbThreads, class Func, class... Args>
    void parallelise_function_n(Func&& func, Args&&... args)
    {
      Initialise(nbThreads);
      parallelise_function(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    /**
     * @brief Helper to initialize and run in one go.
     * @warning Changes the global thread count setting.
     */
    template <class Func, class... Args>
    void parallelise_function(size_t nbThreads, Func&& func, Args&&... args)
    {
      Initialise(nbThreads);
      parallelise_function(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    ////////////////////
    // Helper methods //
    ////////////////////

    template<class T>
    std::vector<std::vector<T>> distribute(std::vector<T> initVec)
    {
      std::vector<std::vector<T>> ret;
      
      auto nbThreads = getThreadsNb();
      if (nbThreads <= 1) nbThreads = 1;

      ret.resize(nbThreads);

      std::cout << (nbThreads) << std::endl;
      
      const size_t total = initVec.size();
      const size_t chunk_size = total / nbThreads;
      const size_t remainder  = total % nbThreads;
  
      size_t offset = 0;
  
      for (size_t thread_i = 0; thread_i < nbThreads; ++thread_i)
      {
        size_t this_chunk = chunk_size + (thread_i < remainder ? 1 : 0);
  
        auto& files_for_thread = ret[thread_i];
        files_for_thread.clear();
        files_for_thread.reserve(this_chunk);
  
        for (size_t i = 0; i < this_chunk; ++i) files_for_thread.push_back(initVec[offset + i]);
  
        offset += this_chunk;
      }
      return ret;
    }

    // template <class... Ts> 
    // void printsln(Ts &&... ts)  noexcept
    // {
    //   int thread_idx = getThreadIndex();  // Assuming this returns 0 to Nthreads-1.
    //   std::lock_guard<std::mutex> lock(cout_mutex);
    //   std::cout << "[" << thread_idx << "] ";
    //   std::cout << "\033[" << (thread_idx + 1) << ";1H\r";  // Position to row (thread_idx + 1), column 1, then carriage return.
    //   println(std::forward<Ts>(ts)...); 
    //   std::cout << std::flush;
    // }
  }
}