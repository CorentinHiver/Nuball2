#ifndef SORTED_EVENT_H
#define SORTED_EVENT_H

#include <Event.hpp>
#include <Timewalk.hpp>
#include <Clovers.hpp>
#include <DSSD.hpp>
#include <RF_Manager.hpp>

struct LaBr3_Hit
{
  uchar label   = 0;
  Float_t       nrjcal  = 0;
  ULong64_t     time    = 0;
};
using LaBr3_Event  = std::vector < LaBr3_Hit >;

class Sorted_Event
{
public:
  Sorted_Event(){initialise(); reset();}
  Sorted_Event(Event * event){initialise(); reset(); setEvent(event);}
  static void Initialise() {Clovers::Initialise();}
  void initialise();
  void reset();

  void setEvent (Event * event) {m_event = event;}
  void setRF(RF_Manager *rf) {m_rf = rf;}
  void fillEvent(Event & event);
  void addGeGateTrig(Float_t const & minE, Float_t const & maxE)
  {
    m_Ge_gates.push_back(std::make_pair(minE,maxE)); m_gateON = true;
  }

  void addGeGateTrig(std::pair<Float_t,Float_t> const & Egate)
  {
    m_Ge_gates.push_back(Egate); m_gateON = true;
  }

  static void setTWcorrectionsDSSD(std::string const & filename);
  static void setMaxTime(Float_t const & max_time);
  static void setDSSDVeto(Float_t const & min_time, Float_t const & max_time, Float_t const & max_E)
  {
    m_max_time_dssd = max_time;
    m_min_time_dssd = min_time;
    m_max_E_dssd = max_E;
  }
  static float m_max_time_dssd;
  static float m_min_time_dssd;
  static float m_max_E_dssd;
  bool dssd_veto = false;


  void sortEvent(Event const & event);
  void sortEvent(){sortEvent(*m_event);}
  bool sortGeClover(Event const & event, int const & i);

  int const & gateGe() const {return m_Ge_gate;}
  auto const & getGatedClover() const {return m_CloverTrig;}


  std::array<Float_t, 2000> time2s;
  RF_Manager *m_rf = nullptr;

  //Clover arrays

  std::vector<uchar> clover_hits; // List of the clovers labels that fired (between 0 and 24)
  std::array < Float_t, 24 > nrj_clover; // Add-backed energy of Clover Ge
  std::array < Float_t, 24 > time_clover; // Time of the Ge crystal with the more energy of a Clover
  // std::array < Float_t, 24 > nrj_BGO; // unused
  std::array < Bool_t,  24 > prompt_Ge; // Tag if the Clover is in the prompt window
  std::array < Bool_t,  24 > delayed_Ge; // Tag if the Clover is in the delayed window
  std::array < Int_t,   24 > Ge; // The number of Ge crystal that fired in a Germanium
  std::array < Int_t,   24 > BGO; // The number of BGO crystals that fired in a Germanium
  std::array < Float_t, 24 > maxE; // Value of the maximum energy in a Ge crystal of a Clover
  std::array < Int_t,   24 > maxE_hit; // Position in the event of the Ge crystal with the maximum energy of the Clover

  // Paris arrays :
  std::vector<uchar> paris_hits; // Position in the event of the paris hits

  // DSSD array :
  std::vector<uchar> DSSD_hits; // Position in the event of the DSSD hits
  std::vector<char>  DSSD_is_Ring; // Tag if the DSSD hit is a ring
  std::vector<char>  DSSD_is_Prompt; // Tag if the DSSD hit is prompt
  std::vector<uchar> DSSD_Rings; // Position in the event of the DSSD rings hits
  std::vector<uchar> DSSD_Sectors; // Position in the event of the DSSD sectors hits

  //Events :
  LaBr3_Event LaBr3_event; // Position in the event of the LaBr3 hits

  //Counters :
  size_t RawMult=0; // Number of hits

  size_t LaBr3Mult=0; // Number of LaBr3 hits
  size_t ParisMult=0; // Number of Paris hits
  size_t ParisLaBr3Mult=0; // Number of Paris hits if the LaBr3 fired

  size_t RawGeMult=0; // Number of Ge crystals hits
  size_t CloverGeMult=0; // Number of Ge Clovers hits (after add-back)
  size_t PromptMult=0; // Number of Ge Clovers hits (after add-back)
  size_t DelayedMult=0; // Number of Ge Clovers hits (after add-back)
  size_t PromptGeMult=0; // Number of Ge Clovers hits (after add-back)
  size_t DelayedGeMult=0; // Number of Ge Clovers hits (after add-back)
  size_t CloverMult=0; // Number of Clovers hits (includes the BGOs)
  size_t CleanGeMult=0; // Number of Clean Clovers hits (after add-back and compton rejection)
  size_t ComptonVetoMult=0; // Number of rejected Clovers

