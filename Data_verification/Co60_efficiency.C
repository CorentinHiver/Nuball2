

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

long max_cursor = 1.e+11;
Label timing_ref_label = 252;
Time time_window = 50_ns;

// All the gates are inclusive :
float gate_low = 1331_keV;
float gate_high = 1335_keV;
float coinc_low = 1171_keV;
float coinc_high = 1176_keV;

void eff(TH1F* histo, int nb_gate, int coinc_low_fit = 900, int coinc_high_fit = 1400, int coinc_low_fit_bckg = 900, int coinc_high_fit_bckg = 1400)
{
  histo->GetXaxis()->SetRangeUser(coinc_low_fit, coinc_high_fit);
  double constante0 = histo -> GetMaximum();
  double mean0 = histo->GetMean();
  double sigma0 = (histo -> FindLastBinAbove(constante0/1.3) - histo -> FindFirstBinAbove(constante0/1.3))/1.8;
  TF1*  gaus(new TF1("gaus","gaus"));
  gaus -> SetRange(coinc_low_fit, coinc_high_fit);
  gaus -> SetParameter(0, constante0);
  gaus -> SetParameter(1, mean0);
  gaus -> SetParameter(2, sigma0);
  histo -> Fit(gaus,"RQ");

  TF1*  gaus_bckg(new TF1("gaus_bckg","gaus(0)+pol1(3)"));
  gaus_bckg -> SetRange(coinc_low_fit, coinc_high_fit);
  gaus_bckg -> SetParameters(gaus->GetParameter(0), gaus->GetParameter(1), gaus->GetParameter(2));
  histo -> Fit(gaus_bckg,"RQ");

  // TF1*  gaus_bckg2(new TF1("gaus_bckg2","gaus(0)+pol2(3)"));
  // gaus_bckg2 -> SetRange(coinc_low_fit, coinc_high_fit);
  // gaus_bckg2 -> SetParameters(gaus_bckg->GetParameter(0), gaus_bckg->GetParameter(1), gaus_bckg->GetParameter(2),
  //                             gaus_bckg->GetParameter(3), gaus_bckg->GetParameter(4));
  // histo -> Fit(gaus_bckg2,"RQ");

  auto const & constante = gaus_bckg->GetParameter(0);
  // auto const & mean = gaus_bckg->GetParameter(1);
  auto const & sigma = gaus_bckg->GetParameter(2);

  auto const & kev_canal = (histo->GetXaxis()->GetXmax() - histo->GetXaxis()->GetXmin()) / histo->GetXaxis()->GetNbins();

  auto const & pe_integral = sqrt_c(2 * 3.141596) * constante * sigma / kev_canal;

  histo->GetXaxis()->UnZoom();

  std::cout << std::setprecision(4);
  print("Efficiency :", nicer_double(histo->GetEntries(), 0), nicer_double(nb_gate, 0), 100.*double(histo->GetEntries()/nb_gate), "% Full-energy :", 100.*double(pe_integral/nb_gate), "%", 
        "realive pe eff : ", 100.*pe_integral/histo->Integral(), "%", histo->GetName());
}

