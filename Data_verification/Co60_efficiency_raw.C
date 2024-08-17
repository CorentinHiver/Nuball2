

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FasterReader.hpp"
#include "../lib/Classes/Alignator.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Modules/Timeshifts.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Timer.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

long max_cursor = 50000000;
Time time_window = 50_ns;
int gate_low = 1331;
int gate_high = 1335;
int coinc_low = 1171;
int coinc_high = 1176;

void eff(TH1F* histo, int nb_gate, bool isGe = false, int coinc_low_fit = 1000, int coinc_high_fit = 1500)
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
  // auto const & mean = gaus_bckg->GetParameter(1);
  auto const & sigma = gaus_bckg->GetParameter(2);

  auto const & kev_canal = (histo->GetXaxis()->GetXmax() - histo->GetXaxis()->GetXmin()) / histo->GetXaxis()->GetNbins();

  auto const & integral = sqrt_c(2 * 3.141596) * constante * sigma / kev_canal;

  histo->GetXaxis()->UnZoom();

  std::cout << std::setprecision(4);
  print("Efficiency :", nicer_double(histo->Integral(), 0), nicer_double(nb_gate, 0), 100.*double(histo->Integral()/nb_gate), "% Full-energy :", 100.*double(integral/nb_gate), "%", histo->GetName());
}

