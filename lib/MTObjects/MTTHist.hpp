#ifndef MTTHIST_HPP
#define MTTHIST_HPP
// #define MTTHIST_MONO
#include "MTObject.hpp"
#include "MTCounter.hpp"

/**
 * @brief Multithreading wrapper for all THist spectra of root library
 * @author corentin.hiver@free.fr
 * @details
 *
 * Inspiration :
 * https://root.cern.ch/doc/master/TThreadedObject_8hxx_source.html#l00167
 *
 * \attention As for any class using my multithreading, initialize the number of threads 
 * BEFORE instantiating any object :
 * 
 *      MTObject::Initialize(nb_threads)
 * 
 * 
 * 
 * Instantiation :
 * 
 *      MTTHist<TH1F> some_TH1F_histo(<normal arguments for TH1F objects>);
 * 
 * Or
 * 
 *      MTTHist<TH1F> some_TH1F_histo;
 *      some_TH1F_histo.reset("name", "title:xaxis:yaxis", bins, min, max);
 * 
 * In default mode, [nb_threads] sub_histograms are created
 * \test In mono mode (MTTHIST_MONO), only one histogram is created UNDER DEVELOPPEMENT
 * 
 * To fill the histogram from within one thread : 
 * 
 *          some_TH1F_histo[MTObject::getThreadIndex()]->Fill(<normal arguments for TH1*::Fill()>)
 * 
 * Or in a more concise way (preferred) :
 * 
 *          some_TH1F_histo.Fill(<normal arguments for TH1*::Fill()>)
 * 
 * Once the histograms have been filled, two options : 
 * 
 * Either write it down directly : 
 * 
 *         // Open a TFile and cd() inside
 *         some_TH1F_histo.Write(); //Merges the sub_histograms then write it if it has counts
 *         // Write and close the TFile
 * 
 * Or you can merge the histograms :
 * 
 *        some_TH1F_histo.Merge();
 *
 *    You can then address the merged histogram using -> : 
 *        
 *        some_TH1F_histo->Integral();
 *        some_TH1F_histo->GetEntries();
 *        some_TH1F_histo->Write();
 *        ...
 */
template <class THist>
class MTTHist
{
public:

  /// @brief Construct a new MTTHist object and send the arguments directly to the underlying root histogramm
  template <class... ARGS>
  MTTHist(ARGS &&... args) 
  {
    this -> reset(std::forward<ARGS>(args)...);
  }

  /// @brief Default initialisator
  template <class... ARGS>
  MTTHist()
  {
    this->reset(nullptr);
  }

  ~MTTHist();


  /// @brief Wrapper around an already existing histograms
  MTTHist(THist* hist) : m_merged(hist)
  {
    if (!hist || hist -> IsZombie()) 
    {
      m_merged = nullptr;
      m_is_merged = m_exists = false;
      m_integral = 0;
      m_name = m_title = "zombie";
    }
    else
    {
      m_collection.push_back(hist);
      m_is_merged = m_outscope = true;
      m_exists = true; 
      m_integral = hist->Integral();
      m_name     = hist->GetName ();
      m_title    = hist->GetTitle();
    }
  }

  void operator=(THist* hist)
  {
    if (!hist || hist -> IsZombie()) 
    {
      m_merged = nullptr;
      m_is_merged = m_exists = false;
      m_integral = 0;
      m_name = m_title = "";
    }
    else
    {
      this -> clean();
      if (m_collection.size()<1) m_collection.reserve(1);
      m_merged = hist; m_is_merged = true; 
      m_exists = m_outscope =true; m_integral = hist->Integral(); 
      m_name = hist->GetName(); m_title = hist->GetTitle();
    }
  }

  /// @brief Copy constructor
  template <class... ARGS>
  MTTHist<THist>(MTTHist<THist> const & hist) : 
    m_comment  (hist.m_comment  ),
    m_name     (hist.m_name     ),
    m_exists   (hist.m_exists   ),
    m_written  (hist.m_written  ),
    m_integral (hist.m_integral ),
    m_merged   (hist.m_merged   )
    #ifndef MTTHIST_MONO
    ,
    m_is_merged  (hist.m_is_merged),
    m_collection (hist.m_collection)
    #endif // NO MTTHIST_MONO
    {
      throw CopyError();
    }
    
