#ifndef PARIS_HPP
#define PARIS_HPP

#include "../Classes/Event.hpp"
#include "../Classes/Detectors.hpp"
#include "../Classes/CoProgressBar.hpp"
std::recursive_mutex intialisation_mutex;
#include "Arrays/Paris.h"
#include "ParisCluster.hpp"
#include "ParisBidimAngles.hpp"

// deprecated ??
// PositionXY paris_getPositionModule(std::size_t const & module_label)
// {
//        if (module_label<8)  return PositionXY(ParisArrays::Paris_R1_x[module_label]   ,  ParisArrays::Paris_R1_y[module_label]);
//   else if (module_label<24) return PositionXY(ParisArrays::Paris_R2_x[module_label-8] ,  ParisArrays::Paris_R2_y[module_label-8]);
//   else                      return PositionXY(ParisArrays::Paris_R3_x[module_label-24],  ParisArrays::Paris_R3_y[module_label-24]);
// }

class Paris
{
public:
  constexpr static uchar cluster_size = 36ul;
  constexpr static double distance_max = 2.;
  // ________________________________________________________________ //
  // ------------------  Setting the lookup tables ------------------ //
  //  ---- From labels to index ---- //
  bool static is_paris(Label const & l)
  {
    auto const & labels = ParisArrays::paris_labels;
    return (l<labels[0] || l>labels.back())
           ? false
           : std::binary_search(labels.begin(), labels.end(), l);
  }

  /// @brief From a given label, returns its index in the ParisArrays::paris_labels array (lib/Analyse/Arrays/Paris.h)
  uchar static label_to_index (Label const & l)
  {
    auto const & labels = ParisArrays::paris_labels;
    if (is_paris(l))
    {
      return static_cast<uchar> ( std::find(labels.begin(), labels.end(), l) - labels.begin() );
    }
    else return -1;
  }

  static Label label(uchar const & cluster, uchar const & index)
  {
    if (index>cluster_size) {error (index,"> cluster size (=",cluster_size,")"); return -1;}
    
    uchar const & paris_index = (cluster)*cluster_size+index;

    if (paris_index>ParisArrays::paris_labels.size()){error (index,"> paris number of labels (=",ParisArrays::paris_labels.size(),")"); return -1;}
    
    return ParisArrays::paris_labels[paris_index];
  }

  static std::array<bool,  1000> is     ; // Does the label correspond to a Paris ?
  static std::array<uchar, 1000> cluster; // Link the label to its cluster (0 : back, 1 : front)
  static std::array<uchar, 1000> index  ; // Link the label to the module's index in the cluster

  // ---- Initialization of static arrays ---- //
  static void InitialiseArrays()
  {
  #ifdef MULTITHREADING 
    std::lock_guard<std::recursive_mutex> lock(intialisation_mutex);
  #endif //MULTITHREADING
    if (!s_initialised)
    {
    #ifdef MULTITHREADING
      print("Initialising Paris arrays in thread", MTObject::getThreadIndex());
    #else
      print("Initialising Paris arrays");
    #endif //MULTITHREADING
      for (int l = 0; l<1000; l++)
      {
        is[l]  = is_paris(l);
        print(is[l], is_paris(l));
        cluster[l] = static_cast<uchar> (l>500);
        index[l] = label_to_index(l)%cluster_size;
      }
      ParisCluster<cluster_size>::InitialiseBidims();
      
      s_initialised = true;
    }
  }
  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // -----------------------  Paris Class  ----------------------- //
  Paris(){this -> InitialiseArrays();}
  void Initialise();
  void fill(Event const & event, size_t const & hit_i);
  void setEvent(Event const & event);
  void reset();
  void clear(){reset();};
  void analyze();
  auto NaI_calorimetry() {return clusterBack.NaI_calorimetry + clusterFront.NaI_calorimetry;}
  auto LaBr3_calorimetry() {return clusterBack.LaBr3_calorimetry + clusterFront.LaBr3_calorimetry;}
  
  StaticVector<uchar> Hits;

  ParisCluster<cluster_size> clusterBack;
  ParisCluster<cluster_size> clusterFront;

  // ______________________________________________________________ //

  // _____________________________________________________________ //
  // ------------------  User defined methods -------------------- //
  auto & back  () { return clusterBack;}
  auto & front () { return clusterFront;}
  // _____________________________________________________________ //

  // _____________________________________________________________ //
  // ------------ Qshort VS Qlong bidim manipulations ------------
  // static std::pair<double, double> findAngles   (TH2F* bidim, int nb_bins = 10, bool write_graphs = false);
  // static std::pair<double, double> findAnglesDeg(TH2F* bidim, int nb_bins = 10, bool write_graphs = false);
  // static void calculateBidimAngles(std::string file, std::string const & out_filename, bool write_graphs = true);
  static TH2F* rotate(TH2F* bidim, double angle);
  static void rotate(Hit & hit, double angle) noexcept
  {
    auto const & _cos = cos(angle);
    auto const & _sin = sin(angle);
    auto const new_nrj  = hit.nrj * _cos - hit.nrj2 * _sin;
    auto const new_nrj2 = hit.nrj * _sin + hit.nrj2 * _cos;
    hit.nrj = new_nrj;
    hit.nrj2 = new_nrj2;
  }
  static void rotate(Hit & hit, double const & _cos, double const & _sin) noexcept
  {
    auto const new_nrj = hit.nrj * _cos - hit.nrj2 * _sin;
    auto const new_nrj2 = hit.nrj * _sin + hit.nrj2 * _cos;
    hit.nrj  = new_nrj;
    hit.nrj2 = new_nrj2;
  }

