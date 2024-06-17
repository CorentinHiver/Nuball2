#ifndef SIMPLEPARIS
#define SIMPLEPARIS

#include "ParisCluster.hpp"

// Paris lookup tables :
namespace Paris
{
  static constexpr auto is = LUT<1000> ([](Label const & label) {
    return (label<ParisArrays::paris_labels[0] || label>ParisArrays::paris_labels.back())
           ? false
           : binary_search(ParisArrays::paris_labels, label);
  });

  static constexpr auto index = LUT<1000> ([](Label const & label) {
    if (is[label])
    {
      return static_cast<Label> ( find(ParisArrays::paris_labels, label) - ParisArrays::paris_labels[0]);
    }
    else return Label{0};
  });

  static constexpr auto cluster_index = LUT<1000> ([](Label const & label){
    if (is[label]) return index[label]%paris_cluster_size;
    else return -1;
  });

  static constexpr auto cluster = LUT<1000> ([](Label const & label){
    if (is[label]) return index[label]/paris_cluster_size;
    else return -1;
  });
};

// Paris classes :
class SimplePhoswitch
{
public:
  SimplePhoswitch() : m_label(g_label++) {}

  double qlong = 0.0;
  double qshort = 0.0;
  double nrj = 0.0;
  double time = 0.0;
  bool rejected = false;

  void clear()
  {
    qlong = 0.;
    qshort = 0.;
    nrj = 0.;
    time = 0.;
    rejected = false;
  }

  auto const & label() const {return m_label;}

  static void resetLabels() {g_label = 0;}

private:
  size_t static thread_local g_label;
  size_t const m_label;
};
size_t thread_local SimplePhoswitch::g_label = 0;
std::ostream& operator<<(std::ostream& out, SimplePhoswitch const & phoswitch)
{
  out << "Id : " << phoswitch.label() << " nrj = " << phoswitch.nrj << " time = " << phoswitch.time;
  return out;
}

class SimpleParisModule
{
public:
  SimpleParisModule() : m_label(g_label++) {}

  double nrj = 0.0;
  double time = 0.0;

  void set(SimplePhoswitch const & phoswitch)
  {
    nrj = phoswitch.nrj;
    time = phoswitch.time;
  }

  void add(SimplePhoswitch const & phoswitch)
  {
    nrj += phoswitch.nrj;
    // Keep the time of the higher energy phoswitch
  }

  void clear()
  {
    nrj = 0.;
    time = 0.;
  }

  auto angle_to_beam() const
  {
    return PositionXY::distance(PositionXY(0,0), ParisCluster<paris_cluster_size>::positions[m_label]);
  }

  static void resetLabels() {g_label = 0;}
  auto const & label() const {return m_label;}

private:
  size_t static thread_local g_label;
  size_t const m_label;
};
size_t thread_local SimpleParisModule::g_label = 0;
std::ostream& operator<<(std::ostream& out, SimpleParisModule const & module)
{
  out << "Id : " << module.label() << " nrj = " << module.nrj << " time = " << module.time;
  return out;
}

class SimpleCluster
{
public:
  SimpleCluster() 
  {
    SimplePhoswitch::resetLabels();
    SimpleParisModule::resetLabels();
  };

  SimplePhoswitch* fill(Event const & event, int const & hit_i, PhoswitchCalib const & calib)
  {
    auto const & label = event.labels[hit_i];
    if (!Paris::is[label]) return nullptr;
    auto const & id = Paris::cluster_index[label];

    if (paris_cluster_size < id+1) {error("in SimpleCluster::fill : index", id, "> paris_cluster_size !!"); return nullptr;}

    phoswitches_id.push_back(id);
    
    auto & phoswitch = phoswitches[id];
    phoswitch.qshort = event.nrjs[hit_i];
    phoswitch.qlong = event.nrj2s[hit_i];
    phoswitch.nrj = calib.calibrate(label, phoswitch.qshort, phoswitch.qlong);
    phoswitch.time = event.times[hit_i];
    calorimetry+=phoswitch.nrj;
    print(*this);
    pauseCo();
    return &phoswitch;
  }

