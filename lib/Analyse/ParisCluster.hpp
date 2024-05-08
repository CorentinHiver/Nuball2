#ifndef PARIS_CLUSTER_HPP
#define PARIS_CLUSTER_HPP

#include "../libRoot.hpp"
#include "ParisPhoswitch.hpp"
#include "PositionXY.hpp"
#include "Arrays/Paris.h"


class ParisModule
{
public:
  ParisModule() : m_label(g_label++) {}

  void switch_cristal(char const & cristal)
  {
    switch (cristal)
    {
      case 0 : ++nb_LaBr3; break;
      case 1 : ++nb_NaI; break;
      case 2 : ++nb_mix; break;
      default : break;
    }
  }

  void set(Time const & _time, NRJ const & _nrj, char const & cristal)
  {
    switch_cristal(cristal);
    time = _time;
    nrj = _nrj;
    state = cristal;
  }
  void set(ParisPhoswitch const & phoswitch) { this -> set(phoswitch.time, phoswitch.nrj, phoswitch.cristal);}

  void add(NRJ const & _nrj, char const & cristal)
  {
    switch_cristal(cristal);
    nrj += _nrj;
    if (state != cristal) state = 3;
  }

  void add(ParisPhoswitch const & phoswitch) {this -> add(phoswitch.nrj, phoswitch.cristal);}

  inline auto const & label() const {return m_label;}
  inline static void reset_g_label() {g_label = 0;}
  
  void reset()
  {
    time = 0ll;
    nrj = 0.f;
    state = -1;
    nb_LaBr3 = 0;
    nb_NaI   = 0;
    nb_mix   = 0;
    LaBr3_calorimetry = 0.f;
    NaI_calorimetry = 0.f;
    total_calorimetry = 0.f;
  }

  Time time = 0ll;
  float nrj = 0.f;
  char state = -1;
  int nb_LaBr3 = 0;
  int nb_NaI   = 0;
  int nb_mix   = 0;
  float LaBr3_calorimetry = 0.f;
  float NaI_calorimetry = 0.f;
  float total_calorimetry = 0.f;

private:
  size_t static thread_local g_label;
  size_t m_label;
};

size_t thread_local ParisModule::g_label = 0;
std::vector<std::string> ParisModule_states = {"LaBr3", "NaI", "Mix", "External mix"};

std::ostream& operator<<(std::ostream& out, ParisModule const & module)
{
  out << "label : " << module.label()
      << " | time : " << module.time 
      << " ps energy : " << module.nrj << " keV | ";
  if (module.state    >=0) out << ParisModule_states[module.state]; 
  if (module.nb_LaBr3 > 0) out << " nb_LaBr3 : " << module.nb_LaBr3;
  if (module.nb_NaI   > 0) out << " nb_NaI : "   << module.nb_NaI;
  if (module.nb_mix   > 0) out << " nb_mix : "   << module.nb_mix;
  return out;
}

/**
 * @brief A cluster is a group of paris that are grouped together.
 * @todo Cut this class into two pieces : a first named ParisCluster that implements a generic way to handle add-back
 * and another one called ParisNuball2 that inherits from the later and implements the geometry
 */
template<size_t n>
class ParisCluster
{
public:
  ParisCluster() : m_label(gLabel) {gLabel++; this -> Initialise(); ParisModule::reset_g_label();}
  void Initialise();
  void InitialiseBidims();
  void reset();
  void fill(Event const & event, int const & hit_i, uchar const & label);
  void analyze();
  void add_back(std::vector<Label> const & phoswitches_hits, std::array<ParisModule, n> & _modules, std::vector<Label> & modules_hits, Time const & _timewindow);
  auto const & label() const {return m_label;}
  auto const & size() const {return n;}

  ParisPhoswitch & operator[] (int const & i) {return phoswitches[i];}

  std::array<PositionXY, n>  positions;
  std::array<std::array<float, n>, n> distances;

  std::array<ParisPhoswitch, n> phoswitches;
  std::array<ParisModule, n> modules;
  std::array<ParisModule, n> modules_pureLaBr;

  std::vector<Label> Hits;
  std::vector<Label> hits_LaBr3;
  std::vector<Label> hits_NaI;
  std::vector<Label> HitsClean;
  std::vector<Label> CleanLaBr3;

  static auto const & nb_clusters() {return gLabel;}
  static Time LaBr3_timewindow;
  static Time timewindow;
  static double distance_max;

  float LaBr3_calorimetry = 0.f;
  float NaI_calorimetry = 0.f;
  float total_calorimetry = 0.f;

private:
  uchar const m_label;
  static thread_local uchar gLabel;
};
template<size_t n>
Time ParisCluster<n>::LaBr3_timewindow = 5_ns;

template<size_t n>
Time ParisCluster<n>::timewindow = 10_ns;

template<size_t n>
double ParisCluster<n>::distance_max = 2;

