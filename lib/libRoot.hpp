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
    m_typeRootMap[typeid(true)          ] = "O";

    m_typeRootMap[typeid(char_cast(1))  ] = "B"; m_typeRootMap[typeid(uchar_cast(1)) ] = "b";
    m_typeRootMap[typeid(short_cast(1)) ] = "S"; m_typeRootMap[typeid(ushort_cast(1))] = "s";
    m_typeRootMap[typeid(int_cast(1))   ] = "I"; m_typeRootMap[typeid(uint_cast(1))  ] = "i";
    m_typeRootMap[typeid(long_cast(1))  ] = "G"; m_typeRootMap[typeid(ulong_cast(1)) ] = "g";
    m_typeRootMap[typeid(double_cast(1))] = "D"; m_typeRootMap[typeid(float_cast(1)) ] = "F";

    m_typeRootMap[typeid(Long64_cast(1))] = "L"; m_typeRootMap[typeid(ULong64_cast(1)) ] = "l";
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
  std::unordered_map<std::type_index, std::string> m_typeRootMap;
}typeRootMap;

/// @brief Create a branch for a given value and name
template<class T>
auto createBranch(TTree* tree, T * value, std::string const & name)
{
  // print(name+"/"+typeRootMap(*value));
  return (tree -> Branch(name.c_str(), value, (name+"/"+typeRootMap(*value)).c_str()), 64000);
}

/// @brief Create a branch for a given array and name
/// @param name_size: The name of the leaf that holds the size of the array
template<class T>
auto createBranchArray(TTree* tree, T * array, std::string const & name, std::string const & name_size)
{
  // using **array because it is an array, so *array takes the first element of the array
  // print(name+"["+name_size+"]/"+typeRootMap(**array));
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
  void projectY(TH1* matrix, TH1* proj, int const & binX)
  {
    if (matrix == nullptr){throw_error("Matrix histo nullptr in CoAnalyse::projectY");}
    if (proj == nullptr) {throw_error("Projection histo nullptr in CoAnalyse::projectY");}
    auto const & nbBins = matrix->GetNbinsY();
    proj->SetBins(nbBins, 0, nbBins);
    for (int binY = 0; binY<nbBins; binY++) 
    {
      proj->SetBinContent(binY, matrix->GetBinContent(binX, binY));
    }

  }
  void setX(TH1* matrix, TH1* proj, int const & binX)
  {
    for (int binY = 0; binY<matrix->GetNbinsY(); binY++) matrix->SetBinContent(binX, binY, proj->GetBinContent(binY));
  }

  void projectX(TH1* matrix, TH1* proj, int const & binY)
  {
    for (int binX = 0; binX<matrix->GetNbinsX(); binX++) proj->SetBinContent(binX, matrix->GetBinContent(binX, binY));
  }
  void setY(TH1* matrix, TH1* proj, int const & binY)
  {
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
      char choice = 0;
      if (bidim_options.find("Y")) choice = 1;
      if (bidim_options.find("S")) choice = 2;

      auto const & nXbins = histo -> GetNbinsX();
      auto const & nYbins = histo -> GetNbinsY();

      if (choice == 2)
      {
        if (nXbins != nYbins) {print("CoAnalyse::removeBackground for 2D spectra is suited only for symetric spectra"); return;}
      }

      switch (choice)
      {
        case 0: case 2: 
          // Substract the background of Y spectra gating on each X bins
          for (int binX = 0; binX<nXbins; binX++)
          {
            std::unique_ptr<TH1F> histo1D(new TH1F());
            projectY(histo, histo1D.get(), binX);
            removeBackground(histo1D.get(), niter);
            setX(histo, histo1D.get(), binX);
          }
        break;

        case 1:
          // Substract the background of X spectra gating on each Y bins
          for (int binX = 0; binX<nYbins; binX++)
          {
            std::unique_ptr<TH1F> histo1D;
            projectY(histo, histo1D.get(), binX);
            removeBackground(histo1D.get(), niter);
            setX(histo, histo1D.get(), binX);
          }
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

#endif //LIBROOT_HPP
