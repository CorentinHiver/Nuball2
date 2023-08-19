#ifndef MTTHIST_H
#define MTTHIST_H
// #define MTTHIST_MONO
#include "MTObject.hpp"
#include "MTCounter.hpp"

/**
 * @brief Multithreading wrapper for all THist spectra of root library
 * @author corentin.hiver@ijclab.in2p3.fr
 * @details
 *
 * Inspiration :
 * https://root.cern.ch/doc/master/TThreadedObject_8hxx_source.html#l00167
 *
 * \attention As for any class deriving from MTObject, first initialize the number of threads :
 * 
 *      MTObject::Initialize(nb_threads)
 * 
 * 
 * Instantiate this class as follow : 
 * 
 *      MTTHist<TH1F> some_TH1F_histo("name", "title:xaxis:yaxis", bins, min, max);
 * 
 * Or
 * 
 *      MTTHist<TH1F> some_TH1F_histo;
 *      some_TH1F_histo.reset("name", "title:xaxis:yaxis", bins, min, max);
 * 
 * \attention Please do not use the copy constructors and operators, may need proper work to make it safe
 * 
 * In default mode, nb_threads histograms are created
 * \test In mono mode (MTTHIST_MONO), only one histogram is created UNDER DEVELOPPEMENT
 * 
 * To fill the histogram from threads : 
 * 
 *          some_TH1F_histo.Fill()
 * 
 * Once the histogram have been filled, two options : 
 * 
 * - Either write it down directly : 
 * 
  *        // Open a TFile
  *        some_TH1F_histo.Write()
  *        // Write and close the TFile
 * 
 * - Or you can merge the histograms :
 * 
 *        some_TH1F_histo.Merge();
 *
 *    You can then address the merged histogram using -> : 
 *        
 *        some_TH1F_histo->Integral();
 * 
 * Special case : you can also instanciate and write down one histogram per thread.
 * 
 * Example : 
 *        void single_thread(MTTHist & histo)
 *        {
 *          histo.reset("name","title", args...);
 *          ... Fill the histo
 *          histo.Write();
 *        }
 * 
 *        int main()
 *        {
 *          MTTHist<TH1> on_histo_per_thread;
 *          std::parallelise_function(single_thread, on_histo_per_thread);
 *          return 1;
 *        }
 * 
 * In this case each histogram is kept separated, no merge occurs. Use this if you want to make the benefits of the user defined
 * method of this class or for consistency with the rest of the code. It is equivalent to declare the histogram directly inside 
 * the funcion
 * 
 * @todo better memory management
 */
// class MTTHist : public MTObject
template <class THist>
class MTTHist
{
public:
  /**
   * @brief Construct a new MTTHist object and send the arguments directly to the underlying root histogramm
   */
  template <class... ARGS>
  MTTHist(ARGS &&... args) 
  {
    (sizeof...(args)==0) ? this -> reset(nullptr) : this -> reset(std::forward<ARGS>(args)...);
  }

  /**
   * @brief Wrapper around an already existing histograms
   * 
   * @attention You'll have to manage the life of the pointer
   */
  template <class... ARGS>
  MTTHist(THist* hist) {m_merged = hist; m_exists = true;}

  /**
   * @brief Copy constructor
   */
  template <class... ARGS>
  MTTHist<THist>(MTTHist<THist> const & hist) : 
    m_comment  (hist.m_comment),
    m_str_name (hist.m_str_name),
    m_file     (hist.m_file),
    m_exists   (hist.m_exists),
    m_written  (hist.m_written),
    m_integral (hist.m_integral),
    m_merged   (hist.m_merged)
    #ifndef MTTHIST_MONO
    ,
    m_is_merged  (hist.m_is_merged),
    m_collection (hist.m_collection),
    m_is_deleted (hist.m_is_deleted)
    #endif // NO MTTHIST_MONO
    {
      throw_error("NO COPY CONSTRUCTOR WORKING YET FOR MTTHist");
    }
    
  /**
   * @brief Copy assignement
   * @attention Please do not use, doesn't seems very safe ...
  */
  void operator=(MTTHist<THist> hist)
  {
    // throw_error("NO COPY ASSIGNEMENT WORKING YET FOR MTTHist");
    m_comment  = hist.m_comment;
    m_str_name = hist.m_str_name;
    m_file     = hist.m_file;
    m_exists   = hist.m_exists;
    m_written  = hist.m_written;
    m_integral = hist.m_integral;
    m_merged   = hist.m_merged;
  #ifndef MTTHIST_MONO
    m_is_merged  = hist.m_is_merged;
    m_collection = hist.m_collection;
    m_is_deleted = hist.m_is_deleted;
  #endif // NO MTTHIST_MONO
  }
  void operator=(std::nullptr_t) {reset(nullptr);}


  ~MTTHist();


