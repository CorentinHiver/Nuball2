#ifndef SPECTRAALINGATOR_HPP
#define SPECTRAALINGATOR_HPP

#include "../libRoot.hpp"

#define MAX_ORDER2

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

///////////////
// SpectraCo //
///////////////

class SpectraCo
{
public:

  SpectraCo() = default;

  SpectraCo(TH1* root_spectra)
  {
    load(root_spectra);
  }

  SpectraCo(SpectraCo const & other) : 
    m_spectra   (other.m_spectra),
    m_name      (other.m_name),
    m_size      (other.m_size),
    m_integral  (other.m_integral),
    m_min_value (other.m_min_value),
    m_max_value (other.m_max_value)
  {
    // throw_error("SpectraCo::SpectraCo(SpectraCo const & other) : function is not working !");
  }

  SpectraCo(SpectraCo const & other, Recalibration const & recal) : 
    m_spectra   (other.m_spectra),
    m_name      (other.m_name),
    m_size      (other.m_size),
    m_integral  (other.m_integral),
    m_min_value (other.m_min_value),
    m_max_value (other.m_max_value)
  {
    // throw_error("SpectraCo::SpectraCo(SpectraCo const & other) : function is not working !");
    this -> recalibrate(recal);
  }

  SpectraCo(std::vector<double> const & data) :
    m_spectra(data),
    m_size(data.size()),
    m_min_value(0),
    m_max_value(m_size)
  {
  }

  SpectraCo& operator=(TH1* root_spectra)
  {
    load(root_spectra);
    return *this;
  }

  void load(TH1* root_spectra)
  {
    if (root_spectra->IsA()->InheritsFrom(TH2::Class()) || root_spectra->IsA()->InheritsFrom(TH3::Class()))
    {
      throw_error("In SpectraAlignator::SpectraAlignator(TH1* root_spectra) : root_spectra is a TH2 or TH3 !!");
    }

    m_name = root_spectra->GetName();
    m_title = root_spectra->GetTitle();
    
    m_integral = root_spectra->Integral();
    m_size = root_spectra -> GetXaxis() -> GetNbins();
    m_min_value = root_spectra -> GetXaxis() -> GetBinLowEdge(0)+1;
    m_max_value = root_spectra -> GetXaxis() -> GetBinLowEdge(m_size)+1;

    m_spectra.resize(m_size);
    for (int bin = 0; bin<m_size; bin++) m_spectra[bin] = root_spectra->GetBinContent(bin);
  }

  std::vector<double> const & derivative() {return m_derivative;}
  std::vector<double> const & derivate(int const & smooth = 1)
  {
    m_derivative.resize(m_size);
    double low_sum = 0.0;
    double up_sum = 0.0;
    int lower_bin = 0;
    int upper_bin = m_size;
    for (int bin = 0; bin<m_size; bin++)
    {
      // First, sum the value of all the bins on the left :
      low_sum = 0.0;
      for (int bin_low = ((lower_bin = bin-smooth) < 1) ? 0 : lower_bin; bin_low<bin; bin_low++)
      {
        low_sum+=m_spectra[bin_low];
      }

      // Second, sum the value of all the bins on the right :
      up_sum = 0.0;
      upper_bin = ((upper_bin = bin+smooth)<(m_size+1)) ? upper_bin : m_size;
      for (int bin_up = bin; bin_up<upper_bin; bin_up++)
      {
        up_sum+=m_spectra[bin_up];
      }

      // Calculate the derivative : (sum_left - sum_right) / (x_right - x_left)
      m_derivative[bin] = (low_sum-up_sum)/(2*smooth);
    }
    // if (smooth<1) throw_error("in SpectraCo::derivate(int const & smooth = 1) : smooth < 1, can't do that !!");
    // if (smooth>3) throw_error("in SpectraCo::derivate(int const & smooth = 1) : smooth > 3, can't do that !!");
    // if (smooth == 1)
    // {
    //   m_derivative[0] = m_spectra[1]-m_spectra[0];
    //   for (int bin = 1; bin<m_size-1; bin++) m_derivative[bin] = (m_spectra[bin+1]-m_spectra[bin-1])/2;
    //   m_derivative[m_size] = m_spectra[m_size]-m_spectra[m_size-1];
    // }
    // else if (smooth == 2)
    // {
    //   m_derivative[0] = m_spectra[1]-m_spectra[0]/2;
    //   m_derivative[1] = m_spectra[2]-m_spectra[0]/2;
    //   for (int bin = 2; bin<m_size-1; bin++) m_derivative[bin] = (m_spectra[bin+2]+m_spectra[bin+1]-m_spectra[bin-1]-m_spectra[bin-2])/4;
    //   m_derivative[m_size-1] = m_spectra[m_size]-m_spectra[m_size-2]/2;
    //   m_derivative[m_size] = m_spectra[m_size]-m_spectra[m_size-1]/2;
    // }
    // else if (smooth == 3)
    // {
    //   m_derivative[0] = m_spectra[1]-m_spectra[0]/2;
    //   m_derivative[1] = m_spectra[2]-m_spectra[0]/2;
    //   m_derivative[2] = m_spectra[3]-m_spectra[1]/2;
    //   for (int bin = 3; bin<m_size-1; bin++) m_derivative[bin] = (m_spectra[bin+3]+m_spectra[bin+2]+m_spectra[bin+1] - m_spectra[bin-1]-m_spectra[bin-2]-m_spectra[bin-3])/6;
    //   m_derivative[m_size-2] = m_spectra[m_size-1]-m_spectra[m_size-3]/2;
    //   m_derivative[m_size-1] = m_spectra[m_size]-m_spectra[m_size-2]/2;
    //   m_derivative[m_size] = m_spectra[m_size]-m_spectra[m_size-1]/2;
    // }
    return m_derivative;
  }

