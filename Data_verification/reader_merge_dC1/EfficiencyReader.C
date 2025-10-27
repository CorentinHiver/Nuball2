#include "../../lib/libRoot.hpp"

std::vector<int> peaks = {205, 244, 279, 301, 309};
int R = 2;
int nb_it = 10;
std::vector<int> peaks_veto = {895, 1546, 1902, 2055, 2973};
int R_veto = 4;
int nb_it_veto = 20;

void EfficiencyReader(std::string filename = "../merge_dC1.root")
{
  auto file = TFile::Open(filename.c_str());
  if (!file) Colib::throw_error(filename+ "absent !");

  auto dd = file->Get<TH2F>("dd");
  auto dd_p = file->Get<TH2F>("dd_p");
  auto dd_P = file->Get<TH2F>("dd_P");
  auto dd_pP = file->Get<TH2F>("dd_pP");
  auto dd_proton = file->Get<TH2F>("dd_pP");

  auto d642 = dd->ProjectionX("d642", 640, 644);
  auto d642_p = dd_p->ProjectionX("d642_p", 640, 644);
  auto d642_P = dd_P->ProjectionX("d642_P", 640, 644);
  auto d642_pP = dd_pP->ProjectionX("d642_pP", 640, 644);

  // 236U

  double mean_eff_p = 0;
  double mean_eff_P = 0;
  double mean_eff_pP = 0;

  for (auto const & peak : peaks) 
  {
    auto eff_p = Colib::peakIntegral(d642_p, peak-R, peak+R, nb_it)/Colib::peakIntegral(d642, peak-R, peak+R, nb_it);
    auto eff_P = Colib::peakIntegral(d642_P, peak-R, peak+R, nb_it)/Colib::peakIntegral(d642, peak-R, peak+R, nb_it);
    auto eff_pP = Colib::peakIntegral(d642_pP, peak-R, peak+R, nb_it)/Colib::peakIntegral(d642, peak-R, peak+R, nb_it);
    print(peak, eff_p, eff_P, eff_pP);
    mean_eff_p += eff_p;
    mean_eff_P += eff_P;
    mean_eff_pP += eff_pP;
  }
  mean_eff_p /= double(peaks.size());
  mean_eff_P /= double(peaks.size());
  mean_eff_pP /= double(peaks.size());
  print ("efficiency p", mean_eff_p);
  print ("efficiency P", mean_eff_P);
  print ("efficiency pP", mean_eff_pP);

  // Veto 142CE

  double mean_eff_p_veto = 0;
  double mean_eff_P_veto = 0;
  double mean_eff_pP_veto = 0;

  for (auto const & peak : peaks_veto) 
  {
    auto eff_p_veto = Colib::peakIntegral(d642_p, peak-R_veto, peak+R_veto, nb_it_veto)/Colib::peakIntegral(d642, peak-R_veto, peak+R_veto, nb_it_veto);
    auto eff_P_veto = Colib::peakIntegral(d642_P, peak-R_veto, peak+R_veto, nb_it_veto)/Colib::peakIntegral(d642, peak-R_veto, peak+R_veto, nb_it_veto);
    auto eff_pP_veto = Colib::peakIntegral(d642_pP, peak-R_veto, peak+R_veto, nb_it_veto)/Colib::peakIntegral(d642, peak-R, peak+R, nb_it_veto);
    print(peak, eff_p_veto, eff_P_veto, eff_pP_veto);
    mean_eff_p_veto += eff_p_veto;
    mean_eff_P_veto += eff_P_veto;
    mean_eff_pP_veto += eff_pP_veto;
  }
  mean_eff_p_veto /= double(peaks_veto.size());
  mean_eff_P_veto /= double(peaks_veto.size());
  mean_eff_pP_veto /= double(peaks_veto.size());
  print ("veto efficiency p", mean_eff_p_veto);
  print ("veto efficiency P", mean_eff_P_veto);
  print ("veto efficiency pP", mean_eff_pP_veto);
}