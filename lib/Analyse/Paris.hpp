#ifndef PARIS_H
#define PARIS_H

#include "../Classes/Event.hpp"
#include "Arrays/Paris.h"
#include "ParisCluster.hpp"

PositionXY paris_getPositionModule(std::size_t const & module_label)
{
       if (module_label<8)  return PositionXY(Paris_R1_x[module_label],  Paris_R1_y[module_label]);
  else if (module_label<24) return PositionXY(Paris_R2_x[module_label-8],  Paris_R2_y[module_label-8]);
  else                      return PositionXY(Paris_R3_x[module_label-24],  Paris_R3_y[module_label-24]);
}

class Paris
{
public:
  // ________________________________________________________________ //
  // ------------------  Setting the lookup tables ------------------ //
  //  ---- From labels to index ---- //
  bool static is_paris(Label const & l)
  {
    return (l<paris_labels[0] || l>paris_labels.back())
           ? false
           : std::binary_search(paris_labels.begin(), paris_labels.end(), l);
  }

  uchar static label_to_index (Label const & l)
  {
    if (is_paris(l))
    {
      return static_cast<uchar> ( std::find(paris_labels.begin(), paris_labels.end(), l) - paris_labels.begin() );
    }
    else return -1;
  }

  static std::array<bool,  1000> is     ; // Does the label correspond to a Paris ?
  static std::array<uchar, 1000> cluster; // Link the label to its cluster (0 : back, 1 : front)
  static std::array<uchar, 1000> index  ; // Link the label to the module's index in the cluster

  // ---- Initialization of static arrays ---- //
  void static Initialize()
  {
    for (int l = 0; l<1000; l++)
    {
      is[l]  = is_paris(l);
      cluster[l] = static_cast<uchar> (l>500);
      index[l] = label_to_index(l)%(paris_labels.size()/2);
    }
  }
  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // -----------------------  Paris Class  ----------------------- //
  Paris(){}
  void Initialize(); // Parameters : the number of clusters, and the number of modules in each cluster
  void Fill(Event const & event, size_t const & i);
  void Reset();
  void Analyse();

  StaticVector<uchar> Hits;

  ParisCluster<28> clusterBack;
  ParisCluster<36> clusterFront;

  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // ------------------  User defined methods -------------------- //
  ParisCluster & back  () { return clusterBack;}
  ParisCluster & front () { return clusterFront;}
  // _____________________________________________________________ //

private:
  bool m_initialised = false;
};

std::array<bool,  1000> Paris::is;    // Does the label correspond to a Paris ?
std::array<uchar, 1000> Paris::cluster;// Link the label to its cluster (0 : back, 1 : front)
std::array<uchar, 1000> Paris::index  ;// Link the label to the module's index in the cluster

void Paris::Initialize(std::size_t const & nb_clusters, std::size_t const & nb_modules)
{// Parameters : number of clusters, number of modules in each cluster
  if (!m_initialised)
  {
    // for (auto & cluster : parisClusters)
    // {
    //   cluster.Initialize(nb_modules);
    //   for (std::size_t i = 0; i<nb_modules; i++)
    //   {
    //     cluster.positions[i] = paris_getPositionModule(i);
    //   }
    //   cluster.InitializeBidims();
    // }
    m_initialised = true;
  }
}

void inline Paris::Reset()
{
  for (auto & cluster : parisClusters) cluster.Reset();
}

void inline Paris::Fill(Event const & event, size_t const & i)
{
  parisClusters[cluster[event.labels[i]]] . Fill(event, i, index[event.labels[i]]);
}

void inline Paris::Analyse()
{
  for (auto & cluster : parisClusters) cluster.Analyse();
}

#endif //PARIS_H
