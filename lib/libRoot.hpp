#ifndef LIBROOT_HPP
#define LIBROOT_HPP

#include "libCo.hpp"
// ********** ROOT includes ********* //
#include <TAxis.h>
#include <TCanvas.h>
#include <TChain.h>
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
    if (!initialized)
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

      initialized = true;
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
  static bool initialized;
  std::unordered_map<std::type_index, std::string> m_typeRootMap;
}typeRootMap;

bool TypeRootMap::initialized = false;

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


///////////////////////
//  CLASS THETCHAIN  //
///////////////////////

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

/////////////////////////
//   USEFULL CLASSES   //
/////////////////////////

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

    print("Substracting...");
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

    print("Substraction done, copying back...");
    delete matrix;
    matrix = static_cast<TH2*>(clone->Clone());

    // print("Renormalising...");
    // normalizeBidim(matrix, 1);

    print("RemoveRandomY done.");
    if (writeIntermediate)
    {
      print("Writting intermediate steps...");
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
  void removeRandomBidim(TH2* matrix, int iterations = 1, bool save_intermediate = false, ProjectionsBins projectionsY = {{}}, ProjectionsBins projectionsX = {{}})
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

    print("Substracting", matrix_name, "with", iterations, "iterations...");
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

      // V4 : (the iterations are done excluding the extremal lines to avoid edge effet :)
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
      print("Writting intermediate steps...");
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
    print("depecrated (", nsmooth, ")");
    // auto s = new TSpectrum();
    // s->Background(source.data(),source.size(),nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
    // s->Delete();
    return source;
  }

  /// @deprecated
  std::vector<double> extractBackgroundArray(TH1F * histo, int const & nsmooth = 10)
  {
    print("depecrated (", histo->GetName(), nsmooth, ")");
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
   * @param niter: Choose a higher number of iterations if the peaks have high resultion (5-10 for LaBr3, 20 for Ge)
   * @param fit_options: options from TH1::ShowBackground
   * @param bidim_options: 
   *    - "X" (default)  : Loop through the X bins, find the background on the Y projection
   *    - "Y" :            Loop through the Y bins, find the background on the X projection
   *    - "S" (symmetric): Loop through the X bins, find the background on the Y projection, then symmetrise the bidim
   *    
   */
  void removeBackground(TH1 * histo, int const & niter = 10, std::string const & fit_options = "", std::string const & bidim_options = "X") noexcept
  {
    if (!histo || histo->IsZombie()) return;
    auto const & dim = histo->GetDimension();
    if (dim == 1)
    {
      auto const & background = histo -> ShowBackground(niter, fit_options.c_str());
      for (int bin=0; bin<histo->GetNbinsX(); bin++) 
      {
        auto const & new_value = histo->GetBinContent(bin) - background->GetBinContent(bin);
        histo->SetBinContent(bin, (new_value<1) ? 1 : new_value);
      }
    }

    else if (dim == 2)
    {
      char choice = 0; // 0 : X, 1 : Y, 2 : symmetric
      if (bidim_options.find("Y")) choice = 1;
      if (bidim_options.find("S")) choice = 2;
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
          // Substract the background of Y spectra gating on each X bins
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
          // Substract the background of X spectra gating on each Y bins
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
      //   //2. Resymetrise the matrice : (PROTOTYPAL !)
      //   print("Symetrisation : ");
      //   for (int binY = 0; binY<nYbins; binY++) for (int binX = 0; binX<nXbins; binX++)
      //   {
      //     histo->SetBinContent(binX, binY, histo->GetBinContent(binY, binX));
      //   }
      // }
    }
  }
};


////////////////////////////
//   Manage histo files   //
////////////////////////////

std::vector<std::string> get_TH1F_names(TFile * file)
{
  std::vector<std::string> ret;
  if (!file) {print("in get_TH1F_names(TFile * file) : file is nullptr"); return ret;}
  auto list = file->GetListOfKeys();
  for (auto&& keyAsObj : *list)
  {
    std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
    std::string className =  key->GetClassName();
    if(className == "TH1F")
    {
      TObject* obj = key->ReadObj();
      TH1* histo = dynamic_cast<TH1*>(obj);
      ret.push_back(histo->GetName());
    }
  }
  return ret;
}

std::vector<std::string> get_TH1F_names(std::string const & filename)
{
  auto file = TFile::Open(filename.c_str());
  auto ret =  get_TH1F_names(file);
  file->Close();
  return ret;
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