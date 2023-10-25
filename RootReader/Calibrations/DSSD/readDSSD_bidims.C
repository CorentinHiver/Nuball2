#include "../../../lib/libRoot.hpp"

void readDSSD_bidims()
{
  std::vector<std::string> names = 
  {
    // "S1_0_DSSD",
    // "S1_1_DSSD",
    "S1_2_DSSD",
    // "S1_3_DSSD",
    // "S1_4_DSSD",
    // "S1_5_DSSD",
    // "S1_6_DSSD",
    // "S1_7_DSSD",
    // "S1_8_DSSD",
    // "S1_9_DSSD",
    // "S1_10_DSSD",
    // "S1_11_DSSD",
    // "S1_12_DSSD",
    // "S1_13_DSSD",
    // "S1_14_DSSD",
    // "S1_15_DSSD",
    // "S2_1_DSSD",
    // "S2_2_DSSD",
    // "S2_5_DSSD",
    // "S2_6_DSSD",
    // "S2_8_DSSD",
    // "S2_9_DSSD",
    // "S2_10_DSSD",
    // "S2_11_DSSD",
    // "S2_12_DSSD",
    // "S2_13_DSSD",
    // "S2_14_DSSD",
    // "S2_15_DSSD"
  };

  auto file = TFile::Open("DSSD_bidims.root", "READ");
  auto out = TFile::Open("DSSD_final.root", "recreate");
  for (auto const & name : names)
  {
    file->cd();
    auto matrix = file->Get<TH2F>(name.c_str());
    CoAnalyse::removeRandomBidim(matrix, 3, true, {{}},{{167,169}});
    auto proj_169 = new TH1F();
    CoAnalyse::projectX(matrix, proj_169, 167., 169.);
    proj_169->Draw();
    print(proj_169->GetMean());
    out->cd();
    matrix->Write();
    out -> Write();

    // delete histo;
    // delete proj_169;
  }
  out->Close();
  file->Close();
}