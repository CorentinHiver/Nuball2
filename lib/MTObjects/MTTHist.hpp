/*
 * MTTHist:
 * Multithreading wrapper for all THist spectra of root library
 * Author: corentin.hiver@ijclab.in2p3.fr
 *
 * Inspiration :
 * https://root.cern.ch/doc/master/TThreadedObject_8hxx_source.html#l00167
 *
 * MANDATORY : MTObject::nb_threads must be set to the correct number of threads BEFORE initialising;
 *
 * There are two ways of using this histogram :
 *
 * The default one consists in having as many sub-histograms as there are threads.
 * That is, each thread can fill its own histogram.
 * Eventually, before beeing analysed (to be done by hand) or written (automatic) all the histograms are merged into one.
 * The advatage of this method is its speed
 * Two defaults : each sub-histogram takes as many place in the memory
 *                the merging time is a function of the number of threads and the size of the histogram
 * These two defaults makes them really heavy to be used if many bidims (TH2) are beeing used
 *
 * The other way [EXPERIMENTAL !!!] is to use a mutex that protects the histogram in writting
 * The advantage is much less memory taking and no merging.
 * However the mutex activation and the concurrency waiting may slow the process a lot.
 * To be tested !
 * Unlock this way with #define MTTHIST_MONO (maybe find a better name)
 *
 */
#ifndef MTTHIST_H
#define MTTHIST_H
// #define MTTHIST_MONO
#include "MTObject.hpp"

template <class THist>
class MTTHist : public MTObject
{
public:
  template <class... ARGS>
  MTTHist(ARGS &&... args) { this -> reset(std::forward<ARGS>(args)...); }
  ~MTTHist();

  // ---   GENERIC INITIALIZATION --- //

  // General constructor, fully based on reset method
  template <class... ARGS>
  MTTHist(std::string name, ARGS &&... args) { this -> reset (name, args...); }
  //Copy initialiser :
  template <class... ARGS>
  void reset(MTTHist<THist> hist) { m_exists = false; *this = hist;}
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
  //General initialiser to construct any root histogram vector :
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

  Bool_t const & exists() const {return m_exists;}

  operator bool() const & {return m_exists;}
  std::vector<THist*> const & getCollection() const {return m_collection;}
  THist * operator->() {return m_merged;}

  void operator=(MTTHist<THist> histo)
  {
    m_file = histo.file();
    m_exists = histo.exists();
    m_str_name = histo.name();
  #ifdef MTTHIST_MONO
    m_merged = histo.get();
  #else //not MTTHIST_MONO
    m_collection = histo.getCollection();
    m_is_merged = static_cast<bool>(m_merged);
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
  THist * Get(UShort_t const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}

  #endif //MTTHIST_MONO

  std::string const & GetName() const {return m_str_name;}

private:
  TFile* m_file = nullptr;
  Bool_t m_exists = false;
  std::string m_str_name = "";

  THist * m_merged = nullptr;

  #ifndef MTTHIST_MONO
  Bool_t m_is_merged = false;
  std::vector<THist*> m_collection;
  std::vector<bool> m_is_deleted;
  #endif //MTTHIST_MONO

};

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::reset(std::string name, ARGS &&... args)
{
  m_mutex.lock();
  m_file = gROOT -> GetFile();// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
  m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
  m_str_name = name;


  #ifdef MTTHIST_MONO
  if (!m_merged) m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);

  #else // not MTTHIST_MONO
  if (isMasterThread())
  {
    m_collection.resize(MTObject::nb_threads, nullptr);
    m_is_deleted.resize(MTObject::nb_threads, false);
    for (size_t i = 0; i<m_collection.size(); i++)
    {
      m_collection[i] = new THist ((name+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
    }
  }
  else
  {
    if (m_collection.size()<MTObject::getThreadsNb())
    {
      m_collection.resize(MTObject::getThreadsNb());
      m_is_deleted.resize(MTObject::getThreadsNb(), false);
    }
    auto const & i = getThreadIndex();
    m_collection[i] = new THist (name.c_str(), std::forward<ARGS>(args)...);
    m_is_deleted[i] = false;
  }

  #endif //MTTHIST_MONO
  m_mutex.unlock();
}

template <class THist>
template <class... ARGS>
inline void MTTHist<THist>::Fill(ARGS &&... args)
{

  #ifdef MTTHIST_MONO
  m_mutex.lock();
  m_merged -> Fill(std::forward<ARGS>(args)...);
  m_mutex.unlock();

  #else // not MTTHIST_MONO :
  m_collection[getThreadIndex()]->Fill(std::forward<ARGS>(args)...);

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
      reset(nullptr);
  }
  else
  {// If we are in one histo per thread mode :
    auto const & thread_i = MTObject::getThreadIndex();
    if  (   !m_exists
          ||!m_collection[thread_i]
          || m_collection[thread_i] -> IsZombie()
          || m_collection[thread_i] -> Integral() < 1
        ) return;
    else
    {
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
  // print("Destructing",m_str_name);
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
