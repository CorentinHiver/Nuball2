#ifndef TIMEGATE_H
#define TIMEGATE_H

/*
 * Uses only the relative time !
 *
*/

class TimeGate
{
public:
  TimeGate(){}

  void operator= (std::pair <Float_t,Float_t> const & gate) {start = gate.first; stop = gate.second;}
  void operator= (TimeGate const & timegate) {start = timegate.start; stop = timegate.stop;}

  bool isIn(Float_t const & time) {return (time>start && time<stop);}

  Float_t start;
  Float_t stop;
};

#endif //TIMEGATE_H
