#include "../../lib/libRoot.hpp"
// for c++17 and higher:

constexpr double R = 4;
constexpr std::array<double, 2> energies = {1843, 2122};
constexpr std::array<double, 2> eff_g = {0.0223, 0.0207};
constexpr std::array<double, 2> I_th = {0.39, 0.2};
constexpr double eff_g_642 = 0.05;
constexpr double eff_g_966 = 0.035;
constexpr std::array<double, 2> I_th_642 = {0.08, 0.15};
constexpr std::array<double, 2> I_th_966 = {0.27, 0};
constexpr double eff_P = 1;
constexpr double eff_p = 0.1;
constexpr double eff_pP = 0.07;

// Determination of nb_SI
constexpr int nb_642_p_measured = 2e6;
constexpr double I_642 = 0.34;
constexpr double I_SI = 3e-4;

constexpr int nb_642_p = nb_642_p_measured/eff_g_642;
constexpr int nb_642 = nb_642_p/eff_p;
constexpr int nb_236U = nb_642/I_642;
constexpr int nb_SI = nb_236U*I_SI;

// Taux de production
constexpr int duration = 3.1e5; // secondes
constexpr double yield_236 = nb_236U/double(duration); // secondes
constexpr double yield_SI = nb_SI/double(duration); // secondes

// template<class THist>
// auto gate_plot(int i, THist * histo, TCanvas * canvas, std::string name, double E, int nb)
auto gate_plot(int i, TH1D * histo, TCanvas * canvas, std::string name, double E, int nb)
{
  auto clone = (TH1D*) (histo->Clone((histo->GetName()+name).c_str()));
  // auto clone = (THist*) (histo->Clone((histo->GetName()+name).c_str()));
  canvas->cd(i+1);
  canvas->SetTitle(name.c_str());
  canvas->SetWindowSize(1000, 500);
  auto histo_bckg = clone->ShowBackground(15);
  Colib::to_int(histo_bckg);
  auto bckg_counts = myIntegralUser(histo_bckg, E-R/2.35*2., E+R/2.35*2.);
  auto data_counts = myIntegralUser(histo, E-R/2.35*2., E+R/2.35*2.);
  auto net_data_count = data_counts-bckg_counts;
  print(name, ": bckg counts =", bckg_counts, "net count : ", net_data_count, "th counts", 
  nb, "-> sigma = ", Colib::nicer_double(net_data_count/std::sqrt(bckg_counts), 2), "sigma th = ", Colib::nicer_double(nb/std::sqrt(bckg_counts), 2));
  // auto histo_bckg = static_cast<TH1F*>(clone->Clone(("d_clean_642g_bckg_"+std::to_string(int(E))).c_str()));
  Colib::simulatePeak(histo_bckg, E, R, nb);
  histo_bckg->SetLineColor(kRed);
  histo_bckg->Draw();
  clone->Draw("same");
  clone->SetStats(0);
  histo_bckg->SetStats(0);
  histo_bckg->GetXaxis()->SetRangeUser(E-10*R, E+10*R);
  clone->GetXaxis()->SetRangeUser(E-10*R, E+10*R);
  histo_bckg->GetXaxis()->SetTitle("Delayed HPGe [keV]");
  histo_bckg->SetTitle((std::to_string(int(E))).c_str());
  pad_set_Y_axis_nice();
  gPad->Update();
}


