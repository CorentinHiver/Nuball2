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
  void reset()
  {
    time = 0ll;
    nrj = 0.f;
    state = -1;
    nb_LaBr3 = 0;
    nb_NaI   = 0;
    nb_mix   = 0;
  }

  void add(Time const & _time, NRJ const & _nrj, char const & cristal)
  {
    if (state < 0) 
    { // First phoswitch
      time = _time;
      nrj = _nrj;
      state = cristal;
    }
    else if (state != cristal) 
    { // Any other phoswitch
      state = 3;
      nrj += _nrj;
    }
  }

  void add(ParisPhoswitch const & phoswitch)
  {
    this -> add(phoswitch.time, phoswitch.nrj, phoswitch.state);
  }

  Time time = 0ll;
  float nrj = 0.f;
  char state = -1;
  int nb_LaBr3 = 0;
  int nb_NaI   = 0;
  int nb_mix   = 0;
  std::vector<std::string> states = {"LaBr3", "NaI", "Mix", "External mix"};

private:
  size_t static thread_local g_label;
  size_t m_label;
};
size_t thread_local ParisModule::g_label = 0;

/**
 * @brief A cluster is a group of paris that are grouped together.
 * @todo Cut this class into two pieces : a first named ParisCluster that implements a generic way to handle add-back
 * and another one called ParisNuball2 that inherits from the later and implements the geometry
 * 
 * @tparam n 
 */
template<size_t n>
class ParisCluster
{
public:
  ParisCluster() : m_label(gLabel) {gLabel++; this -> Initialise();}
  void Initialise();
  void InitialiseBidims();
  void reset();
  void fill(Event const & event, int const & i, uchar const & label);
  void analyse();
  auto const & label() const {return m_label;}
  auto const & size() const {return n;}

  ParisPhoswitch & operator[] (int const & i) {return phoswitches[i];}

  std::array<PositionXY, n>  positions;
  std::array<std::array<float, n>, n> distances;

  std::array<ParisPhoswitch, n> phoswitches;
  std::array<ParisModule, n> modules;

  std::vector<Label> Hits;
  std::vector<Label> HitsClean;
  std::vector<Label> hits_LaBr3;
  std::vector<Label> hits_NaI;

  static auto const & nb_clusters() {return gLabel;}

private:
  uchar const m_label;
  static thread_local uchar gLabel;
};

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

  lock_mutex lock(MTObject::mutex);
  
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
  Hits.clear();
  HitsClean.clear();
  hits_LaBr3.clear();
  hits_NaI.clear();
}

template<size_t n>
void inline ParisCluster<n>::fill(Event const & event, int const & i, uchar const & index)
{
  phoswitches[index].fill(event, i);
  Hits.push_back(index);
}

template<size_t n>
void ParisCluster<n>::analyse()
{
  std::vector<double> phoswitches_added(n, false); // Stores the phoswitches already add-backed

  //1. Fills the phoswitches by energy deposit :
  std::vector<std::pair<Label, double>> energies;
  for (auto const & index : Hits) energies.push_back({index, phoswitches[index].nrj});
  std::sort(energies.begin(), energies.end(), [&](std::pair<Label, double> p1, std::pair<Label, double> p2)
  {
    return p1.second<p2.second;
  });

  // Faire des tests ici

  for (size_t loop_i = 0; loop_i<energies.size(); loop_i++)
  {
    auto const & index_i = energies[loop_i].first;
    auto const & phoswitch_i = phoswitches[index_i];
    modules[index_i].add(phoswitch_i);

    for (size_t loop_j = loop_i+1; loop_j<energies.size(); loop_j++)
    {
      if (phoswitches_added[loop_j]) continue; // If the phoswitch was in coincidence with a previous one
      auto const & index_j = energies[loop_i].first;

      auto const & phoswitch_j = phoswitches[index_j];
      auto const & distance = distances[index_i][index_j];

      if (abs(phoswitch_j.time-phoswitch_i.time) > 50000) continue; // Timing : if they are not simultaneous then they don't belong to the same event
      if (distance<1.3)
      {// If neighbors :
      }
      phoswitches_added[loop_j] = true;
    } // End j loop
  } // End i loop
}

#endif //PARIS_CLUSTER_HPP
