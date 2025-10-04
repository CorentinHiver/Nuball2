#ifndef SPECTRAALINGATOR_HPP
#define SPECTRAALINGATOR_HPP

#define MAX_ORDER2

#include "../libRoot.hpp"

#include "SpectraCo.hpp"

/////////////
// VERTICE //
/////////////

class Vertice
{
public:
  Vertice() = default;

  Vertice(std::vector<double> const & coords) : 
    m_dim(coords.size()),
    m_coordinates(coords)
  {
  }

  Vertice(std::initializer_list<double> const & init_list) : 
    m_dim(init_list.size()),
    m_coordinates(init_list)
  {
  }

  Vertice(Vertice const & _vertice) : 
    m_dim(_vertice.m_dim),
    m_coordinates(_vertice.m_coordinates),
    m_value(_vertice.m_value)
  {
  }

  Vertice(Vertice && _vertice) : 
    m_dim(std::move(_vertice.m_dim)),
    m_coordinates(std::move(_vertice.m_coordinates)),
    m_value(std::move(_vertice.m_value))
  {
  }

  Vertice(size_t size) : 
    m_dim(size),
    m_coordinates(std::vector<double>(m_dim, 0))
  {
  }

  auto & operator[] (int const & i) {return m_coordinates[i];}
  auto const & operator[] (int const & i) const {return m_coordinates[i];}

  // Copy a vertice : checks the dimension consistencies

  void copy(Vertice const & other)
  {
    if (this -> m_dim != other.m_dim) Colib::throw_error("in Vertice::copy(Vertice const & _vertice) : dimension of _vertice different from that of the vertice");
    m_coordinates = other.m_coordinates;
    m_dim = other.m_dim;
    m_value = other.m_value;
  }
  void copy(Vertice && other)
  {
    if (this -> m_dim != other.m_dim) Colib::throw_error("in Vertice::copy(Vertice && _vertice) : dimension of _vertice different from that of the vertice");
    m_coordinates = std::move(other.m_coordinates);
    m_dim = std::move(other.m_dim);
    m_value = std::move(other.m_value);
    cleanMove(other);
  }
  void copy(std::vector<double> const & point)
  {
    if (point.size() != m_dim) Colib::throw_error("in Vertice::copy(std::vector<double> const & point) : dimension of the point different from that of the vertice");
    m_coordinates = point;
    m_value = NAN;
  }

  // Set the vertice : discards the current dimension to set a new one

  void set(Vertice const & other) 
  {
    m_coordinates = other.m_coordinates; 
    m_dim = other.m_dim;
    m_value = other.m_value;
  }
  void set(Vertice && other) 
  {
    m_coordinates = std::move(other.m_coordinates);
    m_dim = std::move(other.m_dim);
    m_value = std::move(other.m_value);
    cleanMove(other);
  }
  void set(std::vector<double> const & point) 
  {
    m_coordinates = point;
    m_dim = m_coordinates.size();
    m_value = NAN;
  }

  void cleanMove(Vertice & _vertice)
  {
    _vertice.m_coordinates.clear();
    _vertice.m_value = 0;
    _vertice.m_dim = 0;
  }

  Vertice& operator=(std::vector<double> const & point) {this->set(point); return *this;}
  Vertice& operator=(Vertice const & vertice) {this->set(vertice); return *this;}
  Vertice& operator=(Vertice && vertice) {this->set(std::move(vertice)); return *this;}

  void set(int const & coordinate_i, double const & coordinate) {m_coordinates[coordinate_i] = coordinate;}

  void fill(double const & value) {std::fill(m_coordinates.begin(), m_coordinates.end(), value);}
  void resize(size_t const & size) {if (m_dim!=size) {m_dim = size; m_coordinates.resize(m_dim);}}
  void resize(size_t const & size, double const & value) 
  {
    this->resize(size);
    this->fill(value);
  }

  auto const & get() const {return m_coordinates;}
  auto & get() {return m_coordinates;}

  auto const & get(int const & coordinate_i) const {return m_coordinates[coordinate_i];}
  auto & get(int const & coordinate_i) {return m_coordinates[coordinate_i];}

  auto & size() {return m_dim;}
  auto const & size() const {return m_dim;}

  auto begin() {return m_coordinates.begin();}
  auto end() {return m_coordinates.end();}

  auto begin() const {return m_coordinates.begin();}
  auto end() const {return m_coordinates.end();}


  Vertice operator+(Vertice const & other) const
  {
    if (other.size() != m_dim) Colib::throw_error("in Vertice::operator+(Vertice const & other) : other and vertice not the same size !!");
    std::vector<double> coords;
    for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i] + other[i]);
    return Vertice(coords);
  }

  Vertice operator-(Vertice const & other) const
  {
    if (other.size() != m_dim) 
    {
      Colib::throw_error("in Vertice::operator-(Vertice const & other) : other and vertice not the same size !!");
    }
    std::vector<double> coords;
    for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i] - other[i]);
    return Vertice(coords);
  }

  // Vectorial product : TBD !!!
  Vertice operator*(Vertice const & other) const
  {
    Colib::throw_error("operator Vertice*Vertice not implemented yet !!");
    // if (other.size() != m_dim) Colib::throw_error("in Vertice::operator+(Vertice const & other) : other and vertice not the same size !!");
    std::vector<double> coords = other.get();
    // for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i]*other[i]);
    // return Vertice(coords);
    return coords;
  }

  Vertice operator*(double const & constant) const
  {
    std::vector<double> coords = this->get();
    for (auto & coord : coords) coord*=constant;
    return coords;
  }

  Vertice& operator*=(double const & constant)
  {
    for (auto & coord : m_coordinates) coord*=constant;
    return *this;
  }

  void setValue(double const & _value) {m_value = _value;}
  auto const & getValue() const {return m_value;}
  auto         getValue()       {return m_value;}

  auto const & evaluate(std::function<double(const Vertice&)> objective_function)
  {
    m_value = objective_function(*this);
    return m_value;
  }

  bool operator<(Vertice const & other) const {return m_value<other.m_value;}
  bool operator>(Vertice const & other) const {return m_value>other.m_value;}
  bool operator<=(Vertice const & other) const {return m_value<=other.m_value;}
  bool operator>=(Vertice const & other) const {return m_value>=other.m_value;}

