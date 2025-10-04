#ifndef SIMPLEPARIS
#define SIMPLEPARIS

#include "../libRoot.hpp"
#include "Arrays/Paris.h"
#include "PositionXY.hpp"
#include "PhoswitchCalib.hpp"

/////////////////////
// PARIS NAMESPACE //
/////////////////////

namespace Paris
{
  constexpr static uchar cluster_size = 36ul;
  static double distance_max = 1.5;

  static std::array<PositionXY, cluster_size>  positions;
  static std::array<std::array<double, cluster_size>, cluster_size> distances;
  std::recursive_mutex initialization_mutex;
  static bool cluster_init = false;

  static void InitialiseBidims()
  {
  #ifdef COMULTITHREADING 
    std::lock_guard<std::recursive_mutex> lock(initialization_mutex);
    if (cluster_init) return;
    if (MTObject::ON) printC("Initializing Paris position and distance lookup tables in thread ", MTObject::getThreadIndex());
    else printC("Initializing Paris position and distance lookup tables");
  #else
    printC("Initializing Paris position and distance lookup tables");
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
    cluster_init = true;
  }

  void printArrays()
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

  // Does the detector label correspond to a Paris ? 
  static constexpr auto is = Colib::LUT<1000> ([](Label const & label) {
    return Colib::binary_search(ParisArrays::labels, label);
  });

  // Return the index of the label in Paris (201 = 0, )
  static constexpr auto index = Colib::LUT<1000> ([](Label const & label) {
    if (is[label]) return static_cast<Label> (find_index(ParisArrays::labels, label));
    else return Label{0};
  });

  static constexpr auto cluster_index = Colib::LUT<1000> ([](Label const & label){
    if (is[label]) return Paris::index[label]%cluster_size;
    else return -1;
  });

  static constexpr auto cluster = Colib::LUT<1000> ([](Label const & label){
    if (is[label]) return index[label]/cluster_size;
    else return -1;
  });

  static constexpr inline bool pid_LaBr3(double const & qshort, double const & qlong)
  {
    if (qshort < 0 || qlong < 0) return false;
    auto const & ratio = qshort/qlong;
    return (0.75 < ratio && ratio < 1.1);
  }

  static constexpr inline bool pid_good_phoswitch(double const & qshort, double const & qlong)
  {
    if (qshort < 0 || qlong < 0) return false;
    auto const & ratio = qshort/qlong;
    return (-0.5 < ratio && ratio < 1.1);
  }

  static inline auto angle_to_beam(Index const & _index)
  {
    return PositionXY::distance(PositionXY(0,0), positions[_index]);
  }
};

////////////////////////////
// SIMPLE PARIS PHOSWITCH //
////////////////////////////

class SimplePhoswitch
{
public:
  SimplePhoswitch() : m_index(g_index++) {}

  double qlong = 0.0;
  double qshort = 0.0;
  double nrj = 0.0;
  double time = 0.0;
  bool rejected = false;
  Label label = 0;
  bool isLaBr3 = false;
  bool isGood = false;

  void clear()
  {
    qlong = 0.;
    qshort = 0.;
    nrj = 0.;
    time = 0.;
    rejected = false;
    isLaBr3 = false;
    isGood = false;
  }

  SimplePhoswitch* fill(Event const & event, int const & hit_i)
  {
    qshort = event.nrjs[hit_i];
    qlong = event.nrj2s[hit_i];
    time = event.times[hit_i];
    isLaBr3 = Paris::pid_LaBr3(qshort, qlong);
    isGood = Paris::pid_good_phoswitch(qshort, qlong);

    if (isLaBr3) nrj = qshort;
    else nrj = qlong;

    return this;
  }

  SimplePhoswitch* fill(Event const & event, int const & hit_i, PhoswitchCalib const & calib)
  {
    qshort = event.nrjs[hit_i];
    qlong = event.nrj2s[hit_i];
    time = event.times[hit_i];
    isLaBr3 = Paris::pid_LaBr3(qshort, qlong);
    isGood = Paris::pid_good_phoswitch(qshort, qlong);

    if (isLaBr3) nrj = qshort;
    else nrj = calib.calibrate(event.labels[hit_i], qshort, qlong);

    return this;
  }