  void clear()
  {
    for(auto const id : phoswitches_id) phoswitches[id].clear();
    phoswitches_id.clear();
    for(auto const id : modules_id) modules[id].clear();
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

    // 1. Order the hits from the highest to lowest energy deposit :
    std::vector<size_t> hits_ordered;
    linspace(hits_ordered, phoswitch_mult);
    std::sort(hits_ordered.begin(), hits_ordered.end(), [&] (int hit_i, int hit_j)
    {
      auto const & id_i = phoswitches_id[hit_i];
      auto const & id_j = phoswitches_id[hit_j];
      return phoswitches[id_i].nrj>phoswitches[id_j].nrj;
    });


// 2. Perform the add-back
    std::vector<bool> phoswitches_added(phoswitch_mult, false); // List to keep track of the hits that have been used for add-back already
    for (size_t ordered_it = 0; ordered_it<phoswitch_mult; ++ordered_it)
    {
      auto const & hit_i = hits_ordered[ordered_it]; // Starts with the highest energy deposit
      if (phoswitches_added[hit_i]) continue; // If this hit has already been used for add-back with a previous hit then discard it

      auto const & id_i = phoswitches_id[hit_i]; // The index of the detector in its cluster (see Paris and ParisCluster class)
      modules_id.push_back(id_i);
      modules[id_i].set(phoswitches[id_i]);

      // Test the other detectors in the event for a potential add-back :
      for (size_t ordered_loop_j = ordered_it+1; ordered_loop_j<phoswitch_mult; ordered_loop_j++)
      {
        auto const & hit_j = hits_ordered[ordered_loop_j];
        auto const & id_j = phoswitches_id[hit_j];

        // Timing : if they are not simultaneous then they don't belong to the same gamma-ray
        if (std::abs(phoswitches[id_j].time - phoswitches[id_i].time) > m_time_window) continue; 

        // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
        auto const & distance_ij = ParisCluster<paris_cluster_size>::distances[id_i][id_j];
        if (distance_ij > paris_distance_max) continue;

        // They pass both conditions, so we add-back them :
        modules[id_i].add(phoswitches[id_i]);
        phoswitches_added[hit_j] = true;
        phoswitches[id_j].rejected = true;
        phoswitches[id_i].rejected = true;
      }
    }
    module_mult = modules_id.size();
    m_addback = true;
    if (module_mult < phoswitch_mult) m_addback_used = true;
  }

  std::array<SimplePhoswitch, paris_cluster_size> phoswitches;
  std::array<SimpleParisModule, paris_cluster_size> modules;

  std::vector<Index> phoswitches_id;
  std::vector<Index> modules_id;

  size_t phoswitch_mult = 0;
  size_t module_mult = 0;

  float calorimetry = 0;
  void setDistanceMax(double const & _distance_max) {paris_distance_max = _distance_max;}
  void setTimeWindow(double const & _time_window) {m_time_window = _time_window;}
  auto const & isAddBack() const {return m_addback;}
  auto const & isAddBackUsed() const {return m_addback_used;}

private:
  bool m_addback = false;
  bool m_addback_used = false;
  Time m_time_window = 10_ns;
};
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
class SimpleParis
{
public:
  SimpleParis() {}

  SimpleParis(PhoswitchCalib* calib) : m_calib(calib) {}

  void fill(Event const & event, int const & hit_i, PhoswitchCalib const & calib)
  {
    auto const & label = event.labels[hit_i];
    auto const & cluster = Paris::cluster[label];
         if (cluster == 0) phoswitches.push_back(back.fill(event, hit_i, calib));
    else if (cluster == 1) phoswitches.push_back(front.fill(event, hit_i, calib));
    else error("SimpleParis::fill : no cluster found for label", label);
    if (phoswitches.back() == nullptr) phoswitches.pop_back();
  }

  void fill(Event const & event, int const & hit_i) {fill(event, hit_i, *m_calib);}

  void setEvent(Event const & event, PhoswitchCalib const & calib)
  {
    clear();
    for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    {
      if (Paris::is[event.labels[hit_i]]) this->fill(event, hit_i, calib);
    }
    addback();
  }

  void setEvent(Event const & event) {setEvent(event, *m_calib);}

  void operator=(Event const & event) {setEvent(event, *m_calib);}

  void addback()
  {
    back.addback();
    front.addback();
  }

  void analyze()
  {
    this->addback();
    std::sort(modules.begin(), modules.end(), [](SimpleParisModule const * p1, SimpleParisModule const * p2)
    {
      return p1->time < p2->time;
    });
    std::sort(phoswitches.begin(), phoswitches.end(), [](SimplePhoswitch const * p1, SimplePhoswitch const * p2)
    {
      return p1->time < p2->time;
    });
  }

  void clear()
  {
    back.clear();
    front.clear();
    modules.clear();
    phoswitches.clear();
  }
  
  int phoswitch_mult() const { return back.phoswitches_id.size() + front.phoswitches_id.size();}
  int module_mult() const { return back.modules_id.size() + front.modules_id.size();}
  float calorimetry() const {return back.calorimetry + front.calorimetry;}
  SimpleCluster back;
  SimpleCluster front;
  std::vector<SimpleParisModule*> modules;
  std::vector<SimplePhoswitch*> phoswitches;

private:
  PhoswitchCalib* m_calib;
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