  std::vector<double> const & derivative2() {return m_derivative2;}
  std::vector<double> const & derivate2(int const & smooth = 1)
  {
    derivate(smooth);
    m_derivative2.resize(m_size);
    double low_sum = 0.0;
    double up_sum = 0.0;
    int lower_bin = 0;
    int upper_bin = m_size;
    for (int bin = 0; bin<m_size; bin++)
    {
      // First, sum the value of all the bins on the left :
      low_sum = 0.0;
      for (int bin_low = ((lower_bin = bin-smooth) < 1) ? 0 : lower_bin; bin_low<bin; bin_low++)
      {
        low_sum+=m_derivative[bin_low];
      }

      // Second, sum the value of all the bins on the right :
      up_sum = 0.0;
      upper_bin = ((upper_bin = bin+smooth)<(m_size+1)) ? upper_bin : m_size;
      for (int bin_up = bin; bin_up<upper_bin; bin_up++)
      {
        up_sum+=m_derivative[bin_up];
      }

      // Calculate the derivative : (sum_left - sum_right) / (x_right - x_left)
      m_derivative2[bin] = (low_sum-up_sum)/(2*smooth);
    }
    // m_derivative2[0] = m_derivative[1]-m_derivative[0];
    // for (int bin = 1; bin<m_size-1; bin++)
    // {
    //   m_derivative2[bin] = (m_derivative[bin+1]-m_derivative[bin-1]);
    // }
    // m_derivative2[m_size] = m_derivative[m_size]-m_derivative[m_size-1];
    return m_derivative2;
  }

  void setMinValue( double const & _min_value) {m_min_value = _min_value;}
  void setMaxValue( double const & _max_value) {m_max_value = _max_value;}

  auto name(std::string const & name) {return (m_name = name);}
  auto title(std::string const & title) {return (m_title = title);}

  auto const & name() const {return m_name;}
  auto const & title() const {return m_title;}
  auto const & size() const {return m_size;}
  auto const & integral() const {return m_integral;}
  auto const & minValue() const {return m_min_value;}
  auto const & maxValue() const {return m_max_value;}

  auto & get() {return m_spectra;}
  auto const & get() const {return m_spectra;}
  auto const & lastBin()  const {return m_spectra.back();}
  auto const & back()  const {return m_spectra.back();}

  TH1D* createTH1D(std::string newName = "", std::string newTitle = "")
  {
    if (newName == "") newName = m_name;
    if (newTitle == "") newTitle = m_title;
    TH1D* out = new TH1D(m_name.c_str(), m_title.c_str(), m_size, this->minValue(), this->maxValue());
    for (int bin = 0; bin<m_size; bin++) out->SetBinContent(bin, m_spectra[bin]);
    return out;
  }

  TH1F* createTH1F(std::string newName = "", std::string newTitle = "")
  {
    if (newName == "") newName = m_name;
    if (newTitle == "") newTitle = m_title;
    TH1F* out = new TH1F(newName.c_str(), newTitle.c_str(), m_size, this->minValue(), this->maxValue());
    for (int bin = 0; bin<m_size; bin++) out->SetBinContent(bin, m_spectra[bin]);
    return out;
  }

  auto const & get(int const & bin) const {return m_spectra[bin];}
  auto const & operator[] (int const & bin) const {return m_spectra[bin];}
  auto & operator[] (int const & bin) {return m_spectra[bin];}
  double operator[] (double const & bin) const
  {
    int i = static_cast<int>(bin); //bin_i
    if (i<0) i = 0;
    else if (i > (m_size-1)) i = m_size;
    double const & a = m_spectra[i+1] - m_spectra[i];// a  =  y_i+1 - y_i
    double const & b = m_spectra[i] - a*i;           // b  =  y_i - a*bin_i
    return a*bin+b;
  }
  
