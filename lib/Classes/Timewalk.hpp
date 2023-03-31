#ifndef TIMEWALK_H
#define TIMEWALK_H

class Timewalk
{
public:
  Timewalk(){}
  Timewalk(std::string const & filename){loadFile(filename);}
  void loadFile(std::string const & filename);
  Float_t const & get(Float_t const & nrj);
  void Print(){for (size_t i = 0; i<m_Timewalk.size(); i++) print(i*m_keVperBin, m_Timewalk[i]);}
  void resize(size_t const & size, Float_t const & value = 0.) {m_Timewalk.resize(size, value);}
  void resize() {m_Timewalk.resize(max, 0.);}
  static void setMax(Float_t const & _max) {max = _max;}
  static Float_t const & getMax() {return max;}
  auto & operator[] (int const & i) {return m_Timewalk[i];}

private:
  Float_t m_keVperBin = 1.;
  std::vector<Float_t> m_Timewalk;
  static Float_t max;
  bool isON;
  Float_t m_float_zero = 0.;
};

Float_t Timewalk::max = 10000;

inline Float_t const & Timewalk::get(Float_t const & nrj)
{
  return (isON) ? ((nrj>Timewalk::max) ? m_float_zero : m_Timewalk[static_cast<int>(nrj/m_keVperBin)])
                : m_float_zero;
}

void Timewalk::loadFile(std::string const & filename)
{
  // ---- Loading file ---- //
  std::vector<Float_t> Energies;
  std::vector<Float_t> Timewalk;
  std::ifstream f (filename, std::ios::in);
  if (!f.good()) {isON = false; return;}
  std::string line;
  getline(f,line); // To get rid of the header
  while(getline(f,line))
  {
    std::istringstream is(line);
    is >> Energies >> Timewalk;
  }
  if (Energies.size()<2) {isON = false; return;}
  else isON = true;
  m_keVperBin = Energies[2]-Energies[1];
  // ---- Filling arrays ---- //
  size_t bin = 0;
  // 1 : extend the first bin timewalk towards 0
  while (bin*m_keVperBin < Energies[0]) {m_Timewalk.push_back(Timewalk[0]);bin++;}
  // 2 : fill normally the vector until max is reached
  int j = 0;
  while (bin*m_keVperBin < max) {m_Timewalk.push_back(Timewalk[j]);bin++;j++;}
  // --- Check for aberrous values --- //
  for (size_t i = 0; i<m_Timewalk.size(); i++)
    if (i*m_keVperBin>5000 && abs(m_Timewalk[i]-m_Timewalk[i-1]) > 15)
    {
      print(rmPathAndExt(filename), i*m_keVperBin, m_Timewalk[i]);
      m_Timewalk[i] = m_Timewalk[i-1];
    }
}

template<uchar nb_param, typename T>
class fParameters // Functions parameters
{
public:
  fParameters() {m_parameters.resize(nb_param);}
  T const & operator[] (size_t const & i) const {return m_parameters[i];}
  T & get(size_t const & i) {return m_parameters[i];}
  operator std::vector<T>() {return m_parameters;}
private:
  std::vector<T> m_parameters;
};

using TWparameters = fParameters<4, Float_t>;

class Timewalks
{
public:
  Timewalks(){};
  void loadFile(std::string const & filename);
  std::vector<Float_t> & operator[] (int const & i) {return m_timewalks[i];}
  void resize(int const & i = 0) {m_timewalks.resize(i); m_parameters.resize(i);}
  Float_t timewalk(Float_t Q, Float_t a, Float_t b, Float_t t0, Float_t factor) {return factor*(t0+a/TMath::Sqrt(Q+b));}
  Float_t timewalk(Float_t Q, TWparameters const & p) {return p[3]*(p[2]+p[0]/TMath::Sqrt(Q+p[1]));}
  Float_t const & get(int const & label, Float_t const & Q);
  static void setMax(Float_t const & _max) {Timewalk::setMax(_max);}

private:
  std::vector<std::vector<Float_t>> m_timewalks;
  std::vector<TWparameters> m_parameters;
  Float_t Emin = 0.;
  Float_t Emax = 20000.;
  Float_t Tmax = 100.;
  Float_t Tmin = 0.;
  bool isON = false;
};

Float_t const & Timewalks::get(int const & label, Float_t const & Q)
{
  if (Q>Emax) return Tmin;
  else
  {
    auto const & y = m_timewalks[label][Q];
    return (y>Tmax) ? Tmax : y;
  }
}

void Timewalks::loadFile(std::string const & filename)
{
  std::ifstream f (filename, std::ios::in);
  if (!f.good()) {isON = false; return;}
  std::string line;
  getline(f,line); getline(f,line); // To get rid of the header

  // To extract the informations about the fit :
  {
    getline(f,line);
    std::istringstream is(line);
    std::string temp; is >> temp;
    if (temp == "Settings:")
    {
      is >> temp;
      if (temp == "minE=") is >> Emin;
    }
  }

  Emin+=20; // To avoid having a way too big value if function diverge at Emin

  // Extract the function for each channel :
  while(getline(f,line) && line != "end")
  {
    std::istringstream is(line);
    int l = 0;
    is >> l;
    l-=800;
    Float_t param = 0.; int i = 0;
    while(is >> param){m_parameters[l].get(i) = param; i++;}
  }
  for (size_t l = 0; l<m_timewalks.size(); l++)
  {
    auto & tw_array = m_timewalks[l];
    auto const & p = m_parameters[l];
    tw_array.resize(Emax);
    for (size_t e = 0; e<tw_array.size(); e++)
    {
      tw_array[e] = (e<Emin) ? timewalk(Emin, p) : timewalk(e, p);
      if (tw_array[e]<Tmin) Tmin = tw_array[e];
    }
  }
  // int l = 0;
  // Timewalk::setMax(20000);
  // for (auto const & p : m_parameters)
  // {
  //   print(l,p);
  //   print("____________________");
  //   m_timewalks[l].resize(20000);
  //   if(p.size()==0) continue;
  //
  //   for (size_t i = 0; i<20000ul; i++)
  //   {
  //     if (i<minE) m_timewalks[l][i] = timewalk(minE, p[0], p[1], p[2], p[3]);
  //     else m_timewalks[l][i] = timewalk(i, p[0], p[1], p[2], p[3]);
  //   }
  //   l++;
  // }
  print("Timewalk arrays ready");
  isON = true;
}

#endif //TIMEWALK_H
