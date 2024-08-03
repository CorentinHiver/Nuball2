

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

long max_cursor = 20000000;
Time time_window = 50_ns;
int gate_low = 1331;
int gate_high = 1335;
int coinc_low = 1171;
int coinc_high = 1176;

void eff(TH1F* histo, bool isGe = false, int coinc_low_fit = 1000, int coinc_high_fit = 1500)
{
  if (isGe)
  {
    coinc_low_fit = 1150;
    coinc_high_fit = 1190;
  }
  histo->GetXaxis()->SetRangeUser(coinc_low_fit, coinc_high_fit);
  double constante0 = histo -> GetMaximum();
  double mean0 = histo->GetMean();
  double sigma0 = (histo -> FindLastBinAbove(constante0/2) - histo -> FindFirstBinAbove(constante0/2))/2.35;
  TF1*  gaus(new TF1("gaus","gaus"));
  gaus -> SetRange(coinc_low_fit, coinc_high_fit);
  gaus -> SetParameter(0, constante0);
  gaus -> SetParameter(1, mean0);
  gaus -> SetParameter(2, sigma0);
  histo -> Fit(gaus,"RQ");

  TF1*  gaus_bckg(new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)"));
  gaus_bckg -> SetRange(coinc_low_fit, coinc_high_fit);
  gaus_bckg -> SetParameters(gaus->GetParameter(0), gaus->GetParameter(1), gaus->GetParameter(2));
  histo -> Fit(gaus_bckg,"RQ");

  auto const & constante = gaus_bckg->GetParameter(0);
  auto const & mean = gaus_bckg->GetParameter(1);
  auto const & sigma = gaus_bckg->GetParameter(2);

  auto const & integral = sqrt_c(2 * 3.141596) * constante * sigma;

  histo->GetXaxis()->UnZoom();

  print(histo->GetName(), ":", integral, "over", histo->GetEntries(), "->", 100*integral/histo->GetEntries(), "% efficiency");
}

