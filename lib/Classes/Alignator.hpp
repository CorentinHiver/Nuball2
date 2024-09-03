#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include "../libRoot.hpp"
#include "Event.hpp"
#include "Timer.hpp"

class Alignator
{
public:
  Alignator(TTree* tree, bool align = true) : m_tree(tree) {alignIndexes(align);}

  /// @brief TBD
  // Alignator(TTree* inputTree, TTree* outputTree) : 
  //   m_tree(inputTree), 
  //   m_out_tree(outputTree) 
  // {
  //   alignIndexes();
  //   align();
  // }

  auto const & operator[](int const & i) const {return m_index[i];}

  auto const & cursor() const {return m_cursor;}
  void reset() {m_cursor = 0;}

  auto begin() {return m_index.begin();}
  auto end()   {return m_index.end();}

  auto GetEntry(int i) {return m_tree->GetEntry(m_index[i]);}
  auto Read() 
  {
    if(m_cursor<m_nb_data) 
    {
      m_tree->GetEntry(m_index[m_cursor++]);
      return true;
    }
    else return false; 
  }

  auto operator->() {return m_tree;}

  auto get() {return m_tree;}

  static void useStdSort() 
  {
    m_use_std = true;
    #if ! (Cpp17 && __cpp_lib_execution)
      error("can't use parallel std::sort in Cpp version < 17, you will experience less efficient tree timing sorting");
    #endif //!Cpp17
  }

  void check()
  {
    this -> reset();

    Event event(m_tree, "t");

    m_tree -> SetBranchStatus("*", false);// Disables all the branches readability
    m_tree -> SetBranchStatus("stamp", true);// Read only the timestamp

    this -> Read();

    int nb = 0;

    Timestamp last_stamp = event.stamp;
    while(this->Read())
    {
      if (event.stamp < last_stamp) 
      {
        ++nb;
        print(last_stamp/1000., event.stamp/1000.);
      }
    }

    m_tree -> SetBranchStatus("*", true);
    this -> reset();
    print(nb, "wrong order");
  }

#ifdef MULTITHREADING
  std::mutex mutex;
#endif //MULTITHREADING

operator TTree*() {return m_tree;}

private:
  void alignIndexes(bool align = true);
  void align(); //TBD

  ULong64_t m_nb_data = 0;
  ULong64_t m_cursor = 0;
  std::vector<int> m_index;
  TTree* m_tree = nullptr;
  // TTree* m_out_tree; // TBD

  static bool m_use_std;
};
bool Alignator::m_use_std = false;

void Alignator::alignIndexes(bool align)
{
#ifdef MULTITHREADING
  mutex.lock();
#endif //MULTITHREADING
  m_nb_data = m_tree -> GetEntries();

  if (m_nb_data==0) {print("NO DATA IN ROOT TREE !"); return;}

  m_index.resize(static_cast<int>(m_nb_data));
  std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data

  if (!align) return;

  Event event(m_tree, "t");

  auto const & nb_hits = m_tree -> GetEntries();
  m_tree -> SetBranchStatus("*", false);// Disables all the branches readability
  m_tree -> SetBranchStatus("stamp", true);// Read only the timestamp

  std::vector<Timestamp> timestamp_buffer(nb_hits, 0);

#ifdef MULTITHREADING
  mutex.unlock();
#endif //MULTITHREADING

  // First creates a buffer of all the timestamps :
  for (int i = 0; i<nb_hits; i++)
  {
    m_tree -> GetEntry(i);
    timestamp_buffer[i]=event.stamp;
  }

// Then computes the correct order :

  if (m_use_std)
  {
    std::sort(
  #if Cpp17 && __cpp_lib_execution
      std::execution::par, 
    #endif // Cpp17
      m_index.begin(), m_index.end(), [&timestamp_buffer](int const a, int const & b){
      return timestamp_buffer[a] < timestamp_buffer[b];
    });
  }
  else
  {
    int i = 0, j = 0;
    Timestamp a = 0;
    m_index[0]=0;
    for (j=0; j<nb_hits;j++)
    {
      m_index[j]=j;
      a=timestamp_buffer[j]; //Focus on this time stamp
      i=j;
      // Find the place to insert it amongst the previously sorted timestamps
      while((i > 0) && (timestamp_buffer[m_index[i-1]] > a))
      {
        m_index[i]=m_index[i-1];
        --i;
      }
      m_index[i]=j;
    }
  }

#ifdef MULTITHREADING
  mutex.lock();
#endif //MULTITHREADING

  m_tree -> SetBranchStatus("*", true); //enables again the whole tree to be read
  
#ifdef MULTITHREADING
  mutex.unlock();
#endif //MULTITHREADING
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