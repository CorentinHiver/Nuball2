#ifndef CLOVERS_H
#define CLOVERS_H

#include <libCo.hpp>
#include <libRoot.hpp>
#include <Event.hpp>
#include <CloverModule.hpp>

class Clovers
{
public:
  // ________________________________________________________________ //
  // ------------------  Setting the lookup tables ------------------ //
  // There are different sets of labels and index. In the following :
  //  - the label is the raw detector label as declared in faster and index_1**.dat
  //  - the Ge crystal index ranges from 0 to 95 and is then used for angles and visualisation purposes
  //  - the BGO crystal index is the same for BGO - NOT USED (so far)

  // Is the detector a clover ? And if yes, is it a germanium or a BGO ?
  static inline bool is_clover     (Label const & l) {return  (l>22 && l<167)                      ;}
  static inline bool is_clover_Ge  (Label const & l) {return ((is_clover(l)) ? ((l-1)%6 < 4) : false);}
  static inline bool is_clover_BGO (Label const & l) {return ((is_clover(l)) ? ((l-1)%6 > 3) : false);}

  // Correspondance between detector label and crystal index :
  static inline uchar label_to_clover   (Label const & l) {return ((l-23)/6);}
  static        uchar label_to_cristal  (Label const & l);

  static std::array<bool, 1000> is;       // Array used to know if a given detector is a clover
  static std::array<bool, 1000> isGe;     // Array used to know if a given detector is a Germanium clover
  static std::array<bool, 1000> isBGO;    // Array used to know if a given detector is a BGO clover
  static std::array<bool, 1000> blacklist;// Array used to know if a given detector is in the blacklist

  static std::array<uchar, 1000> labels;  // Array used to make correspondance between a detector label and its clover label

  static std::array<uchar, 1000> cristaux_index;     // Array used to make correspondance between the detector label and the Ge  index
  static std::array<uchar, 1000> cristaux_index_BGO; // Array used to make correspondance between the detector label and the BGO index
  // static std::vector <uchar> cristaux_opposite;// Array used to make correspondance between a detector label and its cristal label

  static void Initialize()
  {
    for (Label l = 0; l<1000; l++)
    {
      is[l] = is_clover(l);
      isGe[l] = is_clover_Ge(l);
      isBGO[l] = is_clover_BGO(l);

      labels[l] = (is[l]) ? label_to_clover(l) : -1;
      cristaux_index[l] = (isGe[l]) ? label_to_cristal(l) : -1;
      cristaux_index_BGO[l] = (isBGO[l]) ? label_to_cristal(l) : -1;
    }
  }
  // ----------------------  End lookup tables ---------------------- //
  // ________________________________________________________________ //


  // _______________________________________________________________ //
  // -----------------------  Set parameters ----------------------- //
  static float Emin;
  static void setEnergyThreshold(float const & emin) {Emin=emin;}
  // -----------------------  End parameters ----------------------- //
  // _______________________________________________________________ //

  // _______________________________________________________________ //
  // -----------------------  Clovers Class  ----------------------- //

  Clovers(){m_Clovers.resize(24); Reset();}
  void Set(Event const & event);
  Bool_t Fill(Event const & event, int const & index);
  void Reset();

  //_______________________ //
  // ------ Cristals ------ //

  // In the following, the containers are by default for Germaniums crystals
  // To get the BGO containers, _BGO is added after

  // Ge Crystals :
  StaticVector<uchar, 96> cristaux;     // List of indexes of Ge crystals in the event
  std::array<float, 96>   cristaux_nrj; // Array containing the energy of each Ge  cristal
  std::array<float, 96>   cristaux_time;// Array containing the time of each Ge  cristal

  std::size_t CrystalMult = 0; // Ge crystals counter

  // BGO Crystals :
  StaticVector<uchar, 48> cristaux_BGO;     // List of indexes of BGO crystals in the event
  std::array<float, 48>   cristaux_nrj_BGO; // Array containing the energy of each BGO cristal
  std::array<float, 48>   cristaux_time_BGO;// Array containing the time of each BGO cristal

  std::size_t CrystalMult_BGO = 0; // BGO crystals counter

  //_______________________ //
  // ------ Clovers ------- //
  // In the following :
  // - if nothing is specified, the containers are for both BGO and Germanium hits in a clover
  // -

  // Specific methods :
  CloverModule operator[] (uchar const & i) {return m_Clovers[i];}
  void Analyse();

  // Containers :
  StaticVector<uchar, 24> Hits;  // List of clovers that fired in the event
  StaticVector<uchar, 24> Ge;  // List of Germanium modules that fired in the event
  StaticVector<uchar, 24> Bgo;  // List of BGO modules that fired in the event
  StaticVector<uchar, 24> Clean_Ge;  // List of clean Germaniums that fired in the event, that is "Ge only" modules

  std::vector<CloverModule> m_Clovers;

  // Counters :
  std::size_t Mult = 0;
  std::size_t GeMult = 0;
  std::size_t CleanGeMult = 0;
  std::size_t BGOMult = 0;

  // -----------------------  Clovers Class  ----------------------- //
  // _______________________________________________________________ //

  // More detailed analysis :
  bool has511 = false;
};

// ---- Initialize static members : ----- //
// Lookup tables :
std::array<bool, 1000> Clovers::is;
std::array<bool, 1000> Clovers::isGe;
std::array<bool, 1000> Clovers::isBGO;
std::array<bool, 1000> Clovers::blacklist;
std::array<uchar, 1000> Clovers::labels;
std::array<uchar, 1000> Clovers::cristaux_index;
std::array<uchar, 1000> Clovers::cristaux_index_BGO;
// std::vector <uchar> Clovers::cristaux_opposite;