void Co60_efficiency()
{
  SimpleCluster::setDistanceMax(1.1);

  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  int nb_gate = 0;
  int nb_gated = 0;
  int nb_missed = 0;

  TRandom* random = new TRandom(time(0));

  TChain* tree = new TChain("Nuball2");
  // std::string path = Path::home().string()+"nuball2/N-SI-136-root-sources/60Co_3/*";
  // std::string path = Path::home().string()+"nuball2/N-SI-136-root_sources/60Co_2/*";
  std::string path = Path::home().string()+"nuball2/N-SI-136-root_sources/60Co_center_after_calib/*";
  tree->Add(path.c_str());

  print(tree->GetNtrees(), "files found in", path);
  if (tree->GetNtrees()==0) throw_error(concatenate("No files in ", path));

  Event event(tree);
  CloversV2 clovers;
  SimpleParis paris(&calibPhoswitches);
  std::vector<double> rej_Ge;

  auto timing = new TH2F("timing", "timing;[keV]", 1000,0,1000, 500,-250_ns,250_ns);
  auto gated_BGO_vs_index = new TH2F("gated_BGO_vs_index", "gated_BGO_vs_index;[keV]", 24,0,24, 2_k,0_MeV,2_MeV);

  auto spectra_Ge = new TH1F("spectra_Ge", "spectra_Ge;[keV]", 2000, 0, 2000);
  auto spectra_BGO = new TH1F("spectra_BGO", "spectra_BGO;[keV]", 2000, 0, 2000);
  auto spectra_phos = new TH1F("spectra_phos", "spectra_BGO;[keV]", 2000, 0, 2000);
  auto gate_spectra = new TH1F("gate_spectra", "gate_spectra;[keV]", 2000, 0, 2000);

  auto gated_Ge = new TH1F("gated_Ge", "gated_Ge;[keV]", 2000, 0, 2000);
  auto gated_clean_Ge = new TH1F("gated_clean_Ge", "gated_clean_Ge;[keV]", 2000, 0, 2000);
  auto gated_rej_Ge = new TH1F("gated_rej_Ge", "gated_rej_Ge;[keV]", 2000, 0, 2000);

  auto gated_BGO = new TH1F("gated_BGO", "gated_BGO;[keV]", 500, 0, 2000);
  auto gated_BGO_test = new TH1F("gated_BGO_test", "gated_BGO_test;[keV]", 500, 0, 2000);
  auto gated_clean_BGO = new TH1F("gated_clean_BGO", "gated_clean_BGO;[keV]", 500, 0, 2000);
  auto gated_addback_BGO = new TH1F("gated_addback_BGO", "gated_addback_BGO;[keV]", 500, 0, 2000);
  auto gated_only_addback_BGO = new TH1F("gated_only_addback_BGO", "gated_only_addback_BGO;[keV]", 500, 0, 2000);

  auto gated_raw_phos = new TH2F("gated_raw_phos", "gated_raw_phos;[keV]", 1000, 0, 1000, 500, 0, 2000);
  auto gated_phos = new TH1F("gated_phos", "gated_phos;[keV]", 500, 0, 2000);
  auto gated_clean_phos = new TH1F("gated_clean_phos", "gated_clean_phos;[keV]", 500, 0, 2000);
  auto gated_rej_phos = new TH1F("gated_rej_phos", "gated_rej_phos;[keV]", 500, 0, 2000);

  auto gated_paris_module = new TH1F("gated_paris_module", "gated_paris_module;[keV]", 500, 0, 2000);
  auto gated_paris_module_addbacked = new TH1F("gated_paris_module_addbacked", "gated_paris_module_addbacked;[keV]", 500, 0, 2000);

  auto gated_LaBr3 = new TH1F("gated_LaBr3", "gated_LaBr3;[keV]", 500, 0, 2000);
  auto gated_clean_LaBr3 = new TH1F("gated_clean_LaBr3", "gated_clean_LaBr3;[keV]", 500, 0, 2000);
  
  auto gated_calo_VS_MMult = new TH2F("gated_calo_VS_MMult", "gated_calo_VS_MMult;[keV]", 10,0,10, 500,0,2000);
  auto gated_calo = new TH1F("gated_calo", "gated_calo;[keV]", 500, 0, 2000);
  auto gated_caloGe = new TH1F("gated_caloGe", "gated_caloGe;[keV]", 500, 0, 2000);
  auto gated_caloBGO = new TH1F("gated_caloBGO", "gated_caloBGO;[keV]", 500, 0, 2000);
  auto gated_caloClover = new TH1F("gated_caloClover", "gated_caloClover;[keV]", 500, 0, 2000);
  auto gated_caloParis = new TH1F("gated_caloParis", "gated_caloParis;[keV]", 500, 0, 2000);

  std::vector<double> BGO_nrjs;
  for (int evt_i = 0; evt_i<tree->GetEntries(); ++evt_i)
  {
    tree->GetEntry(evt_i);

    clovers.clear();
    paris.clear();
    rej_Ge.clear();
    BGO_nrjs.clear();

    if (evt_i>max_cursor) break;
    for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    {
      auto const & label = event.label(hit_i);
      auto & nrj = event.nrj(hit_i);

      if (nrj < 10_keV) continue;

      if (CloversV2::isBGO(label)) 
      {
        nrj *= 1.11;
        if (CloversV2::index(label) == 2) nrj = 1.352 * nrj;
        BGO_nrjs.push_back(nrj);
      }
      else if (Paris::is[label])
      {
      }

      clovers.fill(event, hit_i);
      paris.fill(event, hit_i);

      if (label == timing_ref_label) for (int hit_j = 0; hit_j<event.mult; ++hit_j)
      {
        if (hit_j == hit_i) continue;
        timing->Fill(event.labels[hit_j], event.times[hit_j] - event.times[hit_i]);
      }
    }

    clovers.analyze();
    paris.analyze();

    for (auto const & id : clovers.BGO_id) spectra_BGO->Fill(clovers[id].nrjBGO);
    for (auto const & phos : paris.phoswitches) spectra_phos->Fill(phos->nrj);

    auto const & MMult = clovers.all.size()+paris.phoswitch_mult(); // Modules multiplicity
    auto const & CMult = clovers.clean.size(); // Clean Ge multiplicity

    for (size_t hit_i = 0; hit_i<CMult; ++hit_i) 
    {
      auto const & nrj_i = clovers.clean[hit_i]->nrj;
      auto const & index_i = clovers.clean[hit_i]->index();
      spectra_Ge->Fill(nrj_i);
      if (gate(gate_low, nrj_i, gate_high))
      {
        double calo = 0;
        double caloGe = 0;
        double caloBGO = 0;
        double caloParis = 0;
        ++nb_gate;

        gate_spectra->Fill(nrj_i);

        if (clovers.all.size() == 1 && paris.phoswitch_mult() == 0)
        {
          ++nb_missed;
          continue;
        }

        // For Clovers :
        for (size_t hit_j = 0; hit_j<clovers.all.size(); ++hit_j)
        {
          auto const & clover_j = *(clovers.all[hit_j]);

          // Ge :
          if (clover_j.nb>0 && clover_j.index() != index_i) 
          {
            calo+=smearGe(clover_j.nrj, random);
            caloGe+=smearGe(clover_j.nrj, random);
            gated_Ge->Fill(clover_j.nrj);
            if (clover_j.isCleanGe())
            {
              gated_clean_Ge->Fill(clover_j.nrj);
              if (gate(coinc_low, clover_j.nrj, coinc_high)) ++nb_gated;
            }
            else gated_rej_Ge->Fill(clover_j.nrj);
          }

          // for(auto const & nrj : rej_Ge) calo+=smearGe(nrj, random);

          // BGO :
          if (clover_j.nbBGO>0)
          {
            gated_BGO_vs_index->Fill(clover_j.index(), clover_j.nrjBGO);
            calo+=clover_j.nrjBGO;
            caloBGO+=clover_j.nrjBGO;
            gated_BGO->Fill(clover_j.nrjBGO);
            gated_addback_BGO->Fill(clover_j.calo);
            if (clover_j.nb == 0) gated_clean_BGO->Fill(clover_j.nrjBGO);
            else gated_only_addback_BGO->Fill(clover_j.calo);
          }
        }

        for (auto const & nrj : BGO_nrjs) gated_BGO_test->Fill(nrj);

        // Paris phoswitches :
        for (auto const & phos : paris.phoswitches) 
        {
          gated_raw_phos->Fill(phos->index(), phos->qlong);
          gated_phos->Fill(phos->nrj);

          calo+=smearParis(phos->nrj, random);
          caloParis+=smearParis(phos->nrj, random);

          if (phos->isLaBr3()) 
          {
            gated_LaBr3->Fill(phos->qshort);
            if (!phos->rejected) gated_clean_LaBr3->Fill(phos->qshort);
          }

          if (!phos->rejected) gated_clean_phos->Fill(phos->nrj);
          else gated_rej_phos->Fill(phos->nrj);
        }

        // Paris modules :
        for (auto const & module : paris.modules)
        {
          gated_paris_module->Fill(module->nrj);
          if (module->nb()>1) gated_paris_module_addbacked->Fill(module->nrj);
        } 

        // Calorimetry :
        auto const & calo_Clover = caloBGO+caloGe;
        if (calo>0) gated_calo_VS_MMult->Fill(MMult-1, calo);
        if (calo>0) gated_calo->Fill(calo);
        if (caloGe>0) gated_caloGe->Fill(caloGe);
        if (caloBGO>0) gated_caloBGO->Fill(caloBGO);
        if (calo_Clover>0) gated_caloClover->Fill(calo_Clover);
        if (caloParis>0) gated_caloParis->Fill(caloParis);
      }
    }
  }
  
  if (nb_gate < 1) print("no gate found !!");
  else print(nb_gate, "gate found, along with", nb_gated, "coincident gamma, which means an absolute efficiency of", 100.*double(nb_gated)/double(nb_gate), "%");
  print("Total efficiency :", 100.*(1-*double(nb_missed)/double(nb_gate)), "%");
  print(nicer_double(max_cursor, 0), "evts read");

  // Efficiency
  auto outfile = TFile::Open("60Co_test.root", "recreate");
  outfile->cd();

    eff(gated_Ge, nb_gate, 1150, 1190);
    eff(gated_clean_Ge, nb_gate, 1150, 1190);
    eff(gated_BGO, nb_gate, 800, 2000);
    eff(gated_BGO_test, nb_gate, 800, 2000);
    eff(gated_clean_BGO, nb_gate, 500, 2000);
    eff(gated_addback_BGO, nb_gate, 500, 2000);
    eff(gated_only_addback_BGO, nb_gate, 500, 2000);
    eff(gated_phos, nb_gate, 900, 1400);
    eff(gated_clean_phos, nb_gate, 900, 1400);
    eff(gated_paris_module, nb_gate, 900, 1400);
    eff(gated_paris_module_addbacked, nb_gate, 500, 2000);
    eff(gated_LaBr3, nb_gate, 1000, 1300);
    eff(gated_clean_LaBr3, nb_gate, 1000, 1300);
    eff(gated_calo, nb_gate, 500, 2000);
    eff(gated_caloGe, nb_gate, 900, 2000);
    eff(gated_caloBGO, nb_gate, 500, 2000);
    eff(gated_caloClover, nb_gate, 500, 2000);
    eff(gated_caloParis, nb_gate, 500, 2000);


    timing->Write();
    gated_BGO_vs_index->Write();

    spectra_Ge->Write();
    spectra_BGO->Write();
    spectra_phos->Write();
    gate_spectra->Write();

    gated_Ge->Write();
    gated_clean_Ge->Write();
    gated_rej_Ge->Write();

    gated_BGO->Write();
    gated_BGO_test->Write();
    gated_clean_BGO->Write();
    gated_addback_BGO->Write();
    gated_only_addback_BGO->Write();

    gated_raw_phos->Write();
    gated_phos->Write();
    gated_clean_phos->Write();
    gated_rej_phos->Write();

    gated_paris_module->Write();
    gated_paris_module_addbacked->Write();

    gated_LaBr3->Write();
    gated_clean_LaBr3->Write();

    gated_calo_VS_MMult->Write();
    gated_calo->Write();
    gated_caloGe->Write();
    gated_caloBGO->Write();
    gated_caloClover->Write();
    gated_caloParis->Write();

  outfile->Close();
  print("60Co_test.root written");
  print(timer());
}

int main()
{
  Co60_efficiency();
  return 1;
}

// g++ -g -o exec Co60_efficiency.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec Co60_efficiency.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17