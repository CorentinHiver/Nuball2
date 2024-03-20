#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include "../libRoot.hpp"
#include "Event.hpp"

class Alignator
{
public:
  Alignator(TTree* tree) : m_tree(tree) {loadNewIndex();}

  /// @brief TBD
  Alignator(TTree* inputTree, TTree* outputTree) : 
    m_tree(inputTree), 
    m_out_tree(outputTree) 
  {
    loadNewIndex();
    align();
  }

  auto const & operator[](int const & i) const {return m_index[i];}

  auto const & next() {return m_index[m_cursor++];}
  auto const & cursor() const {return m_cursor;}
  void reset() {m_cursor = 0;}

  auto begin() {return m_index.begin();}
  auto end()   {return m_index.end();}

  auto GetEntry(int i) {return m_tree->GetEntry(m_index[i]);}

  auto operator->() {return m_tree;}

  auto getTree() {return m_tree;}

  #ifdef MULTITHREADING
  std::mutex mutex;
  #endif //MULTITHREADING

private:
  void loadNewIndex();
  void align(); //TBD

  ULong64_t m_nb_data = 0;
  ULong64_t m_cursor = 0;
  std::vector<int> m_index;
  TTree* m_tree;
  TTree* m_out_tree;
};

void Alignator::loadNewIndex()
{
mutex.lock();
  m_nb_data = m_tree -> GetEntries();

  if (m_nb_data==0) {print("NO DATA IN ROOT TREE !"); return;}

  m_index.resize(static_cast<int>(m_nb_data));
  std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data

  auto const nb_hits = m_tree -> GetEntries();
  m_tree -> SetBranchStatus("*", false);// Disables all the branches readability
  m_tree -> SetBranchStatus("stamp", true);// Read only the time

  ULong64_t timestamp = 0; 
  std::vector<ULong64_t> timestamp_buffer(nb_hits, 0);
  m_tree->SetBranchAddress("stamp", &timestamp);
mutex.unlock();

  // First creates a buffer of all the timestamps :
  for (int i = 0; i<nb_hits; i++)
  {
    m_tree -> GetEntry(i);
    timestamp_buffer[i]=timestamp;
  }

mutex.lock();
  // Then computes the correct order :
  int i = 0, j = 0;
  ULong64_t a = 0;
  m_index[0]=0;
mutex.unlock();
 for (j=0; j<nb_hits;j++)
 {
   m_index[j]=j;
   a=timestamp_buffer[j]; //Focus on this time stamp
   i=j;
  // Find the place to insert it amongst the previously sorted timestamps
   while((i > 0) && (timestamp_buffer[m_index[i-1]] > a))
   {
     m_index[i]=m_index[i-1];
     i--;
   }
   m_index[i]=j;
 }
mutex.lock();
  m_tree -> SetBranchStatus("*", true); //enables again the whole tree to be read
mutex.unlock();
}

/**
 * @brief TBD
 * @details L'objectif c'est de faire une méthode pour aligner en temps 
 * un root tree automatiquement. Pour ça par contre, il faut une méthode
 * qui lise automatiquement le contenu du fichier root et connecte l'event correctement
 */
void Alignator::align()
{
  
}

#endif //ALIGNATOR_HPP