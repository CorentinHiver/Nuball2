#ifndef PARIS_CLUSTER_HPP
#define PARIS_CLUSTER_HPP

#include "../libRoot.hpp"
#include "ParisPhoswitch.hpp"
#include "PositionXY.hpp"
#include "PhoswitchCalib.hpp"
#include "Arrays/Paris.h"

std::recursive_mutex initialization_mutex;

constexpr static uchar paris_cluster_size = 36ul;
constexpr static double LaBr3_timewindow = 2_ns;
constexpr static double paris_timewindow = 5_ns;
static double paris_distance_max = 1.5;


class ParisModule : public Module
{
public:
  ParisModule() : Module(), m_label(g_label++){}

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
  }

  // Time time = 0ll;
  char state = -1;
  int nb_LaBr3 = 0;
  int nb_NaI   = 0;
  int nb_mix   = 0;

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
  ParisCluster() : m_label(gLabel++) {this -> Initialise(); ParisModule::reset_g_label();}
  void Initialise();
  static void InitialiseBidims();
  static void printArrays()
  {
    print("Positions : ");
    std::cout << "     "       << " " << "     "       << " " << positions[24] << " " << positions[25] << " " << positions[26] << " " << "     "       << " " << "     "       << std::endl;
    std::cout << "     "       << " " << positions[23] << " " << positions[ 8] << " " << positions[ 9] << " " << positions[10] << " " << positions[11] << " " << "     "       << std::endl;
    std::cout << positions[35] << " " << positions[22] << " " << positions[ 0] << " " << positions[ 1] << " " << positions[ 2] << " " << positions[12] << " " << positions[27] << std::endl;
    std::cout << positions[34] << " " << positions[21] << " " << positions[ 7] << " " << "     "       << " " << positions[ 3] << " " << positions[13] << " " << positions[28] << std::endl;
    std::cout << positions[33] << " " << positions[20] << " " << positions[ 6] << " " << positions[ 5] << " " << positions[ 4] << " " << positions[14] << " " << positions[29] << std::endl;
    std::cout << "     "       << " " << positions[19] << " " << positions[18] << " " << positions[17] << " " << positions[16] << " " << positions[15] << " " << "     "       << std::endl;
    std::cout << "     "       << " " << "     "       << " " << positions[32] << " " << positions[31] << " " << positions[30] << " " << "     "       << " " << "     "       << std::endl;
    print("_____________________");
    print("Distances : ");
    std::cout << std::setw(4) << "";
    for (size_t i = 0; i<positions.size(); i++)
    {
      std::cout << std::setw(4) << i;
    }
    print();
    print();
    std::cout << std::fixed << std::setprecision(1);
    for (size_t i = 0; i<positions.size(); i++) 
    {
      std::cout << std::setw(4) << i;
      for (size_t j = 0; j<positions.size(); j++)
      {
        std::cout << std::setw(4) << distances[i][j];
      }
      print();
    }
    print();
    for (size_t i = 0; i<positions.size(); i++) 
    {
      print(positions[i], ":");
      std::cout << std::setw(5) << " "              << std::setw(5) << " "              << std::setw(5) << distances[i][24] << std::setw(5) << distances[i][25] << std::setw(5) << distances[i][26] << std::setw(5) << " "              << std::setw(5) << " "              << std::endl;
      std::cout << std::setw(5) << " "              << std::setw(5) << distances[i][23] << std::setw(5) << distances[i][ 8] << std::setw(5) << distances[i][ 9] << std::setw(5) << distances[i][10] << std::setw(5) << distances[i][11] << std::setw(5) << " "              << std::endl;
      std::cout << std::setw(5) << distances[i][35] << std::setw(5) << distances[i][22] << std::setw(5) << distances[i][ 0] << std::setw(5) << distances[i][ 1] << std::setw(5) << distances[i][ 2] << std::setw(5) << distances[i][12] << std::setw(5) << distances[i][27] << std::endl;
      std::cout << std::setw(5) << distances[i][34] << std::setw(5) << distances[i][21] << std::setw(5) << distances[i][ 7] << std::setw(5) << " "              << std::setw(5) << distances[i][ 3] << std::setw(5) << distances[i][13] << std::setw(5) << distances[i][28] << std::endl;
      std::cout << std::setw(5) << distances[i][33] << std::setw(5) << distances[i][20] << std::setw(5) << distances[i][ 6] << std::setw(5) << distances[i][ 5] << std::setw(5) << distances[i][ 4] << std::setw(5) << distances[i][14] << std::setw(5) << distances[i][29] << std::endl;
      std::cout << std::setw(5) << " "              << std::setw(5) << distances[i][19] << std::setw(5) << distances[i][18] << std::setw(5) << distances[i][17] << std::setw(5) << distances[i][16] << std::setw(5) << distances[i][15] << std::setw(5) << " "              << std::endl;
      std::cout << std::setw(5) << " "              << std::setw(5) << " "              << std::setw(5) << distances[i][32] << std::setw(5) << distances[i][31] << std::setw(5) << distances[i][30] << std::setw(5) << " "              << std::setw(5) << " "              << std::endl;
      print();
    }
  }
  void reset();
  void clear() {this->reset();}
  void fill(Event const & event, int const & hit_i, uchar const & label);
  void analyze();
  auto const & label() const {return m_label;}
  auto const & size() const {return n;}

  ParisPhoswitch & operator[] (int const & i) {return phoswitches[i];}

  static std::array<PositionXY, n>  positions;
  static std::array<std::array<double, n>, n> distances;

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
  void add_back(std::vector<Label> const & phoswitches_hits, std::array<ParisModule, n> & _modules, std::vector<Label> & modules_hits, Time const & _timewindow);
  uchar const m_label;
  static thread_local uchar gLabel;
  static bool gParisClusterInitialised;
};

