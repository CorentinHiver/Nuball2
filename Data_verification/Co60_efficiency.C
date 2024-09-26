

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

#include "../lib/MTObjects/MTRootReader.hpp"
#include "../lib/MTObjects/MultiHist.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

Label timing_ref_label = 252;
Time time_window = 50_ns;
int nb_threads = 10;
int nb_files = -1;

// All the gates are inclusive :
float gate_low = 1331_keV;
float gate_high = 1336_keV;
float coinc_low = 1171_keV;
float coinc_high = 1176_keV;

std::map<Label, double> calibLaBr = {
{301, 1.00342},
{302, 1.00342},
{303, 1.00000},
{304, 0.99660},
{305, 1.01034},
{306, 1.00342},
{307, 1.00687},
{308, 0.99660},
{309, 1.00342},
{310, 0.99322},
{311, 1.00000},
{312, 1.00000},
{313, 1.00342},
{314, 0.98653},
{315, 0.99660},
{316, 0.99660},
{401, 1.00687},
{402, 0.97993},
{403, 0.97667},
{404, 0.97020},
{405, 0.99322},
{406, 0.96700},
{407, 0.97993},
{408, 1.00000},
{409, 0.98986},
{410, 0.97020},
{411, 0.98322},
{412, 0.95752},
{501, 1.01736},
{502, 1.01034},
{503, 0.99660},
{504, 0.99660},
{505, 1.01384},
{506, 1.01384},
{507, 1.01736},
{508, 1.01034},
{601, 1.10150},
{602, 0.79620},
{603, 0.97667},
{604, 0.82303},
{605, 1.00342},
{606, 0.76501},
{607, 1.01384},
{608, 0.99660},
{609, 0.99660},
{610, 0.97993},
{611, 0.98322},
{612, 0.87725},
{613, 0.98986},
{614, 0.95440},
{615, 0.98986},
{616, 0.74745},
{701, 0.98986},
{702, 1.00000},
{703, 0.99660},
{704, 0.93610},
{705, 0.97342},
{706, 0.98986},
{707, 0.95752},
{708, 0.98653},
{709, 0.97020},
{710, 0.99322},
{711, 0.97020},
{712, 1.00687}
};

