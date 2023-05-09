#ifndef GATE_H
#define GATE_H

/*
 * Uses only the relative time !
 *
*/

class Gate
{
public:
  Gate(){}

  void operator= (std::pair <Float_t,Float_t> const & gate) {start = gate.first; stop = gate.second;}
  void operator= (Gate const & timegate) {start = timegate.start; stop = timegate.stop;}

  bool isIn(Float_t const & time) {return (time>start && time<stop);}

  Float_t start;
  Float_t stop;

  void use() {m_use = true;}
  bool const & used() const {return m_use;}

private:
  bool m_use = false;
};

#endif //GATE_H
