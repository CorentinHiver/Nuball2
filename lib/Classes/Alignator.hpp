#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include "../libRoot.hpp"
#include "Event.hpp"

class Alignator
{
public:
  Alignator(TTree* tree) : m_tree(tree) {loadNewIndex();}

  /// @brief DEV
  Alignator(TTree* inputTree, TTree* outputTree) : 
    m_tree(inputTree), 
    m_out_tree(outputTree) 
  {align();}

  auto const & operator[](int const & i) const {return m_index[i];}

  auto const & next() {return m_index[m_cursor++];}
  auto const & cursor() const {return m_cursor;}
  void reset() {m_cursor = 0;}

  auto begin() {return m_index.begin();}
  auto end()   {return m_index.end();}

private:
  void loadNewIndex();
  void align();

  ULong64_t m_nb_data = 0;
  ULong64_t m_cursor = 0;
  std::vector<int> m_index;
  TTree* m_tree;
  TTree* m_out_tree;
};

void Alignator::loadNewIndex()
{
  m_nb_data = m_tree -> GetEntries();

  if (m_nb_data==0) {print("NO DATA IN ROOT TREE !"); return;}

  m_index.resize(static_cast<int>(m_nb_data));
  std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data

  auto const NHits = m_tree -> GetEntries();
  m_tree -> SetBranchStatus("*", false);// Disables all the branches readability
  m_tree -> SetBranchStatus("stamp", true);// Read only the time

  std::vector<ULong64_t> TimeStampBuffer(NHits, 0);
  ULong64_t TimeStamp = 0; 
  m_tree->SetBranchAddress("stamp", &TimeStamp);

  // First creates a buffer of all the timestamps :
  for (int i = 0; i<NHits; i++)
  {
    m_tree -> GetEntry(i);
    TimeStampBuffer[i]=TimeStamp;
  }

  // Then computes the correct order :
  int i = 0, j = 0;
  ULong64_t a = 0;
  m_index[0]=0;
 for (j=0; j<NHits;j++)
 {
   m_index[j]=j;
   a=TimeStampBuffer[j]; //Focus on this time stamp
   i=j;
  // Find the place to insert it amongst the previously sorted timestamps
   while((i > 0) && (TimeStampBuffer[m_index[i-1]] > a))
   {
     m_index[i]=m_index[i-1];
     i--;
   }
   m_index[i]=j;
 }
  m_tree -> SetBranchStatus("*", true); //enables again the whole tree to be read
}

/**
 * @brief TBD
 * @details L'objectif c'est de faire une méthode pour aligner en temps 
 * un root tree automatiquement. Pour ça par contre, il faut une méthode
 * qui lise automatiquement le contenu du fichier root et connecte l'event correctement
 */
void Alignator::align()
{
  // m_nb_data = m_tree -> GetEntries();

  // if (m_nb_data==0) {print("NO DATA IN ROOT TREE !"); return;}

  // m_index.resize(static_cast<int>(m_nb_data));
  // std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data

  // Event event

  // auto const NHits = tree -> GetEntries();
  // std::vector<ULong64_t> TimeStampBuffer(NHits, 0);
  // ULong64_t TimeStamp = 0; 
  // tree->SetBranchAddress("time", &TimeStamp);

  // // First creates a buffer of all the timestamps :
  // for (int i = 0; i<NHits; i++)
  // {
  //   tree -> GetEntry(i);
  //   TimeStampBuffer[i]=TimeStamp;
  // }

  // // Then computes the correct order :
  // int i = 0, j = 0;
  // ULong64_t a = 0;
  // m_index[0]=0;
  // for (j=0; j<NHits;j++)
  // {
  //   m_index[j]=j;
  //   a=TimeStampBuffer[j]; //Focus on this time stamp
  //   i=j;
  //   // Find the place to insert it amongst the previously sorted timestamps
  //   while((i > 0) && (TimeStampBuffer[m_index[i-1]] > a))
  //   {
  //     m_index[i]=m_index[i-1];
  //     i--;
  //   }
  //   m_index[i]=j;
  // }
  // tree -> SetBranchStatus("*", true); //enables again the whole tree to be read
}

class AlignedTree : public TTree, public Alignator
{

};

#endif //ALIGNATOR_HPP