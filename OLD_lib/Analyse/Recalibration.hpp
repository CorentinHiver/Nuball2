#ifndef RECALIBRATION_HPP
#define RECALIBRATION_HPP


class Recalibration
{
public:
  Recalibration(double const & _a0 = 0, double const & _a1 = 1, double const & _a2 = 0, double const & _a3 = 0, double const & _aSqrt = 0) :
    m_parameters ({_a0, _a1, _a2, _a3, _aSqrt})
  {
  }

  Recalibration(Recalibration const & other)
  {
    m_parameters = other.m_parameters;
  }

  // Getters :
  //  const getters :
  inline auto const & operator[] (int const & i) const {return m_parameters[i];}
  inline auto const & getParameter (int const & i) const {return m_parameters[i];}
  inline auto const & a0    () const {return m_parameters[0];}
  inline auto const & a1    () const {return m_parameters[1];}
  inline auto const & a2    () const {return m_parameters[2];}
  inline auto const & a3    () const {return m_parameters[3];}
  inline auto const & aSqrt () const {return m_parameters[4];}

  //  not const getters :
  inline auto & operator[] (int const & i) {return m_parameters[i];}
  inline auto & getParameter (int const & i) {return m_parameters[i];}
  inline auto & a0    () {return m_parameters[0];}
  inline auto & a1    () {return m_parameters[1];}
  inline auto & a2    () {return m_parameters[2];}
  inline auto & a3    () {return m_parameters[3];}
  inline auto & aSqrt () {return m_parameters[4];}

  // Setters :
  inline auto & setParameter (int const & i, double const & a) {return (m_parameters[i] = a);}
  inline auto & setParameter (size_t const & i, double const & a) {return (m_parameters[i] = a);}
  inline auto & a0     (double const & a0)    {return (m_parameters[0] = a0);}
  inline auto & a1     (double const & a1)    {return (m_parameters[1] = a1);}
  inline auto & a2     (double const & a2)    {return (m_parameters[2] = a2);}
  inline auto & a3     (double const & a3)    {return (m_parameters[3] = a3);}
  inline auto & aSqrt  (double const & aSqrt) {return (m_parameters[4] = aSqrt);}

  
#ifdef MAX_ORDER2
  inline double calculate (double const & x) const {return a0() + a1()*x + a2()*x*x ;}
  inline double calculate (double const & x)       {return a0() + a1()*x + a2()*x*x ;}
#else 
  inline double calculate (double const & x) const {return a0() + a1()*x + a2()*x*x + a3()*x*x*x + aSqrt()*x;}
  inline double calculate (double const & x)       {return a0() + a1()*x + a2()*x*x + a3()*x*x*x + aSqrt()*x;}
#endif //MAX_ORDER2

  inline double operator() (double const & x) { return calculate(x) ;}
  inline double operator() (double const & x) const { return calculate(x) ;}

private:

  std::vector<double> m_parameters = std::vector<double>(5);
};

std::ostream& operator<<(std::ostream& out, Recalibration const & recal)
{
  out << recal.a0() << " + " << recal.a1() << "x + " << recal.a2() 
      << "x2 + " << recal.a3() << "x3 + " << recal.aSqrt() << "sqrt(x)";
  return out;
}


#endif //RECALIBRATION_HPP