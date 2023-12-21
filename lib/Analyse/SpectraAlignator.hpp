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
  Vertice(std::vector<double> const & coords) : 
    m_dim(coords.size()),
    m_coordinates(coords)
  {
  }

  Vertice(Vertice const & _vertice) : 
    m_dim(_vertice.m_dim),
    m_coordinates(_vertice.m_coordinates)
  {
  }

  Vertice(Vertice && _vertice) : 
    m_dim(std::move(_vertice.m_dim)),
    m_coordinates(std::move(_vertice.m_coordinates))
  {
  }

  Vertice(size_t size) : 
    m_dim(size),
    m_coordinates(std::vector<double>(m_dim))
  {
  }

  auto & operator[] (int const & i) {return m_coordinates[i];}
  auto const & operator[] (int const & i) const {return m_coordinates[i];}

  Vertice& operator=(std::vector<double> const & point) {this->set(point); return *this;}

  Vertice& operator=(Vertice const & vertice) {this->set(vertice); return *this;}

  Vertice& operator=(Vertice && vertice) {this->set(std::move(vertice)); return *this;}

  void set(Vertice const & _vertice)
  {
    if (this -> m_dim != _vertice.m_dim) throw_error("in Vertice::set(Vertice const & _vertice) : dimension of _vertice different from that of the vertice");
    this->m_coordinates = _vertice.m_coordinates;
  }

  void set(Vertice && _vertice)
  {
    if (this -> m_dim != _vertice.m_dim) throw_error("in Vertice::set(Vertice && _vertice) : dimension of _vertice different from that of the vertice");
    this->m_coordinates = std::move(_vertice.m_coordinates);
  }

  void set(std::vector<double> const & point)
  {
    if (point.size()!=(size_t)(m_dim)) throw_error("in Vertice::set(std::vector<double> const & point) : dimension of point different from that of the vertice");
    m_coordinates = point;
  }

  auto const & get() const {return m_coordinates;}
  auto & get() {return m_coordinates;}

  auto & size() {return m_dim;}
  auto const & size() const {return m_dim;}

  auto begin() {return m_coordinates.begin();}
  auto end() {return m_coordinates.end();}

  auto begin() const {return m_coordinates.begin();}
  auto end() const {return m_coordinates.end();}


  Vertice operator+(Vertice const & other)
  {
    if (other.size() != m_dim) throw_error("in Vertice::operator+(Vertice const & other) : other and vertice not the same size !!");
    std::vector<double> coords;
    for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i] + other[i]);
    return Vertice(coords);
  }

  Vertice operator-(Vertice const & other)
  {
    if (other.size() != m_dim) 
    {
      throw_error("in Vertice::operator+(Vertice const & other) : other and vertice not the same size !!");
    }
    std::vector<double> coords;
    for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i] - other[i]);
    return Vertice(coords);
  }

  // Vectorial product : TBD !!!
  Vertice operator*(Vertice const & other)
  {
    throw_error("operator Vertice*Vertice not implemented yet !!");
    // if (other.size() != m_dim) throw_error("in Vertice::operator+(Vertice const & other) : other and vertice not the same size !!");
    std::vector<double> coords = other.get();
    // for (size_t i = 0; i<other.size(); i++) coords.push_back(m_coordinates[i]*other[i]);
    // return Vertice(coords);
    return coords;
  }

  Vertice& operator*(double const & constant)
  {
    for (auto & coord : m_coordinates) coord*=constant;
    return *this;
  }

private:
  size_t m_dim = 0;
  std::vector<double> m_coordinates;
};