// All the gates are inclusive
void Co60_efficiency()
{
  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  int nb_gate = 0;
  int nb_gated = 0;

  TChain* tree = new TChain("Nuball2");
  tree->Add((Path::home().string()+"nuball2/N-SI-136-sources/60Co_center_after_calib/*").c_str());
  Event event(tree);
  CloversV2 clovers;
  SimpleParis paris(&calibPhoswitches);

  auto spectra = new TH1F("spectra", "spectra;[keV]", 2000, 0, 2000);
  auto gate_spectra = new TH1F("gate_spectra", "gate_spectra;[keV]", 2000, 0, 2000);
  auto gated_Ge = new TH1F("gated_Ge", "gated_Ge;[keV]", 2000, 0, 2000);
  auto gated_clean_Ge = new TH1F("gated_clean_Ge", "gated_clean_Ge;[keV]", 2000, 0, 2000);
  auto gated_rej_Ge = new TH1F("gated_rej_Ge", "gated_rej_Ge;[keV]", 2000, 0, 2000);
  auto gated_BGO = new TH1F("gated_BGO", "gated_BGO;[keV]", 500, 0, 2000);
  auto gated_clean_BGO = new TH1F("gated_clean_BGO", "gated_clean_BGO;[keV]", 500, 0, 2000);
  auto gated_phos = new TH1F("gated_phos", "gated_phos;[keV]", 500, 0, 2000);
  auto gated_clean_phos = new TH1F("gated_clean_phos", "gated_clean_phos;[keV]", 500, 0, 2000);
  auto gated_paris_module = new TH1F("gated_paris_module", "gated_paris_module;[keV]", 500, 0, 2000);
  auto gated_LaBr3 = new TH1F("gated_LaBr3", "gated_LaBr3;[keV]", 500, 0, 2000);
  auto gated_clean_LaBr3 = new TH1F("gated_clean_LaBr3", "gated_clean_LaBr3;[keV]", 500, 0, 2000);

  auto spectra_VS_CMult = new TH2F("spectra_VS_Cmult", "gate_spectra;[keV]", 10,0,10, 2000,0,2000);
  
  for (int evt_i = 0; evt_i<tree->GetEntries(); ++evt_i)
  {
    tree->GetEntry(evt_i);

    clovers.clear();
    paris.clear();

    if (evt_i>max_cursor) break;
    for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    {
      auto const & label = event.label(hit_i);
      if (event.nrj(hit_i) < 20_keV) continue;
      if (CloversV2::isGe(label) && CloversIsBlacklist[label]) continue;
      if (CloversV2::isBGO(label)) event.nrj(hit_i) *= 1.11;
      clovers.fill(event, hit_i);
      paris.fill(event, hit_i);
    }

    clovers.analyze();
    paris.analyze();

    auto const & CMult = clovers.clean.size(); // Clean Ge     multiplicity

    for (size_t hit_i = 0; hit_i<CMult; ++hit_i) 
    {
      auto const & nrj_i = clovers.clean[hit_i]->nrj;
      auto const & index_i = clovers.clean[hit_i]->index();
      spectra->Fill(nrj_i);
      spectra_VS_CMult->Fill(CMult, nrj_i);

      if (gate(gate_low, int(nrj_i), gate_high+1))
      {
        ++nb_gate;

        gate_spectra->Fill(nrj_i);

        // If no other gamma found, fill the underflow of the spectra :
        if (clovers.Ge_id.size()) gated_Ge->Fill(-1);

        if (CMult==1) gated_clean_Ge->Fill(-1);

        if (clovers.BGO_id.size()==0) 
        {
          gated_BGO->Fill(-1);
          gated_clean_BGO->Fill(-1);
        }

        for (size_t hit_j = 0; hit_j<clovers.all.size(); ++hit_j)
        {
          auto const & clover_j = *(clovers.all[hit_j]);

          // Ge :
          if (clover_j.nrj>0 && clover_j.index() != index_i) 
          {
            gated_Ge->Fill(clover_j.nrj);
            if (clover_j.isCleanGe())
            {
              gated_clean_Ge->Fill(clover_j.nrj);
              if (gate(coinc_low, int(clover_j.nrj), coinc_high+1)) ++nb_gated;
            }
            else gated_rej_Ge->Fill(clover_j.nrj);
          }

          // BGO :
          if (clover_j.nrjBGO>0) 
          {
            gated_BGO->Fill(clover_j.nrjBGO);
            if (clover_j.isCleanBGO()) gated_clean_BGO->Fill(clover_j.nrjBGO);
          }
        }

        // Paris phoswitches :
        bool LaBr3_found = false;
        bool clean_phos_found = false;
        if (paris.phoswitches.size() == 0) gated_phos->Fill(-1);
        else for (auto const & phos : paris.phoswitches) 
        {
          gated_phos->Fill(phos->nrj);

          if (phos->isLaBr3()) 
          {
            LaBr3_found = true;
            gated_LaBr3->Fill(phos->qshort);
            if (!phos->rejected) gated_clean_LaBr3->Fill(phos->qshort);
          }

          if (!phos->rejected)
          {
            clean_phos_found = true;
            gated_clean_phos->Fill(phos->nrj);
          }
        }
        if (!LaBr3_found) gated_LaBr3->Fill(-1);
        if (!LaBr3_found) gated_clean_LaBr3->Fill(-1);
        if (!clean_phos_found) gated_clean_phos->Fill(-1);
      
        // Paris modules :
        bool clean_mod_found = false;
        if (paris.modules.size() == 0) gated_paris_module->Fill(-1);
        else for (auto const & module : paris.modules)
        {
          gated_paris_module->Fill(module->nrj);
        } 
      }
    }
  }
  
  if (nb_gate < 1) print("no gate found !!");
  else print(nb_gate, "gate found, along with", nb_gated, "coincident gamma, which means an absolute efficiency of", 100*double(nb_gated)/double(nb_gate), "%");

  // Efficiency
  auto outfile = TFile::Open("60Co_test.root", "recreate");
  outfile->cd();

    spectra->Write();
    gate_spectra->Write();

    gated_Ge->Write();
    eff(gated_Ge, true);
    gated_clean_Ge->Write();
    eff(gated_clean_Ge, true);
    gated_rej_Ge->Write();
    gated_BGO->Write();
    eff(gated_BGO);
    gated_clean_BGO->Write();
    eff(gated_clean_BGO);
    gated_phos->Write();
    eff(gated_phos);
    gated_clean_phos->Write();
    eff(gated_clean_phos);
    gated_paris_module->Write();
    eff(gated_paris_module);
    gated_LaBr3->Write();
    eff(gated_LaBr3);
    gated_clean_LaBr3->Write();
    eff(gated_clean_LaBr3);

    spectra_VS_CMult->Write();

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