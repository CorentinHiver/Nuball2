#ifndef PARISCLUSTER_H
#define PARISCLUSTER_H

#include "../utils.hpp"
#include "ParisModule.hpp"
#include "PositionXY.hpp"

using ParisIndex = uchar;

class ParisCluster
{
public:
  ParisCluster() {}
  ParisCluster(std::size_t const & nb_modules) : m_size(nb_modules), m_label(glabel) {glabel++; this -> Initialize(nb_modules);}
  void Initialize(std::size_t const & nb_modules);
  void InitializeBidims();
  void Reset();
  void Fill(Event const & event, int const & i, ParisIndex const & label);
  void Analyse();
  auto const & label() const {return m_label;}
  auto const & size() const {return m_size;}

  ParisModule & operator[] (int const & i) {return modules[i];}

  std::vector<ParisModule>  modules;
  std::vector<ParisCrystal> LaBr3s;
  std::vector<ParisCrystal> NaIs;
  std::vector<PositionXY>   positions;
  std::vector<uchar>        nb_neighbors;

  std::vector<std::vector<Float_t>> distances;

  StaticVector<ParisIndex> Hits;
  StaticVector<ParisIndex> hits_LaBr3;
  StaticVector<ParisIndex> hits_NaI;
  StaticVector<ParisIndex> hits_no_neighbors;
  StaticVector<ParisIndex> hits_has_neighbors;
  StaticVector<ParisIndex> first_cleaned_LaBr3;
  StaticVector<ParisIndex> hits_neighbors_max_E;

  static auto const & nb_clusters() {return glabel;}

private:
  std::size_t m_size;
  uchar m_label;
  static uchar glabel;
};

uchar ParisCluster::glabel = 0;

void ParisCluster::Initialize(std::size_t const & nb_modules)
{
  ParisModule::newCluster();

  // Initialize the containers
  modules.  resize(nb_modules);
  LaBr3s.   resize(nb_modules);
  NaIs.     resize(nb_modules);
  positions.resize(nb_modules);
  nb_neighbors.resize(nb_modules, 0);

  // Initialize the bidim containers
  distances.resize(nb_modules);
  for (auto & distance : distances) distance.resize(nb_modules, 0.);

  // Initialize the readers
  Hits.      static_resize(nb_modules);
  hits_LaBr3.static_resize(nb_modules);
  hits_NaI.  static_resize(nb_modules);
  hits_no_neighbors.  static_resize(nb_modules);
  hits_has_neighbors.  static_resize(nb_modules);
  hits_neighbors_max_E.static_resize(nb_modules);
  first_cleaned_LaBr3.static_resize(nb_modules);
}

void ParisCluster::InitializeBidims()
{
  for (std::size_t i = 0; i<positions.size(); i++)
  {
    for (std::size_t j = i+1; j<positions.size(); j++)
    {
      distances[i][j] = PositionXY::distance(positions[i], positions[j]);
      distances[j][i] = PositionXY::distance(positions[i], positions[j]);
    }
  }
}

void ParisCluster::Reset()
{
  for (auto const & index : Hits) modules[index].Reset();
  Hits.resize();
  hits_LaBr3.resize();
  hits_NaI.resize();
  hits_neighbors_max_E.resize();
  hits_no_neighbors.resize();
  for (auto const & index : hits_has_neighbors) nb_neighbors[index] = 0;
  hits_has_neighbors.resize();
  hits_NaI.resize();
}

void inline ParisCluster::Fill(Event const & event, int const & i, ParisIndex const & index)
{
  modules[index].Fill(event, i);
  Hits.push_back(index);
}

void ParisCluster::Analyse()
{
  for (std::size_t loop_i = 0; loop_i<Hits.size(); loop_i++)
  {
    auto const & index_i = Hits[loop_i];
    auto const & module_i = modules[index_i];
    for (std::size_t loop_j = loop_i+1; loop_j<Hits.size(); loop_j++)
    {
      auto const & index_j = Hits[loop_j];

      auto const & module_j = modules[index_j];
      auto const & distance = distances[index_i][index_j];
      if (abs(module_j.time-module_i.time) > 50.) continue; // Timing : if they are not simultaneous then they don't belong to the same event
      if (distance<1.3)
      {// If neighbors :
        nb_neighbors[index_i]++;
        if (module_i.labr3)
        {
          if (module_j.labr3)
          {// If both are labr3 :
            // auto const & nrj_i = module_i.labr3.energy;
            // auto const & nrj_j = module_j.labr3.energy;
            // auto const & label_max_E
            // hits_neighbors_max_E.[ (nrj_i>nrj_j) ?  ]
          }
        }
      }
    } // End j loop
    ((nb_neighbors[index_i]==0) ? hits_no_neighbors : hits_has_neighbors).push_back(index_i);
  } // End i loop
}

#endif //PARISCLUSTER_H