  /// @brief Move constructor
  template <class... ARGS>
  MTTHist<THist>(MTTHist<THist> && hist) : 
    m_comment  (std::move(hist.m_comment  )),
    m_name     (std::move(hist.m_name     )),
    m_exists   (std::move(hist.m_exists   )),
    m_written  (std::move(hist.m_written  )),
    m_integral (std::move(hist.m_integral )),
    m_merged   (std::move(hist.m_merged   ))
  #ifndef MTTHIST_MONO
    ,
    m_is_merged  (std::move(hist.m_is_merged )),
    m_collection (std::move(hist.m_collection))
  #endif // NO MTTHIST_MONO
  {
    // The moved object is now in an undefined state.
    // It is better to put it back to a defined state :
    hist.cleanMove(); 
  }

  void cleanMove()
  {
    m_comment  = "";
    m_name = "";
    m_exists = false;
    m_written  = false;
    m_integral = 0ull;
    delete m_merged; m_merged = nullptr;
    if (MTObject::ON) for (auto & histo : m_collection) delete histo;
    m_collection.clear();
    // m_merged = nullptr;
    // for (auto & histo : m_collection) histo = nullptr;
    m_is_merged  = false;
  }

  // void operator=(std::nullptr_t) {reset(nullptr);}


  // // ---   GENERIC INSTANCIATION --- //

  /// @brief Copy initializer.
  MTTHist<THist> & reset(MTTHist<THist> const & hist) 
  { 
    // Execution stops here : it is fobidden so far to copy this object !!
    throw CopyError(hist);

    // If want to use this constructor, one has to make the following good looking
    m_comment  = hist.m_comment ;
    m_name     = hist.m_name    ;
    m_exists   = hist.m_exists  ;
    m_written  = hist.m_written ;
    m_integral = hist.m_integral;
    *m_merged  = new TH1F(*hist.m_merged);
    m_collection.reserve(hist.m_collection.size());
    for (auto const & histo : hist.m_collection) 
    {
      m_collection.push_back(histo->Clone());
      m_collection = hist.m_collection;
      m_is_merged  = hist.m_is_merged;
      return *this;
    }
  }

  MTTHist<THist> & operator=(MTTHist<THist> const & hist)
  {
    return this->reset(hist);
  }

  // // /**
  // //  * @brief Unsafe
  // // */
  // // template <class... ARGS>
  // // void reset(THist *hist) {m_exists = true; m_merged = hist;}

  // //Default initialiser, to create a zombie :
  // template <class... ARGS>
  // void reset() {m_exists = false;}

  //Nullptr initialiser :
  template <class... ARGS>
  void reset(std::nullptr_t)
  {
    m_exists = false;
    delete m_merged;
    if (MTObject::ON) for (auto & histo : m_collection) delete histo;
  }

  //General initialiser to construct any root histogram vector starting with its name:
  template <class... ARGS>
  void reset(std::string name, ARGS &&... args);

  // ---   GENERIC FILL --- //

  template <class... ARGS>
  void Fill(ARGS &&... args) noexcept;

  // --- COMMON METHODS --- //
  void Print();
  void Write(bool const & writeEmpty = false);
  void Write_i(int const & thread_index);
  
  std::string const & GetName() const {return m_name ;}
  std::string const & name   () const {return m_name ;}
  std::string const & title  () const {return m_title;}

  std::string & name () {return m_name ;}
  std::string & title() {return m_title;}

  std::string const & setName (std::string const & _name)  {return (m_name  = _name );}
  std::string const & setTitle(std::string const & _title) {return (m_title = _title);}

  bool const & exists   () const {return m_exists;}
  operator bool()        const & {return m_exists;}

