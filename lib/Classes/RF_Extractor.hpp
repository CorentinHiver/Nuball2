#ifndef RF_EXTRACTOR_HPP
#define RF_EXTRACTOR_HPP

#include <libRoot.hpp>
#include <RF_Manager.hpp>

class RF_Extractor
{
public:
  RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator const & gindex);
  auto const & cursor() const {return m_cursor;}

  operator bool() const & {return m_ok;}

private:
  bool m_ok = false;
  Long64_t m_cursor = 0;
};

RF_Extractor::RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator const & gindex)
{
  auto const & nb_data = tree->GetEntries();
  do {tree -> GetEntry(gindex[m_cursor++]);}
  while(hit.label != RF_Manager::label && m_cursor<nb_data);
  if (m_cursor == nb_data) {print("NO RF DATA FOUND !"); m_ok = false; return;}
  rf.setHit(hit);
  m_ok = true;
}
#endif //RF_EXTRACTOR_HPP