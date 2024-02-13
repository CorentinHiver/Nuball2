#ifndef PARIS_HPP
#define PARIS_HPP

#include "../Classes/Event.hpp"
#include "../Classes/Detectors.hpp"
#include "../Classes/CoProgressBar.hpp"
#include "Arrays/Paris.h"
#include "ParisCluster.hpp"
#include "ParisBidimAngles.hpp"

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
  static std::pair<double, double> findAngles   (TH2F* bidim, int nb_bins = 10, bool write_graphs = false);
  static std::pair<double, double> findAnglesDeg(TH2F* bidim, int nb_bins = 10, bool write_graphs = false);
  static void calculateBidimAngles(std::string file, std::string const & outfilename, bool write_graphs = true);
  static TH2F* rotate(TH2F* bidim, double angleLaBr, double angleNaI, bool quickNdirty = false);

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

std::pair<double, double> Paris::findAngles(TH2F* bidim, int nb_bins, bool write_graphs)
{
  std::string outfilename = "angles_paris_bidim.root";
  auto const & nb_bins_long = bidim->GetNbinsX();
  auto const & nb_iterations_long = nb_bins_long/nb_bins;

  // auto const & nb_bins_short = bidim->GetNbinsY();
  // auto const & nb_iterations_short = nb_bins_short/nb_bins;

  std::vector<double> bins;
  std::vector<double> xbin_max_peak1;
  std::vector<double> xbin_max_peak2;

  auto const & maximum = bidim->GetMaximum();

  for (int it = 0; it<nb_iterations_long; it++)
  {

    // Peak finding : 
    auto const & bin_min = it*nb_bins;
    auto const & bin_max = it*(nb_bins+1);
    auto proj_short = std::make_unique<TH1D> (*(bidim->ProjectionX("temp_short", bin_min, bin_max)));
    auto const & max = proj_short->GetMaximum();
    if (max<maximum/20) continue;
    auto const & threshold_start = max/2;
    auto const & threshold_stop = max/5;

    int bin = 0;

      // First peak :
    auto begin_peak1 = findNextBinAbove(proj_short.get(), bin, threshold_start);
    bin+=3; // In order to avoid random threshold crossing
    auto end_peak1   = findNextBinBelow(proj_short.get(), bin, threshold_stop);
    bin+=3; 

      // Second peak :
    auto begin_peak2 = findNextBinAbove(proj_short.get(), bin, threshold_start);
    if (begin_peak2 > proj_short->GetNbinsX()) continue;
    bin+=3;
    auto end_peak2   = findNextBinBelow(proj_short.get(), bin, threshold_stop);
    bin+=3;

    // Do not take the peak if the end of the last peak is at the end of the spectra (i.e. not found)
    if (end_peak2+1 > proj_short->GetNbinsX()) continue;

    // Fitting peaks :
    auto const long_gate_ADC = bidim->GetYaxis()->GetBinCenter(it*(nb_bins+0.5));
    
    auto const & low_edge_peak1 = proj_short->GetBinCenter(begin_peak1*0.7);
    auto const & high_edge_peak1 = proj_short->GetBinCenter(end_peak1*1.3);
    auto const & low_edge_peak2 = proj_short->GetBinCenter(begin_peak2*0.7);
    auto const & high_edge_peak2 = proj_short->GetBinCenter(end_peak2*1.3);

    PeakFitter peak1(proj_short.get(), low_edge_peak1, high_edge_peak1);
    PeakFitter peak2(proj_short.get(), low_edge_peak2, high_edge_peak2);


    auto const & peak1_ADC = peak1.getMean();
    auto const & peak2_ADC = peak2.getMean();

    // Check that the two peaks are far away enough from one another
    if (peak1_ADC>peak2_ADC*0.5) continue;

    // Check that the current points are not too far away from interpolated values
    double mean_slope_peak1 = 0;
    double mean_slope_peak2 = 0;
    for (size_t bin_it = 0; bin_it<bins.size(); bin_it++)
    {
      mean_slope_peak1 += xbin_max_peak1[bin_it]/bins[bin_it];
      mean_slope_peak2 += xbin_max_peak2[bin_it]/bins[bin_it];
    }
    mean_slope_peak1 /= bins.size();
    mean_slope_peak2 /= bins.size();

    auto const & interpolated_peak1 = long_gate_ADC*mean_slope_peak1;
    auto const & interpolated_peak2 = long_gate_ADC*mean_slope_peak2;

    if (peak1_ADC > interpolated_peak1*1.1 || peak1_ADC < interpolated_peak1*0.9) continue;
    if (peak2_ADC > interpolated_peak2*1.1 || peak2_ADC < interpolated_peak2*0.9) continue;

    // Save the informations : 
    bins.push_back(long_gate_ADC);
    xbin_max_peak1.push_back(peak1_ADC);
    xbin_max_peak2.push_back(peak2_ADC);
  }

  // Plot the position of each peak as a function of the mean bin of the projection 
  // Fit the curve, extract the slope and calculate the angle :
  
  auto const & nb_points = bins.size();
  
  TF1* linear1(new TF1("_lin","pol1"));
  TF1* linear2(new TF1("_lin","pol1"));

  auto graph_peak1(new TGraph(nb_points, bins.data(), xbin_max_peak1.data()));
  graph_peak1->SetName(concatenate("First_peak_", bidim->GetName()).c_str());
  graph_peak1->SetTitle(concatenate("First peak ", bidim->GetName()).c_str());
  graph_peak1->Fit(linear1, "q");

  auto first_slope = linear1->GetParameter(1);
  auto const & first_angle = atan(first_slope);

  auto graph_peak2(new TGraph(nb_points, bins.data(), xbin_max_peak2.data()));
  graph_peak2->SetName(concatenate("Second_peak_", bidim->GetName()).c_str());
  graph_peak2->SetTitle(concatenate("Second peak ", bidim->GetName()).c_str());
  graph_peak2->Fit(linear2, "q");

  auto second_slope = linear2->GetParameter(1);
  auto const & second_angle = atan(second_slope);

  // Calculate and plot the residues of the fit :
  std::vector<double> residues_peak1;
  std::vector<double> residues_peak2;

  for (size_t bin_it = 0; bin_it<bins.size(); bin_it++)
  {
    residues_peak1.push_back(xbin_max_peak1[bin_it ]-first_slope*bins[bin_it]);
    residues_peak2.push_back(xbin_max_peak2[bin_it ]-second_slope*bins[bin_it]);
    // print(xbin_max_peak1[bin_it ], first_slope*bins[bin_it], xbin_max_peak1[bin_it ]-first_slope*bins[bin_it]);
  }
  // pauseCo();

  auto graph_residues_peak1(new TGraph(nb_points, bins.data(), residues_peak1.data()));
  graph_residues_peak1->SetName(concatenate("First_peak_", bidim->GetName(), "_residues").c_str());
  graph_residues_peak1->SetTitle(concatenate("First peak ", bidim->GetName(), " residues").c_str());

  auto graph_residues_peak2(new TGraph(nb_points, bins.data(), residues_peak2.data()));
  graph_residues_peak2->SetName(concatenate("Second_peak_", bidim->GetName(), "_residues").c_str());
  graph_residues_peak2->SetTitle(concatenate("Second peak ", bidim->GetName(), " residues").c_str());

  if (write_graphs)
  {
    // Write the graphs in a root file
    auto file = TFile::Open(outfilename.c_str(), "update");
    file -> cd();
    graph_peak1->Write();
    graph_peak2->Write();
    graph_residues_peak1->Write();
    graph_residues_peak2->Write();
    file->Write();
    file->Close();
  }

  return {first_angle, second_angle};
}


