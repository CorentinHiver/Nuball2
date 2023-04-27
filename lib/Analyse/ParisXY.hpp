#ifndef PARISXY_H
#define PARISXY_H
#include "ParisLabel.hpp"
#include "Arrays/Paris.h"

class ParisXY
{
public:
  ParisXY(Paris_Label const & l)
  {
    switch (l.ring)
    {
      case 1: m_x = Paris_R1_x[l.label-1]; m_y = Paris_R1_y[l.label-1]; break;
      case 2: m_x = Paris_R2_x[l.label-1]; m_y = Paris_R2_y[l.label-1]; break;
      case 3: m_x = Paris_R3_x[l.label-1]; m_y = Paris_R3_y[l.label-1]; break;
      default: print("Problème code..."); break;
    }
  }

  int const & x() const {return m_x;}
  int const & y() const {return m_y;}

  static inline Float_t distance(ParisXY const & coord_1, ParisXY const & coord_2)
  {
    return (TMath::Sqrt(coord_1.x()*coord_2.x()+coord_1.y()*coord_2.y()));
  }

private:
  int m_x = 0;
  int m_y = 0;
};
#endif //PARISXY_H
