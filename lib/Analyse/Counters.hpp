#ifndef COUNTERS_H
#define COUNTERS_H

#include "../utils.hpp"
#include "../Classes/Event.hpp"

class Counters
{
public:
  Counters()
  {
    for (int i = 0; i<48; i++) list_clovers[i] = 0;
    for (int i = 0; i<24; i++)
    {
      Ge_Clover[i] = false;
      BGO_Clover[i] = false;
    }
  }

  void clear();
  void count_event(Event const & event);

  uchar mult = 0;
  uchar RawGe=0;
  uchar RawBGO=0;
  uchar GeCleanClovers=0;
  uchar GeClovers=0;
  uchar Clovers=0;
  uchar Modules=0;
  uchar LaBr3Mult=0;
  uchar ParisMult=0;
  uchar DSSDMult=0;

  uchar list_clovers[48];
  std::array<Bool_t, 24> Ge_Clover;
  std::array<Bool_t, 24> BGO_Clover;

};

void Counters::clear()
{
  for (uchar i = 0; i<Clovers; i++)
  {
    auto const & clover = list_clovers[i];
    Ge_Clover [clover] = false;
    BGO_Clover[clover] = false;
  }

  mult = 0;
  RawGe = 0;
  RawBGO = 0;
  GeCleanClovers = 0;
  GeClovers = 0;
  Clovers = 0;
  Modules = 0;
  LaBr3Mult = 0;
  ParisMult = 0;
  DSSDMult = 0;
}

void Counters::count_event(Event const & event)
{
  clear();
  mult = event.mult;
  for (uchar i = 0; i<mult; i++)
  {
    auto const & label = event.labels[i];
    if(isGe[label])
    {
      RawGe++;
      Ge_Clover  [labelToClover_fast[label]] = true;
    }
    else if (isBGO[label])
    {
      RawBGO++;
      BGO_Clover [labelToClover_fast[label]] = true;
    }
    else if (isLaBr3[label])
    {
      #ifdef FATIMA
      LaBr3Mult++;
      Modules++;
      #endif //FATIMA
    }
    else if (isParis[label])
    {
      #ifdef PARIS
      ParisMult++;
      Modules++;
      #endif //PARIS
    }
    else if (isDSSD[label])
    {
      #ifdef USE_DSSD
      DSSDMult++;
      #endif //USE_DSSD
    }
  }

  // Quick analysis of the Clovers :
  for (uchar clover = 0; clover<24; clover++)
  {
    if(BGO_Clover[clover])
    {
      list_clovers[Clovers] = clover;
      Modules++;
      Clovers++;
    }
    if(Ge_Clover[clover])
    {
      list_clovers[Clovers] = clover;
      Modules++;
      Clovers++;
      GeClovers++;
      if(!BGO_Clover[clover])
      {
        GeCleanClovers++;
      }
    }
  }
}


#endif //COUNTERS_H
