#ifndef CLOVERMODULE_H
#define CLOVERMODULE_H

/**
 * @brief Describes a Clover module
 * @details
 * Each Clover has its own label, between 0 and (nb_labels-1)
 * In Nuball2, they range from 0 to 23
 * 
 * This simple class manages the number of Ge crystals and BGO crystals that fired in the event,
 * 
 */
class CloverModule
{
public:
  // CloverModule() {}
  CloverModule() : m_label(gLabel) {gLabel++;}

  void reset();
  void clear() {this -> reset();}
  void clean() {this -> reset();}
  auto const & label() const {return m_label;}
  static void resetGlobalLabel() {gLabel = 0;}
  void addGe(float const & _nrj, double const & _time) 
  {
    nrj += _nrj; 
    time = _time; 
    nb++;
  }
  void addBGO(float const & _nrj, double const & _time) 
  {
    nrj_BGO += _nrj; 
    time_BGO = _time; 
    nbBGO++;
  }

  uchar nbCrystals() const {return nb+nbBGO;}

  // In the following, if nothing is specified then it refers to the Germanium

  uchar nb = 0;       // Number of Ge  crystals in the clover
  uchar nbBGO = 0;   // Number of BGO crystals in the clover
  float nrj = 0.;     // Add-backed energy of Ge  Clovers
  float nrj_BGO = 0.; // Add-backed energy of BGO Clovers
  double time = 0.;   // Time of the crystal with most energy deposit of the clover
  double time_BGO = 0.;// Time of the latest BGO

  uchar maxE_Ge_cristal = 0u; // Index of the Ge crystal with the most energy deposit in the clover

private:
  uchar const m_label;
  static thread_local uchar gLabel;
};

uchar thread_local CloverModule::gLabel = 0;

void CloverModule::reset()
{
  nrj = 0.f;
  nrj_BGO = 0.f;
  nb = 0;
  nbBGO = 0;
  time = 0.0;
  time_BGO = 0.0;

  maxE_Ge_cristal = '\0';

}

std::ostream& operator<<(std::ostream& cout, CloverModule const & cloverModule)
{
  print(
    "Clover n°",int_cast(cloverModule.label()), ":",
    "nb Ge", int_cast(cloverModule.nb),
    "nb BGO", int_cast(cloverModule.nbBGO),
    "nrj :", cloverModule.nrj,
    "time: ", cloverModule.time, "ns",
    "index of crystal with max E :", int_cast(cloverModule.maxE_Ge_cristal)
    );
  return cout;
}

#endif //CLOVERMODULE_H