  std::vector<THist*> const & getCollection() const {return m_collection;}
  std::vector<THist*> & getCollection() {return m_collection;}

#ifdef UNSAFE
  THist * operator->() {return m_merged;}
  THist * operator->() const {return m_merged;}
#else // UNSAFE :
  THist * operator->() 
  {
    if (m_merged) return m_merged;
    else 
    {
      debug("MTTHist not merged !!! Merging ..."); 
      this -> Merge();
      return nullptr;
    }
  }
  THist * operator->() const {if (m_merged) return m_merged; else return nullptr;}

#endif //UNSAFE

  THist * get() {return m_merged;}
  THist * get() const {return m_merged;}
  auto size() const {return m_collection.size();}

  #ifdef MTTHIST_MONO
  THist * Get() {return m_merged;}

  #else // not MTTHIST_MONO :
  void Merge();
  void Merge() const {this -> Merge();}
  void Merge_t(); // Used in multithreading
  // void static Merge_thread(MTTHist<THist> & histo); // Used in multithreading
  THist * Merged();
  THist * Get(ushort const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[MTObject::getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}

  auto const & Integral() const {return m_integral;}

  #endif //MTTHIST_MONO


  void setComment(std::string const & comment) {m_comment = comment;}
  std::string const & readComment(std::string const & comment) const {return m_comment;}


  static bool const & verbose(bool const & _verbose = true) {return (m_verbose = _verbose);}
  static bool & verbosity() {return m_verbose;}

  auto & mutex() {return m_mutex;}

private:

  std::mutex m_mutex;

  std::string m_comment;
  std::string m_name   ;
  std::string m_title  ;

  bool m_exists   = false;
  bool m_written  = false;
  bool m_outscope = false;
  
  ulonglong m_integral = 0ull;

  THist* m_merged = nullptr;
  bool m_merged_deleted = false;

#ifndef MTTHIST_MONO
  bool m_is_merged = false;
  std::vector<THist*> m_collection;
#endif //MTTHIST_MONO

  // static MTCounter m_nb_thread_running;      // Attempt to make the merging in parallel
  // static std::condition_variable m_condition;// Attempt to make the merging in parallel
  static bool m_verbose;

public:
  class CopyError
  {public:
    CopyError(MTTHist<THist> const & histo) 
    {
      error("Can't copy MTTHist !! (", histo.name(), "). You need to std::move() it.");
    }
  };
};

template<class THist> 
bool MTTHist<THist>::m_verbose = true;

// template<class THist> MTCounter MTTHist<THist>::m_nb_thread_running{0};
// template<class THist> std::condition_variable MTTHist<THist>::m_condition;

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::reset(std::string name, ARGS &&... args)
{
  // Extract some informations :
  m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
  m_name = name;

  if (MTObject::ON)
  {
    lock_mutex lock(m_mutex);

  #ifdef MTTHIST_MONO
    if (!m_merged) m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);
  #else // not MTTHIST_MONO
   
    auto const & thread_nb = MTObject::getThreadsNb();
    auto const & thread_id = MTObject::getThreadIndex();
    if (!MTObject::isMasterThread() && m_collection.size() > 0 && !m_collection[thread_id] && thread_id<m_collection.size())
    {// If the initialisation takes place inside a thread, special care is needed
      m_collection[thread_id] = new THist ((name+"_"+std::to_string(thread_id)).c_str(), std::forward<ARGS>(args)...);
      m_collection[thread_id] -> SetDirectory(nullptr);
    }
    else
    {// In most situations, we end up here
      if (m_collection.size()>0)
      {// First delete any previous objects
        for (auto & histo : m_collection) delete histo;
        m_collection.clear();
      }
      // Reserve enough space in the vector
      m_collection.resize(thread_nb, nullptr);
      for (size_t i = 0; i<thread_nb; i++) 
      {
        m_collection[i] = new THist ((name+"_"+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
        m_collection[i]->SetDirectory(nullptr);
      }
    }

  #endif //MTTHIST_MONO
  }
  else // If MTObject is OFF
  {
    delete m_merged;

    for (auto & histo : m_collection) delete histo;
    m_collection.clear();
    m_collection.resize(1, nullptr);

    m_merged = new THist ((name).c_str(), std::forward<ARGS>(args)...);
    m_merged->SetDirectory(nullptr);

    m_collection[0] = m_merged;
  }
}

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::Fill(ARGS &&... args) noexcept
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
  m_merged = static_cast<THist*> (m_collection[0]->Clone(m_name.c_str()));
  delete m_collection[0];
  m_merged->SetDirectory(nullptr);
  for (size_t i = 1; i<m_collection.size(); i++) 
  {
    auto & histo = m_collection[i];
    if (!histo) continue;
    if (histo->Integral() > 0) m_merged -> Add(histo);
    delete histo;
    m_collection.clear();
  }
}

// // We could translate this inside MTObject after testing it really works
// /// @brief do no work
// template <class THist>
// inline void MTTHist<THist>::Merge_thread(MTTHist<THist> & Histos)
// {
//   std::unique_lock<std::mutex> lock(Histos.m_mutex);
//   while (m_nb_thread_running>=MTObject::getThreadsNb()) 
//   {
//     Histos.m_condition.wait(lock);
//   }
//   m_nb_thread_running.increment();
//   lock.unlock();

//   Histos.Merge_t();

//   lock.lock();
//   m_nb_thread_running.decrement();
//   Histos.m_condition.notify_one();
// }

template<class THist>
void MTTHist<THist>::Merge()
{
  if (!m_exists || m_collection.size() == 0 || m_collection[0] -> IsZombie() || this -> Integral() < 1)
  {
    m_is_merged = false;
    delete m_merged;
  }
  else if (!m_is_merged)
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
    return (m_merged = new THist());
  }
  else return m_merged;
}
#endif //! MTTHIST_MONO