private:
  size_t m_dim = 0;
  std::vector<double> m_coordinates;
  double m_value = NAN;
};

std::ostream& operator<<(std::ostream& out, Vertice const & vertice)
{
  out << "coordinates : " << vertice.get();
  if (!std::isnan(vertice.getValue())) out << " value : " << vertice.getValue();
  return out;
}

Vertice operator*(double const & constant, Vertice const & vertice)
{
  std::vector<double> newVertice = vertice.get();
  for (auto & coord : newVertice) coord*=constant;
  return newVertice;
}

using Vertices = std::vector<Vertice>;

/////////////
// SIMPLEX //
/////////////

class Simplex
{
public:
  Simplex(size_t const & dim) : 
    m_size(dim+1),
    m_dim(dim)
  {
    for (size_t i = 0; i<m_size; i++) m_vertices.push_back(Vertice(dim));
  }

  /**
   * @brief Construct a simplex based on a vertice.
   * @details The simplex is built based on the dimension of the given vertex, 
   * which is then duplicated
  */
  Simplex(Vertice const & vertice) : 
    m_size(vertice.size()+1),
    m_dim(vertice.size())
  {
    print("taille :", vertice.size());
    print(vertice);
    for (size_t i = 0; i<m_size; i++) m_vertices.push_back(vertice);
  }

  Simplex(Vertices const & vertices) : 
    m_vertices(vertices), 
    m_size(vertices.size()),
    m_dim(m_size-1)
  {
    for (auto const & vertex : vertices) if (vertex.size() != this->m_dim) 
      Colib::throw_error("in Simplex::Simplex(std::vector<Vertice> const & vertices) : dimension conflict of at least one vertex (simplex must have dim+1 vertices)");
  }

  Simplex(Simplex const & other) : 
    m_vertices(other.m_vertices),
    m_size(other.m_size),
    m_dim(other.m_dim),
    m_centroid(other.m_centroid)
  {
  }

  Simplex(Simplex && other) : 
    m_vertices(std::move(other.m_vertices)),
    m_size(std::move(other.m_size)),
    m_dim(std::move(other.m_dim)),
    m_centroid(std::move(other.m_centroid))
  {
  }

  Simplex& operator=(Simplex const & other)
  {
    m_vertices = other.m_vertices;
    m_size = other.m_size;
    m_dim = other.m_dim;
    m_centroid = other.m_centroid;
    return *this;
  }

  Simplex& operator=(Simplex && other)
  {
    m_vertices = std::move(other.m_vertices);
    m_size = std::move(other.m_size);
    m_dim = std::move(other.m_dim);
    m_centroid = std::move(other.m_centroid);
    return *this;
  }

  Simplex& operator=(Vertices const & vertices)
  {
    m_vertices = vertices;
    m_size = vertices.size();
    m_dim = m_size-1;
    return *this;
  }

  void setVertice(int const & bin, Vertice const & vertice) {if (size_cast(bin)<m_size) m_vertices[bin] = vertice;}
  void setVertice(int const & bin, Vertice && vertice) {if (size_cast(bin)<m_size) m_vertices[bin] = std::move(vertice);}

  auto centroid(size_t const & n_best)
  {
    if (n_best>m_size) Colib::throw_error("in Simplex(size_t n_best) : n_best is higher than the size of the simplex");
    if (n_best == 0) return (m_centroid = m_vertices[0]); // 1D case
    debug("centroid calculation : ");
    for (size_t vertice_i = 0; vertice_i<n_best; ++vertice_i) debug(vertice_i, m_vertices[vertice_i]);

    for (auto & coord : m_centroid) coord = 0.0; // Initialise the centroid
    for (size_t coord_i = 0; coord_i<m_centroid.size(); coord_i++)
    {
      for (size_t vertice_i = 0; vertice_i<n_best; ++vertice_i) 
      {
        m_centroid[coord_i] += m_vertices[vertice_i][coord_i];
      }
    }
    debug("m_centroid before", m_centroid);
    auto const & points = double_cast(n_best);
    for (auto & coord : m_centroid) {coord /= points;}
    return m_centroid;
  }
  auto centroid() {return centroid(m_size);}

  void evaluate(std::function<double(const Vertice&)> objective_function)
  {
    for (auto & vertice : m_vertices) vertice.evaluate(objective_function);
  }

  auto const & getCentroid() const {return m_centroid;}
  
  // Usual methods and operators :

  auto & operator[] (int const & i) {return m_vertices[i];}
  auto const & operator[] (int const & i) const {return m_vertices[i];}