  auto const & index() const noexcept {return m_index;}

  static void resetIndexes() noexcept {g_index = 0;}

private:
  Index static thread_local g_index;
  Index const m_index;
};
Index thread_local SimplePhoswitch::g_index = 0;
std::ostream& operator<<(std::ostream& out, SimplePhoswitch const & phoswitch)
{
  out << "Id : " << phoswitch.index() << " nrj = " << phoswitch.nrj << " time = " << phoswitch.time;
  return out;
}

/////////////////////////
// SIMPLE PARIS MODULE //
/////////////////////////
class SimpleParisModule
{
public:
  SimpleParisModule() : m_index(g_index++) {}

  double nrj = 0.0;
  double time = 0.0;

  void set(SimplePhoswitch const & phoswitch)
  {
    nrj = phoswitch.nrj;
    time = phoswitch.time;
    m_isLaBr3 = phoswitch.isLaBr3;
    ++m_nb;
  }

  void add(SimplePhoswitch const & phoswitch)
  {
    nrj += phoswitch.nrj;
    ++m_nb;
    m_isLaBr3 = m_isLaBr3 && phoswitch.isLaBr3;
    // Keep the time of the higher energy phoswitch which shoud have been set first in the accepted add-back method
  }

  void clear()
  {
    m_isLaBr3 = false;
    nrj = 0.;
    time = 0.;
    m_nb = 0.;
  }

  static void resetIndexes() noexcept {g_index = 0;}

  auto const & index  () const noexcept {return m_index  ;}
  auto const & nb     () const noexcept {return m_nb     ;}
  auto const & isLaBr3() const noexcept {return m_isLaBr3;}

  bool addbacked() const noexcept {return m_nb>0;}

private:
  bool m_isLaBr3 = false;
  size_t static thread_local g_index;
  size_t const m_index;
  int m_nb = 0;
};
size_t thread_local SimpleParisModule::g_index = 0;
std::ostream& operator<<(std::ostream& out, SimpleParisModule const & module)
{
  out << "Id : " << module.index() << " nrj = " << module.nrj << " time = " << module.time;
  return out;
}

//////////////////////////
// SIMPLE PARIS CLUSTER //
//////////////////////////

class SimpleCluster
{
public:
  SimpleCluster() : m_index(g_index++)
  {
    SimplePhoswitch::resetIndexes();
    SimpleParisModule::resetIndexes();
    for (auto & phoswitch : phoswitches) phoswitch.label = ParisArrays::labels[m_index * Paris::cluster_size + phoswitch.index()];
  };

  SimplePhoswitch* fill(Event const & event, int const & hit_i)
  {
    auto const & label = event.labels[hit_i];
    auto const & id = Paris::cluster_index[label];
    auto const & nrj = Paris::cluster_index[label];
    auto const & nrj2 = Paris::cluster_index[label];

    if (nrj < 0 || nrj2 < 0) return nullptr;

    if (Paris::cluster_size < id+1) {error("in SimpleCluster::fill : index", id, "> cluster_size !!"); return nullptr;}

    phoswitches_id.push_back(id);
    
    auto ret = phoswitches[id].fill(event, hit_i);

    calorimetry+=phoswitches[id].nrj;

    return ret;
  }

  SimplePhoswitch* fill(Event const & event, int const & hit_i, PhoswitchCalib const & calib)
  {
    auto const & label = event.labels[hit_i];
    auto const & id = Paris::cluster_index[label];
    auto const & nrj = Paris::cluster_index[label];
    auto const & nrj2 = Paris::cluster_index[label];

    if (nrj < 0 || nrj2 < 0) return nullptr;

    if (Paris::cluster_size < id+1) {error("in SimpleCluster::fill : index", id, "> cluster_size !!"); return nullptr;}

    phoswitches_id.push_back(id);
    
    auto ret = phoswitches[id].fill(event, hit_i, calib);

    calorimetry+=phoswitches[id].nrj;

    return ret;
  }

  void clear()
  {
    for(auto const & id : phoswitches_id) phoswitches[id].clear();
    phoswitches_id.clear();
    // for(auto const & id : modules_id) modules[id].clear();
    for(auto & module : modules) module.clear();
    modules_id.clear();
    phoswitch_mult = 0;
    module_mult = 0;
    m_addback = false;
    m_addback_used = false;
    calorimetry = 0;
  }