template<size_t n>
thread_local uchar ParisCluster<n>::gLabel = 0;

template<size_t n>
void ParisCluster<n>::Initialise()
{
  InitialiseBidims();
}

template<size_t n>
void ParisCluster<n>::InitialiseBidims()
{
  // Filling the positions of the detectors :  
  auto const & i_min_R2 = Paris_R1_x.size();
  auto const & i_min_R3 = Paris_R1_x.size()+Paris_R2_x.size();
#ifdef MULTITHREADING 
  lock_mutex lock(MTObject::mutex);
#endif //MULTITHREADING
  for (size_t i = 1; i<positions.size(); i++)
  {
         if (i<i_min_R2 ) positions[i] = PositionXY(Paris_R1_x[i-1       ], Paris_R1_y[i-1       ]); // Ring 1
    else if (i<i_min_R3)  positions[i] = PositionXY(Paris_R2_x[i-i_min_R2], Paris_R2_y[i-i_min_R2]); // Ring 2
    else                  positions[i] = PositionXY(Paris_R3_x[i-i_min_R3], Paris_R3_y[i-i_min_R3]); // Ring 3
  }

  // Filling the distance lookup table :
  for (size_t i = 0; i<positions.size(); i++)
  {
    for (size_t j = i+1; j<positions.size(); j++)
    {
      auto const & distance = PositionXY::distance(positions[i], positions[j]);
      distances[i][j] = distance;
      distances[j][i] = distance;
    }
  }
}

template<size_t n>
void ParisCluster<n>::reset()
{
  for (auto const & index : Hits) phoswitches[index].reset();
  for (auto const & index : HitsClean) modules[index].reset();
  for (auto const & index : CleanLaBr3) modules_pureLaBr[index].reset();
  Hits.clear();
  hits_LaBr3.clear();
  hits_NaI.clear();
  HitsClean.clear();
  CleanLaBr3.clear();
  LaBr3_calorimetry = 0;
  NaI_calorimetry = 0;
  total_calorimetry = 0;
}

template<size_t n>
void inline ParisCluster<n>::fill(Event const & event, int const & hit_i, uchar const & index)
{
  phoswitches[index].fill(event, hit_i);
  auto const & cristal = phoswitches[index].cristal;
  if (cristal < 0) return; // rejected hits
  Hits.push_back(index);
  if (cristal == 0) 
  {
    hits_LaBr3.push_back(index);
    LaBr3_calorimetry+=event.nrjs[hit_i];
    total_calorimetry+=event.nrjs[hit_i];
  }
  if (cristal == 1) 
  {
    hits_NaI  .push_back(index);
    NaI_calorimetry+=event.nrj2s[hit_i];
    total_calorimetry+=event.nrj2s[hit_i];
  }
}

template<size_t n>
void ParisCluster<n>::analyze()
{
  add_back(Hits, modules, HitsClean, timewindow);
  add_back(hits_LaBr3, modules_pureLaBr, CleanLaBr3, LaBr3_timewindow);
}

template<size_t n>
void ParisCluster<n>::add_back(std::vector<Label> const & phoswitches_hits, std::array<ParisModule, n> & _modules, std::vector<Label> & modules_hits, Time const & _timewindow)
{
  // Fills the phoswitches by energy deposit (maximum first):
  std::vector<std::pair<Label, double>> energies;
  for (auto const & index : phoswitches_hits) energies.push_back({index, phoswitches[index].nrj});
  if (energies.size()==0) return;

  std::sort(energies.begin(), energies.end(), [&](std::pair<Label, double> p1, std::pair<Label, double> p2)
  {
    return p1.second>p2.second;
  });

  // Stores the positions of the phoswitches already used for add-back during the process
  std::vector<double> phoswitches_added(n, false); 
  // Loop through the phoswitches :
  for (size_t loop_i = 0; loop_i<energies.size(); loop_i++)
  {
    // If the phoswitch was already used for add-back then don't handle it twice :
    if (phoswitches_added[loop_i]) continue; 

    auto const & index_i = energies[loop_i].first;
    auto const & phoswitch_i = phoswitches[index_i];
    _modules[index_i].set(phoswitch_i);
    modules_hits.push_back(index_i);

    // Test each other detector in the event :
    for (size_t loop_j = loop_i+1; loop_j<energies.size(); loop_j++)
    {
      auto const & index_j = energies[loop_j].first;

      auto const & phoswitch_j = phoswitches[index_j];
      auto const & distance_ij = distances[index_i][index_j];

      // Timing : if they are not simultaneous then they don't belong to the same event
      if (abs(phoswitch_j.time-phoswitch_i.time) > _timewindow) continue; 

      // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
      if (distance_ij<distance_max) continue;

      // If the two hits meets the criteria they are add-backed :
      _modules[index_i].add(phoswitch_j);
      phoswitches_added[loop_j] = true;
    } // End j loop
  } // End i loop
}


#endif //PARIS_CLUSTER_HPP
