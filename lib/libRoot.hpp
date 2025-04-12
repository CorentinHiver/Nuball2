#ifndef LIBROOT_HPP
#define LIBROOT_HPP

#include "libCo.hpp"

// ********** ROOT includes ********* //
#include <TAxis.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TContextMenu.h>
#include <TError.h>
#include <TF1.h>
#include <TF2.h>
#include <TFile.h>
#include <TFileMerger.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH1S.h>
#include <TH2.h>
#include <TH2F.h>
#include <TH3I.h>
#include <TKey.h>
#include <TLatex.h>
#include <TLeaf.h>
#include <TLegend.h>
#include <TMarker.h>
#include <TMath.h>
#include <TObjString.h>
#include <TPolyMarker.h>
#include <TRandom.h>
#include <TRegexp.h>
#include <TROOT.h>
#include <TSpline.h>
#include <TSpectrum.h>
#include <TStopwatch.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TThread.h>
#include <TTree.h>
#include <TTreeIndex.h>

///////////////
//   Usings  //
///////////////

using unique_TH1F  = std::unique_ptr<TH1F>;
using unique_TH1D  = std::unique_ptr<TH1D>;
using unique_TH1I  = std::unique_ptr<TH1I>;

using unique_TH2F  = std::unique_ptr<TH2F>;
using unique_TH2D  = std::unique_ptr<TH2D>;
using unique_TH2I  = std::unique_ptr<TH2I>;

using unique_TH3F  = std::unique_ptr<TH3F>;
using unique_TH3D  = std::unique_ptr<TH3D>;
using unique_TH3I  = std::unique_ptr<TH3I>;

using unique_TFile = std::unique_ptr<TFile>;
using unique_tree  = std::unique_ptr<TTree>;

//////////////
//   Types  //
//////////////

/// @brief Casts a number into unsigned short
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline ULong64_t ULong64_cast(T const & t) {return static_cast<ULong64_t>(t);}

/// @brief Casts a number into unsigned short
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Long64_t Long64_cast(T const & t) {return static_cast<Long64_t>(t);}

///////////////////////////
// Some Initialisiations //
///////////////////////////

#ifdef MULTITHREADING
  std::mutex mutex_Root;
#endif //MULTITHREADING

// TRandom gRandom(time(0));

/////////////////////////////////////////////////////////////
// Generic functions using some actually nice ROOT classes //
/////////////////////////////////////////////////////////////


/// @brief This method allows one to get the x and y values of where the user clicks on the graph
void GetPoint(TVirtualPad * vpad, double& x, double& y)
{
  auto pad = static_cast<TPad*>(vpad);
  pad->Update();
  auto cutg = static_cast<TMarker*> (pad->WaitPrimitive("TMarker","Marker"));
  if (!cutg) {print("CAN'T FIND THE PAD OR MOUSE CLICK"); return;}
  x = cutg->GetX();
  y = cutg->GetY();
  delete cutg;
}

namespace CoLib
{
  Strings match_regex(Strings list, std::string pattern)
  {
    TRegexp reg((TString(pattern.c_str()).ReplaceAll("*", ".*")).ReplaceAll("?", "."));
    std::vector<TString> strings; for (auto const & e : list) strings.push_back(e.c_str());
    Strings ret;
    for (size_t i = 0; i < strings.size(); ++i) if (strings[i].Index(reg) != kNPOS) ret.push_back(list[i]);
    return ret;
  }

  template<class T>
  auto unload(T* obj)
  {
    obj->SetDirectory(nullptr);
    delete obj;
  }

  template<class T>
  auto clone(std::string name, std::string new_name, TFile* file = nullptr)
  {
    if (file == nullptr) file = gFile;
    // if (!file) { error("in clone<", T::GetClassName(), "> : file is nullptr"); return nullptr;}
    file->cd();
    return dynamic_cast<T*> (file->Get<T>(name.c_str())->Clone(new_name.c_str()));
  }
}

/////////////////////////////////////
//   Some Convenient Definitions   //
/////////////////////////////////////

std::vector<int> ROOT_nice_colors = { 1, 2, 4, 6, 8, 9, 11, 30};

auto getROOTniceColors(int i)
{ // Get the nice colors abovedefined. The "i & 111" is equivalent to "i modulo 8"
  return ROOT_nice_colors[(i & 111)];
}

////////////////////////////
//   HISTO MANIPULATIONS  //
////////////////////////////

template<class THist>
THist* Add(THist* h1, THist* h2, double factor = 1)
{
  auto ret = (THist*) h1->Clone(TString(h1->GetName())+"_plus_"+TString(h2->GetName()));
  ret->SetTitle(TString(h1->GetName())+"+"+TString(h2->GetName()));
  ret->Add(h2, +factor);
  return ret;
}

template<class THist>
THist* Sub(THist* h1, THist* h2, double factor = 1.)
{
  auto ret = (THist*) h1->Clone(TString(h1->GetName())+"_minus_"+TString(h2->GetName()));
  ret->SetTitle(TString(h1->GetName())+"-" + ((factor == 1.) ? "" : std::to_string(factor).c_str()) + TString(h2->GetName()));
  ret->Add(h2, -factor);
  return ret;
}

template<class THist>
THist* SubInteger(THist* h1, THist* h2, double factor = 1)
{
  auto ret = (THist*) h1->Clone(TString(h1->GetName())+"_minus_"+TString(h2->GetName()));
  ret->SetTitle(TString(h1->GetName())+"-"+TString(h2->GetName()));
  for (int bin_i = 0; bin_i<h1->GetNbinsX(); ++bin_i) 
  {
    auto const & diff = static_cast<int>(factor*h2->GetBinContent(bin_i));
    ret->SetBinContent(bin_i, ret->GetBinContent(bin_i)-diff);
  }
  return ret;
}

TH1* Mult(TH1* h1, TH1* h2, double factor = 1)
{
  auto ret = (TH1*) h1->Clone(TString(h1->GetName())+"_times_"+TString(h2->GetName()));
  ret->SetTitle(TString(h1->GetName())+"#times"+TString(h2->GetName()));
  for (int bin = 1; bin<h1->GetNbinsX(); ++bin) ret->SetBinContent(bin, h1->GetBinContent(bin)*h2->GetBinContent(bin)*factor);
  return ret;
}

float minXaxis(TH1* histo)
{
  return histo->GetXaxis()->GetBinLowEdge(1);
}

float maxXaxis(TH1* histo)
{
  return histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX());
}

float minYaxis(TH1* histo)
{
  return histo->GetYaxis()->GetBinLowEdge(1);
}

float maxYaxis(TH1* histo)
{
  return histo->GetYaxis()->GetBinUpEdge(histo->GetNbinsY());
}

bool THist_exists(TH1* histo)
{
  return (histo && !histo->IsZombie() && histo->Integral()>1);
}

/// @brief Patch to the TH1::Add method when the histograms limits are inconsistent
void AddTH1(TH1* histo_total, TH1* histo)
{
  for (int bin = 0; bin<histo_total->GetNbinsX() + 1; bin++)
  {
    auto const & X_value = histo_total->GetBinCenter(bin);
    auto const & content_other = histo->Interpolate(X_value);
    histo_total->SetBinContent(bin, histo_total->GetBinContent(bin) + content_other);
  }
}

double myIntegral(TH1* histo, double bin_min = 1, double bin_max = -1)
{
  int integral = 0;
  for (int bin = bin_min; bin<=bin_max; ++bin) integral += histo->GetBinContent(bin);
  return integral;
}

double myIntegralUser(TH1* histo, double x_min, double x_max)
{
  int integral = 0;
  for (int bin = histo->FindBin(x_min); bin<=histo->FindBin(x_max); ++bin) integral += histo->GetBinContent(bin);
  return integral;
}

bool AddTH1(TH2* histo2, TH1* histo1, int index, bool x = true)
{
  if (!histo2) {print("TH2 do not exists"); return -1;}
  if (!histo1) {print("TH1 do not exists"); return -1;}
  auto axis = (x) ? histo2 -> GetXaxis() : histo2 -> GetYaxis();

  if (axis->GetNbins() < index)
  {
    print("Too many histo like", histo1->GetName(), "to merge with", histo2->GetName() ,"...");
    return false;
  }

  if (x) for (int bin = 0; bin<histo1->GetNbinsX(); bin++) histo2->SetBinContent(index, bin, histo1->GetBinContent(bin));
  else   for (int bin = 0; bin<histo1->GetNbinsX(); bin++) histo2->SetBinContent(bin, index, histo1->GetBinContent(bin));
  
  return true;
}

std::vector<TH1D*> allProjectionsY(TH2* histo)
{
  std::vector<TH1D*> ret;
  for (int x = 0; x<histo->GetNbinsX(); ++x)
  {
    auto const & name = TString(histo->GetName())+"_p"+TString(std::to_string(histo->GetXaxis()->GetBinLowEdge(x)).c_str());
    ret.push_back(histo->ProjectionY(name, x, x));
  }
  return ret;
}

TH1F* lineariseBidim(TH2F* histo, std::string options = "")
{
  bool discard_null = !found(options, "A");
  // bool discard_last = found(options, "l");

  auto xaxis = histo->GetXaxis();
  auto yaxis = histo->GetYaxis();
  
  auto XpBin = (xaxis->GetBinUpEdge(xaxis->GetNbins()) - xaxis->GetBinLowEdge(0))/xaxis->GetNbins();

  std::vector<double> lin_vec;

  for (int y = 1; y<=yaxis->GetNbins(); ++y)for (int x = 1; x<=xaxis->GetNbins(); ++x)
  {
    auto const & value = histo->GetBinContent(x, y);
    if (!discard_null || value != 0) lin_vec.push_back(value);
    // if (discard_last && x<xaxis->GetNbins()-2 && histo->GetBinContent(x+1, y) == 0) lin_vec.pop_back();
  }

  auto name = histo->GetName()+TString("_lin");
  auto const & nb_bins = lin_vec.size();
  auto const & minX = xaxis->GetBinLowEdge(0);
  auto const & maxX = minX + nb_bins*XpBin;
  auto ret = new TH1F(name, name, nb_bins, minX, maxX);
  for (size_t bin_i = 0; bin_i<nb_bins; ++bin_i) ret->SetBinContent(bin_i+1, lin_vec[bin_i]);
  return ret;
}

TH2F* compressBidim(TH2F* histo, std::string axis = "x")
{
  int nb_columns_compressed = 0;
  std::vector<int> not_empty_columns;
  auto xaxis = histo->GetXaxis();
  auto yaxis = histo->GetYaxis();
  if (axis == "y" )
  {
    xaxis = yaxis;
    yaxis = histo->GetXaxis();
  }
  for (int x = 0; x<xaxis->GetNbins(); ++x) for (int y = 0; y<yaxis->GetNbins(); ++y)
  {
    if (((axis == "x") ? histo->GetBinContent(x,y) : histo->GetBinContent(y, x)) != 0)
    {
      ++nb_columns_compressed;
      not_empty_columns.push_back(x);
      break;
    }
  }
  print("nb_columns_compressed", nb_columns_compressed);
  auto name = TString(histo->GetName())+"_compressed";
  auto title = TString(histo->GetTitle())+"_compressed";
  TH2F* ret = new TH2F(name, title, nb_columns_compressed, xaxis->GetXmin(), xaxis->GetBinLowEdge(nb_columns_compressed), yaxis->GetNbins(), yaxis->GetXmin(), yaxis->GetXmax());
  for (size_t x_bis = 0; x_bis<not_empty_columns.size(); ++x_bis) for (int y = 0; y<yaxis->GetNbins(); ++y)
  {
    auto const & x = not_empty_columns[x_bis];
    auto bin_content = (axis == "x") ? histo->GetBinContent(x, y) : histo->GetBinContent(y, x);
    if (bin_content!=0)
    {
      if (axis == "x") ret->SetBinContent(x_bis, y, bin_content);
      else if (axis == "y") ret->SetBinContent(y, x_bis, bin_content);
    }
  }
  return ret;
}

/**
 * @brief Meant for Paris matrix rotation method
 * 
 * @param bidim 
 * @param angle 
 * @param coeff 
 * @return TH2F* 
 */
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

/**
 * @brief Meant for Paris matrix Q_{short}/Q_{long} VS Qshort or Qlong
 * @param slope: true -> slope; false -> inverted slope
 * @param VSaxisX: true -> slope VS x axis ; false -> slope VS y axis
 */
TH2F* slopeVSaxis(TH2F* bidim, bool slope = true, bool VSaxisX = true)
{
  auto const & name = bidim->GetName();
  auto const & title = bidim->GetTitle();
  
  auto xAxis = bidim->GetXaxis();
  auto const & binsX =  xAxis->GetNbins();
  auto const & minX  =  xAxis->GetXmin();
  auto const & maxX  =  xAxis->GetXmax();

  auto yAxis = bidim->GetYaxis();
  auto const & binsY = yAxis->GetNbins();
  // auto const & minY  = yAxis->GetXmin();
  // auto const & maxY  = yAxis->GetXmax();

  auto slopeBidim = new TH2F((name+std::string("_slope")).c_str(), (title+std::string(" slope")).c_str(), binsX,minX,maxX, 1000, 0, 10);
  // return slopeBidim;

  for (int binX = 0; binX<binsX; binX++)
  {
    for (int binY = 0; binY<binsY; binY++)
    {
      auto const & nb_hits   = bidim->GetBinContent(binX, binY);
      auto const & oldX  = bidim->GetXaxis()->GetBinCenter(binX);
      auto const & oldY = bidim->GetYaxis()->GetBinCenter(binY);

      auto const & oldX_range  = bidim->GetYaxis()->GetBinCenter(binX+1)-oldX ;
      auto const & oldY_range = bidim->GetXaxis()->GetBinCenter(binY+1)-oldY;
      for (int hit_i = 0; hit_i<nb_hits; hit_i++)
      {
        auto const & randX = oldY + randomCo::uniform(0, oldY_range);
        auto const & randY  = oldX  + randomCo::uniform(0, oldX_range);
        auto const & _slope = (slope) ? randX/randY : randY/randX;
        auto const & _xaxis = (VSaxisX) ? randX : randY;
        slopeBidim->Fill(_xaxis, _slope);
      }
    }
  }
  return slopeBidim;
}

/**
 * @brief Get the last peak position object
 * 
 * @param threshold procentage over total
 * @param resolution in X value (not bin)
 * @return std::map<int, double> 
 */
std::map<int, double> get_last_peak_position(TH2* histo, double resolution = 1, double threshold = 0.05)
{
  std::map<int, double> ret;
  auto xaxis = histo->GetXaxis();
  for (int xbin = 0; xbin<xaxis->GetNbins(); ++xbin)
  {
    auto proj = histo->ProjectionY("temp_proj", xbin, xbin);
    auto proj_axis = proj->GetXaxis();
    if (!proj) continue;
    auto const & tot_integral = proj->Integral();
    if (tot_integral < 5) continue;
    int integral = 0;
    int biny = proj->GetNbinsX();
    for (;biny>0; --biny)
    {
      integral+=proj->GetBinContent(biny);
      if (double(integral)/double(tot_integral) > threshold) break;
    }
    auto const & yvalue = proj->GetBinLowEdge(biny);
    proj_axis->SetRangeUser(yvalue-3*resolution, yvalue+3*resolution);
    auto mean = proj->GetMean();
    proj_axis->SetRangeUser(mean-2*resolution, mean+2*resolution);
    auto const & value = proj->GetBinLowEdge(proj->GetMaximumBin());
    auto const & xvalue = xaxis->GetBinLowEdge(xbin);
    ret[xvalue] = value;
  }
  return ret;
}

std::map<int, double> quick_calib(TH2F* histo, double peak, double resolution = 1, double threshold = 0.05)
{
  std::map<int, double> ret;
  auto peaks = get_last_peak_position(histo, resolution, threshold);
  if (peaks.empty()) error("in quick_calib : no peaks found...");
  for (auto const & e : peaks) ret[e.first] = 1/e.second*peak;
  return ret;
}

/**
 * @brief Like AddTH1 but adjusts the binning first
 * @todo doesn't work for some reason ...
 * 
 * @param histo2 
 * @param histo1 
 * @param index 
 * @param x 
 * @return true 
 * @return false 
 */
bool AddTH1ByValue(TH2* histo2, TH1* histo1, int index, bool x = true)
{
  // throw_error("AddTH1ByValue() is DEV !");
  if (!histo2) {print("TH2 do not exists"); return -1;}
  if (!histo1) {print("TH1 do not exists"); return -1;}
  auto axis = (x) ? histo2 -> GetXaxis() : histo2 -> GetYaxis();
  
  auto size1 = histo1 ->GetXaxis() -> GetNbins();
  auto size2 = axis -> GetNbins();

  auto m_min_value_2 = axis -> GetBinLowEdge(0)+1;
  auto m_max_value_2 = axis -> GetBinLowEdge(size2)+1;
  
  auto m_min_value_1 = histo1 -> GetXaxis() -> GetBinLowEdge(0)+1;
  auto m_max_value_1 = histo1 -> GetXaxis() -> GetBinLowEdge(size1)+1;

  if (m_min_value_2!=m_min_value_1 || m_max_value_2!=m_max_value_1)
  {// If the ranges are at different
    double const & slope = (m_max_value_1-m_min_value_1)/(m_max_value_2-m_min_value_2);
    double const & intercept = m_min_value_1 - slope*m_min_value_2;

    auto filling_histo (new TH1F("temp", "temp", size2, m_min_value_2, m_max_value_2));
    for (int bin2 = 0; (bin2<size2 && bin2<size1); bin2++)
    {
      auto const & value2 = axis->GetBinCenter(bin2);
      auto const & value1 = value2*slope + intercept;
      auto const & bin1   = histo1->FindBin(value1);
      filling_histo->SetBinContent(bin1, histo1->GetBinContent(bin1));
    }

    AddTH1(histo2, filling_histo, index, x);
    delete filling_histo;
  }
  else AddTH1(histo2, histo1, index, x);
  
  return true;
}

/**
 * @brief Get which bin holds the X = 0
*/
int getBin0(TH1F* spectra)
{
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  std::vector<double> lowEdges(bins);
  spectra -> GetXaxis() -> GetLowEdge(lowEdges.data());
  int bin0 = 0;
  while(lowEdges[bin0] < 0) bin0++;
  return bin0;
}

TH1D* Multiply(TH1D* histo, double const & value)
{
  for (int bin_i = 0; bin_i<histo->GetNbinsX(); ++bin_i) histo->SetBinContent(bin_i, histo->GetBinContent(bin_i)*value);
  return histo;
}

/**
 * @brief Get the mean of the peak of a histogram with one nice single peak
*/
bool getMeanPeak(TH1F* spectra, double & mean)
{
  // Declaration :
  double pospic, amppic, widthpic;
  double Mean, sigma;

  // Extract dump parameters :
  amppic = spectra -> GetMaximum();
  pospic = spectra->GetBinCenter(spectra->GetMaximumBin());
  widthpic = spectra->GetBinCenter(spectra->FindLastBinAbove(amppic*0.8)) - spectra->GetBinCenter(spectra->FindFirstBinAbove(amppic*0.8));

  // Fit the peak :
  auto gaus_pol0 = new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*widthpic,pospic+20*widthpic);
  gaus_pol0 -> SetParameters(amppic, pospic, widthpic, 1);
  gaus_pol0 -> SetRange(pospic-widthpic*20,pospic+widthpic*20);
  spectra -> Fit(gaus_pol0,"R+q");

  if (!gaus_pol0) return false; // Eliminate non existing fits, when fits doesn't converge
  Mean = gaus_pol0->GetParameter(1);
  sigma = gaus_pol0->GetParameter(2);

  auto gaus_pol1 = new TF1("gaus+pol1","gaus(0)+pol1(3)");
  gaus_pol1 -> SetParameters(gaus_pol0->GetParameter(0), gaus_pol0->GetParameter(1), gaus_pol0->GetParameter(2), gaus_pol0->GetParameter(3), 0);
  gaus_pol1 -> SetRange(Mean-sigma*5,Mean+sigma*5);
  spectra -> Fit(gaus_pol1,"R+q");

  if (!gaus_pol1) return false;
  Mean = gaus_pol1->GetParameter(1);
  sigma = gaus_pol1->GetParameter(2);

  auto gaus_pol1_bis = new TF1("gaus+pol2","gaus(0)+pol1(3)");
  gaus_pol1_bis -> SetParameters(gaus_pol1->GetParameter(0), gaus_pol1->GetParameter(1), gaus_pol1->GetParameter(2), gaus_pol1->GetParameter(3), gaus_pol1->GetParameter(4));
  gaus_pol1_bis -> SetRange(Mean-sigma*3,Mean+sigma*3);
  spectra -> Fit(gaus_pol1_bis,"R+q");
  
  if (!gaus_pol1_bis) return false;

  delete gaus_pol0;
  delete gaus_pol1;

  // Extracts the fitted parameters :
  auto fittedPic = gaus_pol1_bis;
  mean = Mean = fittedPic -> GetParameter(1);
  sigma = fittedPic -> GetParameter(2);

  return true;
}

void to_int(TH1* histo)
{
  for (int bin = 0; bin<histo->GetNcells(); ++bin) histo->SetBinContent(bin, int_cast(histo->GetBinContent(bin)));
}

void getData(TH1* histo, std::vector<float> & data)
{
  auto const & size = histo->GetNbinsX()+1;
  data.clear();
  data.reserve(size);
  for (int bin = 1; bin<size; bin++)
  {
    data.push_back(float_cast(histo->GetBinContent(bin)));
  }
}

void getData(TH1* histo, std::vector<double> & data)
{
  auto const & size = histo->GetNbinsX()+1;
  data.clear();
  data.reserve(size);
  for (int bin = 1; bin<size; bin++)
  {
    data.push_back(double_cast(histo->GetBinContent(bin)));
  }
}

int findNextBinBelow(TH1* histo, int & bin, double threshold)
{
  while(histo->GetBinContent(bin++) > threshold)
  {
    if (bin > histo->GetNbinsX()) break;
    else continue;
  } 
  return bin;
}

int findNextBinAbove(TH1* histo, int & bin, double threshold)
{
  while(histo->GetBinContent(bin++) < threshold)
  {
    if (bin > histo->GetNbinsX()) break;
    else continue;
  } 
  return bin;
}

double peak_integral(TH1* histo, int bin_min, int bin_max, TH1* background)
{
  auto const & peak = histo     ->Integral(bin_min, bin_max);
  auto const & bckg = background->Integral(bin_min, bin_max);
  return peak-bckg;
}

double peak_integral(TH1* histo, int bin_min, int bin_max, int smooth_background_it = 20)
{
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = peak_integral(histo, bin_min, bin_max, background);
  delete background;
  return ret;
}

