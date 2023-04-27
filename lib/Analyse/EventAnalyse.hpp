#ifndef EVENTANALYSE_H
#define EVENTANALYSE_H

#include "Clovers.hpp"
#include "DSSD.hpp"
#include "Paris.hpp"
#include "../Classes/Event.hpp"

class EventAnalyse
{
public:
  EventAnalyse();
  ~EventAnalyse(){}
  void Set(Event const & event);

  Clovers & getClovers() {return m_Clovers;}
  Paris   & getParis()   {return m_Paris  ;}
  DSSD    & getDSSD()    {return m_DSSD   ;}

private:
  Clovers m_Clovers;
  Paris   m_Paris;
  DSSD    m_DSSD;
};

EventAnalyse::EventAnalyse()
{
  // Initialize arrays :
  Clovers::Initialize();
  Paris::Initialize();

  // Initialize Paris :
  m_Paris.Initialize(2,36);
}

void EventAnalyse::Set(Event const & event)
{
  m_Clovers . Reset();
  m_Paris   . Reset();
  m_DSSD    . Reset();
  for (size_t i = 0; i < event.size(); i++)
  {
    auto const & label = event.labels[i];
         if (Clovers::is[label]) m_Clovers . Fill(event,i);
    else if (isParis[label])     m_Paris   . Fill(event,i);
    else if (isDSSD[label])      m_DSSD    . Fill(event,i);
  }
  m_Paris.Analyse();
  m_Clovers.Analyse();
}

#endif //EVENTANALYSE_H