  static std::pair<double, double> rotate(double const & nrj, double const & nrj2, double const & angle)
  {
    auto const & _cos = cos(angle);
    auto const & _sin = sin(angle);
    return {
      nrj * _cos - nrj2 * _sin,
      nrj * _sin + nrj2 * _cos
    };
  }

  static std::pair<double, double> rotate(double const & nrj, double const & nrj2, double const & _cos, double const & _sin)
  {
    return {
      nrj * _cos - nrj2 * _sin,
      nrj * _sin + nrj2 * _cos
    };
  }

  // _____________________________________________________________ //

private:
  static bool s_initialised;
  bool m_initialised = false;
};

bool Paris::s_initialised = false;

std::array<bool,  1000> Paris::is;     // Does the label correspond to a Paris ?
std::array<uchar, 1000> Paris::cluster;// Link the label to its cluster (0 : back, 1 : front)
std::array<uchar, 1000> Paris::index  ;// Link the label to the module's index in the cluster

void Paris::Initialise()
{// Parameters : number of clusters, number of modules in each cluster
  if (!m_initialised)
  {
    clusterBack.Initialise();
    clusterFront.Initialise();
    m_initialised = true;
  }
}

void inline Paris::reset()
{
  clusterBack.reset();
  clusterFront.reset();
}

void inline Paris::setEvent(Event const & event)
{
  this->reset();
  for (int hit_i = 0; hit_i <event.mult; ++hit_i) if (Paris::is[event.labels[hit_i]]) this->fill(event, hit_i);
  this->analyze();
}

void inline Paris::fill(Event const & event, size_t const & hit_i)
{
  auto const & cluster_i = Paris::cluster[event.labels[hit_i]];
  if (cluster_i == 0) clusterBack .fill(event, hit_i, index[event.labels[hit_i]]);
  else                clusterFront.fill(event, hit_i, index[event.labels[hit_i]]);
}

void inline Paris::analyze()
{
  clusterBack .analyze();
  clusterFront.analyze();
}


/**
 * In the following, we treat Paris Qlong VS Qshort calibration plots in order to obtain the angles necessary for rotation or orthogonalization.
 * This has successfully been used with 22Na source so far.
 * The first step is to click on the main peaks (511 and 1274 for 22Na), which writes a ParisAnglesFile
 * Reading this file allows one to extract :
 *  the angle between the abscises axis and the LaBr3(CeBr3) diagonal
 *  the angle between the abscises axis and the NaI diagonal
 *  the angle between the internal addback anti-diagonal and the LaBr3(CeBr3) diagonal
 * 
 * The first two are used in the orthogonalization method (not written yet), the third is the angle for rotation method 
 * 
 * Check the rotation method result on the qlong vs qshort plot using rotate() function
 * 
 * Rotate and project on Qlong axis allows for a "phoswitch" calibration.
 */

using Point = std::pair<double, double>;
Point selectPoint(TH1* histo, std::string const & instructions)
{
  double x = 0; double y = 0;
  gPad->SetTitle(instructions.c_str());
  histo->SetTitle(instructions.c_str());
  GetPoint(gPad->cd(), x, y);
  gPad->Update();
  return {x, y};
}
/// @brief A list of LaBr3 and NaI full energy peaks' points in the Qlong VS Qshort calibration plots
class ParisPoints
{
public:
  ParisPoints() noexcept = default;


  void addPoint(int const & peak, Point const & _LaBr3, Point const & _NaI)
  {
    peaks.push_back(peak);
    LaBr3[peak] = _LaBr3;
    NaI[peak] = _NaI;
  }

  void click_bidim(TH2* paris_bidim, std::vector<int> _peaks)
  {
    paris_bidim->SetMinimum(2);
    paris_bidim->Draw("colz");
    gPad->SetLogz(true);
    auto xaxis = paris_bidim->GetXaxis();
    auto yaxis = paris_bidim->GetYaxis();
    //1. Loop trough the peaks :
    for (auto const & peak : _peaks)
    {
      //2. First the LaBr3 peak :
      auto LaBr3_point = selectPoint(paris_bidim, concatenate("Click on the ", peak, " keV peak in the LaBr3 diagonal to zoom on it"));
      xaxis->SetRangeUser(LaBr3_point.first-(xaxis->GetXmax()-xaxis->GetXmin())/10, LaBr3_point.first+(xaxis->GetXmax()-xaxis->GetXmin())/10);
      yaxis->SetRangeUser(LaBr3_point.second-(yaxis->GetXmax()-yaxis->GetXmin())/10, LaBr3_point.second+(yaxis->GetXmax()-yaxis->GetXmin())/10);
      LaBr3_point = selectPoint(paris_bidim, concatenate("Select the ", peak, " keV peak in the LaBr3 diagonal"));
      xaxis->UnZoom();
      yaxis->UnZoom();

      auto NaI_point = selectPoint(paris_bidim, concatenate("Click on the ", peak, " keV peak in the NaI diagonal to zoom on it"));
      xaxis->SetRangeUser(NaI_point.first-(xaxis->GetXmax()-xaxis->GetXmin())/10, NaI_point.first+(xaxis->GetXmax()-xaxis->GetXmin())/10);
      yaxis->SetRangeUser(NaI_point.second-(yaxis->GetXmax()-yaxis->GetXmin())/10, NaI_point.second+(yaxis->GetXmax()-yaxis->GetXmin())/10);
      NaI_point = selectPoint(paris_bidim, concatenate("Select the ", peak, " keV peak in the NaI diagonal"));
      xaxis->UnZoom();
      yaxis->UnZoom();

      addPoint(peak, LaBr3_point, NaI_point);
    }
  }

  auto size() const {return peaks.size();}

std::map<int, Point> LaBr3;
std::map<int, Point> NaI;
std::vector<int> peaks;
};

