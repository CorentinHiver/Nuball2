#ifndef COUNTER136_HPP
#define COUNTER136_HPP

#include <CloversV2.hpp>

class Trigger136
{
public:
  size_t nb_dssd = 0;
  size_t nb_Ge = 0;
  size_t nb_BGO = 0;
  size_t nb_clovers = 0;
  size_t nb_modules = 0;
  size_t nb_clovers_clean = 0;
  size_t nb_modules_prompt = 0;
  size_t nb_modules_delayed = 0;

  CloversV2 prompt_clover;
  CloversV2 delayed_clover;

  static char choice;

  Trigger136(Event* event) : m_event(event) {}

  void reset() noexcept  
  {
    analyzed = false;
    nb_modules = 0; 
    nb_dssd = 0; 
    nb_Ge = 0; 
    nb_BGO = 0;
    nb_clovers = 0;
    nb_clovers_clean = 0;
    nb_modules_prompt = 0;
    nb_modules_delayed = 0;
    prompt_clover.reset();
    delayed_clover.reset();
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
        if (isPrompt(time)) ++nb_modules_prompt;
        else if (isDelayed(time)) ++nb_modules_delayed;
      }
      else if (CloversV2::isClover(label))
      {
        if (CloversV2::isGe(label)) 
        {
          ++nb_Ge;
          
        }
        else if (CloversV2::isBGO(label)) ++nb_BGO;
        if (isPrompt(time)) prompt_clover.fill(*m_event, hit_i);
        else if (isDelayed(time)) delayed_clover.fill(*m_event, hit_i);
      }
    }
  }

  void analyze()
  {
    this->count(); // Need to fill the clovers before analyzing them
    if (analyzed) Colib::throw_error("in Trigger136::analyze() : already analyzed !!"); // Don't analyze twice
    prompt_clover.analyze();
    delayed_clover.analyze();
    for (auto const & index_i : prompt_clover.GeClean) cleanGe_VS_time.Fill(prompt_clover[index_i].time, prompt_clover[index_i].nrj);
    for (auto const & index_i : delayed_clover.GeClean) cleanGe_VS_time.Fill(delayed_clover[index_i].time, delayed_clover[index_i].nrj);
    nb_modules_prompt+=prompt_clover.Hits.size();
    nb_modules_delayed+=delayed_clover.Hits.size();
    nb_clovers += nb_modules_prompt + nb_modules_delayed;
    nb_clovers_clean += prompt_clover.GeClean.size() + delayed_clover.GeClean.size();
    nb_modules = nb_clovers + nb_modules_prompt + nb_modules_delayed;
    analyzed = true;
  }

  bool pass()
  {
    if (choice<7)
    {
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
        case 7:  return prompt_clover.GeClean.size() + delayed_clover.GeClean.size() > 1; //C2
        case 8:  return nb_clovers > 1 && nb_dssd > 0; // PC2
        case 9:  return nb_clovers > 0 && nb_dssd > 0; // PC1
        case 10: return nb_modules_prompt > 0 && delayed_clover.GeClean.size() > 0;// pM1dC1
        case 11: return delayed_clover.GeClean.size() > 0;// dC1
        case 12: return delayed_clover.GeClean.size() > 1;// dC2
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
  {0, "P"},
  {1, "M3G1"},
  {2, "P_M3G1"},
  {3, "PM2G1"},
  {4, "P_M4G1"},
  {5, "M4G1"},
  {6, "G2"},
  {7, "C2"},
  {8, "PC2"},
  {9, "PC1"},
  {10, "pM1dC1"},
  {11, "dC1"},
  {12, "dC2"}
};

std::string Trigger136::legend = "Legend : P = Particle | G = Germanium | M = Module | C = Clean Germanium | _ = OR | p = Prompt | d = Delayed";

#endif // COUNTER136_HPP