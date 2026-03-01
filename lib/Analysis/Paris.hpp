#pragma once

#include "Phoswitch.hpp"
#include "../Classes/Event.hpp"

namespace Paris
{
  class Module
  {
  public:
    Module() : m_index(g_index++) {}
  
    double nrj  = {};
    double time = {};
    Label  first_id = {};
   
    void set(Phoswitch const & phoswitch)
    {
      nrj = phoswitch.nrj;
      time = phoswitch.time;
      m_isLaBr3 = (phoswitch.crystal == Phoswitch::Fast);
      ++m_nb;
      first_id = phoswitch.index();
    }
  
    void add(Phoswitch const & phoswitch)
    {
      nrj += phoswitch.nrj;
      ++m_nb;
      m_isLaBr3 = m_isLaBr3 && (phoswitch.crystal == Phoswitch::Fast);
      // The phoswitch with the more energy deposit is set first in the current add-back method
      // Not updating time in the add method leads to keeping the time of this phoswitch, 
      // which is expected to have been the fastest AND the one the gamma-ray Comptoned in.
    }
  
    void clear()
    {
      m_isLaBr3 = {};
      nrj  = {};
      time = {};
      m_nb = {};
      first_id = {};
    }
  
    constexpr static void resetIndexes() noexcept {g_index = 0;}
  
    constexpr auto const & index  () const noexcept {return m_index  ;}
    constexpr auto const & nb     () const noexcept {return m_nb     ;}
    constexpr auto const & isLaBr3() const noexcept {return m_isLaBr3;}
  
    constexpr bool addbacked() const noexcept {return m_nb > 0;}
  
    friend std::ostream& operator<<(std::ostream& out, Module const & module)
    {
      out << "Id : " << module.index() << " nrj = " << module.nrj << " time = " << module.time;
      return out;
    }
  
  private:
    static inline thread_local Label g_index = {};
    Label mutable m_index;
  
    bool     m_isLaBr3 = {};
    uint16_t m_nb      = {};
  };

  template<size_t size>
  class Cluster
  {
  public:
    Cluster() : m_index(g_index++)
    {
      Phoswitch::resetIndexes();
      Module::resetIndexes();
      // for (auto & phoswitch : phoswitches) phoswitch.label = ParisArrays::labels[m_index * size + phoswitch.index()];
    };

    Phoswitch& fill(Event const & event, int const & hit_i)
    {
      auto const & label = event.labels[hit_i];
      auto const & id    = Paris::cluster_index[label];
      auto const & nrj   = event.nrjs[label];
      auto const & nrj2  = event.nrj2s[label];

      // if (nrj < 0 || nrj2 < 0) return nullptr;

      // if (size < id+1) {error("in Cluster::fill : index", id, "> cluster_size !!"); return nullptr;}

      // phoswitches_id.push_back(id);
      
      // auto ret = phoswitches[id].fill(event, hit_i);

      // calorimetry+=phoswitches[id].nrj;

      return phoswitches[id];
    }

    // Phoswitch* fill(Event const & event, int const & hit_i, Phoswitch::RotationCalib const & calib)
    // Phoswitch* fill(Event const & event, int const & hit_i)
    // {
      // auto const & label = event.labels[hit_i];
      // auto const & id = Paris::cluster_index[label];
      // auto const & nrj = Paris::cluster_index[label];
      // auto const & nrj2 = Paris::cluster_index[label];

      // if (nrj < 0 || nrj2 < 0) return nullptr;

      // if (size < id+1) {error("in Cluster::fill : index", id, "> cluster_size !!"); return nullptr;}

      // phoswitches_id.push_back(id);
      
      // auto ret = phoswitches[id].fill(event, hit_i, calib);

      // calorimetry+=phoswitches[id].nrj;

      // return ret;
    // }