std::ostream& operator<<(std::ostream& out, ParisPoints const & points)
{
  for (auto const & peak : points.peaks) 
  {
    auto const & labr = points.LaBr3.at(peak);
    auto const & nai = points.NaI.at(peak);
    out << " " << labr.first << " " << labr.second << " " << nai.first << " " << nai.second;
  }
  return out;
}

/// @brief List of ParisPoints for all the detectors. Includes an read/write to files system
class ParisPointsFile
{
public:
  ParisPointsFile() noexcept = default;
  ParisPointsFile(std::string const & filename) {read(filename);}

  /// @brief Need a bit more work to be fully functional...
  /// @param filename 
  void write(std::string const & filename)
  {
    std::ofstream outfile(filename, std::ios::out);
    for (auto const & it : list_points) 
    {
      auto const & label = it.first;
      auto const & points = it.second;
      outfile << label << points << std::endl;
    }
    outfile.close();
  }

  void read(std::string const & filename)
  {
    std::ifstream file(filename, std::ios::in);
    if (!file.good()) throw_error(concatenate("in ParisPointsFile::ParisPointsFile(std::string filename) : file ", filename, " can't be open"));
    std::string line;
    // Header : 
    std::getline(file, line);
    header = line;
    handle_header();
    print(header_vec);
    
    while(std::getline(file, line)) 
    {
      ParisPoints points;
      std::istringstream ss(line);
      Label label = 0;
      ss >> label;
      double LaBr_x = 0; double LaBr_y = 0; double NaI_x = 0; double NaI_y = 0;
      int peak_i = 0; 
      while (ss >> LaBr_x >> LaBr_y >> NaI_x >> NaI_y) 
      {
        points.addPoint(energies[peak_i], Point({LaBr_x, LaBr_y}), Point({NaI_x, NaI_y}));
        ++peak_i;
      }
      list_points.emplace(label, points);
    }
    file.close();
  }

  void handle_header()
  {
    std::istringstream ss(header);
    std::string LaBr_x;
    std::string LaBr_y;
    std::string NaI_x;
    std::string NaI_y;
    ss >> LaBr_x; // The first column is always "labels", so skip it (dummy line)
    while(ss >> LaBr_x >> LaBr_y >> NaI_x >> NaI_y )
    {
      energies.push_back(std::stoi(split(LaBr_x, '_')[0]));
      header_vec.push_back(LaBr_x);
      header_vec.push_back(LaBr_y);
      header_vec.push_back(NaI_x);
      header_vec.push_back(NaI_y);
    }
  }

  auto begin() {return list_points.begin();}
  auto end() {return list_points.end();}

  std::string header;
  std::vector<std::string> header_vec;
  std::vector<double> energies;
  std::map<Label, ParisPoints> list_points;
};

std::ostream& operator<<(std::ostream& out, ParisPointsFile const & points)
{
  for (auto const & det : points.list_points) out << det;
  return out;
}

void clickAddbackPoints(std::string filename, std::vector<int> peaks = {511, 1274})
{
  detectors.load("index_129.list");
  auto file = TFile::Open(filename.c_str(), "read");
  file->cd();
  std::ofstream outfilePoints("NaI_136_2024.points");

  outfilePoints << "label ";
  for (auto peak : peaks) outfilePoints << peak << "_LaBr_x " << peak << "_LaBr_y " << peak << "_NaI_x " << peak << "_NaI_y ";
  outfilePoints << std::endl;

  auto map = get_map_histo<TH2F*>(file, "TH2F");
  for (auto const & pair : map)
  {
    auto name = pair.first;
    auto det_name = removeLastPart(name, '_');
    auto const & label = detectors[det_name];
    auto histo = pair.second;
    ParisPoints points;
    points.click_bidim(histo, peaks);

    std::stringstream outputPoints;
    outputPoints << label << " " << points << std::endl;
    print(outputPoints.str());
    outfilePoints<<outputPoints.str();
  }
  outfilePoints.close();
  print("NaI_136_2024.points written");
}

class PhoswitchCalib
{public:
  PhoswitchCalib() noexcept = default;
  PhoswitchCalib(std::string const & filename) {read(filename);}

  void read(std::string const & filename = "NaI_136_2024.angles")
  {
    std::ifstream file(filename, std::ios::in);
    if (!file.good()) throw_error(concatenate("in PhoswitchCalib::PhoswitchCalib(std::string filename) : file", filename, " can't be open"));
    Label label = 0; double angle = 0; double coeff = 0;
    while(file >> label >> angle >> coeff) data.emplace(label, Coefficients({angle, coeff}));
    file.close();
    prepare();
  }

  void write(std::string const & filename = "NaI_136_2024.angles")
  {
    std::ofstream file(filename, std::ios::out);
    for (auto const & it : data) file << it.first << " " << it.second.angle << " " << it.second.coeff << std::endl;
    file.close();
    print(filename, "written");
  }

  void prepare() {for (auto & it : data) it.second.prepare();}

  double calibrate(Hit & hit) {return data.at(hit.label).calibrate(hit);}
  double calibrate(Label const & label, double const & qshort, double const & qlong) {return data.at(label).calibrate(qshort, qlong);}
  inline double calibrate(Label const & label, double const & qshort, double const & qlong) const {return data.at(label).calibrate(qshort, qlong);}

  class Coefficients
  {public:
    Coefficients(std::initializer_list<double> const & inputs)
    {
      if (inputs.size() == 2) 
      {
        auto it = inputs.begin();
        angle = *it;
        coeff = *(++it);
      }
    }

    Coefficients(std::pair<double, double> const & input)
    {
      angle = input.first;
      coeff = input.second;
    }

