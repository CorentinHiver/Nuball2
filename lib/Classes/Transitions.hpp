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

namespace CoLib
{
  double static gate(double const & energyTransition, double const & energy, double const & resolution)
  {
    return ((energy-resolution) < energyTransition) && (energyTransition < (energy+resolution));
  }

}

template<int A, int Z>
class Conversion
{
public:
  int static constexpr Z3 = Z*Z*Z; 

  ConversionCoefficient<1, A, Z> D_K;
  ConversionCoefficient<2, A, Z> D_L;
  ConversionCoefficient<3, A, Z> D_M;

  double coefficient = 0;

  Conversion() {m_ok = false;}

  Conversion(double const & E, double const & L, bool const & electric) : 
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
  double energy   = 0; 
  double spin     = 0;    
  bool   parity   = true; // false = +, true = -
  double lifetime = 0.;   // in s
  
  Level(size_t const & _label, double const & _energy, int const & _spin, bool const & _parity, double _lifetime = 0.) noexcept: 
    energy(_energy), spin(_spin), parity(_parity), lifetime(_lifetime), m_label(_label)
  {}

  bool operator<(Level const & other) const {return energy < other.energy;}
  bool operator>(Level const & other) const {return energy > other.energy;}

  friend std::ostream& operator<<(std::ostream& out, Level const & level)
  {
    out << level.m_label << " " << level.energy << " keV," << level.spin << ((level.parity) ? "+" : "-");
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
  double energy = 0;            // in MeV
  int    L = 0;                 // in hbar
  bool   parity = true;         // + -> true, - -> false
  double partial_lifetime = 0.; // in ns
  double branchingRatio = 1.;

  Conversion<A, Z> conversion;
  Transition(Level<A, Z> const * Level_i, 
             Level<A, Z> const * Level_f,
             double _branchingRatio = 1,
             double _partial_lifetime = 0
            ) noexcept:
    energy            (Level_i->energy - Level_f->energy),
    L                 (int_cast(std::abs(Level_i->spin - Level_f->spin))),
    parity            (Level_i->parity != Level_f->parity),
    partial_lifetime  (_partial_lifetime),
    branchingRatio    (_branchingRatio),
    m_Level_i         (Level_i),
    m_Level_f         (Level_f),
    m_label           ({Level_i->label(), Level_f->label()})
  {
    if (L==0) L = 1;
    conversion.calculate(energy, L, electric());
    if (energy<0) {error("Transition :", Level_i, "to", Level_f, ": energy negative !!"); m_ok = false;}
  }

  bool electric() const {return ((L%2) ? parity : !parity);}

  std::string multipolarity() const 
  {
    return (((electric()) ? "E" : "M") + std::to_string(L));
  }

  double Weisskopf() const 
  {
    auto const & M = multipolarity();
          if (M == "E1") return 1.0e+14 *   std::pow(A, 2/3.) * CoLib::pow(energy, 3);
    else  if (M == "E2") return 7.3e+7  *   std::pow(A, 4/3.) * CoLib::pow(energy, 5);
    else  if (M == "E3") return 34.e+0  * CoLib::pow(A, 3   ) * CoLib::pow(energy, 7);
    else  if (M == "E4") return 1.1e-5  *   std::pow(A, 8/3.) * CoLib::pow(energy, 9);

    else  if (M == "M1") return 5.6e+13 *                       CoLib::pow(energy, 3);
    else  if (M == "M2") return 3.5e+7  *   std::pow(A, 2/3.) * CoLib::pow(energy, 5);
    else  if (M == "M3") return 16.e+0  *   std::pow(A, 2/3.) * CoLib::pow(energy, 7);
    else  if (M == "M4") return 4.5e-6  * CoLib::pow(A, 2   ) * CoLib::pow(energy, 9);
    else return -42.;
  }

  double WeisskopfLifetime() const { return exp(-Weisskopf())/log(2);}

  friend std::ostream& operator<<(std::ostream& out, Transition const & transition)
  {
    out << transition.m_label << " " << transition.energy << " " << nicer_bool(transition.parity) << " " 
    << transition.multipolarity() << " D=" << transition.conversion.coefficient;
    return out;
  }

  bool operator=(Transition const & other) const {return m_label  == other.m_label  ;}
  
  using Label = std::pair<size_t, size_t>;
  auto const & label() const {return m_label;}

  bool decayFrom(Level<A,Z> const & level) const {return m_Level_f->label() == level.label();}
  bool decayTo  (Level<A,Z> const & level) const {return m_Level_i->label() == level.label();}
  bool feed     (Level<A,Z> const & level) const {return m_Level_i->label() == level.label();}

  auto const & fromLevel()    const {return m_Level_i;}
  auto const & toLevel  ()    const {return m_Level_f;}
  auto const & initialLevel() const {return m_Level_i;}
  auto const & finalLevel  () const {return m_Level_f;}

private:

  Level<A,Z> const * m_Level_i = nullptr;       // Initial level
  Level<A,Z> const * m_Level_f = nullptr;       // Final level
  Label m_label;
  bool m_ok = true;
};

/**
 * @brief 
 * @details
 * First place all the levels, then process them, then create the transitions between them, then process them
 * @tparam A 
 * @tparam Z 
 */
template<int A, int Z>
class Nucleus
{
public:
  Nucleus() noexcept = default;
  Nucleus(std::string const & name)
  {m_name = name;}