  void addback()
  {
    phoswitch_mult = phoswitches_id.size();

    if (phoswitch_mult==0) return; 
    else if (phoswitch_mult==1)
    {// Addback algorithm not necessary if there is only one phoswitch in the cluster
      auto const & id_i = phoswitches_id[0];
      modules_id.push_back(id_i);
      modules[id_i].set(phoswitches[id_i]);
      return;
    }

    // 1. Order the hits from the highest to lowest energy deposit :
    std::vector<size_t> hits_ordered(phoswitch_mult);
    std::iota(hits_ordered.begin(), hits_ordered.end(), 0);
    std::sort(hits_ordered.begin(), hits_ordered.end(), [&] (int const & hit_i, int const & hit_j)
    {
      auto const & id_i = phoswitches_id[hit_i];
      auto const & id_j = phoswitches_id[hit_j];
      return phoswitches[id_i].nrj > phoswitches[id_j].nrj;
    });

// 2. Perform the add-back
    for (size_t ordered_loop_i = 0; ordered_loop_i<phoswitch_mult; ++ordered_loop_i)
    {
      auto const & hit_i = hits_ordered[ordered_loop_i]; // Starts with the highest energy deposit
      auto const & id_i = phoswitches_id[hit_i]; // The index of the detector in its cluster (see ParisCluster class)

      if (phoswitches[id_i].rejected) continue; // If this hit has already been used for add-back with a previous hit then discard it
      
      modules[id_i].set(phoswitches[id_i]);
      modules_id.push_back(id_i);

      // Test the other detectors in the event for a potential add-back :
      for (size_t ordered_loop_j = ordered_loop_i+1; ordered_loop_j<phoswitch_mult; ++ordered_loop_j)
      {
        auto const & hit_j = hits_ordered[ordered_loop_j];
        auto & id_j = phoswitches_id[hit_j];
        
        // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
        auto const & distance_ij = Paris::distances[id_i][id_j];
        if (distance_ij > Paris::distance_max) continue;

        // Timing : if the hits are not simultaneous then they don't belong to the same gamma-ray
        if (std::abs(phoswitches[id_j].time - phoswitches[id_i].time) > m_time_window) continue; 

        // They pass both conditions, so we add them back :
        modules[id_i].add(phoswitches[id_j]);

        // If they are add-backed, it means we can reject the individual phoswitches used in the procedure :
        phoswitches[id_j].rejected = true;
        phoswitches[id_i].rejected = true;

        m_addback_used = true;
      }
    }
    module_mult = modules_id.size();
    m_addback = true;
  }

  std::array<SimplePhoswitch, Paris::cluster_size> phoswitches;
  std::array<SimpleParisModule, Paris::cluster_size> modules;

  std::vector<Index> phoswitches_id;
  std::vector<Index> modules_id;

  size_t phoswitch_mult = 0;
  size_t module_mult = 0;

  float calorimetry = 0;
  static void setDistanceMax(double const & _distance_max) {Paris::distance_max = _distance_max;}
  void setTimeWindow(double const & _time_window) {m_time_window = _time_window;}
  auto const & isAddBack() const {return m_addback;}
  auto const & isAddBackUsed() const {return m_addback_used;}

  static void resetIndexes() noexcept {g_index = 0;}

private:
  bool m_addback = false;
  bool m_addback_used = false;
  Time m_time_window = 4_ns;
  
  Index static thread_local g_index;
  Index const m_index;
};

Index thread_local SimpleCluster::g_index = 0;

std::ostream& operator<<(std::ostream& out, SimpleCluster const & cluster)
{
  if (cluster.isAddBackUsed()) out << "Phoswitches :" << std::endl;
  for (auto const & phoswitch_id : cluster.phoswitches_id) out << cluster.phoswitches[phoswitch_id] << std::endl;
  if(cluster.isAddBackUsed()) 
  {
    out << "Modules :" << std::endl;
    for (auto const & module_id : cluster.modules_id) out << cluster.modules[module_id] << std::endl;
  }
  return out;
}

////////////////////////
// SIMPLE PARIS CLASS //
////////////////////////

