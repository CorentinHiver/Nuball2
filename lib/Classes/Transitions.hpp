#include "../libCo.hpp"

namespace TransitionsLib
{
  constexpr static double hbar = 1.054571817e-34;  // Reduced Planck's constant in J.s
  constexpr static double c = 299792458;        // Speed of light in m/s
  constexpr static double epsilon0 = 8.8541878128e-12; // Vacuum permittivity in F/m
  constexpr static double e = 1.602176634e-19;  // Elementary charge in C
  constexpr static double me = 9.1093837015e-31; // electron mass in kg

  // Calculate the fine-structure constant
  constexpr static double alpha = e * e / (4 * M_PI * epsilon0 * hbar * c);
  constexpr static double alpha4 = alpha * alpha * alpha * alpha;
  constexpr static double p = 2 * me * c * c;
};

template<int n, int A, int Z>
struct ConversionCoefficient
{
  auto static constexpr alpha4 = TransitionsLib::alpha4;
  auto static constexpr p = TransitionsLib::p;
  auto static constexpr e = TransitionsLib::e;

  int const Z3 = 0;
  bool electric = true;
  double value = 0;
  double const & operator()() const {return value;}
  operator double() const & {return value;} 
  
  ConversionCoefficient() noexcept : 
    Z3(Z*Z*Z)
  {
  }

  ConversionCoefficient(double const & E, int const & L, bool const & _electric) noexcept : 
    Z3(Z*Z*Z),
    electric(_electric)
  {
    calculate(E, L, electric);
  }

  void calculate(double const & E, double const & L, bool const & _electric) noexcept {
    electric = _electric;
    auto const & E_joules = 1000 * e * E; // convert keV into Joules
    value = Z3/(n*n*n) * alpha4 * 
            ((electric) ? ((L/(L+1)) * std::pow(p/E_joules, L+5/2))   // electric
                        : (            std::pow(p/E_joules, L+3/2))); // magnetic
  }
};

template<int A, int Z>
class Conversion
{
public:
  int const Z3; 

  ConversionCoefficient<1, A, Z> D_K;
  ConversionCoefficient<2, A, Z> D_L;
  ConversionCoefficient<3, A, Z> D_M;

  double coefficient = 0;

  Conversion() : Z3(Z*Z*Z) {m_ok = false;}

  Conversion(double const & E, double const & L, bool const & electric) : 
    Z3(Z*Z*Z),
    D_K(A, Z, electric),
    D_L(A, Z, electric),
    D_M(A, Z, electric),
    coefficient(calculate(E, L, electric))
  {m_ok = true;}

  double const & calculate(double const & E, double const & L, bool const & electric)
  {
    D_K.calculate(E, L, electric);
    D_L.calculate(E, L, electric);
    D_M.calculate(E, L, electric);
    coefficient = D_K.value + D_L.value + D_M.value;
    return coefficient;
  }
  
private:
  bool m_ok = false;
};

template<int A, int Z>
class Level
{
public:
  double energy = 0; 
  int  spin = 0;
  bool parity = true;
  double lifetime = 0.;
  
  Level(double const & _energy, int const & _spin, bool const & _parity, double _lifetime = 0.) noexcept: 
    energy(_energy), spin(_spin), parity(_parity), lifetime(_lifetime), m_label(glabel++)
  {}

  bool operator<(Level const & other){return energy < other.energy;}
  bool operator>(Level const & other){return energy > other.energy;}  
  friend std::ostream& operator<<(std::ostream& out, Level const & level)
  {
    out << level.energy << " keV," << level.spin << ((level.parity) ? "+" : "-");
    return out;
  }

  auto const & label() const {return m_label;}

private:
  size_t m_label = 0;
  size_t static thread_local glabel;
};

template<int A, int Z>
size_t thread_local Level<A,Z>::glabel = 0;

template<int A, int Z>
class Transition
{
public:
  double energy = 0;    // in MeV
  int    L = 0;            // in hbar
  bool   parity = true;   // + -> true, - -> false
  double partial_lifetime = 0.; // in ns
  double intensity = 1.;
  Conversion<A, Z> conversion;

