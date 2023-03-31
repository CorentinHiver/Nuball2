#ifndef PARISARRAY_H
#define PARISARRAY_H

#include "ParisXY.hpp"

class Paris_Hit
{
public:
  Paris_Label   label;
  Float_t       nrjcal  = 0;
  Float_t       nrjcal2 = 0;
  Int_t         crystal = 0;
  Detector      type = null;
  Time          time    = 0;
  static std::vector<char> labelToSide;
  static std::vector<uchar> labelToRing;
  static std::vector<uchar> labelToLabel;
};

std::vector<char> labelToSide;
std::vector<uchar> labelToRing;
std::vector<uchar> labelToLabel;
using Paris_Event  = std::vector < Paris_Hit >;

class ParisArray
{
public:
  ParisArray(){}
private:
  static std::array<std::array<int,28>, 28> Paris_Back_Cluster_voisins;
  static std::array<std::array<int,32>, 32> Paris_Front_Cluster_voisins;
}


#endif //PARISARRAY_H