  // ---   GENERIC INITIALIZATION --- //

  /**
   * @brief Copy initializer.
   * @attention Please do not use, doesn't seem very safe ...
   */
  template <class... ARGS>
  void reset(MTTHist<THist> hist) { debug("dont copy MTTHist"); m_exists = false; m_merged = static_cast<THist*>(hist.m_merged); m_collection = hist.m_collection;}

  /**
   * @brief Unsafe
  */
  template <class... ARGS>
  void reset(THist *hist) {m_exists = true; m_merged = hist;}

  //Default initialiser, to create a zombie :
  template <class... ARGS>
  void reset() {m_exists = false;}
  
  //Nullptr initialiser, to create a zombie as default initializer :
  template <class... ARGS>
  void reset(std::nullptr_t)
  {
    m_exists = false;
    delete m_merged;
    for (size_t i = 0; i<m_is_deleted.size(); i++)
    {
      if (!m_is_deleted[i]) delete m_collection[i];
      m_is_deleted[i] = true;
    }
    
  }

  //General initialiser to construct any root histogram vector starting with its name:
  template <class... ARGS>
  void reset(std::string name, ARGS &&... args);

  // Initialize only one histogram at a time
  template <class... ARGS>
  void resetOne(std::string name, ARGS &&... args);

  // ---   GENERIC FILL --- //

  template <class... ARGS>
  void Fill(ARGS &&... args);

  // --- COMMON METHODS --- //
  void Print();
  void Write();
  void Write_i(int const & thread_index);
  
  std::string const & name() const {return m_str_name;}

  bool const & exists() const {return m_exists;}  
  bool const & isWritten() const {return m_exists;}  
  operator bool() const & {debug("coucou");return m_exists;}

  std::vector<THist*> const & getCollection() const {return m_collection;}
  #ifdef SAFE
  THist * operator->() {if (m_merged) return m_merged; else print("MTTHist not merged !!!");}
  #else 
  THist * operator->() {return m_merged;}
  #endif //SAFE

  operator THist*() {return m_merged;}
  THist * get() {return m_merged;}
  TFile * file() {return m_file;}
  auto size() const {return m_collection.size();}

  #ifdef MTTHIST_MONO
  THist * Get() {return m_merged;}

