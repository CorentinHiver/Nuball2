#ifndef POSITIONXY_H
#define POSITIONXY_H

class PositionXY
{
public:
  PositionXY(){}
  PositionXY(Float_t const & x, Float_t const & y) : m_x(x), m_y(y) {}
  PositionXY(PositionXY const & pos) {*this = pos;}
  void operator= (PositionXY const & pos) {Set(pos.x(), pos.y());}
  void inline Set(Float_t const & x, Float_t const & y) {m_x = x; m_y = y;}

  Float_t const & x() const {return m_x;}
  Float_t const & y() const {return m_y;}

  static inline Float_t distance(PositionXY const & coord_1, PositionXY const & coord_2)
  {
    auto const dx = coord_2.x()-coord_1.x();
    auto const dy = coord_2.y()-coord_1.y();
    return (TMath::Sqrt(dx*dx+dy*dy));
  }

  inline Float_t distanceTo(PositionXY const & coord_2)
  {
    return distance(*this, coord_2);
  }

  void Print() {print("x :", m_x, "y : ", m_y);}

private:
  Float_t m_x = 0;
  Float_t m_y = 0;
};
#endif //POSITIONXY_H