    Coefficients & operator=(std::initializer_list<double> const & inputs)
    {
      if (inputs.size() == 2) 
      {
        auto it = inputs.begin();
        angle = *it;
        coeff = *(++it);
      }
      return *this;
    }

    void prepare()
    {
      cos_a = cos(angle);
      sin_a = sin(angle);
    }

    double calibrate(Hit const & hit) {return (hit.nrj * sin_a + hit.nrj2 * cos_a) * coeff;}
    double calibrate(double const & qshort, double const & qlong) {return (qshort * sin_a + qlong * cos_a) * coeff;}
    double calibrate(double const & qshort, double const & qlong) const {return (qshort * sin_a + qlong * cos_a) * coeff;}

    double angle = 0;
    double cos_a = 0;
    double sin_a = 0;
    double coeff = 0;
  };

  std::map<Label, Coefficients> data;
};

// double getAddbackAngle(ParisPoints const & points)
// {
//   double ret = 0;
//   for (auto const & peak : points.peaks) ret += points.slope(peak);
//   return ret/double_cast(points.size());
// }
void createAddbackAngleFile(std::string const & parisPoints_filename = "NaI_136_2024.points")
{
  PhoswitchCalib angles_rotation;
  ParisPointsFile all_points(parisPoints_filename);
  for (auto const & it : all_points)
  {
    auto const & label = it.first;
    auto const & points_label = it.second;
    double nb_peaks = points_label.peaks.size();
    double angle_LaBr = 0;
    double angle_NaI = 0;

    // First step : calculate the angle between the short gate axis and both the LaBr3/CeBr3 and NaI diagonal : 
    for (auto const & peak : points_label.peaks)
    {
      angle_LaBr+=points_label.LaBr3.at(peak).second/points_label.LaBr3.at(peak).first;
      angle_NaI+=points_label.NaI.at(peak).second/points_label.NaI.at(peak).first;
    }

    angle_LaBr/=nb_peaks;
    angle_NaI/=nb_peaks;

    // Second step : calculate the angle between LaBr3/CeBr3 diagonal and the internal add-back anti-diagonal :
    double slope_mixed = 0;
    for (auto const & peak : points_label.peaks)
    {
      auto const & LaBr3 = points_label.LaBr3.at(peak);
      auto const & NaI = points_label.NaI.at(peak);
      slope_mixed += (LaBr3.second - NaI.second) / (LaBr3.first - NaI.first);
    }
    slope_mixed/=nb_peaks;
    auto angle = -slope_mixed;

    // Third step : the rotated Qlong values of the points are now shifted, but we know what energy they correspond to
    // We therefore perform a calibration to use for the QDC2 point :
    std::vector<double> points;

    for (auto const & peak : points_label.peaks)
    {
      auto const & LaBr3 = points_label.LaBr3.at(peak);
      auto const & NaI = points_label.NaI.at(peak);
      Point rotated_LaBr3 = Paris::rotate(LaBr3.first, LaBr3.second, angle);
      Point rotated_NaI = Paris::rotate(NaI.first, NaI.second, angle);
      points.push_back((rotated_LaBr3.second+rotated_NaI.second)/2); // We store the average of both of the rotated points for each energy
    }

    // Fitting the points :
    TF1* fit_linear = new TF1("linear", "[0]*x");
    std::vector<double> peaks_double; for (auto const & peak : points_label.peaks) peaks_double.push_back(double_cast(peak)); // Formatting the data
    auto graph = new TGraph(points_label.peaks.size(), points.data(), peaks_double.data());
    graph->Fit(fit_linear, "q");
    angles_rotation.data.emplace(label, PhoswitchCalib::Coefficients{-slope_mixed, fit_linear->GetParameter(0)});
  }
  angles_rotation.write();
}

TH2F* rotate(TH2F* bidim, double angle)
{
  auto const & name = bidim->GetName();
  auto const & title = bidim->GetTitle();
  
  auto xAxis = bidim->GetXaxis();
  auto const & binsX =  xAxis->GetNbins();
  auto const & minX  =  xAxis->GetXmin();
  auto const & maxX  =  xAxis->GetXmax();

  auto yAxis = bidim->GetYaxis();
  auto const & binsY = yAxis->GetNbins();
  auto const & minY  = yAxis->GetXmin();
  auto const & maxY  = yAxis->GetXmax();

  auto rotated_bidim = new TH2F((name+std::string("_rotated")).c_str(), (title+std::string(" rotated")).c_str(), binsX,minX,maxX, binsY,minY,maxY);
  // return rotated_bidim;
  double _sin = sin(angle);
  double _cos = cos(angle);

  for (int binX = 0; binX<binsX; binX++)
  {
    for (int binY = 0; binY<binsY; binY++)
    {
      auto const & nb_hits   = bidim->GetBinContent(binX, binY);
      auto const & old_long  = bidim->GetYaxis()->GetBinCenter(binY);
      auto const & old_short = bidim->GetXaxis()->GetBinCenter(binX);

      auto const & old_long_range  = bidim->GetYaxis()->GetBinCenter(binY+1)-old_long ;
      auto const & old_short_range = bidim->GetXaxis()->GetBinCenter(binX+1)-old_short;
      for (int hit_i = 0; hit_i<nb_hits; hit_i++)
      {
        auto const & rand_short = old_short + randomCo::uniform(0, old_short_range);
        auto const & rand_long  = old_long  + randomCo::uniform(0, old_long_range);

        // Rotate the NaI+both toward the long gate :
        auto const & new_short = rand_short * _cos - rand_long * _sin; // * (abs(_tan)/_tan);
        auto const & new_long  = rand_short * _sin + rand_long * _cos; // * (abs(_tan)/_tan);

        rotated_bidim->Fill(new_short, new_long);
      }
    }
  }
  return rotated_bidim;
}