  auto begin() {return m_vertices.begin();}
  auto end() {return m_vertices.end();}

  auto begin() const {return m_vertices.begin();}
  auto end() const {return m_vertices.end();}

  auto const & size() const {return m_size;}
  auto & size() {return m_size;}

  auto const & dim() const {return m_dim;}
  auto & dim() {return m_dim;}

private:
  Vertices m_vertices;
  size_t m_size = 0;
  size_t m_dim = 0;
  Vertice m_centroid = Vertice(m_dim);
  std::vector<double> m_angles;
};

std::ostream& operator<<(std::ostream& out, Simplex const & simplex)
{
  out << simplex.size() << " points : ";
  for (auto const & vertice : simplex) out << std::endl << vertice;
  return out;
}

////////////////////////
// Objective function //
////////////////////////

class ObjectiveFunction
{
public:
  ObjectiveFunction() noexcept = default;
  virtual double evaluate(std::vector<double> const & coefficients) const = 0;
};

class TestObjectiveFunction : public ObjectiveFunction
{
public:
  TestObjectiveFunction() noexcept = default;
  double evaluate(std::vector<double> const & coefficients) const override
  {
    double ret = 0.0;
    for (auto const & coeff : coefficients) ret += coeff*coeff - 2*coeff + 1;
    return ret;
  }
};

class ObjectiveFunctionChi2 : public ObjectiveFunction
{
public:
  ObjectiveFunctionChi2() noexcept = default;
  ObjectiveFunctionChi2(SpectraCo* spectra_ref, SpectraCo* spectra_test) : m_spectra_ref(spectra_ref), m_spectra_test(spectra_test) {}
  void setRef(SpectraCo* spectra_ref) {m_spectra_ref = spectra_ref;}
  void setTest(SpectraCo* spectra_test) {m_spectra_test = spectra_test;}

  double evaluate(std::vector<double> const & coefficients) const override
  {
    auto const & spectra_ref = *m_spectra_ref; // Simple aliasing
    SpectraCo spectra_test (*m_spectra_test); // Copy the spectra in order to calibrate it afterwards
    auto const & nb_bins = spectra_ref.size();

    // Calibrating the X axis of the test spectra then scale the Y axis :
    if (coefficients.size() > 1) spectra_test.calibrateAndScale(coefficients);

    if (nb_bins != spectra_test.size()) {print ("in ObjectiveFunctionChi2::evaluate(): both spectra don't have the same size"); return 0;}

    double sum_errors_squared = 0.0;

    auto spectra_diff = spectra_ref - spectra_test;

    for (int bin = 0; bin<nb_bins; bin++) 
    {
      auto const & max_bin = std::max(spectra_ref[bin], spectra_test[bin]);
      if (max_bin == 0) continue;

      // // Variance of the bin (here is taken the mean between both ref and test spectra):
      double const & weight = 1/max_bin; // V = sigma² = 1/N,

      // // Calculate the error for this bin :
      // double const & error = spectra_ref[bin] - spectra_test[bin];

      // Add the error to the total squared error of the spectra :
      sum_errors_squared += spectra_diff[bin]*spectra_diff[bin]*weight;
    }
    return sum_errors_squared/nb_bins;
  }

  
private:
  SpectraCo* m_spectra_ref = nullptr;
  SpectraCo* m_spectra_test = nullptr;
};

class Minimisator
{
public:
  Minimisator() = default;
  
  struct NelderMeadParameters
  {
    void adjustToDimension(double const & n)
    { // Gao + Han, Comput Optim Appl (2012) 51:259-277
      print("adjust nelder-mead coefficients to the dimension", n);
      a_extend  = 1+2/n;
      a_contract= 0.75-0.5/n;
      a_shrink  = 1-1/n;
      adjusted = true;
      Print();
    }

    void Print()
    {
      print("a_reflect :", a_reflect, "a_extend :", a_extend, "a_contract :", a_contract, "a_shrink :", a_shrink, "a_expand :", a_expand, 
            "adjusted :", (adjusted) ? "yes" : "no");
    }

    double a_reflect = 1;
    double a_extend  = 2;
    double a_contract = 0.5;
    double a_shrink = 0.5;
    double a_expand = 1.3; // Unused in standard method (without adjustement)
    bool adjusted = false;
  } nmParam;