  template <typename... Args>
  void addLevelFast(Args&&... args) {m_levels.emplace_back(m_levels.size(), std::forward<Args>(args)...);}
  
  void processLevels()
  {
    m_nodes.clear();
    for (auto & level : m_levels) m_nodes.emplace_back(&level);
  }
  
  template <typename... Args>
  void addLevel(Args&&... args) 
  {
    addLevelFast(std::forward<Args>(args)...);
    processLevels();
  }

  template <typename... Args>
  void addTransitionFast(int const & i, int const & f, Args&&... args) 
  {
    if (m_nodes.size() < size_cast(f)) {error("final level do not exist"); return;}
    m_transitions.emplace_back(&m_levels[i], &m_levels[f], std::forward<Args>(args)...); // Add a new transition from initial level to final level
  }

  void processTransitions()
  {
    for (auto & node : m_nodes) node.clearTransitions();
    for (auto & transition : m_transitions)
    {
      auto const & label = transition.label();
      m_nodes[label.first ].transitions_out.emplace_back(&transition); // Store the transition that decays out of the initial level
      m_nodes[label.second].transitions_in .emplace_back(&transition); // Store the transition that feeds the final level
    }
  }

  template <typename... Args>
  void addTransition(Args&&... args) 
  {
    addTransitionFast(std::forward<Args>(args)...);
    processTransitions();
  }

  struct Cascade
  {
    struct GatedTransition
    {
      double energy = 0.;
      double intensity = 0;
      double conversionCoeff = 0.;
      std::pair<size_t, size_t> label;
      GatedTransition(Transition<A, Z> const * transition) :
        energy(transition->energy),
        intensity(transition->branchingRatio),
        conversionCoeff(transition->conversion.coefficient),
        label(transition->label())
      {}
      friend std::ostream& operator<<(std::ostream& out, GatedTransition const & transition)
      {
        out << transition.label << " " << transition.energy << " " << transition.intensity << " " << transition.conversionCoeff;
        return out;
      }
    };

    std::vector<GatedTransition> m_transitions;

    void addTransition(Transition<A, Z> const * transition, double const & intensity = 1.)
    {
      for (auto & trans : m_transitions) 
      {
        if (trans.label == transition->label()) 
        {
          trans.intensity += intensity;
          return;
        }
      }
      m_transitions.emplace_back(transition);
      m_transitions.back().intensity *= intensity;
    }

    friend std::ostream& operator<<(std::ostream& out, Cascade const & cascade)
    {
      for (auto const & transition : cascade.m_transitions) out << transition << std::endl;
      return out;
    }
  };