TH2F* rotateAndCalibrate(TH2F* bidim, double angle, double const & coeff)
{
  auto const & name = bidim->GetName();
  auto const & title = bidim->GetTitle();
  
  auto xAxis = bidim->GetXaxis();
  auto const & binsX =  xAxis->GetNbins();
  auto const & minX  =  xAxis->GetXmin();
  auto const & maxX  =  xAxis->GetXmax();

  auto yAxis = bidim->GetYaxis();
  auto const & binsY = yAxis->GetNbins();
  auto const & minY  = yAxis->GetXmin();
  auto const & maxY  = yAxis->GetXmax();

  auto rotated_bidim = new TH2F((name+std::string("_rotated")).c_str(), (title+std::string(" rotated")).c_str(), binsX,minX,maxX, binsY,minY,maxY);
  // return rotated_bidim;
  double _sin = sin(angle);
  double _cos = cos(angle);

  for (int binX = 0; binX<binsX; binX++)
  {
    for (int binY = 0; binY<binsY; binY++)
    {
      auto const & nb_hits   = bidim->GetBinContent(binX, binY);
      auto const & old_long  = bidim->GetYaxis()->GetBinCenter(binY);
      auto const & old_short = bidim->GetXaxis()->GetBinCenter(binX);

      auto const & old_long_range  = bidim->GetYaxis()->GetBinCenter(binY+1)-old_long ;
      auto const & old_short_range = bidim->GetXaxis()->GetBinCenter(binX+1)-old_short;
      for (int hit_i = 0; hit_i<nb_hits; hit_i++)
      {
        auto const & rand_short = old_short + randomCo::uniform(0, old_short_range);
        auto const & rand_long  = old_long  + randomCo::uniform(0, old_long_range);

        // Rotate the NaI+both toward the long gate :
        auto const & new_short = rand_short * _cos - rand_long * _sin; // * (abs(_tan)/_tan);
        auto const & new_long  = coeff * (rand_short * _sin + rand_long * _cos); // * (abs(_tan)/_tan);

        rotated_bidim->Fill(new_short, new_long);
      }
    }
  }
  return rotated_bidim;
}

// void clickRecalPhoswitch(std::string filename, std::vector<int> peaks = {511, 1278})
// {
//   detectors.load("index_129.list");
//   auto file = TFile::Open(filename.c_str(), "read");
//   file->cd();
//   auto map = get_map_histo<TH2F*>(file, "TH2F");
//   PhoswitchCalib angles("NaI_136_2024.angles");
//   auto c1 = new TCanvas("c1");
//   auto c2 = new TCanvas("c2");
//   c2->SetWindowPosition(c1->GetWindowTopX()+c1->GetWindowWidth(), c1->GetWindowTopY());
//   for (auto const & pair : map)
//   {
//     auto name = pair.first;
//     auto det_name = removeLastPart(name, '_');
//     auto const & label = detectors[det_name];
//     auto histo = pair.second;
//     print(det_name, label, angles.data[label]);
//     auto rotated = rotate(histo, angles.data[label]);
//     c1->cd();
//     gPad->SetLogz();
//     rotated->SetMinimum(2);
//     rotated->Draw("colz");
//     gPad->Update();
//     c2->cd();
//     auto projected = rotated->ProjectionY();
//     auto xaxis = projected->GetXaxis();
//     for (auto const & peak : peaks)
//     {
//       projected->Draw();
//       auto point = selectPoint(projected, concatenate("Click on the ", peak, " keV peak to zoom on it"));
//       auto const & new_low_edge = point.first-(xaxis->GetXmax()-xaxis->GetXmin())/10;
//       auto const & new_high_edge = point.first+(xaxis->GetXmax()-xaxis->GetXmin())/10;
//       print(new_low_edge, new_high_edge);
//       xaxis->SetRangeUser(new_low_edge, new_high_edge);
//       gPad->Update();
//       point = selectPoint(projected, concatenate("Select the ", peak, " keV peak"));
//       xaxis->UnZoom();
//     }

//   }
// }

void recalPhoswitch(std::string filename)
{
  detectors.load("index_129.list");
  auto file = TFile::Open(filename.c_str(), "read");
  file->cd();
  auto map = get_map_histo<TH2F*>(file, "TH2F");
  PhoswitchCalib angles("NaI_136_2024.angles");
  auto c1 = new TCanvas("c1");
  auto c2 = new TCanvas("c2");
  c2->SetWindowPosition(c1->GetWindowTopX()+c1->GetWindowWidth(), c1->GetWindowTopY());
  std::map<Label, TH2F*> rotated_calibrated;
  std::map<Label, TH1D*> projections;
  std::vector<Label> labels;
  for (auto const & pair : map)
  {
    auto name = pair.first;
    auto det_name = removeLastPart(name, '_');
    auto const & label = detectors[det_name];
    labels.push_back(label);
    auto histo = pair.second;
    print(det_name, label, angles.data.at(label).angle, angles.data.at(label).coeff);
    auto rotated = rotateAndCalibrate(histo, angles.data.at(label).angle, angles.data.at(label).coeff);
    rotated->SetDirectory(nullptr);
    rotated_calibrated.emplace(label, rotated);
    c1->cd();
    gPad->SetLogz();
    rotated->SetMinimum(2);
    rotated->Draw("colz");
    gPad->Update();
    c2->cd();
    auto projected = rotated->ProjectionY((std::string(det_name)+"_calib_phoswitch").c_str());
    projections.emplace(label, projected);
    projected->SetDirectory(nullptr);
    projected->Draw();
    gPad->Update();
  }
  file->Close();

  std::sort(labels.begin(), labels.end());

  auto outFile(TFile::Open("phoswitches_calibrated.root", "recreate"));
  outFile->cd();
  auto first_spectra = projections[labels[0]];
  TH2F* bidim = new TH2F("all_spectra", "all spectra", projections.size(),0,projections.size(), first_spectra->GetNbinsX(), first_spectra->GetXaxis()->GetXmin(), first_spectra->GetXaxis()->GetXmax());
  int histo_i = 0;
  for (auto const & label : labels)
  {
    ++histo_i;
    for (int bin = 1; bin<projections[label]->GetNbinsX()+1; ++bin) 
    {
      bidim->SetBinContent(histo_i, bin, projections[label]->GetBinContent(bin));
    }
  }
  bidim->Write("all_spectra", TObject::kOverwrite);
  for (auto const & label : labels) if (projections.at(label)) projections.at(label)->Write();
  for (auto const & label : labels) if (rotated_calibrated.at(label)) rotated_calibrated.at(label)->Write();
  outFile->Close();
}


