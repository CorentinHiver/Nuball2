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
 * Finally, before beeing written all the histograms are merged into one.
 * The advatage of this method is its speed
 * Two defaults : each sub-histogram takes as many place in the memory
 *                the merging time is a function of the number of threads and the size of the histogram
 *
 * The other way [EXPERIMENTAL !!!] is to use a mutex that protects the histogram in writting
 * The advantage is much less memory taking and no merging.
 * However the mutex activation and the concurrency waiting may slow the process a lot.
 * To be tested !
 * Unlock this way with #define MEMORY (maybe find a better name)
 *
 */
#ifndef MTTHIST_H
#define MTTHIST_H
// #define MEMORY
#include "MTObject.hpp"

template <class THist>
class MTTHist : public MTObject
{
public:
  template <class... ARGS>
  MTTHist(ARGS &&... args){this -> reset(std::forward<ARGS>(args)...);}
  ~MTTHist()
  {
    if (!m_file)
    {
      for (auto & histo: m_collection) if (histo) delete histo;
    }
    if (m_merged) delete m_merged;
  }

  // General constructor, fully based on reset method
  template <class... ARGS>
  MTTHist(std::string name, ARGS &&... args){ this -> reset (name, args...); }

  //Default initialiser, to create a zombie
  template <class... ARGS>
  void reset(MTTHist<THist> const & hist) {m_exists = false;}

  //Default initialiser, to create a zombie
  template <class... ARGS>
  void reset() {m_exists = false;}

  //Nullptr initialiser, to create a zombie as default initializer
  template <class... ARGS>
  void reset(std::nullptr_t)
  {
    m_exists = false;
  }

  //General initialiser to construct any root histogram vector
  template <class... ARGS>
  void reset(std::string name, ARGS &&... args)
  {
    local_mutex.lock();
    m_file = gROOT -> GetFile();// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
    m_exists = true;// If SIGSEGV here, have you instantiated the object ? (e.g., in an array of histograms)
    m_str_name = name;
    // m_merged = new THist (name.c_str(), std::forward<ARGS>(args)...);
    m_collection.resize(MTObject::nb_threads, emptyTHist);
    if (isMasterThread())
    {
      for (size_t i = 0; i<m_collection.size(); i++)
      {
        m_collection[i] = new THist ((name+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
      }
    }
    else
    {
      auto const & i = getThreadIndex();
      m_collection[i] = new THist ((name+std::to_string(i)).c_str(), std::forward<ARGS>(args)...);
      print(i,m_collection[i]);
    }
    local_mutex.unlock();
  };

  template <class... ARGS>
  void Fill(ARGS &&... args)
  {
    m_collection[getThreadIndex()]->Fill(std::forward<ARGS>(args)...);
  }

  operator THist*() {return m_merged;}

  void Merge();
  void Write();

  Bool_t const & exists() {return m_exists;};
  operator bool() {return m_exists;};
  THist * Merged();
  THist * Get(UShort_t const & thread_nb) {return m_collection[thread_nb];}
  THist * Get() {return m_collection[getThreadIndex()];}
  THist * operator[](int const & thread_nb) {return m_collection[thread_nb];}
  THist * operator->() {return m_merged;}

  void operator=(MTTHist<THist> const & histo) {}

private:
  TFile* m_file = nullptr;
  Bool_t m_exists = false;
  std::string m_str_name = "";
  Bool_t m_is_merged = false;

  std::vector<THist*> m_collection;
  THist* emptyTHist = nullptr;
  THist* m_merged = nullptr;
};

template<class THist>
void MTTHist<THist>::Merge()
{
  if (!m_exists || !m_collection.size() || m_collection[0] -> IsZombie() || m_collection[0] -> Integral() < 1)
  {
    m_exists = false;
    m_merged = new THist();
  }
  else
  {
    if (MTObject::nb_threads == 1)
    {
      m_merged = m_collection[0];
    }
    else
    {
      auto file = gROOT -> GetFile();
      if (file) gROOT -> cd(); //To get out of scope of any potential TFile
      m_merged = (THist*) (m_collection[0]->Clone(m_str_name.c_str()));
      for (unsigned int i = 1; i<m_collection.size(); i++)
      {
        m_merged -> Add(m_collection[i]);
      }
      if (file) file -> cd(); //To return to the scope of any potential TFile
    }
    m_is_merged = true;
  }
}

template<class THist>
void MTTHist<THist>::Write()
{
  if (!m_is_merged) this -> Merge();
  if (m_exists == false
    || !m_merged
    || m_merged -> IsZombie()
    || m_merged -> Integral() < 1) return;
  m_merged -> Write(m_str_name.c_str(), TROOT::kOverwrite);
  reset(nullptr);
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

template <class THist>
using Vector_MTTHist = std::vector<MTTHist<THist>>;

#endif //MTTHIST_H
