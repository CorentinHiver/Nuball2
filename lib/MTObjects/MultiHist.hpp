#ifndef MULTITHIST_HPP
#define MULTITHIST_HPP
// #define MULTITHIST_MONO Not tested !!
#include "MTObject.hpp"
#include "../libCo.hpp"
#include "TFile.h"
#include "TROOT.h"

/**
 * @brief Multithreading wrapper for all THist spectra of root library
 * @author corentin.hiver@free.fr
 * @details
 *
 * Inspiration :
 * https://root.cern.ch/doc/master/TThreadedObject_8hxx_source.html#l00167
 *
 * \attention As for any class using my MTObject multithreading manager, Initialise the number of threads 
 * BEFORE instantiating any MTObject :
 * 
 *      MTObject::Initialise(nb_threads)
 * 
 * Instantiation :
 * 
 *      MultiHist<TH1F> some_TH1F_histo("name", "title:xAxis:yAxis", bins, min, max);
 * 
 * Or
 * 
 *      MultiHist<TH1I> some_TH1F_histo;
 *      some_TH1F_histo.reset("name", "title:xAxis:yAxis", bins, min, max);
 * 
 * You can also use a container : 
 * 
 *      Vector_MultiHist<TH1D> histograms_vec;
 *      emplaceMultiHist(histograms_vec, "name", "title:xAxis:yAxis", bins, min, max);
 * 
 *      Map_MultiHist<TH2F> histograms_map;
 *      emplaceMultiHist(histograms_map, label, "name", "title:xAxis:yAxis:xAxis", binsX, minX, maxX, binsY, minY, maxY)
 * 
 * In default mode, [nb_threads] sub_histograms are created
 * 
 * To fill the histogram from within one thread : 
 * 
 *          some_TH1F_histo[MTObject::getThreadIndex()]->Fill(<normal arguments for TH1*::Fill()>)
 * 
 * Or in a more concise way (prefered) :
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
class MultiHist
{
public:

  /// @brief Construct a new MultiHist object and send the arguments directly to the underlying root histogram
  template <class... ARGS>
  MultiHist(ARGS &&... args) 
  {
    this -> reset(std::forward<ARGS>(args)...);
  }

  /// @brief Default initializator
  template <class... ARGS>
  MultiHist()
  {
    this->reset(nullptr);
  }

  ~MultiHist();

  /// @brief Wrapper around an already existing histograms
  MultiHist(THist* hist) : m_merged(hist)
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
      m_is_merged = m_outScope = true;
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
      m_collection.reserve(1);
      m_merged = hist; m_is_merged = true; 
      m_exists = m_outScope = true; m_integral = hist->Integral(); 
      m_name = hist->GetName(); m_title = hist->GetTitle();
    }
  }

  /// @brief Copy constructor
  // template <class... ARGS>
  // MultiHist<THist>(MultiHist<THist> const & hist) : 
  //   m_comment    (hist.m_comment  ),
  //   m_name       (hist.m_name     ),
  //   m_exists     (hist.m_exists   ),
  //   m_is_written (hist.m_is_written  ),
  //   m_integral   (hist.m_integral ),
  //   m_merged     (hist.m_merged   )
  //   #ifndef MULTITHIST_MONO
  //   ,
  //   m_is_merged  (hist.m_is_merged),
  //   m_collection (hist.m_collection)
  //   #endif // NO MULTITHIST_MONO
  //   {
  //     throw CopyError();
  //   }
    
  /// @brief Move constructor
  // template <class... ARGS>
  // MultiHist<THist>(MultiHist<THist> && hist) : 
  //   m_comment  (std::move(hist.m_comment  )),
  //   m_name     (std::move(hist.m_name     )),
  //   m_exists   (std::move(hist.m_exists   )),
  //   m_is_written  (std::move(hist.m_is_written  )),
  //   m_integral (std::move(hist.m_integral )),
  //   m_merged   (std::move(hist.m_merged   ))
  // #ifndef MULTITHIST_MONO
  //   ,
  //   m_is_merged  (std::move(hist.m_is_merged )),
  //   m_collection (std::move(hist.m_collection))
  // #endif // NO MULTITHIST_MONO
  // {
  //   // The moved object is now in an undefined state.
  //   // It is better to put it back to a defined state :
  //   hist.cleanMove(); 
  // }

  void clean()
  {
    m_comment  = "";
    m_name = "";
    m_exists = false;
    m_is_written  = false;
    m_integral = 0ull;
    if (!m_collection.empty()) for (auto & histo : m_collection) delete histo;
    else delete m_merged;
    m_collection.clear();
    m_merged = nullptr;
    m_is_merged  = false;
  }

  void cleanMove()
  {
    m_comment  = "";
    m_name = "";
    m_exists = false;
    m_is_written  = false;
    m_integral = 0ull;
    if (!m_collection.empty()) for (auto & histo : m_collection) delete histo;
    else delete m_merged;
    m_collection.clear();
    m_merged = nullptr;
    m_is_merged  = false;
  }

  // ---   GENERIC INSTANTIATION --- //

  /// @brief Copy Initializer. @attention Forbidden so far ! (lead to many errors ...)
  MultiHist<THist> & reset(MultiHist<THist> const & hist) 
  { 
    // Execution stops here : it is forbidden to copy a MultiHist, because of the memory handling of root !!
    throw CopyError(hist);
  }

  MultiHist<THist> & operator=(MultiHist<THist> const & hist)
  {
    return this->reset(hist);
  }

    // /**
    //  * @brief Unsafe
    // */
    // template <class... ARGS>
    // void reset(THist *hist) {m_exists = true; m_merged = hist;}

  // Default initializer, to create a zombie :
  // template <class... ARGS>
  // void reset() {m_exists = false;}

  //Nullptr initializer :
  template <class... ARGS>
  void reset(std::nullptr_t)
  {
    m_exists = false;
    if (!m_collection.empty()) for (auto & histo : m_collection) delete histo;
    else delete m_merged;
    m_merged = nullptr;
  }

  //General initializer to construct any root histogram vector starting with its name:
  template <class... ARGS>
  void reset(std::string name, ARGS &&... args);

  // ---   GENERIC FILL --- //

  template <class... ARGS>
  void Fill(ARGS &&... args) noexcept;


  // --- COMMON METHODS --- //
  template <class... ARGS>
  void SetBinContent(ARGS &&... args) noexcept;
  template <class... ARGS>
  auto GetBinContent(ARGS &&... args) noexcept;
  void Print();
  void Write();
  // void Write_i(int const & thread_index);
  
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
      debug("in THist::operator->() : MultiHist", m_name ,"not merged !!! Merging ..."); 
      this -> Merge();
      if (m_merged) return m_merged;
      else return nullptr;
    }
  }

  THist * operator->() const 
  {
    if (m_merged) return m_merged; 
    else 
    {
      debug("in THist::operator->() : MultiHist", m_name ,"not merged !!! Can't merge because calling the const method."); 
      return nullptr;
    }
  }