  SpectraCo operator+(SpectraCo const & other)
  {
    if (other.m_size != m_size) throw_error("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
    SpectraCo spectra(*this);
    for (int bin = 0; bin<m_size; bin++) spectra[bin] += other[bin];
    spectra.name(spectra.name()+" + "+other.name());
    spectra.title(spectra.title()+" + "+other.title());
    return spectra;
  }

  SpectraCo operator-(SpectraCo const & other)
  {
    if (other.m_size != m_size) throw_error("in operator+(SpectraCo const & other) : other size is different from that of this spectra");
    SpectraCo spectra(*this);
    for (int bin = 0; bin<m_size; bin++) spectra[bin] -= other[bin];
    spectra.name(spectra.name()+" - "+other.name());
    spectra.title(spectra.title()+" - "+other.title());
    return spectra;
  }

  /// @brief wrapper around operator[]
  double interpolate(double const & bin) const {return (*this)[bin];} 

  void recalibrate(Recalibration const & recal)
  {
    std::vector<double> newSpectra(m_size);
    for (int bin = 0; bin<m_size; bin++)
    {
      auto const & new_bin = recal.calculate(bin);
      newSpectra[bin] = interpolate(new_bin);
    }
    m_spectra = newSpectra;
  }

  void Draw(const char* param = "")
  {
    TH1F* drawer = createTH1F();
    drawer->Draw(param);
    // pauseCo();
  }

private:
  std::vector<double> m_spectra;
  std::vector<double> m_derivative;
  std::vector<double> m_derivative2; // Second derivative

  std::string m_name = "Unnamed";
  std::string m_title = "Untitled";

  int m_size = -1;
  int m_integral = 0;
  int m_min_value = 0;
  int m_max_value = 0;
};

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
    m_dim(dim),
    m_size(m_dim+1)
  {
    for (int i = 0; i<m_size; i++) m_vertices.push_back(Vertice(dim));
  }

  Simplex(Vertices const & vertices) : 
    m_vertices(vertices), 
    m_dim(vertices.size()-1),
    m_size(m_dim+1)
  {
    for (auto const & vertex : vertices) if (vertex.size() != this->m_dim) 
      throw_error("in Simplex::Simplex(std::vector<Vertice> const & vertices) : dimension conflict of at least one vertex (simplex must have dim+1 vertices)");
  }

  Simplex(Simplex const & other) : 
    m_vertices(other.m_vertices),
    m_dim(other.m_dim),
    m_size(other.m_size),
    m_centroid(other.m_centroid)
  {
  }

  Simplex(Simplex && other) : 
    m_vertices(std::move(other.m_vertices)),
    m_dim(std::move(other.m_dim)),
    m_size(std::move(other.m_size)),
    m_centroid(std::move(other.m_centroid))
  {
  }

  Simplex& operator=(Simplex const & other)
  {
    m_vertices = other.m_vertices;
    m_dim = other.m_dim;
    m_size = other.m_size;
    m_centroid = other.m_centroid;
    return *this;
  }

  Simplex& operator=(Simplex && other)
  {
    m_vertices = std::move(other.m_vertices);
    m_dim = std::move(other.m_dim);
    m_size = std::move(other.m_size);
    m_centroid = std::move(other.m_centroid);
    return *this;
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
  auto const & size() {return m_size;}

  auto const & dim() const {return m_dim;}
  auto const & dim() {return m_dim;}

private:
  Vertices m_vertices;
  size_t m_dim = 0;
  int m_size = 0;
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

      double minimum_chi2 = 0; // First dumb chi2
      int loop_i = 0;

      while(loop_i<50)
      {
        // 1.a Calculate the chi2 for each vertice of the simplex :
        std::vector<double> chi2s;
        for (auto const & vertice : simplex) chi2s.push_back(this -> chi2(ref_spectra, spectra, vertice));

        print("");
        print("chi2s");
        print(chi2s);

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
        // auto pauseCo();

        double const & min_chi2 = chi2(ref_spectra, spectra, simplex[0]);
        print(loop_i, min_chi2, minimum_chi2, abs((minimum_chi2-min_chi2)/minimum_chi2));
        // if ( loop_i>10 && abs((minimum_chi2-min_chi2)/minimum_chi2) < 0.001) break;
        minimum_chi2 = min_chi2;
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

  // std::vector<double> getRealignedSpectra(int spectra_nb)
  // {
  //   // auto & spectra = m_spectra[spectra_nb];
  //   return std::vector<double>(0);

  // }

  void setIterations(int const & iterations) {m_nb_iterations = iterations;}

  void writeChi2Spectra(TFile* file)
  {
    file->cd();
    m_chi2_spectra->Write();
  }

  void setBruteForce(bool const & brute_force = true) {m_brute_force = brute_force;}

private:
  SpectraCo m_ref_spectra;

  // std::vector<int> m_lookup;
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