std::vector<double> peak_integral(TH1* histo, std::vector<double> energies, int resolution, int smooth_background_it = 20)
{
  std::vector<double> ret;
  auto background = histo->ShowBackground(smooth_background_it);
  for (auto const & nrj : energies)
  {
    ret.push_back(peak_integral(histo, nrj-resolution, nrj+resolution+1, background));
  }
  delete background;
  return ret;
}

double peak_over_background(TH1* histo, int bin_min, int bin_max, TH1* background)
{
  auto const & peak = peak_integral(histo, bin_min, bin_max, background);
  auto const & bckg = background->Integral(bin_min, bin_max);
  return peak/bckg;
}
double peak_over_background(TH1* histo, int bin_min, int bin_max, int smooth_background_it = 20)
{
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = peak_over_background(histo, bin_min, bin_max, background);
  delete background;
  return ret;
}

double peak_over_total(TH1* histo, int bin_min, int bin_max, TH1* background)
{
  auto total = histo->Integral();
  if (total == 0) return 0;
  else return peak_integral(histo, bin_min, bin_max, background)/total;
}
double peak_over_total(TH1* histo, int bin_min, int bin_max, int smooth_background_it = 20)
{
  return peak_integral(histo, bin_min, bin_max, smooth_background_it)/histo->Integral();
}