  Transition(Level<A, Z> const * L_i, Level<A, Z> const * L_f, double _intensity = 1, double _partial_lifetime = 0) noexcept:
    m_L_i(L_i),
    m_L_f(L_f),
    energy(L_i->energy - L_f->energy),
    L(L_i->spin - L_f->spin),
    parity(L_i->parity == L_f->parity),
    intensity(_intensity),
    partial_lifetime(_partial_lifetime),
    m_label({L_i->label(), L_f->label()})
  {
    conversion.calculate(energy, L, electric());
  }

  bool electric() const {return ((L%2) ? !parity : parity);}

  std::string multipolarity() const 
  {
    return (((electric()) ? "E" : "M") + std::to_string(L));
  }

  friend std::ostream& operator<<(std::ostream& out, Transition const & transition)
  {
    out << transition.m_label << " " << transition.energy << " " << transition.L << " " << nicer_bool(transition.parity) << " " 
    << transition.multipolarity() << " D=" << transition.conversion.coefficient;
    return out;
  }

  auto const & label() const {return m_label;}

private:
  Level<A,Z> const * m_L_i = nullptr;       // Initial level
  Level<A,Z> const * m_L_f = nullptr;       // Final level
  std::pair<size_t, size_t> m_label;
};

template<int A, int Z>
class Nucleus
{
public:
  Nucleus() noexcept = default;
  Nucleus(std::string const & name)
  {m_name = name;}

  template <typename... Args>
  void addLevel(Args&&... args) 
  {
    m_levels.emplace_back(std::forward<Args>(args)...);
    m_nodes.emplace_back(&m_levels.back());
  }

  template <typename... Args>
  void addTransition(int const & i, int const & f, Args&&... args) 
  {
    m_transitions.emplace_back(&m_levels[i], &m_levels[f], std::forward<Args>(args)...); // Add a new transition from initial level to final level
    m_nodes[m_levels[f].label()].transition_i.emplace_back(&m_transitions.back()); // Store the transition to feed final level
    m_nodes[m_levels[i].label()].transition_o.emplace_back(&m_transitions.back()); // Store the transition to decay out of initial level
  }

  auto const & getLevel(size_t const & i) const {return m_levels[i];}
  auto const & getTransition(size_t const & i) const {return m_transitions[i];}

  auto const & getLevels() const {return m_levels;}
  auto const & getTransitions() const {return m_transitions;}

  auto const & getNodes() const {return m_nodes;}

private:
  struct Node
  {
    Node(Level<A,Z> const * _level_i) noexcept : level_i(_level_i) {}

    Level<A,Z> const * level_i = nullptr;
    std::vector<Transition<A,Z> const *> transition_i;
    std::vector<Transition<A,Z> const *> transition_o;
    friend std::ostream& operator<<(std::ostream& out, Node const & node)
    {
      out << node.level_i->label() << " : in";
      for (auto const & trans_i : node.transition_i) out << " " << trans_i->energy;
      out << " : out";
      for (auto const & trans_o : node.transition_o) out << " " << trans_o->energy;
      out << std::endl;
      return out;
    }
  };
  std::vector<Level<A,Z>> m_levels;
  std::vector<Transition<A, Z>> m_transitions;
  std::vector<Node> m_nodes;
  std::string m_name = "";
};

void Transitions()
{
  Nucleus<236, 92> U236("Uranium 236");

  U236.addLevel(1052, 4, false); // 0

  U236.addLevel(987, 2, false); // 1

  U236.addLevel(848, 5, false); // 2
  U236.addLevel(744, 3, false); // 3
  U236.addLevel(688, 1, false); // 4

  U236.addLevel(149, 2, true); // 5
  U236.addLevel(45 , 2, true); // 6
  U236.addLevel(0  , 0, true); // 7


  U236.addTransition(0, 1, 0.65);
  U236.addTransition(0, 2, 1.00);
  U236.addTransition(0, 3, 0.90);
  U236.addTransition(0, 5, 0.41);

  U236.addTransition(1, 3, 0.26);
  U236.addTransition(1, 4, 0.17);
  U236.addTransition(1, 5, 1.00);

  U236.addTransition(2, 3, 1.00);

  U236.addTransition(3, 4, 0.05);
  U236.addTransition(3, 5, 1.00);

  U236.addTransition(4, 6, 1.00);
  U236.addTransition(4, 7, 0.27);


  print(U236.getLevels());
  print(U236.getTransitions());
  print(U236.getNodes());
}
