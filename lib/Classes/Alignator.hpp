#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include <libRoot.hpp>

class Alignator
{
public:
  Alignator(TTree* tree);
  auto const & operator[](int const & i) const {return m_index[i];}

  auto begin() {return m_index.begin();}
  auto end() {return m_index.end();}

private:
  ULong64_t m_nb_data = 0;
  std::vector<int> m_index;
};

Alignator::Alignator(TTree* tree)
{
  m_nb_data = tree -> GetEntries();

  if (m_nb_data==0) {print("NO DATA IN ROOT TREE !"); return;}

  m_index.resize(static_cast<int>(m_nb_data));
  std::iota(std::begin(m_index), std::end(m_index), 0); // Fill with 0,1,2,...,m_nb_data
  alignator(tree, m_index.data()); // defined in libRoot.hpp
}
#endif //ALIGNATOR_HPP