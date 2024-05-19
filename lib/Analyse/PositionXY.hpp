#ifndef POSITIONXY_H
#define POSITIONXY_H

class PositionXY
{
public:
  PositionXY(){}
  PositionXY(double const & x, double const & y) : m_x(x), m_y(y) {}
  PositionXY(PositionXY const & pos) {*this = pos;}
  void operator= (PositionXY const & pos) {Set(pos.x(), pos.y());}
  void inline Set(double const & x, double const & y) {m_x = x; m_y = y;}

  double const & x() const {return m_x;}
  double const & y() const {return m_y;}

  static inline double distance(PositionXY const & coord_1, PositionXY const & coord_2)
  {
    auto const dx = coord_2.x()-coord_1.x();
    auto const dy = coord_2.y()-coord_1.y();
    return (sqrt(dx*dx+dy*dy));
  }

  inline double distance(PositionXY const & coord_2)
  {
    return distance(*this, coord_2);
  }

  void Print() {print("x :", m_x, "y : ", m_y);}

private:
  double m_x = 0;
  double m_y = 0;
};

std::ostream& operator<< (std::ostream& out, PositionXY const & pos)
{
  // out << "x : " << pos.x() <<  " y : " << pos.y();
  out << std::setw(2) << pos.x() << ";" << std::setw(2) <<  pos.y();
  return out;
}
#endif //POSITIONXY_H