    void clear()
    {
      // for(auto const & id : phoswitches_id) phoswitches[id].clear();
      // phoswitches_id.clear();
      // // for(auto const & id : modules_id) modules[id].clear();
      // for(auto & module : modules) module.clear();
      // modules_id.clear();
      // phoswitch_mult = 0;
      // module_mult = 0;
      // m_addback = false;
      // m_addback_used = false;
      // calorimetry = 0;
    }

    void addback()
    {
  //     phoswitch_mult = phoswitches_id.size();

  //     if (phoswitch_mult==0) return; 
  //     else if (phoswitch_mult==1)
  //     {// Addback algorithm not necessary if there is only one phoswitch in the cluster
  //       auto const & id_i = phoswitches_id[0];
  //       modules_id.push_back(id_i);
  //       modules[id_i].set(phoswitches[id_i]);
  //       return;
  //     }

  //     // 1. Order the hits from the highest to lowest energy deposit :
  //     std::vector<size_t> hits_ordered(phoswitch_mult);
  //     std::iota(hits_ordered.begin(), hits_ordered.end(), 0);
  //     std::sort(hits_ordered.begin(), hits_ordered.end(), [&] (int const & hit_i, int const & hit_j)
  //     {
  //       auto const & id_i = phoswitches_id[hit_i];
  //       auto const & id_j = phoswitches_id[hit_j];
  //       return phoswitches[id_i].nrj > phoswitches[id_j].nrj;
  //     });

  // // 2. Perform the add-back
  //     for (size_t ordered_loop_i = 0; ordered_loop_i<phoswitch_mult; ++ordered_loop_i)
  //     {
  //       auto const & hit_i = hits_ordered[ordered_loop_i]; // Starts with the highest energy deposit
  //       auto const & id_i = phoswitches_id[hit_i]; // The index of the detector in its cluster (see ParisCluster class)

  //       if (phoswitches[id_i].rejected) continue; // If this hit has already been used for add-back with a previous hit then discard it
        
  //       modules[id_i].set(phoswitches[id_i]);
  //       modules_id.push_back(id_i);

  //       // Test the other detectors in the event for a potential add-back :
  //       for (size_t ordered_loop_j = ordered_loop_i+1; ordered_loop_j<phoswitch_mult; ++ordered_loop_j)
  //       {
  //         auto const & hit_j = hits_ordered[ordered_loop_j];
  //         auto & id_j = phoswitches_id[hit_j];
          
  //         // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
  //         auto const & distance_ij = Paris::distances[id_i][id_j];
  //         if (distance_ij > Paris::distance_max) continue;

  //         // Timing : if the hits are not simultaneous then they don't belong to the same gamma-ray
  //         if (std::abs(phoswitches[id_j].time - phoswitches[id_i].time) > m_time_window) continue; 

  //         // They pass both conditions, so we add them back :
  //         modules[id_i].add(phoswitches[id_j]);

  //         // If they are add-backed, it means we can reject the individual phoswitches used in the procedure :
  //         phoswitches[id_j].rejected = true;
  //         phoswitches[id_i].rejected = true;

  //         m_addback_used = true;
  //       }
  //     }
  //     module_mult = modules_id.size();
  //     m_addback = true;
    }

    std::array<Phoswitch, size> phoswitches;
    std::array<Module, size> modules;

    std::vector<Index> phoswitches_id;
    std::vector<Index> modules_id;

    size_t phoswitch_mult = 0;
    size_t module_mult = 0;

    float calorimetry = 0;
    // static void setDistanceMax(double const & _distance_max) {Paris::distance_max = _distance_max;}
    void setTimeWindow(double const & _time_window) {m_time_window = _time_window;}
    auto const & isAddBack() const {return m_addback;}
    auto const & isAddBackUsed() const {return m_addback_used;}

    static void resetIndexes() noexcept {g_index = 0;}

    friend std::ostream& operator<<(std::ostream& out, Cluster const & cluster)
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

  private:
    bool m_addback = false;
    bool m_addback_used = false;
    Time m_time_window = 4_ns;
    
    Index inline static thread_local g_index= 0;
    Index const m_index;
  };
}