void SimulatedPeaks(std::string filename = "merge_dC1_V2.root")
{
  print("nb_SI : ", Colib::nicer_double(nb_SI, 3));
  print("yield_236, yield_SI : ", yield_236, yield_SI, "/s");
  print();
  auto file = TFile::Open(filename.c_str());
  if (!file) Colib::throw_error(filename+ "absent !");

  auto d = file->Get<TH1F>("d");
  auto d_P = file->Get<TH1F>("d_P");
  auto d_PC3 = file->Get<TH1F>("d_PC3");
  // auto d_VS_PC = file->Get<TH2F>("d_VS_PC");
  // auto d_PC3 = d_VS_PC->ProjectionY("d_PC3_", 2, 300);
  auto d_VS_DC_VS_PC = file->Get<TH3F>("d_VS_DC_VS_PC");
  auto d_PC3DC3 = (TH1F*)d_VS_DC_VS_PC->ProjectionZ("d_PC3DC3_", 2, 30, 2, 30);
  auto d_prompt_veto = file->Get<TH1F>("d_prompt_veto");
  auto d_clean = Colib::removeVeto(d, d_prompt_veto, 505, 515, "d_clean");

  auto d_p = file->Get<TH1F>("d_p");
  auto d_pP = file->Get<TH1F>("d_pP");
  auto d_VS_PC_p = file->Get<TH2F>("d_VS_PC_p");
  auto d_pPC3 = d_VS_PC_p->ProjectionY("d_pPC3_", 2, 300);
  auto d_p_prompt_veto = file->Get<TH1F>("d_p_prompt_veto");
  auto d_p_clean = Colib::removeVeto(d, d_prompt_veto, 505, 515, "d_clean");


  auto dd = file->Get<TH2F>("dd");
  auto dd_prompt_veto = file->Get<TH2F>("dd_prompt_veto");
  auto dd_clean = Colib::removeVeto(dd, dd_prompt_veto, 505, 515, "dd_clean");
  // CoAnalyse::removeBackground(dd_clean);
  auto d_clean_642g = dd->ProjectionX("d_clean_642g", 640, 644);
  auto d_clean_966g = dd->ProjectionX("d_clean_966g", 963, 968);
  // auto d_clean_642g = dd_clean->ProjectionX("d_clean_642g", 640, 644);
  // auto d_clean_966g = dd_clean->ProjectionX("d_clean_966g", 963, 968);

  auto dd_PC3DC3 = file->Get<TH2F>("dd_PC3DC3");
  // auto dd_prompt_veto = file->Get<TH2F>("dd_prompt_veto");
  // auto dd_clean = Colib::removeVeto(dd_PC3DC3, dd_prompt_veto, 505, 515, "dd_clean");
  // CoAnalyse::removeBackground(dd_clean);
  auto d_PC3DC3_clean_642g = dd_PC3DC3->ProjectionX("d_PC3DC3_clean_642g", 640, 644);
  auto d_PC3DC3_clean_966g = dd_PC3DC3->ProjectionX("d_PC3DC3_clean_966g", 963, 968);

  auto dd_p = file->Get<TH2F>("dd_p");
  // auto dd_p_prompt_veto = file->Get<TH2F>("dd_p_prompt_veto");
  // auto dd_p_clean = Colib::removeVeto(dd_p, dd_p_prompt_veto, 505, 515, "dd_p_clean");
  // CoAnalyse::removeBackground(dd_p_clean);
  auto d_p_clean_642g = dd_p->ProjectionX("d_p_clean_642g", 640, 644);
  auto d_p_clean_966g = dd_p->ProjectionX("d_p_clean_966g", 963, 968);
  // auto d_p_clean_642g = dd_p_clean->ProjectionX("d_p_clean_642g", 640, 644);
  // auto d_p_clean_966g = dd_p_clean->ProjectionX("d_p_clean_966g", 963, 968);

  auto dd_pP = file->Get<TH2F>("dd_pP");
  // auto dd_pP_prompt_veto = file->Get<TH2F>("dd_pP_prompt_veto");
  // auto dd_pP_clean = Colib::removeVeto(dd_pP, dd_pP_prompt_veto, 505, 515);
  // CoAnalyse::removeBackground(dd_pP);
  auto d_pP_clean_642g = dd_pP->ProjectionX("d_pP_clean_642g", 640, 644);
  auto d_pP_clean_966g = dd_pP->ProjectionX("d_pP_clean_966g", 963, 968);
  // auto d_pP_clean_642g = dd_pP_clean->ProjectionX("d_pP_clean_642g", 640, 644);
  // auto d_pP_clean_966g = dd_pP_clean->ProjectionX("d_pP_clean_966g", 963, 968);


  auto d_canvas = new TCanvas; d_canvas->Divide(2,1);
  auto d_P_canvas = new TCanvas; d_P_canvas->Divide(2,1);
  auto d_PC3_canvas = new TCanvas; d_PC3_canvas->Divide(2,1);
  auto d_PC3DC3_canvas = new TCanvas; d_PC3DC3_canvas->Divide(2,1);
  auto d_PC3DC3_clean_canvas = new TCanvas; d_PC3DC3_clean_canvas->Divide(2,1);

  auto d_p_canvas = new TCanvas; d_p_canvas->Divide(2,1);
  auto d_pP_canvas = new TCanvas; d_pP_canvas->Divide(2,1);
  auto d_pPC3_canvas = new TCanvas; d_pPC3_canvas->Divide(2,1);
  
  auto d_clean_642g_canvas = new TCanvas; d_clean_642g_canvas->Divide(2,1);
  auto d_clean_966g_canvas = new TCanvas; d_clean_966g_canvas->Divide(2,1);
  auto d_p_clean_642g_canvas = new TCanvas; d_p_clean_642g_canvas->Divide(2,1);
  auto d_p_clean_966g_canvas = new TCanvas; d_p_clean_966g_canvas->Divide(2,1);
  auto d_PC3DC3_clean_642g_canvas = new TCanvas; d_PC3DC3_clean_642g_canvas->Divide(2,1);
  auto d_PC3DC3_clean_966g_canvas = new TCanvas; d_PC3DC3_clean_966g_canvas->Divide(2,1);

  int i = 0; 

  auto singles_plot = [&](TH1* histo, TCanvas * canvas, std::string name, double E, int nb)
  {
    canvas->cd(i+1);
    canvas->SetTitle(name.c_str());
    canvas->SetWindowSize(1000, 500);
    auto histo_bckg = histo->ShowBackground(15);
    to_int(histo_bckg);
    auto bckg_counts = myIntegralUser(histo_bckg, E-R/2.35*2., E+R/2.35*2.);
    auto data_counts = myIntegralUser(histo, E-R/2.35*2., E+R/2.35*2.);
    auto net_data_count = data_counts-bckg_counts;
    print(name, ": bckg counts =", bckg_counts, "net count : ", net_data_count, "th counts", 
      nb, "-> sigma = ", Colib::nicer_double(net_data_count/std::sqrt(bckg_counts), 2), "sigma th = ", Colib::nicer_double(nb/std::sqrt(bckg_counts), 2));
    // auto histo_bckg = static_cast<TH1F*>(histo->Clone(("d_bckg_"+std::to_string(int(E))).c_str()));
    Colib::simulatePeak(histo_bckg, E, R, nb);
    histo_bckg->SetLineColor(kRed);
    histo_bckg->Draw();
    histo->Draw("same");
    histo->SetStats(0);
    histo_bckg->SetStats(0);
    histo_bckg->GetXaxis()->SetRangeUser(int(E-10*R), int(E+10*R));
    histo->GetXaxis()->SetRangeUser(int(E-10*R), int(E+10*R));
    histo_bckg->GetXaxis()->SetTitle("Delayed HPGe [keV]");
    histo_bckg->SetTitle((std::to_string(int(E))).c_str());
    pad_set_Y_axis_nice();
    gPad->Update();
    histo->GetXaxis()->UnZoom();
  };

  for (; i<energies.size(); ++i)
  {
    auto E = energies[i];
    int raw_n = nb_SI*eff_g[i]*I_th[i];
    int n_P = raw_n*eff_P;
    int n_p = raw_n*eff_p;
    int n_pP = raw_n*eff_pP;

    print();
    print("E = ", int(E), "raw_n", raw_n, "n_P", n_P, "n_p", n_p, "n_pP", n_pP);

    singles_plot(d, d_canvas, "raw", E, raw_n);
    singles_plot(d_P, d_P_canvas, "P condition", E, n_P);
    singles_plot(d_PC3, d_PC3_canvas, "PC3 condition", E, n_P);
    singles_plot(d_PC3DC3, d_PC3DC3_canvas, "P3DC3 condition", E, n_P);
    singles_plot(d_clean, d_PC3DC3_clean_canvas, "P3DC3 veto clean condition", E, n_P);
    singles_plot(d_p, d_p_canvas, "p condition", E, n_pP);
    singles_plot(d_pP, d_pP_canvas, "pP condition", E, n_pP);
    singles_plot(d_pPC3, d_pPC3_canvas, "pPC3 condition", E, n_pP);

    int nb_gate_642 = nb_SI*I_th_642[i]*eff_g_642*eff_g[i]*eff_P;
    int nb_gate_966 = nb_SI*I_th_966[i]*eff_g_966*eff_g[i]*eff_P;
    int nb_gate_642_p = nb_SI*I_th_642[i]*eff_g_642*eff_g[i]*eff_pP;
    int nb_gate_966_p = nb_SI*I_th_966[i]*eff_g_966*eff_g[i]*eff_pP;

    print("nb_gate_642", nb_gate_642, "nb_SI", nb_SI, "I_th_642[i]", I_th_642[i], "eff_g_642", eff_g_642, "eff_g[i]", eff_g[i], "eff_P", eff_P);
    print("nb_gate_966", nb_gate_966, "nb_SI", nb_SI, "I_th_966[i]", I_th_966[i], "eff_g_966", eff_g_966, "eff_g[i]", eff_g[i], "eff_P", eff_P);
    print("642g", nb_gate_642,"966g", nb_gate_966,"642g_p", nb_gate_642_p,"966g_p", nb_gate_966_p);

    gate_plot(i, d_clean_642g, d_clean_642g_canvas, "642 gate", E, nb_gate_642);
    gate_plot(i, d_clean_966g, d_clean_966g_canvas, "966 gate", E, nb_gate_966);
    gate_plot(i, d_PC3DC3_clean_642g, d_PC3DC3_clean_642g_canvas, "PC3DC3 642 gate", E, nb_gate_642);
    gate_plot(i, d_PC3DC3_clean_966g, d_PC3DC3_clean_966g_canvas, "PC3DC3 966 gate", E, nb_gate_966);
    gate_plot(i, d_p_clean_642g, d_p_clean_642g_canvas, "p 642 gate", E, nb_gate_642_p);
    gate_plot(i, d_p_clean_966g, d_p_clean_966g_canvas, "p 966 gate", E, nb_gate_966_p);
  }
}