  void nelderMead(ObjectiveFunction const & function)
  {
    bool verbose = false;

    // Alias the call to the objective function on a vertice :
    auto computeFunction = [&](Vertice const & vertice) {
      return function.evaluate(vertice.get());
    };

    // -- Create the lambdas to be used in the nelder-mead algorithm -- //

    // Reflect the worst point through the average of the best n vertices
    auto reflectVertice = [&](Vertice const & centroid, Vertice const & worst_vertice) {
      debug();
      debug("centroid", centroid, "worst_vertice",worst_vertice,"   diff", centroid - worst_vertice);
      return centroid + nmParam.a_reflect * (centroid - worst_vertice);
    };

    // Extend the worst point further past the average of the best n vertices
    auto extendVertice = [&](Vertice const & centroid, Vertice const & reflection) {
      return centroid + nmParam.a_extend * (reflection-centroid);
    };

    // Contract inside the worst point to a point between the worst point and the reflexion point
    auto contractVerticeIn = [&](Vertice const & centroid, Vertice const & reflection) {
      return centroid - nmParam.a_contract * (centroid - reflection);
    };

    // Contract outside the worst point to a point between the worst point and the reflexion point
    auto contractVerticeOut = [&](Vertice const & centroid, Vertice const & reflection) {
      return centroid + nmParam.a_contract * (centroid - reflection);
    };

    // Shrinks the simplex towards the best point
    auto shrinkSimplex = [&](Simplex & simplex) {
        for (size_t point_i = 1; point_i<simplex.size(); point_i++){
          simplex[point_i] = simplex[0] + nmParam.a_shrink * (simplex[point_i] - simplex[0]);
        };
    };

    // Initialise the simplex :
    Simplex simplex(m_initial_vertice);

    auto const & n = simplex.dim();// The dimension of the problem
    if (n==0) Colib::throw_error("in Minimisator::nelderMead() : The problem is null dimensioned !!");
    else if (n==1) Colib::throw_error("in Minimisator::nelderMead() : The problem is 1 dimension, currently not supported !!");

    // nmParam.adjustToDimension(simplex.dim());

    if (simplex.dim() != m_steps.size()) 
      Colib::throw_error("in Minimisator::nelderMead(), the dimension of the vertices do not match the dimensions of the initial steps");

    // Create a rectangle triangle with :
    for (size_t vertice_i = 1; vertice_i<simplex.size(); ++vertice_i)
    {
      auto const & coord_i = vertice_i-1;
      simplex[vertice_i][coord_i]+=m_steps[coord_i];
    }

    // Evaluate the result of the objective function for each vertice.
    simplex.evaluate(computeFunction);
    
    if (verbose) print();
    if (verbose) print("initial guess : ", m_initial_vertice);
    if (verbose) print("first simplex : ", simplex);
    if (verbose) print();
    g_simplex_history.push_back(simplex);

    // Sorts the simplex based on the evaluated value of the vertices.
    std::sort(simplex.begin(), simplex.end());
    Vertice newVertice(n);


    int loop_i = 0;
    while(loop_i++<g_nb_rounds)
    {
      if (verbose) debug();
      if (verbose) debug("========================");
      if (verbose) debug("Round", loop_i);
      // if (verbose) debug(simplex);

      // Calculate the CENTROID 
      auto centroid (simplex.centroid(n));
      centroid.evaluate(computeFunction);      if (verbose) debug("centroid", centroid);

      // REFLECT the worst point through the average of the best n vertices
      auto reflection (reflectVertice(centroid, simplex[n]));
      reflection.evaluate(computeFunction);
      if (verbose) debug("reflection", reflection);

      bool hasNewVertice = false;

      // If the reflection is worst than the best point but better than the second to worse point
      if (simplex[0] <= reflection && reflection < simplex[n-1]) {
        newVertice.set(reflection);
        if (verbose) debug("new vertice is reflection point", reflection);
      }

      // If the reflection is better than the best point
      if (!hasNewVertice && reflection < simplex[0])
      {
        // EXTEND the worst point further past the average of the best n vertices
        auto extension (extendVertice(centroid, reflection));
        extension.evaluate(computeFunction);
        if (extension < reflection) {hasNewVertice = true; newVertice.copy(extension) ; if (verbose) {debug("new vertice is extension point", extension)  ;};}
        else                        {hasNewVertice = true; newVertice.copy(reflection); if (verbose) {debug("new vertice is reflection point", reflection);};}
      }

      // If the reflection is worse than second to worst but better than worst point
      if (!hasNewVertice && reflection < simplex[n])
      {
        // CONTRACT the point. Two kinds : inside or outside contraction.
        auto outContraction(contractVerticeOut(centroid, reflection));
        outContraction.evaluate(computeFunction);
        auto inContraction(contractVerticeIn(centroid, reflection));
        inContraction.evaluate(computeFunction);

        if (outContraction < inContraction)
        {
          if (outContraction < reflection) 
          {
            hasNewVertice = true; 
            newVertice.copy(outContraction); if (verbose) debug("new vertice is outContraction point", outContraction);
          }
        }
        else if (inContraction < reflection) 
        {
          hasNewVertice = true; 
          newVertice.copy(inContraction); if (verbose) debug("new vertice is inContraction point", inContraction);
        }
      }

      if (hasNewVertice && verbose) print(newVertice);

      if (hasNewVertice) {simplex[n].copy(newVertice); simplex.evaluate(computeFunction);}
      else
      {
        // If no point has been accepted, the simplex is shrinks the simplex onto itself
        if (verbose) debug("_________________");
        if (verbose) debug("shrinking ...");
        if (verbose) debug(simplex);
        shrinkSimplex(simplex);
        simplex.evaluate(computeFunction);
        if (verbose) debug(simplex);
        if (verbose) debug("_________________");
      }
      
      if (nmParam.adjusted)
      {
        // ELONGATE

      }


      // CONVERGENCE checks
      // First, sorts the simplex :
      std::sort(simplex.begin(), simplex.end());
      g_minimums.push_back(simplex[0].getValue());

      // if ((simplex[n+1].getValue()-simplex[0].getValue())/(simplex[0].getValue()+1.e-9) < tolerance) break;

      if (verbose) debug(simplex);
      // Check that each angle of the hyper-triangle have is large enough :
      // simplex.calculateAngles();
      g_minima_history.push_back(simplex[0]);
      g_simplex_history.push_back(simplex);
    }
    if (verbose) print();
    if (verbose) print(simplex);
    print(loop_i, "iterations");
    m_minimum = simplex[0];
  }