  #else // not MTTHIST_MONO :
  void Merge();
  void Merge_t(); // Used in multithreading
  void static Merge_thread(MTTHist<THist> & histo); // Used in multithreading
  THist * Merged();
  THist * Get(ushort const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[MTObject::getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}

  auto const & Integral() const {return m_integral;}

  #endif //MTTHIST_MONO

  std::string const & GetName() const {return m_str_name;}

  void setComment(std::string const & comment) {m_comment = comment;}
  std::string const & readComment(std::string const & comment) const {return m_comment;}

  std::mutex m_mutex;
private:

  std::string m_comment;
  std::string m_str_name;
  TFile* m_file = nullptr;
  bool m_exists = false;
  bool m_written = false;
  ulonglong m_integral = 0ull;

  THist* m_merged = nullptr;

  #ifndef MTTHIST_MONO
  bool m_is_merged = false;
  std::vector<THist*> m_collection;
  std::vector<bool> m_is_deleted;
  #endif //MTTHIST_MONO

  static MTCounter m_nb_thread_running;     // Attempt to make the merging in parallel
  static std::condition_variable m_condition;// Attempt to make the merging in parallel

};


template<class THist> MTCounter MTTHist<THist>::m_nb_thread_running{0};
template<class THist> std::condition_variable MTTHist<THist>::m_condition;

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::reset(std::string name, ARGS &&... args)
{
  // Extract some informations :
  m_file = gROOT -> GetFile();// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
  m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
  m_str_name = name;

  if (MTObject::ON)
  {
  #ifdef MTTHIST_MONO
    if (!m_merged) m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);
  #else // not MTTHIST_MONO
   
    lock_mutex lock(m_mutex);
    if (MTObject::isMasterThread())
    { // If we are in the master thread, then it means we will merge the histograms afterwards
      for (size_t histo = 0; histo<m_collection.size(); histo++) if (!m_is_deleted[histo]) delete m_collection[histo]; 
      auto const & size = MTObject::getThreadsNb();
      m_collection.resize(size);
      m_is_deleted.resize(size);
      for (size_t i = 0; i<size; i++) 
      {
        m_collection[i] = new THist ((name+"_"+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
        m_is_deleted[i] = false;
      }
    }
    else
    {
      auto const & thread_nb = MTObject::getThreadsNb();
      if (m_collection.size()<thread_nb)
      {
        m_collection.resize(thread_nb);
        m_is_deleted.resize(thread_nb, true);
      }
      auto const & i = MTObject::getThreadIndex();
      m_collection[i] = new THist (name.c_str(), std::forward<ARGS>(args)...);
      m_is_deleted[i] = false;
    }
  #endif //MTTHIST_MONO
  }
  else // If MTObject is OFF
  {
    m_collection.resize(1);
    m_collection[0] = new THist ((name).c_str(), std::forward<ARGS>(args)...);
    m_merged = m_collection[0];
  }
}

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::Fill(ARGS &&... args)
{
  m_integral++;
#ifdef MTTHIST_MONO
  lock_mutex lock(m_mutex);
  m_merged -> Fill(std::forward<ARGS>(args)...);

#else // not MTTHIST_MONO :
  m_collection[MTObject::getThreadIndex()]->Fill(std::forward<ARGS>(args)...);

#endif //MTTHIST_MONO
}

#ifndef MTTHIST_MONO
template <class THist>
inline void MTTHist<THist>::Merge_t()
{
  m_file = gROOT -> GetFile();
  if (m_file) gROOT -> cd(); //To get out of scope of any potential TFile
  m_merged = static_cast<THist*> (m_collection[0]->Clone(m_str_name.c_str()));
  for (size_t i = 1; i<m_collection.size(); i++) 
  {
    auto & histo = m_collection[i];
    if (histo->Integral() > 0) m_merged -> Add(histo);
    delete histo;
    m_is_deleted[i] = true;
  }
  if (m_file) m_file -> cd(); //To return to the scope of any potential TFile
}

// We could translate this inside MTObject after testing it really works
template <class THist>
inline void MTTHist<THist>::Merge_thread(MTTHist<THist> & Histos)
{
  std::unique_lock<std::mutex> lock(Histos.m_mutex);
  while (m_nb_thread_running>=MTObject::getThreadsNb()) 
  {
    Histos.m_condition.wait(lock);
  }
  m_nb_thread_running.increment();
  lock.unlock();

  Histos.Merge_t();

  lock.lock();
  m_nb_thread_running.decrement();
  Histos.m_condition.notify_one();
}

template<class THist>
void MTTHist<THist>::Merge()
{
  if (!m_exists || m_collection.size()==0 || m_collection[0] -> IsZombie() || this -> Integral() < 1)
  {
    m_exists = false;
    m_is_merged = false;
  }
  else
  {
    if (MTObject::ON)
    {
      lock_mutex lock(m_mutex);
      this -> Merge_t();
    }
    else
    {
      m_merged = m_collection[0];
    }
    m_is_merged = true;
  }
}

template<class THist>
THist* MTTHist<THist>::Merged()
{
  if (!m_is_merged)
  {
    print("No merged histogram yet");
    return (new THist());
  }
  else return m_merged;
}
  #endif //not MTTHIST_MONO

template<class THist>
void MTTHist<THist>::Write_i(int const & thread_index)
{
  if  (   !m_exists
        ||!m_collection[thread_index]
        || m_collection[thread_index] -> IsZombie()
        || m_collection[thread_index] -> Integral() < 1
      ) return;
  else
  {
    if (m_is_deleted[thread_index]) return;
    m_collection[thread_index] -> Write();
    delete m_collection[thread_index];
    m_collection[thread_index] = nullptr;
    m_is_deleted[thread_index] = true;
    m_written = true;
  }
}

template<class THist>
void MTTHist<THist>::Write()
{
  if (MTObject::ON && MTObject::isMasterThread()) this -> Merge();
  if (MTObject::ON && !MTObject::isMasterThread()) this -> Write_i(MTObject::getThreadIndex());
  else
  {
    if (   !m_exists
        || !m_merged
        || m_merged -> IsZombie()
        || m_merged -> Integral() < 1) return;
    else
    {
      m_merged -> Write(m_str_name.c_str(), TROOT::kOverwrite);
      m_written = true;
    }
  }
}

template <class THist>
void MTTHist<THist>::Print()
{
  print("_________________");
  print(m_str_name);
  print(m_collection.size(), "histograms :");
  for (auto const & histo : m_collection) print(histo, (histo) ? histo->Integral() : 0, "counts");
  print();
}

template <class THist>
MTTHist<THist>::~MTTHist()
{
#ifdef MTTHIST_MONO
  if (!m_file && m_merged) delete m_merged;

# else // not MTTHIST_MONO
if (MTObject::ON)
{
  for (size_t i = 0; i<m_is_deleted.size(); i++)
  {
    auto & histo = m_collection[i];
    if (!m_is_deleted[i]) delete histo;
  }
  if (!m_file && m_exists)
  {
    // for (auto histo: m_collection) delete histo;
    // delete m_merged;
  }
}
else 
{
  delete m_merged;
}

#endif //MTTHIST_MONO
}

template <class THist>
using Vector_MTTHist = std::vector<MTTHist<THist>>;

#endif //MTTHIST_H
