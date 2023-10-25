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

  for (int bin = 0; bin<histo1->GetNbinsX(); bin++)
  {
    if (x) histo2->SetBinContent(index, bin, histo1->GetBinContent(bin));
    else   histo2->SetBinContent(bin, index, histo1->GetBinContent(bin));
  }
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
  std::unique_ptr<TF1> gaus_pol0;
  std::unique_ptr<TF1> fittedPic;
  double pospic, amppic, dump_sigma;
  double cte, Mean, sigma;

  // Histogram characteristics :
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  auto const xmax = spectra -> GetXaxis() -> GetXmax();
  auto const xmin = spectra -> GetXaxis() -> GetXmin();

  auto const xPerBin = (xmax-xmin)/bins;
  auto const bin0 = getBin0(spectra);
  
  // Extract dump parameters :
  amppic = spectra -> GetMaximum();
  pospic = static_cast<double>( (spectra->GetMaximumBin() - bin0)*xPerBin );
  dump_sigma = static_cast<double>( (spectra->FindLastBinAbove(amppic/2) - spectra->FindFirstBinAbove(amppic/2)) * xPerBin/2 );

  // Fits the peak :
  gaus_pol0.reset(new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*dump_sigma,pospic+20*dump_sigma));
  gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
  gaus_pol0 -> SetRange(pospic-dump_sigma*20,pospic+dump_sigma*20);
  spectra -> Fit(gaus_pol0.get(),"R+q");

  // Extracts the fitted parameters :
  fittedPic.reset (spectra -> GetFunction("gaus+pol0"));
  if (!fittedPic) return false; // Eliminate non existing fits, when not enough statistics fit doesn't converge
  cte = fittedPic -> GetParameter(0);
  mean = Mean = fittedPic -> GetParameter(1);
  sigma = fittedPic -> GetParameter(2);

  if (false) print("cte", cte, "Mean", Mean, "sigma", sigma);

  return true;
}

///////////////////////////
//   TREE MANIPULATIONS  //
///////////////////////////

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
auto createBranch(TTree* tree, T * value, std::string const & name)
{
  return (tree -> Branch(name.c_str(), value, (name+"/"+typeRootMap(*value)).c_str()), 64000);
}

/// @brief Create a branch for a given array and name
/// @param name_size: The name of the leaf that holds the size of the array
template<class T>
auto createBranchArray(TTree* tree, T * array, std::string const & name, std::string const & name_size)
{
  // using **array because it is an array, so *array takes the first element of the array
  return (tree -> Branch(name.c_str(), array, (name+"["+name_size+"]/"+typeRootMap(**array)).c_str()), 64000);
}


///////////////////////
//  CLASS THETCHAIN  //
///////////////////////