template<class THist>
void MTTHist<THist>::Write_i(int const & thread_index)
{
  if  (   !m_exists
        || m_collection.size()<1
        ||!m_collection[thread_index]
        || m_collection[thread_index] -> IsZombie()
        || m_collection[thread_index] -> Integral() < 1
      ) return;
  else
  {
    if (m_verbose) print("writting", m_name);
    m_collection[thread_index] -> Write();
    delete m_collection[thread_index];
    m_written = true;
  }
}

template<class THist>
void MTTHist<THist>::Write(bool const & writeEmpty)
{
  if (m_integral<1) return;
  if (MTObject::ON && MTObject::isMasterThread()) this -> Merge();
  if (MTObject::ON && !MTObject::isMasterThread()) this -> Write_i(MTObject::getThreadIndex());
  else
  {
    if (   !m_exists
        || !m_merged
        ||  m_merged -> IsZombie()
        || (!writeEmpty && m_merged -> Integral() < 1)) return;
    else
    {
      if (m_verbose) print("writting", m_name);
      m_merged -> Write(m_name.c_str(), TROOT::kOverwrite);
      m_written = true;
    }
  }
}

template <class THist>
std::ostream& operator<<(std::ostream& out, MTTHist<THist> const & histo)
{
  out << "_________________" << std::endl;
  out << histo.name() << std::endl;
  out << histo.size() << " histogram" << ((histo.size() == 1) ? "s" : "") << " :" << std::endl;
  for (auto const & histo : histo.getCollection()) 
  {
    out << histo << " " << ((histo) ? histo->Integral() : -1) << " counts" << std::endl;
  }
  out << "Total : " << histo.get() << " " << ((histo.get()) ? histo->Integral() : -1) << " counts";
  out << std::endl;
  return out;
}

template <class THist>
void MTTHist<THist>::Print()
{
  print(*this);
}

template <class THist>
MTTHist<THist>::~MTTHist()
{
  // print("---------------");
  // print(*this);
  delete m_merged; m_merged = nullptr;
  for (auto & histo : m_collection) delete histo;
  m_collection.clear();
  // print(*this);

#ifndef MTTHIST_MONO
  // else if (!MTObject::ON && !m_merged_deleted && m_exists && !m_outscope)
  // {
  //   m_merged_deleted = true;
  // }
#endif //!MTTHIST_MONO

}

template <class THist>
using Vector_MTTHist = std::vector<MTTHist<THist>>;

#endif //MTTHIST_HPP