class SimpleParis
{
public:
  SimpleParis(PhoswitchCalib* calib = nullptr) : m_calib(calib) 
  {
    SimpleCluster::resetIndexes();
    Paris::InitialiseBidims();
  }

  void fill(Event const & event, int const & hit_i)
  {
    auto const & label = event.labels[hit_i];

    if (!Paris::is[label]) return;

    auto const & cluster = Paris::cluster[label];

         if (cluster == 0) phoswitches.push_back(back .fill(event, hit_i));
    else if (cluster == 1) phoswitches.push_back(front.fill(event, hit_i));    
    else error("SimpleParis::fill : no cluster (",cluster,") found for label", label);

    if (phoswitches.size() > 0 && phoswitches.back() == nullptr) phoswitches.pop_back();

    if (!phoswitches.back()->rejected) clean_phoswitches.push_back(phoswitches.back());
  }

  void fill(Event const & event, int const & hit_i, PhoswitchCalib const & calib)
  {
    auto const & label = event.labels[hit_i];

    if (!Paris::is[label]) return;

    auto const & cluster = Paris::cluster[label];

         if (cluster == 0) phoswitches.push_back(back .fill(event, hit_i, calib));
    else if (cluster == 1) phoswitches.push_back(front.fill(event, hit_i, calib));    
    else error("SimpleParis::fill : no cluster (",cluster,") found for label", label);

    if (phoswitches.size() > 0 && phoswitches.back() == nullptr) phoswitches.pop_back();

    if (!phoswitches.back()->rejected) clean_phoswitches.push_back(phoswitches.back());
  }

  // void fill(Event const & event, int const & hit_i) 
  // {
  //   if (m_calib) fill(event, hit_i, *m_calib);
  //   else fill(event, hit_i);
  // }

  void setEvent(Event const & event, PhoswitchCalib const & calib)
  {
    clear();
    for (int hit_i = 0; hit_i<event.mult; ++hit_i) this->fill(event, hit_i, calib);
    analyze();
  }

  void setEvent(Event const & event)
  {
    clear();
    for (int hit_i = 0; hit_i<event.mult; ++hit_i) this->fill(event, hit_i);
    analyze();
  }

  void operator=(Event const & event) {(m_calib) ? setEvent(event, *m_calib) : setEvent(event);}

  void timeOrder()
  {
    std::sort(phoswitches.begin(), phoswitches.end(), [](SimplePhoswitch const * p1, SimplePhoswitch const * p2)
    {
      return p1->time < p2->time;
    });

    std::sort(clean_phoswitches.begin(), clean_phoswitches.end(), [](SimplePhoswitch const * p1, SimplePhoswitch const * p2)
    {
      return p1->time < p2->time;
    });

    std::sort(modules.begin(), modules.end(), [](SimpleParisModule const * p1, SimpleParisModule const * p2)
    {
      return p1->time < p2->time;
    });
  }

  void analyze(bool const & time_order = false)
  {
    if (phoswitches.size() == 0) return;
    
    back.addback();
    front.addback();

    for (auto const & module_id : back.modules_id) modules.push_back(&(back.modules[module_id]));
    for (auto const & module_id : front.modules_id) modules.push_back(&(front.modules[module_id]));

    if (time_order) timeOrder();
  }

  void clear()
  {
    back.clear();
    front.clear();
    modules.clear();
    phoswitches.clear();
    clean_phoswitches.clear();
  }
  
  int phoswitch_mult() const { return back.phoswitches_id.size() + front.phoswitches_id.size();}
  int module_mult() const { return back.modules_id.size() + front.modules_id.size();}
  float calorimetry() const {return back.calorimetry + front.calorimetry;}

  SimpleCluster back;
  SimpleCluster front;
  std::vector<SimpleParisModule*> modules;
  std::vector<SimplePhoswitch*> phoswitches;
  std::vector<SimplePhoswitch*> clean_phoswitches;

private:
  PhoswitchCalib* m_calib = nullptr;
};

std::ostream& operator<<(std::ostream& out, SimpleParis const & paris)
{
  out << "Back cluster :" << std::endl;
  out << paris.back;
  out << "Front cluster :" << std::endl;
  out << paris.front;
  return out;
}

#endif //SIMPLEPARIS