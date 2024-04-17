#ifndef PARIS_CLUSTER_HPP
#define PARIS_CLUSTER_HPP

#include "../libRoot.hpp"
#include "ParisPhoswitch.hpp"
#include "PositionXY.hpp"

template<std::size_t nb_phoswitch>
class ParisCluster
{
public:
  // ParisCluster() {}
  // ParisCluster() : m_label(gLabel.fetch_add(1)) {this -> Initialise();}
  ParisCluster() : m_label() {gLabel++; this -> Initialise();}
  void Initialise();
  void InitialiseBidims();
  void Reset();
  void Fill(Event const & event, int const & i, uchar const & label);
  void Analyse();
  auto const & label() const {return m_label;}
  auto const & size() const {return nb_phoswitch;}

  ParisPhoswitch & operator[] (int const & i) {return phoswitches[i];}

  std::vector<ParisPhoswitch>  phoswitches = std::vector<ParisPhoswitch>(nb_phoswitch);
  // std::vector<ParisModule>  modules; // On va lui emplace_back des nouveaux modules - à voir niveau efficacité
  // std::vector<PositionXY>   positions;
  // std::vector<uchar>        nb_neighbors;

  // std::vector<std::vector<Float_t>> distances;

  // std::array<uchar, nb_phoswitch> Hits;
  StaticVector<uchar> Hits        = StaticVector<uchar>(nb_phoswitch);
  StaticVector<uchar> hits_LaBr3  = StaticVector<uchar>(nb_phoswitch);
  StaticVector<uchar> hits_NaI    = StaticVector<uchar>(nb_phoswitch);
  // StaticVector<uchar> hits_no_neighbors;
  // StaticVector<uchar> hits_has_neighbors;
  // StaticVector<uchar> first_cleaned_LaBr3;
  // StaticVector<uchar> hits_neighbors_max_E;

  static auto const & nb_clusters() {return gLabel;}

private:
  uchar const m_label;
  // static thread_local std::atomic<uchar> gLabel;
  static thread_local uchar gLabel;
};

template<std::size_t nb_phoswitch>
// thread_local std::atomic<uchar> ParisCluster<nb_phoswitch>::gLabel = 0;
thread_local uchar ParisCluster<nb_phoswitch>::gLabel = 0;

template<std::size_t nb_phoswitch>
void ParisCluster<nb_phoswitch>::Initialise()
{
  int i = 0;
  for (auto & phoswitch : phoswitches) phoswitch.setLabel(i++);
  // Initialise the containers
  // LaBr3s.   resize(nb_phoswitch);
  // NaIs.     resize(nb_phoswitch);
  // positions.resize(nb_phoswitch);
  // nb_neighbors.resize(nb_phoswitch, 0);

  // Initialise the bidim containers
  // distances.resize(nb_phoswitch);
  // for (auto & distance : distances) distance.resize(nb_phoswitch, 0.);

  // Initialise the readers
  // Hits.      static_resize(nb_phoswitch);
  // hits_no_neighbors.  static_resize(nb_phoswitch);
  // hits_has_neighbors.  static_resize(nb_phoswitch);
  // hits_neighbors_max_E.static_resize(nb_phoswitch);
  // first_cleaned_LaBr3.static_resize(nb_phoswitch);
}

template<std::size_t nb_phoswitch>
void ParisCluster<nb_phoswitch>::InitialiseBidims()
{
  // for (std::size_t i = 0; i<positions.size(); i++)
  // {
  //   for (std::size_t j = i+1; j<positions.size(); j++)
  //   {
  //     distances[i][j] = PositionXY::distance(positions[i], positions[j]);
  //     distances[j][i] = PositionXY::distance(positions[i], positions[j]);
  //   }
  // }
}

template<std::size_t nb_phoswitch>
void ParisCluster<nb_phoswitch>::Reset()
{
  // for (auto const & index : Hits) phoswitches[index].Reset();
  for (auto & phoswitch : phoswitches) phoswitch.Reset();
  Hits.clear();
  hits_LaBr3.clear();
  hits_NaI.clear();
  // hits_neighbors_max_E.resize();
  // hits_no_neighbors.resize();
  // for (auto const & index : hits_has_neighbors) nb_neighbors[index] = 0;
  // hits_has_neighbors.resize();
}

template<std::size_t nb_phoswitch>
void inline ParisCluster<nb_phoswitch>::Fill(Event const & event, int const & i, uchar const & index)
{
  // print((int)event.labels[i], (int)index);
  phoswitches[index].Fill(event, i);
  // Hits.push_back(index);
  // hits_LaBr3.push_back(index);
  // hits_NaI.push_back(index);
}

template<std::size_t nb_phoswitch>
void ParisCluster<nb_phoswitch>::Analyse()
{
  /*
  for (std::size_t loop_i = 0; loop_i<Hits.size(); loop_i++)
  {
    auto const & index_i = Hits[loop_i];
    auto const & module_i = phoswitches[index_i];

    for (std::size_t loop_j = loop_i+1; loop_j<Hits.size(); loop_j++)
    {
      auto const & index_j = Hits[loop_j];

      auto const & module_j = phoswitches[index_j];
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
    // ((nb_neighbors[index_i]==0) ? hits_no_neighbors : hits_has_neighbors).push_back(index_i);
  } // End i loop
  */
}

#endif //PARIS_CLUSTER_HPP