std::ostream& operator<<(std::ostream& out, Vertice const & vertice)
{
  out << vertice.get();
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
    m_size(dim+1)
    m_dim(dim),
  {
    for (int i = 0; i<m_size; i++) m_vertices.push_back(Vertice(dim));
  }

  Simplex(Vertices const & vertices) : 
    m_vertices(vertices), 
    m_size(vertices.size())
    m_dim(m_size-1),
  {
    for (auto const & vertex : vertices) if (vertex.size() != this->m_dim) 
      throw_error("in Simplex::Simplex(std::vector<Vertice> const & vertices) : dimension conflict of at least one vertex (simplex must have dim+1 vertices)");
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

    Simplex() : 
    m_vertices(vertices), 
    m_size(vertices.size())
    m_dim(m_size-1),
  {
    for (auto const & vertex : vertices) if (vertex.size() != this->m_dim) 
      throw_error("in Simplex::Simplex(std::vector<Vertice> const & vertices) : dimension conflict of at least one vertex (simplex must have dim+1 vertices)");
  }

  Simplex& operator=(Vertices const & vertices)
  {
    m_vertices(vertices);
    m_size(vertices.size());
    m_dim(m_size-1);
  }

  void setVertice(int const & bin, Vertice const & vertice) {if (bin<m_size) m_vertices[bin] = vertice;}
  void setVertice(int const & bin, Vertice && vertice) {if (bin<m_size) m_vertices[bin] = std::move(vertice);}

  auto & operator[] (int const & i) {return m_vertices[i];}
  auto const & operator[] (int const & i) const {return m_vertices[i];}

  auto begin() {return m_vertices.begin();}
  auto end() {return m_vertices.end();}

  auto begin() const {return m_vertices.begin();}
  auto end() const {return m_vertices.end();}

  auto centroid(std::vector<int> const & sorted_vertices_indexes)
  {
    auto const & nb_good_points = sorted_vertices_indexes.size()-1;
    for (auto & coord : m_centroid) coord = 0;
    for (size_t point_i = 0; point_i<nb_good_points; point_i++) for (size_t coord_i = 0; coord_i<m_centroid.size(); coord_i++)
    {
      m_centroid[coord_i] += m_vertices[sorted_vertices_indexes[point_i]][coord_i];
    }
    for (auto & coord : m_centroid) coord /= (double)(nb_good_points);
    return m_centroid;
  }

  auto const & size() const {return m_size;}
  auto & size() {return m_size;}

  auto const & dim() const {return m_dim;}
  auto & dim() {return m_dim;}

private:
  Vertices m_vertices;
  int m_size = 0;
  size_t m_dim = 0;
  Vertice m_centroid = Vertice(m_dim);
};

std::ostream& operator<<(std::ostream& out, Simplex const & simplex)
{
  out << simplex.size() << " points : ";
  for (auto const & vertice : simplex) out << std::endl << vertice;
  return out;
}

class Minimisator
{
public:
  Minimisator(Recalibration * recal) : m_recal(recal)
  {
    
  }

  void setC(double const & _C) {m_C = _C;}

  double chi2 (SpectraCo const & ref_spectra, SpectraCo const & spectra, Vertice const & vertice)
  {
    // To generalise here
    m_C = vertice[0];
    for (size_t param = 1; param<vertice.size(); param++) m_recal->setParameter(param, vertice[param]);
    return chi2(ref_spectra, spectra);
  }

  double chi2 (SpectraCo const & ref_spectra, SpectraCo const & spectra)
  {
    double sum_errors_squared = 0.0;
    // print(spectra.size(), m_spectra_threshold);
    int const & nb_bins_studied = spectra.size()-0;
    for (int bin = 0; bin<spectra.size(); bin++) if (spectra[bin]>0)
    {
      // Calculate new bin value :
      double const & new_bin = m_recal->calculate(bin);

      // Do a linear interpolation to get the value of the new bin :
      double const & new_value = spectra.interpolate(new_bin);

      // Calculate the error for this bin :
      double const & error = ref_spectra[bin] - m_C*new_value;

      // Variance of the bin :
      double const & weight = 1/spectra[bin]; // V = sigma² = 1/N

      // Add the error to the total squared error of the spectra :
      sum_errors_squared += error*error*weight;

      // print(spectra[bin], weight, new_bin, new_value, error, sum_errors_squared);
    }
    // print(sum_errors_squared/(nb_bins_studied-m_nb_freedom_degrees));
    // pauseCo();
    return sum_errors_squared/(nb_bins_studied-m_nb_freedom_degrees);
  }

  double chi2_second_derivative(SpectraCo & ref_spectra, SpectraCo & test_spectra)
  {
    if (ref_spectra.derivative2().size()<1) ref_spectra.derivate2();
    double sum_errors_squared = 0.0;
    // print(test_spectra.size(), m_spectra_threshold);
    // int const & nb_bins_studied = test_spectra.size()-m_spectra_threshold;
    // for (int bin = m_spectra_threshold; bin<test_spectra.size(); bin++) if (test_spectra[bin]>0)
    SpectraCo recal_spectra(test_spectra, *m_recal);
    auto & derivative2_spectra = recal_spectra.derivate2();
    for (int bin = 0; bin<test_spectra.size(); bin++) if (test_spectra[bin]>0)
    {
      // Variance of the bin :
      double const & weight = 1/derivative2_spectra[bin];

      // Calculate the error for this bin :
      double const & error = ref_spectra[bin] - m_C*derivative2_spectra[bin];

      // Add it to the total squared error of the test_spectra :
      sum_errors_squared += error*error*weight;
      
    }
    return sum_errors_squared/(test_spectra.size()-m_nb_freedom_degrees);
  }

  void setDegreesOfFreedom(int const & degrees) {m_nb_freedom_degrees = degrees;}
  auto const & getDegreesOfFreedom() const {return m_nb_freedom_degrees;}

  void setMinC (double const & _minC) {m_minC = _minC;}
  void setMaxC (double const & _maxC) {m_maxC = _maxC;}
  void setMina0 (double const & _mina0) {m_mina0 = _mina0;}
  void setMaxa0 (double const & _maxa0) {m_maxa0 = _maxa0;}
  void setMina1 (double const & _mina1) {m_mina1 = _mina1;}
  void setMaxa1 (double const & _maxa1) {m_maxa1 = _maxa1;}
  void setMina2 (double const & _mina2) {m_mina2 = _mina2;}
  void setMaxa2 (double const & _maxa2) {m_maxa2 = _maxa2;}
  void setMinaSqrt (double const & _minaSqrt) {m_minaSqrt = _minaSqrt;}
  void setMaxaSqrt (double const & _maxaSqrt) {m_maxaSqrt = _maxaSqrt;}

  Vertice minimise(SpectraCo const & ref_spectra, SpectraCo const & spectra, std::string const & method = "nelder-mead")
  {
    if (method == "nelder-mead")
    {
      // To generalise here
      auto const & dumb_C = (m_maxC+m_minC)/2;
      auto const & dumb_a0 = (m_maxa0+m_mina0)/2;
      auto const & dumb_a1 = (m_maxa1+m_mina1)/2;
      // auto const & dumb_a2 = (m_maxa2-m_mina2)/2;
      Simplex simplex ({
        Vertice({dumb_C, dumb_a0, dumb_a1}),
        Vertice({m_minC, m_mina0, m_mina1}),
        Vertice({m_maxC, m_maxa0, m_maxa1}),
        Vertice({dumb_C, m_maxa0, m_maxa1})
      });

      print("first simplex : ");
      print(simplex);

      double last_chi2 = 0; // First dumb chi2
      int loop_i = 0;

      while(loop_i<50)
      {
        // 1.a Calculate the chi2 for each vertice of the simplex :
        std::vector<double> chi2s;
        for (auto const & vertice : simplex) chi2s.push_back(this -> chi2(ref_spectra, spectra, vertice));

        // print("");
        // print("chi2s");
        // print(chi2s);

        auto const & dim = chi2s.size()-1;

        // 1.b Sorts the chi2s
        std::vector<double> ordered_chi2s;
        std::vector<int> ordered_indexes;
        bubbleSort(chi2s, ordered_indexes);
        for (auto const & index : ordered_indexes) ordered_chi2s.push_back(chi2s[index]);

        // Calculate the centroid 
        Vertice centroid (simplex.centroid(ordered_indexes)); // centroid

        // 2.a Compute the reflexion point : centroid + alpha * (centroid-worst_vertice)
        Vertice reflection (centroid + nmParam.alpha*(centroid - simplex[ordered_indexes.back()]));
        // 2.b Compute its chi2
        auto chi2_reflection = chi2(ref_spectra, spectra, reflection);
        print("chi2_reflection", chi2_reflection);
        
        if (chi2_reflection < ordered_chi2s[0])
        { // If chi2_1>chi2_reflection
          // 3.a Compute the expansion point : centroid + beta * (reflection-centroid)
          Vertice expansion(centroid + nmParam.beta*(reflection-centroid));
          // 3.b Compute its chi2
          auto chi2_expansion = chi2(ref_spectra, spectra, expansion);
          print("chi2_expansion", chi2_expansion);
          if (chi2_expansion < chi2_reflection) simplex[ordered_indexes.back()] = expansion;
          else simplex[ordered_indexes.back()] = reflection;
        }
        else if (chi2_reflection < ordered_chi2s[dim])
        {
          simplex[ordered_indexes.back()] = reflection;
        }
        else if (chi2_reflection < ordered_chi2s.back())
        {
          // 4.a Compute the outside contraction point : centroid + beta * (reflection-centroid)
          Vertice outsideContraction(centroid + nmParam.gamma*(reflection-centroid));
          // 4.b Compute its chi2
          auto chi2_outside_contraction = chi2(ref_spectra, spectra, outsideContraction);
          print("chi2_outside_contraction", chi2_outside_contraction);
          if (chi2_outside_contraction < chi2_reflection) simplex[ordered_indexes.back()] = outsideContraction;
        }
        else if (ordered_chi2s[dim] < chi2_reflection)
        {
          // 5.a Compute the inside contraction point : centroid + beta * (reflection-centroid)
          Vertice insideContraction(centroid - nmParam.gamma*(reflection-centroid));
          // 5.b Compute its chi2
          auto chi2_inside_contraction = chi2(ref_spectra, spectra, insideContraction);
        print("chi2_inside_contraction", chi2_inside_contraction);
          if (chi2_inside_contraction < chi2_reflection) simplex[ordered_indexes.back()] = insideContraction;
        }

        // 6 Shrinks the simplex :
        Simplex tempSimplex(simplex);
        tempSimplex[0] = simplex[ordered_indexes[0]]; // this is the best point
        for (size_t point_i = 1; point_i<dim+1; point_i++)
        {
          tempSimplex[point_i] = tempSimplex[0] + nmParam.delta * (simplex[ordered_indexes[point_i]] - tempSimplex[0]);
        }

        simplex = tempSimplex;
        print("second simplex : ");
        print(simplex);

        double const & min_chi2 = chi2(ref_spectra, spectra, simplex[0]);
        print(loop_i, min_chi2, last_chi2, abs((last_chi2-min_chi2)/last_chi2));

        // Stop condition :
        // if ( loop_i>10 && abs((last_chi2-min_chi2)/last_chi2) < 0.001) break;
        last_chi2 = min_chi2;
        loop_i++;
      }
      print(loop_i);
      print(simplex);
      return simplex[0];
    }
    else return Vertice({});
  }

  auto operator->() {return m_recal;}

  void setThreshold(int const & bin) {m_spectra_threshold = bin;}

private:
  Recalibration * m_recal = nullptr;

  // For the brute force :
  double m_C = 0.0; 

  // For the minimisation techniques :
  double m_minC = 0.0;
  double m_maxC = 0.0;
  double m_mina0 = 0.0;
  double m_maxa0 = 0.0;
  double m_mina1 = 0.0;
  double m_maxa1 = 0.0;
  double m_mina2 = 0.0;
  double m_maxa2 = 0.0;
  double m_minaSqrt = 0.0;
  double m_maxaSqrt = 0.0;

  struct NelderMeadParameters
  {
    double alpha = 1;
    double beta  = 2;
    double gamma = 0.5;
    double delta = 0.8;
  } nmParam;

  int m_nb_freedom_degrees = 3; // 1 : rescaling | 2 : 1+offset | 3 : 1+affine | 4 : 1+quadratic ...
  int m_spectra_threshold = 0;
};

class SpectraAlignator
{
public:
  SpectraAlignator(TH1* ref) :
    m_ref_spectra(ref)
  {
  }

  Recalibration const & alignSpectra(TH1* _spectra, TH1* spectra_output, int const & degreesOfFreedom = 3, double const & energyThreshold = 0)
  {
    m_spectra.emplace_back(_spectra);
    auto & spectra = m_spectra.back();
    m_recals.emplace_back(Recalibration());
    auto & recal = m_recals.back();
    m_minimisator.emplace_back(&recal);
    auto & minimisator = m_minimisator.back();
    minimisator.setDegreesOfFreedom(degreesOfFreedom);
    if (energyThreshold>0 && energyThreshold<_spectra->GetXaxis()->GetXmax()) minimisator.setThreshold(_spectra->FindBin(energyThreshold));

    // std::vector<std::vector<std::vector<double>>> minimisation_space;

    double const & C_dumb = 1;
    double const & C_min = C_dumb - 0.1;
    double const & C_max = C_dumb + 0.1;
    double const & C_0 = (C_min+C_max)/2;
    double const & C_steps = (C_max-C_min)/m_nb_iterations;
    
    double const & a0_min = -0.5;
    double const & a0_max = 0.5;
    double const & a0_0 = (a0_min+a0_max)/2;
    double const & a0_steps = (a0_max-a0_min)/m_nb_iterations;
    
    double const & a1_min = 0.99;
    double const & a1_max = 1.005;
    double const & a1_0 = (a1_min+a1_max)/2;
    double const & a1_steps = (a1_max-a1_min)/m_nb_iterations;

    double const & a2_min = -0.00001;
    double const & a2_max = 0.00001;
    double const & a2_0 = (a2_min+a2_max)/2;
    double const & a2_steps = (a2_max-a2_min)/m_nb_iterations;

    // Initialise at default values :
    auto min_C = C_0;
    auto min_a0 = a0_0;
    auto min_a1 = a1_0;
    auto min_a2 = a2_0;

    minimisator.setC(min_C);
    minimisator->a0(min_a0);
    minimisator->a1(min_a1);
    if (degreesOfFreedom>3) recal.a2(min_a2);

    auto min_chi2 = minimisator.chi2(m_ref_spectra, spectra);
    if (degreesOfFreedom>3) print(min_chi2, min_C, min_a0, min_a1, min_a2);
    else print(min_chi2, min_C, min_a0, min_a1);
    
    m_chi2_spectra = new TH3F("chi2", "chi2;a0;a1;C", m_nb_iterations,a0_min,a0_max, m_nb_iterations,a1_min,a1_max, m_nb_iterations,C_min,C_max);

    int tries = 0;
    // Use brute force calculation :
    if (m_brute_force)
    {
      for (int C = 0; C<m_nb_iterations; C++)
      {
        for (int a0 = 0; a0<m_nb_iterations; a0++)
        {
          for (int a1 = 0; a1<m_nb_iterations; a1++)
          {
            if (degreesOfFreedom>3) for (int a2 = 0; a2<m_nb_iterations; a2++)
            {
              tries++;
              double const & C_value = C_min+C*C_steps;
              double const & a0_value = a0_min+a0*a0_steps;
              double const & a1_value = a1_min+a1*a1_steps;
              double const & a2_value = a2_min+a2*a2_steps;

              minimisator.setC(C_value);
              double const & _chi2 = minimisator.chi2(m_ref_spectra, spectra, Vertice({a0_value, a1_value, a2_value}));

              m_chi2_spectra->Fill(min_a0, min_a1, min_C);
              if (_chi2 < min_chi2)
              {
                min_C  = C_value ;
                min_a0 = a0_value;
                min_a1 = a1_value;
                min_a2 = a2_value;
                min_chi2 = _chi2;
                print(min_chi2, min_C, min_a0, min_a1, min_a2);
              } 
            }
            else
            {
              tries++;
              double const & C_value = C_min+C*C_steps;
              // print(C_value, C_min, C,C_steps);
              double const & a0_value = a0_min+a0*a0_steps;
              double const & a1_value = a1_min+a1*a1_steps;
              minimisator.setC(C_value);
              minimisator->a0(a0_value);
              minimisator->a1(a1_value);
              // print(recal);
              // double const & _chi2 = minimisator.chi2_second_derivative(m_ref_spectra, spectra);
              double const & _chi2 = minimisator.chi2(m_ref_spectra, spectra);
              // print(a0_value, a1_value, C_value);
              // print(a0_value, a1_value, C_value, _chi2);
              m_chi2_spectra->SetBinContent(a0, a1, C, _chi2);
//               print(_chi2, C_value
// ,a0_value
// ,a1_value);

              // if ((int)(C_value+a0_value+a1_value)%100 == 0) print(_chi2, C_value, a0_value, a1_value);

              if (_chi2 < min_chi2)
              {
                min_C  = C_value ;
                min_a0 = a0_value;
                min_a1 = a1_value;
                min_chi2 = _chi2;
                print("best khi2 :", min_chi2, "vertice : ", min_C, min_a0, min_a1);
              }
            }
          }
        }
      }
    }
    // Use minimisator
    else
    {
      minimisator.setMinC(C_min);
      minimisator.setMaxC(C_max);
      minimisator.setMina0(a0_min);
      minimisator.setMaxa0(a0_max);
      minimisator.setMina1(a1_min);
      minimisator.setMaxa1(a1_max);
      minimisator.setMina2(a2_min);
      minimisator.setMaxa2(a2_max);

      minimisator.minimise(m_ref_spectra, spectra);

      print(recal);
    }

    // Set the previously found minimum values :
    if (degreesOfFreedom>3) print(min_chi2, min_C, min_a0, min_a1, min_a2);
    else print(min_chi2, min_C, min_a0, min_a1);
    recal.a0(min_a0);
    recal.a1(min_a1);
    if (degreesOfFreedom>3) recal.a2(min_a2);

    spectra.recalibrate(recal);
    if (spectra_output) delete spectra_output;
    spectra_output = new TH1F(
      (spectra.name()+"_recal").c_str(), 
      (spectra.name()+"_recal").c_str(), 
      spectra.size(), spectra.minValue(), spectra.maxValue());
    for (int bin = 0; bin<spectra.size(); bin++) spectra_output->SetBinContent(bin, spectra[bin]);
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
  std::vector<Minimisator> m_minimisator;
  std::vector<Recalibration> m_recals;
  std::vector<double> m_chi2s;

  TH2F* surface_chi2 = nullptr;

  int m_nb_iterations = 20;
  bool m_brute_force = false;
  TH3F* m_chi2_spectra = nullptr;
  int m_spectra_threshold = 0;
};

#endif //SPECTRAALINGATOR_HPP