double peak_significance(TH1* histo, int bin_min, int bin_max, TH1* background)
{
  auto const & peak_total = histo     ->Integral(bin_min, bin_max);
  auto const & peak_only = peak_integral(histo, bin_min, bin_max, background);
  if (peak_total == 0) return 0;
  else return peak_only/sqrt(peak_total);
}
double peak_significance(TH1* histo, int bin_min, int bin_max, int smooth_background_it = 20)
{
  auto background = histo->ShowBackground(smooth_background_it, "");
  auto ret = peak_significance(histo, bin_min, bin_max, background);
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH1F* 
 */
TH1F* count_to_peak_significance(TH1* histo, int resolution, int smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_significance";
  std::string title = histo->GetName(); title+=";keV;significance;";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
    ret -> SetBinContent(bin, peak_significance(histo, bin-resolution, bin+resolution, background));
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH1F* 
 */
TH1F* count_to_peak_significance(TH1* histo, int resolution, TH1* background)
{
  std::string name = histo->GetName(); name += "_significance";
  std::string title = histo->GetName(); title+=";keV;significance;";
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
    ret -> SetBinContent(bin, peak_significance(histo, bin-resolution, bin+resolution, background));
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH1F* 
 */
TH1F* count_to_peak_over_background(TH1* histo, int resolution, int smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_over_background";
  std::string title = histo->GetName(); title+=";keV;peak_over_background;";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX()-resolution; ++bin)
    ret -> SetBinContent(bin, peak_over_background(histo, bin-resolution, bin+resolution, background));
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution
 * @return TH1F* 
 */
TH1F* count_to_peak_over_total(TH1* histo, int resolution, int smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_over_total";
  std::string title = histo->GetName(); title+=";keV;peak_over_total";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
    ret -> SetBinContent(bin, peak_over_total(histo, bin-resolution, bin+resolution, background));
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH1* (cast it to the actual)
 */
TH1F* count_to_peak_integral(TH1F* histo, int const & resolution, int const & smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_integral";
  std::string title = histo->GetName(); title+=";keV;peak_integral";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
    ret -> SetBinContent(bin, peak_integral(histo, bin-resolution, bin+resolution, background));
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH1* (cast it to the actual)
 */
TH1D* count_to_peak_integral(TH1D* histo, int const & resolution, int const & smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_integral";
  std::string title = histo->GetName(); title+=";keV;peak_integral";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1D(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
    ret -> SetBinContent(bin, peak_integral(histo, bin-resolution, bin+resolution, background));
  delete background;
  return ret;
}

/**
 * @brief 
 * 
 * @param resolution Each peak is at bin += resolution/2
 * @return TH2* 
 */
TH2F* count_to_peak_integral(TH2F* histo, int const & resolution, int const & smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_integral";
  std::string title = histo->GetName(); title+=";keV;"+TString(histo->GetYaxis()->GetTitle())+";peak_integral";
  auto ret = new TH2F(name.c_str(), title.c_str(), 
                      histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax(), 
                      histo->GetNbinsY(), histo->GetYaxis()->GetXmin(), histo->GetYaxis()->GetXmax());
  for (int y = 1; y<=histo->GetNbinsY(); ++y) 
  {
    std::unique_ptr<TH1D> proj (histo->ProjectionX("temp", y, y));
    std::unique_ptr<TH1D> modified_proj (count_to_peak_integral(proj.get(), resolution, smooth_background_it));
    for (int x = 0; x<=histo->GetNbinsX(); ++x) ret->SetBinContent(x, y, modified_proj->GetBinContent(x));
  }
  return ret;
}

namespace CoLib
{
  template<class THist>
  THist* AND(THist* histo1, THist* histo2, int smooth_background_it = 20)
  {
    int bins = histo1->GetNbinsX();
    double min = histo1->GetXaxis()->GetXmin();
    double max = histo1->GetXaxis()->GetXmax();
    if (bins != histo2->GetNbinsX()) bins = (histo1->GetNbinsX() < histo2->GetNbinsX()) ? histo1->GetNbinsX() : histo1->GetNbinsX();
    if (min  != histo2->GetXaxis()->GetXmin()) {error("in AND(THist *histo1, THist *histo2) : axis inconsistent..."); return nullptr;}
    if (max  != histo2->GetXaxis()->GetXmax()) {error("in AND(THist *histo1, THist *histo2) : axis inconsistent..."); return nullptr;}
    TString name = histo1->GetName() + TString("_AND_") + histo1->GetName();
    TString title = histo1->GetTitle() + TString("_AND_") + histo1->GetTitle();
    auto background1 = histo1->ShowBackground(smooth_background_it);
    auto background2 = histo2->ShowBackground(smooth_background_it);
    auto ret = new THist(name, title, bins, min, max);
    for (int bin = 0; bin<bins; ++bin)
    {
      auto const & count_1 = histo1->GetBinContent(bin) - background1->GetBinContent(bin) ;
      auto const & count_2 = histo2->GetBinContent(bin) - background2->GetBinContent(bin) ;
      ret->SetBinContent(bin, (count_1<count_2) ? count_1 : count_2);
    }
    return ret;
  }
}

template<class THist>
THist* operator&(THist histo1, THist histo2) {return CoLib::AND(&histo1, &histo2);}

namespace CoLib
{
  /// @brief Shifts a histogram by 'shift' X value
  /// @param shift Shifts each bin content by 'shift' units of the x axis
  void shiftX(TH1* histo, double shift)
  {
    auto temp = static_cast<TH1*> (histo->Clone(concatenate(histo->GetName(), "_shifted").c_str()));
    auto const & xmin = histo->GetXaxis()->GetXmin();
    auto const & xmax = histo->GetXaxis()->GetXmax();

    auto const & nb_bins = histo->GetNbinsX();
    for (int bin_i = 1; bin_i<nb_bins+1; ++bin_i)
    {
      auto const & value = histo->GetBinCenter(bin_i);
      auto const & shifted_value = value-shift;
      if (shifted_value < xmin|| shifted_value > xmax) 
          temp->SetBinContent(bin_i, 0);
      else temp->SetBinContent(bin_i, histo->Interpolate(shifted_value));
    }
    for (int bin_i = 1; bin_i<nb_bins+1; ++bin_i) histo->SetBinContent(bin_i, temp->GetBinContent(bin_i));
    delete temp;
  }
}

double X_per_bin(TAxis* axis)
{
  return (axis->GetXmax() - axis->GetXmin())/axis->GetNbins();
}

namespace CoLib
{
  /// @brief First-order interpolation to get the value of a non-integer bin, most likely from calibration
  /// @param histo 
  /// @param calibrated_bin 
  /// @return 
  double linearInterpolatedBinContent(TH1* histo, double const & calibrated_bin)
  {
    int bin_i = static_cast<int>(calibrated_bin); //bin_i
    if (0 < bin_i || bin_i > (histo->GetNbinsX()-2)) return 0; // todo Faire attention au -2, c'est peut-être -1 ou -3 dans un TH1
    auto const & a = histo->GetBinContent(bin_i+1) - histo->GetBinContent(bin_i);// a  =  y_i+1 - y_i
    auto const & b = histo->GetBinContent(bin_i)   - a*bin_i;                    // b  =  y_i - a*bin_i
    return a*calibrated_bin+b;
  }

  double linearInterpolatedXContent(TH1F* hist, double const & x)
  {
    int bin = hist->FindBin(x);
    
    // Prevent out-of-range access
    if (bin <= 0) return hist->GetBinContent(bin);
    if (bin >= hist->GetNbinsX()) bin = hist->GetNbinsX() - 1;

    double x0 = hist->GetBinCenter(bin);
    double y0 = hist->GetBinContent(bin);

    double x1 = hist->GetBinCenter(bin + 1);
    double y1 = hist->GetBinContent(bin + 1);

    if (x1 == x0) return y0; // prevent division by zero

    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
  }

  /// @brief Quadratic interpolation to get the value of any random X value
  /// @param hist 
  /// @param x 
  /// @return 
  double quadraticInterpolatedXContent(TH1F* hist, double const & x)
  {
    int bin = hist->FindBin(x);

    // Ensure enough bins around for quadratic interpolation
    if (bin <= 1) bin = 2;
    if (bin >= hist->GetNbinsX() - 1) bin = hist->GetNbinsX() - 1;

    double x0 = hist->GetBinCenter(bin - 1);
    double y0 = hist->GetBinContent(bin - 1);

    double x1 = hist->GetBinCenter(bin);
    double y1 = hist->GetBinContent(bin);

    double x2 = hist->GetBinCenter(bin + 1);
    double y2 = hist->GetBinContent(bin + 1);

    // Lagrange quadratic interpolation formula
    double L0 = ((x - x1)*(x - x2)) / ((x0 - x1)*(x0 - x2));
    double L1 = ((x - x0)*(x - x2)) / ((x1 - x0)*(x1 - x2));
    double L2 = ((x - x0)*(x - x1)) / ((x2 - x0)*(x2 - x1));

    return y0 * L0 + y1 * L1 + y2 * L2;
  }
  
  class InterpolatedSpectrum
  {
  public:
    InterpolatedSpectrum() noexcept = default;
    InterpolatedSpectrum(TH1 const * hist, int order = 1) noexcept {
      set(hist, order);
    }

    void set(TH1 const * hist, int order = 1)
    {
      m_order = order;
      m_coeffs.resize(m_order + 1);
      int nbins = hist->GetNbinsX();
      for (auto & coeff : m_coeffs)
        coeff.resize(nbins + 2); // account for bin numbering starting at 1
    
      for (int bin = 1; bin <= nbins; ++bin)
      {
        if (m_order == 1)
        {
          auto const & coeff = linearInterpolationCoeffs(hist, bin);
          m_coeffs[0][bin] = coeff.second; // b (intercept)
          m_coeffs[1][bin] = coeff.first;  // a (slope)
        }
        else
        {
          throw_error("InterpolatedSpectrum : order must be 1");
        }
      }
      m_ok = true;
    }

    InterpolatedSpectrum& operator=(TH1 const * hist)
    {
      this->set(hist);
      return *this;
    }

    std::pair<double, double> linearInterpolationCoeffs(TH1 const * hist, int const & bin)
    {
      int nbins = hist->GetNbinsX();
    
      if (bin < 2 || bin >= nbins)
      {
        auto const & y = hist->GetBinContent(std::min(std::max(bin, 1), nbins));
        return {0.0, y};
      }
    
      auto const & y0 = hist->GetBinContent(bin);
      auto const & y1 = hist->GetBinContent(bin + 1);
    
      double a = y1 - y0;//    // slope
      double b = y0 - a * bin; // intercept
    
      return {a, b}; // return (a, b)
    }
    
    double operator[](double const & new_bin) const noexcept
    {
      int bin = int_cast(new_bin);
      if (bin < 1 || bin >= static_cast<int>(m_coeffs[0].size())) return 0.0;
    
      return m_coeffs[0][bin] + m_coeffs[1][bin] * new_bin;
    }

    operator bool() const & {return m_ok;}

  private:
    size_t m_order = -1; 
    std::vector<std::vector<double>> m_coeffs;
    bool m_ok = false;
  };

  /**
   * @brief X-Calibrating and Y-Scaling a spectrum
   */
  class CalibAndScale
  {
  public:

    // Constructors and loaders :
    CalibAndScale(){}
    template<class THist> CalibAndScale(THist * histo){this->setHisto(histo);}
    void init()
    {
      m_order = m_coeffs.size()-1;
    }
    CalibAndScale(std::initializer_list<double> initList)
    {
      m_coeffs.clear();
      auto it = initList.begin();
      for (int i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
      m_scale = double_cast(*it++);
      init();
    }
    CalibAndScale& operator=(std::initializer_list<double> initList)
    {
      m_coeffs.clear();
      auto it = initList.begin();
      for (int i = 0; i<initList.size()-1; ++i) m_coeffs.push_back(double_cast(*it++));
      m_scale = double_cast(*it++);
      init();
      return *this;
    }    
    void setCoeffs(std::initializer_list<double> initCoeffs)
    {
      m_coeffs.clear();
      m_coeffs = initCoeffs;
      init();
    }
    void setScale (double scale) {m_scale = scale;}

    CalibAndScale(std::vector<double> const & vec)
    {
      m_coeffs.clear();
      for (int i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
      m_scale = vec.back();
      init();
    }
    CalibAndScale& operator=(std::vector<double> const & vec)
    {
      m_coeffs.clear();
      for (int i = 0; i<vec.size()-1; ++i) m_coeffs.push_back(vec[i]);
      m_scale = vec.back();
      init();
      return *this;
    }
    void setCoeffs(std::vector<double> initCoeffs)
    {
      m_coeffs = initCoeffs;
      init();
    }

    CalibAndScale(CalibAndScale const & other) : m_coeffs(other.m_coeffs), m_scale(other.m_scale) {
      init();
    }

    CalibAndScale& operator=(CalibAndScale const & other)
    {
      m_coeffs = other.m_coeffs;
      m_scale  = other.m_scale ; 
      init();
      return *this;
    }

    template<class THist>
    void setHisto(THist * histo)
    {
      m_base_histo = histo;
      m_interpol.set(histo);
    }

    // Getters :
    std::vector<double> get() const {
      auto ret = m_coeffs;
      ret.push_back(m_scale);
      return ret;
    }
    auto const & getCoeffs() const {return m_coeffs;}
    auto const & getScale () const {return m_scale ;}

    // Methods :

    double calibrate(double const & value) const
    {
      if (m_order < 0) return value;
      double ret = 0.;
      double power = 1.;
      for (int order = 0; order <= m_order; ++order)
      {
        ret += power * m_coeffs[order];
        power *= value;  // Compute value^order iteratively instead of using std::pow
      }
      return ret;
    }

    double operator[](double const & bin) const
    {
      return m_interpol[this->calibrate(bin)]*m_scale;
    }

    /// @brief Calibrate and scale a histogram
    /// @deprecated
    /// @tparam THist: Any TH1
    /// @param hist: Histogram
    /// @return Calibrated and scaled histogram
    template<class THist>
    THist* operator()(THist* const hist) const
    {
      TString name  = hist->GetName()  + TString("_m");
      TString title = hist->GetTitle() + TString("_m");

      if (m_order < 0) // No calibration
      {
        auto ret = dynamic_cast<THist*>(hist->Clone());
        return ret;
      }

      auto xaxis = hist->GetXaxis();
      auto const & bins = xaxis->GetNbins();
      auto const & xmin = xaxis->GetBinLowEdge(1);
      auto const & xmax = xaxis->GetBinLowEdge(bins+1);

      if (m_order == 0) // Simple shift
      {
        auto ret = dynamic_cast<THist*>(hist->Clone());
        shiftX(ret, m_coeffs[0]);
        return ret;
      }
      else // Calibration
      {
        auto ret = new THist(name, title, bins, xmin, xmax);
        InterpolatedSpectrum interpol(hist);
        for (int bin = 1; bin<=bins; ++bin)
        {
          ret->SetBinContent(bin, interpol[this->calibrate(bin)]*m_scale);
        }
        // For expanded regions, some bins may have been skipped
        return ret;
      }
    }

    TH1F* getCalibratedHisto(std::string const & _name) const
    {
      TString name  = (m_base_histo->GetName()  + std::string("_") + _name).c_str();
      TString title = (m_base_histo->GetTitle() + std::string("_") + _name).c_str();

      if (m_order < 0) // No calibration
      {
        auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
        return ret;
      }

      auto xaxis = m_base_histo->GetXaxis();
      auto const & bins = xaxis->GetNbins();
      auto const & xmin = xaxis->GetBinLowEdge(1);
      auto const & xmax = xaxis->GetBinLowEdge(bins+1);

      if (m_order == 0) // Simple shift
      {
        auto ret = dynamic_cast<TH1F*>(m_base_histo->Clone());
        shiftX(ret, m_coeffs[0]);
        return ret;
      }
      else // Calibration
      {
        auto ret = new TH1F(name, title, bins, xmin, xmax);
        for (int bin = 1; bin<=bins; ++bin)
        {
          ret->SetBinContent(bin, m_interpol[this->calibrate(bin)]*m_scale);
        }
        // For expanded regions, some bins may have been skipped
        return ret;
      }
    }

    TH1F const * getHisto() const {return m_base_histo;}

  private:
    TH1F* m_base_histo = nullptr;
    InterpolatedSpectrum m_interpol;
    std::vector<double> m_coeffs;
    double m_scale = 1.;
    int m_order;
  };

  class Chi2Calculator
  {
  public:
    Chi2Calculator(TH1* reference) : m_reference(reference){}
    template<class THist>
    double operator()(THist* const test, CalibAndScale const & calib)
    {
      auto testCal = calib(test);
      testCal->SetName((test->GetName() + std::string("calib : ") + mergeStrings(calib.get(), "_")).c_str());
      // testCal->Write();
      return calculate(testCal);
      // return m_reference->Chi2Test(testCal);
    }
    
    template<class THist> double calculate(THist* const testCal)
    {
      double sum_errors_squared = 0.0;
      auto const & bins = testCal->GetNbinsX();
  
      for (int bin = 0; bin<bins; bin++) if (testCal->GetBinContent(bin)>0)
      {
        // Calculate the difference for this bin :
        auto const & diff = m_reference->GetBinContent(bin)-testCal->GetBinContent(bin);
  
        // Variance of the bin :
        double const & weight = 1/testCal->GetBinContent(bin); // V = sigma² = 1/N
  
        // Add the diff to the total squared diff of the spectra :
        sum_errors_squared += diff*diff*weight;
  
      }
      return sum_errors_squared/bins;
    }

    double calculate(CalibAndScale const & calib)
    {
      double sum_errors_squared = 0.0;

      auto const & bins = calib.getHisto()->GetNbinsX();
  
      for (int bin = 0; bin<bins; bin++) if (calib[bin]>0)
      {
        // Calculate the difference for this bin :
        auto const & diff = m_reference->GetBinContent(bin)-calib[bin];
  
        // Variance of the bin :
        double const & weight = 1/calib[bin]; // V = sigma² = 1/N
  
        // Add the diff to the total squared diff of the spectra :
        sum_errors_squared += diff*diff*weight;
  
      }
      return sum_errors_squared/bins;
    }
    
  private:
    TH1* m_reference = nullptr;
  };

  struct MinimiserVariable
  {
    double min = 0.;
    double max = 0.;
    double step = 0.;
    int nb_steps = 0; 

    MinimiserVariable(std::initializer_list<double> init)
    {
      auto it = init.begin();
      min = double_cast(*it++);
      max = double_cast(*it++);
      nb_steps = int_cast(*it++);
      calculateStep();
    }

    MinimiserVariable& operator=(std::initializer_list<double> init)
    {
      auto it = init.begin();
      min = double_cast(*it++);
      max = double_cast(*it++);
      nb_steps = int_cast(*it++);
      calculateStep();
      return *this;
    }

  private:
    void calculateStep() {step = (max-min)/double_cast(nb_steps);}

  };

  class Minimiser
  {
  public:
    template<class THist>
    Minimiser(THist* reference, THist* test, MinimiserVariable xParam, MinimiserVariable yParam, MinimiserVariable zParam)
    {
      Chi2Calculator chi2Calc(reference);
      CalibAndScale calib(test);
      m_calib.setHisto(test);
      m_chi2map = new TH3F("chi2map", "chi2map;x;y;z", 
                          xParam.nb_steps, xParam.min, xParam.max+1e-5, 
                          yParam.nb_steps, yParam.min, yParam.max+1e-5,
                          zParam.nb_steps, zParam.min, zParam.max+1e-5
                        );

      for (auto stepx = 0; stepx<=xParam.nb_steps; ++stepx)
      {
        for (auto stepy = 0; stepy<=yParam.nb_steps; ++stepy)
        {
          for (auto stepz = 0; stepz<=zParam.nb_steps; ++stepz)
          {
            auto const & x = xParam.min+stepx*xParam.step;
            auto const & y = yParam.min+stepy*yParam.step;
            auto const & z = zParam.min+stepz*zParam.step;
            calib = {x, y, z};

            // auto testCalib = calib.getCalibratedHisto(mergeStrings(calib.get()));
            // auto chi2 = chi2Calc.calculate(testCalib);
            auto chi2 = chi2Calc.calculate(calib);
            if (chi2<m_min_chi2) 
            {
              m_min_chi2 = chi2;
              print(chi2, calib.get());
              m_calib = calib;
            }
            m_chi2map->Fill(x, y, z, chi2);
          }
        }
      }
    }

    auto const & getCalib() const {return m_calib;}
    auto const & getMinChi2() const {return m_min_chi2;}
    auto & getChi2Map() const {return m_chi2map;}

  private:
    double m_min_chi2 = 1e100;
    CalibAndScale m_calib;
    TH3F* m_chi2map = nullptr;
  };
}


///////////////////////////
//   TREE MANIPULATIONS  //
///////////////////////////

std::ostream& operator<<(std::ostream& out, TTree * tree)
{
  tree->Print();
  return out;
}

void alignator(TTree * tree, int *NewIndex)
{
  auto const NHits = tree -> GetEntries();
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Read only the time

  std::vector<ULong64_t> TimeStampBuffer(NHits,0);
  ULong64_t TimeStamp = 0; tree->SetBranchAddress("time", &TimeStamp);

  // First creates a buffer of all the timestamps :
  for (int i = 0; i<NHits; i++)
  {
    tree -> GetEntry(i);
    TimeStampBuffer[i]=TimeStamp;
  }

  // Then computes the correct order :
  int i = 0, j = 0;
  ULong64_t a = 0;
  NewIndex[0]=0;
  for (j=0; j<NHits;j++)
  {
    NewIndex[j]=j;
    a=TimeStampBuffer[j]; //Focus on this time stamp
    i=j;
    // Find the place to insert it amongst the previously sorted timestamps
    while((i > 0) && (TimeStampBuffer[NewIndex[i-1]] > a))
    {
      NewIndex[i]=NewIndex[i-1];
      i--;
    }
    NewIndex[i]=j;
  }
  tree -> SetBranchStatus("*", true); //enables again the whole tree to be read
}

void test_alignator(TTree *tree, int* NewIndex= nullptr, bool useNewIndex = false)
{
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Enables to read only the time
  ULong64_t TimeStamp; tree->SetBranchAddress("time", &TimeStamp);
  ULong64_t PrevTimeStamp = 0; Long64_t j = 0;
  auto maxIt = tree -> GetEntries();
  for (Long64_t i = 0; i < maxIt; i++)
  {
    if (useNewIndex) j = NewIndex[i] ;
    else j = i;
    tree -> GetEntry(j);
    if (static_cast<Long64_t> (TimeStamp - PrevTimeStamp) < 0)
    std::cout << j << " -> " << static_cast<Long64_t> (TimeStamp - PrevTimeStamp) << std::endl;
    PrevTimeStamp = TimeStamp;
  }
  tree -> SetBranchStatus("*", true); //enables again the whole tree
}

class TypeRootMap
{
public:
  TypeRootMap()
  {
    if (!Initialised)
    {
      // Bool :
      m_typeRootMap[typeid(true)          ] = "O";
      
      // Integers :
      m_typeRootMap[typeid(  char_cast(1))] = "B"; m_typeRootMap[typeid( uchar_cast(1))] = "b";
      m_typeRootMap[typeid( short_cast(1))] = "S"; m_typeRootMap[typeid(ushort_cast(1))] = "s";
      m_typeRootMap[typeid(   int_cast(1))] = "I"; m_typeRootMap[typeid(  uint_cast(1))] = "i";
      m_typeRootMap[typeid(  long_cast(1))] = "G"; m_typeRootMap[typeid( ulong_cast(1))] = "g";

      // Floating point :
      m_typeRootMap[typeid(double_cast(1))] = "D"; m_typeRootMap[typeid( float_cast(1))] = "F";

      // ROOT types :
      m_typeRootMap[typeid(Long64_cast(1))] = "L"; m_typeRootMap[typeid(ULong64_cast(1)) ] = "l";

      Initialised = true;
    }
  }

  template<class T>
  std::string operator() (T const & t) const 
  {
     auto typeIndex = std::type_index(typeid(t));
        auto it = m_typeRootMap.find(typeIndex);
        if (it != m_typeRootMap.end()) {
            return it->second;
        } else {
            return "Unknown";
        }
  }

private:
  static bool Initialised;
  std::unordered_map<std::type_index, std::string> m_typeRootMap;
}typeRootMap;

bool TypeRootMap::Initialised = false;

/// @brief Create a branch for a given value and name
template<class T>
auto createBranch(TTree* tree, T * value, std::string const & name, int buffsize = 64000)
{
  auto const & type_root_format = name+"/"+typeRootMap(*value);
  return (tree -> Branch(name.c_str(), value, type_root_format.c_str(), buffsize));
}

/// @brief Create a branch for a given array and name
/// @param name_size: The name of the leaf that holds the size of the array
template<class T>
auto createBranchArray(TTree* tree, T * array, std::string const & name, std::string const & name_size, int buffsize = 64000)
{
  // using **array because it is an array, so *array dereferences the array and **array the first element of the array
  auto const & type_root_format = name+"["+name_size+"]/"+typeRootMap(**array);
  return (tree -> Branch(name.c_str(), array, type_root_format.c_str(), buffsize));
}


/////////////////////////
//   USELESS CLASSES   //
/////////////////////////

/**
 * @brief Not functionnal yet
 * @todo maybe
 * 
 * 1: Add all the files
 * TheTChain chain("Nuball", "/path/to/data/files*.root");
 * chain.Add("/other_path/to/data/files*.root")
 *
 * 2: Setup the chain :
 * chain.set();
 *
 * 3: Links all the variables
 * chain.SetBranchAddress("branch", &variable);
*/
class TheTChain
{
public:
  TheTChain(std::string const & name, std::string const & expression = "", std::string const & readMode = "READ") : m_name(name), m_read_mode(readMode)
  {
    if (expression!="") this -> Add(expression);
  }

  // TTree wrapping :
  void Add(std::string const & expression)
  {
    m_input_files_expressions.push_back(expression);
  }

  template<class... ARGS>
  void SetBranchAddress(ARGS &&... args) {for (auto & tree : m_trees) tree -> SetBranchAddress(std::forward<ARGS>(args)...);}

  // template <class Func, class... ARGS> // Attempt to create a generic wrapping method
  // operator-> ()


  // Class own methods :
  void set();
  bool read(){return true;}

  TTree* operator[] (int const & i) {return m_trees[i];}

  auto begin() {return m_trees.begin();}
  auto end()   {return m_trees.end()  ;}

private:
  std::string m_name = "";
  std::string m_read_mode = "READ";

  void set(std::string const & expression);
  void newTTree(std::string const & fileName)
  {
    m_files.push_back( TFile::Open(fileName.c_str()) );
    m_trees.push_back( m_files.back() -> Get<TTree>(m_name.c_str()) );
  }

  Strings m_input_files_expressions;
  Strings m_files_vec;

  UInt_t    m_tree_cursor = 0;
  ULong64_t m_evt_cursor = 0;
  ULong64_t m_size = 0;

  std::vector<TTree*> m_trees;
  std::vector<TFile*> m_files;
};

void TheTChain::set()
{
  // for (auto const & expression : m_input_files_expressions)
  // {
  //   if (!folder_exists(expression)) {print("folder",getPath(expression),"empty !");return;}
  //   if (expression.back() == '/')
  //   {// If a folder is given then search the whole folder for .root files
  //     findFilesWildcard(expression+"*.root", m_files_vec);
  //   }
  //   else findFilesWildcard(expression, m_files_vec);
  // }
  // print(m_files_vec);
  // for (auto const & filename : m_files_vec) newTTree(filename);
}

/**
 * @brief Binning of a root histogram (TH1) : number of bins, min value, max value
 * 
 */
struct THBinning
{
  THBinning() = default;
  THBinning(std::initializer_list<double> initList)
  {
    if (initList.size() != 3) 
    {
      throw std::invalid_argument("Initialization of THBinning must contain only 3 elements");
    }

    auto it = initList.begin();
    bins = static_cast<int>(*it++);
    min  = static_cast<float>(*it++);
    max  = static_cast<float>(*it  );
  }

  THBinning(double _bins, double _min, double _max) 
  {
    bins = static_cast<int>(_bins);
    min  = static_cast<float>(_min) ;
    max  = static_cast<float>(_max) ;
  }

  // The three parameters :
  int   bins = 0  ;
  float min  = 0.f;
  float max  = 0.f;
};

std::ostream& operator<<(std::ostream& cout, THBinning binning)
{
  cout << binning.bins << " " << binning.min << " " << binning.max << " ";
  return cout;
}

std::vector<double> log_bins(size_t nb_bins, int min, double power)
{
  if (min <= 0) throw std::invalid_argument("min and max must be positive.");
  std::vector<double> ret(nb_bins + 1);
  double factor = min; // Start with the initial minimum value
  
  for (size_t i = 0; i <= nb_bins; ++i) {
      ret[i] = factor * std::pow(power, i);
  }
  return ret;
}

//////////////
// LOG BINS //
//////////////

std::vector<double> log2_bins(size_t nb_bins, int min) {return log_bins(nb_bins, min, 2);}
std::vector<double> log10_bins(size_t nb_bins, int min) {return log_bins(nb_bins, min, 10);}

std::vector<double> linear_bins(size_t nb_bins, double min, double max)
{
  if (min >= max) throw std::invalid_argument("min must be less than max.");
  std::vector<double> ret(nb_bins+1);
  double binWidth = (max-min)/nb_bins;
  for (size_t i = 0; i <= nb_bins; ++i) ret[i] = double(min + i * binWidth);
  return ret;
}


///////////////////
//   COANALYSE   //
///////////////////

namespace CoAnalyse
{
  /**
   * @brief Returns the average number of hits per keV for by default 5 energy interval.
   * @details This is used to estimate the significance of a peak in a low-count spectrum
   * @todo Il semble qu'il y a un décalage de quelques bins
   * 
   * @param histo : gamma spectrum with x axis in keV
   * @param dE_zone : divide the histogram into zones of dE_zone keV large to calculate the count per keV
   * @return std::vector<double> 
   */
  std::vector<double> counts_per_keV(TH1* histo, int dE_zone = 100)
  {
    auto const & nX = histo->GetNbinsX();
    auto const & xmax = histo->GetXaxis()->GetXmax();
    auto const & xmin = histo->GetXaxis()->GetXmin();
    auto const & range = xmax - xmin;
    auto const & n_zones = size_cast(range/dE_zone); // Get the number of zones
    auto const & nX_zone = nX/n_zones;
    std::vector<double> ret(nX+1);
    std::vector<double> data;
    for (size_t zone_i = 0; zone_i<n_zones; ++zone_i)
    {
      auto const & bin_low = 1+zone_i*nX_zone;
      auto const & bin_high = (zone_i+1)*nX_zone;
      auto const & integral = histo->Integral(bin_low, bin_high);
      data.push_back(integral/dE_zone);
    }
    for (int bin_i = 1; bin_i<=nX; ++bin_i) ret[bin_i] = data[int(bin_i/nX_zone)];
    return ret;
  }

  bool inline checkMatrixSquare(TH2* mat) noexcept
  {
    if (!mat) {error(" in CoAnalyse::checkMatrixSquare(TH2* mat) : mat is nullptr"); return false;}
    return (mat->GetNbinsX() == mat->GetNbinsY()
    || mat->GetXaxis()->GetXmin()== mat->GetYaxis()->GetXmin()
    || mat->GetXaxis()->GetXmax()== mat->GetYaxis()->GetXmax());

  }
  /// @brief Vector of pairs of min and max bins 
  using ProjectionsBins = std::vector<std::pair<double,double>>;

  /// @brief Get a sub-histogram between x1 and x2
  template<class THist>
  THist* subHisto(THist* histo, int xmin, int xmax)
  {
    auto name = TString(histo->GetName())+("_"+std::to_string(xmin)+"_"+std::to_string(xmax)).c_str();
    auto bin_low = histo->GetBinLowEdge(xmin);
    auto bin_high = histo->GetBinLowEdge(xmax);
    auto const & N = bin_high-bin_low;
    auto ret = new THist(name, name, N, xmin, xmax);
    int dest_bin = 0;
    for (int bin_i = bin_low; bin_i<=bin_high; ++bin_i) ret->SetBinContent(dest_bin++, histo->GetBinContent(bin_i));
    return ret;
  }

  /// @brief For each X bin, normalise the Y histogram
  void normalizeY(TH2* matrix, double const & factor = 1)
  {
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    for (int x = 1; x<bins_x; x++)
    {
      Float_t maxRow = 0.;
      // 1 : Get the maximum
      for (int y = 1; y<bins_y+1; y++) if(matrix->GetBinContent(x, y) > maxRow) maxRow = matrix->GetBinContent(x, y);
      // 2 : Normalize to set maximum = factor
      if (maxRow>0) for (int y = 1; y<bins_y+1; y++) 
      {
        matrix -> SetBinContent(x, y, factor*matrix->GetBinContent(x, y)/maxRow);
      }
    }
  }

  /// @brief For each X bin, normalise the Y histogram
  TH2F* normalizeYperX(TH2F* matrix, double const & factor = 1)
  {
    auto ret = static_cast<TH2F*>(matrix->Clone(matrix->GetName()+TString("_normalizeYperX")));
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    unique_TH1D projX(matrix->ProjectionX());
    for (int x = 1; x<bins_x; x++)
    {
      auto const & proj = projX->GetBinContent(x);
      if (proj>0) for (int y = 1; y<bins_y+1; y++) 
      {
        ret -> SetBinContent(x, y, factor*matrix->GetBinContent(x, y)/proj*factor);
      }
    }
    return ret;
  }

  /// @brief Normalise the whole bidim
  void normalizeBidim(TH2* matrix, double const & factor = 1.0)
  {
    auto const & bins_x = matrix->GetNbinsX();
    auto const & bins_y = matrix->GetNbinsY();
    double const & max = matrix->GetMaximum();
    if (max>0.) for (int x = 0; x<bins_x+1; x++) for (int y = 0; y<bins_y+1; y++)
    {
      auto const & value = matrix->GetBinContent(x, y);
      if (value>0) matrix -> SetBinContent(x, y, factor*value/max);
    }
  }

  /// @brief Project matrix on Y axis at a given X bin. This fills proj.
  void projectY(TH2* matrix, TH1* proj, int const & binX)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectY");}
    auto const & nbBins = matrix->GetNbinsY();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins,minYaxis(matrix), maxYaxis(matrix));
    for (int binY = 0; binY<nbBins; binY++) proj->SetBinContent(binY, matrix->GetBinContent(binX, binY));
  }

  /// @brief Project matrix on Y axis between bin binXmin included and binXmax excluded [binXmin;binXmax[. This fills proj.
  void projectY(TH2* matrix, TH1* proj, int const & binXmin, int const & binXmax)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectY");}
    auto const & nbBins = matrix->GetNbinsY();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins, minYaxis(matrix), maxYaxis(matrix));
    for (int x = binXmin; x<binXmax; x++) for (int y = 0; y<nbBins; y++) 
      proj->AddBinContent(y, matrix->GetBinContent(x, y));
  }

  /// @brief Project matrix on Y axis between values valueXmin and valueXmax included [valueXmin;valueXmax]. This fills proj.
  void projectY(TH2* matrix, TH1* proj, double const & valueXmin, double const & valueXmax)
  {
    projectY(matrix, proj, matrix->GetXaxis()->FindBin(valueXmin), matrix->GetXaxis()->FindBin(valueXmax));
  }
  
  /// @brief LEGACY
  void removeRandomY(TH2* matrix, int _stopX = -1, int _stopY = -1, bool writeIntermediate = false, ProjectionsBins projections = {{}})
  {
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    int startX = 0;
    int stopX = (_stopX<0) ? bins_x+1 : _stopX;
    int startY = 0;
    int stopY = (_stopY<0) ? bins_y+1 : _stopY;;

    // print("Normalizing...");
    // normalizeY(matrix, 1);// This is in order to have floating points in the z axis
    // normalizeBidim(matrix, 1);// This is in order to have floating points in the z axis

    print("Cloning...");
    auto clone = static_cast<TH2*>(matrix->Clone());
    clone->SetDirectory(nullptr);

    print("Projecting on both axis...");
    std::vector<double> totProjX(bins_x+1);
    std::vector<double> totProjY(bins_y+1);
    for (int x = startX; x<bins_x+1; x++) for (int y = startY; y<bins_y+1; y++) 
    {
      auto const & value = matrix->GetBinContent(x,y);
      totProjX[x] += value;
      totProjY[y] += value;
    }

    print("Subtracting...");
    std::vector<TH2*> intermediate;
    std::vector<std::vector<TH1*>> intermediate_proj(projections.size());
    auto const & total = matrix->Integral();
    for (int x = startX; x<stopX; x++)
    {
      if (x%(stopX/100) == 0) 
      {
        auto advancement = int_cast(100*x/stopX);
        print(advancement, "%");
        if (writeIntermediate && advancement%10 == 0)
        {
          print("Saving at", advancement, "% process");
          std::string matrix_name = matrix->GetName()+std::to_string(advancement);
          intermediate.emplace_back(dynamic_cast<TH2*>(clone->Clone(matrix_name.c_str())));
          for (size_t proj_i = 0; proj_i<projections.size(); proj_i++)
          {
            auto histo = new TH1F();
            auto const & gate = projections[proj_i];
            projectY(intermediate.back(), histo, gate.first, gate.second);
            auto const & histo_name = matrix_name+"_"+std::to_string(gate.first)+"_"+std::to_string(gate.second);
            intermediate_proj[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
          }
        }
      }
      // w = totProjX[x]/total; // Weight of the y spectra at bin x
      for (int y = startY; y<stopY; y++) 
      {
        auto const & sub = totProjY[y] * totProjX[x];
        // auto const & sub = totProjY[y] * w * matrix->GetBinContent(x, y);
        // if (sub>0) for (int x2 = startX; x2<stopX; x2++) 
        // {
          // auto const & global_bin = matrix->GetBin(x2, y);
          // auto const & new_value = clone->GetBinContent(global_bin)-sub;
          // if (new_value>0) clone -> SetBinContent(global_bin, new_value);
          auto const & new_value = clone->GetBinContent(x, y)-sub/total;
          clone -> SetBinContent(x, y, new_value);
          // clone -> SetBinContent(x, y, (new_value>0) ? new_value : 0);
        // }
      }
    }

    print("Subtraction done, copying back...");
    delete matrix;
    matrix = static_cast<TH2*>(clone->Clone());

    // print("Renormalising...");
    // normalizeBidim(matrix, 1);

    print("RemoveRandomY done.");
    if (writeIntermediate)
    {
      print("Writing intermediate steps...");
      std::string filename = std::string("Intermediate_")+matrix->GetName()+".root";
      auto file = TFile::Open(filename.c_str(), "recreate");
      file->cd();
      matrix->Write();
      // for (auto & histo : intermediate) if (histo!=nullptr) histo -> Write();
      for (auto & projections : intermediate_proj) for (auto & histo : projections) if (histo!=nullptr) histo -> Write();
      file->Write();
      file->Close();
      print(filename, "written");
    }
  }

  /// @brief Set Y histogram proj in matrix at binX.
  void setX(TH2* matrix, TH1* proj, int const & binX)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::setX");}
    if (proj == nullptr) {throw_error("Projection histo nullptr in CoAnalyse::setX");}
    for (int binY = 0; binY<matrix->GetNbinsY(); binY++) matrix->SetBinContent(binX, binY, proj->GetBinContent(binY));
  }

  /// @brief Project the whole matrix on the X axis. This fills proj.
  void projectX(TH2* matrix, TH1* proj)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectX");}
    auto const & nbBins = matrix->GetNbinsX();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins, minXaxis(matrix), maxXaxis(matrix));
    for (int y = 0; y<nbBins; y++) for (int x = 0; x<nbBins; x++) proj->AddBinContent(x, matrix->GetBinContent(x, y));
  }
  
  /// @brief Project on X axis at a given Y bin. This fills proj.
  void projectX(TH2* matrix, TH1* proj, int const & binY)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectX");}
    auto const & nbBins = matrix->GetNbinsX();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins,minXaxis(matrix), maxXaxis(matrix));
    for (int x = 0; x<nbBins; x++) proj->SetBinContent(x, matrix->GetBinContent(x, binY));
  }

  /// @brief Project on X axis between bin binYmin included and binYmax excluded [binYmin;binYmax[. This fills proj.
  void projectX(TH2* matrix, TH1* proj, int const & binYmin, int const & binYmax)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectX");}
    auto const & nbBins = matrix->GetNbinsX();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins, minXaxis(matrix), maxXaxis(matrix));
    for (int y = binYmin; y<binYmax; y++) for (int x = 0; x<nbBins; x++) 
      proj->AddBinContent(x, matrix->GetBinContent(x, y));
  }

  /// @brief Project on Y axis between values binYmin and binYmax included [binYmin;binYmax]. This fills proj.
  void projectX(TH2* matrix, TH1* proj, double const & binYmin, double const & binYmax)
  {
    projectX(matrix, proj, matrix->GetYaxis()->FindBin(binYmin), matrix->GetYaxis()->FindBin(binYmax));
  }

  /// @brief Set X histogram proj in matrix at binY.
  void setY(TH2* matrix, TH1* proj, int const & binY)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::setY");}
    if (proj == nullptr) {throw_error("Projection histo nullptr in CoAnalyse::setY");}
    for (int binX = 0; binX<matrix->GetNbinsX(); binX++) matrix->SetBinContent(binX, binY, proj->GetBinContent(binY));
  }

  /// @deprecated
  void removeRandomBidim(TH2* matrix, int iterations = 1, bool save_intermediate = false, 
                        ProjectionsBins projectionsY = {{}}, ProjectionsBins projectionsX = {{}})
  {
    // matrix->Rebin2D(2);
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    int startX = 0;
    int stopX = bins_x+1;
    int startY = 0;
    int stopY = bins_y+1;
    std::string matrix_name = matrix->GetName();
    auto const & iterations_sqr = iterations*iterations;
    auto const & proportions = 2;
    // auto const & iterations_pow4 = iterations*iterations*iterations*iterations;
    // auto const maximum = matrix->GetMaximum();

    std::vector<std::vector<TH1*>> intermediate_projX(projectionsX.size());
    std::vector<std::vector<TH1*>> intermediate_projY(projectionsY.size());
    std::vector<TH1D*> save_totProjX;
    std::vector<TH1D*> save_totProjY;
    // std::vector<TH2*> clones;
    std::vector<double> integrals(iterations_sqr,0.);

    // std::vector<std::vector<std::vector<double>>> save_sub(iterations_sqr);
    // std::vector<std::vector<double>> sub_moyX(iterations_sqr);
    // std::vector<std::vector<double>> sub_moyY(iterations_sqr);

    std::vector<TH1D*> save_sub_projX;
    std::vector<TH1D*> save_sub_projY;
    for (int it = 0; it<iterations; it++) 
    {
      // save_sub[it].resize(bins_x+1);
      // sub_moyX[it].resize(bins_x+1);
      for (int x = 0; x<bins_x+1; x++) 
      {
        // sub_moyX[it][x] = 0.0;
        
        // save_sub[it][x].resize(bins_y+1);
        // for (int y = 0; y<bins_y+1; y++) save_sub [it][x][y] = 0.0;
      }

      // sub_moyY[it].resize(bins_y+1);
      // for (int y = 0; y<bins_y+1; y++) sub_moyY[it][y] = 0.0;
    }

    std::vector<double> totProjX(bins_x+1);
    std::vector<double> totProjY(bins_y+1);
    std::vector<double> totProjX_buf(bins_x+1);
    std::vector<double> totProjY_buf(bins_y+1);
    for (int x = startX; x<bins_x+1; x++) 
    {
      for (int y = startY; y<bins_y+1; y++) 
      {
        auto const & value = matrix->GetBinContent(x,y);
        totProjX[x] += value;
        totProjY[y] += value;
        totProjX_buf[x] += value;
        totProjY_buf[y] += value;
      }
    }

    // Remove the extremal lines to avoid edge effects :
    for (int x = 0; x<stopX; x++) 
    {
      matrix->SetBinContent(x,0,0);
      matrix->SetBinContent(x,bins_x,0);
    }
    for (int y = 0; y<stopX; y++) matrix->SetBinContent(0,y,0);

    auto firstTotProjX = matrix->ProjectionX("firstTotProjX");
    auto firstTotProjY = matrix->ProjectionY("firstTotProjY");

    std::vector<std::vector<double>> sub_array;
    fill2D(sub_array, stopX, stopY, 0.0);
    std::vector<std::vector<double>> speed_array;
    fill2D(speed_array, stopX, stopY, 0.0);
    // std::vector<std::vector<double>> real_sub_array;
    // fill2D(real_sub_array, stopX, stopY, 0.0);

    if (save_intermediate)
    {
      save_totProjX.emplace_back(dynamic_cast<TH1D*>(firstTotProjX->Clone("totProjX_init")));
      save_totProjY.emplace_back(dynamic_cast<TH1D*>(firstTotProjY->Clone("totProjY_init")));
      for (size_t proj_i = 0; proj_i<projectionsY.size(); proj_i++)
      {
        auto histo = new TH1F();
        auto const & gate = projectionsY[proj_i];
        if (gate.first == gate.second) continue;
        projectY(matrix, histo, gate.first, gate.second);
        auto const & histo_name = matrix_name+"_projY_"+std::to_string(int(gate.first))+"_"+std::to_string(int(gate.second))+"_init";
        intermediate_projY[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
      }
      for (size_t proj_i = 0; proj_i<projectionsX.size(); proj_i++)
      {
        auto histo = new TH1F();
        auto const & gate = projectionsX[proj_i];
        if (gate.first == gate.second) continue;
        projectX(matrix, histo, gate.first, gate.second);
        auto const & histo_name = matrix_name+"_projX_"+std::to_string(gate.first)+"_"+std::to_string(gate.second)+"_init";
        intermediate_projX[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
      }
    }

    print("Subtracting", matrix_name, "with", iterations, "iterations...");
    for (int it = 0; it<iterations; it++)
    {
      print("Iteration", it);
      if(save_intermediate)
      {
        save_totProjX.emplace_back(dynamic_cast<TH1D*>(firstTotProjX->Clone(("totProjX_"+std::to_string(int_cast(it))).c_str())));
        save_totProjY.emplace_back(dynamic_cast<TH1D*>(firstTotProjY->Clone(("totProjY_"+std::to_string(int_cast(it))).c_str())));
        // if (it>0) save_sub_projX.emplace_back(dynamic_cast<TH1D*>(firstTotProjX->Clone(("sub_projX_"+std::to_string((int)(it-1))).c_str())));
        // if (it>0) save_sub_projY.emplace_back(dynamic_cast<TH1D*>(firstTotProjY->Clone(("sub_projY_"+std::to_string((int)(it-1))).c_str())));

        for (int x = 0; x<bins_x+1; x++) 
        {
          save_totProjX[it]->SetBinContent(x, totProjX[x]);
          // if (it>0) save_sub_projX[it-1]->SetBinContent(x, sub_moyX[it-1][x]);
        }
        for (int y = 0; y<bins_y+1; y++) 
        {
          save_totProjY[it]->SetBinContent(y, totProjY[y]);
          // if (it>0) save_sub_projY[it-1]->SetBinContent(y, sub_moyY[it-1][y]);
        }
      }

      auto const total = matrix->Integral();
      // auto const & prev_total = (it>0) ? clones[it-1]->Integral() : total;
      // auto const & prev_total2 = (it>0) ? clones[it-1]->Integral() : total;

      for (int x = startX; x<stopX; x++)
      {
        for (int y = startY; y<stopY; y++) 
        {
          double value = matrix->GetBinContent(x, y);
          if (value == 0) continue;

          // V1 :
          // auto diff = (it>0) ? clones[it-1]->GetBinContent(x,y)*total/prev_total - value : 0;
          // auto const & sub = (totProjY[y] * totProjX[x])/(iterations*total);
          // auto const & new_value = value - sub - sqrt(diff)/iterations;

          // V2 :
          // save_sub[it][x][y] = (totProjX[x] * totProjY[y])/(iterations*total);
          // auto const new_value = value - save_sub[it][x][y];
          // totProjX_buf[x] -= save_sub[it][x][y];
          // totProjY_buf[y] -= save_sub[it][x][y];

          // V3 :
          double sub = (totProjX[x] * totProjY[y])/(proportions*total);
          if (iterations>1) sub *= ( 1. - (sub/(proportions*value)));
          else sub *= proportions;

          matrix -> SetBinContent(x, y, value - sub);

          totProjX_buf[x] -= sub;
          totProjY_buf[y] -= sub;

          // V4 :
          // sub_array[x][y] = (totProjX[x] * totProjY[y])/(proportions*total);
          // speed_array[x][y] = sub_array[x][y]/value;
        }
      }

      // V4 : (the iterations are done excluding the extrema lines to avoid edge effect :)
      // for (int x = 1; x<bins_x; x++)
      // {
      //   for (int y = 1; y<bins_y; y++) 
      //   {
      //     // Do an average of the speed around the bin :
      //     auto const & mean_speed = speed_array[x][y];
      //       // speed_array[x-1][y-1]*0.0313 + speed_array[x][y-1]*0.0938 + speed_array[x+1][y-1]*0.0313 + 
      //       // speed_array[x-1][y]  *0.0938 + speed_array[x][y]  *0.5    + speed_array[x+1][y]  *0.0938 + 
      //       // speed_array[x-1][y+1]*0.0313 + speed_array[x][y+1]*0.0938 + speed_array[x+1][y+1]*0.0313 ;
      //     auto const & sub = sub_array[x][y] * ( 1 - mean_speed);
      //     matrix -> SetBinContent(x, y, matrix->GetBinContent(x,y) - sub);
      //     totProjX_buf[x] -= sub;
      //     totProjY_buf[y] -= sub;
      //   }
      // }

      if (save_intermediate) 
      {
        // Project on the axis :
        for (size_t proj_i = 0; proj_i<projectionsY.size(); proj_i++)
        {
          auto histo = new TH1F();
          auto const & gate = projectionsY[proj_i];
          if (gate.first == gate.second) continue;
          projectY(matrix, histo, gate.first, gate.second);
          auto const & histo_name = matrix_name+"_projY_"+std::to_string(int(gate.first))+"_"+std::to_string(int(gate.second))+"_"+std::to_string(it);
          intermediate_projY[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
        }
        for (size_t proj_i = 0; proj_i<projectionsX.size(); proj_i++)
        {
          auto histo = new TH1F();
          auto const & gate = projectionsX[proj_i];
          if (gate.first == gate.second) continue;
          projectX(matrix, histo, gate.first, gate.second);
          auto const & histo_name = matrix_name+"_projX_"+std::to_string(gate.first)+"_"+std::to_string(gate.second)+"_"+std::to_string(it);
          intermediate_projX[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
        }
      }

      // Update the total projections :
      totProjX = totProjX_buf;
      totProjY = totProjY_buf;

      // normalizeBidim(matrix, maximum);
    }

    if (save_intermediate)
    {
      print("Writing intermediate steps...");
      std::string filename = "Background_removed_"+matrix_name+".root";
      auto file = TFile::Open(filename.c_str(), "recreate");
      file->cd();
      matrix->Write();
      for (auto & histo : save_totProjX) if (histo!=nullptr) histo -> Write();
      for (auto & histo : save_totProjY) if (histo!=nullptr) histo -> Write();
      for (auto & histo : save_sub_projX) if (histo!=nullptr) histo -> Write();
      for (auto & histo : save_sub_projY) if (histo!=nullptr) histo -> Write();
      for (auto & projections : intermediate_projX) for (auto & histo : projections) if (histo!=nullptr) histo -> Write();
      for (auto & projections : intermediate_projY) for (auto & histo : projections) if (histo!=nullptr) histo -> Write();
      file->Write();
      file->Close();
      print(filename, "written");
    }
  }

  /// @deprecated
  std::vector<double> extractBackgroundArray(std::vector<double> & source, int const & nsmooth = 10)
  {
    print("deprecated (", nsmooth, ")");
    // auto s = new TSpectrum();
    // s->Background(source.data(),source.size(),nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
    // s->Delete();
    return source;
  }

  /// @deprecated
  std::vector<double> extractBackgroundArray(TH1F * histo, int const & nsmooth = 10)
  {
    print("deprecated (", histo->GetName(), nsmooth, ")");
    // auto const & nbins = histo->GetNbinsX();
    // std::vector<double> source(nbins);
    // for (int bin=0;bin<nbins;bin++) source[bin]=histo->GetBinContent(bin+1);
    // return extractBackgroundArray(source, nsmooth);
    return std::vector<double>(0);
  }

  /**
   * @brief Remove the background in the given histo
   * 
   * @param histo: Can be TH1 or TH2 histogram
   * @param niter: Choose a higher number of iterations if the peaks have high resolution (5-10 for LaBr3, 20 for Ge)
   * @param fit_options: options from TH1::ShowBackground
   * @param bidim_options: @deprecated
   *    - "X" (default)  : Loop through the X bins, find the background on the Y projection
   *    - "Y" :            Loop through the Y bins, find the background on the X projection
   *    - "S" (symmetric): Loop through the X bins, find the background on the Y projection, then symmetrize the bidim
   * @param nice: if true, the minimum values of the bin content is 1.
   */
  void removeBackground(TH1 * histo, int const & niter = 20, std::string const & fit_options = "", bool nice = false) noexcept
  {
    if (!histo || histo->IsZombie()) return;
    auto const & dim = histo->GetDimension();
    if (dim == 1)
    {
      auto const & background = histo -> ShowBackground(niter, fit_options.c_str());
      for (int bin=0; bin<histo->GetNbinsX(); bin++) 
      {
        auto const & new_value = histo->GetBinContent(bin) - int_cast(background->GetBinContent(bin));
        if (nice) histo->SetBinContent(bin, (new_value<1) ? 1 : new_value);
        else histo->SetBinContent(bin, new_value);
      }
    }

    else if (dim == 2)
    {
      error ("weird, removeBackground(TH2*) should have been called ....");
      return;
      // char choice = 0; // 0 : X, 1 : Y, 2 : symmetric
      // // if (bidim_options.find("Y")) choice = 1;
      // // if (bidim_options.find("S")) choice = 2;
      // auto bidim = dynamic_cast<TH2F*>(histo);

      // auto const & nXbins = bidim -> GetNbinsX();
      // auto const & nYbins = bidim -> GetNbinsY();

      // if (choice == 2)
      // {
      //   if (nXbins != nYbins) {print("CoAnalyse::removeBackground for 2D spectra is suited only for symmetric spectra"); return;}
      // }

      // switch (choice)
      // {
      //   case 0: case 2: 
      //     // Subtract the background of Y spectra gating on each X bins
      //     for (int binX = 0; binX<nXbins; binX++)
      //     {
      //       TH1F* histo1D = nullptr; 
      //       projectY(bidim, histo1D, binX);
      //       removeBackground(histo1D, niter);
      //       setX(bidim, histo1D, binX);
      //       delete histo1D;
      //     }
      //   break;

      //   case 1:
      //     // Subtract the background of X spectra gating on each Y bins
      //     for (int binY = 0; binY<nYbins; binY++)
      //     {
      //       TH1F* histo1D = nullptr; new TH1F("temp","temp",nYbins, bidim->GetYaxis()->GetXmax(), bidim->GetYaxis()->GetXmin());
      //       projectX(bidim, histo1D, binY);
      //       removeBackground(histo1D, niter);
      //       setY(bidim, histo1D, binY);
      //       delete histo1D;
      //     }
      //   break;
      // }

      // if (choice == 2)
      // {
      //   //2. Re-symmetrize the matrix : (PROTOTYPAL !)
      //   print("Symmetrization : ");
      //   for (int binY = 0; binY<nYbins; binY++) for (int binX = 0; binX<nXbins; binX++)
      //   {
      //     histo->SetBinContent(binX, binY, histo->GetBinContent(binY, binX));
      //   }
      // }
    }
  }

  std::vector<bool> mainPeaksLookup(TH1D* histo, double const & sigma = 2., double const & threshold = 0.05, double const & n_sigma = 2, int const & verbose = 0, bool remove511 = false)
  {
    if (!histo) {error("in CoAnalyse::mainPeaksLookup() : histo is nullptr"); return std::vector<bool>(0);}
    std::vector<double> peaks;
    auto xAxis = histo->GetXaxis();
    if (remove511) xAxis->SetRangeUser(0, 500);
    histo->ShowPeaks(sigma, "", threshold); // Get the peaks from the Root ShowPeaks method
    auto markers = static_cast<TPolyMarker*>(histo->GetListOfFunctions()->FindObject("TPolyMarker")); // Extracts the peaks list
    auto N = markers->GetN(); // Get the number of peaks
    auto raw_X = markers->GetX(); // Get the X values of the maximum of the peak (double*)
    for (int peak_i = 0; peak_i<N; ++peak_i) peaks.push_back(raw_X[peak_i]);
    if (remove511)
    {
      xAxis->UnZoom();
      xAxis->SetRangeUser(520, xAxis->GetXmax());
      auto markers2 = static_cast<TPolyMarker*>(histo->GetListOfFunctions()->FindObject("TPolyMarker")); // Extracts the peaks list
      auto N2 = markers2->GetN(); // Get the number of peaks
      auto raw_X2 = markers2->GetX(); // Get the peaks values of the maximum of the peak (double*)
      for (int peak_i = 0; peak_i<N2; ++peak_i) peaks.push_back(raw_X2[peak_i]);
    }
    if (verbose>0) print(N, "peaks found");

    std::vector<bool> lookup; lookup.reserve(histo->GetNbinsX()); // Prepare the lookup vector
    print(histo->GetNbinsX());
    int bin = 1; // Iterator over the bins of the histogram
    for (auto const & peak : peaks)// Loop through the peaks 
    {
      // By default += 2 sigma captures 95% of the peak's surface
      auto const & peak_begin = peak-n_sigma*sigma; // The beginning of the peak 
      auto const & peak_end = peak+n_sigma*sigma+1; // The end of the peak 
      for (;bin<peak_begin; ++bin) lookup.push_back(false); // Loop through the bins. While before the beginning of the peak, fill false
      for (;bin<peak_end  ; ++bin) lookup.push_back(true);  // When in the peak, fill true
      if (verbose>1) println(" ", peak);
    }
    if (verbose > 1) print();
    for (;bin<histo->GetNbinsX()+1; ++bin) lookup.push_back(false); // After the last peak, fill false until the end
    if (remove511) for (int bin_i = 505; bin_i<517; ++bin_i)lookup[bin_i] = true;
    return lookup;
  }

  /// @brief Based on Radware methods D.C. Radford/Nucl. Instr. and Meth. in Phys. Res. A 361 (1995) 306-316
  /// @param choice: 0 : classic radware | 1 : Palameta and Waddington (PW) | 2 : Palameta and Waddington asymetric
  void removeBackground(
    TH2 * histo,
    int const & niter = 20,
    uchar const & choice = 0,
    double const & sigmaX = 2.,
    double const & sigmaY = 2.,
    double const & threshold = 0.05,
    bool remove511 = false)
  {
    // if (!checkMatrixSquare(histo)) {error ("The matrix must be square"); return;}

    auto const & T = histo->Integral();

    auto projX0 = histo->ProjectionX("projX0"); // Get the total X projection of the matrix
    auto bckgX = projX0->ShowBackground(niter); // Get the total X projection's fitted background 
    to_int(bckgX);
    auto projX = static_cast<TH1D*>(projX0->Clone("projX")); // Prepare the background-clean total X projection
    projX->Add(bckgX, -1); // Calculate the background-subtracted spectra

    auto projY0 = histo->ProjectionY("projY0");
    auto bckgY = projY0->ShowBackground(niter);
    to_int(bckgY);
    auto projY = static_cast<TH1D*>(projY0->Clone("projY"));
    projY->Add(bckgY, -1);

    // Extract informations from the histogram :
    auto xAxis = histo->GetXaxis();
    auto yaxis = histo->GetYaxis();
    auto const & Nx = xAxis->GetNbins()+1;
    auto const & xmin = xAxis->GetXmin();
    auto const & xmax = xAxis->GetXmax();
    auto const & Ny = yaxis->GetNbins()+1;
    auto const & ymin = yaxis->GetXmin();
    auto const & ymax = yaxis->GetXmax();

    auto bckg_clean = new TH2F("bckg_clean","bckg_clean", Nx,xmin,xmax, Ny,ymin,ymax);
    auto bckg2D = new TH2F("bckg2D","bckg2D", Nx,xmin,xmax, Ny,ymin,ymax);
    
    if(choice == 0) // classic radware
    {
      for (int x=0; x<Nx; x++) for (int y=0; y<Ny; y++) 
      {
        // auto const & Px = projX0->GetBinContent(x);
        // auto const & Py = projY0->GetBinContent(y);
        auto const & px = projX->GetBinContent(x);
        auto const & bx = bckgX->GetBinContent(x);
        auto const & py = projY->GetBinContent(y);
        auto const & by = bckgY->GetBinContent(y);
        // auto const & bx2 = bckgX->GetBinContent(y);
        // auto const & by2 = bckgY->GetBinContent(x);
        // The bin xy has the contribution of true coincidence E1*E2, as well as E1*background2 + E2*background1 + background1*background2
        // We are interested only at the true coincidence, the background is therefore :
        auto const & bckg_at_xy = int_cast((px*by + py*bx + bx*by)/T);
        bckg2D->SetBinContent(x, y, bckg_at_xy);
        auto const & new_value = histo->GetBinContent(x, y) - bckg_at_xy;
        bckg_clean->SetBinContent(x, y, new_value);
      }
    }
    else if (choice == 1 || choice == 2) // Palameta and Waddington
    {
      //1. Finds the biggest peaks in the total projections :
      // Promotes negative values to 0 in the background-substracted projections :
      for (int bin = 1; bin<Nx; ++bin)
      {
        if(projX->GetBinContent(bin) < 0) projX->SetBinContent(bin, 0);
        if(projY->GetBinContent(bin) < 0) projY->SetBinContent(bin, 0);
      }

      // Get the biggest peaks in each projection
      auto const & peaksX = mainPeaksLookup(projX, sigmaX, threshold, 1, 1, remove511);
      auto const & peaksY = mainPeaksLookup(projY, sigmaY, threshold, 1, 1, remove511);
      auto projX_bis = new TH1D("projX_bis", "projX_bis", Nx,xmin,xmax);
      auto projY_bis = new TH1D("projY_bis", "projY_bis", Ny,ymin,ymax);
      // auto proj_diag = new TH1D("proj_diag", "proj_diag", 2*Nx,xmin,2*xmax);
      int sum_y = 0; // for projX
      int sum_x = 0; // for projY
      
      // Create the projection without the contribution of the main peaks in projection :
      for (int bin_i = 1; bin_i<Nx; ++bin_i)
      { // i=x for projX, i=y for projY
        sum_y = 0; // Sum over all y for the x bin
        sum_x = 0; // Sum over all x for the y bin
        for (int bin_j = 1; bin_j<Ny; ++bin_j)
        { // j=y for projX, j=x for projY
          auto const & Mxy = histo->GetBinContent(bin_i, bin_j);
          if (peaksY[bin_j]) sum_y+=Mxy;
          if (peaksX[bin_j]) sum_x+=Mxy;
        }
        projX_bis->SetBinContent(bin_i, sum_y);
        projY_bis->SetBinContent(bin_i, sum_x);
      }

      //2. Perform the background subtraction
      // Calculate the constant S and A used for the method (in the classic method, S=T and A=0 )
      auto S = projX_bis->Integral();

      // Attempt for asymetric matrix :
      if (choice == 2) 
      {
        S += projX_bis->Integral();
        S /= 2.;
      }
      double A = 0.0; 
      for (int x = 0; x<Nx; ++x) if (peaksX[x]) A+=projX_bis->GetBinContent(x);
      if (choice == 2) for (int y = 0; y<Ny; ++y) if (peaksY[y]) A+=projY_bis->GetBinContent(y);
      A/=S;
      if (choice == 2) A/=2.;

      print("T, S, A", T, S, A);
      
      for (int x = 1; x<Nx; ++x) for (int y = 1; y<Ny; ++y)
      {
        auto const & Px_bis = projX_bis->GetBinContent(x);
        auto const & Py_bis = projY_bis->GetBinContent(y);
        auto const & bx = bckgX->GetBinContent(x);
        auto const & by = bckgY->GetBinContent(y);

        auto const & bckg_at_xy = int_cast((bx*Py_bis + by*Px_bis - A*bx*by)/S);
        bckg2D->SetBinContent(x, y, bckg_at_xy);
        auto const & new_value = histo->GetBinContent(x, y) - bckg_at_xy;
        bckg_clean->SetBinContent(x, y, new_value);
      }
    }
    for (int x=0; x<Nx; ++x) for (int y=0; y<Ny; ++y) histo->SetBinContent(x, y, bckg_clean->GetBinContent(x, y));
  }

  TH1D* projectDiagonals(TH2* histo)
  {
    if (!checkMatrixSquare(histo)) {error("projectDiagonals(TH2*) : the matrix must be square"); return (new TH1D("void","void",1,0,1));}
    auto diag = new TH1D("diagProj","diagProj", 2*histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), 2*histo->GetXaxis()->GetXmax());
    auto const & nb_bins = histo->GetNbinsX()+1;
    for (int bin_x = 1; bin_x < nb_bins; ++bin_x) for (int bin_y = 1; bin_y < bin_x; ++bin_y)
    {
      auto const & value_xy = histo->GetBinContent(bin_x, bin_y);
      auto const & value_yx = histo->GetBinContent(bin_y, bin_x);
      auto const & diag_bin = bin_x+bin_y;
      auto const & old_value = diag->GetBinContent(diag_bin);
      diag->SetBinContent(diag_bin, old_value+value_xy+value_yx);
    }
    return diag;
  }

  /// @brief TODO
  void removeDiagonals(TH2* histo, int nb_iter = 20, int choice = 0)
  {
    if (choice == 0)
    {
      auto InitProjDiag = projectDiagonals(histo);InitProjDiag->SetName("init proj");
      auto InitbckgProjDiag = InitProjDiag->ShowBackground(nb_iter, "q");
      InitProjDiag->Add(InitbckgProjDiag, -1);
      InitProjDiag->SetLineColor(kRed);
      InitProjDiag->Draw();

      double power = 1.; 
      for (int i = 0; i<nb_iter; ++i) 
      {
        // 1 peform a first iteration
        auto projDiag = projectDiagonals(histo);projDiag->SetName("init proj");
        // auto bckgProjDiag = projDiag->ShowBackground(nb_iter, "q");
        auto const & T = histo->Integral();
        auto const & T2 = projDiag->Integral();
        print(T,T2,T2/T);
        auto clone = static_cast<TH2*>(histo->Clone("projDiag_temp"));
        for (int bin_x = 1; bin_x < histo->GetNbinsX()+1; ++bin_x) for (int bin_y = 1; bin_y < bin_x; ++bin_y)
        {
          // First part : (bottom left corner)
          auto const & init_value_xy = clone->GetBinContent(bin_x, bin_y);
          auto const & init_value_yx = clone->GetBinContent(bin_y, bin_x);
          auto const & diag_value = projDiag->GetBinContent(bin_x+bin_y);
          // auto const & diag_bckg_value = bckgProjDiag->GetBinContent(bin_x+bin_y);
          // auto const & correction_xy = int_cast((diag_value*diag_bckg_value)/T);
          // auto const & correction_yx = int_cast((diag_value*diag_bckg_value)/T);
          auto const & correction_xy = int_cast(power*diag_value*init_value_xy/T2);
          auto const & correction_yx = int_cast(power*diag_value*init_value_yx/T2);
          histo->SetBinContent(bin_x, bin_y, init_value_xy - correction_xy);
          histo->SetBinContent(bin_y, bin_x, init_value_yx - correction_yx);

          // Second part :(up right corner)
          bin_y = bin_x-bin_y;
          auto const & init_value_xy_bis = clone->GetBinContent(bin_x, bin_y);
          auto const & init_value_yx_bis = clone->GetBinContent(bin_y, bin_x);
          auto const & diag_value_bis = projDiag->GetBinContent(bin_x+bin_y);
          // auto const & diag_bckg_value_bis = bckgProjDiag->GetBinContent(bin_x+bin_y);
          // auto const & correction_xy_bis = int_cast((diag_value_bis*init_value_yx_bis)/T);
          // auto const & correction_yx_bis = int_cast((diag_value_bis*init_value_yx_bis)/T);
          auto const & correction_xy_bis = int_cast(power*diag_value_bis*init_value_xy_bis/T2);
          auto const & correction_yx_bis = int_cast(power*diag_value_bis*init_value_yx_bis/T2);
          histo->SetBinContent(bin_x, bin_y, init_value_xy_bis - correction_xy_bis);
          histo->SetBinContent(bin_y, bin_x, init_value_xy_bis - correction_yx_bis);

          bin_y = bin_x-bin_y;// Reset bin_y for next iteration
        }
        // //2 Determine how much it worked
        auto after = projectDiagonals(histo);
        auto diff = (TH1D*)projDiag->Clone("diff");
        diff->Add(after, -1);
        power = 1/diff->Integral();
      }
    }
    else if (choice == 1)
    {
      
    }
    auto after = projectDiagonals(histo);
    removeBackground(after);
    after->Draw("same");
  }

  template<typename T> T LLS(T const & value) {return (log(log(sqrt(std::abs(value)+1.)+1.)+1.));}
  template<typename T> T LLS_inverse(T const & value) {return exp(exp(value -1)-1) * exp(exp(value-1)-1) -1;}

  TH1F* LLS_convertor(TH1F* h)
  {
    auto ret = (TH1F*) h->Clone(h->GetName()+TString("_LLS"));
    for (int x = 0; x<=h->GetNbinsX(); ++x) 
    {
      auto const & value = LLS(h->GetBinContent(x));
      ret->SetBinContent(x, value);
    }
    return ret;
  }

  TH1F* SNIP_background(TH1F* h, int m, bool show = true)
  {
    auto b = (TH1F*) h->Clone("test");
    b->Reset();
    auto const & N = h->GetNbinsX();
    std::vector v(N, 0);
    std::vector w(N, 0);
    for (int bin = 0; bin<N; ++bin) v[bin] = h->GetBinContent(bin);
    // for (int bin = 0; bin<N; ++bin) v[bin] = LLS(h->GetBinContent(bin));
    for (int p = 1; p<m; ++p)
    {
      for (int bin = p+1; bin<=N; ++bin)
      {
        auto const & a1 = v[bin];
        auto const & a2 = (v[bin-p]+v[bin+p])/2;
        w[bin] = std::min(a1, a2);
      }
      for (int bin = p+1; bin<=N; ++bin) v[bin] = w[bin];
    }
    for (int bin = 0; bin<N; ++bin) b->SetBinContent(bin, v[bin]);
    // for (int bin = 0; bin<N; ++bin) b->SetBinContent(bin, LLS_inverse(v[bin]));
    if (show)
    {
      h->Draw();
      b->SetLineColor(kRed);
      b->Draw("same");
    }
    return b;
  }

  
  /// @brief Tests the existence of two peaks separated by gate_size (e.g. two different gamma-rays feeding or decaying from the same state)
  /// @attention The background must have been removed 
  TH1F* moving_gates(TH1* hist, int gate_size)
  {
    auto const & name = hist->GetName()+std::string("_moving_gates_")+std::to_string(gate_size);
    auto const & Nbins = hist->GetNbinsX();
    auto const & xmin = hist->GetXaxis()->GetXmin();
    auto const & xmax = hist->GetXaxis()->GetXmax();
    auto ret = new TH1F(name.c_str(), name.c_str(), Nbins, xmin, xmax);
    for (int bin = gate_size+3; bin<Nbins; ++bin)
    {
      auto const & value_low = hist->GetBinContent(bin-gate_size);
      auto const & value_high = hist->GetBinContent(bin);
      ret ->SetBinContent(bin, value_low * value_high);
    }
    return ret;
  }

  /// @brief Tests the existence of two peaks separated by gate_size (e.g. two different gamma-rays feeding or decaying from the same state)
  /// @details Writes down the minimum value for each bin and bin+gate_size bin
  /// @attention The background must have been removed 
  TH1F* AND_shifted(TH1* hist, int shift)
  {
    auto const & name = hist->GetName()+std::string("_AND_shifted_")+std::to_string(shift);
    auto const & Nbins = hist->GetNbinsX();
    auto const & xmin = hist->GetXaxis()->GetXmin();
    auto const & xmax = hist->GetXaxis()->GetXmax();
    auto ret = new TH1F(name.c_str(), name.c_str(), Nbins, xmin, xmax);
    for (int bin = shift+1; bin<Nbins; ++bin)
    {
      auto const & value = hist->GetBinContent(bin-shift);
      auto const & value_shifted = hist->GetBinContent(bin);
      ret ->SetBinContent(bin, (value < value_shifted) ? value : value_shifted );
    }
    return ret;
  }

  void test(TH2F* histo)
  {
    removeBackground(histo);
    // removeBackground(histo);
    // histo->ProjectionX("h642",640,644)->Draw();
    // auto test = moving_gates(histo->ProjectionX("h642",640,644), 279);
    // test->Draw();
    // removeDiagonals(histo);
    // projectDiagonals(histo)->Draw();
    // histo->Draw("colz");
    // new TCanvas;
    // histo->ProjectionY("myProjY",641*2,643*2)->Draw();
    // new TCanvas;
    // histo->ProjectionX("myProjX",641*2,643*2)->Draw();
  }

};


/////////////////////
//   Manage pads   //
/////////////////////

template<class THist = TH1>
std::vector<THist*> pad_get_histos(TPad * pad = nullptr)
{
  std::vector<THist*> ret;
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return ret;}
  }

  TList *primitives = gPad->GetListOfPrimitives();
  TIterator *it = primitives->MakeIterator();
  TObject *obj;
  TClass *hist_class = TClass::GetClass(typeid(THist));

  while ((obj = it->Next()) != nullptr) 
  {
    if (obj->IsA()->InheritsFrom(hist_class))
    {
      ret.push_back(dynamic_cast<THist*>(obj));
    }
  }
  delete it;
  return ret;
}