  auto const & getMinimum() const {return m_minimum;}
  void setInitialGuess(Vertice const & init_vertice) 
  {
    g_order = init_vertice.size();
    m_initial_vertice.set(init_vertice);
  }

  void setInitialSteps(Vertice const & steps)
  {
    m_steps.set(steps);
  }

  /// @brief C'est peut-être une fausse piste ...
  /// @param func 
  void simpleGrad(ObjectiveFunction & func)
  {
    // m_grad_vertices.resize(g_order, m_initial_vertice);
    // m_weights.resize(g_order, 1.0);
    // for (int coeff_i = 0; coeff_i<g_order; ++coeff_i)
    // {
    //   Vertice test_vertice(m_initial_vertice);
    //   m_grad_vertices[coeff_i].resize(g_order);
    //   for (int step = 0; step<m_nb_steps_grad; step++)
    //   {
    //     auto const & before = func.evaluate(m_grad_vertices[coeff_i].get());
    func.evaluate(m_grad_vertices[0].get()); // Dummy line
    //     m_grad_vertices[coeff_i].set(coeff_i, m_grad_vertices[coeff_i][coeff_i]+m_weights[coeff_i]*step);
    //     auto const & after = func.evaluate(m_grad_vertices[coeff_i].get());
    //     print(m_grad_vertices[coeff_i].get(), before, after, before-after);
    //     if (before == after) m_weights[coeff_i]*=10;
    //     else m_weights[coeff_i]/= after - before;
    //     Colib::pause(" ");
    //   }
    // }
    // print(m_weights);
  }

  double tolerance = 1.e-3;
  static int g_order;
  static int g_nb_rounds;// Provisoire
  std::vector<double> g_minimums;
  Vertices g_minima_history;
  std::vector<Simplex> g_simplex_history;

private:
  int m_nb_steps_grad = 10; // Should be an even number
  Vertice m_minimum = 0.0;
  Vertice m_weights;
  Vertice m_steps;
  Vertice m_initial_vertice;
  Vertices m_grad_vertices;
};

int Minimisator::g_order = 3; // By default, first order calibration + Y axis scaling
int Minimisator::g_nb_rounds = 5; 



/// @deprecated
class SpectraAlignator
{
public:
  SpectraAlignator() noexcept = default;

  SpectraAlignator(TH1* ref) :
    m_ref_spectra(ref)
  {
  }

  /// @deprecated
  void newAlignement(TH1* _spectra, TH1* spectra_output)
  {
    print(spectra_output); // dummy line
    SpectraCo test_spectra(_spectra);
    ObjectiveFunctionChi2 func(&m_ref_spectra, &test_spectra);
    Minimisator min;
    min.setInitialGuess({0, 1, 1});
    min.setInitialSteps({10, 1, 0.5});
    // min.simpleGrad(func);
    min.nelderMead(func);
    print(min.getMinimum());
  }


