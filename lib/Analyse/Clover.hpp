#ifndef CLOVER_H
#define CLOVER_H

class CLOVER
{
public:
  // CLOVER() {}
  CLOVER() : m_label(glabel) {glabel++;}

  void Reset();
  auto const & label() const {return m_label;}

  // In the following, if nothing is specified then it refers to the Germanium

  float nrj = 0.;  // Add-backed energy of Ge  Clovers
  float nrj_BGO = 0.; // Add-backed energy of BGO Clovers
  float nb = 0.;   // Number of Ge  crystals in each clover
  float nb_BGO = 0.;  // Number of BGO crystals in each clover
  float time = 0.; // Time of the crystal with most energy deposit of the clover
  // float time_BGO = 0.;

  uchar maxE_Ge_cristal = 0u; // Crystal number of the crystal with most energy deposit of the clover


private:
  uchar const m_label;
  static uchar glabel;
};

uchar CLOVER::glabel = 0;

void CLOVER::Reset()
{
  nrj = 0.;
  nrj_BGO = 0.;
  nb = 0.;
  nb_BGO = 0.;
  time = 0.;
  // time_BGO = 0.;
  maxE_Ge_cristal = 0u;
}

#endif //CLOVER_H