std::pair<double, double> Paris::findAnglesDeg(TH2F* bidim, int nb_bins, bool write_graphs)
{
  auto ret (findAngles(bidim, nb_bins, write_graphs));
  ret.first *=57.295718;
  ret.second*=57.295718;
  return ret;
}

void Paris::calculateBidimAngles(std::string filename, std::string const & outfilename, bool write_graphs)
{
  auto file(TFile::Open(filename.c_str(), "READ"));
  file -> cd();
  auto names(get_names_of<TH2F>(file));

  std::map<std::string, TH1D*> projs;
  ParisBidimAngles angles;

  if (File("angles_paris_bidim.root").exists()) 
  {
    print("rm angles_paris_bidim.root");
    system("rm angles_paris_bidim.root");
  }

  size_t avancement = 0;
  CoProgressBar prog(&avancement, names.size());

  for (auto const & name : names)
  {
    auto bidim = file->Get<TH2F>(name.c_str());
    auto const & nb_bins = 10;
    auto temp (Paris::findAngles(bidim, nb_bins, write_graphs));
    auto detector_name = name;
    remove(detector_name, "_bidim");
    angles.set(detector_name, temp.first, temp.second);
    ++avancement;
    prog.show();
  }
  print();
  file->Close();
  print("angles_paris_bidim.root written");
  angles.write(outfilename);
}