  Recalibration const & alignSpectra(TH1* _spectra, TH1* spectra_output, int const & degreesOfFreedom = 3, double const & energyThreshold = 0)
  {
    print(_spectra, spectra_output, degreesOfFreedom, energyThreshold); // dummy line
    // m_spectra.emplace_back(_spectra);
    // auto & spectra = m_spectra.back();
    // m_recals.emplace_back(Recalibration());
    // auto & recal = m_recals.back();
    // m_minimisator.emplace_back(&recal);
    // auto & minimisator = m_minimisator.back();
    // minimisator.setDegreesOfFreedom(degreesOfFreedom);
    // if (energyThreshold>0 && energyThreshold<_spectra->GetXaxis()->GetXmax()) minimisator.setThreshold(_spectra->FindBin(energyThreshold));

    // // std::vector<std::vector<std::vector<double>>> minimisation_space;

    // double const & C_dumb = 1;
    // double const & C_min = C_dumb - 0.3;
    // double const & C_max = C_dumb + 0.3;
    // double const & C_0 = (C_min+C_max)/2;
    // double const & C_steps = (C_max-C_min)/m_nb_iterations;
    
    // double const & a0_min = -1;
    // double const & a0_max = 1;
    // double const & a0_0 = (a0_min+a0_max)/2;
    // double const & a0_steps = (a0_max-a0_min)/m_nb_iterations;
    
    // double const & a1_min = 0.98;
    // double const & a1_max = 1.005;
    // double const & a1_0 = (a1_min+a1_max)/2;
    // double const & a1_steps = (a1_max-a1_min)/m_nb_iterations;

    // double const & a2_min = -0.00001;
    // double const & a2_max = 0.00001;
    // double const & a2_0 = (a2_min+a2_max)/2;
    // double const & a2_steps = (a2_max-a2_min)/m_nb_iterations;

    // // Initialise at default values :
    // auto min_C = C_0;
    // auto min_a0 = a0_0;
    // auto min_a1 = a1_0;
    // auto min_a2 = a2_0;

    // minimisator.setC(min_C);
    // minimisator->a0(min_a0);
    // minimisator->a1(min_a1);
    // if (degreesOfFreedom>3) recal.a2(min_a2);

    // // auto min_chi2 = minimisator.chi2(m_ref_spectra, spectra);
    // auto min_chi2 = m_ref_spectra.chi2(spectra);
    // if (degreesOfFreedom>3) print(min_chi2, min_C, min_a0, min_a1, min_a2);
    // else print(min_chi2, min_C, min_a0, min_a1);
    
    // m_chi2_spectra = new TH3F("chi2", "chi2;a0;a1;C", m_nb_iterations,a0_min,a0_max, m_nb_iterations,a1_min,a1_max, m_nb_iterations,C_min,C_max);

    // int tries = 0;
    // // Use brute force calculation :
    // if (m_brute_force)
    // {
    //   for (int C = 0; C<m_nb_iterations; C++)
    //   {
    //     for (int a0 = 0; a0<m_nb_iterations; a0++)
    //     {
    //       for (int a1 = 0; a1<m_nb_iterations; a1++)
    //       {
    //         if (degreesOfFreedom>3) for (int a2 = 0; a2<m_nb_iterations; ++a2)
    //         {
    //           tries++;
    //           double const & C_value = C_min+C*C_steps;
    //           double const & a0_value = a0_min+a0*a0_steps;
    //           double const & a1_value = a1_min+a1*a1_steps;
    //           double const & a2_value = a2_min+a2*a2_steps;

    //           minimisator.setC(C_value);
    //           double const & _chi2 = minimisator.chi2(m_ref_spectra, spectra, Vertice({a0_value, a1_value, a2_value}));

    //           m_chi2_spectra->Fill(min_a0, min_a1, min_C);
    //           if (_chi2 < min_chi2)
    //           {
    //             min_C  = C_value ;
    //             min_a0 = a0_value;
    //             min_a1 = a1_value;
    //             min_a2 = a2_value;
    //             min_chi2 = _chi2;
    //             print(min_chi2, min_C, min_a0, min_a1, min_a2);
    //           } 
    //         }
    //         else
    //         {
    //           tries++;
    //           double const & C_value = C_min+C*C_steps;
    //           // print(C_value, C_min, C,C_steps);
    //           double const & a0_value = a0_min+a0*a0_steps;
    //           double const & a1_value = a1_min+a1*a1_steps;
    //           minimisator.setC(C_value);
    //           minimisator->a0(a0_value);
    //           minimisator->a1(a1_value);
    //           // print(recal);
    //           // double const & _chi2 = minimisator.chi2_second_derivative(m_ref_spectra, spectra, 20);
    //           // double const & _chi2 = minimisator.chi2(m_ref_spectra, spectra);
    //           SpectraCo new_spectra(spectra*C_value);
    //           print(new_spectra[200]);
    //           new_spectra.calibrateX(a1_value, a0_value);
    //           print(new_spectra[200]);
    //           double const & _chi2 = m_ref_spectra.chi2(new_spectra);
    //           // print(a0_value, a1_value, C_value);
    //           // print(a0_value, a1_value, C_value, _chi2);
    //           m_chi2_spectra->SetBinContent(a0, a1, C, _chi2);
    //           //print(_chi2, C_value, a0_value, a1_value);

    //           // if ((int)(C_value+a0_value+a1_value)%100 == 0) print(_chi2, C_value, a0_value, a1_value);

    //           if (_chi2 < min_chi2)
    //           {
    //             min_C  = C_value ;
    //             min_a0 = a0_value;
    //             min_a1 = a1_value;
    //             min_chi2 = _chi2;
    //             print("best khi2 :", min_chi2, "vertice : ", min_C, min_a0, min_a1);
    //           }
    //         }
    //       }
    //     }
    //   }
    // }
    // // Use minimisator
    // else
    // {
    //   minimisator.setMinC(C_min);
    //   minimisator.setMaxC(C_max);
    //   minimisator.setMina0(a0_min);
    //   minimisator.setMaxa0(a0_max);
    //   minimisator.setMina1(a1_min);
    //   minimisator.setMaxa1(a1_max);
    //   minimisator.setMina2(a2_min);
    //   minimisator.setMaxa2(a2_max);

    //   minimisator.minimise(m_ref_spectra, spectra);

    //   print(recal);
    // }

    // // Set the previously found minimum values :
    // if (degreesOfFreedom>3) print(min_chi2, min_C, min_a0, min_a1, min_a2);
    // else print(min_chi2, min_C, min_a0, min_a1);
    // recal.a0(min_a0);
    // recal.a1(min_a1);
    // if (degreesOfFreedom>3) recal.a2(min_a2);

    // print("final recal :", recal);
    // spectra.calibrateX(min_a1, min_a0);
    // // spectra.recalibrate(recal);
    // if (spectra_output) delete spectra_output;
    // spectra_output = new TH1F(
    //   (spectra.name()+"_recal").c_str(), 
    //   (spectra.name()+"_recal").c_str(), 
    //   spectra.size(), spectra.minX(), spectra.maxX());
    // for (int bin = 0; bin<spectra.size(); bin++) spectra_output->SetBinContent(bin, spectra[bin]);
    return m_recals.back();
  }

  void setIterations(int const & iterations) {m_nb_iterations = iterations;}

  void writeChi2Spectra(TFile* file)
  {
    file->cd();
    m_chi2_spectra->Write();
  }

  void setBruteForce(bool const & brute_force = true) {m_brute_force = brute_force;}

private:
  SpectraCo m_ref_spectra;

  std::vector<SpectraCo> m_spectra;
  // std::vector<OldMinimisator> m_minimisator;
  std::vector<Recalibration> m_recals;
  std::vector<double> m_chi2s;

