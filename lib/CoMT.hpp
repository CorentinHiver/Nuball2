#pragma once

#define CoMT

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Terminal.hh"

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
    inline std::atomic<bool> killed   {false};
    inline std::atomic<bool> activated{false};
    
    void kill() noexcept { killed.store(true); }

    bool isKilled() noexcept { return killed.load(std::memory_order_relaxed); }
    bool isActivated() noexcept { return activated.load(); }

    inline std::vector<std::string> thread_statuses;

    inline static thread_local size_t local_thread_index = 0;
    inline std::atomic<size_t> s_nbThreads{1};

    size_t getNbThreads  () noexcept { return s_nbThreads.load(); }
    size_t getThreadIndex() noexcept { return local_thread_index; }
    std::string getThreadIndexStr() noexcept { return std::to_string(local_thread_index); }

    inline std::mutex mutex;
    inline std::mutex cout_mutex;

    enum class ThreadRole : uint8_t {Master, Worker};
    inline static thread_local ThreadRole local_thread_role = ThreadRole::Master;
    bool isMasterThread() noexcept {return local_thread_role == ThreadRole::Master;}
    bool isWorkerThread() noexcept {return local_thread_role == ThreadRole::Worker;}

    inline static std::atomic<bool> root_safety_enabled {false};

    template<class Func>
    void inject_print(Func print_func) noexcept
    {
      lock_mutex lock(cout_mutex);
      size_t n_threads = getNbThreads();
      using namespace Colib;

      if (isActivated() && n_threads > 1) 
      {
        // 1. Erase current thread block
        std::cout << Terminal::DISABLE_LINES_WRAPPING;
        for (size_t i = 0; i < n_threads; ++i) 
        {
          std::cout << Terminal::CLEAR_ROW;
          if (i + 1 < n_threads) Terminal::move_up(1);
        }
        if (n_threads > 1) Terminal::move_down(n_threads - 1);
        std::cout << Terminal::ENABLE_LINES_WRAPPING << Terminal::RETURN;

        // 2. Execute normal print (this natively moves the cursor down a line)
        print_func();

        // 3. Redraw the thread block underneath
        std::cout << Terminal::DISABLE_LINES_WRAPPING;
        for (size_t i = 0; i < n_threads; ++i) {
          std::cout << Terminal::CLEAR_ROW << Terminal::RETURN << thread_statuses[i];
          if (i + 1 < n_threads) std::cout << Terminal::NEWLINE;
        }
        // Move cursor back to the top of the newly shifted thread block
        if (n_threads > 1) Terminal::move_up(n_threads - 1);
        std::cout << Terminal::ENABLE_LINES_WRAPPING << Terminal::RETURN << std::flush;
      } 
      else 
      {
        print_func();
      }
    }
    
    void signalHandler(int signal) noexcept
    {
      if (activated.load())
      {
        if (signal == SIGINT)
        {
          if (isKilled()) ::_exit(42); // Second Ctrl+C: Force exit immediately
  
          killed.store(true);
          lock_mutex lock(mutex);
          Colib::MT::inject_print([&]() { 
            std::cout << "Ctrl+C caught. Waiting for threads to finish... Press again to force quit." << Colib::Terminal::NEWLINE;
          });
        }
      }
      else 
      {
        ::_exit(42); 
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

    // SIOF-safe storage using Meyers Singletons
    inline std::mutex& getSignalMutex() 
    {
      static std::mutex m;
      return m;
    }

    inline std::map<size_t, std::function<void(size_t)>>& getInitCallbacks() 
    {
      static std::map<size_t, std::function<void(size_t)>> cb;
      return cb;
    }

    inline std::atomic<size_t>& getCallbackCounter() 
    {
      static std::atomic<size_t> counter{0};
      return counter;
    }

    // A flag to know if we've initialized at least once
    inline static std::atomic<bool> has_been_initialized{false};

    /**
     * @brief Subscribe to the Initialization signal.
     * @return A unique ID to be used for unsubscription.
     */
    inline size_t subscribeToInitialise(std::function<void(size_t)> cb)
    {
      size_t id = ++getCallbackCounter();
      bool trigger_immediately = false;
      size_t current_threads = 1;

      {
        lock_mutex lock(getSignalMutex());
        getInitCallbacks()[id] = cb;
        
        // 2. Late Arrival Check
        if (has_been_initialized.load()) 
        {
          trigger_immediately = true;
          current_threads = getNbThreads();
        }
      }

      if (trigger_immediately && cb) 
      {
        try { cb(current_threads); } 
        catch (...) { /* Handle or log late-arrival exception */ }
      }

      return id;
    }

    /**
     * @brief Unsubscribe to prevent dangling pointers.
     */
    inline void unsubscribeToInitialise(size_t id)
    {
      lock_mutex lock(getSignalMutex());
      getInitCallbacks().erase(id);
    }

    inline std::mutex& getFinaliseMutex() 
    {
      static std::mutex m;
      return m;
    }

    inline std::map<size_t, std::function<void(size_t)>>& getFinaliseCallbacks() 
    {
      static std::map<size_t, std::function<void(size_t)>> cb;
      return cb;
    }

    inline std::atomic<size_t>& getFinaliseCounter() 
    {
      static std::atomic<size_t> counter{0};
      return counter;
    }

    /**
     * @brief Subscribe to the Finalize signal.
     * @details Callbacks are invoked after threads have joined and cleanup is done
     *          (same storage + lock + exception-safe execution pattern as Initialise).
     *          Subscriptions are persistent and fire on every parallel job completion.
     * @return A unique ID to be used for unsubscription.
     */
    inline size_t subscribeToFinalise(std::function<void(size_t)> cb)
    {
      size_t id = ++getFinaliseCounter();

      {
        lock_mutex lock(getFinaliseMutex());
        getFinaliseCallbacks()[id] = cb;
        // No "late arrival" immediate trigger (unlike init) because finalise
        // is a repeating event, not a one-time state change.
      }

      return id;
    }

    /**
     * @brief Unsubscribe to prevent dangling pointers.
     */
    inline void unsubscribeToFinalise(size_t id)
    {
      lock_mutex lock(getFinaliseMutex());
      getFinaliseCallbacks().erase(id);
    }

    // Internal helper - exact same callback execution logic as Initialise
    void runFinaliseCallbacks(size_t current_threads)
    {
      std::map<size_t, std::function<void(size_t)>> callbacks_to_run;
      
      {
        lock_mutex cb_lock(getFinaliseMutex());
        callbacks_to_run = getFinaliseCallbacks(); 
      } 

      for (auto& [id, cb] : callbacks_to_run) 
      {
        if (cb) 
        {
          try {
              cb(current_threads);
          } catch (const std::exception& e) {
              std::cerr << "[MT] Warning: Exception in finalise callback ID " 
                        << id << ": " << e.what() << '\n';
          } catch (...) {
              std::cerr << "[MT] Warning: Unknown exception in finalise callback ID " 
                        << id << '\n';
          }
        }
      }
    }

    void enableRootSafety()
    {
      if (!root_safety_enabled.load())
      {
        ROOT::EnableThreadSafety();
        root_safety_enabled.store(true);
      }
    }

    void Initialise(size_t n_threads)
    {
      {
        lock_mutex lock(mutex);
  
        size_t hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 1;
        
        // If we are already set up for this many threads, do nothing.
        size_t target = std::clamp(n_threads, 1ul, hw);
        if (has_been_initialized.load() && target == s_nbThreads.load()) return; 
  
        s_nbThreads.store(target);
  
        if (1 < getNbThreads())
        {
          enableRootSafety();
          installSignalHandler();
        }
      }
      
      has_been_initialized.store(true);
      
      // Safe Copy of Callbacks
      std::map<size_t, std::function<void(size_t)>> callbacks_to_run;
      {
          lock_mutex cb_lock(getSignalMutex());
          callbacks_to_run = getInitCallbacks(); 
      } // <--- Signal mutex released

      // Safe Execution with Exception Handling
      size_t current_threads = getNbThreads();
      for (auto& [id, cb] : callbacks_to_run) 
      {
        if (cb) {
          try {
              cb(current_threads);
          } catch (const std::exception& e) {
            std::cerr << "[MT] Warning: Exception in init callback ID " 
                      << id << ": " << e.what() << '\n';
          } catch (...) {
            std::cerr << "[MT] Warning: Unknown exception in init callback ID " 
                      << id << '\n';
          }
        }
      }
      
      std::cout << "[MT] Initialised with " << getNbThreads() << " threads.\n";
    }

    void reserve_thread_lines(size_t nb_threads) 
    {
      {
        lock_mutex lock(cout_mutex);
        thread_statuses.assign(nb_threads, "");
      }
      if (!isActivated() || nb_threads == 1) return;
      Colib::Terminal::new_lines(nb_threads);
      Colib::Terminal::move_up(nb_threads);
      std::cout << std::flush;
    }

    template <typename... Ts>
    void printsln(Ts&&... args) noexcept 
    {
      using namespace Colib;
      
      lock_mutex lock(cout_mutex);
      size_t n_threads = getNbThreads();
      size_t thread_i = getThreadIndex();

      if (!isActivated() || n_threads <= 1) 
      {
        std::cout << Terminal::CLEAR_ROW << Terminal::RETURN;
        if (!isMasterThread()) std::cout << "[" << thread_i << "] ";
        ((std::cout << std::forward<Ts>(args) << ' '), ...);
        std::cout << std::flush;
        return;
      }

      std::ostringstream oss;
      if (!isMasterThread()) oss << "[" << thread_i << "] ";
      ((oss << std::forward<Ts>(args) << ' '), ...);
      thread_statuses[thread_i] = oss.str();

      std::cout << Terminal::DISABLE_LINES_WRAPPING;
      if (thread_i > 0) Terminal::move_down(thread_i);
      std::cout << Terminal::CLEAR_ROW << Terminal::RETURN << thread_statuses[thread_i];
      if (thread_i > 0) Terminal::move_up(thread_i);
      else std::cout << Terminal::RETURN;

      std::cout << Terminal::ENABLE_LINES_WRAPPING;
      
      std::cout << std::flush;
    }

    void reset_after_threads(size_t nb_threads) 
    {
      lock_mutex lock(cout_mutex);
      std::cout << "\033[" << nb_threads << "B" << std::endl;
    }

    //////////////
    // Function //
    //////////////

    /// @brief Run a function in parallel.
    template <class Func, class... Args>
    void parallelise_function(Func&& func, Args&&... args)
    {
      // 1. Serial execution optimization
      if (getNbThreads() < 2)
      {
        local_thread_index = 0;
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return;
      }

      // 2. Launch Threads

      activated.store(true, std::memory_order_relaxed);
      std::vector<std::thread> workers;
      workers.reserve(getNbThreads());
      reserve_thread_lines(getNbThreads());

      for (size_t id = 0; id < getNbThreads(); ++id)
      {
        workers.emplace_back(
          // [&, id]() // (before AI fix)
          [func_copy = std::forward<Func>(func),
           args_tuple = std::make_tuple(std::forward<Args>(args)...),
           id]() mutable
        {
          local_thread_index = id;          
          if(isKilled()) return; 
          printsln("Worker started");          
          local_thread_role  = ThreadRole::Worker;
          std::apply(std::move(func_copy), std::move(args_tuple));
          // std::invoke(func, args...);// (before AI fix)
          printsln("Worker go to sleep");          
        });
      }

      // 3. Wait for Threads
      for (auto& t : workers) if (t.joinable()) t.join();

      local_thread_role  = ThreadRole::Master;
      activated.store(false);

      // 4. Handle Exit Request
      if (isKilled())
      {
        if (isActivated() || 1 < getNbThreads())
        {
          lock_mutex lock(cout_mutex);
          Colib::Terminal::move_down(getNbThreads());
          std::cout << std::endl << "[MT] Exiting after threads completion" << std::endl;
        }
        ::_exit(42);
      }

      reset_after_threads(getNbThreads());
      runFinaliseCallbacks(getNbThreads());
      std::cout << std::endl;
      std::cout << "Parallel processing over" << std::endl;;
    }

    /**
     * @brief Template helper to initialize and run in one go.
     * @warning Changes the global thread count setting.
     */
    template <size_t nbThreads, class Func, class... Args>
    void parallelise_function(Func&& func, Args&&... args)
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

    
    template<class T>
    std::vector<std::vector<T>> distribute(std::vector<T> const & initVec)
    {
      std::vector<std::vector<T>> ret;
      
      auto nbThreads = getNbThreads();
      if (nbThreads <= 1) nbThreads = 1;

      ret.resize(nbThreads);

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

    // template<class T, class Func, class... Args>
    // void parallelFor(std::vector<T> vector, Func&& func, Args&&... args)
    // {
    //   auto const distributedVec = distribute(vector);
    //   parallelise_function()
    // }


    ///////////////////////
    // Producer Consumer //
    ///////////////////////

    template <typename T>
    class SafeQueue 
    {
        std::queue<T> q;
        std::mutex m;
        std::condition_variable cv;
        bool finished = false;

    public:
      void push(T val) 
      {
        {
          lock_mutex lock(m);
          q.push(std::move(val));
        }
          cv.notify_one();
      }

      void set_finished() 
      {
        {
          lock_mutex lock(m);
          finished = true;
        }
        cv.notify_all();
      }

      bool pop(T& out) 
      {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this] { return !q.empty() || finished; });
        if (q.empty()) return false;
        out = std::move(q.front());
        q.pop();
        return true;
      }
    };

    /**
     * @brief EXPERIMENTAL Master thread produces data, Workers consume it in parallel.
     * @attention EXPERIMENTAL
     * @param producer A function: std::optional<T>(void). Return std::nullopt when done.
     * @param consumer A function: void(T data).
     */
    template <typename Producer, typename Consumer>
    void parallelise_stream(Producer&& producer, Consumer&& consumer)
    {
      // Automatically deduce T from the Producer's return type
      // We expect Producer to return std::optional<T>, so we grab the 'value_type'
      using ReturnType = std::invoke_result_t<Producer>;
      using T = typename ReturnType::value_type;

      size_t total_threads = getNbThreads();
      
      if (total_threads <= 1) 
      {
        local_thread_index = 0;
        while (auto data = producer()) 
        {
          if (isKilled()) break;
          consumer(std::move(*data));
        }
        runFinaliseCallbacks(total_threads);
        return;
      }

      reserve_thread_lines(total_threads);
      
      auto queue = std::make_shared<SafeQueue<T>>();
      activated.store(true);

      std::vector<std::thread> workers;
      workers.reserve(total_threads - 1);

      auto shared_consumer = std::make_shared<std::decay_t<Consumer>>(std::forward<Consumer>(consumer));

      // 2. Launch Workers
      for (size_t id = 1; id < total_threads; ++id) 
      {
        workers.emplace_back([queue, shared_consumer, id]() 
        {
          local_thread_index = id;
          local_thread_role = ThreadRole::Worker;
          T item;
          while (queue.pop(item)) 
          {
            if (isKilled()) break;
            (*shared_consumer)(std::move(item));
          }
        });
      }

      // 3. Master Thread
      local_thread_index = 0;
      local_thread_role = ThreadRole::Master;
      
      try {
        while (auto data = producer()) {
            if (isKilled()) break;
            queue->push(std::move(*data));
        }
      } catch (...) {
          queue->set_finished(); // Ensure workers don't hang on producer error
          throw;
      }

      queue->set_finished();
      for (auto & t : workers) if (t.joinable()) t.join();

      activated.store(false);
      reset_after_threads(total_threads);

      if (isKilled())
      {
        Colib::MT::inject_print([&]() { 
            std::cout << "Ctrl+C caught. Waiting for threads to finish... Press again to force quit." << Colib::Terminal::NEWLINE;
          });
        ::_exit(42);
      }
      runFinaliseCallbacks(total_threads);
    }
  }
}