/**
 * @brief Not functionnal yet
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
  void normalizeY(TH2* matrix, double const & factor = 1)
  {
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    for (int x = 1; x<bins_x; x++)
    {
      Float_t maxRow = 0.;
      // 1 : Get the maximum
      for (int y = 1; y<bins_y; y++) if(matrix->GetBinContent(x, y) > maxRow) maxRow = matrix->GetBinContent(x, y);
      // 2 : Normalize to set maximum = factor
      if (maxRow>0) for (int y = 1; y<bins_y; y++) 
      {
        matrix -> SetBinContent(x, y, factor*matrix->GetBinContent(x, y)/maxRow);
      }
    }
  }

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

  /// @brief Project on Y axis at a given X bin
  void projectY(TH2* matrix, TH1* proj, int const & binX)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectY");}
    auto const & nbBins = matrix->GetNbinsY();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins,minXaxis(matrix), maxXaxis(matrix));
    for (int binY = 0; binY<nbBins; binY++) proj->SetBinContent(binY, matrix->GetBinContent(binX, binY));
  }

  /// @brief Project on Y axis between bin binXmin included and binXmax excluded [binXmin;binXmax[
  void projectY(TH2* matrix, TH1* proj, int const & binXmin, int const & binXmax)
  {
    if (matrix == nullptr) {throw_error("Matrix histo nullptr in CoAnalyse::projectY");}
    auto const & nbBins = matrix->GetNbinsY();
    if (proj == nullptr) proj = new TH1F();
    proj->SetBins(nbBins, minXaxis(matrix), maxXaxis(matrix));
    for (int x = binXmin; x<binXmax; x++) for (int y = 0; y<nbBins; y++) 
      proj->AddBinContent(y, matrix->GetBinContent(x, y));
  }

  /// @brief Project on Y axis between values valueXmin and valueXmax included [valueXmin;valueXmax]
  void projectY(TH2* matrix, TH1* proj, double const & valueXmin, double const & valueXmax)
  {
    projectY(matrix, proj, matrix->GetXaxis()->FindBin(valueXmin), matrix->GetXaxis()->FindBin(valueXmax));
  }

  void removeRandomY(TH2* matrix, int _stopX = -1, int _stopY = -1, bool writeIntermediate = false, std::vector<std::pair<double,double>> projections = {{508, 515}})
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

void removeRandomBidim(TH2* matrix, int iterations = 1, bool save_intermediate = false, std::vector<std::pair<double,double>> projections = {{}})
  {
    int const & bins_x = matrix->GetNbinsX();
    int const & bins_y = matrix->GetNbinsY();
    int startX = 0;
    int stopX = bins_x+1;
    int startY = 0;
    int stopY = bins_y+1;
    std::string matrix_name = matrix->GetName();
    auto const & iterations_sqr = iterations*iterations;
    // auto const & iterations_pow4 = iterations*iterations*iterations*iterations;
    // auto const maximum = matrix->GetMaximum();

    std::vector<std::vector<TH1*>> intermediate_proj(projections.size());
    std::vector<TH1D*> save_totProjX;
    std::vector<TH1D*> save_totProjY;
    // std::vector<TH2*> clones;
    std::vector<double> integrals(iterations_sqr*iterations,0.);

    // std::vector<std::vector<std::vector<double>>> save_sub(iterations_sqr*iterations);
    // std::vector<std::vector<double>> sub_moyX(iterations_sqr*iterations);
    // std::vector<std::vector<double>> sub_moyY(iterations_sqr*iterations);
    std::vector<std::vector<double>> sub_array;
    std::vector<TH1D*> save_sub_projX;
    std::vector<TH1D*> save_sub_projY;
    for (int it = 0; it<iterations_sqr*iterations; it++) 
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
    for (int x = startX; x<bins_x+1; x++) for (int y = startY; y<bins_y+1; y++) 
    {
      auto const & value = matrix->GetBinContent(x,y);
      totProjX[x] += value;
      totProjY[y] += value;
      totProjX_buf[x] += value;
      totProjY_buf[y] += value;
    }

    auto firstTotProjX = matrix->ProjectionX("firstTotProjX");
    auto firstTotProjY = matrix->ProjectionY("firstTotProjY");

    print("Substracting...");
    for (int it = 0; it<iterations_sqr*iterations; it++)
    {
      print("Iteration", it);
      if(save_intermediate)
      {
        save_totProjX.emplace_back(dynamic_cast<TH1D*>(firstTotProjX->Clone(("totProjX_"+std::to_string((int)(it))).c_str())));
        save_totProjY.emplace_back(dynamic_cast<TH1D*>(firstTotProjY->Clone(("totProjY_"+std::to_string((int)(it))).c_str())));
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
          auto const & value = matrix->GetBinContent(x, y);
          if (value == 0) continue;

          // V1 :
          // auto diff = (it>0) ? clones[it-1]->GetBinContent(x,y)*total/prev_total - value : 0;
          // auto const & sub = (totProjY[y] * totProjX[x])/(iterations*total);
          // auto const & new_value = value - sub - sqrt(diff)/iterations;

          // V2 :
          // save_sub[it][x][y] = (totProjX[x] * totProjY[y])/(iterations*total);
          // auto const new_value = value - save_sub[it][x][y];

          // V3 :
          auto sub = (totProjX[x] * totProjY[y])/(iterations*total);
          sub = sub *(1 - sub/(iterations*value));

          // matrix -> SetBinContent(x, y, value - sub);

          // totProjX_buf[x] -= sub;
          // totProjY_buf[y] -= sub;

          // totProjX_buf[x] -= save_sub[it][x][y];
          // totProjY_buf[y] -= save_sub[it][x][y];

          // sub_moyX[it][x] += save_sub[it][x][y]/totProjX[x]/stopX;
          // sub_moyY[it][y] += save_sub[it][x][y]/totProjY[y]/stopY;
        }
      }

      // Project on axis :
      if (save_intermediate) for (size_t proj_i = 0; proj_i<projections.size(); proj_i++)
      {
        auto histo = new TH1F();
        auto const & gate = projections[proj_i];
        projectY(matrix, histo, gate.first, gate.second);
        auto const & histo_name = matrix_name+"_"+std::to_string(gate.first)+"_"+std::to_string(gate.second)+"_"+std::to_string(it);
        intermediate_proj[proj_i].emplace_back(dynamic_cast<TH1F*>(histo->Clone(histo_name.c_str())));
      }

      // Update the total projections :
      totProjX = totProjX_buf;
      totProjY = totProjY_buf;

      // normalizeBidim(matrix, maximum);
    }

    print("Writting intermediate steps...");
    std::string filename = "Background_removed_"+matrix_name+".root";
    auto file = TFile::Open(filename.c_str(), "recreate");
    file->cd();
    matrix->Write();
    for (auto & histo : save_totProjX) if (histo!=nullptr) histo -> Write();
    for (auto & histo : save_totProjY) if (histo!=nullptr) histo -> Write();
    for (auto & histo : save_sub_projX) if (histo!=nullptr) histo -> Write();
    for (auto & histo : save_sub_projY) if (histo!=nullptr) histo -> Write();
    for (auto & projections : intermediate_proj) for (auto & histo : projections) if (histo!=nullptr) histo -> Write();
    file->Write();
    file->Close();
    print(filename, "written");
  }

  void setX(TH2* matrix, TH1* proj, int const & binX)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::setX");}
    if (proj == nullptr) {throw_error("Projection histo nullptr in CoAnalyse::setX");}
    for (int binY = 0; binY<matrix->GetNbinsY(); binY++) matrix->SetBinContent(binX, binY, proj->GetBinContent(binY));
  }

  void projectX(TH2* matrix, TH1* proj, int const & binY)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::projectX");}
    // if (proj == nullptr) proj = new TH1F("temp","temp",nXbins, bidim->GetXaxis()->GetXmax(), bidim->GetXaxis()->GetXmin());
    for (int binX = 0; binX<matrix->GetNbinsX(); binX++) {print(binX, binY); proj->SetBinContent(binX, matrix->GetBinContent(binX, binY));}
  }

  void setY(TH2* matrix, TH1* proj, int const & binY)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::setY");}
    if (proj == nullptr) {throw_error("Projection histo nullptr in CoAnalyse::setY");}
    for (int binX = 0; binX<matrix->GetNbinsX(); binX++) matrix->SetBinContent(binX, binY, proj->GetBinContent(binY));
  }


  std::vector<double> extractBackgroundArray(std::vector<double> & source, int const & nsmooth = 10)
  {
    print("depecrated (", nsmooth, ")");
    // auto s = new TSpectrum();
    // s->Background(source.data(),source.size(),nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
    // s->Delete();
    return source;
  }

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
  void removeBackground(TH1 * histo, int const & niter = 10, std::string const & fit_options = "", std::string const & bidim_options = "X")
  {
    auto const & dim = histo->GetDimension();
    if (dim == 1)
    {
      auto const & background = histo -> ShowBackground(niter, fit_options.c_str());
      for (int bin=0; bin<histo->GetNbinsX(); bin++) {histo->SetBinContent(bin, histo->GetBinContent(bin) - background->GetBinContent(bin));}
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

  // void removeBackground(TH1F * histo, int const & niter = 10, std::string const & options = "")
  // {
  //   // auto const & background = extractBackgroundArray(histo, nsmooth);
  //   auto const & background = histo -> ShowBackground(niter, options.c_str());
  //   for (int bin=0; bin<histo->GetNbinsX(); bin++) {histo->SetBinContent(bin, histo->GetBinContent(bin) - background->GetBinContent(bin));}
  // }

  // void removeBackgroundX(TH2F * histo, int const & nsmooth = 10)
  // {
    
  // }
};


////////////////////////////
//   Manage histo files   //
////////////////////////////

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
        // print(name);
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
            print("NOT THE SAME FILES :", nb_histos, "ème file : ", name, all_TH1F[nb_histos]->GetName());
            
            // Trying to find the histogram forward :
            auto const checkpoint = nb_histos;
            do {nb_histos++;} while (nb_histos<all_TH1F.size() && name != all_TH1F[nb_histos]->GetName());

            // If not found, create it at current position :
            if (nb_histos == all_TH1F.size()) 
            {
              all_TH1F.emplace(all_TH1F.begin()+checkpoint, std::unique_ptr<TH1>(dynamic_cast<TH1*>(histo->Clone((name).c_str()))));
              nb_histos = checkpoint+1;
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

void draw_all_TH1F_with_X_window(std::string const & filename, int minX, int maxX, int rebin = 1)
{
  auto file = TFile::Open(filename.c_str(), "READ");
  auto list = file->GetListOfKeys();
  auto c1 = new TCanvas("c1");
  for (auto&& keyAsObj : *list)
  {
    std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
    if(std::string(key->GetClassName()) == "TH1F")
    {
      std::unique_ptr<TObject> obj (key->ReadObj());
      auto histo = dynamic_cast<TH1F*>(obj.get());
      std::string name = histo->GetName();
      print(name);
      histo->Rebin(rebin);
      histo->GetXaxis()->SetRangeUser(minX, maxX);

      // Create the TApplication :
      // int argc = 0; 
      // char** argv = nullptr;
      // TApplication app("app", &argc, argv);
      // TCanvas* c1 = new TCanvas("c1");
      c1->cd();
      histo->Draw();

      // TObjArray* histArray = new TObjArray();
      // histArray->Add(histo);
      // c1->Connect("onClick", "onClick(int,int,int,TObject*", 0, 0);
      gPad->WaitPrimitive();
      c1->Update();

      // app.Run();

      // gPad->Update();
      // gPad->WaitPrimitive();
      // c1->WaitPrimitive();
      // pauseCo();
    }
  }
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