#endif //UNSAFE

  THist * get() {return m_merged;}
  THist * get() const {return m_merged;}
  auto size() const {return m_collection.size();}

  #ifdef MULTITHIST_MONO
  THist * Get() {return m_merged;}

  #else // not MULTITHIST_MONO :
  THist* Merge();
  THist* Merge() const {static_assert(true, "Can't merge a const MultiHist");}
  void Merge_t(); // Used in multithreading
  // void static Merge_thread(MultiHist<THist> & histo); // Used in multithreading
  THist * Merged();
  THist * Get(ushort const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[MTObject::getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}

  auto const & Integral() const {return m_integral;}

  #endif //MULTITHIST_MONO

  void setComment(std::string const & comment) {m_comment = comment;}
  std::string const & readComment() const {return m_comment;}


  static bool const & verbose(bool const & _verbose = true) {return (m_verbose = _verbose);}
  static bool & verbosity() {return m_verbose;}

  auto & mutex() {return m_mutex;}

private:

  std::string m_comment;
  std::string m_name   ;
  std::string m_title  ;

  bool m_exists   = false;
  bool m_is_written  = false;
  bool m_outScope = false;
  
  ulonglong m_integral = 0ull;

  THist* m_merged = nullptr;
  bool m_merged_deleted = false;

  std::mutex m_mutex;

#ifndef MULTITHIST_MONO
  bool m_is_merged = false;
  std::vector<THist*> m_collection;
#endif //MULTITHIST_MONO

  // static MTCounter m_nb_thread_running;      // Attempt to make the merging in parallel
  // static std::condition_variable m_condition;// Attempt to make the merging in parallel
  static bool m_verbose;

public:
  class CopyError
  {public:
    CopyError(MultiHist<THist> const & histo) 
    {
      error("Can't copy MultiHist !! (", histo.name(), "). You need to std::move() it.");
    }
  };
};