  size_t BGOMult=0; // Number of BGOs hits
  size_t ModulesMult=0; // Number of Modules hits : one module is Clover, LaBr3 or Paris

  size_t DSSDMult=0; // Number of DSSD hits
  size_t DSSDSectorMult=0; // Number of DSSD Sectors hits
  size_t DSSDRingMult=0; // Number of DSSD Rings hits

  size_t ModulePromptMult;
  size_t ModuleDelayedMult;

  //Pre-analysis
#ifdef N_SI_129
  Bool_t DSSDFirstBlob=false;
  Bool_t DSSDSecondBlob=false;
  Bool_t DSSDThirdBlob=false;
  Bool_t DSSDTail=false;
#endif //N_SI_129
  Bool_t DSSDPrompt=false;

private:
  Event * m_event;

  // --- Gating --- //
  std::vector<std::pair<Float_t,Float_t>> m_Ge_gates;
  bool m_gateON = false; // 0 : no gate, n : n-th gate
  int m_Ge_gate = 0; // 0 : no gate, n : n-th gate
  size_t m_CloverTrig = 0.;
  static bool m_TW_DSSD ;
  static Timewalks m_Timewalks_DSSD;

  //Parameters :
  static Float_t m_max_time;
};

Float_t Sorted_Event::m_max_time = 9999999.; // Dumb default value
bool Sorted_Event::m_TW_DSSD = false;
Timewalks Sorted_Event::m_Timewalks_DSSD;
float Sorted_Event::m_max_time_dssd = -100;
float Sorted_Event::m_min_time_dssd = 400;
float Sorted_Event::m_max_E_dssd = 1.E+12;

void Sorted_Event::setTWcorrectionsDSSD(std::string const & filename)
{
  m_Timewalks_DSSD.resize(56);
  m_Timewalks_DSSD.loadFile(filename);
  m_TW_DSSD = true;
}

void Sorted_Event::initialise()
{
  nrj_clover.fill(0);
  time_clover.fill(0);
  prompt_Ge.fill(0);
  delayed_Ge.fill(0);
  Ge.fill(0);
  BGO.fill(0);
  maxE.fill(0);
  maxE_hit.fill(0);
}

void Sorted_Event::fillEvent(Event & event)
{
  event.clear();
  event.mult = RawMult;

}

void Sorted_Event::reset()
{ // Reset all the data and counters

  // In order to be efficient, only the clovers that have data are reset
  // and clover_hits is the list of the clovers with data
  for (auto const & i : clover_hits)
  {
    nrj_clover[i] = 0.;
    time_clover[i] = 0.;
    prompt_Ge[i] = 0;
    delayed_Ge[i] = 0;
    Ge[i] = 0;
    maxE[i] = 0;
    maxE_hit[i] = 0.;
  }
  clover_hits.resize(0);
  BGO.fill(0);

  paris_hits.resize(0);

  DSSD_hits.resize(0);
  DSSD_is_Prompt.resize(0);
  DSSD_is_Ring.resize(0);
  DSSD_Rings.resize(0);
  DSSD_Sectors.resize(0);
  dssd_veto = false;

  LaBr3_event.resize(0);

  RawMult=0;
  LaBr3Mult=0; ParisMult=0; ParisLaBr3Mult=0;
  RawGeMult=0; CloverMult=0; CloverGeMult=0; CleanGeMult=0;
  BGOMult=0; ComptonVetoMult=0;
  DSSDMult = 0; DSSDSectorMult=0; DSSDRingMult=0;
  ModulesMult=0;

  PromptMult = 0; DelayedMult = 0;
  PromptGeMult = 0; DelayedGeMult = 0;
  DSSDPrompt=false;

#ifdef N_SI_129
  DSSDFirstBlob=false;
  DSSDSecondBlob=false;
  DSSDThirdBlob=false;
  DSSDTail = false;
#endif //N_SI_129

  m_Ge_gate = 0;
  m_CloverTrig = 0;
}

void Sorted_Event::setMaxTime(Float_t const & max_time)
{
  m_max_time = max_time;
}

bool Sorted_Event::sortGeClover(Event const & event, int const & i)
{
  auto const & label = event.labels[i];
  auto const & nrj = event.nrjs[i];
  auto const & clover_label = labelToClover[label];

  if (nrj<5) return false;

  push_back_unique(clover_hits, static_cast<uchar>(clover_label));

  nrj_clover[clover_label] += nrj;
  Ge[clover_label]++;

  // To get the crystal that got the maximum energy in a clover :
  if(nrj > maxE[clover_label])
  {
    if (event.read.T) time_clover[clover_label] = time2s[i];
    maxE[clover_label] = nrj;
    maxE_hit[clover_label] = i;
  }
  return true;
}

