#ifndef DETECTOR_H
#define DETECTOR_H

#include <libCo.hpp>

using Label = ushort;

std::vector<char> isClover(1000,false);
std::vector<char> isGe(1000,false);
std::vector<char> isBGO(1000,false);
std::vector<char> isLaBr3(1000,false);
std::vector<char> isEden(1000,false);
std::vector<char> isRF(1000,false);
std::vector<char> isParis(1000,false);
std::vector<char> isDSSD(1000,false);
std::vector<char> isDSSD_Sector(1000,false);
std::vector<char> isDSSD_Ring(1000,false);


class Detectors
{
public:
  static std::string type (ushort const & label)
  {
    if      (isGe    [label]) return "Ge";
    else if (isBGO   [label]) return "BGO";
    else if (isLaBr3 [label]) return "LaBr3";
    else if (isParis [label]) return "paris";
    else if (isDSSD  [label]) return "dssd";
    else if (isRF    [label]) return "RF";
    else if (isEden  [label]) return "EDEN";
    else                      return "null";
  }
  static void Initialize();
};

void Detectors::Initialize()
{
  isRF[251] = true;
  for (int label = 0; label<1000; label++)
  {
    isBGO[label] = (label>22 && label<167 && (label+1)%6 <2 );
    isGe[label]  = (label>22 && label<167 && (label+1)%6 >1 );

  #if defined (USE_LICORNE)
    isLaBr3[label] = (label>199 && label<220);
    isEden[label] = (label == 500 || label == 501);
  #elif defined (USE_PARIS)
    isEden[label] = false;
    isLaBr3[label] = (label == 252);
    isParis[label] =  ((label > 200 && label < 209)
                    || (label > 300 && label < 317)
                    || (label > 400 && label < 413)
                    || (label > 500 && label < 509)
                    || (label > 600 && label < 617)
                    || (label > 700 && label < 713));
  #endif

  #ifdef USE_DSSD
    isDSSD[label] = ( (label>799 && label<816) || (label>819 && label<836) || (label>839 && label<856) );
    isDSSD_Sector[label] = ( (label>799 && label<816) || (label>819 && label<836) );
    isDSSD_Ring[label] = ( (label>839 && label<856) );
  #else
    isDSSD[label] = false;
    isDSSD_Sector[label] = false;
    isDSSD_Ring[label] = false;
  #endif //USE_DSSD
  }
}

#endif //DETECTOR_H
