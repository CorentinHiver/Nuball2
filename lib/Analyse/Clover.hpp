#ifndef CLOVER_H
#define CLOVER_H

class Clover
{
public:
  Clover(){}

  void Fill(Event const & event, int const & index);
  void Reset();

  Float_t & nrj() {return m_nrj}


private:

  Float_t m_nrj = 0.; // Add-backed energy of Clover Ge
  Float_t m_time = 0.; // Time of the Ge crystal with the more energy of a Clover
  Float_t nrj_BGO = 0.; // Sum energy on the BGOs

  // Bool_t prompt_Ge = false; // Tag if the Clover is in the prompt window
  // Bool_t delayed_Ge = false; // Tag if the Clover is in the prompt window

  Int_t Ge = 0; // The number of Ge crystal that fired in a Germanium
  Int_t BGO = 0; // The number of BGO crystals that fired in a Germanium
  Int_t crystals = 0; // The number of Ge and BGO crystal that fired

  // Int_t maxE_hit = 0; // Position in the event of the Ge crystal with the maximum energy of the Clover
  // Float_t maxE = 0.; // Value of the maximum energy in a Ge crystal of a Clover
};

void Clover::Fill(Event const & event, int const & index)
{
  crystals++;
  if (isBGO[event.labels[index]])
  {
    BGO++;
    nrj_BGO = event.times[index];
  }
  else
  {
    m_time = event.times[index];
    m_nrj += event.nrjs[index];
    Ge++;
  }
}

void Clover::Reset()
{
  m_nrjs = 0.;
  m_times = 0.;
  nrj_BGO = 0.;

  Ge = 0;
  BGO = 0;
  crystals = 0;
}

#endif //CLOVER_H
