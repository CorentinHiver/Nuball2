#ifndef LIBROOTCO_HPP
#define LIBROOTCO_HPP

#include "libCo.hpp"

// ********** ROOT includes ********* //
#include "TCanvas.h"
#include "TChain.h"
#include "TError.h"
#include "TF1.h"
#include "TF2.h"
#include "TFile.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH1S.h"
#include "TH2.h"
#include "TH2F.h"
#include "TH3I.h"
#include "TKey.h"
#include "TLeaf.h"
#include "TLegend.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TThread.h"
#include "TTree.h"
#include "TTreeIndex.h"

////////////////////////////
//   HISTO MANIPULATIONS  //
////////////////////////////

bool THist_exists(TH1* histo)
{
  return (histo && !histo->IsZombie() && histo->Integral()>1);
}

///////////////////////////
//   TREE MANIPULATIONS  //
///////////////////////////

void alignator(TTree * tree, int *NewIndex)
{
  int const NHits = tree -> GetEntries();
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Read only the time
  // tree -> SetBranchStatus("label", true);// Read only the time
  std::vector<ULong64_t> TimeStampBuffer(NHits,0);
  ULong64_t TimeStamp = 0; tree->SetBranchAddress("time", &TimeStamp);
  // UShort_t label = 0; tree->SetBranchAddress("label", &label);
  for (int i = 0; i<NHits; i++)
  {
    tree -> GetEntry(i);
    // if (i<20) std::cout << label << "\t" << TimeStamp << std::endl;
    TimeStampBuffer[i]=TimeStamp;
  }
  int i = 0, j = 0;
  ULong64_t a = 0;
  NewIndex[0]=0;
	for (j=0; j<NHits;j++)
	{
  	NewIndex[j]=j;
  	a=TimeStampBuffer[j]; //Focus on this time stamp
  	i=j;
		// Find the place to insert it amongst the previously sorted
  	while((i > 0) && (TimeStampBuffer[NewIndex[i-1]] > a))
  	{
    	NewIndex[i]=NewIndex[i-1];
    	i--;
  	}
  	NewIndex[i]=j;
	}
  tree -> SetBranchStatus("*", true); //enables again the whole tree
}

void test_alignator(TTree *tree, int* NewIndex= nullptr, bool useNewIndex = false)
{
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Eneables to read only the time
  ULong64_t TimeStamp; tree->SetBranchAddress("time", &TimeStamp);
  ULong64_t PrevTimeStamp = 0; int j = 0;
  int maxIt = tree -> GetEntries();
  for (int i = 0; i < maxIt; i++)
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



/////////////////////////
//   CLASS THE TCHAIN  //
/////////////////////////

/*
 * 1: Add all the files
 * TheTChain chain("Nuball", "/path/to/data/files*.root");
 * chain.Add("/other_path/to/data/files*.root")
 *
 * 2: Setup the chain :
 * chain.set();
 *
 * 3: Links all the variables
 * chain.SetBranchAddress("branch", &variable);
 *
 *
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
  for (auto const & expression : m_input_files_expressions)
  {
    if (!folder_exists(expression)) {print("folder",getPath(expression),"empty !");return;}
    if (expression.back() == '/')
    {// If a folder is given then search the whole folder for .root files
      findFilesWildcard(expression+"*.root", m_files_vec);
    }
    else findFilesWildcard(expression, m_files_vec);
  }
  print(m_files_vec);
  for (auto const & filename : m_files_vec) newTTree(filename);
}

#endif //LIBROOTCO_HPP
