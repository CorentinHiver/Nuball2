#include "../lib/Analyse/Minimiser.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/Detectors.hpp"

void ReadDataAlignement()
{
  detectors.load("../136/index_129.list");
  CalibAndScale calib;
  
  auto init_chi2 = new TH2F("init_chi2", "init #chi^{2};label;run", 1000,0,1000, 200,0,200);
  auto best_chi2 = new TH2F("best_chi2", "best #chi^{2};label;run", 1000,0,1000, 200,0,200);
  auto best_first_bs = new TH2F("best_first_bs", "best_first_bs;label;run", 1000,0,1000, 200,0,200);
  auto best_first_as = new TH2F("best_first_as", "best_first_as;label;run", 1000,0,1000, 200,0,200);
  auto best_first_Cs = new TH2F("best_first_Cs", "best_first_Cs;label;run", 1000,0,1000, 200,0,200);
  auto best_bs = new TH2F("best_bs", "best_bs;label;run", 1000,0,1000, 200,0,200);
  auto best_as = new TH2F("best_as", "best_as;label;run", 1000,0,1000, 200,0,200);
  auto best_Cs = new TH2F("best_Cs", "best_Cs;label;run", 1000,0,1000, 200,0,200);

  for (auto const & detector : detectors)
  {
    TString det_name = detector.data();
    int det_label = detectors[detector];
    
    auto file = TFile::Open("Alignement/"+det_name+".root", "READ");
    if (!file) {print(det_name, "not found"); continue;}
    print("processing", file->GetName());

    auto init_chi2_evol = file->Get<TH1F>("init_chi2_evol");
    auto best_chi2_evol = file->Get<TH1F>("best_chi2_evol");
    auto best_first_bs_evol = file->Get<TH1F>("best_first_bs_evol");
    auto best_first_as_evol = file->Get<TH1F>("best_first_as_evol");
    auto best_first_Cs_evol = file->Get<TH1F>("best_first_Cs_evol");
    auto best_bs_evol = file->Get<TH1F>("best_bs_evol");
    auto best_as_evol = file->Get<TH1F>("best_as_evol");
    auto best_Cs_evol = file->Get<TH1F>("best_Cs_evol");

    if (best_chi2_evol) Colib::AddTH1 (best_chi2, best_chi2_evol, det_label, true);
    if (init_chi2_evol) Colib::AddTH1 (init_chi2, init_chi2_evol, det_label, true);
    if (best_first_bs_evol) Colib::AddTH1 (best_first_bs, best_first_bs_evol, det_label, true);
    if (best_first_as_evol) Colib::AddTH1 (best_first_as, best_first_as_evol, det_label, true);
    if (best_first_Cs_evol) Colib::AddTH1 (best_first_Cs, best_first_Cs_evol, det_label, true);
    if (best_bs_evol) Colib::AddTH1 (best_bs, best_bs_evol, det_label, true);
    if (best_as_evol) Colib::AddTH1 (best_as, best_as_evol, det_label, true);
    if (best_Cs_evol) Colib::AddTH1 (best_Cs, best_Cs_evol, det_label, true);

  }

}