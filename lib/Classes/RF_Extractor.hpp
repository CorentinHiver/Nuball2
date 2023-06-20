#ifndef RF_EXTRACTOR_HPP
#define RF_EXTRACTOR_HPP

#include <libRoot.hpp>
#include <RF_Manager.hpp>

class RF_Extractor
{
public:
  RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator & gindex);
  auto const & cursor() const {return m_cursor;}

private:
  Long64_t m_cursor = 0;
};

RF_Extractor::RF_Extractor(TTree * tree, RF_Manager & rf, Hit & hit, Alignator & gindex)
{
  auto const & nb_data = tree->GetEntries();
  do{ tree -> GetEntry(gindex[m_cursor++]);}
  while(hit.label != RF_Manager::label && m_cursor<nb_data);
  if (m_cursor == nb_data) {print("NO RF DATA FOUND !");}
  rf.setHit(hit);
}
#endif //RF_EXTRACTOR_HPP