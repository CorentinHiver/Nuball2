#ifndef MTTHIST_H
#define MTTHIST_H
// #define MTTHIST_MONO
#include "MTObject.hpp"

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
 * \test In mono mode (MTTHIST_MONO), only one histogram is created
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
      m_merged(hist.m_merged), 
      m_exists(hist.m_exists), 
      m_collection(hist.m_collection), 
      m_is_deleted(hist.m_is_deleted), 
      m_comment(hist.m_comment),
      m_str_name(hist.m_str_name),
      m_is_merged(hist.m_is_merged)
      {}

  ~MTTHist();


  // ---   GENERIC INITIALIZATION --- //

  /**
   * @brief Copy initializer.
   * @attention Please do not use, doesn't seems very safe ...
   */
  template <class... ARGS>
  void reset(MTTHist<THist> hist) { m_exists = false; m_merged = static_cast<THist*>(hist.m_merged); m_collection = hist.m_collection;}

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
  std::string const & name() const {return m_str_name;}

  bool const & exists() const {return m_written;}  
  bool const & isWritten() const {return m_exists;}  
  operator bool() const & {return m_exists;}

  std::vector<THist*> const & getCollection() const {return m_collection;}
  THist * operator->() {return m_merged;}

  /**
   * @brief Copy operator
   * @attention Please do not use, doesn't seems very safe ...
  */
  void operator=(MTTHist<THist> histo)
  {
    m_file = histo.file();
    m_exists = histo.exists();
    m_str_name = histo.name();
    m_merged = histo.m_merged;
  #ifndef MTTHIST_MONO
    m_collection = histo.getCollection();
    m_is_merged = histo.m_is_merged;
  #endif
  }
  void operator=(std::nullptr_t) {reset(nullptr);}

  operator THist*() {return m_merged;}
  THist * get() {return m_merged;}
  TFile * file() {return m_file;}

  #ifdef MTTHIST_MONO
  THist * Get() {return m_merged;}

  #else // not MTTHIST_MONO :
  void Merge();
  THist * Merged();
  THist * Get(ushort const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[MTObject::getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}

  auto const & Integral() const {return m_integral;}

  #endif //MTTHIST_MONO

  std::string const & GetName() const {return m_str_name;}

  void setComment(std::string const & comment) {m_comment = comment;}
  std::string const & readComment(std::string const & comment) const {return m_comment;}

private:
  std::mutex m_mutex;

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

};

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::reset(std::string name, ARGS &&... args)
{
  if (MTObject::ON)
  {
    m_mutex.lock();
    m_file = gROOT -> GetFile();// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
    m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
    m_str_name = name;


  #ifdef MTTHIST_MONO

    if (!m_merged) m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);

  #else // not MTTHIST_MONO
    // If we are in the master thread, then it means we are 
    if (MTObject::isMasterThread())
    {
      for (uint histo = 0; histo<m_collection.size(); histo++) if (!m_is_deleted[histo]) delete m_collection[histo];
      m_collection.resize(MTObject::getThreadsNb(), nullptr);// Memory leak here !!
      m_is_deleted.resize(MTObject::getThreadsNb(), false);
      for (size_t i = 0; i<m_collection.size(); i++)
      {
        m_collection[i] = new THist ((name+"_"+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
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
    m_mutex.unlock();
  }
  else 
  {// If MTObject::OFF
    if (m_merged) delete (m_merged);
    m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);
  }
}

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::Fill(ARGS &&... args)
{
  m_integral++;
#ifdef MTTHIST_MONO
  m_mutex.lock();
  m_merged -> Fill(std::forward<ARGS>(args)...);
  m_mutex.unlock();

#else // not MTTHIST_MONO :
  m_collection[MTObject::getThreadIndex()]->Fill(std::forward<ARGS>(args)...);

#endif //MTTHIST_MONO
}

  #ifndef MTTHIST_MONO
template<class THist>
void MTTHist<THist>::Merge()
{
  if (!m_exists || !m_collection.size() || m_collection[0] -> IsZombie() || m_collection[0] -> Integral() < 1)
  {
    m_exists = false;
    delete m_merged;
    m_is_merged = false;
  }
  else
  {
    if (MTObject::nb_threads == 1)
    {
      m_merged = m_collection[0];
    }
    else
    {
      m_file = gROOT -> GetFile();
      if (m_file) gROOT -> cd(); //To get out of scope of any potential TFile
      m_merged = static_cast<THist*> (m_collection[0]->Clone(m_str_name.c_str()));
      for (size_t i = 1; i<m_collection.size(); i++) m_merged -> Add(m_collection[i]);
      if (m_file) m_file -> cd(); //To return to the scope of any potential TFile
    }
    m_is_merged = true;
  }
}

template<class THist>
THist* MTTHist<THist>::Merged()
{
  if (!m_is_merged)
  {
    print("No merged histogram yet ! Issues in the code :/");
    return (new THist());
  }
  else return m_merged;
}
  #endif //not MTTHIST_MONO

template<class THist>
void MTTHist<THist>::Write()
{
  if (MTObject::isMasterThread())
  {
  #ifndef MTTHIST_MONO
    if (!m_is_merged) this -> Merge();
  #endif // not MTTHIST_MONO
    if ( !m_exists
      || !m_merged
      || m_merged -> IsZombie()
      || m_merged -> Integral() < 1) return;
      m_merged -> Write(m_str_name.c_str(), TROOT::kOverwrite);
      reset(nullptr); // Not sure this is truly safe...
      m_written = true;
  }
  else
  {// If we are in one histo per thread mode :
    auto const & thread_i = MTObject::MTObject::getThreadIndex();
    if  (   !m_exists
          ||!m_collection[thread_i]
          || m_collection[thread_i] -> IsZombie()
          || m_collection[thread_i] -> Integral() < 1
        ) return;
    else
    {
      m_written = true;
      m_collection[thread_i] -> Write();
      delete m_collection[thread_i];
      m_is_deleted[thread_i] = true;
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

#endif //MTTHIST_MONO
}

template <class THist>
using Vector_MTTHist = std::vector<MTTHist<THist>>;

#endif //MTTHIST_H
