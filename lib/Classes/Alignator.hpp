#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include <libRoot.hpp>

class Alignator
{
public:
  Alignator(TTree* tree);
  int const & operator[](int const & i) const {return m_index[i];}

private:
  ULong64_t m_nb_data = 0;
  std::vector<int> m_index;
};

Alignator::Alignator(TTree* tree)
{
  m_nb_data = tree -> GetEntries();

  m_index.resize(static_cast<int>(m_nb_data));
  print (m_index.size());
  std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data
  alignator(tree, m_index.data()); // defined in libRoot.hpp
}
#endif //ALIGNATOR_HPP