template<class THist> 
bool MultiHist<THist>::m_verbose = true;

// template<class THist> MTCounter MultiHist<THist>::m_nb_thread_running{0};
// template<class THist> std::condition_variable MultiHist<THist>::m_condition;

template <class THist>
template <class... ARGS>
inline void MultiHist<THist>::reset(std::string name, ARGS &&... args)
{
  // Extract some informations :
  m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array or map of histograms)
  m_name = name;

  if (MTObject::ON)
  {
    lock_mutex lock(m_mutex);

  #ifdef MULTITHIST_MONO
    if (!m_merged) m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);
  #else // not MULTITHIST_MONO
   
    auto const & thread_nb = MTObject::getThreadsNb();
    auto const & thread_id = MTObject::getThreadIndex();
    if (!MTObject::isMasterThread() && m_collection.size() > 0 && !m_collection[thread_id] && thread_id<m_collection.size())
    {// If the initialisation takes place inside a thread, special care is needed
      m_collection[thread_id] = new THist ((name+"_"+std::to_string(thread_id)).c_str(), std::forward<ARGS>(args)...);
      m_collection[thread_id] -> SetDirectory(nullptr);
    }
    else
    {// In most situations, we end up here
      if (!m_collection.empty())
      {// First delete any previous objects (should never be the case but who knows)
        for (auto & histo : m_collection) delete histo;
        m_collection.clear();
      }
      // Reserve enough space in the vector to hold all the histograms :
      m_collection.resize(thread_nb, nullptr);
      
      // Fill the vector of histograms:
      for (size_t i = 0; i<thread_nb; i++) 
      {
        m_collection[i] = new THist ((name+"_"+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
        m_collection[i]->SetDirectory(nullptr);
      }
    }

  #endif //MULTITHIST_MONO
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
inline void MultiHist<THist>::SetBinContent(ARGS &&... args) noexcept
{
  m_integral-=m_collection[MTObject::getThreadIndex()]->Integral();
  m_collection[MTObject::getThreadIndex()]->SetBinContent(std::forward<ARGS>(args)...);
  m_integral+=m_collection[MTObject::getThreadIndex()]->Integral();
}

template <class THist>
template <class... ARGS>
inline auto MultiHist<THist>::GetBinContent(ARGS &&... args) noexcept
{
  return m_collection[MTObject::getThreadIndex()]->GetBinContent(std::forward<ARGS>(args)...);
}

template <class THist>
template <class... ARGS>
inline void MultiHist<THist>::Fill(ARGS &&... args) noexcept
{
  ++m_integral;
#ifdef MULTITHIST_MONO
  lock_mutex lock(m_mutex);
  m_merged -> Fill(std::forward<ARGS>(args)...);

#else // not MULTITHIST_MONO :
  m_collection[MTObject::getThreadIndex()]->Fill(std::forward<ARGS>(args)...);

#endif //MULTITHIST_MONO
}

#ifndef MULTITHIST_MONO
template <class THist>
inline void MultiHist<THist>::Merge_t()
{
  // Set the merged histogram to the address of the first histogram
  m_merged = m_collection[0];
  if (MTObject::ON)
  {
    for (size_t i = 1; i<m_collection.size(); i++) 
    {
      auto & histo = m_collection[i]; // Simple alias to call the histogram
      if (!histo || histo->IsZombie()) continue; // If no pointer or already deleted histogram, do not treat
      if (histo->Integral() > 0) m_merged -> Add(histo);
      delete histo;
    }
  }
  m_collection.clear();
  m_merged->SetName(m_name.c_str());
  m_merged->SetTitle(m_title.c_str());
  m_is_merged = true;
  m_integral = m_merged->Integral();
}

// // We could translate this inside MTObject after testing it really works
// /// @brief do no work
// template <class THist>
// inline void MultiHist<THist>::Merge_thread(MultiHist<THist> & Histos)
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

/// @brief Merges the histograms
template<class THist>
THist* MultiHist<THist>::Merge()
{
  if (m_is_merged) return m_merged; // Can only merge once
  if (!m_exists || !m_collection[0] || this -> Integral() < 1)
  {
    debug("problem with", m_name, " : exists ?", (m_exists) ? "oui" : "non", " | nb histos : ", m_collection.size(),
          "| integral = ", this -> Integral());
    m_is_merged = false;
    delete m_merged;
    m_merged = nullptr;
  }
  else if (!m_is_merged) this -> Merge_t();
  return m_merged;
}

template<class THist>
THist* MultiHist<THist>::Merged()
{
  #ifdef DEBUG
  if (!m_is_merged) print("No merged histogram yet for");
  #endif //DEBUG
  return m_merged;
}
#endif //! MULTITHIST_MONO

template<class THist>
void MultiHist<THist>::Write()
{
  if (!gFile) {error("MultiHist<THist>::Write() : No output file selected for", m_name); return;}
  bool ready_for_write = false;
  m_is_written = false;

  // If the histogram has already been merged :
  if (m_exists && m_is_merged && m_merged && !m_merged->IsZombie()) ready_for_write = true;
  else 
  {
    if (MTObject::ON) 
    {
      if (MTObject::isMasterThread())
      {
        this -> Merge();
        if (m_is_merged && m_exists && m_merged && !m_merged -> IsZombie() && (m_merged -> Integral() > 0)) ready_for_write = true;
      }
      else error("Please write the MultiHist in the master thread");
    }
    else error("Error in MultiHist<THist>::Write() : histogram", m_name, "do not exist (so far, monothread is not working I think ...)");
  }

  if (ready_for_write)
  {
    if (m_verbose) 
    {
      print("writing", m_name);
      if (MTObject::getThreadsNb()*m_merged->GetNcells() > 1.e9) print("Might take some time ..."); // If there is more than 1B bins in total
    }
    m_merged -> SetDirectory(gFile);
    // m_merged -> Write();
    // m_merged -> Write(m_name.c_str());
    m_merged -> Write(m_name.c_str(), TROOT::kOverwrite);
    m_is_written = true;
  }
}

/// @brief Print operator
template <class THist>
std::ostream& operator<<(std::ostream& out, MultiHist<THist> const & histo)
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
void MultiHist<THist>::Print()
{
  print(*this);
}

template <class THist>
MultiHist<THist>::~MultiHist()
{
  if (!m_is_written) 
  { // If the histogram have been written then ROOT handles the memory
    if (MTObject::ON) for (auto & histo : m_collection) delete histo;
    else delete m_merged;
    m_collection.clear();
  }
}

// Useful containers

template <class THist>
using Vector_MultiHist = std::vector<MultiHist<THist>>;

template <class THist, typename T = int>
using Map_MultiHist = std::unordered_map<T, MultiHist<THist>>;

// Useful containers' methods

template<class THist, typename... ARGS>
void emplaceMultiHist(std::vector<THist>& vec, ARGS&&... args) {
    vec.emplace_back(std::forward<ARGS>(args)...);
}

/**
 * @brief Create in place a new MultiHist in an Map_MultiHist.
 * 
 * Skips the usual move operation by the use of std::piecewise_construct : it is LITTERALLY built in place
 * 
 * @tparam THist: Any TH1* type from TH1I to TH3D
 * @tparam T: Key type (default int). Can be string or anything you want that can works as a key.
 * @tparam ARGS: Generic template variadic argument
 * @param map: Map_MultiHist<THist, T>. 
 * @param label: Must be unique
 * @param args: Usual arguments of MultiHist (see its definition). e.g. "name", "title", nbins, min, max)
 */
template<class THist, typename T, typename... ARGS>
void emplaceMultiHist(Map_MultiHist<THist, T> & map, T const & label, ARGS&&... args) 
{
  map.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(label),
    std::forward_as_tuple(std::forward<ARGS>(args)...)
  );
}

#endif //MULTITHIST_HPP