void click_all_in_one()
{
  print("name of the bidim file to use for 2D peak clicking : (type q to skip that part)");
  std::string bidim_file; std::cin >> bidim_file;
  std::vector<int> peaks;
  auto getEnergies = [&](){
    print("energies to use (for default 22Na 511 and 1274 type d, q for stop):");
    std::string peak_str;
    while (true)
    {
      std::cout << "> ";
      std::cin >> peak_str; 
      if (peak_str == "d")
      {
        print();
        print("Treating 511 and 1274 keV");
        peaks = {511, 1274};
        break;
      }
      else if (peak_str == "q")
      try
      {
        peaks.push_back(std::stoi(peak_str));
      }
      catch (const std::invalid_argument& err)
      {
        error("Invalid argument: ", err.what());
      }
    }
  };

  if (bidim_file!="q")
  {
    getEnergies();

    clickAddbackPoints(bidim_file, peaks);
  }

  print("Calculate the angles from the points extracted at previous stage : (type q to skip that part)");
  std::string choice; std::cin >> choice;
  if (choice!="q") createAddbackAngleFile();

  print("Name of the bidim file to use to calculate phoswitch recalibration coefficient : (type q to skip that part)");
  std::cin >> choice;
  if (choice!="q")
  {
    getEnergies();
    std::string filename; std::cin >> filename;
    // clickRecalPhoswitch(filename, peaks);
  }
}


// namespace NaI_coeffs
// {
//   std::pair<double, double> selectPoint(TH1* histo, std::string const & instructions)
//   {
//     double x = 0; double y = 0;
//     gPad->SetTitle(instructions.c_str());
//     histo->SetTitle(instructions.c_str());
//     GetPoint(gPad->cd(), x, y);
//     gPad->Update();
//     return {x, y};
//   }

//   double ratioNaILaBr(TH2* paris_bidim, std::vector<int> peaks = {511, 1274})
//   {
//     paris_bidim->SetMinimum(2);
//     paris_bidim->Draw("colz");
//     gPad->SetLogz(true);
//     auto xaxis = paris_bidim->GetXaxis();
//     auto yaxis = paris_bidim->GetYaxis();
//     std::vector<double> ratios;
//     //1. Loop trough the peaks :
//     for (auto const & peak : peaks)
//     {
//       //2. First the LaBr3 peak :
//       auto LaBr3_point = selectPoint(paris_bidim, concatenate("Click on the ", peak, " keV peak in the LaBr3 diagonal to zoom on it"));
//       xaxis->SetRangeUser(LaBr3_point.first-(xaxis->GetXmax()-xaxis->GetXmin())/10, LaBr3_point.first+(xaxis->GetXmax()-xaxis->GetXmin())/10);
//       yaxis->SetRangeUser(LaBr3_point.second-(yaxis->GetXmax()-yaxis->GetXmin())/10, LaBr3_point.second+(yaxis->GetXmax()-yaxis->GetXmin())/10);
//       LaBr3_point = selectPoint(paris_bidim, concatenate("Select the ", peak, " keV peak in the LaBr3 diagonal"));
//       xaxis->UnZoom();
//       yaxis->UnZoom();

//       auto NaI_point = selectPoint(paris_bidim, concatenate("Click on the ", peak, " keV peak in the NaI diagonal to zoom on it"));
//       xaxis->SetRangeUser(NaI_point.first-(xaxis->GetXmax()-xaxis->GetXmin())/10, NaI_point.first+(xaxis->GetXmax()-xaxis->GetXmin())/10);
//       yaxis->SetRangeUser(NaI_point.second-(yaxis->GetXmax()-yaxis->GetXmin())/10, NaI_point.second+(yaxis->GetXmax()-yaxis->GetXmin())/10);
//       NaI_point = selectPoint(paris_bidim, concatenate("Select the ", peak, " keV peak in the LaBr3 diagonal"));
//       xaxis->UnZoom();
//       yaxis->UnZoom();

//       ratios.push_back(NaI_point.second/LaBr3_point.first);
//     }
//     gPad->SetLogz(false);
//     return mean(ratios);
//   }

//   void calculate(std::string filename)
//   {
//     detectors.load("index_129.list");
//     auto file = TFile::Open(filename.c_str(), "read");
//     file->cd();
//     std::ofstream outfile("coeffs_NaI.calib");
    
//     auto map = get_map_histo<TH2F*>(file, "TH2F");
//     for (auto const & pair : map)
//     {
//       auto name = pair.first;
//       auto det_name = removeLastPart(name, '_');
//       auto const & label = detectors[det_name];
//       auto histo = pair.second;
//       std::stringstream output;
//       output << label << " 0 " << ratioNaILaBr(histo) << std::endl;
//       print(output.str());
//       outfile << output.str();
//     }
//     outfile.close();
//   }
// }