template<size_t n>
Time ParisCluster<n>::LaBr3_timewindow = LaBr3_timewindow;

template<size_t n>
Time ParisCluster<n>::timewindow = paris_timewindow;

template<size_t n>
double ParisCluster<n>::distance_max = paris_distance_max;

template<size_t n>
std::array<PositionXY, n> ParisCluster<n>::positions;

template<size_t n>
std::array<std::array<double, n>, n> ParisCluster<n>::distances = {{{0}}};

template<size_t n>
thread_local uchar ParisCluster<n>::gLabel = 0;

template<size_t n>
bool ParisCluster<n>::gParisClusterInitialised = false;

template<size_t n>
void ParisCluster<n>::Initialise()
{
  InitialiseBidims();
}

template<size_t n>
void ParisCluster<n>::InitialiseBidims()
{
#ifdef COMULTITHREADING 
  std::lock_guard<std::recursive_mutex> lock(initialization_mutex);
  if (gParisClusterInitialised) return;
  printC("Initializing ParisCluster<",n,"> position and distance lookup tables in thread ", MTObject::getThreadIndex());
#else
  printC("Initializing ParisCluster<",n,"> position and distance lookup tables");
#endif //COMULTITHREADING

  // Filling the positions of the detectors :  
  auto const & i_min_R2 = ParisArrays::Paris_R1_x.size();
  auto const & i_min_R3 = ParisArrays::Paris_R1_x.size()+ParisArrays::Paris_R2_x.size();
  for (size_t i = 0; i<positions.size()+1; i++)
  {
         if (i<i_min_R2) positions[i] = PositionXY(ParisArrays::Paris_R1_x[i         ], ParisArrays::Paris_R1_y[i         ]); // Ring 1
    else if (i<i_min_R3) positions[i] = PositionXY(ParisArrays::Paris_R2_x[i-i_min_R2], ParisArrays::Paris_R2_y[i-i_min_R2]); // Ring 2
    else                 positions[i] = PositionXY(ParisArrays::Paris_R3_x[i-i_min_R3], ParisArrays::Paris_R3_y[i-i_min_R3]); // Ring 3
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
  // printArrays();
  // Colib::pause();
  gParisClusterInitialised = true;
}

template<size_t n>
inline void ParisCluster<n>::reset()
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
  // add_back(hits_LaBr3, modules_pureLaBr, CleanLaBr3, LaBr3_timewindow);
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
      if (std::abs(phoswitch_j.time-phoswitch_i.time) > _timewindow) continue; 

      // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
      if (distance_ij>distance_max) continue;

      // If the two hits meets the criteria they are add-backed :
      _modules[index_i].add(phoswitch_j);
      phoswitches_added[loop_j] = true;
    } // End j loop
  } // End i loop
}

#endif //PARIS_CLUSTER_HPP