template<class THist = TH1>
std::map<std::string, THist*> pad_get_histos_map(TPad * pad = nullptr)
{
  std::map<std::string, THist*> ret;
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return ret;}
  }

  TList *primitives = gPad->GetListOfPrimitives();
  TIterator *it = primitives->MakeIterator();
  TObject *obj;
  TClass *hist_class = TClass::GetClass(typeid(THist));

  while ((obj = it->Next()) != nullptr) 
  {
    if (obj->IsA()->InheritsFrom(hist_class))
    {
      ret.emplace(obj->GetName(), dynamic_cast<THist*>(obj));
    }
  }
  delete it;
  return ret;
}

/**
 * @brief 
 * @todo MAYBE NOT PORTABLE !!! the static_cast works fine, but one should create a myDynamicCast<TH1D>(TH1F) and reverse instead
 * @tparam THist 
 * @param name 
 * @param pad 
 * @return THist* 
 */
template<class THist = TH1>
THist* pad_get_histo(std::string name, TPad * pad = nullptr)
{
  THist* ret = nullptr;
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return ret;}
  }

  TList *primitives = gPad->GetListOfPrimitives();
  TIterator *it = primitives->MakeIterator();
  TObject *obj;
  TClass *hist_class = TClass::GetClass(typeid(THist));

  while ((obj = it->Next()) != nullptr) 
  {
    if (name == obj->GetName())
    {
      ret = static_cast<THist*>(obj);
      if (!ret) error("pad_get_histo : can't cast", obj->ClassName(), "to", hist_class->GetName());
      break;
    }
  }
  // delete it;
  print(ret);
  return ret;
}

