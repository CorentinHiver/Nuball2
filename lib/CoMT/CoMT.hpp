#pragma once
#define MT_AVAILABLE

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

#ifdef ROOT_TObject 
  #include "TROOT.h"
  #include "TThread.h"
#endif

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
   *        MT::parallelise_function_nb<4>([](){
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
   * You can also set a unique thread number for the whole programm :
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
   *        MT::parallelise_function_nb<4>([]()
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
  class MT
  {
    inline static std::atomic<bool> m_killed   {false};
    inline static std::atomic<bool> m_activated{false};

    inline static thread_local size_t m_thread_index = 0;
    inline static size_t s_nbThreads = 1;

    #ifdef ROOT_TObject
    inline static bool root_safety_enabled = false;
    #endif

    static void installSignalHandler()
    {
      static bool installed = false;
      if (installed) return;

      struct sigaction sa{};
      sa.sa_handler = signalHandler;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0; 

      if (sigaction(SIGINT, &sa, nullptr) == -1) std::cerr << "[MT] Failed to set signal handler.\n";

      installed = true;
    }

    static void enableRootSafety()
    {
    #ifdef ROOT_TObject
      if (!root_safety_enabled)
      {
        ROOT::EnableThreadSafety();
        root_safety_enabled = true;
      }
    #endif
    }

  public:

    static void Initialise(size_t n_threads)
    {
      size_t hw = std::thread::hardware_concurrency();
      if (hw == 0) hw = 1;

      s_nbThreads = std::clamp(n_threads, 1ul, hw);

      if (1 < s_nbThreads)
      {
        enableRootSafety();
        installSignalHandler();
      }

      std::cout << "[MT] Initialised with " << s_nbThreads << " threads.\n";
    }

    static size_t getThreadsNb  () noexcept { return s_nbThreads; }
    static size_t getThreadIndex() noexcept { return m_thread_index; }

    static void signalHandler(int signal) noexcept
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

    static void kill() noexcept { m_killed.store(true); }

    static bool isKilled() noexcept { return m_killed.load(); }
    static bool isActivated() noexcept { return m_activated.load(); }

    /**
     * @brief Run a function in parallel.
     * Arguments are passed by reference to threads (Zero-Copy).
     */
    template <class Func, class... Args>
    static void parallelise_function(Func&& func, Args&&... args)
    {
      // 1. Serial execution optimization
      if (s_nbThreads <= 1)
      {
        m_thread_index = 0;
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
          m_thread_index = id;
          // Check if killed before starting work (optional optimization)
          if(m_killed.load()) return; 
          
          std::invoke(func, args...); 
        });
      }

      // 3. Wait for Threads
      for (auto& t : workers) if (t.joinable()) t.join();

      m_activated = false;

      // 4. Handle Exit Request
      if (isKilled())
      {
        std::cout << "[MT] Exiting after thread completion.\n";
        std::exit(42);
      }
    }

    /**
     * @brief Helper to initialize and run in one go.
     * @warning Changes the global thread count setting.
     */
    template <size_t nbThreads, class Func, class... Args>
    static void parallelise_function_nb(Func&& func, Args&&... args)
    {
      Initialise(nbThreads);
      parallelise_function(std::forward<Func>(func), std::forward<Args>(args)...);
    }
    /**
     * @brief Helper to initialize and run in one go.
     * @warning Changes the global thread count setting.
     */
    template <class Func, class... Args>
    static void parallelise_function(size_t nbThreads, Func&& func, Args&&... args)
    {
      Initialise(nbThreads);
      parallelise_function(std::forward<Func>(func), std::forward<Args>(args)...);
    }
  };
}