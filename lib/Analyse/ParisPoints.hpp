#ifndef PARISPOINTSFILE_HPP
#define PARISPOINTSFILE_HPP

// Dealing with the Qshort VS Qlong matrix

#include "../Classes/Detectors.hpp"
#include "ParisBidimAngles.hpp"
#include "PhoswitchCalib.hpp"

using Point = CoLib::Point;

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

  void RootToBidims(std::string data_path, std::string bidim_filename);
  void clickAddbackPoints(std::string filename, std::vector<int> peaks = {511, 1274});
  void createAddbackAngleFile(std::string const & parisPoints_filename = "NaI_136_2024.points");


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

void ParisPointsFile::RootToBidims(std::string data_path, std::string bidim_filename)
{

}

void ParisPointsFile::clickAddbackPoints(std::string filename, std::vector<int> peaks)
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
    outfilePoints << outputPoints.str();
  }

  outfilePoints.close();
  print("NaI_136_2024.points written");
}

void ParisPointsFile::createAddbackAngleFile(std::string const & parisPoints_filename)
{
  PhoswitchCalib angles_rotation;
  ParisPointsFile all_points(parisPoints_filename);
  for (auto const & it : all_points)
  {
    auto const & label = it.first;
    auto const & points_label = it.second;
    double nb_peaks = points_label.peaks.size();
    
    // First step : calculate the angle between LaBr3/CeBr3 diagonal and the internal add-back anti-diagonal :
    double slope_mixed = 0;
    for (auto const & peak : points_label.peaks)
    {
      auto const & LaBr3 = points_label.LaBr3.at(peak);
      auto const & NaI = points_label.NaI.at(peak);
      slope_mixed += (LaBr3.second - NaI.second) / (LaBr3.first - NaI.first);
    }
    slope_mixed/=nb_peaks;
    auto angle = -slope_mixed;

    // Second step : the rotated Qlong values of the points are now shifted, but we know what energy they correspond to
    // We therefore perform a first rough calibration to use for the QDC2 point :
    std::vector<double> points;
    for (auto const & peak : points_label.peaks)
    {
      auto const & LaBr3 = points_label.LaBr3.at(peak);
      auto const & NaI = points_label.NaI.at(peak);
      Point rotated_LaBr3 = CoLib::rotate(LaBr3, angle);
      Point rotated_NaI = CoLib::rotate(NaI, angle);
      points.push_back((rotated_LaBr3.second+rotated_NaI.second)/2); // We store the average of both of the rotated points for each energy
    }

    // Fitting the points :
    TF1* fit_linear = new TF1("linear", "[0]*x");
    std::vector<double> peaks_double; for (auto const & peak : points_label.peaks) peaks_double.push_back(double_cast(peak)); // Formatting the data
    auto graph = new TGraph(points_label.peaks.size(), points.data(), peaks_double.data());
    graph->Fit(fit_linear, "q");
    angles_rotation.data.emplace(label, PhoswitchCalib::Calib{-slope_mixed, fit_linear->GetParameter(0)});
  }
  angles_rotation.write();
}
#endif //PARISPOINTSFILE_HPP