template<class THist = TH1>
Strings pad_get_names_of(TPad * pad = nullptr)
{
  Strings ret;
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return ret;}
  }
  auto histos = pad_get_histos(pad);
  for (auto const & histo : histos) ret.push_back(histo->GetName());
  return ret;
}

/**
 * @brief Sets the Y axis so that one can see the minimum and the maximum of each spectrum
 * 
 * @param pad 
 */
void pad_set_Y_axis_nice(TPad * pad = nullptr)
{
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos(pad);

  std::vector<double> mins;
  std::vector<double> maxs;
  for (auto const & histo : histos)
  {
    mins.push_back(histo->GetMinimum()*0.9);
    maxs.push_back(histo->GetMaximum()*1.1);
  }
  histos[0]->GetYaxis()->SetRangeUser(minimum(mins), maximum(maxs));
}

void pad_set_colors(TPad * pad = nullptr)
{
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos(pad);

  for (size_t histo_i = 0; histo_i<histos.size(); ++histo_i)
  {
    histos[histo_i]->SetLineColor(getROOTniceColors(histo_i));    
  }
}

void pad_set_title_with_names(TPad * pad = nullptr)
{
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos(pad);
  for (auto & histo : histos) histo->SetTitle(histo->GetName());
}

void pad_remove_stats(TPad * pad = nullptr)
{
  if (!pad) 
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histo0 = pad_get_histos(pad)[0];
  histo0->SetStats(0);
  pad->Update();
  gPad->Update();
}

bool pad_has_legend(TPad* pad = nullptr)
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return false;}
  }
  TList *primitives = gPad->GetListOfPrimitives();
  TIterator *it = primitives->MakeIterator();
  TObject *obj;

  while ((obj = it->Next()) != nullptr) 
  {
    if (obj->IsA()->InheritsFrom("TLegend"))
    {
      return true;
    }
  }
  return false;
}

template<class THist = TH1>
void pad_normalize_histos(TPad* pad = nullptr, TString options = "nosw2")
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<THist>(pad);
  if (histos.empty()){error("no", TClass::GetClass(typeid(THist))->GetName(), "drawn in pad"); return;}
  double maxi = 0;
  for (auto const & hist : histos) if (maxi < hist->GetMaximum()) maxi = hist->GetMaximum();
  if (maxi==0.0) {error("max is 0"); return;}
  bool hasLegend = pad_has_legend(pad);
  int i = 0;
  for (auto & hist : histos) 
  {
    gPad->GetListOfPrimitives()->Remove(hist);
    hist->Scale(maxi/hist->GetMaximum(), options);
    if (i++ == 0) hist->Draw(); // i is incremented after the comparison
    else hist->Draw("same");
  }
  if (hasLegend) gPad->BuildLegend();
  pad->Update();
}

template<class THist = TH1>
void pad_normalize_histos_min(TPad* pad = nullptr, TString options = "nosw2")
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<THist>(pad);
  if (histos.empty()){error("no", TClass::GetClass(typeid(THist))->GetName(), "drawn in pad"); return;}
  double mini = histos[0]->GetMaximum();
  for (auto const & hist : histos) if (mini > hist->GetMaximum()) mini = hist->GetMaximum();
  if (mini==0.0) {error("min is 0"); return;}
  bool hasLegend = pad_has_legend(pad);
  int i = 0;
  for (auto & hist : histos) 
  {
    gPad->GetListOfPrimitives()->Remove(hist);
    hist->Scale(mini/hist->GetMaximum(), options);
    if (i++ == 0) hist->Draw();
    else hist->Draw("same");
  }
  if (hasLegend) gPad->BuildLegend();
  pad->Update();
}

template<class THist = TH1>
void pad_normalize_histos_click(TPad* pad = nullptr, TString options = "nosw2")
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<TH1>(pad);
  if (histos.empty()){error("no", TClass::GetClass(typeid(THist))->GetName(), "drawn in pad"); return;}
  
  double x, y;
  GetPoint(pad, x, y);
  auto bin = histos[0]->GetXaxis()->FindBin(x);

  double maxi = 0;
  for (auto const & hist : histos) if (maxi < hist->GetBinContent(bin)) maxi = hist->GetMaximum();
  if (maxi==0.0) {error("max is 0"); return;}
  bool hasLegend = pad_has_legend(pad);
  int i = 0;
  for (auto & hist : histos) 
  {
    gPad->GetListOfPrimitives()->Remove(hist);
    hist->Scale(maxi/hist->GetBinContent(bin), options);
    if (i++ == 0) hist->Draw();
    else hist->Draw("same");
  }
  if (hasLegend) gPad->BuildLegend();
  pad->Update();
}

template<class THist = TH1>
void pad_normalize_histos_click_min(TPad* pad = nullptr, TString options = "nosw2")
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<TH1>(pad);
  if (histos.empty()){error("no", TClass::GetClass(typeid(THist))->GetName(), "drawn in pad"); return;}
  
  double x, y;
  GetPoint(pad, x, y);
  auto bin = histos[0]->GetXaxis()->FindBin(x);

  double mini = histos[0]->GetBinContent(bin);
  for (auto const & hist : histos) if (mini > hist->GetBinContent(bin)) mini = hist->GetBinContent(bin);
  if (mini==0.0) {error("min is 0"); return;}
  bool hasLegend = pad_has_legend(pad);
  int i = 0;
  for (auto & hist : histos) 
  {
    gPad->GetListOfPrimitives()->Remove(hist);
    hist->Scale(mini/hist->GetBinContent(bin), options);
    if (i++ == 0) hist->Draw();
    else hist->Draw("same");
  }
  if (hasLegend) gPad->BuildLegend();
  pad->Update();
}

void pad_subtract_histos(TPad* pad = nullptr)
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<TH1>(pad);
  if (histos.size() != 2) {error("pad_subtract_histos : needs exactly two histos"); return;}
  histos[0]->GetXaxis()->UnZoom();
  histos[1]->GetXaxis()->UnZoom();
  histos[0]->Add(histos[1], -1);
  if (pad_has_legend(pad)) gPad->BuildLegend();
  pad->Update();
}

void pad_show_peaks(Double_t sigma=2, Option_t *option="", Double_t threshold=0.05, TPad* pad = nullptr)
{
  if (!pad)
  {
    pad = (TPad*)gPad;
    if (!pad) {error("no pad"); return;}
  }
  auto histos = pad_get_histos<TH1>(pad);
  if (histos.size() != 1) {error("pad_show_peaks : implemented for one histogram"); return;}
  auto histo = histos[0];
  histo->ShowPeaks(sigma, option, threshold);
  auto markers = static_cast<TPolyMarker*>(histo->GetListOfFunctions()->FindObject("TPolyMarker")); // Extracts the peaks list
  auto N = markers->GetN(); // Get the number of peaks
  auto raw_X = markers->GetX(); // Get the X values of the maximum of the peak (double*)
  auto raw_Y = markers->GetY(); // Get the X values of the maximum of the peak (double*)
  
  std::vector<TLatex*> latex;
  for (int i = 0; i<N; ++i)
  {
    latex.push_back(new TLatex());
    latex.back()->SetTextSize(0.04);
    latex.back()->SetTextAlign(12); // Align the text
    std::string text = std::to_string(int(raw_X[i]-0.5));
    latex.back()->DrawLatex(raw_X[i], raw_Y[i], text.c_str());
  }
}


////////////////////////////
//   Manage histo files   //
////////////////////////////

Strings get_list_histo(TFile * file, std::string const & class_name = "TH1F")
{
  Strings ret;
  if (!file) {error("in get_list_histo(TFile * file, std::string class_name) : file is nullptr"); return ret;}
  auto list = file->GetListOfKeys();
  for (auto&& keyAsObj : *list)
  {
    auto key = dynamic_cast<TKey*>(keyAsObj);
    if(key->GetClassName() == class_name)
    {
      ret.push_back(key->GetName());
    }
  }
  return ret;
}

template <class THist>
std::map<std::string, THist> get_map_histo(TFile * file, std::string const & class_name = "TH1F")
{
  auto names = get_list_histo(file, class_name);
  std::map<std::string, THist> ret;
  for (auto const & name : names)
  {
    ret.emplace(name, static_cast<THist>(file->Get(name.c_str())));
  }
  return ret;
}

Strings get_TH1F_names(std::string const & filename)
{
  auto file = TFile::Open(filename.c_str());
  auto ret =  get_list_histo(file, "TH1F");
  file->Close();
  return ret;
}

Strings get_TH1F_names(TFile * file)
{
  return get_list_histo(file, "TH1F");
}

using TH1F_map = std::map<std::string, TH1F*>;

TH1F_map get_TH1F_map(TFile * file)
{
  TH1F_map ret;
  auto names = get_TH1F_names(file);
  for (auto const & name : names)
  {
    ret.emplace(name, file->Get<TH1F>(name.c_str()));
  }
  return ret;
}

TH1F_map get_TH1F_map(TFile * file, Strings & names)
{
  TH1F_map ret;
  names = get_TH1F_names(file);
  for (auto const & name : names)
  {
    ret.emplace(name, file->Get<TH1F>(name.c_str()));
  }
  return ret;
}

/**
 * @brief Get the list of all the object of a certain class (TH1F, TH2F...) inside a TFile
 * @details
 * TFile* file(TFile::Open("file.root","read"));
 * auto list = file_get_names_of<TH1F>(file);
 * 
 * If no file is passed as parameter, reads the current file.
 * Internally perform a file->cd()
 */

template<class T>
Strings file_get_names_of(TFile* file = nullptr)
{
  // init
  Strings ret;
  T *temp_obj = new T(); 

  // Check the files :
  if (file == nullptr) file = gFile;
  if (!file) { error("in file_get_names_of<", temp_obj->ClassName(), ">(TFile* file): file is nullptr"); return ret;}
  file->cd();

  // Get the class name :
  auto const & classname = temp_obj->ClassName();
  
  // Loop over the list of keys of every object in the TFile :
  auto list = file->GetListOfKeys();
  for (auto&& keyAsObj : *list)
  {
    auto key = dynamic_cast<TKey*>(keyAsObj);
    if(strcmp(key->GetClassName(), classname) == 0) ret.push_back(key->GetName());
  }
  delete temp_obj;
  return ret;
}


/**
 * @brief Creates a map of all the object of a certain class (TH1F, TH2F...) inside a TFile, indexed by their name
 * @details
 * TFile* file(TFile::Open("file.root","read"));
 * auto list = file_get_map_of<TH1F>(file);
 * 
 * If no file is passed as parameter, reads the current file.
 * Internally perform a file->cd()
 */

template<class T>
std::map<std::string, T*> file_get_map_of(TFile* file = nullptr)
{
  // init
  std::map<std::string, T*> ret;
  T temp_obj; 

  // Check the files :
  if (file == nullptr) file = gFile;
  if (!file) { error("in file_get_map_of<", temp_obj.ClassName(), ">(TFile* file): file is nullptr"); return ret;}
  file->cd();

  // Get the class name :
  auto const & classname = temp_obj.ClassName();
  
  // Loop over the list of keys of every object in the TFile :
  auto list = file->GetListOfKeys();
  for (auto&& keyAsObj : *list)
  {
    auto key = dynamic_cast<TKey*>(keyAsObj);
    if(strcmp(key->GetClassName(), classname) == 0) 
    {
      T* obj = dynamic_cast<T*>(key->ReadObj());
      if (obj)
      {
        ret.emplace(obj->GetName(), obj);
      }
    }
  }

  return ret;
}

////////////
// OTHERS //
////////////

// Clone an empty histogram with the same binning
TH1F* cloneEmpty(const TH1F* histo, const std::string& name = "", const std::string& title = "")
{
  if (!histo) throw std::invalid_argument("Input histogram is null!");
  auto xaxis = histo->GetXaxis();
  int N_binsX = xaxis->GetNbins();
  std::string new_name = name.empty() ? std::string(histo->GetName()) + "_Clone" : name;
  std::string new_title = title.empty() ? histo->GetTitle() : title;
  return new TH1F(new_name.c_str(), new_title.c_str(), N_binsX, xaxis->GetXmin(), xaxis->GetXmax());
}

// Compute the first derivative of a histogram
TH1F* derivate(const TH1F* histo, size_t smooth = 1) 
{
  if (!histo) throw std::invalid_argument("Input histogram is null!");
  if (smooth == 0) throw std::invalid_argument("Smoothing factor must be greater than 0!");

  auto result = cloneEmpty(histo, histo->GetName()+std::string("der"));
  size_t N = histo->GetNbinsX();

  for (size_t bin = 1; bin <= N; ++bin)
  { // ROOT bins are 1-indexed
    size_t lower_bin = std::max(1ul, bin - smooth); // Ensure within valid range
    size_t upper_bin = std::min(N, bin + smooth); // Ensure within valid range

    double low_sum = 0.0, up_sum = 0.0;

    // Sum left and right bin contents
    for (size_t i = lower_bin; i < bin; ++i) low_sum += histo->GetBinContent(i);
    for (size_t i = bin + 1; i <= upper_bin; ++i) up_sum += histo->GetBinContent(i);

    // Calculate the derivative
    double smooth_range = (upper_bin - bin) + (bin - lower_bin);
    result->SetBinContent(bin, (up_sum - low_sum) / smooth_range);
  }

  return result;
}

// Compute the second derivative of a histogram
TH1F* derivate2(const TH1F* histo, size_t smooth = 1) 
{
  auto intermediate = derivate(histo, smooth);
  auto result = derivate(intermediate, smooth);
  delete intermediate; // Free intermediate memory
  return result;
}

