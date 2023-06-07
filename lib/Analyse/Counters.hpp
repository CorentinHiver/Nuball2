#ifndef COUNTERS_H
#define COUNTERS_H

#include <libCo.hpp>
#include <Event.hpp>
#include <Detectors.hpp>

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
  bool hasTrigged();

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

  bool trig = false;
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
      auto const & clover = labelToClover[label];
      Ge_Clover  [clover] = true;
      list_clovers.push_back_unique(clover);
    }
    else if (isBGO[label])
    {
      RawBGO++;
      auto const & clover = labelToClover[label];
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

bool Counters::hasTrigged()
{
#ifdef NO_TRIG
  trig = true;
#endif //NO_TRIG

#ifdef G1_TRIG
  trig = (!trig) ? Counter.RawGe>0 : true; // At least one Ge cystal
#endif //G1_TRIG

#ifdef D1_TRIG
  trig = (!trig) ? Counter.DSSDMult>0 : true; // At least one dssd
#endif //D1_TRIG

#ifdef M2_TRIG
  trig = (!trig) ? Counter.Modules>1 : true; // At least 2 modules
#endif //M2_TRIG

#ifdef L2_TRIG
  trig = (!trig) ? Counter.LaBr3Mult>1 : true; // At least 2 modules
#endif //M2_TRIG

#ifdef C2_TRIG
  trig = (!trig) ? Counter.CleanGeMult>1 : true; // At least 2 modules
#endif //C2_TRIG

#ifdef CG2_TRIG
  trig = (!trig) ? Counter.CloverGeMult>1 : true; // At least two Clover Ge
#endif //CG2_TRIG

#ifdef G1M2_TRIG
  trig = (!trig) ? (Counter.RawGe>0 && Counter.Modules>1) : true; // At least one clean Ge and 2 LaBr3
#endif //C1L2_TRIG

#ifdef C1L2_TRIG
  trig = (!trig) ? (Counter.CleanGeMult>0 && Counter.LaBr3Mult>1) : true; // At least one clean Ge and 2 LaBr3
#endif //C1L2_TRIG

#ifdef CG1L2_TRIG
  trig = (!trig) ? (Counter.CloverGeMult>0 && Counter.LaBr3Mult>1) : true; // At least one Clover Ge and 2 LaBr3
#endif //CG1L2_TRIG

#ifdef DOWNSCALE_M1
  if(Counter.ModulesMult==1) M1_counter++;
  if (M1_counter>1000)
  {
    trig = true;
    M1_counter = 0;
  }
#endif //DOWNSCALE_M1
 return true;
}


#endif //COUNTERS_H
