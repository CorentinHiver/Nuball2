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
#include <TLeaf.h>
#include <TLegend.h>
#include <TMarker.h>
#include <TMath.h>
#include <TPolyMarker.h>
#include <TRandom.h>
#include <TROOT.h>
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
using unique_TH2F  = std::unique_ptr<TH2F>;
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


////////////////////////////
//   HISTO MANIPULATIONS  //
////////////////////////////

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

double peak_over_background(TH1* histo, int bin_min, int bin_max, TH1* background)
{
  auto const & peak = histo     ->Integral(bin_min, bin_max);
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
  auto const & peak = histo     ->Integral(bin_min, bin_max);
  auto const & bckg = background->Integral(bin_min, bin_max);
  if (peak == 0) return 0;
  else return (peak-bckg)/sqrt(peak);
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
TH1F* count_to_peak_over_background(TH1* histo, int resolution, int smooth_background_it = 20)
{
  std::string name = histo->GetName(); name += "_peak_over_background";
  std::string title = histo->GetName(); title+=";keV;peak_over_background;";
  auto background = histo->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), histo->GetNbinsX(), histo->GetXaxis()->GetXmin(), histo->GetXaxis()->GetXmax());
  for (int bin = 0+resolution; bin<histo->GetNbinsX(); ++bin)
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

TH1F* AND(TH1* histo1, TH1* histo2, int smooth_background_it = 20)
{
  int bins = histo1->GetNbinsX();
  double min = histo1->GetXaxis()->GetXmin();
  double max = histo1->GetXaxis()->GetXmax();
  if (bins != histo2->GetNbinsX()) bins = (histo1->GetNbinsX() < histo2->GetNbinsX()) ? histo1->GetNbinsX() : histo1->GetNbinsX();
  if (min  != histo2->GetXaxis()->GetXmin()) {print("in AND(TH1F *histo1, TH1F *histo2) : axis inconsistent..."); return nullptr;}
  if (max  != histo2->GetXaxis()->GetXmax()) {print("in AND(TH1F *histo1, TH1F *histo2) : axis inconsistent..."); return nullptr;}
  std::string name = histo1->GetName() + std::string("_AND_") + histo1->GetName();
  std::string title = histo1->GetTitle() + std::string("_AND_") + histo1->GetTitle();
  auto background1 = histo1->ShowBackground(smooth_background_it);
  auto background2 = histo2->ShowBackground(smooth_background_it);
  auto ret = new TH1F(name.c_str(), title.c_str(), bins, min, max);
  for (int bin = 0; bin<bins; ++bin)
  {
    auto const & count_1 = histo1->GetBinContent(bin) - background1->GetBinContent(bin) ;
    auto const & count_2 = histo2->GetBinContent(bin) - background2->GetBinContent(bin) ;
    ret->SetBinContent(bin, (count_1<count_2) ? count_1 : count_2);
  }
  return ret;
}


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

  std::vector<std::string> m_input_files_expressions;
  std::vector<std::string> m_files_vec;

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


///////////////////
//   COANALYSE   //
///////////////////