  TH2F* surface_chi2 = nullptr;

  int m_nb_iterations = 20;
  bool m_brute_force = false;
  TH3F* m_chi2_spectra = nullptr;
  int m_spectra_threshold = 0;
};


// class OldMinimisator
// {
// public:
//   OldMinimisator(Recalibration * recal) : m_recal(recal)
//   {
    
//   }

//   void setC(double const & _C) {m_C = _C;}

//   double chi2 (SpectraCo const & ref_spectra, SpectraCo const & spectra, Vertice const & vertice)
//   {
//     // To generalise here
//     m_C = vertice[0];
//     for (size_t param = 1; param<vertice.size(); param++) m_recal->setParameter(param, vertice[param]);
//     return chi2(ref_spectra, spectra);
//   }

//   double chi2 (SpectraCo const & ref_spectra, SpectraCo const & spectra)
//   {
//     double sum_errors_squared = 0.0;
//     // print(spectra.size(), m_spectra_threshold);
//     int const & nb_bins_studied = spectra.size()-0;
//     for (int bin = 0; bin<spectra.size(); bin++) if (spectra[bin]>0)
//     {
//       // Calculate new bin value :
//       double const & new_bin = m_recal->calculate(bin);
//       debug(*m_recal, new_bin, bin);

//       // Do a linear interpolation to get the value of the recalibrated bin :
//       double const & new_value = spectra.interpolate(new_bin);

//       // Calculate the error for this bin :
//       double const & error = ref_spectra[bin] - m_C*new_value;

//       // Variance of the bin :
//       double const & weight = 1/spectra[bin]; // V = sigma² = 1/N

//       // Add the error to the total squared error of the spectra :
//       sum_errors_squared += error*error*weight;

//       // print(spectra[bin], weight, new_bin, new_value, error, sum_errors_squared);
//     }
//     // print(sum_errors_squared/(nb_bins_studied-m_nb_freedom_degrees));
//     // Colib::pause();
//     return sum_errors_squared/(nb_bins_studied-m_nb_freedom_degrees);
//   }

//   double chi2_second_derivative(SpectraCo & ref_spectra, SpectraCo & test_spectra, int const & smooth)
//   {
//     if (!ref_spectra.derivative2()) ref_spectra.derivate2(smooth);
//     auto const & ref_derder = *ref_spectra.derivative2();
//     double sum_errors_squared = 0.0;
//     // print(test_spectra.size(), m_spectra_threshold);
//     // int const & nb_bins_studied = test_spectra.size()-m_spectra_threshold;
//     // for (int bin = m_spectra_threshold; bin<test_spectra.size(); bin++) if (test_spectra[bin]>0)
//     if(!test_spectra.derivative2()) test_spectra.derivate2(smooth);
//     SpectraCo derivative2_spectra(*(test_spectra.derivative2()), *m_recal);
//     for (int bin = 0; bin<test_spectra.size(); bin++) if (test_spectra[bin]>0)
//     {
//       // Variance of the bin of the original spectra :
//       double const & weight = 1/abs(test_spectra[bin]);

//       // Calculate the error for this bin :
//       double const & error = ref_derder[bin] - m_C*derivative2_spectra[bin];

//       // Add it to the total squared error of the test_spectra :
//       sum_errors_squared += error*error*weight;
      
//     }
//     return sum_errors_squared/(test_spectra.size()-m_nb_freedom_degrees);
//   }

//   void setDegreesOfFreedom(int const & degrees) {m_nb_freedom_degrees = degrees;}
//   auto const & getDegreesOfFreedom() const {return m_nb_freedom_degrees;}

//   void setMinC (double const & _minC) {m_minC = _minC;}
//   void setMaxC (double const & _maxC) {m_maxC = _maxC;}
//   void setMina0 (double const & _mina0) {m_mina0 = _mina0;}
//   void setMaxa0 (double const & _maxa0) {m_maxa0 = _maxa0;}
//   void setMina1 (double const & _mina1) {m_mina1 = _mina1;}
//   void setMaxa1 (double const & _maxa1) {m_maxa1 = _maxa1;}
//   void setMina2 (double const & _mina2) {m_mina2 = _mina2;}
//   void setMaxa2 (double const & _maxa2) {m_maxa2 = _maxa2;}
//   void setMinaSqrt (double const & _minaSqrt) {m_minaSqrt = _minaSqrt;}
//   void setMaxaSqrt (double const & _maxaSqrt) {m_maxaSqrt = _maxaSqrt;}

//   Vertice minimise(SpectraCo const & ref_spectra, SpectraCo const & spectra, std::string const & method = "nelder-mead")
//   {
//     if (method == "nelder-mead")
//     {
//       // To generalise here
//       auto const & dumb_C = (m_maxC+m_minC)/2;
//       auto const & dumb_a0 = (m_maxa0+m_mina0)/2;
//       auto const & dumb_a1 = (m_maxa1+m_mina1)/2;
//       // auto const & dumb_a2 = (m_maxa2-m_mina2)/2;
//       Simplex simplex ({
//         Vertice({dumb_C, dumb_a0, dumb_a1}),
//         Vertice({m_minC, m_mina0, m_mina1}),
//         Vertice({m_maxC, m_maxa0, m_maxa1}),
//         Vertice({dumb_C, m_maxa0, m_maxa1})
//       });