void Sorted_Event::sortEvent(Event const & event)
{
  this->reset();
  RawMult = event.size();

  uchar clover_label = 0;
  for (uchar i = 0; i<event.size(); i++)
  {
    if (event.read.t) time2s[i] = (m_rf) ? m_rf->pulse_ToF(event.time2s[i], 50000ll) : (event.time2s[i]-event.time2s[0])/_ns;
    if (event.read.T) time2s[i] = event.time2s[i]; // Overwrites the absolute time if relative Time is read in the data (parameter 'T' in connect() Event's method)
    if (time2s[i]>m_max_time) continue;
    if (isGe[event.labels[i]])
    {
      if (!sortGeClover(event, i)) continue;
      RawGeMult++;
    }
    else if (isBGO[event.labels[i]])
    {
      clover_label = labelToClover[event.labels[i]];
      if (m_rf) time_clover[clover_label] = m_rf->pulse_ToF(event.time2s[i], 50000ll)/_ns;
      else time_clover[clover_label] = (event.time2s[i]-event.time2s[0])/_ns;
      BGO[clover_label]++;
    }
    else if (isLaBr3[event.labels[i]])
    {
      LaBr3_Hit labr3_hit;
      labr3_hit.nrjcal = event.nrjs[i];
      labr3_hit.label = event.labels[i]-199;
      if (m_rf) labr3_hit.time = m_rf->pulse_ToF(event.time2s[i], 50000ll)/_ns;
      else labr3_hit.time = (event.time2s[i]-event.time2s[0])/_ns;
      LaBr3_event.push_back(labr3_hit);
      ModulesMult++;
      LaBr3Mult++;
    }
    else if (isParis[event.labels[i]])
    {
      paris_hits.push_back(i);
      ModulesMult++;
      LaBr3Mult++;
      ParisMult++;
    }
    else if (isDSSD[event.labels[i]])
    {
      DSSD_hits.push_back(i);
      auto const & nrj = event.nrjs[i];
      auto const & label = event.labels[i];
      #ifdef N_SI_129
      auto const & time = time2s[i];
      if (time>-5 && time<15 && nrj>8500 && nrj<11000) DSSDFirstBlob = true;
      else if (time> 3 && time<18 && nrj>5500 && nrj<7500) DSSDSecondBlob = true;
      else if (time>10 && time<20 && nrj>3800 && nrj<5500) DSSDThirdBlob = true;
      else if (time>25 && time<70 && nrj>2000 && nrj<3500) DSSDTail = true;
    #endif //N_SI_129
      if (m_TW_DSSD)
      {
        time2s[i] -= m_Timewalks_DSSD.get(label-800, nrj);
        DSSD_is_Prompt.push_back((time2s[i]>-10 && time2s[i]<10));
        if (time2s[i]<m_min_time_dssd || time2s[i]>m_max_time_dssd || nrj>m_max_E_dssd) dssd_veto = true;
      }
      else DSSD_is_Prompt.push_back((time2s[i]>0 && time2s[i]<50));


      bool const & is_ring = isRing[event.labels[i]];
      DSSD_is_Ring.push_back(is_ring);
      if (is_ring)
      {
        DSSD_Rings.push_back(i);
        DSSDRingMult++;
      }
      else
      {
        DSSD_Sectors.push_back(i);
        DSSDSectorMult++;
      }
      DSSDMult++;
    }
  }

  // Add-Back and Compton Cleaning of Clovers :
  for (size_t clover = 0; clover<24l; clover++)
  {
    if (Ge[clover])
    {
      if (time_clover[clover_label]>-10 && time_clover[clover_label]<70) prompt_Ge[clover_label] = true;
      else if (time_clover[clover_label]>70) delayed_Ge[clover_label] = true;
      CloverMult++;
      ModulesMult++;
      CloverGeMult++;
      if (BGO[clover]) ComptonVetoMult++;
      else
      {
        CleanGeMult++;
        // Energy gates :
        if (m_gateON && m_Ge_gate == 0) for (size_t gate = 0; gate<m_Ge_gates.size(); gate++)
        {
          if (nrj_clover[clover]>m_Ge_gates[gate].first && nrj_clover[clover]<m_Ge_gates[gate].second)
          {
            m_Ge_gate = gate+1;
            m_CloverTrig = clover;
            break;
          }
        }
      }
    }
    else if (BGO[clover])
    {
      CloverMult++;
      ModulesMult++;
      BGOMult++;
    }
  }

  //Analyse DSSD :
  for (auto const & prompt : DSSD_is_Prompt)
  {
    if (prompt)  DSSDPrompt = true;
    else {DSSDPrompt = false; break;}
  }
}

#endif //SORTED_EVENT_H