// std::pair<double, double> Paris::findAngles(TH2F* bidim, int nb_bins, bool write_graphs)
// {
//   std::string out_filename = "angles_paris_bidim.root";
//   auto const & nb_bins_long = bidim->GetNbinsX();
//   auto const & nb_iterations_long = nb_bins_long/nb_bins;

//   std::vector<double> bins;
//   std::vector<double> xBin_max_peak1;
//   std::vector<double> xBin_max_peak2;

//   auto const & maximum = bidim->GetMaximum();

//   for (int it = 0; it<nb_iterations_long; it++)
//   {

//     // Peak finding : 
//     auto const & bin_min = it*nb_bins;
//     auto const & bin_max = it*(nb_bins+1);
//     auto proj_short = std::make_unique<TH1D> (*(bidim->ProjectionX("temp_short", bin_min, bin_max)));
//     auto const & max = proj_short->GetMaximum();
//     if (max<maximum/20) continue;
//     auto const & threshold_start = max/2;
//     auto const & threshold_stop = max/5;

//     int bin = 0;

//       // First peak :
//     auto begin_peak1 = findNextBinAbove(proj_short.get(), bin, threshold_start);
//     bin+=3; // In order to avoid random threshold crossing
//     auto end_peak1   = findNextBinBelow(proj_short.get(), bin, threshold_stop);
//     bin+=3; 

//       // Second peak :
//     auto begin_peak2 = findNextBinAbove(proj_short.get(), bin, threshold_start);
//     if (begin_peak2 > proj_short->GetNbinsX()) continue;
//     bin+=3;
//     auto end_peak2   = findNextBinBelow(proj_short.get(), bin, threshold_stop);
//     bin+=3;

//     // Do not take the peak if the end of the last peak is at the end of the spectra (i.e. not found)
//     if (end_peak2+1 > proj_short->GetNbinsX()) continue;

//     // Fitting peaks :
//     auto const long_gate_ADC = bidim->GetYaxis()->GetBinCenter(it*(nb_bins+0.5));
    
//     auto const & low_edge_peak1 = proj_short->GetBinCenter(begin_peak1*0.7);
//     auto const & high_edge_peak1 = proj_short->GetBinCenter(end_peak1*1.3);
//     auto const & low_edge_peak2 = proj_short->GetBinCenter(begin_peak2*0.7);
//     auto const & high_edge_peak2 = proj_short->GetBinCenter(end_peak2*1.3);

//     PeakFitter peak1(proj_short.get(), low_edge_peak1, high_edge_peak1);
//     PeakFitter peak2(proj_short.get(), low_edge_peak2, high_edge_peak2);


//     auto const & peak1_ADC = peak1.getMean();
//     auto const & peak2_ADC = peak2.getMean();

//     // Check that the two peaks are far away enough from one another
//     if (peak1_ADC>peak2_ADC*0.5) continue;

//     // Check that the current points are not too far away from values interpolated from previous points :
//     double mean_slope_peak1 = 0;
//     double mean_slope_peak2 = 0;
//     for (size_t bin_it = 0; bin_it<bins.size(); bin_it++)
//     {
//       mean_slope_peak1 += xBin_max_peak1[bin_it]/bins[bin_it];
//       mean_slope_peak2 += xBin_max_peak2[bin_it]/bins[bin_it];
//     }
//     mean_slope_peak1 /= bins.size();
//     mean_slope_peak2 /= bins.size();

//     auto const & interpolated_peak1 = long_gate_ADC*mean_slope_peak1;
//     auto const & interpolated_peak2 = long_gate_ADC*mean_slope_peak2;

//     if (peak1_ADC > interpolated_peak1*1.1 || peak1_ADC < interpolated_peak1*0.9) continue;
//     if (peak2_ADC > interpolated_peak2*1.1 || peak2_ADC < interpolated_peak2*0.9) continue;

//     // Save the informations : 
//     bins.push_back(long_gate_ADC);
//     xBin_max_peak1.push_back(peak1_ADC);
//     xBin_max_peak2.push_back(peak2_ADC);
//   }

//   // Plot the position of each peak as a function of the mean bin of the projection 
//   // Fit the curve, extract the slope and calculate the angle :
  
//   auto const & nb_points = bins.size();
  
//   TF1* linear1(new TF1("_lin","pol1"));
//   TF1* linear2(new TF1("_lin","pol1"));

//   auto graph_peak1(new TGraph(nb_points, bins.data(), xBin_max_peak1.data()));
//   graph_peak1->SetName(concatenate("First_peak_", bidim->GetName()).c_str());
//   graph_peak1->SetTitle(concatenate("First peak ", bidim->GetName()).c_str());
//   graph_peak1->Fit(linear1, "q");

//   auto first_slope = linear1->GetParameter(1);
//   auto const & first_angle = atan(first_slope);

//   auto graph_peak2(new TGraph(nb_points, bins.data(), xBin_max_peak2.data()));
//   graph_peak2->SetName(concatenate("Second_peak_", bidim->GetName()).c_str());
//   graph_peak2->SetTitle(concatenate("Second peak ", bidim->GetName()).c_str());
//   graph_peak2->Fit(linear2, "q");

//   auto second_slope = linear2->GetParameter(1);
//   auto const & second_angle = atan(second_slope);

//   // Calculate and plot the residues of the fit :
//   std::vector<double> residues_peak1;
//   std::vector<double> residues_peak2;

//   for (size_t bin_it = 0; bin_it<bins.size(); bin_it++)
//   {
//     residues_peak1.push_back(xBin_max_peak1[bin_it ]-first_slope*bins[bin_it]);
//     residues_peak2.push_back(xBin_max_peak2[bin_it ]-second_slope*bins[bin_it]);
//   }
//   // pauseCo();

