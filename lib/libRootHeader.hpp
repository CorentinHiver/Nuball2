#ifndef LIBROOTHEADER_HPP
#define LIBROOTHEADER_HPP

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

#ifdef INCLUDE_MINUIT
// Require additional -lMinuit2 in the compilation line
  #include "Minuit2/Minuit2Minimizer.h"
  #include "Math/Functor.h"
  #include "Math/Factory.h"
  #include "Math/Minimizer.h"
#endif //INCLUDE_MINUIT

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

#endif //LIBROOTHEADER_HPP
