#include "../../lib/libRoot.hpp"

constexpr int nb_SI = 1.5e6;
constexpr double R = 4;
constexpr std::array<double, 2> energies = {1843, 2122};
constexpr std::array<std::pair<int, int>, 2> E_window = {{{1800, 1860}, {2100, 2160}}};
constexpr std::array<double, 2> eff = {0.0223, 0.0207};
constexpr std::array<double, 2> I_th = {0.39, 0.2};
constexpr std::array<double, 2> I_th_642 = {0.08, 0.15};
constexpr std::array<double, 2> I_th_966 = {0, 0.27};
constexpr double prompt_eff = 0.5;
constexpr double particle_eff = 0.1;

void SimulatedPeaks(std::string filename = "new_new_merge_dC1.root")
{
  auto file = TFile::Open(filename.c_str());
  if (!file) throw_error(filename+ "absent !");

  auto d = file->Get<TH1F>("d");
  auto d_P = file->Get<TH1F>("d_P");
  auto d_VS_PC = file->Get<TH2F>("d_VS_PC");
  auto d_PC3 = d_VS_PC->ProjectionY("d_PC3_", 2, 300);
  // auto d_p = file->Get<TH1F>("d_p");
  // auto d_pP = file->Get<TH1F>("d_pP");
  // auto d_VS_PC_p = file->Get<TH2F>("d_VS_PC_p");
  // auto d_pPC3_p = d_VS_PC_p->ProjectionY("d_pPC3_p", 2, 300);

  auto d_canvas = new TCanvas; d_canvas->Divide(2,1);
  auto d_P_canvas = new TCanvas; d_P_canvas->Divide(2,1);
  auto d_PC3_canvas = new TCanvas; d_PC3_canvas->Divide(2,1);
  // auto d_p_canvas = new TCanvas; d_p_canvas->Divide(2,1);
  // auto d_pP_canvas = new TCanvas; d_pP_canvas->Divide(2,1);
  // auto d_pPC3_canvas = new TCanvas; d_pPC3_canvas->Divide(2,1);

  for (int i = 0; i<energies.size(); ++i)
  {
    auto E = energies[i];
    auto raw_n = nb_SI*eff[i]*I_th[i];
    auto prompt_gate_n = raw_n*prompt_eff;

    d_canvas->cd(i+1);
    d_canvas->SetTitle("raw");
    auto d_bckg = static_cast<TH1F*>(d->Clone(("d"+std::to_string(int(E))).c_str()));
    CoLib::simulate_peak(d_bckg, E, R, raw_n);
    d_bckg->SetLineColor(kRed);
    d_bckg->Draw();
    d->Draw("same");
    d->SetStats(0);
    d_bckg->SetStats(0);
    d_bckg->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);
    d_bckg->GetXaxis()->SetTitle("Delayed");
    d_bckg->SetTitle(("gate "+std::to_string(int(E))).c_str());

    d_P_canvas->cd(i+1);
    d_P_canvas->SetTitle("prompt gate");
    auto d_P_bckg = static_cast<TH1F*>(d_P->Clone(("d_P"+std::to_string(int(E))).c_str()));
    CoLib::simulate_peak(d_P_bckg, E, R, raw_n);
    d_P_bckg->SetLineColor(kRed);
    d_P_bckg->Draw();
    d_P->Draw("same");
    d->SetStats(0);
    d_P_bckg->SetStats(0);
    d_P_bckg->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);
    d_P_bckg->GetXaxis()->SetTitle("Delayed");
    d_P_bckg->SetTitle(("gate "+std::to_string(int(E))).c_str());

    d_PC3_canvas->cd(i+1);
    d_PC3_canvas->SetTitle("PC3 gate");
    auto d_PC3_bckg = static_cast<TH1F*>(d_PC3->Clone(("d_PC3"+std::to_string(int(E))).c_str()));
    CoLib::simulate_peak(d_PC3_bckg, E, R, raw_n);
    d_PC3_bckg->SetLineColor(kRed);
    d_PC3_bckg->Draw();
    d_PC3->Draw("same");
    d->SetStats(0);
    d_PC3_bckg->SetStats(0);
    d_PC3_bckg->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);
    d_PC3_bckg->GetXaxis()->SetTitle("Delayed");
    d_PC3_bckg->SetTitle(("gate "+std::to_string(int(E))).c_str());

    // d_p_canvas->cd(i);
    // CoLib::simulate_peak(d_p, E, R, raw_n, 1);
    // d_p->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);

    // d_pP_canvas->cd(i);
    // CoLib::simulate_peak(d_pP, E, R, prompt_gate_n, 1);
    // d_pP->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);

    // d_pPC3_canvas->cd(i);
    // CoLib::simulate_peak(d_pPC3_p, E, R, prompt_gate_n, 1);
    // d_pPC3_p->GetXaxis()->SetRangeUser(E_window[i].first, E_window[i].second);
  }
}