TH2F* Paris::rotate(TH2F* bidim, double angleLaBr, double angleNaI, bool quickNdirty)
{
  if (!bidim) throw_error("in Paris::rotate(TH2F* bidim, double angleLaBr, double angleNaI) : bidim is nullptr");
  if (quickNdirty) print("pas beau ...");

  auto rotated_bidim (static_cast<TH2F*>(bidim->Clone(concatenate(bidim->GetName(), "_rotated").c_str())));
  rotated_bidim->Reset();

  auto const & nb_binsx = bidim->GetNbinsX();
  auto const & nb_binsy = bidim->GetNbinsY();
  double _tan = tan(angleNaI);
  double _sin = sin(angleNaI);
  double _cos = cos(angleNaI);
  // double _tan = tan(angleLaBr);
  // double _sin = sin(angleLaBr);
  // double _cos = cos(angleLaBr);

  for (int bin_x = 0; bin_x<nb_binsx; bin_x++)
  {
    for (int bin_y = 0; bin_y<nb_binsy; bin_y++)
    {
      auto const & nb_hits   = bidim->GetBinContent(bin_x, bin_y);
      auto const & old_long  = bidim->GetYaxis()->GetBinCenter(bin_y);
      auto const & old_short = bidim->GetXaxis()->GetBinCenter(bin_x);

      // Reject LaBr3 events :
      auto const & pid = (old_long-old_short)/old_long;
      if (pid<0.1) continue;

      if (quickNdirty)
      {
        auto const & new_short = old_long*_sin + old_short*_cos*(abs(_tan)/_tan);
        auto const & new_long = old_long*_cos - old_short*_sin*(abs(_tan)/_tan);

        auto const & new_y = bidim->GetYaxis()->FindBin(new_long);
        auto const & new_x = bidim->GetYaxis()->FindBin(new_short);

        rotated_bidim->SetBinContent(new_x, new_y, quickNdirty);
      }
      else
      {
        auto const & old_long_range  = bidim->GetYaxis()->GetBinCenter(bin_y+1)-old_long ;
        auto const & old_short_range = bidim->GetXaxis()->GetBinCenter(bin_x+1)-old_short;
        for (int hit_i = 0; hit_i<nb_hits; hit_i++)
        {
          auto const & rand_short = old_short + double_random_uniform(0, old_short_range);
          auto const & rand_long  = old_long  + double_random_uniform(0, old_long_range);

          auto const & new_short = rand_short * _cos - rand_long * _sin; // * (abs(_tan)/_tan);
          auto const & new_long  = rand_short * _sin + rand_long * _cos; // * (abs(_tan)/_tan);

          rotated_bidim->Fill(new_short, new_long);
        }
      }


    }
  }
  return rotated_bidim;
}



#endif //PARIS_HPP
