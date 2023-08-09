#ifndef CLOVERS_H
#define CLOVERS_H

#include "../libRoot.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/Event.hpp"

#include "CloverModule.hpp"

/**
 * @brief Analyse the clovers in the event
 * @details
 * 
 * Typical use case : 
 * 
 *        // Open tree...
 *        Event event(tree, "lTn");
 *        
 *        auto root_histo = new THisto(...);
 *        
 *        auto maxEvt = tree -> GetEntries();
 *        int eventNumber = 0;
 *        Clovers clovers;
 *        while (eventNumber<maxEvt)
 *        {
 *          tree -> GetEntry();
 *          clovers.SetEvent(event);
 *          for (auto const & clover : event.CleanGe)
 *          {
 *            root_histo->Fill(clover.nrj);
 *          }
 *        }
 * 
 */
class Clovers
{
public:
  // ________________________________________________________________ //
  // ------------------  Setting the lookup tables ------------------ //
  // There are different sets of labels and index. In the following :
  //  - the label is the raw detector label as declared in faster and index_1**.dat
  //  - the Ge  crystal index ranges from 0 to 95 and is then used for angles and visualisation purposes
  //  - the BGO crystal index ranges from 0 to 47 and "                                                "

  static inline bool is_clover(Label const & l) {return  (l>22 && l<167);}
  // static inline bool is_clover_Ge  (Label const & l) {return ((is_clover(l)) ? ((l-1)%6 < 4) : false);}
  // static inline bool is_clover_BGO (Label const & l) {return ((is_clover(l)) ? ((l-1)%6 > 3) : false);}

  // Correspondance between detector label and crystal index :
  static inline uchar label_to_clover   (Label const & l) {return ((l-23)/6);} // Ranges from 0 to 23
  static        uchar label_to_cristal  (Label const & l);

  static std::array<bool, 1000> is;          // Array used to know if a given detector is a clover
  // static std::array<bool, 1000> isGe;     // Array used to know if a given detector is a Germanium clover
  // static std::array<bool, 1000> isBGO;    // Array used to know if a given detector is a BGO clover
  // static std::array<bool, 1000> blacklist;// Array used to know if a given detector is in the blacklist

  static std::array<uchar, 1000> labels;  // Array used to make correspondance between a detector label and its clover label

  static std::array<uchar, 1000> cristaux_index;     // Array used to make correspondance between the detector label and the Ge  index
  static std::array<uchar, 1000> cristaux_index_BGO; // Array used to make correspondance between the detector label and the BGO index
  // static std::vector <uchar> cristaux_opposite;// Array used to make correspondance between a detector and the label of the detector opposite 

