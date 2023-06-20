#ifndef ALIGNATOR_HPP
#define ALIGNATOR_HPP

#include <libRoot.hpp>

class Alignator
{
public:
  Alignator(TTree* tree);
  int const & operator[](int const & i) const {return gindex[i];}

private:
  ULong64_t m_nb_data = 0;
  std::vector<int> gindex;
};

Alignator::Alignator(TTree* tree)
{
  m_nb_data = tree -> GetEntries();

  std::vector<int> gindex(m_nb_data);
  std::iota(std::begin(gindex), std::end(gindex), 0); // Fill with 0,1,2,...,m_nb_data
  alignator(tree, gindex.data()); // defined in libRoot.hpp
}
#endif //ALIGNATOR_HPP