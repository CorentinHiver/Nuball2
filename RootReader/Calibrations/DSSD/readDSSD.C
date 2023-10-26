#include "../../../lib/libRoot.hpp"
#include "TPad.h"

  std::vector<std::string> names = 
  {
    "S1_2_DSSD",
    "S1_3_DSSD",
    "S1_4_DSSD",
    "S1_5_DSSD",
    "S1_6_DSSD",
    "S1_7_DSSD",
    "S1_8_DSSD",
    "S1_9_DSSD",
    "S1_10_DSSD",
    "S1_11_DSSD",
    "S1_12_DSSD",
    "S1_13_DSSD",
    "S1_14_DSSD",
    "S1_15_DSSD",
    "S2_1_DSSD",
    "S2_2_DSSD",
    "S2_5_DSSD",
    "S2_6_DSSD",
    "S2_8_DSSD",
    "S2_9_DSSD",
    "S2_10_DSSD",
    "S2_11_DSSD",
    "S2_12_DSSD",
    "S2_13_DSSD",
    "S2_14_DSSD",
    "S2_15_DSSD"
  };

void readDSSD(char choix = 0)
{

  if (choix == 0)
  {// Read the initial DSSD_bidims.root and removes the background
    std::vector<TH2F*> matrixes;
    std::vector<TH1D*> proj169;

    auto file = TFile::Open("DSSD_bidims.root", "READ");
    for (auto const & name : names)
    {
      matrixes.emplace_back(file->Get<TH2F>(name.c_str()));
      auto & matrix = matrixes.back();
      CoAnalyse::removeRandomBidim(matrix, 30);
      // proj169.emplace_back(new TH1D());
      // CoAnalyse::projectX(matrix, proj169.back(), 167., 171.);
      // auto c = new TCanvas("canvas", name.c_str());
      // proj169.back()->Draw();
      // c->Update();
      // c->WaitPrimitive();
      break;
    }

    auto out = TFile::Open("DSSD_final.root", "recreate");
    out->cd();
    for (auto & matrix : matrixes) if (matrix) matrix->Write();
    out->Write();
    out->Close();  
    print("DSSD_final.root written");
  }
  else if (choix == 1)
  {// Reads the DSSD_final.root
    auto file2 = TFile::Open("DSSD_bidims.root", "READ");
  }
}