  /// @brief Static initialize. Allows one to use the arrays event if no object has been instantiated
  static void Initialize()
  {
    if (!sm_isInitialized)
    {
      for (Label l = 0; l<1000; l++)
      {
        is[l] = is_clover(l);
        // isGe[l] = is_clover_Ge(l);
        // isBGO[l] = is_clover_BGO(l);

        labels[l] = (is[l]) ? label_to_clover(l) : -1;
        cristaux_index[l] = (isGe[l]) ? label_to_cristal(l) : -1;
        cristaux_index_BGO[l] = (isBGO[l]) ? label_to_cristal(l) : -1;
      }
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

  Clovers(){Initialize(); m_Clovers.resize(24); Reset();}
  void SetEvent(Event const & event);
  Bool_t Fill(Event const & event, int const & index);
  void Reset();

  //_______________________ //
  // ------ Cristals ------ //

  // In the following, the containers are by default for Germaniums crystals
  // To get the BGO containers, _BGO is added after

  // Ge Crystals :
  StaticVector<uchar> cristaux = StaticVector<uchar>(96);     // List of indexes of Ge crystals in the event
  std::array<float, 96>   cristaux_nrj; // Array containing the energy of each Ge  cristal
  std::array<double, 96>  cristaux_time;// Array containing the time of each Ge  cristal

  std::size_t CrystalMult = 0; // Ge crystals counter

  // BGO Crystals :
  StaticVector<uchar> cristaux_BGO = StaticVector<uchar>(48);     // List of indexes of BGO crystals in the event
  std::array<float, 48>   cristaux_nrj_BGO; // Array containing the energy of each BGO cristal
  std::array<double, 48>  cristaux_time_BGO;// Array containing the absolute time of each BGO cristal

  std::size_t CrystalMult_BGO = 0; // BGO crystals counter

  //_______________________ //
  // ------ Clovers ------- //
  // In the following :
  // - if nothing is specified, the containers are for both BGO and Germanium hits in a clover
  // -

  // Specific methods :
  CloverModule operator[] (uchar const & i) {return m_Clovers[i];}
  auto begin() const {return m_Clovers.begin();}
  auto end()   const {return m_Clovers.end  ();}
  uint size() const {return Hits.size();}
  void Analyse();

  // Containers :
  StaticVector<uchar> Hits = StaticVector<uchar>(24);  // List of clovers that fired in the event
  StaticVector<uchar> Ge = StaticVector<uchar>(24);  // List of Germanium modules that fired in the event
  StaticVector<uchar> Bgo = StaticVector<uchar>(24);  // List of BGO modules that fired in the event
  StaticVector<uchar> Clean_Ge = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules
  
  StaticVector<uchar> rawGe  = StaticVector<uchar>(96); //List of the position of the raw Ge  in the event
  StaticVector<uchar> rawBGO = StaticVector<uchar>(48); //List of the position of the raw BGO in the event

  std::vector<CloverModule> m_Clovers; // Array containing the 24 clovers

  // Counters :
  std::size_t Mult = 0;
  std::size_t GeMult = 0;
  std::size_t CleanGeMult = 0;
  std::size_t BGOMult = 0;

private:
  // Parameters :
  static bool sm_isInitialized;

  // -----------------------  Clovers Class  ----------------------- //
  // _______________________________________________________________ //

  // More detailed analysis :
public: 
  bool has511 = false;
};

// ---- Initialize static members : ----- //
// Lookup tables :
std::array<bool, 1000> Clovers::is;
// std::array<bool, 1000> Clovers::isGe;
// std::array<bool, 1000> Clovers::isBGO;
// std::array<bool, 1000> Clovers::blacklist;
std::array<uchar, 1000> Clovers::labels;
std::array<uchar, 1000> Clovers::cristaux_index;
std::array<uchar, 1000> Clovers::cristaux_index_BGO;
// std::vector <uchar> Clovers::cristaux_opposite;

// Parameters :
float Clovers::Emin = 5.; // by default 5 keV
bool Clovers::sm_isInitialized = false;

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

  rawBGO.resize();
  rawGe.resize();

  Hits.resize();
  Ge.resize();
  Bgo.resize();
  Clean_Ge.resize();

  cristaux.resize();
  cristaux_BGO.resize();

  Mult = 0;
  GeMult = 0;
  CleanGeMult = 0;
  BGOMult = 0;

  CrystalMult = 0;
  CrystalMult_BGO = 0;

  has511 = false;
}

void Clovers::SetEvent(Event const & event)
{
  Reset();
  for (size_t i = 0; i<event.size(); i++) this -> Fill(event, i);
}

/// @brief 
Bool_t Clovers::Fill(Event const & event, int const & hit_index)
{
  auto const & label = event.labels[hit_index];

  if (isClover[label])
  {
    auto const & index_clover = labels[label];

    auto const & nrj  = event.nrjcals[hit_index];
    auto const & time = event.time2s[hit_index];

    auto & clover = m_Clovers[index_clover];

    // Fill the vector containing the list of all the clovers that fired :
    Hits.push_back_unique(index_clover);

    if (isGe[label])
    {
      // -------------------------------------- //
      // --- Individual crystals managing : --- //

      CrystalMult++;

      // Position of the hit in the event :
      rawBGO.push_back(hit_index);

      // Ge crystal index (ranges from 0 to 96):
      auto const & index_cristal = cristaux_index[label];

      // Fill the vector containing the list of all the Ge crystals that fired :
      cristaux.push_back(index_cristal);

      // Filling the germanium crystals informations :
      cristaux_nrj[index_cristal] = nrj;
      cristaux_time[index_cristal] = time;

      // ------------------------- //
      // --- Clovers managing: --- //

      // Counts the number of Ge crystals that fired in the clover :
      clover.nb++;

      // Find the Ge cristal that received the most energy and use its time for the clover :
      if (nrj > cristaux_nrj[clover.maxE_Ge_cristal])
      {
        clover.maxE_Ge_cristal = index_cristal;
        clover.time = time;
      }

      print(clover.time, time);

      // Fill the vector containing the list of Ge clovers that fired :
      Ge.push_back_unique(index_clover);

      // Do a raw add-back : sum the energy of all the crystals in a clover
      clover.nrj += nrj;

      // Detailed analysis :
      if (nrj>507 && nrj<516) has511 = true;
    }

    else
    {// if isBGO[label] :

      // -------------------------------------- //
      // --- Individual crystals managing : --- //

      CrystalMult_BGO++;

      // Position of the hit in the event :
      rawBGO.push_back(hit_index);

      // BGO crystal index (ranges from 0 tp 48):
      auto const & index_cristal = cristaux_index_BGO[label];

      // Fill the vector containing the list of all the BGO crystals that fired :
      cristaux_BGO.push_back(index_cristal);

      // Filling the germanium crystals informations :
      cristaux_nrj_BGO[index_cristal] = nrj;
      cristaux_time_BGO[index_cristal] = time;

      // ------------------------- //
      // --- Clovers managing: ---//

      // Counts the number of BGO crystals that fired in the clover :
      clover.nb_BGO ++;

      // Fill the vector containing the list of BGO clovers that fired :
      Bgo.push_back_unique(index_clover);

      // Fill the cell containing the total energy deposit in the module's BGOs
      clover.nrj_BGO += nrj;

      // Manage the time of the BGOs. To be improved if necessary : if 2 BGOs, only the latest one is stored
      clover.time_BGO = time;
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
  auto const clover_cristal = cristal%6;

  // BGO : 
  if (clover_cristal<2) return clover_cristal+2*(cristal/6);
  
  // Ge :
  else
  {
    auto const clover = cristal/6; // Clover number
    auto const Ge_cristal = clover_cristal-2; // Cristal number inside the clover [0,3]
    auto cristal_index = Ge_cristal+4*clover;

    // For normally rotated clovers, the following disposition is taken : 
    // With sub_index beeing ranging from 0 to 3 for Ge crystal,
    // With beam from right to left, and the gamma rays going out of the sheet :
    // The color green is top right, black top left, red bottom right and blue bottom left for normally rotated clovers
    // The sub_index 0 is top right, 1 top left, 2 bottom right, 3 bottom_left
    // That is, the angle phi increases with the sub_index
    //          the even labels are on the right side
    //          the odd  labels are on the left  side

    switch (clover) 
    { // Rotation is trigowise direction

      // pi/2 rotation : R2A6
    case 17:
      switch(Ge_cristal)
      {
        case 0: return cristal_index+0; // Red
        case 1: return cristal_index+0; // Green
        case 2: return cristal_index+1; // Black
        case 3: return cristal_index-1; // Blue
        default : return -1;
      }

      // -pi/2 rotation : R3A3, R3A4
    case 2: case 3:
      switch(Ge_cristal)
      {
        case 0: return cristal_index+3; // Red
        case 1: return cristal_index+1; // Green
        case 2: return cristal_index-2; // Black
        case 3: return cristal_index+0; // Blue
        default : return -1;
      }

      // pi rotation : R3A5
    case 4:
      switch(Ge_cristal)
      {
        case 0: return cristal_index+1; // Red
        case 1: return cristal_index+2; // Green
        case 2: return cristal_index+0; // Black
        case 3: return cristal_index-3; // Blue
        default : return -1;
      }

      default: // All the correctly turned clovers:
      switch(Ge_cristal)
      {
        case 0: return cristal_index+2; // Red
        case 1: return cristal_index-1; // Green
        case 2: return cristal_index-1; // Black
        case 3: return cristal_index+0; // Blue
        default : return -1;
      }
    }
  }
  return -1;
}

#endif //CLOVERS_H