/// @brief Attempt to adapt the derivative with different smoothing approaches
/// @param: choice == 0, derivative with quadratic weight
/// choise == 1, slope of points before and after
/// @todo
TH1F* peaky(const TH1F* histo, int choice = 0, size_t smooth = 1)
{
  if (!histo) throw std::invalid_argument("Input histogram is null!");
  if (smooth == 0) throw std::invalid_argument("Smoothing factor must be greater than 0!");

  auto result = cloneEmpty(histo, histo->GetName()+std::string("der"));
  size_t N = histo->GetNbinsX();

  std::vector<double> before;
  std::vector<double> after;
  std::vector<double> before_X; linspace(before_X,smooth+1);
  std::vector<double> after_X; linspace(after_X,3,smooth+1);


  for (size_t bin = 1+smooth; bin <= N-smooth; ++bin)
  { // ROOT bins are 1-indexed
    size_t lower_bin = std::max(1ul, bin - smooth); // Ensure within valid range
    size_t upper_bin = std::min(N, bin + smooth); // Ensure within valid range

    if (choice == 0)
    {
      double low_sum = 0.0, up_sum = 0.0;

      // Sum left and right bin contents
      for (size_t i = lower_bin; i < bin; ++i) low_sum += histo->GetBinContent(i)/((bin-i)*(bin-i));
      for (size_t i = bin + 1; i <= upper_bin; ++i) up_sum += histo->GetBinContent(i)/((i-bin)*(i-bin));

      // Calculate the derivative
      double smooth_range = (upper_bin - bin) + (bin - lower_bin);
      result->SetBinContent(bin, (up_sum - low_sum) / smooth_range);
    }

    else if (choice == 1)
    {
      before.clear();
      after.clear();

      for (size_t i = lower_bin; i < bin; ++i) before.push_back(histo->GetBinContent(i));
      before.push_back(histo->GetBinContent(bin));
      after.push_back(histo->GetBinContent(bin));
      for (size_t i = bin + 1; i <= upper_bin; ++i) after.push_back(histo->GetBinContent(i));

      // auto graph_before = new TGraph(smooth, before.data(), before.data());
      // auto graph_after = new TGraph(smooth, after.data(), after.data());

      // TODO
      // TF1 fitFunc("fitFunc", "[0] + [1]*x", 0, 6);
      // graph_before->Fit(&fitFunc);
      // int slope_before = 
      // graph_after->Fit(&fitFunc);
    }

  }

  return result;
}

// Compute the second derivative of a histogram with different smoothing approaches
TH1F* peaky2(const TH1F* histo, size_t smooth = 1) 
{
  auto intermediate = peaky(histo, smooth);
  auto result = peaky(intermediate, smooth);
  delete intermediate; // Free intermediate memory
  return result;
}



template<class THist>
THist* mergeAllMatching(std::string expression, TFile* file = nullptr)
{
  if (file == nullptr) file = gFile;
  THist temp_histo;
  auto list = file_get_names_of<THist>(file);
  auto matching_list = CoLib::match_regex(list, expression);
  if (matching_list.empty()) {error("no", temp_histo.ClassName(), "matching regex", expression, "in file", file->GetName()); return nullptr;}

  auto name = expression;
  replace_all(name, "*", "_");
  auto ret = CoLib::clone<THist>(matching_list[0], name.c_str());

  for (size_t i = 0; i<matching_list.size(); ++i) 
  {
    print("merging", matching_list[i].c_str());
    auto histo = file->Get<THist>(matching_list[i].c_str());
    ret->Add(histo);
    CoLib::unload(histo);
  }
  return ret;
}

template<>
TH1F* mergeAllMatching(std::string expression, TFile* file)
{
  if (file == nullptr) file = gFile;
  TH1F temp_histo;
  auto list = file_get_names_of<TH1F>(file);
  auto matching_list = CoLib::match_regex(list, expression);
  if (matching_list.empty()) {error("no", temp_histo.ClassName(), "matching regex", expression, "in file", file->GetName()); return nullptr;}

  auto name = expression;
  replace_all(name, "*", "_");
  auto ret = CoLib::clone<TH1F>(matching_list[0], name.c_str());

  for (size_t i = 0; i<matching_list.size(); ++i) 
  {
    print("merging", matching_list[i].c_str());
    auto histo = file->Get<TH1F>(matching_list[i].c_str());
    ret->Add(histo);
    CoLib::unload(histo);
  }
  return ret;
}

void resize_view_range(TH1F * histo)
{
  histo->GetXaxis()->SetRange(histo->FindFirstBinAbove(histo->GetMinimum()), histo->FindLastBinAbove(histo->GetMinimum()));
}
void resize_view_range(TH1F * histo, float const & min)
{
  histo->GetXaxis()->SetRange(min, histo->FindLastBinAbove(histo->GetMinimum()));
}

