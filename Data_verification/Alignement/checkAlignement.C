#include "../../lib/libRoot.hpp"
#include "../../lib/Classes/Detectors.hpp"

void checkAlignement()
{
  detectors.load("../../136/index_129.list");

  auto bestChi2s = new TH2F("bestChi2s", "bestChi2s;label;run;best #Chi^{2}", 1000,0,1000, 100, 50, 150);

  for (auto const & detector : detectors)
  {
    if (detector == "") continue;
    auto tfile = TFile::Open(TString(detector.c_str()) + ".root", "READ");
    if (!tfile) {print("No file for", detector); continue;}
    auto bestChi2 = tfile->Get<TH1F>("best_chi2_evol");
    Colib::AddTH1(bestChi2s, bestChi2, detectors[detector], true);
  }

  bestChi2s->Draw();
}