namespace CoAnalyse
{
  bool inline checkMatrixSquare(TH2* mat) noexcept
  {
    if (!mat) {error(" in CoAnalyse::checkMatrixSquare(TH2* mat) : mat is nullptr"); return false;}
    return (mat->GetNbinsX() == mat->GetNbinsY()
    || mat->GetXaxis()->GetXmin()== mat->GetYaxis()->GetXmin()
    || mat->GetXaxis()->GetXmax()== mat->GetYaxis()->GetXmax());

  }
  /// @brief Vector of pairs of min and max bins 
  using ProjectionsBins = std::vector<std::pair<double,double>>;

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
          auto const & histo_name = matrix_name+"_projY_"+std::to_string(gate.first)+"_"+std::to_string(gate.second)+"_"+std::to_string(it);
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
        auto const & new_value = histo->GetBinContent(bin) - background->GetBinContent(bin);
        if (nice) histo->SetBinContent(bin, (new_value<1) ? 1 : new_value);
        else histo->SetBinContent(bin, new_value);
      }
    }

    else if (dim == 2)
    {
      error ("weird, removeBackground(TH2*) should have been called ....");
      return;
      char choice = 0; // 0 : X, 1 : Y, 2 : symmetric
      // if (bidim_options.find("Y")) choice = 1;
      // if (bidim_options.find("S")) choice = 2;
      auto bidim = dynamic_cast<TH2F*>(histo);

      auto const & nXbins = bidim -> GetNbinsX();
      auto const & nYbins = bidim -> GetNbinsY();

      if (choice == 2)
      {
        if (nXbins != nYbins) {print("CoAnalyse::removeBackground for 2D spectra is suited only for symmetric spectra"); return;}
      }

      switch (choice)
      {
        case 0: case 2: 
          // Subtract the background of Y spectra gating on each X bins
          for (int binX = 0; binX<nXbins; binX++)
          {
            TH1F* histo1D = nullptr; 
            projectY(bidim, histo1D, binX);
            removeBackground(histo1D, niter);
            setX(bidim, histo1D, binX);
            delete histo1D;
          }
        break;

        case 1:
          // Subtract the background of X spectra gating on each Y bins
          for (int binY = 0; binY<nYbins; binY++)
          {
            TH1F* histo1D = nullptr; new TH1F("temp","temp",nYbins, bidim->GetYaxis()->GetXmax(), bidim->GetYaxis()->GetXmin());
            projectX(bidim, histo1D, binY);
            removeBackground(histo1D, niter);
            setY(bidim, histo1D, binY);
            delete histo1D;
          }
        break;
      }

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
    for (;bin<histo->GetNbinsX()+1; ++bin) lookup.push_back(false); // After the last peak, fill false
    if (remove511) for (int bin_i = 505; bin_i<517; ++bin_i)lookup[bin_i] = true;
    return lookup;
  }

  /// @brief Based on Radware methods D.C. Radford/Nucl. Instr. and Meth. in Phys. Res. A 361 (1995) 306-316
  /// @param choice: 0 : classic radware | 1 : Palameta and Waddington (PW) | 2 : classic + remove diagonals | 3 : PW + remove diagonals
  void removeBackground(TH2 * histo, int const & niter = 20, uchar const & choice = 0, double const & threshold = 0.05, double const & sigmaX = 2., double const & sigmaY = 2., bool remove511 = false)
  {
    if (!checkMatrixSquare(histo)) {error ("The matrix must be square"); return;}

    auto const & T = histo->Integral();

    auto projX0 = histo->ProjectionX("projX0"); // Get the total X projection of the matrix
    auto bckgX = projX0->ShowBackground(niter); // Get the total X projection's fitted background 
    auto projX = static_cast<TH1D*>(projX0->Clone("projX")); // Prepare the background-clean total X projection
    projX->Add(bckgX, -1); // Calculate the background-subtracted spectra

    auto projY0 = histo->ProjectionY("projY0");
    auto bckgY = projY0->ShowBackground(niter);
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
    
    if(choice == 0 || choice == 2) // classic radware
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
    if (choice > 0) // Finds the biggest peaks in the total projections :
    {
      // Promotes negative values to 0 in the background-substracted projections :
      for (int bin = 1; bin<Nx; ++bin)
      {
        if(projX->GetBinContent(bin) < 0) projX->SetBinContent(bin, 0);
        if(projY->GetBinContent(bin) < 0) projY->SetBinContent(bin, 0);
      }

      // Get the biggest peaks in each projection
      auto const & peaksX = mainPeaksLookup(projX, sigmaX, threshold, 1, 1, remove511);
      auto const & peaksY = mainPeaksLookup(projY, sigmaY, threshold, 1, 1, remove511);
      print(peaksX.size(), peaksY.size());
      auto projX_bis = new TH1D("projX_bis", "projX_bis", Nx,xmin,xmax);
      auto projY_bis = new TH1D("projY_bis", "projY_bis", Ny,ymin,ymax);
      auto proj_diag = new TH1D("proj_diag", "proj_diag", 2*Nx,xmin,2*xmax);
      int sum_y = 0; // for projX
      int sum_x = 0; // for projY
      
      // The following works only for a symmetric matrix :
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

      if (choice == 1 || choice == 3) // Palameta and Waddington
      {
        // Calculate the constant S and A used for the method (in the classic method, S=T and A=0 )
        auto const & S = projX_bis->Integral();
        double A = 0.0; for (int x = 0; x<Nx; ++x) if (peaksX[x]) A+=projX_bis->GetBinContent(x);
        A/=S;

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

      if (choice > 1) // Removes the diagonals
      {
        for (int bin_x = 1; bin_x<Nx; ++bin_x) for (int bin_y = 1; bin_y<Ny; ++bin_y)
        {
          auto const & bin_sum = bin_x+bin_y;
          if ((size_cast(bin_sum)<peaksX.size() && peaksX[bin_sum]) ||
              (size_cast(bin_sum)<peaksY.size() && peaksY[bin_sum]))
          {
            auto const & counts = bckg_clean->GetBinContent(bin_x, bin_y);
            proj_diag->SetBinContent(bin_sum, proj_diag->GetBinContent(bin_sum)+counts);
            bckg_clean->SetBinContent(bin_x, bin_y, 0);
          }
        }
      }
    // #ifndef __CINT__
    //   delete projX_bis;
    //   delete projY_bis;
    // #endif //!__CINT__
    // proj_diag->Draw();
    }

    for (int x=0; x<histo->GetNbinsX(); x++) for (int y=0; y<histo->GetNbinsY(); y++) 
      histo->SetBinContent(x, y, bckg_clean->GetBinContent(x, y));
  // #ifndef __CINT__
  //   delete projX;
  //   delete bckgX;
  //   delete projY;
  //   delete bckgY;
  //   delete bckg_clean;
  //   delete bckg2D;
  // #endif //!__CINT__
  }

  TH1D* projectDiagonals(TH2* histo)
  {
    if (!checkMatrixSquare(histo)) return (new TH1D("void","void",1,0,1));
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

  /// @brief Work in progress
  void removeDiagonals(TH2* histo, int nb_iter = 20, int choice = 0)
  {
    if (choice == 0)
    {
      auto projDiag = projectDiagonals(histo);
      // auto projDiag0 = static_cast<TH2*>(histo->Clone("projDiag0"));
      auto bckgProjDiag = projDiag->ShowBackground(nb_iter, "q");
      projDiag->Add(bckgProjDiag, -1);

      int const & T = projDiag->Integral();
      auto clone = static_cast<TH2*>(histo->Clone("projDiag_temp"));
      for (int bin_x = 1; bin_x < histo->GetNbinsX()+1; ++bin_x) for (int bin_y = 1; bin_y < bin_x; ++bin_y)
      {
        auto const & init_value_xy = clone->GetBinContent(bin_x, bin_y);
        auto const & init_value_yx = clone->GetBinContent(bin_y, bin_x);
        auto const & diag_value = projDiag->GetBinContent(bin_x+bin_y);
        auto const & diag_bckg_value = bckgProjDiag->GetBinContent(bin_x+bin_y);
        auto const & correction_xy = int_cast((diag_value*diag_bckg_value)/T);
        auto const & correction_yx = int_cast((diag_value*diag_bckg_value)/T);
        histo->SetBinContent(bin_x, bin_y, init_value_xy - correction_xy);
        histo->SetBinContent(bin_y, bin_x, init_value_yx - correction_yx);
      }
    }
    else if (choice == 1)
    {
      
    }
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


////////////////////////////
//   Manage histo files   //
////////////////////////////

std::vector<std::string> get_list_histo(TFile * file, std::string const & class_name = "TH1F")
{
  std::vector<std::string> ret;
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

std::vector<std::string> get_TH1F_names(std::string const & filename)
{
  auto file = TFile::Open(filename.c_str());
  auto ret =  get_list_histo(file, "TH1F");
  file->Close();
  return ret;
}

std::vector<std::string> get_TH1F_names(TFile * file)
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

TH1F_map get_TH1F_map(TFile * file, std::vector<std::string> & names)
{
  TH1F_map ret;
  names = get_TH1F_names(file);
  for (auto const & name : names)
  {
    ret.emplace(name, file->Get<TH1F>(name.c_str()));
  }
  return ret;
}

template<class T>
std::vector<std::string> get_names_of(TFile * file)
{
  std::vector<std::string> ret;
  T* temp_obj = new T;
  if (!file) {print("in get_names_of<T>(TFile * file) : file is nullptr"); return ret;}
  auto list = file->GetListOfKeys();
  for (auto&& keyAsObj : *list)
  {
    std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
    if(strcmp(key->GetClassName(), temp_obj->ClassName()) == 0)
    {
      TObject* obj = key->ReadObj();
      T* t = dynamic_cast<T*>(obj);
      ret.push_back(t->GetName());
    }
  }
  delete temp_obj;
  return ret;
}

template<class T>
std::map<std::string, T> create_map_of(TFile * file)
{
  throw_error("create_map_of<T>() DEV !!");
  std::map<std::string, T> ret;
  auto names (get_names_of<T>(file));
  for (auto const & name : names)
  {
    ret.emplace(name, file->Get<T>(name.c_str()));
  }
  return ret;
}

////////////
// OTHERS //
////////////

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

double selectXPoints(TH1* histo, std::string const & instructions)
{
  double x = 0; double y = 0;
  gPad->SetTitle(instructions.c_str());
  histo->SetTitle(instructions.c_str());
  GetPoint(gPad->cd(), x, y);
  gPad->Update();
  return x;
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
  auto projY = histo->ProjectionY(concatenate_c("projY_", histo->GetName()));
  projY->GetYaxis()->SetRangeUser(x_value-5*resolution, x_value+5*resolution);
  auto const & title = histo->GetTitle();
  double const & xvalue_min = selectXPoints(projY, "Select low edge of peak");
  double const & xvalue_max = selectXPoints(projY, "Select high edge of peak");
  double const & xvalue_min_bckg = selectXPoints(projY, "Select low edge of background");
  double const & xvalue_max_bckg = selectXPoints(projY, "Select high edge of background");
  delete projY;
  return myProjectionX(histo, name, xvalue_min, xvalue_max, xvalue_min_bckg, xvalue_max_bckg);
}


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

void libRoot()
{
  print("Welcome to Corentin's ROOT library. May you find some usefull stuff around !");
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