/// @brief allows one to fuse all the histograms with the same name from different files
void fuse_all_histo(std::string const & folder, std::string const & outRoot = "fused_histo.root", bool const & bidim = true)
{
  auto const files = list_files_in_folder(folder, {"root"});
  bool first_file = true;
  std::vector<std::unique_ptr<TH1>> all_TH1F;
  for (auto const & filename : files)
  {
    auto file = TFile::Open(filename.c_str(), "READ");
    auto list = file->GetListOfKeys();
    size_t nb_histos = 0;
    print(filename);
    for (auto&& keyAsObj : *list)
    {
      std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
      std::string className =  key->GetClassName();
      if(className == "TH1F" || (bidim && className == "TH2F"))
      {
        std::unique_ptr<TObject> obj (key->ReadObj());
        auto histo = dynamic_cast<TH1*>(obj.get());
        std::string name = histo->GetName();
        if (first_file) all_TH1F.emplace_back(std::unique_ptr<TH1>(dynamic_cast<TH1*>(histo->Clone((name).c_str()))));
        else
        {
          if (nb_histos >= all_TH1F.size()) 
          {
            all_TH1F.emplace(all_TH1F.begin()+nb_histos, std::unique_ptr<TH1>(dynamic_cast<TH1*>(histo->Clone((name).c_str()))));
            nb_histos++;
            continue;
          }
          else if (name != all_TH1F[nb_histos]->GetName()) 
          {
            // If the two files has at least one histogram more or less, one need to find it :
            auto const checkpoint = nb_histos;
            do {nb_histos++;} while (nb_histos<all_TH1F.size() && name != all_TH1F[nb_histos]->GetName());

            // If not found it means it does not exist and needs to be created at the current position :
            if (nb_histos == all_TH1F.size())
            {
              auto const & it = all_TH1F.begin()+checkpoint;
              all_TH1F.emplace(it, std::unique_ptr<TH1>(dynamic_cast<TH1*>(histo->Clone((name).c_str()))));
              nb_histos = checkpoint+1;
              print(all_TH1F[checkpoint]->GetName(), "created");
              continue;
            }
          }
          all_TH1F[nb_histos]->Add(histo);
        }
        nb_histos++;
      }
    }
    first_file = false;
  }

  unique_TFile outFile(TFile::Open(outRoot.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : all_TH1F) histo -> Write();
  outFile -> Write();
  outFile -> Close();
  print(outRoot, "written");
}

// void onClick(Int_t event, Int_t x, Int_t y, TObject* obj) {
//     if (event == 11) {  // Left mouse button click
//         TH1F* hist = dynamic_cast<TH1F*>(obj);
//         if (hist) {
//             Double_t xValue = hist->GetXaxis()->GetBinCenter(hist->GetXaxis()->FindBin(gPad->AbsPixeltoX(x)));
//             printf("Clicked at x-value: %.2f\n", xValue);
//         }
//     }
// }

/// @brief Draws all the TH1F of a given file one by one
/// @attention Only works in CINT environnement (= macro only)
void draw_all_TH1(std::string const & filename, int minX = 0, int maxX = 0, int rebin = 1, std::string pattern = "")
{
  auto file = TFile::Open(filename.c_str(), "READ");
  auto list = file->GetListOfKeys();
  auto c1 = new TCanvas("c1");
  for (auto&& keyAsObj : *list)
  {
    std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
    if(std::string(key->GetClassName()) == "TH1F" || std::string(key->GetClassName()) == "TH1D")
    {
      std::unique_ptr<TObject> obj (key->ReadObj());
      auto histo = dynamic_cast<TH1F*>(obj.get());
      std::string name = histo->GetName();
      if (pattern != "" && !found(name, pattern)) continue;
      print(name);
      if (histo->Integral()<2) continue;
      histo->Rebin(rebin);
      if (maxX == 0) maxX = histo->GetXaxis()->GetBinCenter(histo->GetNbinsX());
      histo->GetXaxis()->SetRangeUser(minX, maxX);

      c1->cd();
      histo->Draw();
      gPad->WaitPrimitive();
      c1->Update();
    }
  }
}

/**
 * @brief Gets the ponderated Y mean value of a TH1F between two gates
 */
double MeanBetweenEdges(TH1F* hist, double edge1, double edge2) 
{
  // Find the bin indices corresponding to the edges
  int bin1 = hist->FindBin(edge1);
  int bin2 = hist->FindBin(edge2);

  // Calculate the mean value of bin centers between the edges
  double sum = 0.0;
  int count = 0;
  for (int i = bin1; i <= bin2; ++i) {
      sum += hist->GetBinCenter(i) * hist->GetBinContent(i);
      count += hist->GetBinContent(i);
  }

  // Return the mean value :
  return (count != 0) ? sum / count : 0.0;
}

/////////////////////////////
// User friendly functions //
/////////////////////////////

void removeFits(TH1* histo)
{
  auto funcs = histo->GetListOfFunctions();
  for (int i = 0; i < funcs->GetSize(); ++i) 
  {
    TObject* obj = funcs->At(i);
    if (obj && obj->InheritsFrom(TF1::Class())) 
    {
      funcs->Remove(obj);
      delete obj; // Delete the fit function object
      --i; // Decrement index because funcs size has changed
    }
  }
}

CoLib::Point selectPoint(TH1* histo, std::string const & instructions)
{
  double x = 0; double y = 0;
  gPad->SetTitle(instructions.c_str());
  histo->SetTitle(instructions.c_str());
  GetPoint(gPad->cd(), x, y);
  gPad->Update();
  return {x, y};
}

double selectPointX(TH1* histo, std::string const & instructions)
{
  double x = 0; double y = 0;
  gPad->SetTitle(instructions.c_str());
  histo->SetTitle(instructions.c_str());
  GetPoint(gPad->cd(), x, y);
  gPad->Update();
  return x;
}

double selectYPoint(TH1* histo, std::string const & instructions)
{
  double x = 0; double y = 0;
  gPad->SetTitle(instructions.c_str());
  histo->SetTitle(instructions.c_str());
  GetPoint(gPad->cd(), x, y);
  gPad->Update();
  return y;
}

void addBinContent(TH1* histo, int const & bin, double const & value)
{
  histo->SetBinContent(bin, histo->GetBinContent(bin)-value);
}

void addBinContent(TH2* histo, int const & binx, int const & biny, double const & value)
{
  histo->SetBinContent(binx, biny, histo->GetBinContent(binx, biny)-value);
}

void addBinContent(TH3* histo, int const & binx, int const & biny, int const & binz, double const & value)
{
  histo->SetBinContent(binx, biny, binz, histo->GetBinContent(binx, biny, binz)-value);
}

TH1D* myProjectionX(TH2* histo, std::string const & name, double const & xvalue_min, double const & xvalue_max, double const & xvalue_min_bckg, double const & xvalue_max_bckg, int peak_norm_min, int peak_norm_max)
{
  TH1D* proj = histo->ProjectionX(name.c_str(), xvalue_min, xvalue_max);
  std::unique_ptr<TH1D> bckg (histo->ProjectionX((name+"_bckg").c_str(), xvalue_min_bckg, xvalue_max_bckg));
  proj->Add(bckg.get(), -peak_integral(proj, peak_norm_min, peak_norm_max)/peak_integral(bckg.get(), peak_norm_min, peak_norm_max));
  return proj;
}

TH1D* myProjectionX(TH2* histo, std::string const & name, double const & xvalue_min, double const & xvalue_max, double const & xvalue_min_bckg, double const & xvalue_max_bckg)
{
  TH1D* proj = histo->ProjectionX(name.c_str(), xvalue_min, xvalue_max);
  std::unique_ptr<TH1D> bckg (histo->ProjectionX((name+"_bckg").c_str(), xvalue_min_bckg, xvalue_max_bckg));
  proj->Add(bckg.get(), -1);
  return proj;
}

TH1D* myProjectionX(TH2* histo, std::string const & name, double const & x_value, double const & resolution)
{
  if (!gPad) histo->Draw("colz");
  auto projY = histo->ProjectionY(concatenate("projY_", histo->GetName()).c_str());
  projY->GetYaxis()->SetRangeUser(x_value-5*resolution, x_value+5*resolution);
  double const & xvalue_min = selectPointX(projY, "Select low edge of peak");
  double const & xvalue_max = selectPointX(projY, "Select high edge of peak");
  double const & xvalue_min_bckg = selectPointX(projY, "Select low edge of background");
  double const & xvalue_max_bckg = selectPointX(projY, "Select high edge of background");
  delete projY;
  return myProjectionX(histo, name, xvalue_min, xvalue_max, xvalue_min_bckg, xvalue_max_bckg);
}

TH1D* myProjectionY(TH2* histo, std::string const & name, double const & xvalue_min, double const & xvalue_max, double const & xvalue_min_bckg, double const & xvalue_max_bckg)
{
  TH1D* proj = histo->ProjectionY(name.c_str(), xvalue_min, xvalue_max);
  std::unique_ptr<TH1D> bckg (histo->ProjectionY((name+"_bckg").c_str(), xvalue_min_bckg, xvalue_max_bckg));
  proj->Add(bckg.get(), -1);
  return proj;
}

TH1D* myProjectionY(TH2* histo, std::string const & name, double const & x_value, double const & resolution)
{
  if (!gPad) histo->Draw("colz");
  auto projX = histo->ProjectionX(concatenate("projX_", histo->GetName()).c_str());
  projX->GetYaxis()->SetRangeUser(x_value-5*resolution, x_value+5*resolution);
  double const & xvalue_min = selectPointX(projX, "Select low edge of peak");
  double const & xvalue_max = selectPointX(projX, "Select high edge of peak");
  double const & xvalue_min_bckg = selectPointX(projX, "Select low edge of background");
  double const & xvalue_max_bckg = selectPointX(projX, "Select high edge of background");
  delete projX;
  return myProjectionY(histo, name, xvalue_min, xvalue_max, xvalue_min_bckg, xvalue_max_bckg);
}

TH1D* bestProjectionY(TH2F* histo, std::string name, int bin, int resolution)
{
  auto proTot = histo->ProjectionY();
  TH1D* pro(histo->ProjectionY(name.c_str(), bin-resolution, bin+resolution));
  pro->Add(proTot,-(pro->Integral()/proTot->Integral()));
  std::unique_ptr<TH1D> pro_bckg_low(histo->ProjectionY(("proj_bckg_low_"+std::to_string(bin)).c_str(), bin-3*resolution, bin-resolution));
  pro_bckg_low->Add(proTot,-(pro->Integral()/proTot->Integral()));
  std::unique_ptr<TH1D> pro_bckg_high(histo->ProjectionY(("proj_bckg_high"+std::to_string(bin)).c_str(), bin+resolution, bin+3*resolution));
  pro_bckg_high->Add(proTot,-(pro->Integral()/proTot->Integral()));

  pro->Add(pro_bckg_low.get(), -(pro->Integral()/pro_bckg_low->Integral()));
  pro->Add(pro_bckg_high.get(), -(pro->Integral()/pro_bckg_high->Integral()));

  return pro;
}

TH1D* bestProjectionX(TH2F* histo, std::string name, int bin, int resolution)
{
  auto proTot = histo->ProjectionX();
  TH1D* pro(histo->ProjectionX(name.c_str(), bin-resolution, bin+resolution));
  // pro->Add(proTot,-(pro->Integral()/proTot->Integral()));
  std::unique_ptr<TH1D> pro_bckg_low(histo->ProjectionX(("proj_bckg_low_"+std::to_string(bin)).c_str(), bin-3*resolution, bin-resolution));
  pro_bckg_low->Add(proTot,-(pro->Integral()/proTot->Integral()));
  std::unique_ptr<TH1D> pro_bckg_high(histo->ProjectionX(("proj_bckg_high"+std::to_string(bin)).c_str(), bin+resolution, bin+3*resolution));
  pro_bckg_high->Add(proTot,-(pro->Integral()/proTot->Integral()));

  pro->Add(pro_bckg_low.get(), -(pro->Integral()/pro_bckg_low->Integral()));
  pro->Add(pro_bckg_high.get(), -(pro->Integral()/pro_bckg_high->Integral()));

  return pro;
}

/**
 * @brief Projection on the xy plane between [binzmin; binzmax] (gates included); 
 *        Available types : XY, YX, XZ, ZX, YZ, ZY.
 */
TH2F* myProjection2D(TH3F* histo, std::string type = "XY", int binmin = 0, int binmax = -1, std::string name = "")
{
  if (histo == nullptr) {error("in myProjection2D : histo is nullptr"); return nullptr;}

  static Strings types = {"XY", "YX", "XZ", "ZX", "YZ", "ZY"};
  if (!found(types, type)) {error("in myProjection2D(type, histo) :", type, "is not known"); return nullptr;}
  // Preparing the return histo :
  if (name == "") name = histo->GetName()+std::string("_p")+type;

  TAxis* xaxis = nullptr;
  TAxis* yaxis = nullptr;
  TAxis* zaxis = nullptr;

  // Choose the axis accordingly to the requested output, so that the operation consist in projecting the z axis onto the XY plane
  if (type == types[0])
  {
    xaxis = histo->GetXaxis();
    yaxis = histo->GetYaxis();
    zaxis = histo->GetZaxis();
  }
  else if (type == types[1])
  {
    xaxis = histo->GetYaxis();
    yaxis = histo->GetXaxis();
    zaxis = histo->GetZaxis();
  }
  else if (type == types[2])
  {
    xaxis = histo->GetXaxis();
    yaxis = histo->GetZaxis();
    zaxis = histo->GetYaxis();
  }
  else if (type == types[3])
  {
    xaxis = histo->GetZaxis();
    yaxis = histo->GetXaxis();
    zaxis = histo->GetYaxis();
  }
  else if (type == types[4])
  {
    xaxis = histo->GetYaxis();
    yaxis = histo->GetZaxis();
    zaxis = histo->GetXaxis();
  }
  else if (type == types[5])
  {
    xaxis = histo->GetZaxis();
    yaxis = histo->GetYaxis();
    zaxis = histo->GetXaxis();
  }


  auto const & nbinsX = xaxis->GetNbins();
  auto const & minX = xaxis->GetBinLowEdge(0);
  auto const & maxX = xaxis->GetBinUpEdge(nbinsX);

  auto const & nbinsY = yaxis->GetNbins();
  auto const & minY = yaxis->GetBinLowEdge(0);
  auto const & maxY = yaxis->GetBinUpEdge(nbinsY);

  if (binmax==-1) binmax = zaxis->GetNbins();

  TH2F* ret = nullptr; 
  if (gFile->Get<TH2F>(name.c_str())) ret = gFile->Get<TH2F>(name.c_str());
  else 
  {
    ret = new TH2F(name.c_str(), name.c_str(), nbinsX+1,minX,maxX, nbinsY+1,minY,maxY);
    ret->GetXaxis()->SetTitle(xaxis->GetTitle());
    ret->GetYaxis()->SetTitle(yaxis->GetTitle());
  }


  // Perform the projection :
  for(int binx = 1; binx<=nbinsX; ++binx) for(int biny = 1; biny<=nbinsY; ++biny) 
  {
    int value = 0;
    for(int binz = binmin; binz<=binmax; ++binz) 
    {
           if (type == "XY") value += histo->GetBinContent(binx, biny, binz);
      else if (type == "YX") value += histo->GetBinContent(biny, binx, binz);
      else if (type == "XZ") value += histo->GetBinContent(binx, binz, biny);
      else if (type == "ZX") value += histo->GetBinContent(biny, binz, binx);
      else if (type == "YZ") value += histo->GetBinContent(binz, biny, binz);
      else if (type == "ZY") value += histo->GetBinContent(binz, binz, biny);
    }
    ret->SetBinContent(binx+1, biny+1, value);
  }

  return ret;
}


template<class... Args> TH2F* myProjectionXY(TH3F* histo, Args... args) {return myProjection2D(histo, "XY", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionYX(TH3F* histo, Args... args) {return myProjection2D(histo, "YX", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionXZ(TH3F* histo, Args... args) {return myProjection2D(histo, "XZ", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionZX(TH3F* histo, Args... args) {return myProjection2D(histo, "ZX", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionYZ(TH3F* histo, Args... args) {return myProjection2D(histo, "YZ", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionZY(TH3F* histo, Args... args) {return myProjection2D(histo, "ZY", std::forward<Args>(args)...);}


TH2F* myProjection2Db(TH3F* histo, std::string type = "XY", int binminGate = 0, int binmaxGate = -1, int binminBackground = 0, int binmaxBackground = -1, std::string name = "")
{
  auto gate = myProjection2D(histo, type, binminGate, binmaxGate, name);
  auto bckg = myProjection2D(histo, type, binminBackground, binmaxBackground, "temp_bckg");
  gate->Add(bckg, -1);
  delete bckg;
  return gate;
}

template<class... Args> TH2F* myProjectionXYb(TH3F* histo, Args... args) {return myProjection2Db(histo, "XY", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionYXb(TH3F* histo, Args... args) {return myProjection2Db(histo, "YX", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionXZb(TH3F* histo, Args... args) {return myProjection2Db(histo, "XZ", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionZXb(TH3F* histo, Args... args) {return myProjection2Db(histo, "ZX", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionYZb(TH3F* histo, Args... args) {return myProjection2Db(histo, "YZ", std::forward<Args>(args)...);}
template<class... Args> TH2F* myProjectionZYb(TH3F* histo, Args... args) {return myProjection2Db(histo, "ZY", std::forward<Args>(args)...);}


double ratio_integrals(TH1* histo1, TH1* histo2, int peak_min, int peak_max)
{
  return peak_integral(histo1, peak_min, peak_max)/peak_integral(histo2, peak_min, peak_max);
}
// double ratio_integrals(TH1D* histo1, TH1D* histo2, int peak_min, int peak_max)
// {
//   return peak_integral(histo1, peak_min, peak_max)/peak_integral(histo2, peak_min, peak_max);
// }

namespace CoLib
{
  TH3F* removeVeto(TH3F* histo, TH3F* histo_veto, double norm = 1, std::string name = "", bool nice = true)
  {
    if (name == "") name = histo->GetName()+std::string("_veto_clean");
    auto ret = static_cast<TH3F*>(histo->Clone(name.c_str()));
    print(ret->GetName());

    ret->Add(histo_veto, (nice) ? -norm : ((norm<1) ? 0 : int(-norm)));

    return ret;
  }

  // TODO : nice ne fonctionne pas
  TH2F* removeVeto(TH2F* histo, TH2F* histo_veto, double norm, std::string name = "", bool nice = true)
  {
    if (name == "") name = histo->GetName()+std::string("_veto_clean");
    auto ret = static_cast<TH2F*>(histo->Clone(name.c_str()));
    print(ret->GetName());

    if (nice)
    {
      for (int x = 0; x<histo->GetNbinsX(); ++x) for (int y = 0; y<histo->GetNbinsX(); ++y)
      {
        ret->SetBinContent(x, y, ret->GetBinContent(x, y) - int_cast(histo_veto->GetBinContent(x, y) * norm));
      }
    }
    else
    {
      ret->Add(histo_veto, -norm);
    }

    return ret;
  }

  TH2F* removeVeto(TH2F* histo, TH2F* histo_veto, int peak_norm_min, int peak_norm_max, std::string name = "", bool nice = true)
  {
    auto ret = removeVeto(histo, histo_veto, ratio_integrals(histo->ProjectionX("histo_projX"), histo_veto->ProjectionX("histo_veto_projX"), peak_norm_min, peak_norm_max), name, nice);
    delete gDirectory->Get("histo_projX");
    delete gDirectory->Get("histo_veto_projX");
    return ret;
  }

  TH1F* removeVeto(TH1* histo, TH1* histo_veto, double norm, std::string name = "", bool nice = true)
  {
    if (name == "") name = histo->GetName()+std::string("_veto_clean");
    auto ret = static_cast<TH1F*>(histo->Clone(name.c_str()));

    if (nice)
    {
      for (int x = 0; x<histo->GetNbinsX(); ++x)
      {
        ret->SetBinContent(x, ret->GetBinContent(x) - int_cast(histo_veto->GetBinContent(x) * norm));
      }
    }
    else
    {
      ret->Add(histo_veto, -norm);
    }

    ret->Add(histo_veto, -norm);
    
    return ret;
  }

  TH1F* removeVeto(TH1* histo, TH1* histo_veto, int peak_norm_min, int peak_norm_max, std::string name = "", bool nice = false)
  {
    return removeVeto(histo, histo_veto, ratio_integrals(histo, histo_veto, peak_norm_min, peak_norm_max), name, nice);
  }

  // template<class... Args>
  // TH1D* removeVeto(TH1D* histo, TH1D* histo_veto, Args... args)
  // {
  //   return removeVeto(std::make_unique<TH1F*>(dynamic_cast<TH1F*>(histo)), std::make_unique<TH1F*>(dynamic_cast<TH1F*>(histo_veto)), std::forward<Args>(args)...);
  // }

  void simulate_peak(TH1* histo, double const & x_center, double const & x_resolution, int const & nb_hits)
  {
    TRandom* random = new TRandom();
    for (int it = 0; it<nb_hits; ++it) histo->Fill(random->Gaus(x_center, x_resolution/2.35));
    delete random;
  }

  void simulate_peak(TH1* histo, double const & x_center, double const & x_resolution, int const & nb_hits, bool draw)
  {
    if (!histo) {error("in simulate_peak : histo is nullptr"); return;}
    if (draw) 
    {
      auto name = histo->GetName()+std::string("_simulated");
      TH1D* newHisto = nullptr;
      if (gFile) newHisto = gFile->Get<TH1D>(name.c_str());
      if (!newHisto) newHisto = pad_get_histo<TH1D>(name);
      if (!newHisto) {print("new histo"); newHisto = static_cast<TH1D*> (histo->Clone(name.c_str()));}
      simulate_peak(newHisto, x_center, x_resolution, nb_hits);
      newHisto->SetLineColor(kRed);
      newHisto->Draw();
      histo->Draw("same");
    }
    else simulate_peak(histo, x_center, x_resolution, nb_hits); 
  }

  /**
   * @brief histo
   * 
   * @return double 
   */
  double calculateHalfLife(TH1* decay_histo)
  {
    TF1* fit = new TF1("expo", "expo");
    decay_histo->Fit(fit, "Q");
    return -log(2)/fit->GetParameter(1);
  }

  /**
   * @brief Histo is E VS time with time in ps and E in keV
  */
  TGraph* calculateHalfLife(TH2F* histo, int min_bin, int max_bin, int sigma, std::vector<TH1D*> & projs)
  {
    std::vector<int> bins;
    std::vector<int> half_lifes;
    if (min_bin-5*sigma < 0 || max_bin+5*sigma>histo->GetXaxis()->GetXmax()) {error ("in calculateHalfLife : out of bounds !"); return nullptr;}
    auto proTot = histo->ProjectionX();
    projs.push_back(proTot);
    for (int bin = min_bin-4*sigma; bin < max_bin+4*sigma; ++bin)
    {
      auto pro = histo->ProjectionX(("proj_"+std::to_string(bin)).c_str(), bin-sigma, bin+sigma);
      pro->Add(proTot, -(pro->Integral()/proTot->Integral()));
      // TH1D* pro_bckg_low(histo->ProjectionX(("proj_bckg_low_"+std::to_string(bin)).c_str(), bin-3*sigma, bin-sigma));
      // TH1D* pro_bckg_high(histo->ProjectionX(("proj_bckg_high"+std::to_string(bin)).c_str(), bin+sigma, bin+3*sigma));
      // pro->Add(pro_bckg_low, -0.5);
      // pro->Add(pro_bckg_high, -0.5);
      pro->SetDirectory(nullptr);
      projs.push_back(pro);

      TF1* expo = new TF1("expo","expo");
      pro->Fit(expo);
      TF1* expo_pol = new TF1("expo","expo(0)+pol1(2)");
      expo_pol->SetParameters(expo->GetParameter(0), expo->GetParameter(1));
      pro->Fit(expo_pol);

      bins.push_back(bin);
      half_lifes.push_back(log(2)/(-1000*expo_pol->GetParameter(1)));
      pro->Draw();
      // pro->SetLineColor(kRed);
      // pro_bckg_low->Draw("same");
      // pro_bckg_high->Draw("same");
      gPad->WaitPrimitive();
    }
    TGraph* ret = new TGraph(bins.size(), bins.data(), half_lifes.data());
    return ret;
  }

  /**
   * @brief Calculate half-life for 3D matrices in the form XYZ = E_1E_2t
   * 
   * @param histo 
   * @param gate_bins 
   */
  void calculateHalfLife3D(TH3F* histo, std::vector<std::pair<int, int>> gates)
  {

    // auto xaxis = histo->GetXaxis();
    // auto yaxis = histo->GetYaxis();
    auto zaxis = histo->GetZaxis();

    // auto E1bins = xaxis->GetNbins();
    // auto E2bins = yaxis->GetNbins();
    auto time_bins = zaxis->GetNbins();

    std::vector<int> xvec; linspace(xvec, time_bins);

    for (auto const & gate : gates)
    {
      auto const & E1 = gate.first;
      auto const & E2 = gate.second;
      std::vector<int> counts;
      std::vector<int> bckg_counts;
      std::vector<int> net_counts;
      for (int t = 1; t<time_bins; ++t)
      {
        counts.push_back(histo->GetBinContent(E1, E2, t));
        int bckg_count = 0;
        for (int E1_bckg = E1-1; E1_bckg <= E1+1; ++E1_bckg) for (int E2_bckg = E2-1; E2_bckg <= E2+1; ++E2_bckg)
        {
          if (E1_bckg!=E1 && E2_bckg!=E2) bckg_count += int(histo->GetBinContent(E1_bckg, E2_bckg, t)/8);
        }
        bckg_counts.push_back(bckg_count);
        net_counts.push_back(histo->GetBinContent(E1, E2, t)-bckg_count);
      }
      auto graph = new TGraph(counts.size(), xvec.data(), counts.data());
      auto graph2 = new TGraph(counts.size(), xvec.data(), bckg_counts.data());
      auto graph3 = new TGraph(counts.size(), xvec.data(), net_counts.data());

      graph->SetLineColor(kBlue);
      graph2->SetLineColor(kRed);  
      graph3->SetLineColor(kGreen);  

      graph->Draw();
      graph2->Draw("same");
      graph3->Draw("same");
      gPad->WaitPrimitive();
    }
  }

  TGraph* meanXvsY(TH2* bidim)
  {
    std::vector<double> values;
    std::vector<double> means;
    for (int y = 1; y<=bidim->GetNbinsY(); ++y) 
    {
      values.push_back(bidim->GetYaxis()->GetBinCenter(y));
      double mean = 0;
      double total = 0;
      for (int x = 1; x<=bidim->GetNbinsX(); ++x)
      {
        auto const & binvalue = bidim->GetBinContent(x, y);
        auto const & xvalue = bidim->GetXaxis()->GetBinCenter(x);
        total+=binvalue;
        mean+=xvalue*binvalue;
      }
      if (total==0) means.push_back(0);
      else means.push_back(mean/total);
    }
    auto ret = new TGraph(values.size(), values.data(), means.data());
    ret->GetXaxis()->SetTitle(bidim->GetYaxis()->GetTitle()); // TODO
    return ret;
  }


  template<class THist>
  THist* AddNorm(THist* h1, THist* h2, double min_range, double max_range)
  {
    auto ret = (THist*) h1->Clone(TString(h1->GetName())+"_plus_norm_"+TString(h2->GetName()));
    ret->SetTitle(TString(h1->GetName())+"+"+TString(h2->GetName()));

    auto canvas = new TCanvas("temp_canvas", "temp_canvas");
    h1->Draw();
    h2->Draw("same");
    h1->GetXaxis()->SetRangeUser(min_range, max_range);

    double factor = h1->GetMaximum()/h2->GetMaximum();
    ret->Add(h2, +factor);

    h1->GetXaxis()->UnZoom();

    delete canvas;
    return ret;
  }
  template<class THist>
  THist* SubNorm(THist* h1, THist* h2, double min_range, double max_range)
  {
    auto ret = (THist*) h1->Clone(TString(h1->GetName())+"_minus_norm_"+TString(h2->GetName()));
    ret->SetTitle(TString(h1->GetName())+"-"+TString(h2->GetName()));

    auto canvas = new TCanvas("temp_canvas", "temp_canvas");
    h1->Draw();
    h2->Draw("same");
    h1->GetXaxis()->SetRangeUser(min_range, max_range);

    double factor = h1->GetMaximum()/h2->GetMaximum();
    ret->Add(h2, -factor);

    h1->GetXaxis()->UnZoom();

    delete canvas;
    return ret;
  }

  template<class THist>
  THist* get(std::string name, TFile* file = nullptr)
  {
    if (!file)
    {
      file = (TFile*)gFile;
      if (!file) {error("no file"); return nullptr;}
    }
    if (auto ret = dynamic_cast<THist*>(file->Get(name.c_str()))) return ret;
    else return nullptr;
  }

  /// @brief For low_count spectra : calculate the mean count/keV in a given region and returns the variance
  double calculateVariance(TH1F* hist, int const & bin_min, int const & bin_max) 
  {
    auto const & bins = bin_max-bin_min;
    double sum = 0;
    double sumSq = 0;

    for (int i = bin_min+1; i <= bin_max; ++i) 
    {
      auto const & binContent = hist->GetBinContent(i);
      sum += binContent;
      sumSq += binContent * binContent;
    }

    auto const & mean = sum / bins;

    return (sumSq / bins) - (mean * mean);
  }

  /// @brief For low_count spectra : calculate the mean count/keV in a given region and returns the variance
  double calculateVariance(TH1F* hist)
  {
    return calculateVariance(hist, 0, hist->GetNbinsX());
  } 

  auto adaptative_mean(TH1F* hist, size_t length)
  {
    auto const & bins = hist->GetNbinsX();
    TH1F *out = new TH1F("out", "out", bins, hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
    int sum = 0;
    for(size_t bin = 1; bin<2*length+1; ++bin) sum += hist->GetBinContent(bin);
    for (size_t bin = length+1; bin<=bins-length; ++bin)
    {
      out->SetBinContent(bin, sum/(2*length+1));
      sum-=hist->GetBinContent(bin-length);
      sum+=hist->GetBinContent(bin+length+1);
    }
    out->SetLineColor(kRed);
    return out;
  }

  /// @brief 
  /// @param hist 
  /// @param length 
  /// @param bin_min must be at least length bins from the edge
  /// @param bin_max must be at least length bins from the edge
  auto adaptative_mean(TH1F* hist, size_t length, size_t bin_min, size_t bin_max)
  {
    auto const & bins = hist->GetNbinsX();
    
    if (bin_min<length) throw_error("CoLib::adaptative_mean : bin_min must be at least length bins from the edge.");
    if ((bins-bin_max)<length) throw_error("CoLib::adaptative_mean : bin_max must be at least length bins from the edge.");

    TH1F* out = new TH1F("out", "out", bins, hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
    int sum = 0;
    for(size_t bin = bin_min-length+1; bin<bin_min+length+1; ++bin) sum += hist->GetBinContent(bin);
    for (size_t bin = bin_min+1; bin<=bin_max; ++bin)
    {
      out->SetBinContent(bin, sum/(2*length));
      sum-=hist->GetBinContent(bin-length);
      sum+=hist->GetBinContent(bin+length);
    }
    out->SetLineColor(kRed);
    return out;
  }

  // TODO ?
  void plot_variance(TH1F* hist, int const & bin_min, int const & bin_max)
  {
    auto means = adaptative_mean(hist, 5, bin_min, bin_max);
    std::vector<double> diffs;
    for (int bin = bin_min+1; bin <= bin_max; ++bin) diffs.push_back(hist->GetBinContent(bin) - means->GetBinContent(bin));
    auto const & min = minimum(diffs);
    auto const & max = maximum(diffs);
    auto temp = new TH1F("temp","temp",int(max-min), min, max);
    for (auto const & diff : diffs) temp->Fill(diff);
    temp->Draw();
  }

}

// TH2F* removeVeto(TH2F* histo, TH1F* veto_projx, TH1F* veto_projy, int bin_min, int bin_max)
// {
//   auto ret = static_cast<TH2F*>(histo->Clone(TString(histo->GetName())+"_veto_clean"));
//   auto projX = histo->ProjectionX();
//   auto projY = histo->ProjectionY();
//   // auto const & normx = veto_projx->Integral(bin_min, bin_max)/(projX->Integral(bin_min, bin_max));
//   // auto const & normy = veto_projy->Integral(bin_min, bin_max)/(projY->Integral(bin_min, bin_max));

//   print("TODO");

//   for (int x = 1; x<=histo->GetNbinsX(); ++x) for (int y = 1; y<=histo->GetNbinsX(); ++y) if (histo->GetBinContent(x, y) > 0)
//   {
//     print(histo->GetBinContent(x, y));
//     // TODO !!
//     // addBinContent(histo, x, y, -int((projX->GetBinContent(x, y)+projY->GetBinContent(x, y))/2*normx));
//   }

//   return ret;
// }



 /// @brief Allows one to fit a peak of a histogram in the range [low_edge, high_edge]
 /// @attention The edges must be well centered, this is not a peak finder.
class PeakFitter
{
public:
  PeakFitter() = default;
  ~PeakFitter()
  {
  }

  PeakFitter(TH1* histo, double low_edge, double high_edge) : 
  m_histo (histo),
  m_low_edge  (low_edge),
  m_high_edge (high_edge)
  {
    this -> fit();
  }

  /// @brief Allows one to fit a peak of a histogram in the range [low_edge, high_edge]
  void fit() {this -> fit(m_histo, m_low_edge, m_high_edge);}

  /// @brief Allows one to fit a peak of a histogram in the range [low_edge, high_edge]
  void fit(TH1* histo, double low_edge, double high_edge, double mean = -1, double sigma = -1, double constante = -1)
  {
  #ifdef __CINT__
    histo->GetXaxis()->SetRangeUser(low_edge, high_edge);
    histo->Draw();
    gPad->Update();
    gPad->WaitPrimitive();
  #endif //__CINT__
    if (mean == -1) mean = (high_edge+low_edge)/2;
    if (constante == -1) constante = histo->GetMaximum();
    if (sigma == -1) sigma = (high_edge-low_edge)/5.;


    TF1* gaus0(new TF1("gaus0","gaus"));
    gaus0 -> SetRange(low_edge, high_edge);
    gaus0 -> SetParameters( constante, mean, sigma);
    histo -> Fit(gaus0,"RQN+");

    sigma= gaus0->GetParameter(2);
    mean = gaus0->GetParameter(1);
    // The mean can be shifted away from the actual peak position because of the background
    // Therefore, I get the mean position from the mean of the fitted peak and the weighted
    // the X value of the bin with maximum content and the two bins around it :
    double mean_max_bins = 0.; double content_max_bins = 0;
    auto max_bin = histo->GetMaximumBin();
    if (max_bin<1) max_bin = 1;
    for (int i = -1; i<2; i++) 
    {
      mean_max_bins+=histo->GetBinCenter(max_bin+i) * histo->GetBinContent(max_bin+i);
      content_max_bins+=histo->GetBinContent(max_bin+i);
    }

    mean = (mean+mean_max_bins/content_max_bins)/2;


    if (m_order_background<2) {final_fit = gaus0; return;}

    TF1* gaus1(new TF1("gaus1","gaus(0)+pol1(3)"));
    gaus1 -> SetRange(mean-5*sigma, mean+5*sigma);
    gaus1 -> SetParameters(gaus0->GetParameter(0), gaus0->GetParameter(1), gaus0->GetParameter(2));
    histo -> Fit(gaus1,"RQN+");

    if (m_order_background<3) {final_fit = gaus1; return;}

    sigma= gaus1->GetParameter(2);
    mean = gaus1->GetParameter(1);
    mean = (mean+histo->GetBinCenter(histo->GetMaximumBin()))/2;

    TF1* gaus2(new TF1("gaus2","gaus(0)+pol2(3)"));
    if (m_low_binning) gaus2 -> SetRange(mean-4*sigma, mean+4*sigma);
    else               gaus2 -> SetRange(mean-2*sigma, mean+2*sigma);
    gaus2 -> SetParameters(gaus1->GetParameter(0), gaus1->GetParameter(1), gaus1->GetParameter(2));
    histo -> Fit(gaus2,"RQN+");

    final_fit = gaus2;
    final_fit->Draw("same");
  }

  auto operator->(){return final_fit;}
  auto const & getFit() const {return final_fit;}
  auto getConstante() const {return final_fit->GetParameter(0);}
  auto getMean() const {return final_fit->GetParameter(1);}
  auto getSigma() const {return final_fit->GetParameter(2);}
  auto getBackground() const 
  {
    auto background (new TF1("background", "pol2"));
    background->SetParameters(final_fit->GetParameter(3), final_fit->GetParameter(4), final_fit->GetParameter(5));
    return background;
  }

  auto setBackgroundOrder(int const & order_background) {m_order_background = order_background;}

protected:
  TH1* m_histo;
  double m_low_edge = 0.0;
  double m_high_edge = 0.0;
  double m_order_background = 3;

  bool m_low_binning = true;

  TF1* final_fit = nullptr;
};


/**
 * @brief Allows one to find the most significant peak in the range [low_edge, high_edge]
 * @details 
 * Requires little background so you may need to make a background substraction first
 * requires the number of counts of the most significant bin to be at least 1.2x higher than in the other peaks
 */
class BiggestPeakFitter : public PeakFitter
{
public:
  /// @brief Allows one to find the most significant peak in the range [low_edge, high_edge]
  BiggestPeakFitter(TH1* histo, double low_edge = -1, double high_edge = -1, int const & order_background = 3)
  {
    PeakFitter::setBackgroundOrder(order_background);
    
    auto const & initialRangeMin = histo->GetXaxis()->GetXmin();
    auto const & initialRangeMax = histo->GetXaxis()->GetXmax();

    if (low_edge  == -1) low_edge  = initialRangeMin;
    if (high_edge == -1) high_edge = initialRangeMax;

    histo->GetXaxis()->SetRangeUser(low_edge, high_edge);

    // Dumb sigma of the maximum peak :
    double max = histo->GetMaximum();
    double maxbin = histo->GetMaximumBin();

    // First, trying to find the edges starting at the center :
    int begin_peak_bin = histo->FindFirstBinAbove(max*0.8);
    int end_peak_bin   = histo->FindLastBinAbove(max*0.8);
    if (end_peak_bin-begin_peak_bin<3) {--begin_peak_bin; ++end_peak_bin;}
    auto sigma_bin = end_peak_bin-begin_peak_bin;

    print(histo->GetBinCenter(end_peak_bin), histo->GetBinCenter(begin_peak_bin));

    auto const & sigma = histo->GetBinCenter(end_peak_bin) - histo->GetBinCenter(begin_peak_bin);
    auto const & mean  = histo->GetBinCenter(maxbin);

    if (sigma_bin < 4) PeakFitter::m_low_binning = true;
    PeakFitter::fit(histo, mean-5*sigma, mean+5*sigma, mean, sigma);

    histo->GetXaxis()->SetRangeUser(initialRangeMin, initialRangeMax);

  #ifdef __CINT__
    histo->Draw();
    this->Draw("same");
  #endif //__CINT__
  }
};


class Efficiency
{
public:
  Efficiency(std::string const & filename, std::string options = "ascii")
  {
    if (options == "ascii")
    {
      std::ifstream file(filename, std::ios::in);
      int energy = 0; double value = 0;
      while (file >> energy >> value) m_data.push_back(value/100);
      m_max = maximum(m_data);
    }
  }
  auto const & operator[](double const & energy) const {return m_data[int_cast(energy)];}
  auto const & operator()(double const & energy) const {return m_data[int_cast(energy)];}
  auto operator() (double const & energy1, double const & energy2) const {return m_data[int_cast(energy1)]*m_data[int_cast(energy2)];}
  auto operator() (double const & energy1, double const & energy2, double const & energy3) const {return m_data[int_cast(energy1)]*m_data[int_cast(energy2)]*m_data[int_cast(energy3)];}
  auto normalizedValue(double const & energy) const {return m_data[int_cast(energy)]/m_max;}
  auto normalizedCounts(int const & counts, double const & energy) const {return counts*normalizedValue(energy);}
  auto draw()
  { 
    // int argc = 0;
    // char *argv[] = {};
    // TApplication app("app", &argc, argv);

    // Create a canvas
    TCanvas *canvas = new TCanvas("canvas", "Interactive Canvas", 800, 600);

    // Generate x values using linspace
    std::vector<double> x;
    linspace(x, m_data.size());

    // Create the graph
    TGraph *graph = new TGraph(m_data.size(), x.data(), m_data.data());
    graph->SetName("Efficiency");
    graph->SetTitle("Efficiency");
    graph->Draw("AL");

    // Update the canvas to draw the graph
    canvas->Update();

    // // Run the application in interactive mode
    // app.Run();
  }

private:
  std::vector<double> m_data;
  double m_max = 0.0;
  TF1* function = nullptr;
};

TH1D* apply_efficiency(TH1* histo, Efficiency const & eff)
{
  auto const & xaxis = histo->GetXaxis();
  auto const & bins = xaxis->GetNbins();
  auto const & bin_min = xaxis->GetXmin();
  auto const & bin_max = xaxis->GetXmax();
  std::string name = histo->GetName() + std::string("_efficiency");
  std::string title = histo->GetTitle() + std::string("_efficiency");

  TH1D* ret = new TH1D(name.c_str(), title.c_str(), bins, bin_min, bin_max);

  for (int bin = 0; bin<bins; ++bin)
  {
    auto const & nrj = histo->GetBinCenter(bin);
    ret->SetBinContent(bin, histo->GetBinContent(bin)/eff[nrj]);
  }
  return ret;
}

// #include <readline/readline.h>
// #include <readline/history.h>

// #ifdef __CINT__
class Radware
{public:
  Radware(TH2* _bidim, int autoRemoveBackground = 0)
  {
    file = gFile;
    gROOT->cd();
    m_bidim = (TH2F*) _bidim->Clone(TString(_bidim->GetName())+"_radware");
    if (autoRemoveBackground>0) CoAnalyse::removeBackground(m_bidim, m_nb_it_bckg, autoRemoveBackground);
    init();
    proj();
    launch();
  }
  
  void init()
  {
    canvas->ToggleEventStatus();
    canvas->ToggleEditor();
    canvas->ToggleToolBar();
    canvas->cd();
    m_finish = false;
  }

  static bool m_finish;

  void launch()
  {
    std::thread root_thread([]()
    {
      while(!m_finish) 
      {
        // gPad->WaitPrimitive();
        gSystem->ProcessEvents();
        gSystem->Sleep(10);
      }
    });
    
    print(m_finish);
    std::string instruction;
    while(!m_finish)
    {
      std::cout << "> ";

      // Ctrl+D events :
      auto const & getline_ret = std::getline(std::cin, instruction);
      if (!getline_ret) 
      {
        error("CTRL-D pressed, interactive interface will not work next time. Use st command instead");
        m_finish = true;
      }
      else if (instruction == "") continue;
      else if (instruction == "todo") this->todo();
      else if (instruction == "ag") this->addGate();
      else if (instruction == "bd") {print("May take a while..."); this->draw(m_bidim, "colz");}
      else if (instruction == "bi") this->set_nb_it_bckg();
      else if (instruction == "cl") this->clean();
      // else if (instruction == "di") this->projectDiag();
      else if (instruction == "ex") this->ex();
      else if (instruction == "es") this->exportSpectrum();
      else if (instruction == "gs") this->setGateSize();
      else if (instruction == "h" ) this->printHelp();
      else if (instruction == "in") this->integral();
      else if (instruction == "ns") this->normalizeSpectra();
      else if (instruction == "pr") this->proj();
      else if (instruction == "px") this->px();
      else if (instruction == "py") this->py();
      else if (instruction == "rb") this->removeBackground();
      else if (instruction == "rs") pad_remove_stats();
      else if (instruction == "sg") this->addGate(false);
      else if (instruction == "sh") pad_subtract_histos();
      else if (instruction == "sp") this->simulatePeak();
      else if (instruction == "sm") this->gate_same();
      else if (instruction == "st") {m_finish = true;}
      else if (instruction == "uz") this->unZoom();
      else if (instruction == "+") this->gate(m_gate_number+1);
      else if (instruction == "-") this->gate(m_gate_number-1);
      else if (isNumber(instruction)) this->gate(std::stod(instruction));
      else error("Wrong input...");
    }
    print("Exiting CoRadware");
    root_thread.join();
    free_resources();
  }

  void todo()
  {
    print("Faire un mode asymetrique où une gate génère les projections sur les deux axes côte à côte");
  }

  void printHelp()
  {
    print("List of commands :");
    print("ag : add gate");
    print("bd : display bidim");
    print("bi : set number of iterations for automatic background subtraction (rb) of one dimensional spectra");
    print("cl : clean");
    // print("di : project bidim diagonal");
    print("ex : set range spectrum");
    print("es : export spectra to the root environement");
    print("gs : set gate size in bin");
    print("h  : display this help");
    print("in : peak integral");
    print("ns : normalize spectra");
    print("pr : display total projection");
    print("px : projection onto the x axis");
    print("py : projection onto the y axis (default)");
    print("rb : remove background automatically");
    print("rs : remove stat box");
    print("sg : subtract gate");
    print("sh : subtract the two displayed histograms");
    print("sp : simulate peak");
    print("sm : overlay another gate");
    print("st : finish session");
    print("uz : unzoom axis");
  }

  void draw(TH1* histo, std::string options = "")
  {
    m_nb_sm = 0;
    canvas->cd();
    if (m_focus) histo->GetXaxis()->SetRangeUser(
      m_focus->GetXaxis()->GetBinLowEdge(m_focus->GetXaxis()->GetFirst()), 
      m_focus->GetXaxis()->GetBinUpEdge(m_focus->GetXaxis()->GetLast()));
    m_focus = histo;
    if (m_focus->InheritsFrom(TH2::Class())) options+="colz"; // For 2D
    else options+="hist"; // For 1D
    m_focus->Draw(options.c_str());
    gPad->Update();
  }

  void clean()
  {
    if (m_focus != m_gate) {print("Cleaning is for gated spectrum only..."); return;}
    int min_E, max_E;
    std::string instruction;

    std::cout << "normalisation peak min : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    min_E = std::stoi(instruction);

    std::cout << "normalisation peak max : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    max_E = std::stoi(instruction);

    auto clean_gate = CoLib::removeVeto(m_gate, m_proj, min_E, max_E);
    delete m_gate;
    m_gate = clean_gate;
    // m_gate->Draw();
    // gPad->Update();
    this->draw(m_gate);
  }

  void simulatePeak()
  {
    int peak, resolution, nb;
    std::string instruction;

    std::cout << "peak mean : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    peak = std::stoi(instruction);

    std::cout << "resolution : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    resolution = std::stoi(instruction);

    std::cout << "nb : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    nb = std::stoi(instruction);

    CoLib::simulate_peak(m_focus, peak, resolution, nb, true);
    gPad->Update();
  }

  void normalizeSpectra()
  {
    pad_normalize_histos();
  }

  void exportSpectrum()
  {
    std::cout << "Name to set (return for " << m_focus->GetName() << ")";
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = m_focus->GetName();
    auto test = static_cast<TH1F*> (m_focus->Clone(name.c_str()));
    print(TString("exporting")+test->GetName());
  }

  void setGateSize() 
  {
    printC("Choose gate size (current ",m_gate_size,")");
    std::cin >> m_gate_size;
  }

  void setHist(TH2F* _bidim) 
  {
    m_bidim = _bidim;
    this->proj();
  }
  
  void proj()
  {
    if (m_py) 
    {
      m_proj = static_cast<TH1*> (m_bidim->ProjectionY()->Clone("total projection y"));
      m_proj->SetTitle("total projection y");
    }
    else 
    {
      m_proj = static_cast<TH1*> (m_bidim->ProjectionX()->Clone("total projection x"));
      m_proj->SetTitle("total projection x");
    }
    this->draw(m_proj);
  }

  void px()
  {
    m_py = false;
  }

  void py()
  {
    m_py = true;
  }

  void addGate(bool add = true)
  {
    if (!m_gate) {error("No gate so far..."); return;}

    std::cout << "gate to " << ((add) ? "add" : "subtract") << " :";
    std::string instruction;
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    int bin = std::stoi(instruction);

    std::cout << "weight (rtn for 1.) :";
    std::getline(std::cin, instruction);
    double weight = 1.;
    if (!(instruction.empty()))
    {
      if (!checkIsNumber(instruction)) return;
      weight = std::stod(instruction);
    }

    if (!add) weight*=-1.;

    if (m_py) for (int y = 0; y<m_bidim->GetNbinsY(); ++y) for (int x = bin-m_gate_size; x<=bin+m_gate_size; ++x)
    {
      m_gate->AddBinContent(y, m_bidim->GetBinContent(x, y)*weight);
    }
    else for (int x = 0; x<m_bidim->GetNbinsY(); ++x) for (int y = bin-m_gate_size; y<=bin+m_gate_size; ++y)
    {
      m_gate->AddBinContent(x, m_bidim->GetBinContent(x, y)*weight);
    }
    this->draw(m_gate);
  }

  void ex()
  {
    auto const & low = selectPointX(m_focus, "Low edge");
    auto const & high = selectPointX(m_focus, "High edge");
    m_focus->GetXaxis()->SetRangeUser(low, high);
    this->draw(m_focus);
    gPad->Update();
  }

  void gate_same()
  {
    std::string val_str;
    print("Choose energy to draw on top");
    std::cin >> val_str;
    TH1* gate = nullptr;
    std::string name;
    if (val_str == "pr") 
    {
      name = "m_proj2";
      gate = ((TH1*)m_proj->Clone(name.c_str()));
      gate->SetTitle("total projection");
    }
    else 
    {
      if (!checkIsNumber(val_str)) return;
      auto const & e = std::stoi(val_str);
      auto const & bin = m_bidim->GetXaxis()->FindBin(e)-1;
      auto const & low_e = e-m_gate_size;
      auto const & high_e = e+m_gate_size;
      name = std::to_string(bin)+" g same";
      gate = static_cast<TH1*>((m_py) 
        ? m_bidim->ProjectionY("g", low_e, high_e)->Clone(name.c_str())
        : m_bidim->ProjectionX("g", low_e, high_e)->Clone(name.c_str()));
      gate->SetTitle((std::to_string(bin)+" gate").c_str());
    }
    gate->Draw("same");
    gate->SetLineColor(ROOT_nice_colors[(++m_nb_sm & 111)]);
    m_list_histo_to_delete.push_back(name);
    gPad->Update();
  }

  void gate(double e)
  {
    delete m_gate;
    m_gate_number = e;
    auto const & bin = m_bidim->GetXaxis()->FindBin(e)-1;
    auto const & low_e = e-m_gate_size;
    auto const & high_e = e+m_gate_size;
    m_gate = static_cast<TH1*> ((m_py)
      ? m_bidim->ProjectionY("g", low_e, high_e)->Clone((std::to_string(bin)+" g").c_str())
      : m_bidim->ProjectionX("g", low_e, high_e)->Clone((std::to_string(bin)+" g").c_str()));
    m_gate->SetTitle((std::to_string(bin)+" gate").c_str());
    this->draw(m_gate);
    print(peak_integral(m_focus, low_e, high_e, m_nb_it_bckg));
  }

  void integral()
  {
    double start, stop;
    print("Choose borne inf");
    std::cin >> start;
    print("Choose borne sup");
    std::cin >> stop;
    print(peak_integral(m_proj, start, stop, m_nb_it_bckg));
  }

  void removeBackground()
  {
    if (m_focus == m_bidim)
    {
      print("May take a while..."); 
      CoAnalyse::removeBackground(m_bidim, m_nb_it_bckg);
      this->proj();
    }
    else 
    {
      CoAnalyse::removeBackground(m_focus, m_nb_it_bckg, "", true);
      this->draw(m_focus);
    }
  }

  ~Radware()
  {
    free_resources();
  }

  void unZoom()
  {
    print("unzoom");
    m_focus->GetXaxis()->UnZoom();
    if (m_focus->InheritsFrom(TH2::Class())) m_focus->GetYaxis()->UnZoom();
    draw(m_focus);
  }

  void set_nb_it_bckg()
  {
    print("Choose iterations number for automatic background research");
    std::cin >> m_nb_it_bckg;
  }

private:

  void free_resources()
  {
    delete m_bidim;
    delete m_proj;
    delete m_gate;
    // delete m_focus;
    for (auto const & name : m_list_histo_to_delete) delete gDirectory->Get(name.c_str());
    canvas->Close();
    if (file) file->cd();
  }

  bool checkIsNumber(std::string instruction, std::string message = "")
  {
    auto const & ret = isNumber(instruction);
    if (!ret) print((message == "") ? message : "error input : must be a number");
    return ret;
  }

  TFile* file = nullptr;

  TCanvas *canvas = new TCanvas("RadwareCanvas", "RadwareCanvas");
  int m_nb_it_bckg = 20;
  TH2F* m_bidim = nullptr;
  TH1* m_proj = nullptr;
  TH1* m_gate = nullptr;
  TH1* m_focus = nullptr;
  Strings m_list_histo_to_delete;

  bool m_py = true;
  int m_gate_size = 1;
  double m_gate_number = 0;
  int m_nb_sm = 0;
};
bool Radware::m_finish = false;



/**
 * @brief Simple background subtraction for bidim with Ge spectra on one axis and a background-free quantity on the x (like multiplicity, particle energy...)
 * @details
 * The broad spectra is on the x axis, the 
 */
TH2F* removeBackgroundBroadVSGe(TH2F* histo, std::string new_name = "", int nb_iteration_bckg = 20 )
{
  TH2F* ret = (TH2F*)histo->Clone(new_name.c_str());
  for (int i = 0; i<1; ++i)
  {
    std::unique_ptr<TH1D> ProjX(ret->ProjectionX("ProjX"));
    std::unique_ptr<TH1D> ProjY(ret->ProjectionY("ProjY"));
    if (new_name == "") new_name = ret->GetName() + std::string("_tryRemoveBackground");
    auto const & integral = double_cast(ret->Integral());

    // Get the bacground for the Ge spectra :
    std::unique_ptr<TH1D> bckgY ((TH1D*)ProjY->ShowBackground(nb_iteration_bckg));
    std::unique_ptr<TH1D> projY((TH1D*)ProjY->Clone("projY"));
    projY->Add(projY.get(), -1);

    for (int x = 0; x<ProjX->GetNbinsX(); ++x) for (int y = 0; y<ProjY->GetNbinsX(); ++y)
    {
      auto const & old_value = ret->GetBinContent(x,y);
      auto const & Px = double_cast(ProjX->GetBinContent(x));
      auto const & px = 0.9*Px;
      auto const & bx = 0.1*Px;
      // auto const & Py = double_cast(ProjY->GetBinContent(y));
      auto const & py = double_cast(projY->GetBinContent(y));
      auto const & by = double_cast(bckgY->GetBinContent(y));
      auto const & new_value = old_value - int_cast(1*(px*by + py*bx + bx*by)/integral);
      ret->SetBinContent(x, y, new_value);
    }
  }
  return ret;
}

// Needs to be a simmetrized histogram BEFORE background subtraction
auto removeLine(TH2F* histo, int bin_min, int max_bin, int nb_it = 20)
{
  auto ret = (TH2F*) histo->Clone(TString(histo->GetName())+"_clean");
  for (int x = 1; x<=histo->GetXaxis()->GetNbins(); ++x)
  {
    auto proj(histo->ProjectionY("temp", x, x));
    auto bckg(proj->ShowBackground(nb_it));
    for (int y = bin_min; y<=max_bin; ++y) 
    {
      ret->SetBinContent(y, x, bckg->GetBinContent(y));
      ret->SetBinContent(x, y, bckg->GetBinContent(y));
    }
    delete proj;
    delete bckg;
  }
  return ret;
}

// Needs to be a simmetrized histogram BEFORE background subtraction
auto removeLines(TH2F* histo, std::vector<std::pair<int, int>> bins, int nb_it = 20)
{
  auto ret = (TH2F*) histo->Clone(TString(histo->GetName())+"_clean");
  for (int x = 1; x<=histo->GetXaxis()->GetNbins(); ++x)
  {
    auto proj(histo->ProjectionY("temp", x, x));
    auto bckg(proj->ShowBackground(nb_it));
    for (auto const & range : bins) for (int y = range.first; y<=range.second; ++y) 
    {
      ret->SetBinContent(y, x, bckg->GetBinContent(y));
      ret->SetBinContent(x, y, bckg->GetBinContent(y));
    }
    delete proj;
    delete bckg;
  }
  return ret;
}

TH2F* symetrise(TH2F* histo)
{
  TH2F* ret = new TH2F (TString(histo->GetName())+"_inv", histo->GetTitle(), 
                        histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax(), 
                        histo->GetNbinsY(), histo->GetYaxis()->GetXmin(), histo->GetYaxis()->GetXmax());

  for (int x = 1; x <= histo->GetNbinsX(); ++x) for (int y = 1; y <= histo->GetNbinsY(); ++y)
  {
    ret->SetBinContent(x, y, histo->GetBinContent(y, x) + histo->GetBinContent(y, x));
  }
  return ret;
}

TH2F* invert(TH2F* histo)
{
  TH2F* ret = new TH2F (TString(histo->GetName())+"_inv", TString(histo->GetTitle())+";"+TString(histo->GetYaxis()->GetTitle())+";"+TString(histo->GetXaxis()->GetTitle()), 
                        histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax(), 
                        histo->GetNbinsY(), histo->GetYaxis()->GetXmin(), histo->GetYaxis()->GetXmax());

  for (int x = 1; x <= histo->GetNbinsX(); ++x) for (int y = 1; y <= histo->GetNbinsY(); ++y)
  {
    ret->SetBinContent(x, y, histo->GetBinContent(y, x));
  }
  return ret;
}

Strings wildcard(std::string const & name)
{
  TString result = gSystem->GetFromPipe(("ls "+name).c_str());
  return getList(result.Data(), "\n");
}

/**
 * @brief Merges files with a maximum output size
 * 
 * @param target the target name with extension (e.g. output)
 */
void hadd(std::string source, std::string target, double size_file_Mo, std::string options = "", int nb_threads = 1)
{
  if (found(options, "-j")) 
  {
    error("in CoLib hadd, please use the parameter nb_threads instead of -j option of hadd");
    return;
  }

  // Get the list of files that matches the source wildcard
  TString pattern = source.c_str();
  TString command = "ls " + pattern;
  TString result = gSystem->GetFromPipe(command.Data());
  TObjArray* files = result.Tokenize("\n");
  auto const & nb_files = files->GetEntries();
  print(nb_files);
  if (!files) {error("No file matching", source); return;}

  // Get the size of each file and the total amount of files
  double total_size = 0;
  std::vector<double> size_files;
  Strings name_files;
  for (int i = 0; i < nb_files; ++i) 
  {
    TObjString* str = (TObjString*)files->At(i);
    auto const & name = str->GetString().Data();
    auto const & size = size_file(name, "Mo");
    if (size > size_file_Mo) {error("Input size > output size case not handled:", size, ">", size_file_Mo, "for", name); return;}
    name_files.push_back(name);
    size_files.push_back(size);
    total_size+=size;
  }

  // Estimate the number of output files. If lower than the number of threads, adjust the latter
  auto const & estimated_nb_files_out = int_cast(total_size/size_file_Mo)+1;
  if (nb_threads > estimated_nb_files_out) 
  {
    nb_threads = estimated_nb_files_out;
    print("threads number reduced to", nb_threads);
  }

  if (nb_threads == 1) print("Running without multithreading ...");
  else print("threads number", nb_threads);
  
  // Run hadd :
  std::mutex mutex;
  int infile_i = 0; 
  int outfile_i = 0; 
  bool end = false;

  std::vector<std::thread> threads;
  for (int thread_i = 0; thread_i<nb_threads; ++thread_i) threads.emplace_back([&](){

    mutex.lock();
    while(!end)
    {
      // Take the needed number of files :
      Strings files;
      double thread_size = 0;
      for (; infile_i<nb_files; ++infile_i)
      {
        if (thread_size+size_files[infile_i]>size_file_Mo) break;
        files.push_back(name_files[infile_i]);
        thread_size+=size_files[infile_i];
      }
      if (nb_files == infile_i) end = true;
      ++outfile_i;

    mutex.unlock();

      // Configure output file
      std::string output_name = target+"_"+std::to_string(outfile_i)+".root";
      std::string command = "hadd " + options + " " + output_name + " " + strings(files);
      // print(command);
      system(command.c_str());
    }
    mutex.unlock();
  });
  for (auto & thread : threads) thread.join();
}

void libRoot()
{
  print("Welcome, may you find some usefull stuff around");
}

#endif //LIBROOT_HPP


/*
 // unique_TFile file(TFile::Open(filename.c_str(), "READ"));
    // file -> cd();
    // if (!file.get()->IsOpen()) throw_error("Can't open"+filename);
    // print("Reading", filename);
    
    // TIter nextKey(file->GetListOfKeys());
    // TKey* key = nullptr;

    // int histo_nb = 0;
    // while (histo_nb<10 && (key = dynamic_cast<TKey*>(nextKey()))) 
    // {
    //   TObject* obj = key->ReadObj();
    //   if (obj->IsA()->InheritsFrom(TH1::Class())) 
    //   {
    //     if (obj->IsA()->InheritsFrom(TH1F::Class())) 
    //     {
    //       auto histo = dynamic_cast<TH1F*>(obj);
    //       std::string name = histo->GetName();
    //       print(name, histo_nb);
    //       if (first_file) all_TH1F.emplace_back(dynamic_cast<TH1F*>(histo->Clone((name+"_manip").c_str())));
    //       // if (first_file) all_TH1F.emplace_back(std::unique_ptr<TH1F>(static_cast<TH1F*>(histo->Clone())));
    //       else 
    //       {
    //         print(all_TH1F[histo_nb]->GetName());
    //         if (name == all_TH1F[histo_nb]->GetName()) all_TH1F[histo_nb]->Add(histo);
    //         else throw_error("Root files not identical !!!");
    //       }
    //       histo_nb++;
    //     }
    //     delete obj;
    //   }
    //   // delete obj;
    // }
    // delete key;
    // file->Close();
    // first_file = false; // Usefull only at the first iteration*/



    //  0;
    // int end_peak_bin = 0;
    // for (int bin_i = maxbin; bin_i<high_edge_bin; ++bin_i)
    // {
    //   print(histo->GetBinContent(bin_i), max*0.7);
    //   if (histo->GetBinContent(bin_i)<max*0.7) {end_peak_bin = bin_i; break;}
    // }
    // for (int bin_i = maxbin; bin_i>low_edge_bin; --bin_i)
    // {
    //   print(histo->GetBinContent(bin_i), max*0.7);
    //   if (histo->GetBinContent(bin_i)<max*0.7) {begin_peak_bin = bin_i; break;}
    // }

    // // Try again to find the edges. If found too far away, this means we had to peak the first time.
    // auto const & first_left_displacement = maxbin-begin_peak_bin;
    // auto const & first_right_displacement = maxbin+end_peak_bin;
    // int begin_peak_bin_bis = 0;
    // int end_peak_bin_bis = 0;
    // for (int bin_i = maxbin; bin_i<first_left_displacement; ++bin_i)
    // {
    //   if (histo->GetBinContent(bin_i)<max*0.7) {end_peak_bin = bin_i; break;}
    // }
    // for (int bin_i = maxbin; bin_i>low_edge_bin; --bin_i)
    // {
    //   if (histo->GetBinContent(bin_i)<max*0.7) {begin_peak_bin = bin_i; break;}
    // }