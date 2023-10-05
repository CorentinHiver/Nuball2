#ifndef CLOVERS_H
#define CLOVERS_H

#include "../libRoot.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Gate.hpp"

#include "../MTObjects/MTObject.hpp"

#include "CloverModule.hpp"

// class Grouper
// {
//   public:

//   private:
//   float time_ref = 0;
// };

std::array<float, 48> BGO_coeff = 
{
1.1747f, 1.2617f,
1.2617f, 1.2342f,
0.1729f, 0.51f,
1.0199f, 0.9358f,
1.f,     1.f,
1.1181f, 1.1801f,

1.3342f, 1.3590f,
1.3482f, 1.3885f,
1.3102f, 1.f,
1.1967f, 1.1640f,
1.02f,   1.0179f,
1.0471f, 1.0690f,

1.4394f, 1.3663f,
1.0078f, 0.9845f,
1.3204f, 1.0872f,
1.0849f, 1.02f,
1.4811f, 1.3626f,
1.2433f, 1.2283f,

1.1132f, 1.1355f,
1.1157f, 1.1280f,
1.3810f, 1.3002f,
1.f,1.f,
1.f,1.f,
1.f,1.f
};

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
  static inline uchar label_to_clover   (Label const & l) {return static_cast<uchar>((l-23)/6);} // Ranges from 0 to 23
  static        uchar label_to_cristal  (Label const & l);

  static std::array<bool, 1000> is;          // Array used to know if a given detector is a clover
  // static std::array<bool, 1000> isGe;     // Array used to know if a given detector is a Germanium clover
  // static std::array<bool, 1000> isBGO;    // Array used to know if a given detector is a BGO clover
  // static std::array<bool, 1000> blacklist;// Array used to know if a given detector is in the blacklist

  static std::array<uchar, 1000> labels;  // Array used to make correspondance between a detector label and its clover label

  static std::array<uchar, 1000> cristaux_index;     // Array used to make correspondance between the detector label and the Ge  index
  static std::array<uchar, 1000> cristaux_index_BGO; // Array used to make correspondance between the detector label and the BGO index
  // static std::vector <uchar> cristaux_opposite;// Array used to make correspondance between a detector and the label of the detector at 180° 

  /// @brief Static initialize. Allows one to use the arrays event if no object has been instantiated
  static void InitializeArrays()
  {
    if (!s_initialised)
    {
    #ifdef MTOBJECT_HPP
      lock_mutex(MTObject::mutex);
    #endif //MTOBJECT_HPP

      print("Initialising clovers arrays");
      for (Label l = 0; l<1000; l++)
      {
        is[l] = is_clover(l);
        // isGe[l] = is_clover_Ge(l);  // Already exists in libCo
        // isBGO[l] = is_clover_BGO(l);// Already exists in libCo

        labels[l] = (is[l]) ? label_to_clover(l) : -1;
        cristaux_index[l] = (isGe[l]) ? label_to_cristal(l) : -1;
        cristaux_index_BGO[l] = (isBGO[l]) ? label_to_cristal(l) : -1;
      }
      s_initialised = true;
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

  Clovers()
  {
    InitializeArrays(); 
    m_clovers.resize(24); 
    CloverModule::resetLabel();
    PromptClovers.resize(24);
    CloverModule::resetLabel();
    DelayedClovers.resize(24);
    Reset();
  }
  void SetEvent(Event const & event, char const & analyse = 0);
  bool Fill(Event const & event, size_t const & index);
  void Reset();

  //_______________________ //
  // ------ Cristals ------ //

  // In the following, the containers are by default for Germaniums crystals
  // To get the BGO containers, _BGO is added after

  // Ge Crystals :
  StaticVector<uchar> cristaux = StaticVector<uchar>(96);     // List of indexes of Ge crystals in the event
  std::array<float, 96>   cristaux_nrj = {0}; // Array containing the energy of each Ge  cristal
  std::array<double, 96>  cristaux_time = {0};// Array containing the time of each Ge  cristal

  uchar CrystalMult = 0; // Ge crystals counter

  // BGO Crystals :
  StaticVector<uchar> cristaux_BGO = StaticVector<uchar>(48);     // List of indexes of BGO crystals in the event
  std::array<float, 48>   cristaux_nrj_BGO = {0}; // Array containing the energy of each BGO cristal
  std::array<double, 48>  cristaux_time_BGO = {0};// Array containing the absolute time of each BGO cristal

  uchar CrystalMult_BGO = 0; // BGO crystals counter

  //_______________________ //
  // ------ Clovers ------- //
  // In the following :
  // - if nothing is specified, the containers are for both BGO and Germanium hits in a clover
  // -

  // Specific methods :
  CloverModule operator[] (uchar const & i) {return m_clovers[i];}
  CloverModule operator[] (uchar const & i) const {return m_clovers[i];}
  auto begin() const {return m_clovers.begin();}
  auto end()   const {return m_clovers.end  ();}
  auto const & size() const {return Hits.size();}
  auto mult() const {return int_cast(Hits.size());}
  void Analyse();
  void Analyse_more();
  bool FillFast(Event const & event, size_t const & hit_index);
  bool isPrompt (CloverModule const & clover) {return promptGate(clover.time, clover.nrj) ;}
  bool isDelayed(CloverModule const & clover) {return delayedGate(clover.time, clover.nrj);}

  // Positions in the raw event :
  StaticVector<uchar> rawGe  = StaticVector<uchar>(96); //List of the position of the raw Ge  in the event
  StaticVector<uchar> rawBGO = StaticVector<uchar>(48); //List of the position of the raw BGO in the event

  // Hits lists :
  StaticVector<uchar> Hits = StaticVector<uchar>(24);  // List of clovers that fired in the event

  StaticVector<uchar> Ge = StaticVector<uchar>(24);  // List of Germanium modules that fired in the event
  StaticVector<uchar> CleanGe = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules
  StaticVector<uchar> RejectedGe = StaticVector<uchar>(24);  // List of rejected Germaniums that fired in the event, the "garbage"
  StaticVector<uchar> PromptGe = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules
  StaticVector<uchar> DelayedGe = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules


  StaticVector<uchar> Bgo = StaticVector<uchar>(24);  // List of BGO modules that fired in the event
  StaticVector<uchar> BGOonly = StaticVector<uchar>(24);  // List of rejected Germaniums that fired in the event, the "garbage"
  StaticVector<uchar> PromptBGO = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules
  StaticVector<uchar> DelayedBGO = StaticVector<uchar>(24);  // List of clean Germaniums that fired in the event, that is "Ge only" modules
  

  std::vector<CloverModule> m_clovers; // Array containing the 24 clovers

  // Counters :
  uchar TotalMult = 0;
  uchar PromptMult = 0;
  uchar DelayedMult = 0;
  uchar PromptCleanGeMult = 0;
  uchar DelayedCleanGeMult = 0;

  // AnalyseFast :
  std::vector<uchar> delayedClean;
  std::vector<uchar> promptClean;
  std::vector<CloverModule> PromptClovers;
  std::vector<CloverModule> DelayedClovers;
  
  void CleanFast()
  {
    for (auto const & clover_i : promptClean ) PromptClovers [clover_i].clear();
    for (auto const & clover_i : delayedClean) DelayedClovers[clover_i].clear();
    delayedClean.clear();
    promptClean.clear();
    PromptCleanGeMult = 0;
    DelayedCleanGeMult = 0;
  }

private:
  // Parameters :
  static bool s_initialised;

  // -----------------------  Clovers Class  ----------------------- //
  // _______________________________________________________________ //

  // More detailed analysis :
public: 
  bool has511 = false;

  // Calorimetry :
  double totGe = 0.0;
  double totBGO = 0.0;
  double totGe_prompt = -0.1d;
  double totBGO_prompt = -0.1;
  double totGe_delayed = -0.1;
  double totBGO_delayed = -0.1;

  // Timing : 
  class PromptGate
  {
  public:
    PromptGate()
    {// Calculate the intermediate region :
      m_start_coeff = (m_high_E_gate.start-m_low_E_gate.start) / (m_high_E-m_low_E);
      m_start_intercept = m_low_E_gate.start-m_low_E*m_start_coeff;

      m_stop_coeff = (m_high_E_gate.stop-m_low_E_gate.stop) / (m_high_E-m_low_E);
      m_stop_intercept = m_low_E_gate.stop-m_low_E*m_stop_coeff;
    }
    
    /**
     * @brief Is the hit in the gate
     * @details
     * Three zones : high energy, intermediate energy and lower energy.
     * The intermediate energies, links the high and low by a straight line
     * whose parameters ax+b are calculated in the constructor
     * 
     * if in normal mode, simple time gates
     */
    bool operator() (double const & time, float const & energy)
    {
      if (normal)
      {
        return (time>-20 && time<5);
      }
      else 
      {
        auto const & timef = static_cast<float>(time);
            if (energy > m_high_E) return m_high_E_gate.isIn(timef);
        else if (energy > m_low_E)  return (timef > intermediate_start(energy) && timef < intermediate_stop(energy));
        else                        return m_low_E_gate.isIn(timef);
      }
    }

    bool operator() (double const & time) {return (time>-20 && time<5);}

  private:

    float intermediate_start(float const energy) {return (m_start_coeff*energy+m_start_intercept);}
    float intermediate_stop(float const energy) {return (m_stop_coeff*energy+m_stop_intercept );}

    float m_high_E = 200;
    float m_low_E  = 50;
    Gate m_high_E_gate = {-15.d, 7.d };
    Gate m_low_E_gate  = {-20.d, 40.d};
    float m_start_coeff = 0.f;
    float m_start_intercept = 0.f;
    float m_stop_coeff = 0.f;
    float m_stop_intercept = 0.f;
    bool normal = true;
  } promptGate;

  class DelayedGate
  {
  public:
    DelayedGate(){}
    bool operator() (double const & time, float const & energy)
    {
      if (normal)
      {
        return (*this)(time);
      }
      auto const & timef = static_cast<float>(time);
      if (energy > m_low_E) return m_high_E_gate.isIn(timef);
      else                  return m_low_E_gate.isIn(timef);
    }

    bool operator() (double const & time)
    {
           if (time>20  && time<145) return true;
      else if (time>220 && time<345) return true;
      else if (time>420 && time<545) return true;
      else return false;
    }

  private:
    float m_high_E = 100;
    float m_low_E = 100;
    Gate m_high_E_gate = {40.f, 145.f};
    Gate m_low_E_gate  = {60.f, 145.f};
    bool normal = true;
  } delayedGate;

  Gate promptBGOgate = {-20.f, 10.f };
  Gate delayedBGOgate = {60.f, 145.f};
};

// ---- InitializeArrays static members : ----- //
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
bool Clovers::s_initialised = false;

// ---- Methods : --- //

void Clovers::Reset()
{
  for (auto const & index : Hits)
  {
    m_clovers[index].Reset();
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

  rawGe.clear();
  rawBGO.clear();

  Hits.clear();

  Ge.clear();
  CleanGe.clear();
  RejectedGe.clear();
  PromptGe.clear();
  DelayedGe.clear();

  Bgo.clear();
  BGOonly.clear();
  PromptBGO.clear();
  DelayedBGO.clear();

  cristaux.clear();
  cristaux_BGO.clear();

  TotalMult = 0;
  PromptMult = 0;
  DelayedMult = 0;
  PromptCleanGeMult = 0;
  DelayedCleanGeMult = 0;

  CrystalMult = 0;
  CrystalMult_BGO = 0;

  has511 = false;
  totGe = -1.E-12;
  totBGO = -1.E-12;
  totGe_prompt = -1.E-12;
  totBGO_prompt = -1.E-12;
  totGe_delayed = -1.E-12;
  totBGO_delayed = -1.E-12;
}

void Clovers::SetEvent(Event const & event, char const & analyse)
{
  if (analyse == 1 || analyse == 0) 
  {
    this -> Reset();
    for (size_t i = 0; i<event.size(); i++) this -> Fill(event, i);
    if (analyse == 1) this -> Analyse();
  }
  else if (analyse == 2) 
  {
    this->CleanFast();
    for (size_t i = 0; i<event.size(); i++) this -> FillFast(event, i);
    PromptCleanGeMult  = promptClean .size();
    DelayedCleanGeMult = delayedClean.size();
    // this -> AnalyseFast();
  }
}

bool Clovers::FillFast(Event const & event, size_t const & hit_index)
{
  auto const & label = event.labels[hit_index];
  if (!isClover[label]) return false;

  auto const & index_clover = labels[label];
  Hits.push_back_unique(index_clover);

  auto const & nrj = event.nrjs[hit_index];
  auto const & time = event.time2s[hit_index];

  if (isGe[label])
  {
    if (promptGate (time)) 
    {
      promptClean.push_back(index_clover);
      PromptClovers[index_clover].addGe(nrj, time);
    }
    else if (delayedGate(time)) 
    {
      delayedClean.push_back(index_clover);
      DelayedClovers[index_clover].addGe(nrj, time);
    }
  }
  else // if isBGO[label] :
  {
    if (promptGate (time)) PromptClovers[index_clover].addBGO(nrj, time);
    else if (delayedGate(time)) DelayedClovers[index_clover].addBGO(nrj, time);
  }
  return true;
}

/// @brief 
bool Clovers::Fill(Event const & event, size_t const & hit_index)
{
  auto const & label = event.labels[hit_index];

  if (isClover[label])
  {
    auto const & index_clover = labels[label]; // The clovers are indexed between 0 and 23

    auto const & nrj  = event.nrjs[hit_index];
    auto const & time = (event.read.t) ? event.times[hit_index] : event.time2s[hit_index];

    auto & clover = m_clovers[index_clover];

    // Fill the vector containing the list of all the clovers that fired in the event :
    Hits.push_back_unique(index_clover);

    if (isGe[label])
    {
      // -------------------------------------- //
      // --- Individual crystals managing : --- //

      // Ge crystal index (ranges from 0 to 96):
      auto const & index_cristal = cristaux_index[label];

      // RUSTINE A ENLEVER AVEC UNE BONNE CALIB :
      //      if (index_cristal == 3) nrj = nrj*0.855f+5.6f;
      // else if (index_cristal == 27) nrj = nrj*0.03027f-0.32f;
      // else if (index_cristal == 30) nrj = nrj*0.0205f+0.138f;

      // if (nrj>10000) return false; // Set a maximum nrj deposition in a single crystal (can be more in the total clover of course)

      CrystalMult++;

      // Position of the hit in the event :
      rawGe.push_back(hit_index);

      // Fill the vector containing the list of all the Ge crystals that fired :
      // if (label == 26) print(index_cristal);
      cristaux.push_back(index_cristal);

      // Filling the germanium crystals informations :
      cristaux_nrj [index_cristal] = nrj;
      cristaux_time[index_cristal] = time;

      // ------------------------- //
      // --- Clovers managing: --- //

      // Counts the number of Ge crystals that fired in the clover :
      clover.nb++;

      // Find the Ge cristal that received the most energy and use its time for the clover :
      if (nrj >= cristaux_nrj[clover.maxE_Ge_cristal])
      {
        clover.maxE_Ge_cristal = index_cristal;
        clover.time = time;
      }

      // Fill the vector containing the list of Ge clovers that fired :

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
      // cristaux_nrj_BGO [index_cristal] = nrj*BGO_coeff[index_cristal];
      cristaux_nrj_BGO [index_cristal] = nrj;
      cristaux_time_BGO[index_cristal] = time;

      // ------------------------- //
      // --- Clovers managing: ---//

      // Counts the number of BGO crystals that fired in the clover :
      clover.nb_BGO++;

      // Fill the vector containing the list of BGO clovers that fired :

      // Fill the cell containing the total energy deposit in the module's BGOs
      // clover.nrj_BGO += nrj*BGO_coeff[index_cristal];
      clover.nrj_BGO += nrj;

      // Manage the time of the BGOs. To be improved if necessary : if 2 BGOs, only the latest is stored
      clover.time_BGO = time;
    }
   
    return true;
  }
  else return false;
}

/**
 * @brief 
 * @details
 * 
 * About the timing : A module hit is defined as either:
 *  a clean Ge, 
 *  a rejected Ge,
 *  a single BGO
 * 
 * Idée d'amélioration : faire deux conteneurs de clovers, le premier contenant les hits 
 * prompts, et le second les hits delayed. Comme ça on peut faire la réjection Compton 
 * et l'add-back en confinant tous les hits dans deux fenêtres en temps
 * Ou mieux : un module clover contient deux champs, l'un pour le prompt, l'autre pour le delayed
 */
void Clovers::Analyse()
{
  for (auto const & index : Hits)
  {
    auto & clover = m_clovers[index];
    TotalMult++;

    // Analysing Germaniums
    if (clover.nb > 0)
    {
      Ge.push_back(index);
      totGe+=clover.nrj;

      clover.isGePrompt  = promptGate (clover.time);
      clover.isGeDelayed = delayedGate(clover.time);

      if (clover.isGePrompt)
      {
        PromptMult++;
        PromptGe.push_back(index);
        totGe_prompt+=clover.nrj;
      }
      else if (clover.isGeDelayed)
      {
        DelayedMult++;
        DelayedGe.push_back(index);
        totGe_delayed+=clover.nrj;
      }

      if (clover.nb_BGO==0)
      {
        CleanGe.push_back(index);
             if (clover.isGePrompt ) PromptCleanGeMult++;
        else if (clover.isGeDelayed) DelayedCleanGeMult++;
      }
      else RejectedGe.push_back(index);
    }

    // Analysing BGOs
    if (clover.nb_BGO > 0)
    {
      Bgo.push_back(index);
      totBGO+=clover.nrj_BGO;

      clover.isBGOPrompt  = promptGate (clover.time_BGO, clover.nrj_BGO);
      clover.isBGODelayed = delayedGate(clover.time_BGO, clover.nrj_BGO);

      if (clover.isBGOPrompt)
      {
        PromptBGO.push_back(index);
        totBGO_prompt+=clover.nrj_BGO;
      }
      else if (clover.isBGODelayed) 
      {
        DelayedBGO.push_back(index);
        totBGO_delayed+=clover.nrj_BGO;
      }

      // To avoid double counting with the clovers containing both bgo and ge :
      if (clover.nb == 0) 
      {
        if (clover.isBGOPrompt) PromptMult++;
        if (clover.isBGODelayed)DelayedMult++;
        BGOonly.push_back(index);
      }
    }
  }
}

void Clovers::Analyse_more()
{

}

uchar Clovers::label_to_cristal(Label const & l)
{
  auto const cristal = static_cast<uchar>(l-23);
  auto const clover_cristal = static_cast<uchar>(cristal%6);

  // BGO : 
  if (clover_cristal<2) return static_cast<uchar>(clover_cristal+2*(cristal/6));
  
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
        case 0: return static_cast<uchar>(cristal_index+0); // Red
        case 1: return static_cast<uchar>(cristal_index+0); // Green
        case 2: return static_cast<uchar>(cristal_index+1); // Black
        case 3: return static_cast<uchar>(cristal_index-1); // Blue
        default : return -1;
      }

      // -pi/2 rotation : R3A3, R3A4
    case 2: case 3:
      switch(Ge_cristal)
      {
        case 0: return static_cast<uchar>(cristal_index+3); // Red
        case 1: return static_cast<uchar>(cristal_index+1); // Green
        case 2: return static_cast<uchar>(cristal_index-2); // Black
        case 3: return static_cast<uchar>(cristal_index+0); // Blue
        default : return -1;
      }

      // pi rotation : R3A5
    case 4:
      switch(Ge_cristal)
      {
        case 0: return static_cast<uchar>(cristal_index+1); // Red
        case 1: return static_cast<uchar>(cristal_index+2); // Green
        case 2: return static_cast<uchar>(cristal_index+0); // Black
        case 3: return static_cast<uchar>(cristal_index-3); // Blue
        default : return -1;  
      }

      default: // All the correctly turned clovers:
      switch(Ge_cristal)
      {
        case 0: return static_cast<uchar>(cristal_index+2); // Red
        case 1: return static_cast<uchar>(cristal_index-1); // Green
        case 2: return static_cast<uchar>(cristal_index-1); // Black
        case 3: return static_cast<uchar>(cristal_index+0); // Blue
        default : return -1;
      }
    }
  }
  return 255u;
}

std::ostream& operator<<(std::ostream& cout, Clovers const & clovers)
{
  print(clovers.Hits.size(), "clover hits");
  for (auto const & clover_index : clovers.Hits)
  {
    print(clovers[clover_index]);
  }
  return cout;
}

#endif //CLOVERS_H
