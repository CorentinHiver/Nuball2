#ifndef COUNTER136_HPP
#define COUNTER136_HPP

#include <CloversV2.hpp>

bool isPrompt(Time const & time)  {return (-10_ns < time && time < 10_ns );}
bool isDelayed(Time const & time) {return (60_ns  < time && time < 200_ns);}
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

  CloversV2 prompt_clover;
  CloversV2 delayed_clover;

  Counter136(Event* event) : m_event(event) {}

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
        if (CloversV2::isGe(label)) ++nb_Ge;
        else if (CloversV2::isBGO(label)) ++nb_BGO;
        if (isPrompt(time)) prompt_clover.fill(*m_event, hit_i);
        else if (isDelayed(time)) delayed_clover.fill(*m_event, hit_i);
      }
    }
    counted = true;
  }

  void analyse()
  {
    if (!counted) this->count(); // Need to fill the clovers before analysing them
    if (analyzed) throw_error("in Counter136::analyse() : already analysed !!"); // Don't analyse twice
    prompt_clover.analyze();
    delayed_clover.analyze();
    nb_modules_prompt+=prompt_clover.Hits.size();
    nb_modules_delayed+=delayed_clover.Hits.size();
    nb_clovers += nb_modules_prompt + nb_modules_delayed;
    nb_clovers_clean += prompt_clover.GeClean.size() + delayed_clover.GeClean.size();
    nb_modules = nb_clovers + nb_modules_prompt + nb_modules_delayed;
    analyzed = true;
  }

private:
  Event * m_event = nullptr;
};
#endif // COUNTER136_HPP