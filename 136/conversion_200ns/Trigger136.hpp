#ifndef COUNTER136_HPP
#define COUNTER136_HPP

#include <CloversV2.hpp>

class Trigger136
{
public:
  Trigger136(Event* event) : m_event(event) {}

  size_t nb_dssd = 0;
  size_t nb_Ge = 0;
  size_t nb_BGO = 0;
  size_t nb_clovers = 0;
  size_t nb_modules = 0;
  size_t nb_clovers_clean = 0;
  size_t nb_pclovers_clean = 0;
  size_t nb_dclovers_clean = 0;
  size_t nb_pclovers = 0;
  size_t nb_dclovers = 0;
  size_t nb_pmodules = 0;
  size_t nb_dmodules = 0;

  CloversV2 clovers;

  static char choice;

  void reset() noexcept  
  {
    analyzed = false;
    nb_dssd = 0; 
    nb_Ge = 0; 
    nb_BGO = 0; 
    nb_clovers = 0;
    nb_modules = 0;
    nb_clovers_clean = 0;
    nb_pclovers_clean = 0;
    nb_dclovers_clean = 0;
    nb_pclovers = 0;
    nb_dclovers = 0;
    nb_pmodules = 0;
    nb_dmodules = 0;
    clovers.reset();
  }
  
  void count()
  {
    reset();
    for (int hit_i = 0; hit_i<m_event->mult; hit_i++)
    {
      auto const & label = m_event->labels[hit_i];
      auto const & time = m_event->times[hit_i];
      if (isDSSD[label]) ++nb_dssd;
      else if (isLaBr3[label] || isParis[label]) 
      {
        if (isPrompt(time)) ++nb_pmodules;
        else if (isDelayed(time)) ++nb_dmodules;
      }
      else if (CloversV2::isClover(label))
      {
        if (CloversV2::isGe(label)) ++nb_Ge;
        else if (CloversV2::isBGO(label)) ++nb_BGO;
        clovers.fill(*m_event, hit_i);
      }
    }
  }

  void analyze()
  {
    this->count(); // Need to fill the clovers before analyzing them
    if (analyzed) Colib::throw_error("in Trigger136::analyze() : already analyzed !!"); // Don't analyze twice
    clovers.analyze();
    for (auto const & id_i : clovers.Hits_id) 
    {
      if      (isPrompt(clovers[id_i].time )) ++nb_pclovers;
      else if (isDelayed(clovers[id_i].time)) ++nb_dclovers;
    }
    for (auto const & id_i : clovers.GeClean_id)
    {
      if      (isPrompt(clovers[id_i].time )) ++nb_pclovers_clean;
      else if (isDelayed(clovers[id_i].time)) ++nb_dclovers_clean;
    }
    nb_clovers = nb_pclovers + nb_dclovers;
    nb_modules = nb_clovers + nb_pmodules + nb_dmodules;
    nb_clovers_clean = nb_pclovers_clean+nb_dclovers_clean;
    analyzed = true;
  }

  bool pass()
  {
    if (choice<7)
    {
      this->count();
      switch (choice)
      {
        case 0: return nb_dssd>0; //P
        case 1: return nb_modules>2 && nb_Ge>0; //M3G1
        case 2: return (nb_dssd>0 || (nb_modules>2 && nb_Ge>0)); //P_M3G1
        case 3: return (nb_dssd>0 && nb_modules>1 && nb_Ge>0); //PM2G1
        case 4: return (nb_dssd>0 || (nb_modules>3 && nb_Ge>0)); //P_M4G1
        case 5: return (nb_modules>3 && nb_Ge>0); //M4G1
        case 6: return nb_Ge > 1; // G2
        default: return true;
      }
    }
    else
    {
      this->analyze();
      switch (choice)
      {
        case 7:  return nb_clovers_clean > 1; // C2
        case 8:  return nb_clovers_clean > 1 && nb_dssd > 0; // PC2
        case 9:  return nb_clovers_clean > 0 && nb_dssd > 0; // PC1
        case 10: return nb_pmodules > 0 && nb_dclovers_clean > 0;// pM1dC1
        case 11: return nb_dclovers_clean > 0;// dC1
        case 12: return nb_dclovers_clean > 1;// dC2
        default: return true;
      }
    }
  }

  static std::map<char, std::string> names ;
  static std::string legend;

private:
  Event * m_event = nullptr;
  bool analyzed = false;
};

char Trigger136::choice = -1;

std::map<char, std::string> Trigger136::names = 
{
  {-1,"all"},
  {0, "p"},
  {1, "M3G1"},
  {2, "p_M3G1"},
  {3, "pM2G1"},
  {4, "p_M4G1"},
  {5, "M4G1"},
  {6, "G2"},
  {7, "C2"},
  {8, "PC2"},
  {9, "PC1"},
  {10, "pM1dC1"},
  {11, "dC1"},
  {12, "dC2"}
};

std::string Trigger136::legend = "Legend : p = Particle | G = Germanium | M = Module | C = Clean Germanium | _ = OR | p = Prompt | d = Delayed";

#endif // COUNTER136_HPP