#ifndef PARIS_HPP
#define PARIS_HPP

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
  void static InitializeArrays()
  {
    if (!s_initialised)
    {
    #ifdef MTOBJECT_HPP
      lock_mutex(MTObject::mutex);
    #endif //MTOBJECT_HPP
      print("Initialising Paris arrays");
      for (int l = 0; l<1000; l++)
      {
        is[l]  = is_paris(l);
        cluster[l] = static_cast<uchar> (l>500);
        index[l] = label_to_index(l)%(paris_labels.size()/2);
      }
      s_initialised = true;
    }
  }
  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // -----------------------  Paris Class  ----------------------- //
  Paris(){this -> InitializeArrays();}
  void Initialize();
  void Fill(Event const & event, size_t const & i);
  void Reset();
  void Analyse();

  StaticVector<uchar> Hits;

  ParisCluster<28> clusterBack;
  ParisCluster<36> clusterFront;

  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // ------------------  User defined methods -------------------- //
  auto & back  () { return clusterBack;}
  auto & front () { return clusterFront;}
  // _____________________________________________________________ //

  // _____________________________________________________________ //
  // ------------ Qshort VS Qlong bidim manipulations ------------
  static void orthogonalise(TH2F* bidim);
  static void findAngles(TH2F* bidim, int nb_bins = 1);
  // _____________________________________________________________ //

private:
  static bool s_initialised;
  bool m_initialised = false;
};

bool Paris::s_initialised = false;

std::array<bool,  1000> Paris::is;     // Does the label correspond to a Paris ?
std::array<uchar, 1000> Paris::cluster;// Link the label to its cluster (0 : back, 1 : front)
std::array<uchar, 1000> Paris::index  ;// Link the label to the module's index in the cluster

void Paris::Initialize()
{// Parameters : number of clusters, number of modules in each cluster
  if (!m_initialised)
  {
    clusterBack.Initialize();
    clusterFront.Initialize();
    m_initialised = true;
  }
}

void inline Paris::Reset()
{
  clusterBack.Reset();
  clusterFront.Reset();
}

void inline Paris::Fill(Event const & event, size_t const & i)
{
  clusterBack.Fill(event, i, index[event.labels[i]]);
  clusterFront.Fill(event, i, index[event.labels[i]]);
}

void inline Paris::Analyse()
{
  clusterBack.Analyse();
  clusterFront.Analyse();
}

void Paris::findAngles(TH2F* bidim, int nb_bins = 1);
{
  auto const & nb_bins_long = bidim->GetNbinsX();
  auto const & nb_bins_short = bidim->GetNbinsY();

  auto const & nb_iterations_long = nb_bins_long/nb_bins;
  auto const & nb_iterations_short = nb_bins_short/nb_bins;

  std::vector<double> peaks_;

  for (int it = 0; it<nb_iterations_long; it++)
  {
    auto proj_long = std::make_unique<TH1F> (bidim->ProjectionX("temp_long"), it*nb_bins, it*(nb_bins+1));
    
  }
}

void Paris::orthogonalise(TH2F* bidim)
{

}


#endif //PARIS_HPP