//   auto graph_residues_peak1(new TGraph(nb_points, bins.data(), residues_peak1.data()));
//   graph_residues_peak1->SetName(concatenate("First_peak_", bidim->GetName(), "_residues").c_str());
//   graph_residues_peak1->SetTitle(concatenate("First peak ", bidim->GetName(), " residues").c_str());

//   auto graph_residues_peak2(new TGraph(nb_points, bins.data(), residues_peak2.data()));
//   graph_residues_peak2->SetName(concatenate("Second_peak_", bidim->GetName(), "_residues").c_str());
//   graph_residues_peak2->SetTitle(concatenate("Second peak ", bidim->GetName(), " residues").c_str());
  
//   if (write_graphs)
//   {// Write the graphs in a root file :
//     auto file = TFile::Open(out_filename.c_str(), "update");
//     file -> cd();
//     graph_peak1->Write();
//     graph_peak2->Write();
//     graph_residues_peak1->Write();
//     graph_residues_peak2->Write();
//     file->Write();
//     file->Close();
//   }

//   return {first_angle, second_angle};
// }

// std::pair<double, double> Paris::findAnglesDeg(TH2F* bidim, int nb_bins, bool write_graphs)
// {
//   auto ret (findAngles(bidim, nb_bins, write_graphs));
//   ret.first *=57.295718;
//   ret.second*=57.295718;
//   return ret;
// }

// void Paris::calculateBidimAngles(std::string filename, std::string const & out_filename, bool write_graphs)
// {
//   auto file(TFile::Open(filename.c_str(), "READ"));
//   file -> cd();
//   auto names(get_names_of<TH2F>(file));

//   std::map<std::string, TH1D*> projs;
//   ParisBidimAngles angles;

//   if (File("angles_paris_bidim.root").exists()) 
//   {
//     print("rm angles_paris_bidim.root");
//     system("rm angles_paris_bidim.root");
//   }

//   size_t avancement = 0;
//   CoProgressBar<size_t> prog(&avancement, names.size());

//   for (auto const & name : names)
//   {
//     auto bidim = file->Get<TH2F>(name.c_str());
//     auto const & nb_bins = 10;
//     auto temp (Paris::findAngles(bidim, nb_bins, write_graphs));
//     auto detector_name = name;
//     remove(detector_name, "_bidim");
//     angles.set(detector_name, temp.first, temp.second);
//     ++avancement; prog.show(); // progress bar
//   }
//   print();
//   file->Close();
//   print("angles_paris_bidim.root written");
//   angles.write(out_filename);
// }

// TH2F* Paris::rotate(TH2F* bidim, double angleLaBr, double angleNaI, bool quickNdirty)
// {
//   if (!bidim) throw_error("in Paris::rotate(TH2F* bidim, double angleLaBr, double angleNaI) : bidim is nullptr");
//   if (quickNdirty) print("pas beau ...");

//   ADC short_shift = 2000;
//   ADC long_shift = 2000;

//   auto rotated_bidim (static_cast<TH2F*>(bidim->Clone(concatenate(bidim->GetName(), "_rotated").c_str())));
//   rotated_bidim->Reset();

//   auto const & nb_binsx = bidim->GetNbinsX();
//   auto const & nb_binsy = bidim->GetNbinsY();
//   auto const & pi_sur_2 = 1.570798;
//   // double _sin = sin(-angleLaBr);
//   // double _cos = cos(-angleLaBr);
//   double _sin = sin(angleNaI-angleLaBr);
//   double _cos = cos(angleNaI-angleLaBr);
//   double _sinLaBr = sin(-pi_sur_2+angleLaBr);
//   double _cosLaBr = cos(-pi_sur_2+angleLaBr);

//   for (int binX = 0; binX<nb_binsx; binX++)
//   {
//     for (int binY = 0; binY<nb_binsy; binY++)
//     {
//       auto const & nb_hits   = bidim->GetBinContent(binX, binY);
//       auto const & old_long  = bidim->GetYaxis()->GetBinCenter(binY);
//       auto const & old_short = bidim->GetXaxis()->GetBinCenter(binX);

//       // Reject LaBr3 events :
//       // auto const & pid = (old_long-old_short)/old_long;
//       // if (pid<0.25) continue;

//       auto const & old_long_range  = bidim->GetYaxis()->GetBinCenter(binY+1)-old_long ;
//       auto const & old_short_range = bidim->GetXaxis()->GetBinCenter(binX+1)-old_short;
//       for (int hit_i = 0; hit_i<nb_hits; hit_i++)
//       {
//         auto const & rand_short = old_short + randomCo::uniform(0, old_short_range);
//         auto const & rand_long  = old_long  + randomCo::uniform(0, old_long_range);

//         // Rotate the NaI+both toward the long gate :
//         auto const & new_short = rand_short * _cos - rand_long * _sin; // * (abs(_tan)/_tan);
//         auto const & new_long  = rand_short * _sin + rand_long * _cos; // * (abs(_tan)/_tan);

//         // Rotate the LaBr3 towards the short gate :
//         // auto const & new_short_LaBr = rand_short * _cosLaBr - rand_long * _sinLaBr; // * (abs(_tan)/_tan);
//         auto const & new_long_LaBr  = rand_short * _sinLaBr + rand_long * _cosLaBr; // * (abs(_tan)/_tan);
//         if (new_long_LaBr<3000) continue;


//         rotated_bidim->Fill(new_short+short_shift, new_long+long_shift);
//       }
//     }
//   }
//   return rotated_bidim;
// }



#endif //PARIS_HPP
