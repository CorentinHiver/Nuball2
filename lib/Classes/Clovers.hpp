#ifndef CLOVERS_H
#define CLOVERS_H

#include "Clover.hpp"
#include "Event.hpp"
#include "utils.hpp"


class Clovers
{
public:
  Clovers(){}

  Bool_t Fill(Event const & event, int const & index)
  {
    auto const & label = event.labels[index];
    if (isClover[label])
    {
      // Get the label of the clover from the label of the detector :
      auto const & clover_label = labelToClover_fast[label];

      // Fill the clover :
      m_clovers[clover_label].Fill(event, index);

      // Fill the vector containing the list of all the clovers that fired :
      std::vector<unsigned char>::iterator finder = std::find(std::begin(m_clover_labels), std::end(m_clover_labels), clover_label);
      if (finder == std::end(m_clover_labels)) m_clover_labels.push_back(clover_label);

      // Fill the vector containing the list of all the Ge crystals that fired :
      if (isGe[label])
      {
        std::vector<unsigned char>::iterator finder = std::find(std::begin(m_clover_Ge_labels), std::end(m_clover_Ge_labels), clover_label);
        if (finder == std::end(m_clover_Ge_labels)) m_clover_Ge_labels.push_back(clover_label);
        m_Ge_crystals.push_back(index);
      }
      else
      {
        std::vector<unsigned char>::iterator finder = std::find(std::begin(m_clover_Ge_labels), std::end(m_clover_Ge_labels), clover_label);
        if (finder == std::end(m_clover_Ge_labels)) m_clover_Ge_labels.push_back(clover_label);
        m_clover_BGO_labels.push_back(index);
      }

      return true;
    }
    else return false;
  }

  void Reset()
  {
    for (auto const & hit_label : m_clover_labels)
    {
      m_clovers[hit_label].Reset();
    }
    m_clover_labels.resize(0);
    m_clover_Ge_labels.resize(0);
    m_clover_BGO_labels.resize(0);
    m_Ge_crystals.resize(0);
    m_BGO_crystals.resize(0);
  }

private:

  std::vector<unsigned char> m_clover_labels; // List of labels of the clovers that fired in the event
  std::vector<unsigned char> m_clover_Ge_labels; // List of labels of the clovers that fired in the event
  std::vector<unsigned char> m_clover_BGO_labels; // List of labels of the clovers that fired in the event
  std::vector<unsigned char> m_Ge_crystals; // List of indexes of Ge crystals in the event
  std::vector<unsigned char> m_BGO_crystals; // List of indexes of Ge crystals in the event

  std::array<Clover,24> m_clovers; // The 24 Clovers containing all the data
};

#endif //CLOVERS_H