// All the gates are inclusive
void Co60_efficiency_raw()
{
  SimpleCluster::setDistanceMax(2.1);

  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  Calibration calib("../136/136_2024_Co.calib");
  Timeshifts ts("../136/Timeshifts/60Co_after.dT");

  int nb_gate = 0;
  int nb_gated = 0;
  int nb_missed = 0;

  TRandom* random = new TRandom(time(0));

  std::string path = Path::home().string()+"nuball2/N-SI-136-sources/60Co_center_after.fast/*";
  auto files_list = wildcard(path);

  print(files_list.size(), "files found in", path);
  if (files_list.size()==0) throw_error(concatenate("No files in ", path));

  // Event event(tree);
  CloversV2 clovers;
  SimpleParis paris(&calibPhoswitches);
  std::vector<double> rej_Ge;

  auto spectra_Ge = new TH1F("spectra_Ge", "spectra_Ge;[keV]", 2000, 0, 2000);
  auto gate_spectra = new TH1F("gate_spectra", "gate_spectra;[keV]", 2000, 0, 2000);

  auto gated_Ge = new TH1F("gated_Ge", "gated_Ge;[keV]", 2000, 0, 2000);
  auto gated_clean_Ge = new TH1F("gated_clean_Ge", "gated_clean_Ge;[keV]", 2000, 0, 2000);
  auto gated_rej_Ge = new TH1F("gated_rej_Ge", "gated_rej_Ge;[keV]", 2000, 0, 2000);

  auto gated_BGO = new TH1F("gated_BGO", "gated_BGO;[keV]", 500, 0, 2000);
  auto gated_clean_BGO = new TH1F("gated_clean_BGO", "gated_clean_BGO;[keV]", 500, 0, 2000);

  auto gated_phos = new TH1F("gated_phos", "gated_phos;[keV]", 500, 0, 2000);
  auto gated_clean_phos = new TH1F("gated_clean_phos", "gated_clean_phos;[keV]", 500, 0, 2000);
  auto gated_rej_phos = new TH1F("gated_rej_phos", "gated_rej_phos;[keV]", 500, 0, 2000);

  auto gated_paris_module = new TH1F("gated_paris_module", "gated_paris_module;[keV]", 500, 0, 2000);
  auto gated_paris_module_addbacked = new TH1F("gated_paris_module_addbacked", "gated_paris_module_addbacked;[keV]", 500, 0, 2000);

  auto gated_LaBr3 = new TH1F("gated_LaBr3", "gated_LaBr3;[keV]", 500, 0, 2000);
  auto gated_clean_LaBr3 = new TH1F("gated_clean_LaBr3", "gated_clean_LaBr3;[keV]", 500, 0, 2000);
  
  auto gated_calo = new TH1F("gated_calo", "gated_calo;[keV]", 500, 0, 2000);

  int hit_counter = -1;
  for (auto const & filename : files_list)
  {
    Hit hit;
    FasterReader reader(&hit, filename);

    unique_tree temp (new TTree("temp", "temp"));
    temp->SetDirectory(nullptr);

    hit.writing(temp.get(), "ltEQ");

    while(reader.Read())
    {
      // if (++hit_counter>max_cursor) break;
      calib(hit);
      ts(hit);
      temp->Fill();
    }

    Alignator aligned_tree(temp.get(), false);
    hit.clear();  
    hit.reading(aligned_tree);
    aligned_tree.check();

    // aligned_tree.Read();

    // Hit last_hit = hit;
    // // Event event;

    // std::vector<Index> indexes;

    // while(aligned_tree.Read())
    // {
    //   // event.clear();

    //   // event = last_hit;

    //   do
    //   {
    //     // print((hit.stamp - last_hit.stamp)/1000.);
    //     // pauseCo();
    //     if (std::abs(Time_cast(hit.stamp - last_hit.stamp)) < 50_ns) 
    //     {
    //       // event.push_back(hit);
    //     }
    //     else 
    //     {
    //       last_hit = hit;
    //       break;
    //     }
    //   } while(aligned_tree.Read());

    //   clovers.clear();
    //   paris.clear();
    //   rej_Ge.clear();

    //   for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    //   {
    //     auto const & label = event.label(hit_i);
    //     if (event.nrj(hit_i) < 10_keV) continue;
    //     if (CloversV2::isBGO(label)) event.nrj(hit_i) *= 1.11;
    //     clovers.fill(event, hit_i);
    //     paris.fill(event, hit_i);
    //   }

    //   clovers.analyze();
    //   paris.analyze();

    //   auto const & CMult = clovers.clean.size(); // Clean Ge multiplicity

    //   for (size_t hit_i = 0; hit_i<CMult; ++hit_i) 
    //   {
    //     auto const & nrj_i = clovers.clean[hit_i]->nrj;
    //     auto const & index_i = clovers.clean[hit_i]->index();
    //     spectra_Ge->Fill(nrj_i);
    //     if (gate(gate_low-1, int(nrj_i), gate_high+1))
    //     {
    //       double calo = 0;
    //       ++nb_gate;

    //       gate_spectra->Fill(nrj_i);

    //       if (clovers.all.size()+paris.module_mult() == 1)
    //       {
    //         ++nb_missed;
    //         continue;
    //       }

    //       // For Clovers :
    //       for (size_t hit_j = 0; hit_j<clovers.all.size(); ++hit_j)
    //       {
    //         auto const & clover_j = *(clovers.all[hit_j]);

    //         // Ge :
    //         if (clover_j.nb>0 && clover_j.index() != index_i) 
    //         {
    //           calo+=smearGe(clover_j.nrj, random);
    //           gated_Ge->Fill(clover_j.nrj);
    //           if (clover_j.isCleanGe())
    //           {
    //             gated_clean_Ge->Fill(clover_j.nrj);
    //             if (gate(coinc_low-1, int(clover_j.nrj), coinc_high+1)) ++nb_gated;
    //           }
    //           else gated_rej_Ge->Fill(clover_j.nrj);
    //         }

    //         // for(auto const & nrj : rej_Ge) calo+=smearGe(nrj, random);

    //         // BGO :
    //         if (clover_j.nbBGO>0)
    //         {
    //           calo+=clover_j.nrjBGO;
    //           gated_BGO->Fill(clover_j.nrjBGO);
    //           if (clover_j.nb == 0) gated_clean_BGO->Fill(clover_j.nrjBGO);
    //         }
    //       }

    //       // Paris phoswitches :
    //       for (auto const & phos : paris.phoswitches) 
    //       {
    //         gated_phos->Fill(phos->nrj);

    //         calo+=smearParis(phos->nrj, random);

    //         if (phos->isLaBr3()) 
    //         {
    //           gated_LaBr3->Fill(phos->qshort);
    //           if (!phos->rejected) gated_clean_LaBr3->Fill(phos->qshort);
    //         }

    //         if (!phos->rejected) gated_clean_phos->Fill(phos->nrj);
    //         else gated_rej_phos->Fill(phos->nrj);
    //       }

    //       // Paris modules :
    //       bool clean_mod_found = false;
    //       for (auto const & module : paris.modules)
    //       {
    //         gated_paris_module->Fill(module->nrj);
    //         if (module->nb()>1) gated_paris_module_addbacked->Fill(module->nrj);
    //       } 

    //       // Calorimetry :
    //       if (calo>0) gated_calo->Fill(calo);
    //     }
    //   }
    // }
  }

  if (nb_gate < 1) print("no gate found !!");
  else print(nb_gate, "gate found, along with", nb_gated, "coincident gamma, which means an absolute efficiency of", 100.*double(nb_gated)/double(nb_gate), "%");
  print("Total efficiency :", 100.-100.*double(nb_missed)/double(nb_gate), "%");

  // Efficiency
  auto outfile = TFile::Open("60Co_test.root", "recreate");
  outfile->cd();

    eff(gated_Ge, nb_gate, true);
    eff(gated_clean_Ge, nb_gate, true);
    eff(gated_BGO, nb_gate);
    eff(gated_clean_BGO, nb_gate);
    eff(gated_phos, nb_gate);
    eff(gated_clean_phos, nb_gate);
    eff(gated_paris_module, nb_gate);
    eff(gated_paris_module_addbacked, nb_gate);
    eff(gated_LaBr3, nb_gate);
    eff(gated_clean_LaBr3, nb_gate);
    eff(gated_calo, nb_gate, false, 800);


    spectra_Ge->Write();
    gate_spectra->Write();

    gated_Ge->Write();
    gated_clean_Ge->Write();
    gated_rej_Ge->Write();

    gated_BGO->Write();
    gated_clean_BGO->Write();

    gated_phos->Write();
    gated_clean_phos->Write();
    gated_rej_phos->Write();

    gated_paris_module->Write();
    gated_paris_module_addbacked->Write();

    gated_LaBr3->Write();
    gated_clean_LaBr3->Write();
    gated_calo->Write();

  outfile->Close();
  print("60Co_test.root written");
  print(timer());
}

int main()
{
  Co60_efficiency_raw();
  return 1;
}

// g++ -g -o exec Co60_efficiency_raw.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec Co60_efficiency_raw.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -lSpectrum -std=c++17