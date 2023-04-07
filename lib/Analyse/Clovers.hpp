#ifndef CLOVERS_H
#define CLOVERS_H

#include "Clover.hpp"
#include "Event.hpp"
#include "utils.hpp"


class Clovers
{
public:
  Clovers(){}

  static void setArray()
  {

  }

  static uchar setLabelToCrystal(Label const & label)
  {
    (i-1)%6;
    return (label)
  }

  Bool_t Fill(Event const & event, int const & index);

  void Reset()
  {
    for (auto const & hit_label : m_labels)
    {
      m_clovers[hit_label].Reset();
    }
    m_labels.resize(0);
    m_clover_Ge_labels.resize(0);
    m_clover_BGO_labels.resize(0);
    m_Ge_crystals.resize(0);
    m_BGO_crystals.resize(0);
  }

private:

  static std::vector<bool> is; // Array used to know if a given label corresponds

  std::vector<uchar> m_labels; // List of labels of the clovers that fired in the event
  std::vector<uchar> m_clover_Ge_labels; // List of labels of the clovers that fired in the event
  std::vector<uchar> m_clover_BGO_labels; // List of labels of the clovers that fired in the event
  std::vector<uchar> m_Ge_crystals; // List of indexes of Ge crystals in the event
  std::vector<uchar> m_BGO_crystals; // List of indexes of Ge crystals in the event

  std::array<Clover,24> m_clovers; // The 24 Clovers containing all the data
};

Bool_t Clovers::Fill(Event const & event, int const & index)
{
  auto const & label = event.labels[index];
  if (isClover[label])
  {
    // Get the label of the clover from the label of the detector :
    auto const & clover_label = labelToClover_fast[label];

    // Fill the clover :
    m_clovers[clover_label].Fill(event, index);

    // Fill the vector containing the list of all the clovers that fired :
    push_back_unique(m_labels, clover_label);

    // Fill the vector containing the list of all the Ge/BGO crystals that fired :
    push_back_unique( (isGe[label]) ? m_clover_Ge_labels : m_clover_BGO_labels, clover_label);

    return true;
  }
  else return false;
}

#endif //CLOVERS_H
