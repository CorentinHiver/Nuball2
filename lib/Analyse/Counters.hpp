#ifndef COUNTERS_H
#define COUNTERS_H

#include "../utils.hpp"
#include "../Classes/Event.hpp"

class Counters
{
public:
  Counters()
  {
    list_clovers.resize();
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

  StaticVector<uchar,24> list_clovers;
  std::array<Bool_t, 24> Ge_Clover;
  std::array<Bool_t, 24> BGO_Clover;

};

void Counters::clear()
{
  for (auto clover : list_clovers)
  {
    Ge_Clover [clover] = false;
    BGO_Clover[clover] = false;
  }
  list_clovers.resize();
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
  this -> clear();
  mult = event.mult;
  for (uchar i = 0; i<mult; i++)
  {
    auto const & label = event.labels[i];
    if(isGe[label])
    {
      RawGe++;
      auto const & clover = labelToClover_fast[label];
      Ge_Clover  [clover] = true;
      list_clovers.push_back_unique(clover);
    }
    else if (isBGO[label])
    {
      RawBGO++;
      auto const & clover = labelToClover_fast[label];
      BGO_Clover [clover] = true;
      list_clovers.push_back_unique(clover);
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

  Clovers  = list_clovers.size();
  Modules += Clovers;

  // Quick analysis of the Clovers :
  for (auto const & clover : list_clovers)
  {
    if(Ge_Clover[clover])
    {
      GeClovers++;
      if(!BGO_Clover[clover])
      {
        GeCleanClovers++;
      }
    }
  }
}


#endif //COUNTERS_H
