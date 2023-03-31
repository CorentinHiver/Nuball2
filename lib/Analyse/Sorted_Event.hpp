#ifndef SORTED_EVENT_H
#define SORTED_EVENT_H
#include "../utils.hpp"
#include "../Classes/Event.hpp"

class Sorted_Event
{
public:
  Sorted_Event(){initialise(); reset();}
  Sorted_Event(Event * event){initialise(); reset(); setEvent(event);}
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

  void sortEvent(Event const & event);
  void sortEvent(){sortEvent(*m_event);}
  bool sortGeClover(Event const & event, int const & i);

  int const & gateGe() const {return m_Ge_gate;}
  auto const & getGatedClover() const {return m_CloverTrig;}

  std::array<Float_t, 2000> times;
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
};

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
  DSSD_is_Ring.resize(0);
  DSSD_Rings.resize(0);
  DSSD_Sectors.resize(0);

  LaBr3_event.resize(0);

  RawMult=0;
  LaBr3Mult=0;
  ParisMult=0;
  ParisLaBr3Mult=0;
  RawGeMult=0;
  CloverMult=0;
  CloverGeMult=0;
  CleanGeMult=0;
  BGOMult=0;
  DSSDMult = 0;
  DSSDSectorMult=0;
  DSSDRingMult=0;
  ComptonVetoMult=0;
  ModulesMult=0;

  PromptMult = 0;
  DelayedMult = 0;
  PromptGeMult = 0;
  DelayedGeMult = 0;

#ifdef N_SI_129
  DSSDFirstBlob=false;
  DSSDSecondBlob=false;
  DSSDThirdBlob=false;
  DSSDTail = false;
#endif //N_SI_129
  DSSDPrompt=false;

  m_Ge_gate = 0;
  m_CloverTrig = 0;
}

bool Sorted_Event::sortGeClover(Event const & event, int const & i)
{
  auto const & label = event.labels[i];
  auto const & time = event.times[i];
  auto const & Time = event.Times[i];
  auto const & nrj = event.nrjs[i];
  auto const & clover_label = labelToClover_fast[label];

  if (nrj<5) return false;

  push_back_unique(clover_hits, clover_label);

  if (event.readtime())
  {
    if (m_rf) time_clover[clover_label] = m_rf->pulse_ToF(time, 50000ll)/_ns;
    else time_clover[clover_label] = (time-event.times[0])/_ns;
  }

  // Next line overwrites the Time (relative timestamp) if Time is indeed read in the event
  if (event.readTime()) time_clover[clover_label] = Time;

  nrj_clover[clover_label] += nrj;
  Ge[clover_label]++;

  if(nrj > maxE[clover_label])
  {// To get the crystal that got the maximum energy in a clover :
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
    if (event.readtime()) times[i] = (m_rf) ? m_rf->pulse_ToF(event.times[i], 50000ll) : (event.times[i]-event.times[0])/_ns;
    if (event.readTime()) times[i] = event.Times[i];

    if (isGe[event.labels[i]])
    {
      if (!sortGeClover(event, i)) continue;
      RawGeMult++;
    }
    else if (isBGO[event.labels[i]])
    {
      clover_label = labelToClover_fast[event.labels[i]];
      if (m_rf) time_clover[clover_label] = m_rf->pulse_ToF(event.times[i], 50000ll)/_ns;
      else time_clover[clover_label] = (event.times[i]-event.times[0])/_ns;
      BGO[clover_label]++;
    }
    else if (isLaBr3[event.labels[i]])
    {
      LaBr3_Hit labr3_hit;
      labr3_hit.nrjcal = event.nrjs[i];
      labr3_hit.label = event.labels[i]-199;
      if (m_rf) labr3_hit.time = m_rf->pulse_ToF(event.times[i], 50000ll)/_ns;
      else labr3_hit.time = (event.times[i]-event.times[0])/_ns;
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
      auto const & time = times[i];
    #ifdef N_SI_129
      auto const & nrj = event.nrjs[i];
      if (time>-5 && time<15 && nrj>8500 && nrj<11000) DSSDFirstBlob = true;
      else if (time> 3 && time<18 && nrj>5500 && nrj<7500) DSSDSecondBlob = true;
      else if (time>10 && time<20 && nrj>3800 && nrj<5500) DSSDThirdBlob = true;
      else if (time>25 && time<70 && nrj>2000 && nrj<3500) DSSDTail = true;
    #endif //N_SI_129
      if (time>0&&time<50)DSSDPrompt=true;
      bool isRing = isDSSD_Ring[event.labels[i]];
      DSSD_is_Ring.push_back(isRing);
      if (isRing)
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
}

#endif //SORTED_EVENT_H