// Parameters :
float Clovers::Emin = 5.;

// ---- Methods : --- //

void Clovers::Reset()
{
  for (auto const & index : Hits)
  {
    m_Clovers[index].Reset();
  }

  for (auto const & index : cristaux)
  {
    cristaux_nrj  [index] = 0.;
    cristaux_time [index] = 0.;
  }

  for (auto const & index : cristaux_BGO)
  {
    cristaux_nrj_BGO  [index] = 0.;
    cristaux_time_BGO [index] = 0.;
  }

  Hits.resize(0);
  Ge.resize(0);
  Bgo.resize(0);
  Clean_Ge.resize(0);

  cristaux.resize(0);
  cristaux_BGO.resize(0);

  Mult = 0;
  GeMult = 0;
  CleanGeMult = 0;
  BGOMult = 0;

  CrystalMult = 0;
  CrystalMult_BGO = 0;

  has511 = false;
}

void Clovers::Set(Event const & event)
{
  Reset();
  for (size_t i = 0; i<event.size(); i++) this -> Fill(event, i);
}

Bool_t Clovers::Fill(Event const & event, int const & hit_i)
{
  auto const & label = event.labels[hit_i];
  if (isClover[label])
  {
    auto const & index_clover  =  labels[label];

    auto const & nrj  = event.nrjs [hit_i];
    auto const & time = event.time2s[hit_i];

    auto & clover = m_Clovers[index_clover];

    // Fill the vector containing the list of all the clovers that fired :
    Hits.push_back_unique(index_clover);

    if (isGe[label])
    {
      CrystalMult++;
      // Ge crystal index :
      auto const & index_cristal = cristaux_index[label];
      // Fill the vector containing the list of all the Ge crystals that fired :
      cristaux.push_back(index_cristal);
      // Filling the germanium crystals informations :
      cristaux_nrj[index_cristal] = nrj;
      cristaux_time[index_cristal] = time;
      // Counts the number of Ge crystals that fired in the clover :
      clover.nb ++;
      // Finds the Ge cristal that received the most energy and use its time for the index_clover :
      if (nrj > cristaux_nrj[clover.maxE_Ge_cristal])
      {
        clover.maxE_Ge_cristal = index_cristal;
        clover.time = time;
      }
      // Fill the vector containing the list of Ge clovers that fired :
      Ge.push_back_unique(index_clover);
      // Do a raw add-back : sum the energy of all the crystals in a clover
      clover.nrj += event.nrjs[hit_i];

      // Detailed analysis :
      if (nrj>507 && nrj<516) has511 = true;
    }

    else
    {
      // Cristals managing :
      CrystalMult_BGO++;
      // BGO crystal index :
      auto const & index_cristal = cristaux_index_BGO[label];
      // Fill the vector containing the list of all the BGO crystals that fired :
      cristaux_BGO.push_back(index_cristal);
      // Counts the number of BGO crystals that fired in the clover :
      clover.nb_BGO ++;

      // Clovers managing :
      // Fill the vector containing the list of BGO clovers that fired :
      Bgo.push_back_unique(index_clover);
      // Fill the cell containing the total energy deposit in the module's BGOs
      clover.nrj_BGO += event.nrjs[hit_i];
    }
    return true;
  }
  else return false;
}

void Clovers::Analyse()
{
  for (auto const & index : Hits)
  {
    auto & clover = m_Clovers[index];
    Mult++;
    if (clover.nb > 0)
    {
      GeMult++;
      if (clover.nb_BGO==0)
      {
        CleanGeMult++;
        Clean_Ge.push_back(index);
      }
    }
    else
    {
      BGOMult++;
    }
  }
}

uchar Clovers::label_to_cristal(Label const & l)
{
  auto const cristal = l-23;
  auto const clover_cristal = cristal%6; // 0 and 1 are BGOs, 2 to 5 are
  if (clover_cristal<2) return clover_cristal+cristal/6;
  else
  {
    auto const clover = cristal/6;
    switch (clover) // clover number, to take into account the rotated ones :
    { // Rotation is trigowise direction
      // pi rotation
    case 17:
      switch(clover_cristal-2)
      {
        case 0: return clover;   // Red
        case 1: return clover+1; // Green
        case 2: return clover+3; // Black
        case 3: return clover+2; // Blue
        default : return -1;
      }

      // -pi rotation : R3A3, R3A4
    case 2: case 3:
      switch(clover_cristal-2)
      {
        case 0: return clover+3; // Red
        case 1: return clover+2; // Green
        case 2: return clover;   // Black
        case 3: return clover+1; // Blue
        default : return -1;
      }

      // 2pi rotation
    case 4:
      switch(clover_cristal-2)
      {
        case 0: return clover+1; // Red
        case 1: return clover+3; // Green
        case 2: return clover+2; // Black
        case 3: return clover+0; // Blue
        default : return -1;
      }

      default: // All the correctly turned clovers:
      switch(clover_cristal-2)
      {
        case 0: return clover+2; // Red
        case 1: return clover;   // Green
        case 2: return clover+1; // Black
        case 3: return clover+3; // Blue
        default : return -1;
      }
    }
  }
  return -1;
}

#endif //CLOVERS_H