template <class THist>
void eff(MultiHist<THist> & histo, int nb_gate, int coinc_low_fit = 900, int coinc_high_fit = 1400
//, int coinc_low_fit_bckg = 900, int coinc_high_fit_bckg = 1400
)
{
  histo.Merge();
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

  // PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024_Co.angles");

  std::atomic<int> nb_gate_global(0);
  std::atomic<int> nb_gated_global(0);
  std::atomic<int> nb_missed_global(0);

  TRandom* random = new TRandom(time(0));

  MTObject::Initialise(nb_threads);
  if (nb_files>0) MTObject::adjustThreadsNumber(nb_threads);


  // TChain* tree = new TChain("Nuball2");
  // std::string path = Path::home().string()+"nuball2/N-SI-136-root-sources/60Co_3/*";
  // std::string path = Path::home().string()+"nuball2/N-SI-136-root_sources/60Co_2/*";
  std::string path = Path::home().string()+"nuball2/N-SI-136-root_sources/60Co_center_after_calib/";
  // FilesManager files(path.c_str());
  // MTList files_MT(files.get());
  // tree->Add(path.c_str());

  // print(tree->GetNtrees(), "files found in", path);
  // if (tree->GetNtrees()==0) throw_error(concatenate("No files in ", path));
  // print(files.size(), "files found in", path);
  // if (files.isEmpty()) throw_error(concatenate("No files in ", path));


  MultiHist<TH2F> timing ("timing", "timing;[keV]", 1000,0,1000, 500,-250_ns,250_ns);
  MultiHist<TH2F> gated_BGO_vs_index ("gated_BGO_vs_index", "gated_BGO_vs_index;[keV]", 48,0,48, 500,0_MeV,2_MeV);

  MultiHist<TH1F> spectra_Ge ("spectra_Ge", "spectra_Ge;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> spectra_BGO ("spectra_BGO", "spectra_BGO;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> spectra_phos ("spectra_phos", "spectra_BGO;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> gate_spectra ("gate_spectra", "gate_spectra;[keV]", 2000, 0, 2000);

  MultiHist<TH1F> gated_raw_Ge ("gated_raw_Ge", "gated_raw_Ge;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> gated_Ge ("gated_Ge", "gated_Ge;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> gated_clean_Ge ("gated_clean_Ge", "gated_clean_Ge;[keV]", 2000, 0, 2000);
  MultiHist<TH1F> gated_rej_Ge ("gated_rej_Ge", "gated_rej_Ge;[keV]", 2000, 0, 2000);

  MultiHist<TH1F> gated_BGO ("gated_BGO", "gated_BGO;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_BGO_test ("gated_BGO_test", "gated_BGO_test;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_clean_BGO ("gated_clean_BGO", "gated_clean_BGO;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_addback_BGO ("gated_addback_BGO", "gated_addback_BGO;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_only_addback_BGO ("gated_only_addback_BGO", "gated_only_addback_BGO;[keV]", 500, 0, 2000);

  MultiHist<TH2F> gated_qlong_Phoswitch_VS_label ("gated_qlong_Phoswitch_VS_label", "gated_qlong_Phoswitch_VS_label;[keV]", 1000, 0, 1000, 500, 0, 2000);
  MultiHist<TH2F> gated_Phoswitch_VS_label ("gated_Phoswitch_VS_label", "gated_Phoswitch_VS_label;[keV]", 1000, 0, 1000, 500, 0, 2000);
  MultiHist<TH2F> gated_LaBr_VS_label ("gated_LaBr_VS_label", "gated_LaBr_VS_label;[keV]", 1000, 0, 1000, 500, 0, 2000);
  MultiHist<TH1F> gated_NaI ("gated_NaI", "NaI;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_phos_mix ("gated_phos_mix", "LaBr_{3}+phoswitch;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_phos ("gated_phos", "gated_phos;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_clean_phos ("gated_clean_phos", "gated_clean_phos;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_rej_phos ("gated_rej_phos", "gated_rej_phos;[keV]", 500, 0, 2000);

  MultiHist<TH1F> gated_paris_module ("gated_paris_module", "gated_paris_module;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_paris_module_addbacked ("gated_paris_module_addbacked", "gated_paris_module_addbacked;[keV]", 500, 0, 2000);

  MultiHist<TH1F> gated_LaBr3 ("gated_LaBr3", "gated_LaBr3;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_clean_LaBr3 ("gated_clean_LaBr3", "gated_clean_LaBr3;[keV]", 500, 0, 2000);
  
  MultiHist<TH2F> gated_calo_VS_MMult ("gated_calo_VS_MMult", "gated_calo_VS_MMult;Module Multiplicity;Calorimetry[keV]", 10,0,10, 500,0,2000);
  MultiHist<TH1F> gated_calo_raw ("gated_calo_raw", "gated_calo_raw;Calorimetry[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_calo ("gated_calo", "gated_calo;Calorimetry[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_caloGe ("gated_caloGe", "gated_caloGe;Calorimetry[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_caloBGO ("gated_caloBGO", "gated_caloBGO;Calorimetry[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_caloClover ("gated_caloClover", "gated_caloClover;Calorimetry[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_caloParis ("gated_caloParis", "gated_caloParis;Calorimetry[keV]", 500, 0, 2000);

  // Here I want to get the probability for a gamma ray that interacted first 
  // inside of each kind of detector to get fully absorbed.
  
  MultiHist<TH1F> gated_calo_include_Paris ("gated_calo_include_Paris", "gated_calo_include_Paris;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_calo_include_Ge ("gated_calo_include_Ge", "gated_calo_include_Ge;[keV]", 500, 0, 2000);
  MultiHist<TH1F> gated_calo_include_BGO ("gated_calo_include_BGO", "gated_calo_include_BGO;[keV]", 500, 0, 2000);

  // This section is for measure the time resolution
  MultiHist<TH2F> coinc_LaBr3_E_dT_Ge ("coinc_LaBr3_E_dT_Ge", "coinc_LaBr3_E_dT_Ge;[keV]", 400,-100_ns,100_ns, 10000,0,10000);
  MultiHist<TH2F> coinc_LaBr3_E_dT_BGO ("coinc_LaBr3_E_dT_BGO", "coinc_LaBr3_E_dT_BGO;[keV]", 400,-100_ns,100_ns, 1000,0,10000);
  MultiHist<TH2F> coinc_LaBr3_E_dT_phoswitch ("coinc_LaBr3_E_dT_phoswitch", "coinc_LaBr3_E_dT_phoswitch;[keV]", 400,-100_ns,100_ns, 1000,0,10000);
  MultiHist<TH2F> coinc_LaBr3_E_dT_NaI ("coinc_LaBr3_E_dT_NaI", "coinc_LaBr3_E_dT_NaI;[keV]", 400,-100_ns,100_ns, 1000,0,10000);
  MultiHist<TH2F> coinc_LaBr3_E_dT_LaBr ("coinc_LaBr3_E_dT_LaBr", "coinc_LaBr3_E_dT_LaBr;[keV]", 400,-100_ns,100_ns, 1000,0,10000);


  std::atomic<int> global_evt = -1;
  // for (auto const & file : files)
  MTRootReader reader(path, nb_files);
  reader.read([&](Nuball2Tree & tree, Event & event)
  {
    CloversV2 clovers;
    std::vector<std::pair<Label, double>> BGOs;
    SimpleParis paris(&calibPhoswitches);
    // Nuball2Tree tree(file, event);
    int nb_gate = 0;
    int nb_gated = 0; 
    int nb_missed = 0;  
    int evt_i = 0; 
    for (;evt_i<tree->GetEntries(); ++evt_i)
    {
      tree->GetEntry(evt_i);

      clovers.clear();
      paris.clear();
      BGOs.clear();

      for (int hit_i = 0; hit_i<event.mult; ++hit_i)
      {
        auto const & label = event.label(hit_i);
        auto & nrj = event.nrj(hit_i);

        if (nrj < 10_keV) continue;

        if (CloversV2::isBGO(label)) 
        {
          nrj *= 1.11;
          if (CloversV2::index(label) == 2) nrj = 1.352 * nrj;
          // if (label == ) nrj = 1.352 * nrj;
          BGOs.push_back({label, nrj});
        }
        else if (Paris::is[label])
        {

        }
        else if (CloversV2::isGe(label))
        {
          if (CloversIsBlacklist[label]) continue;
        }
        else if (label == 252)
        {// Calculate time resolution
          auto const & labr_time = event.times[hit_i];
          auto const & labr_nrj = event.nrjs[hit_i];
          
          // Full energy gate :
          if (gate(1130_keV, labr_nrj, 1200_keV) || gate(1300_keV, labr_nrj, 1360_keV)) for (int hit_j = 0; hit_j<event.mult; ++hit_j)
          {
            auto const & label_j = event.labels[hit_j];
            auto const & nrj_j = event.nrjs[hit_j];
            auto const & nrj2_j = event.nrj2s[hit_j];
            auto const & time_j = event.times[hit_j];
            auto const & dT = time_j-labr_time;

            if (label_j == 252) continue;
            else if (CloversV2::isGe(label_j)) coinc_LaBr3_E_dT_Ge.Fill(dT, nrj_j);
            else if (CloversV2::isBGO(label_j)) coinc_LaBr3_E_dT_BGO.Fill(dT, nrj_j);
            else if (Paris::is[label_j])
            {
              auto const & nrjcal = calibPhoswitches.calibrate(label_j, nrj_j, nrj2_j);
              coinc_LaBr3_E_dT_phoswitch.Fill(dT, nrjcal);
              if (Paris::pid_LaBr3(nrj_j, nrj2_j)) coinc_LaBr3_E_dT_LaBr.Fill(dT, nrj_j);
              else if (Paris::pid_good_phoswitch(nrj_j, nrj2_j)) coinc_LaBr3_E_dT_NaI.Fill(dT, nrjcal);
            }
          }
        }

        clovers.fill(event, hit_i);
        paris.fill(event, hit_i);

        if (label == timing_ref_label) for (int hit_j = 0; hit_j<event.mult; ++hit_j)
        {
          if (hit_j == hit_i) continue;
          timing.Fill(event.labels[hit_j], event.times[hit_j] - event.times[hit_i]);
        }
      }

      clovers.analyze();
      for (auto & phos : paris.phoswitches) phos->nrj *= calibLaBr[phos->label];
      paris.analyze();

      for (auto const & id : clovers.BGO_id) spectra_BGO.Fill(clovers[id].nrjBGO);
      for (auto const & phos : paris.phoswitches) spectra_phos.Fill(phos->nrj);

      auto const & MMult = clovers.all.size()+paris.phoswitch_mult(); // Modules multiplicity
      auto const & CMult = clovers.clean.size(); // Clean Ge multiplicity

      for (size_t hit_i = 0; hit_i<CMult; ++hit_i) 
      {
        auto const & clover_i = *(clovers.clean[hit_i]);
        auto const & nrj_i = clover_i.nrj;
        auto const & index_i = clover_i.index();
        spectra_Ge.Fill(nrj_i);
        if (gate(gate_low, nrj_i, gate_high))
        {
          double calo_raw = 0;
          double calo = 0;
          double caloGe = 0;
          double caloBGO = 0;
          double caloParis = 0;
          ++nb_gate;

          gate_spectra.Fill(nrj_i);

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
            if (clover_j.nb>0 && clover_j.index() != index_i && abs(clover_j.time-clover_i.time) < 50_ns) 
            {
              calo_raw+=clover_j.nrj;
              calo+=smearGe(clover_j.nrj, random);
              caloGe+=smearGe(clover_j.nrj, random);
              for (auto const & crystal_id : clover_j.GeCrystalsId) gated_raw_Ge.Fill(clover_j.GeCrystals[crystal_id]);
              gated_Ge.Fill(clover_j.nrj);
              if (clover_j.isCleanGe())
              {
                gated_clean_Ge.Fill(clover_j.nrj);
                if (gate(coinc_low, clover_j.nrj, coinc_high)) ++nb_gated;
              }
              else gated_rej_Ge.Fill(clover_j.nrj);
            }

            // BGO :
            if (clover_j.nbBGO>0)
            {
              calo_raw+=clover_j.nrjBGO;
              calo+=clover_j.nrjBGO;
              caloBGO+=clover_j.nrjBGO;
              gated_BGO.Fill(clover_j.nrjBGO);
              gated_addback_BGO.Fill(clover_j.calo);
              if (clover_j.nb == 0) gated_clean_BGO.Fill(clover_j.nrjBGO);
              else gated_only_addback_BGO.Fill(clover_j.calo);
            }
          }

          for (auto const & bgo : BGOs) 
          {
            auto const & label = bgo.first;
            auto const & nrj = bgo.second;
            gated_BGO_vs_index.Fill((label-23)%6+(label-23)/6, nrj);
            gated_BGO_test.Fill(nrj);
          }

          // Paris phoswitches :
          for (auto const & phos : paris.phoswitches) 
          {
            if (phos->isLaBr3) gated_LaBr_VS_label.Fill(phos->label, phos->nrj);
            else 
            {
              gated_Phoswitch_VS_label.Fill(phos->label, phos->nrj);
              gated_qlong_Phoswitch_VS_label.Fill(phos->label, phos->qlong);
            }
            gated_phos.Fill(phos->nrj);

            calo_raw+=phos->nrj;
            calo+=smearParis(phos->nrj, random);
            caloParis+=smearParis(phos->nrj, random);

            if (phos->isLaBr3) 
            {
              // auto const & nrjcal = phos->qshort*calibLaBr[phos->label];
              gated_LaBr3.Fill(phos->nrj);
              if (!phos->rejected) gated_clean_LaBr3.Fill(phos->nrj);
              gated_phos_mix.Fill(phos->nrj);
            }
            else 
            {
              gated_NaI.Fill(phos->nrj);
              gated_phos_mix.Fill(phos->nrj);
            }

            if (!phos->rejected) gated_clean_phos.Fill(phos->nrj);
            else gated_rej_phos.Fill(phos->nrj);
          }

          // Paris modules :
          for (auto const & module : paris.modules)
          {
            gated_paris_module.Fill(module->nrj);
            if (module->nb()>1) gated_paris_module_addbacked.Fill(module->nrj);
          } 

          // Calorimetry :
          auto const & calo_Clover = caloBGO+caloGe;
          if (calo>0) 
          {
            gated_calo_VS_MMult.Fill(MMult-1, calo);
            gated_calo_raw.Fill(calo_raw);
            gated_calo.Fill(calo);
            if (caloParis > 0) gated_calo_include_Paris.Fill(calo);
            if (caloGe > 0) gated_calo_include_Ge.Fill(calo);
            if (caloBGO > 0) gated_calo_include_BGO.Fill(calo);
          }
          if (caloGe>0) gated_caloGe.Fill(caloGe);
          if (caloBGO>0) gated_caloBGO.Fill(caloBGO);
          if (calo_Clover>0) gated_caloClover.Fill(calo_Clover);
          if (caloParis>0) gated_caloParis.Fill(caloParis);
        }
      }
    }
    global_evt+=evt_i;
    nb_gate_global += nb_gate;
    nb_gated_global += nb_gated;
    nb_missed_global += nb_missed;
  }
  );

  int nb_gate = nb_gate_global.load();
  int nb_gated = nb_gated_global.load();
  int nb_missed = nb_missed_global.load();
  int nb_evts = global_evt.load();

  if (nb_gate < 1) print("no gate found !!");
  else print(nb_gate, "gate found, along with", nb_gated, "coincident gamma, which means an absolute efficiency of", 100.*double(nb_gated)/double(nb_gate), "%");
  print("Total efficiency :", 100.*(1-double(nb_missed)/double(nb_gate)), "%");
  print(nicer_double(nb_evts, 0), "evts read");

  // Efficiency

  eff(gated_raw_Ge, nb_gate, 1150, 1190);
  eff(gated_Ge, nb_gate, 1150, 1190);
  eff(gated_clean_Ge, nb_gate, 1150, 1190);
  eff(gated_BGO, nb_gate, 900, 1600);
  eff(gated_BGO_test, nb_gate, 900, 1600);
  eff(gated_clean_BGO, nb_gate, 900, 1600);
  eff(gated_addback_BGO, nb_gate, 900, 1600);
  eff(gated_only_addback_BGO, nb_gate, 900, 1600);
  eff(gated_NaI, nb_gate, 900, 1400);
  eff(gated_phos_mix, nb_gate, 900, 1400);
  eff(gated_phos, nb_gate, 900, 1400);
  eff(gated_clean_phos, nb_gate, 900, 1400);
  eff(gated_paris_module, nb_gate, 900, 1400);
  eff(gated_paris_module_addbacked, nb_gate, 500, 2000);
  eff(gated_LaBr3, nb_gate, 1000, 1300);
  eff(gated_clean_LaBr3, nb_gate, 1000, 1300);
  eff(gated_calo_raw, nb_gate, 900, 1600);
  eff(gated_calo, nb_gate, 900, 1600);
  eff(gated_caloGe, nb_gate, 900, 2000);
  eff(gated_caloBGO, nb_gate, 700, 1700);
  eff(gated_caloClover, nb_gate, 700, 1700);
  eff(gated_caloParis, nb_gate, 900, 1600);

    auto outfile = TFile::Open("60Co_test.root", "recreate");
  outfile->cd();


    timing.Write();
    gated_BGO_vs_index.Write();

    spectra_Ge.Write();
    spectra_BGO.Write();
    spectra_phos.Write();
    gate_spectra.Write();

    gated_raw_Ge.Write();
    gated_Ge.Write();
    gated_clean_Ge.Write();
    gated_rej_Ge.Write();

    gated_BGO.Write();
    gated_BGO_test.Write();
    gated_clean_BGO.Write();
    gated_addback_BGO.Write();
    gated_only_addback_BGO.Write();

    gated_qlong_Phoswitch_VS_label.Write();
    gated_Phoswitch_VS_label.Write();
    gated_LaBr_VS_label.Write();
    gated_NaI.Write();
    gated_phos_mix.Write();
    gated_phos.Write();
    gated_clean_phos.Write();
    gated_rej_phos.Write();

    gated_paris_module.Write();
    gated_paris_module_addbacked.Write();

    gated_LaBr3.Write();
    gated_clean_LaBr3.Write();

    gated_calo_VS_MMult.Write();
    gated_calo_raw.Write();
    gated_calo.Write();
    gated_caloGe.Write();
    gated_caloBGO.Write();
    gated_caloClover.Write();
    gated_caloParis.Write();

    gated_calo_include_Paris.Write();
    gated_calo_include_Ge   .Write();
    gated_calo_include_BGO  .Write();  

    coinc_LaBr3_E_dT_Ge .Write();
    coinc_LaBr3_E_dT_BGO  .Write();
    coinc_LaBr3_E_dT_phoswitch  .Write();
    coinc_LaBr3_E_dT_NaI  .Write();
    coinc_LaBr3_E_dT_LaBr .Write();

  outfile->Close();
  print("60Co_test.root written");
  print(timer());
}

int main(int argc, char** argv)
{
  if (argc == 2) 
  {
    nb_files =std::stoi(argv[1]);
  }
  else if (argc == 3)
  {
    nb_files =std::stoi(argv[1]);
    nb_threads = std::stoi(argv[2]);
  }
  Co60_efficiency();
  return 1;
}

// g++ -g -o exec Co60_efficiency.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec Co60_efficiency.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17