#ifndef COUNTER136_HPP
#define COUNTER136_HPP

#include <Event.hpp>

class Counter136
{
public:
  bool counted = false; 
  bool analyzed = false;
  size_t nb_modules = 0;
  size_t nb_dssd = 0;
  size_t nb_Ge = 0;
  size_t nb_BGO = 0;
  size_t nb_clovers = 0;
  size_t nb_clovers_clean = 0;
  size_t nb_modules_prompt = 0;
  size_t nb_modules_delayed = 0;
  size_t nb_clover_clean_prompt = 0;
  size_t nb_clover_clean_delayed = 0;

  std::vector<int> clovers;
  std::vector<int> clovers_ge;
  std::vector<int> clovers_bgo;
  std::vector<int> clovers_clean_Ge;
  std::vector<int> clover_clean_prompt ;
  std::vector<int> clover_clean_delayed;

  std::array<std::vector<Time>, 24> ge_clovers_times;

  Counter136(Event* event) : m_event(event) {}

  // Timing:
  static std::pair<Time, Time> promptGate;
  bool isPrompt(Time const & time) {return (promptGate.first<time && time<promptGate.second);}
  static std::pair<Time, Time> delayedGate;
  bool isDelayed(Time const & time) {return (delayedGate.first<time && time<delayedGate.second);}


  void reset() noexcept  
  {
    counted = false;
    analyzed = false;
    nb_modules = 0; 
    nb_dssd = 0; 
    nb_Ge = 0; 
    nb_BGO = 0;
    nb_clovers = 0;
    nb_clovers_clean = 0;
    nb_modules_prompt = 0;
    nb_modules_delayed = 0;
    nb_clover_clean_prompt = 0;
    nb_clover_clean_delayed = 0;
    clovers.clear();
    clovers_ge.clear();
    clovers_bgo.clear();
    clovers_clean_Ge.clear();
    clover_clean_prompt.clear();
    clover_clean_delayed.clear();

    for (auto const & clover_i : clovers_ge)
    {
      ge_clovers_times[clover_i].clear();
    }
  }
  
  void count()
  {
    reset();
    for (int hit = 0; hit<m_event->mult; hit++)
    {
      auto const & label = m_event->labels[hit];
      auto const & time = m_event->times[hit];
      if (isDSSD[label]) ++nb_dssd;
      else
      {
        if (isClover[label])
        {
          auto const & clover_label = int_cast(labelToClover[label]);
               if (isPrompt(time)) ++nb_modules_prompt;
          else if (isDelayed(time)) ++nb_modules_delayed;
              if(isGe[label] ) 
          {
            ge_clovers_times[clover_label].push_back(time);
            push_back_unique(clovers, clover_label); 
            push_back_unique(clovers_ge , clover_label);
          }
          else if(isBGO[label]) 
          {
            push_back_unique(clovers, clover_label); 
            push_back_unique(clovers_bgo, clover_label);
          }
        }
        else if (isLaBr3[label] || isParis[label]) 
        {
          if (isPrompt(time)) ++nb_modules_prompt;
          else if (isDelayed(time)) ++nb_modules_delayed;
          ++nb_modules;
        }
      }
    }
    nb_Ge  = clovers_ge .size();
    nb_BGO = clovers_bgo.size();
    nb_modules += (nb_clovers = clovers.size());
    counted = true;
  }

  void analyse()
  {
    if (!counted) this->count();
    if (analyzed) return;
    for (auto & clover_i : clovers)
    {
      if (found(clovers_ge, clover_i) && !found(clovers_bgo, clover_i))
      {
        clovers_clean_Ge.push_back(clover_i);
        bool is_prompt = false;
        bool is_delayed = false;
        for (auto const & time : ge_clovers_times[clover_i]) 
        { // Here we loop over the times registered for the germaniums of the clover
          if (isPrompt(time)) is_prompt = true;
          if (isDelayed(time))
          {
            if (is_prompt) is_prompt = false; // If the clover is both prompt and delayed, then it is not clean
            else if (isDelayed(time)) is_delayed = true; // If the first hit is delayed then the whole clover is delayed
            break; // No need to continue looking for the other times of this clover
          }
        }
             if (is_prompt) clover_clean_prompt.push_back(clover_i);
        else if(is_delayed) clover_clean_delayed.push_back(clover_i);
      }
    }
    nb_clovers_clean = clovers_clean_Ge.size();
    nb_clover_clean_prompt = clover_clean_prompt.size();
    nb_clover_clean_delayed = clover_clean_delayed.size();
    analyzed = true;
  }

private:
  Event * m_event = nullptr;
};

std::pair<Time, Time> Counter136::promptGate  = {-10_ns, 10_ns};
std::pair<Time, Time> Counter136::delayedGate = {60_ns, 180_ns};

#endif // COUNTER136_HPP