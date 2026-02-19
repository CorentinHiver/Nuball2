#ifndef TIMEWALK_H
#define TIMEWALK_H

#include "../libCo.hpp"

class Timewalk
{
public:
  Timewalk(){}
  Timewalk(std::string const & filename){loadFile(filename);}
  void loadFile(std::string const & filename);
  float const & get(float const & nrj);
  void Print(){for (size_t i = 0; i<m_Timewalk.size(); i++) print(static_cast<float>(i)*m_keVperBin, m_Timewalk[i]);}
  void resize(size_t const & size, float const & value = 0.) {m_Timewalk.resize(size, value);}
  void resize() {m_Timewalk.resize(static_cast<int>(max), 0.);}
  static void setMax(float const & _max) {max = _max;}
  static float const & getMax() {return max;}
  auto & operator[] (int const & i) {return m_Timewalk[i];}

private:
  float m_keVperBin = 1.;
  std::vector<float> m_Timewalk;
  static float max;
  bool isON;
  float m_float_zero = 0.;
};

float Timewalk::max = 10000.f;

inline float const & Timewalk::get(float const & nrj)
{
  return (isON) ? ((nrj>Timewalk::max) ? m_float_zero : m_Timewalk[static_cast<int>(nrj/m_keVperBin)])
                : m_float_zero;
}

void Timewalk::loadFile(std::string const & filename)
{
  // ---- Loading file ---- //
  std::vector<float> Energies;
  std::vector<float> Timewalk;
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
  float bin = 0;
  // 1 : extend the first bin timewalk towards 0
  while (bin*m_keVperBin < Energies[0]) {m_Timewalk.push_back(Timewalk[0]);bin++;}
  // 2 : fill normally the vector until max is reached
  int j = 0;
  while (bin*m_keVperBin < max) {m_Timewalk.push_back(Timewalk[j]);bin++;j++;}
  // --- Check for aberrous values --- //
  for (size_t i = 0; i<m_Timewalk.size(); i++)
  {
    if (static_cast<float>(i)*m_keVperBin>5000 && abs(m_Timewalk[i]-m_Timewalk[i-1]) > 15)
    {
      print(rmPathAndExt(filename), static_cast<float>(i)*m_keVperBin, m_Timewalk[i]);
      m_Timewalk[i] = m_Timewalk[i-1];
    }
  }
}

/**
 * @brief Functions parameter
 * 
 * A wrapper around a vector of parameters
 */
template<uchar nb_param, typename T>
class fParameters 
{
public:
  fParameters() {m_parameters.resize(nb_param);}
  T const & operator[] (size_t const & i) const {return m_parameters[i];}
  T & get(size_t const & i) {return m_parameters[i];}
  operator std::vector<T>() {return m_parameters;}
private:
  std::vector<T> m_parameters;
};

using TWparameters = fParameters<4, float>;

class Timewalks
{
public:
  Timewalks(){};
  void loadFile(std::string const & filename);
  std::vector<float> & operator[] (int const & i) {return m_timewalks[i];}
  void resize(int const & i = 0) {m_timewalks.resize(i); m_parameters.resize(i);}
  float timewalk(float Q, float a, float b, float t0, float factor) {return factor*(t0+a/static_cast<float>(TMath::Sqrt(Q+b)));}
  float timewalk(float Q, TWparameters const & p) {return p[3]*(p[2]+p[0]/static_cast<float>(TMath::Sqrt(Q+p[1])));}
  float const & get(int const & label, float const & Q);
  static void setMax(float const & _max) {Timewalk::setMax(_max);}

private:
  std::vector<std::vector<float>> m_timewalks;
  std::vector<TWparameters> m_parameters;
  float Emin = 0.;
  float Emax = 20000.;
  float Tmax = 100.;
  float Tmin = 0.;
  bool isON = false;
};

float const & Timewalks::get(int const & label, float const & Q)
{
  if (Q>Emax) return Tmin;
  else
  {
    auto const & y = m_timewalks[label][static_cast<size_t>(Q)];
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

  // Extract the function for each channel :
  while(getline(f,line) && line != "end")
  {
    std::istringstream is(line);
    int l = 0;
    is >> l;
    l-=800;
    float param = 0.; int i = 0;
    while(is >> param){m_parameters[l].get(i) = param; i++;}
  }
  for (size_t l = 0; l<m_timewalks.size(); l++)
  {
    auto const & p = m_parameters[l];
    m_timewalks[l].resize(static_cast<size_t>(Emax));
    for (size_t e = 0; e<m_timewalks[l].size(); e++)
    {
      auto const ef = static_cast<float>(e);
      m_timewalks[l][e] = (ef<Emin) ? timewalk(Emin, p) : timewalk(ef, p);
      if (m_timewalks[l][e]<Tmin) Tmin = m_timewalks[l][e];
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