//       print("first simplex : ");
//       print(simplex);

//       double last_chi2 = 0; // First dumb chi2
//       int loop_i = 0;

//       while(loop_i<50)
//       {
//         // 1.a Calculate the chi2 for each vertice of the simplex :
//         std::vector<double> evaluated_values;
//         for (auto const & vertice : simplex) evaluated_values.push_back(this -> chi2(ref_spectra, spectra, vertice));

//         // print("");
//         // print("evaluated_values");
//         // print(evaluated_values);

//         auto const & dim = evaluated_values.size()-1;

//         // 1.b Sorts the evaluated_values
//         std::vector<double> ordered_chi2s;
//         std::vector<int> ordered_indexes;
//         bubble_sort(evaluated_values, ordered_indexes);
//         for (auto const & index : ordered_indexes) ordered_chi2s.push_back(evaluated_values[index]);

//         // Calculate the centroid 
//         Vertice centroid (simplex.centroid(ordered_indexes)); // centroid

//         // 2.a Compute the reflexion point : centroid + alpha * (centroid-worst_vertice)
//         Vertice reflection (centroid + nmParam.alpha*(centroid - simplex[ordered_indexes.back()]));
//         // 2.b Compute its chi2
//         auto eval_reflection = chi2(ref_spectra, spectra, reflection);
//         print("eval_reflection", eval_reflection);
        
//         if (eval_reflection < ordered_chi2s[0])
//         { // If chi2_1>eval_reflection
//           // 3.a Compute the expansion point : centroid + beta * (reflection-centroid)
//           Vertice expansion(centroid + nmParam.beta*(reflection-centroid));
//           // 3.b Compute its chi2
//           auto eval_expansion = chi2(ref_spectra, spectra, expansion);
//           print("eval_expansion", eval_expansion);
//           if (eval_expansion < eval_reflection) simplex[ordered_indexes.back()] = expansion;
//           else simplex[ordered_indexes.back()] = reflection;
//         }
//         else if (eval_reflection < ordered_chi2s[dim])
//         {
//           simplex[ordered_indexes.back()] = reflection;
//         }
//         else if (eval_reflection < ordered_chi2s.back())
//         {
//           // 4.a Compute the outside contraction point : centroid + beta * (reflection-centroid)
//           Vertice outsideContraction(centroid + nmParam.gamma*(reflection-centroid));
//           // 4.b Compute its chi2
//           auto eval_outside_contraction = chi2(ref_spectra, spectra, outsideContraction);
//           print("eval_outside_contraction", eval_outside_contraction);
//           if (eval_outside_contraction < eval_reflection) simplex[ordered_indexes.back()] = outsideContraction;
//         }
//         else if (ordered_chi2s[dim] < eval_reflection)
//         {
//           // 5.a Compute the inside contraction point : centroid + beta * (reflection-centroid)
//           Vertice insideContraction(centroid - nmParam.gamma*(reflection-centroid));
//           // 5.b Compute its chi2
//           auto eval_inside_contraction = chi2(ref_spectra, spectra, insideContraction);
//         print("eval_inside_contraction", eval_inside_contraction);
//           if (eval_inside_contraction < eval_reflection) simplex[ordered_indexes.back()] = insideContraction;
//         }

//         // 6 Shrinks the simplex :
//         Simplex tempSimplex(simplex);
//         tempSimplex[0] = simplex[ordered_indexes[0]]; // this is the best point
//         for (size_t point_i = 1; point_i<dim+1; point_i++)
//         {
//           tempSimplex[point_i] = tempSimplex[0] + nmParam.delta * (simplex[ordered_indexes[point_i]] - tempSimplex[0]);
//         }

//         simplex = tempSimplex;
//         print("second simplex : ");
//         print(simplex);

//         double const & min_chi2 = chi2(ref_spectra, spectra, simplex[0]);
//         print(loop_i, min_chi2, last_chi2, abs((last_chi2-min_chi2)/last_chi2));

//         // Stop condition :
//         // if ( loop_i>10 && abs((last_chi2-min_chi2)/last_chi2) < 0.001) break;
//         last_chi2 = min_chi2;
//         loop_i++;
//       }
//       print(loop_i);
//       print(simplex);
//       return simplex[0];
//     }
//     else return Vertice({});
//   }

//   auto operator->() {return m_recal;}

//   void setThreshold(int const & bin) {m_spectra_threshold = bin;}

// private:
//   Recalibration * m_recal = nullptr;

//   // For the brute force :
//   double m_C = 0.0; 

//   // For the minimisation techniques :
//   double m_minC = 0.0;
//   double m_maxC = 0.0;
//   double m_mina0 = 0.0;
//   double m_maxa0 = 0.0;
//   double m_mina1 = 0.0;
//   double m_maxa1 = 0.0;
//   double m_mina2 = 0.0;
//   double m_maxa2 = 0.0;
//   double m_minaSqrt = 0.0;
//   double m_maxaSqrt = 0.0;

//   struct NelderMeadParameters
//   {
//     double alpha = 1;
//     double beta  = 2;
//     double gamma = 0.5;
//     double delta = 0.8;
//   } nmParam;

//   int m_nb_freedom_degrees = 3; // 1 : rescaling | 2 : 1+offset | 3 : 1+affine | 4 : 1+quadratic ...
//   int m_spectra_threshold = 0;
// };



#endif //SPECTRAALINGATOR_HPP