  /**
   * @brief Get the cascade of transition going through the given energy
   * 
   * @param energy 
   * @param resolution 
   * @return Cascade 
   */
  Cascade gate(double const & energy, double const & resolution, double const & intensity = 1.)
  {
    Cascade cascade;

    for (auto const & transition : m_transitions) 
    {
      if (CoLib::gate(transition.energy, energy, resolution)) // Gating : transition energy match input "energy"
      {
        auto const & node = m_nodes[transition.fromLevel()->label()];

        using Path = std::vector<Transition<A, Z> *>;

        std::vector<Path>  feeding_paths;
        std::vector<Path> decaying_paths;

        // Backtracking the cascades feeding the level
        std::function<void(Node, int)> getFeeders = [&](Node const & node, int const & path_nb)
        {
          if (node.transitions_in.empty()) return;
          for (size_t in_it = 0; in_it < node.transitions_in.size(); ++in_it)
          {
            auto in = node.transitions_in[in_it];
            if (feeding_paths.size() <= in_it) feeding_paths.emplace_back();
            feeding_paths[path_nb].push_back(in);
            getFeeders(m_nodes[in->label().first], path_nb + in_it);
          }
        };
        getFeeders(node, 0);
        for (auto const & path : feeding_paths) 
        {
          print();
          for (auto const & trans : path) print(*trans);
        }

      //   // Follow the cascade decaying out of the level
      //   std::function<void(Node, double)> getFed = [&](Node const & node, double const & branchingRatio)
      //   {
      //     if (node.transitions_out.empty()) return;
      //     for (auto const & out : node.transitions_out)
      //     {
      //       cascade.addTransition(out, branchingRatio*intensity);
      //       getFed(m_nodes[out->label().second], out->branchingRatio);
      //     }
      //   };
      //   getFed(node, 1.);
      }
    }
    return cascade;
  }

  auto const & getLevel     (size_t const & i) const {return m_levels     [i];}
  auto const & getTransition(size_t const & i) const {return m_transitions[i];}
  auto const & getNodes     (size_t const & i) const {return m_nodes      [i];}

  auto const & getLevels      () const {return m_levels     ;}
  auto const & getTransitions () const {return m_transitions;}
  auto const & getNodes       () const {return m_nodes      ;}


private:
  struct Node
  {
    Node(Level<A,Z> const * _level) noexcept : level(_level) {}

    Level<A,Z> const * level = nullptr;
    using Transitions = std::vector<Transition<A,Z>*>;
    using Levels = std::vector<Level<A,Z> const *>;
    Transitions transitions_in;
    Transitions transitions_out;

    void clearTransitions()
    {
      transitions_in.clear();
      transitions_out.clear();
    }
    
    friend std::ostream& operator<<(std::ostream& out, Node const & node)
    {
      out << node.level->label() << " " << node.level->energy; 
      if (!node.transitions_in.empty()) 
      {
        out << " : in";
        for (auto const & trans_i : node.transitions_in) out << " " << trans_i->energy;
      }
      if (!node.transitions_out.empty()) 
      {
        out << " : out";
        for (auto const & trans_o : node.transitions_out) out << " " << trans_o->energy;
      }
      out << std::endl;
      return out;
    }
  };

  std::vector<Level<A,Z>>       m_levels;
  std::vector<Transition<A, Z>> m_transitions;
  std::vector<Node>             m_nodes;
  
  std::string m_name = "";
};

void Transitions()
{
  Nucleus<236, 92> U236("Uranium 236");

//   // Pour une session interactive, on peut faire une LUT avec tous les noyaux existant
//   // et la session simplement la lit

  U236.addLevelFast(1052, 4, false); // 0

  U236.addLevelFast(987, 2, false); // 1

  U236.addLevelFast(848, 5, false); // 2
  U236.addLevelFast(744, 3, false); // 3
  U236.addLevelFast(688, 1, false); // 4

  U236.addLevelFast(149, 4, true); // 5
  U236.addLevelFast(45 , 2, true); // 6
  U236.addLevelFast(0  , 0, true); // 7


  U236.processLevels();


  U236.addTransition(0, 1, 0.65);
  U236.addTransition(0, 2, 1.00);
  U236.addTransition(0, 3, 0.90);
  U236.addTransition(0, 5, 0.41);

  U236.addTransition(1, 3, 0.26);
  U236.addTransition(1, 4, 0.17);
  U236.addTransition(1, 6, 1.00);

  U236.addTransition(2, 3, 1.00);

  U236.addTransition(3, 4, 0.05);
  U236.addTransition(3, 5, 1.00);

  U236.addTransition(4, 5, 0.01);
  U236.addTransition(4, 6, 1.00);
  U236.addTransition(4, 7, 0.27);

  U236.addTransition(5, 6, 100);
  U236.addTransition(6, 7, 100);

  // print(U236.getLevels());
  std::cout << std::scientific << std::setprecision(1);
  for (auto const & transition : U236.getTransitions()) print(transition, transition.WeisskopfLifetime()/1.e-9, "ns");
  // print(U236.getNodes());
//   U236.gate(350, 2);
}
