#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Analyse/WarsawDSSD.hpp"
#include "../lib/Classes/Timer.hpp"
#include "CoefficientCorrection.hpp"
#include "Utils.h"

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((300.0/sqrt(nrj))/100.0)/2.35);
}

constexpr static bool kCalibGe = true;

static std::vector<Label> blacklist = {501};

void macro5(int nb_files = -1, double nb_hits_read = -1, int nb_threads = 10)
{
  if (nb_hits_read<0) nb_hits_read = 1.e+50;
  std::string target = "Th";
  // std::string target = "U";
  // std::string trigger = "P";
  std::string trigger = "dC1";
  // std::string trigger = "C2";

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  MTObject::Initialise();

  detectors.load("../136/index_129.list");

  bool make_triple_coinc_ddd = false;
  bool make_triple_coinc_dpp = false;
  bool make_triple_coinc_ppp = false;
  bool bidim_by_run = false;

  // If too much bidims, need to reduce the number of threads
  if (make_triple_coinc_ddd || make_triple_coinc_dpp || make_triple_coinc_ppp) nb_threads = 8;
  if (bidim_by_run) nb_threads = 5;

  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_evts_number(0);
  std::vector<double> time_runs(1000, 0);
  double total_time_of_beam_s = 0;
  int freq_hit_display = (nb_hits_read > 1.e+100) ? 1.e+7 : nb_hits_read/10;

  // Calibration calibNaI("../136/coeffs_NaI.calib");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  CoefficientCorrection calibGe("../136/GainDriftCoefficients.dat");
  // ExcitationEnergy Ex("../136/dssd_table.dat");
  ExcitationEnergy Ex("../136/Excitation_energy", "U5", "d", "p", "10umAl");

  std::mutex init_mutex;
  std::mutex write_mutex;

  static constexpr Time bidimTimeWindow = 40_ns;

  static constexpr size_t gate_bin_size = 2; // Take 2 keV
  static constexpr std::array<int, 25> ddd_gates = {99, 104, 205, 222, 244, 279, 301, 309, 642, 688, 699, 873, 885, 903, 912, 921, 942, 958, 966, 991, 1750, 1836, 1846, 2115, 2125}; // keV
  static constexpr std::array<int, 25> dpp_gates = {99, 104, 205, 222, 244, 279, 301, 309, 352, 642, 688, 699, 873, 903, 912, 921, 942, 958, 966, 991, 1750, 1836, 1846, 2115, 2125}; // keV
  static constexpr std::array<int, 16> ppp_gates = {99, 104, 205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991}; // keV

  static auto constexpr ddd_gate_bin_max = maximum(ddd_gates)+gate_bin_size+1;
  static auto constexpr ddd_gate_lookup = LUT<ddd_gate_bin_max> ([](int bin){
    for (auto const & gate : ddd_gates) if (abs_const(gate-bin)<3) return true;
    return false;
  });
  static auto constexpr ddd_id_gate_lkp = LUT<ddd_gate_bin_max> ([&](int bin){
    if (ddd_gate_lookup[bin])
    {
      for (size_t i = 0; i<ddd_gates.size(); ++i) if (abs_const(ddd_gates[i] - bin)<3) return int(i);
      return 0;
    }
    else return 0;
  });
  
  static auto constexpr dpp_gate_bin_max = maximum(dpp_gates)+gate_bin_size+1;
  static auto constexpr dpp_gate_lookup = LUT<dpp_gate_bin_max> ([](int bin){
    for (auto const & gate : dpp_gates) if (abs_const(gate-bin)<3) return true;
    return false;
  });
  static auto constexpr dpp_id_gate_lkp = LUT<dpp_gate_bin_max> ([&](int bin){
    if (dpp_gate_lookup[bin])
    {
      for (size_t i = 0; i<dpp_gates.size(); ++i) if (abs_const(dpp_gates[i] - bin)<3) return int(i);
      return 0;
    }
    else return 0;
  });

  static auto constexpr ppp_gate_bin_max = maximum(ppp_gates)+gate_bin_size+1;
  static auto constexpr ppp_gate_lookup = LUT<ppp_gate_bin_max> ([](int bin){
    for (auto const & gate : ppp_gates) if (abs_const(gate-bin)<3) return true;
    return false;
  });
  static auto constexpr ppp_id_gate_lkp = LUT<ppp_gate_bin_max> ([&](int bin){
    if (ppp_gate_lookup[bin])
    {
      for (size_t i = 0; i<ppp_gates.size(); ++i) if (abs_const(ppp_gates[i] - bin)<3) return int(i);
      return 0;
    }
    else return 0;
  });

  std::vector<double> run_times(200,0);
  
  MTObject::parallelise_function([&](){

    TRandom* random = new TRandom();
    random->SetSeed(time(0));

    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    auto const & thread_i_str = std::to_string(thread_i);
    while(MTfiles.getNext(file))
    {
      auto const & filename = removePath(file);
      print(file);
      auto const & run_name = removeExtension(filename);
      auto const & run_name_vector = split(run_name, '_');
      auto const & run_number_str = run_name_vector[1];
      int const & run_number = std::stoi(run_number_str);
      if (target == "Th" && run_number>72) continue;
      if (target == "U" && run_number<74) continue;
      Nuball2Tree tree(file.c_str());
      if (!tree) continue;
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);

      int nb_bins_Ge_singles = 10000;
      int max_bin_Ge_singles = 10000;
      int nb_bins_Ge_bidim = 4096;
      double max_bin_Ge_bidim = 4096;

      // Simple spectra :
      std::unordered_map<std::string, unique_TH2F> T_vs_run;
      std::unordered_map<std::string, unique_TH2F> E_vs_run;
      auto constexpr binning_E = LUT<1000>([](Label const & label){
             if (CloversV2::isBGO(label)) return 500;
        else if (CloversV2::isGe (label)) return 10000;
        else if (Paris::is       [label]) return 500;
        else                              return 500;
      });
      if (bidim_by_run) for (size_t label = 0; label<detectors.names().size(); ++label)
      {
        std::string name = detectors.names()[label];
        if (name == "") continue;
        T_vs_run.emplace(name, unique_TH2F(new TH2F(("T_vs_run_"+name+"_"+thread_i_str).c_str(),("T vs run "+name).c_str(), 100,50,150, 300,-50_ns,250_ns)));
        E_vs_run.emplace(name, unique_TH2F(new TH2F(("E_vs_run_"+name+"_"+thread_i_str).c_str(),("E vs run "+name).c_str(), 100,50,150, binning_E[label],0,10000)));
      }

      unique_TH2F timestamp_hist_VS_run (new TH2F(("timestamp_hist_VS_run_"+thread_i_str).c_str(), "timestamp_hist_VS_run", 100,0,3, 100,50,150));

      unique_TH2F Fatima_E_dT (new TH2F(("Fatima_E_dT_"+thread_i_str).c_str(), "Fatima_E_dT", 800,-100_ns,300_ns, 1000,0,10000));
      unique_TH1F p_before_correction (new TH1F(("p_before_correction"+thread_i_str).c_str(), "prompt before correction", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_before_correction (new TH1F(("d_before_correction"+thread_i_str).c_str(), "delayed before correction", nb_bins_Ge_singles,0,max_bin_Ge_singles));

      unique_TH1F n (new TH1F(("n_"+thread_i_str).c_str(), "neutrons", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p (new TH1F(("p_"+thread_i_str).c_str(), "prompt", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp (new TH2F(("pp_"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F pp_bckg (new TH2F(("pp_bckg_"+thread_i_str).c_str(), "gamma-gamma prompt background;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d (new TH1F(("d_"+thread_i_str).c_str(), "delayed", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_prompt_veto (new TH1F(("d_prompt_veto_"+thread_i_str).c_str(), "d_prompt_veto", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd (new TH2F(("dd_"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dd_bckg (new TH2F(("dd_bckg_"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_prompt_veto (new TH2F(("dd_prompt_veto_"+thread_i_str).c_str(), "gamma-gamma delayed prompt veto;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_mega_prompt_veto (new TH2F(("dd_mega_veto_"+thread_i_str).c_str(), "gamma-gamma delayed prompt mega veto;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp (new TH2F(("dp_"+thread_i_str).c_str(), "delayed VS prompt;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp_particle_veto (new TH2F(("dp_particle_veto_"+thread_i_str).c_str(), "delayed VS prompt  particle veto;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH3F ddt (new TH3F(("ddt_"+thread_i_str).c_str(), "time_{delayed} VS delayed - delayed ;E_{prompt} [keV];E_{delayed} [keV];t_{delayed} [ns]", nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, 14,40_ns,180_ns));
      unique_TH3F ddt_veto (new TH3F(("ddt_veto"+thread_i_str).c_str(), "time_{delayed} VS delayed - delayed prompt veto;E_{prompt} [keV];E_{delayed} [keV];t_{delayed} [ns]", nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, 14,40_ns,180_ns));
      unique_TH3F dpt (new TH3F(("dpt"+thread_i_str).c_str(), "time_{delayed} VS delayed VS prompt ;E_{prompt} [keV];E_{delayed} [keV];t_{delayed} [ns]", nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, 14,40_ns,180_ns));
      unique_TH3F dpt_veto (new TH3F(("dpt_veto"+thread_i_str).c_str(), "time_{delayed} VS delayed VS prompt ;E_{prompt} [keV];E_{delayed} [keV];t_{delayed} [ns]", nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, nb_bins_Ge_bidim/2,0,max_bin_Ge_bidim, 14,40_ns,180_ns));
      unique_TH2F E_dT (new TH2F(("E_dT_"+thread_i_str).c_str(), "E_dT clean", 600,-100_ns,200_ns, 20000,0,20000));
      unique_TH2F E_dT_phoswitch (new TH2F(("E_dT_phoswitch"+thread_i_str).c_str(), "E_dT_phoswitch clean", 600,-100_ns,200_ns, 2000,0,10000));
      unique_TH2F p_VS_dT_LaBr3 (new TH2F(("p_VS_dT_LaBr3"+thread_i_str).c_str(), "p_VS_dT_LaBr3", 300,-100_ns,200_ns, 10000,0,10000));
      
      unique_TH1F dT_642_VS_205 (new TH1F(("dT_642_VS_205_"+thread_i_str).c_str(), "dT_642_VS_205;T(642)-T(205) [ns]", 100,-100_ns,100_ns));
      unique_TH1F dT_642_VS_279 (new TH1F(("dT_642_VS_279_"+thread_i_str).c_str(), "dT_642_VS_279;T(642)-T(279) [ns]", 100,-100_ns,100_ns));
      unique_TH1F dT_642_VS_309 (new TH1F(("dT_642_VS_309_"+thread_i_str).c_str(), "dT_642_VS_309;T(642)-T(309) [ns]", 100,-100_ns,100_ns));
      unique_TH1F dT_642_VS_244 (new TH1F(("dT_642_VS_244_"+thread_i_str).c_str(), "dT_642_VS_244;T(642)-T(244) [ns]", 100,-100_ns,100_ns));
      unique_TH1F dT_642_VS_301 (new TH1F(("dT_642_VS_301_"+thread_i_str).c_str(), "dT_642_VS_301;T(642)-T(301) [ns]", 100,-100_ns,100_ns));
      unique_TH1F dT_205_VS_104__d642 (new TH1F(("dT_205_VS_104__d642_"+thread_i_str).c_str(), "dT_205_VS_104__d642", 50,-100_ns,100_ns));
      unique_TH2F dT_642_VS_d (new TH2F(("dT_642_VS_d_"+thread_i_str).c_str(), "dT_642_VS_d;d[keV];T(642)-T(d) [ns]", 1000,0,1000, 1000,-50_ns,50_ns));
      unique_TH2F dT_642_Clover_VS_dLabr3 (new TH2F(("dT_642_Clover_VS_dLabr3_"+thread_i_str).c_str(), "dT_642_Clover_VS_dLabr3;d[keV];T(642)-T(d) [ns]", 1000,0,1000, 1000,-50_ns,50_ns));
      unique_TH2F dT_642_VS_dLabr3 (new TH2F(("dT_642_VS_dLabr3_"+thread_i_str).c_str(), "dT_642_VS_dLabr3;d[keV];T(642)-T(d) [ns]", 1000,0,1000, 1000,-50_ns,50_ns));

      unique_TH1F d_sumC2 (new TH1F(("d_sumC2_"+thread_i_str).c_str(), "d_sumC2", 2*nb_bins_Ge_singles,0,2*max_bin_Ge_singles));
      unique_TH1F d_sumC2_prompt_veto (new TH1F(("d_sumC2_prompt_veto_"+thread_i_str).c_str(), "d_sumC2_prompt_veto", 2*nb_bins_Ge_singles,0,2*max_bin_Ge_singles));
      unique_TH2F d_VS_sumC2 (new TH2F(("d_VS_sumC2_"+thread_i_str).c_str(), "d_VS_sumC2", 2*nb_bins_Ge_bidim,0,2*max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sumC2_prompt_veto (new TH2F(("d_VS_sumC2_prompt_veto_"+thread_i_str).c_str(), "d_VS_sumC2_prompt_veto", 2*nb_bins_Ge_bidim,0,2*max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sumC2_p (new TH2F(("d_VS_sumC2_p_"+thread_i_str).c_str(), "d_VS_sumC2_p", 2*nb_bins_Ge_bidim,0,2*max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sumC2_p_prompt_veto (new TH2F(("d_VS_sumC2_p_prompt_veto_"+thread_i_str).c_str(), "d_VS_sumC2_p_prompt_veto", 2*nb_bins_Ge_bidim,0,2*max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));


      std::vector<std::unique_ptr<TH2I>> ddd_gated;
      if (make_triple_coinc_ddd) for (auto const & _gate : ddd_gates) ddd_gated.push_back(std::unique_ptr<TH2I>(new TH2I(concatenate("ddd_gated_on_", _gate, "_", thread_i, "_delayed").c_str(), 
                                        concatenate("gamma-gamma delayed gated on delayed ", _gate, "keV;delayed [keV];delayed [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim)));

      std::vector<std::unique_ptr<TH2I>> dpp_gated;
      if (make_triple_coinc_dpp) for (auto const & _gate : dpp_gates) dpp_gated.push_back(std::unique_ptr<TH2I>(new TH2I(concatenate("dpp_gated_on_", _gate, "_", thread_i, "_delayed").c_str(),
                                        concatenate("gamma-gamma prompt gated on delayed ", _gate, "keV;prompt [keV];prompt [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim)));
      std::vector<TH2I*> ppp_gated;
      if (make_triple_coinc_ppp) for (auto const & _gate : ppp_gates) ppp_gated.push_back(new TH2I(concatenate("ppp_gated_on_", _gate, "_", thread_i, "_prompt").c_str(), 
                                        ("gamma-gamma prompt gated on prompt"+std::to_string(_gate)+";prompt [keV];prompt [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      // General codes : P = prompt, D = delayed, p = particle, M multiplicity, C calorimetry, n = no 
      // Multiplicity : (codes : PM = prompt multiplicity ; DM = delayed multiplicity)
      unique_TH1F p_D (new TH1F(("p_D"+thread_i_str).c_str(), "prompt with delayed; mult", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_P (new TH1F(("d_P"+thread_i_str).c_str(), "delayed with prompt; mult", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_P (new TH2F(("dd_P_"+thread_i_str).c_str(), "gamma-gamma delayed with prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_particle_veto (new TH2F(("dd_nop_"+thread_i_str).c_str(), "gamma-gamma delayed particle veto;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F p_mult (new TH1F(("p_mult_"+thread_i_str).c_str(), "prompt multiplicity; mult", 20,0,20));
      unique_TH1F d_mult (new TH1F(("d_mult"+thread_i_str).c_str(), "delayed multiplicity; mult", 20,0,20));
      unique_TH2F dp_mult (new TH2F(("dp_mult"+thread_i_str).c_str(), "delayed VS prompt multiplicity; prompt mult; delayed mult", 20,0,20, 20,0,20));
      unique_TH2F p_VS_PM (new TH2F(("p_VS_PM_"+thread_i_str).c_str(), "prompt Ge VS prompt multiplicity;prompt mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM (new TH2F(("d_VS_PM_"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity;prompt mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_DM (new TH2F(("p_VS_DM_"+thread_i_str).c_str(), "prompt Ge VS delayed multiplicity;delayed mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM (new TH2F(("d_VS_DM_"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity;delayed mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_PM4DM4 (new TH2F(("dd_PM4DM4_"+thread_i_str).c_str(), "gamma-gamma delayed with prompt multiplicity < 4 and delayed multiplicity < 4;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      // Calorimetry : (codes : DC = prompt calorimetry ; DC = delayed calorimetry)
      unique_TH1F p_calo (new TH1F(("prompt_calorimetry_"+thread_i_str).c_str(), "prompt calorimetry;Prompt calorimetry[keV]", 2000,0,20000));
      unique_TH1F d_calo (new TH1F(("delayed_calorimetry_"+thread_i_str).c_str(), "delayed calorimetry;Delayed calorimetry[keV]", 2000,0,20000));
      unique_TH1F p_calo_clover (new TH1F(("prompt_calorimetry_clover_"+thread_i_str).c_str(), "prompt calorimetry clover;Prompt calorimetry[keV]", 2000,0,20000));
      unique_TH1F d_calo_clover (new TH1F(("delayed_calorimetry_clover_"+thread_i_str).c_str(), "delayed calorimetry clover;Delayed calorimetry[keV]", 2000,0,20000));
      unique_TH1F p_calo_phoswitch (new TH1F(("prompt_calorimetry_paris_"+thread_i_str).c_str(), "prompt calorimetry paris;Prompt calorimetry[keV]", 2000,0,20000));
      unique_TH1F d_calo_phoswitch (new TH1F(("delayed_calorimetry_paris_"+thread_i_str).c_str(), "delayed calorimetry paris;Delayed calorimetry[keV]", 2000,0,20000));
      unique_TH2F p_calo_clover_VS_p_calo_phoswitch (new TH2F(("p_calo_clover_VS_p_calo_paris_"+thread_i_str).c_str(), "prompt calorimetry clover VS prompt calorimetry paris;Prompt calorimetry Paris[keV];Prompt calorimetry Clovers[keV]", 1000,0,20000, 1000,0,20000));
      unique_TH2F d_calo_clover_VS_d_calo_phoswitch (new TH2F(("d_calo_clover_VS_d_calo_phoswitch_"+thread_i_str).c_str(), "prompt calorimetry clover VS delayed calorimetry paris;Delayed calorimetry Paris[keV];Delayed calorimetry Clovers[keV]", 1000,0,20000, 1000,0,20000));
      unique_TH2F dp_calo (new TH2F(("prompt_VS_DC_"+thread_i_str).c_str(), "delayed calorimetry VS prompt calorimetry;Delayed calorimetry[keV];Prompt calorimetry[keV]", 1000,0,10000, 1000,0,10000));
      unique_TH2F d_VS_PC (new TH2F(("d_VS_PC_"+thread_i_str).c_str(), "delayed Ge VS prompt calorimetry;Prompt calorimetry[keV];E[keV]", 1000,0,10000, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DC (new TH2F(("d_VS_DC_"+thread_i_str).c_str(), "delayed Ge VS delayed calorimetry;Delayed calorimetry[keV];E[keV]", 1000,0,10000, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_p_VS_PC (new TH3F(("d_VS_p_VS_PC"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity VS prompt calorimetry;Prompt calorimetry[keV];Prompt Ge [keV];Delayed Ge[keV]", 100,0,10000, nb_bins_Ge_bidim/4,0,max_bin_Ge_bidim/2, nb_bins_Ge_bidim/4,0,max_bin_Ge_bidim/2));
      unique_TH3F d_VS_PM_VS_PC (new TH3F(("d_VS_PM_VS_PC"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity VS prompt calorimetry;Prompt calorimetry[keV];Prompt multiplicity;delayed Ge[keV]", 100,0,10000, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DM_VS_DC (new TH3F(("d_VS_DM_VS_DC"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity VS delayed calorimetry;Delayed calorimetry[keV];Delayed multiplicity;delayed Ge[keV]", 100,0,10000, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DM_VS_PM (new TH3F(("d_VS_DM_VS_PM"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity VS prompt multiplicity;Prompt multiplicity;Delayed multiplicity;delayed Ge[keV]", 20,0,20, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DC_VS_PC (new TH3F(("d_VS_DC_VS_PC"+thread_i_str).c_str(), "delayed Ge VS delayed calorimetry VS prompt calorimetry;Prompt calorimetry[keV];Delayed calorimetry[keV];delayed Ge[keV]", 100,0,10000, 100,0,10000, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_p_VS_PC_particleveto (new TH3F(("d_VS_p_VS_PC_particleveto"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity VS prompt calorimetry particle veto;Prompt calorimetry[keV];Prompt Ge [keV];Delayed Ge[keV]", 100,0,10000, nb_bins_Ge_bidim/4,0,max_bin_Ge_bidim/2, nb_bins_Ge_bidim/4,0,max_bin_Ge_bidim/2));
      unique_TH3F d_VS_PM_VS_PC_particleveto (new TH3F(("d_VS_PM_VS_PC_particleveto"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity VS prompt calorimetry particle veto;Prompt calorimetry[keV];Prompt multiplicity;delayed Ge[keV]", 100,0,10000, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DM_VS_DC_particleveto (new TH3F(("d_VS_DM_VS_DC_particleveto"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity VS delayed calorimetry particle veto;Delayed calorimetry[keV];Delayed multiplicity;delayed Ge[keV]", 100,0,10000, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DM_VS_PM_particleveto (new TH3F(("d_VS_DM_VS_PM_particleveto"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity VS prompt multiplicity particle veto;Prompt multiplicity;Delayed multiplicity;delayed Ge[keV]", 20,0,20, 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH3F d_VS_DC_VS_PC_particleveto (new TH3F(("d_VS_DC_VS_PC_particleveto"+thread_i_str).c_str(), "delayed Ge VS delayed calorimetry VS prompt calorimetry particle veto;Prompt calorimetry[keV];Delayed calorimetry[keV];delayed Ge[keV]", 100,0,10000, 100,0,10000, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition Prompt Calorimetry < 5 MeV (code PC5):
      unique_TH1F p_PC5 (new TH1F(("p_PC5_"+thread_i_str).c_str(), "prompt Ge PC5", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC5 (new TH2F(("pp_PC5_"+thread_i_str).c_str(), "gamma-gamma prompt PC5;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC5 (new TH1F(("d_PC5_"+thread_i_str).c_str(), "delayed PC5", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_PC5 (new TH2F(("dd_PC5_"+thread_i_str).c_str(), "gamma-gamma delayed PC5;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC5 (new TH2F(("dp_PC5_"+thread_i_str).c_str(), "delayed VS prompt PC5;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC5 (new TH2F(("d_VS_DC_PC5_"+thread_i_str).c_str(), "d_VS_DC_PC5", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC5 (new TH2F(("d_VS_PM_PC5_"+thread_i_str).c_str(), "d_VS_PM_PC5", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC5 (new TH2F(("d_VS_DM_PC5_"+thread_i_str).c_str(), "d_VS_DM_PC5", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition Prompt Calorimetry < 3 MeV (code PC3):
      unique_TH1F p_PC3 (new TH1F(("p_PC3_"+thread_i_str).c_str(), "prompt PC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3 (new TH2F(("pp_PC3_"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3 (new TH1F(("d_PC3_"+thread_i_str).c_str(), "delayed PC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_PC3 (new TH2F(("dd_PC3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC3 (new TH2F(("dp_PC3_"+thread_i_str).c_str(), "delayed VS prompt PC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3 (new TH2F(("d_VS_DC_PC3_"+thread_i_str).c_str(), "d_VS_DC_PC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3 (new TH2F(("d_VS_PM_PC3_"+thread_i_str).c_str(), "d_VS_PM_PC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3 (new TH2F(("d_VS_DM_PC3_"+thread_i_str).c_str(), "d_VS_DM_PC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition Prompt Calorimetry < 2 MeV (code PC2):
      unique_TH1F p_PC2 (new TH1F(("p_PC2_"+thread_i_str).c_str(), "prompt PC2", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC2 (new TH2F(("pp_PC2_"+thread_i_str).c_str(), "gamma-gamma prompt PC2;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC2 (new TH1F(("d_PC2_"+thread_i_str).c_str(), "delayed PC2", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_PC2 (new TH2F(("dd_PC2_"+thread_i_str).c_str(), "gamma-gamma delayed PC2;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC2 (new TH2F(("dp_PC2_"+thread_i_str).c_str(), "delayed VS prompt PC2;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC2 (new TH2F(("d_VS_DC_PC2_"+thread_i_str).c_str(), "d_VS_DC_PC2", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC2 (new TH2F(("d_VS_PM_PC2_"+thread_i_str).c_str(), "d_VS_PM_PC2", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC2 (new TH2F(("d_VS_DM_PC2_"+thread_i_str).c_str(), "d_VS_DM_PC2", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      
      // Condition Delayed Calorimetry < 3 MeV (code DC3):
      unique_TH1F p_DC3 (new TH1F(("p_DC3_"+thread_i_str).c_str(), "prompt DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_DC3 (new TH2F(("pp_DC3_"+thread_i_str).c_str(), "gamma-gamma prompt DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_DC3 (new TH1F(("d_DC3_"+thread_i_str).c_str(), "delayed DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_DC3 (new TH2F(("dd_DC3_"+thread_i_str).c_str(), "gamma-gamma delayed DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_DC3 (new TH2F(("dp_DC3_"+thread_i_str).c_str(), "delayed VS prompt DC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_DC3 (new TH2F(("d_VS_DC_DC3_"+thread_i_str).c_str(), "d_VS_DC_DC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_DC3 (new TH2F(("d_VS_PM_DC3_"+thread_i_str).c_str(), "d_VS_PM_DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_DC3 (new TH2F(("d_VS_DM_DC3_"+thread_i_str).c_str(), "d_VS_DM_DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      
      // Condition 1 < Delayed Calorimetry < 3 MeV (code DC1_3):
      unique_TH1F p_DC1_3 (new TH1F(("p_DC1_3_"+thread_i_str).c_str(), "prompt DC1_3", nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F pp_DC1_3 (new TH2F(("pp_DC1_3_"+thread_i_str).c_str(), "gamma-gamma prompt DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_DC1_3 (new TH1F(("d_DC1_3_"+thread_i_str).c_str(), "delayed DC1_3", nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_DC1_3 (new TH2F(("dd_DC1_3_"+thread_i_str).c_str(), "gamma-gamma delayed DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_DC1_3 (new TH2F(("dp_DC1_3_"+thread_i_str).c_str(), "delayed VS prompt DC1_3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_DC1_3 (new TH2F(("d_VS_DC_DC1_3_"+thread_i_str).c_str(), "d_VS_DC_DC1_3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_DC1_3 (new TH2F(("d_VS_PM_DC1_3_"+thread_i_str).c_str(), "d_VS_PM_DC1_3", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM_DC1_3 (new TH2F(("d_VS_DM_DC1_3_"+thread_i_str).c_str(), "d_VS_DM_DC1_3", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      // Condition Delayed Calorimetry < 3 MeV AND Prompt Calorimetry < 3 MeV (code PC3DC3):
      unique_TH1F p_PC3DC3 (new TH1F(("p_PC3DC3_"+thread_i_str).c_str(), "prompt PC3DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3DC3 (new TH2F(("pp_PC3DC3_"+thread_i_str).c_str(), "gamma-gamma prompt PC3DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3DC3 (new TH1F(("d_PC3DC3_"+thread_i_str).c_str(), "delayed PC3DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_PC3PM3DC3DM3 (new TH1F(("d_PC3PM3DC3DM3"+thread_i_str).c_str(), "delayed PC3PM3DC3DM3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_PC3DC3 (new TH2F(("dd_PC3DC3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_PC3PM3DC3DM3 (new TH2F(("dd_PC3PM3DC3DM3"+thread_i_str).c_str(), "gamma-gamma delayed PC3DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp_PC3DC3 (new TH2F(("dp_PC3DC3_"+thread_i_str).c_str(), "delayed VS prompt PC3DC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3DC3 (new TH2F(("d_VS_DC_PC3DC3_"+thread_i_str).c_str(), "d_VS_DC_PC3DC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3DC3 (new TH2F(("d_VS_PM_PC3DC3_"+thread_i_str).c_str(), "d_VS_PM_PC3DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3DC3 (new TH2F(("d_VS_DM_PC3DC3_"+thread_i_str).c_str(), "d_VS_DM_PC3DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition 1 < Delayed Calorimetry < 3 MeV AND Prompt Calorimetry < 3 MeV (code PC3DC1_3):
      unique_TH1F p_PC3DC1_3 (new TH1F(("p_PC3DC1_3_"+thread_i_str).c_str(), "prompt PC3DC1_3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3DC1_3 (new TH2F(("pp_PC3DC1_3_"+thread_i_str).c_str(), "gamma-gamma prompt PC3DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3DC1_3 (new TH1F(("d_PC3DC1_3_"+thread_i_str).c_str(), "delayed PC3DC1_3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_PC3DC1_3 (new TH2F(("dd_PC3DC1_3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC3DC1_3 (new TH2F(("dp_PC3DC1_3_"+thread_i_str).c_str(), "delayed VS prompt PC3DC1_3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3DC1_3 (new TH2F(("d_VS_DC_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_DC_PC3DC1_3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3DC1_3 (new TH2F(("d_VS_PM_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_PM_PC3DC1_3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3DC1_3 (new TH2F(("d_VS_DM_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_DM_PC3DC1_3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Paris
      // unique_TH1F paris_prompt (new TH1F(("paris_prompt_"+thread_i_str).c_str(), "paris_prompt;keV", 10000,0,20000));
      // unique_TH1F paris_delayed (new TH1F(("paris_delayed_"+thread_i_str).c_str(), "paris_delayed;keV", 10000,0,20000));
      unique_TH1F LaBr3_prompt (new TH1F(("LaBr3_prompt_"+thread_i_str).c_str(), "LaBr3_prompt;keV", 10000,0,20000));
      unique_TH1F LaBr3_delayed (new TH1F(("LaBr3_delayed_"+thread_i_str).c_str(), "LaBr3_delayed;keV", 10000,0,20000));
      unique_TH2F d_VS_dLaBr3 (new TH2F(("d_VS_dLaBr3_"+thread_i_str).c_str(), "d_VS_dLaBr3;LaBr3 [keV];Delayed clover [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F pLaBr3_VS_label (new TH2F(("pLaBr3__VS_label"+thread_i_str).c_str(), "pLaBr3_VS_label;label;Prompt LaBr3 [keV]", 64,0,64, 1000,0,10000));
      unique_TH2F dLaBr3_VS_label (new TH2F(("dLaBr3__VS_label"+thread_i_str).c_str(), "dLaBr3_VS_label;label;Delayed LaBr3 [keV]", 64,0,64, 1000,0,10000));

      unique_TH2F short_vs_long_prompt (new TH2F(("short_vs_long_prompt_"+thread_i_str).c_str(), "short_vs_long_prompt;keV", 1000,0,10000, 1000,0,10000));
      unique_TH2F short_vs_long_delayed (new TH2F(("short_vs_long_delayed_"+thread_i_str).c_str(), "short_vs_long_delayed;keV", 1000,0,10000, 1000,0,10000));
      unique_TH2F short_over_long_VS_time (new TH2F(("short_over_long_VS_time_"+thread_i_str).c_str(), "short_over_long_VS_time;keV", 1000,-2,2, 600,-50_ns,250_ns));
      unique_TH1F phoswitches_prompt (new TH1F(("phoswitches_prompt_"+thread_i_str).c_str(), "Phoswitches_prompt;keV", 10000,0,20000));
      unique_TH1F phoswitches_delayed (new TH1F(("phoswitches_delayed_"+thread_i_str).c_str(), "Phoswitches_delayed;keV", 10000,0,20000));

      unique_TH2F p_VS_dparis_mod (new TH2F(("p_VS_dparis_mod_"+thread_i_str).c_str(), "p_VS_dparis_mod;Paris module_{delayed} [keV]; E#gamma_{prompt} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F p_VS_pparis_mod (new TH2F(("p_VS_pparis_mod_"+thread_i_str).c_str(), "p_VS_pparis_mod;Paris module_{prompt} [keV]; E#gamma_{prompt} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_dparis_mod (new TH2F(("d_VS_dparis_mod_"+thread_i_str).c_str(), "d_VS_dparis_mod;Paris module_{delayed} [keV]; E#gamma_{delayed} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_dclean_phos (new TH2F(("d_VS_dclean_phos_"+thread_i_str).c_str(), "d_VS_dclean_phos;Paris clean phoswtich_{delyaed} [keV]; E#gamma_{delayed} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_pclean_phos (new TH2F(("d_VS_pclean_phos_"+thread_i_str).c_str(), "d_VS_pclean_phos;Paris module_{prompt} [keV]; E#gamma_{prompt} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_daddback_mod (new TH2F(("d_VS_daddback_mod_"+thread_i_str).c_str(), "d_VS_daddback_mod;Paris module_{prompt} [keV]; E#gamma_{prompt} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F daddback_module_gate_1370_VS_run_paris (new TH2F(("daddback_module_gate_1370_VS_run_"+thread_i_str).c_str(), "daddback_module_gate_1370_VS_run_paris;run number;Paris module_{prompt} [keV];", 100,50,150, 5000,0,20000));
      unique_TH2F daddback_module_gate_1370_VS_index_paris (new TH2F(("daddback_module_gate_1370_VS_index_paris_"+thread_i_str).c_str(), "daddback_module_gate_1370_VS_run_paris;index;Paris module_{prompt} [keV];", 100,0,100, 5000,0,20000));

      unique_TH2F paris_VS_sum_paris_M2 (new TH2F(("paris_VS_sum_paris_M2_"+thread_i_str).c_str(), "two delayed paris VS their sum;E sum [keV]; E#gamma_{delayed}[keV]", 2000,0,20000, 1000,0,10000));
      unique_TH2F paris_VS_sum_paris_M2_P (new TH2F(("paris_VS_sum_paris_M2_P_"+thread_i_str).c_str(), "two delayed paris VS their sum, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", 2000,0,20000, 1000,0,10000));
      unique_TH2F paris_VS_sum_paris_M2_pP (new TH2F(("paris_VS_sum_paris_M2_pP_"+thread_i_str).c_str(), "two delayed paris VS their sum, particle + prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", 2000,0,20000, 1000,0,10000));
      unique_TH2F prompt_paris_VS_sum_paris_M2_pP (new TH2F(("prompt_paris_VS_sum_paris_M2_pP_"+thread_i_str).c_str(), "prompt paris VS two delayed paris sum, particle + prompt trigger;E sum_{delayed} [keV]; E#gamma_{delayed}prompt[keV]", 2000,0,20000, 1000,0,10000));

      // BGO
      unique_TH2F d_VS_pBGO (new TH2F(("d_VS_pBGO_"+thread_i_str).c_str(), "d_VS_pBGO;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_dBGO (new TH2F(("d_VS_dBGO_"+thread_i_str).c_str(), "d_VS_dBGO;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_dBGO_addback (new TH2F(("d_VS_dBGO_addback_"+thread_i_str).c_str(), "d_VS_dBGO_addback;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));

      // Particle trigger (code p)
      unique_TH1F p_p (new TH1F(("p_p_"+thread_i_str).c_str(), "prompt particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp_p (new TH2F(("pp_p_"+thread_i_str).c_str(), "gamma-gamma prompt particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_p (new TH1F(("d_p_"+thread_i_str).c_str(), "delayed particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_p_prompt_veto (new TH1F(("d_p_prompt_veto"+thread_i_str).c_str(), "delayed particle trigger prompt veto;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_p (new TH2F(("dd_p_"+thread_i_str).c_str(), "gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_p_prompt_veto (new TH2F(("dd_p_prompt_veto_"+thread_i_str).c_str(), "gamma-gamma delayed particle trigger, prompt veto;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_pPM4DM4 (new TH2F(("dd_pPM4DM4_"+thread_i_str).c_str(), "gamma-gamma delayed with particle & prompt multiplicity < 4 and delayed multiplicity < 4;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_pPM4PC3DM4 (new TH2F(("dd_pPM4PC3DM4_"+thread_i_str).c_str(), "gamma-gamma delayed with particle & prompt multiplicity < 4 & prompt calcorimetry < 3 & delayed multiplicity < 4;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_pPM4PC3DM4Ex (new TH2F(("dd_pPM4PC3DM4Ex_"+thread_i_str).c_str(), "gamma-gamma delayed with particle & prompt multiplicity < 4 & prompt calcorimetry < 3 & delayed multiplicity < 4 & proton;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp_p (new TH2F(("dp_p_"+thread_i_str).c_str(), "delayed VS prompt particle, trigger;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F E_dT_p (new TH2F(("E_dT_p_"+thread_i_str).c_str(), "E vs time particle trigger", 600,-100_ns,200_ns, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F E_dT_p__642d (new TH2F(("E_dT_p__642d_"+thread_i_str).c_str(), "E vs time, particle and delayed 642 trigger", 130,40_ns,170_ns, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_calo_p (new TH1F(("p_calo_p_"+thread_i_str).c_str(), "prompt calorimetry, particle trigger", 2000,0,20000));
      unique_TH1F d_calo_p (new TH1F(("d_calo_p_"+thread_i_str).c_str(), "prompt calorimetry, particle trigger", 2000,0,20000));

      // Prompt trigger and particle (code pP)

      unique_TH1F d_pP (new TH1F(("d_pP_"+thread_i_str).c_str(), "with prompt gamma, delayed particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_pP (new TH2F(("dd_pP_"+thread_i_str).c_str(), "with prompt gamma, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_calo_pP (new TH1F(("d_calo_pP_"+thread_i_str).c_str(), "with prompt gamma, prompt calorimetry particle trigger", 2000,0,20000));

      unique_TH1F p_pPC5 (new TH1F(("p_pPC5_"+thread_i_str).c_str(), "prompt particle trigger PC5;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5 (new TH1F(("d_pPC5_"+thread_i_str).c_str(), "delayed particle trigger PC5;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5PM4DM4 (new TH1F(("d_pPC5PM4DM4_"+thread_i_str).c_str(), "delayed particle trigger PC5 PM4 DM4;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pPC3 (new TH1F(("p_pPC3_"+thread_i_str).c_str(), "prompt particle trigger PC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC3 (new TH1F(("d_pPC3_"+thread_i_str).c_str(), "delayed particle trigger PC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC3PM4DM4 (new TH1F(("d_pPC3PM4DM4_"+thread_i_str).c_str(), "delayed particle trigger PC3 PM4DM4;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pPC2 (new TH1F(("p_pPC2_"+thread_i_str).c_str(), "prompt particle trigger PC2;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC2 (new TH1F(("d_pPC2_"+thread_i_str).c_str(), "delayed particle trigger PC2;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC2_PM4DM4 (new TH1F(("d_pPC2_PM4DM4_"+thread_i_str).c_str(), "delayed particle trigger PC2 DM4PM4;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pDC3 (new TH1F(("p_pDC3_"+thread_i_str).c_str(), "prompt particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pDC3 (new TH1F(("d_pDC3_"+thread_i_str).c_str(), "delayed particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPDC3 (new TH1F(("d_pPDC3_"+thread_i_str).c_str(), "delayed particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pDC1_3 (new TH1F(("p_pDC1_3_"+thread_i_str).c_str(), "prompt particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pDC1_3 (new TH1F(("d_pDC1_3_"+thread_i_str).c_str(), "delayed particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPDC1_3 (new TH1F(("d_pPDC1_3_"+thread_i_str).c_str(), "delayed particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5DC3 (new TH1F(("d_pPC5DC3_"+thread_i_str).c_str(), "delayed particle trigger PC5DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5DC1_3 (new TH1F(("d_pPC5DC1_3_"+thread_i_str).c_str(), "delayed particle trigger PC5DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));

      unique_TH2F p_VS_PM_p (new TH2F(("p_VS_PM_p_"+thread_i_str).c_str(), "prompt particle trigger VS prompt mult;prompt mult;prompt [keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_p (new TH2F(("d_VS_PM_p_"+thread_i_str).c_str(), "delayed particle trigger VS prompt mult;prompt mult;delayed [keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_DM_p (new TH2F(("p_VS_DM_p_"+thread_i_str).c_str(), "prompt particle trigger VS delayed mult;delayed mult;prompt [keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM_p (new TH2F(("d_VS_DM_p_"+thread_i_str).c_str(), "delayed particle trigger VS delayed mult;delayed mult;delayed [keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM_pP (new TH2F(("d_VS_DM_pP_"+thread_i_str).c_str(), "with prompt, delayed particle trigger VS delayed mult;delayed mult;delayed [keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_PC_p (new TH2F(("p_VS_PC_p_"+thread_i_str).c_str(), "prompt particle trigger VS prompt calorimetry;prompt calorimetry [10 bins/keV]; prompt [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PC_p (new TH2F(("d_VS_PC_p_"+thread_i_str).c_str(), "delayed particle trigger VS prompt calorimetry;prompt calorimetry [10 bins/keV]; delayed [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PC_pP (new TH2F(("d_VS_PC_pP_"+thread_i_str).c_str(), "delayed particle trigger VS prompt calorimetry;prompt calorimetry [10 bins/keV]; delayed [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_DC_p (new TH2F(("p_VS_DC_p_"+thread_i_str).c_str(), "prompt particle trigger VS delayed calorimetry;delayed calorimetry [10 bins/keV]; prompt [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_p (new TH2F(("d_VS_DC_p_"+thread_i_str).c_str(), "delayed particle trigger VS delayed calorimetry;delayed calorimetry [10 bins/keV]; delayed [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_pP (new TH2F(("d_VS_DC_pP_"+thread_i_str).c_str(), "delayed particle trigger VS delayed calorimetry;delayed calorimetry [10 bins/keV]; delayed [keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      unique_TH1F d_pPC3MP3DC3MD4 (new TH1F(("d_pPC3MP3DC3MD4_"+thread_i_str).c_str(), "delayed particle trigger d_pPC3MP5DC3MD4;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC3MP3DC3MD4Ex6_5 (new TH1F(("d_pPC3MP3DC3MD4Ex6_5_"+thread_i_str).c_str(), "delayed particle trigger d_pPC3MP5DC3MD4;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      
      // DSSD
      // Excitation energy trigger (code Ex, ExSI = 4 MeV < Ex < 6.5 MeV)
      constexpr static int bins_Ex = 200;
      constexpr static double max_Ex = 10000;
      unique_TH2F Ex_U6_VS_ring (new TH2F(("Ex_U6_VS_ring_"+thread_i_str).c_str(), "excitation energy proton VS ring number;Excitation energy [keV]" , 20,0,20, bins_Ex,0,max_Ex));
      unique_TH2F p_VS_Ex_U6 (new TH2F(("p_VS_Ex_U6_"+thread_i_str).c_str(), "prompt Ge VS excitation energy proton;Excitation energy [keV];keV" , bins_Ex,0,max_Ex, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_Ex_U6__P (new TH2F(("d_VS_Ex_U6__P_"+thread_i_str).c_str(), "delayed Ge VS excitation energy proton;Excitation energy [keV];keV", bins_Ex,0,max_Ex, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F Ex_U6_histo (new TH1F(("Ex_U6_histo_"+thread_i_str).c_str(), "Excitation energy;keV", bins_Ex,0,max_Ex));
      unique_TH2F Ex_VS_PC_p (new TH2F(("Ex_VS_PC_p_"+thread_i_str).c_str(), "excitation energy proton VS prompt calorimetry;Calorimetry prompt [keV];Excitation energy [keV]" ,2000,0,20000,  bins_Ex,0,max_Ex));
      unique_TH2F Ex_VS_DC_p__P (new TH2F(("Ex_VS_DC_p__P_"+thread_i_str).c_str(), "excitation energy proton VS delayed calorimetry;Calorimetry delayed [keV];Excitation energy [keV]", 2000,0,20000, bins_Ex,0,max_Ex));
      constexpr static int bins_Emiss = 200;
      constexpr static double max_Emis = 10000;
      unique_TH1F Emiss__P (new TH1F(("Emiss__P_"+thread_i_str).c_str(), "Missing energy;Missing energy [keV];", bins_Emiss,0,max_Emis));
      unique_TH2F Emiss_VS_Edelayed_p__P (new TH2F(("Emiss_VS_Edelayed_p__P_"+thread_i_str).c_str(), "Missing energy VS delayed energy;Calorimetry delayed [keV];Missing energy [keV]", 2000,0,20000, bins_Emiss,0,max_Emis));
      
      unique_TH1F p_proton (new TH1F(("p_proton"+thread_i_str).c_str(), "prompt, proton trigger; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp_proton (new TH2F(("pp_proton"+thread_i_str).c_str(), "prompt-prompt, proton trigger; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F p_proton_veto (new TH1F(("p_proton_veto"+thread_i_str).c_str(), "prompt, proton trigger veto; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp_proton_veto (new TH2F(("pp_proton_veto"+thread_i_str).c_str(), "prompt-prompt, proton trigger veto; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_proton (new TH1F(("d_proton"+thread_i_str).c_str(), "delayed, proton trigger; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_proton (new TH2F(("dd_proton"+thread_i_str).c_str(), "delayed-delayed, proton trigger; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_proton_P (new TH1F(("d_proton_P"+thread_i_str).c_str(), "delayed, proton prompt trigger veto; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_proton_P (new TH2F(("dd_proton_P"+thread_i_str).c_str(), "delayed-delayed, proton trigger veto; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_proton_P_veto (new TH1F(("d_proton_P_veto"+thread_i_str).c_str(), "delayed prompt & proton trigger veto; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_proton_P_veto (new TH2F(("dd_proton_P_veto"+thread_i_str).c_str(), "delayed-delayed prompt & proton trigger veto; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_proton_PM1_3 (new TH1F(("d_proton_PM1_3"+thread_i_str).c_str(), "delayed, proton & 1<=prompt mult=<3 trigger; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_proton_PM1_3_veto (new TH2F(("dd_proton_PM1_3_veto"+thread_i_str).c_str(), "delayed-delayed prompt & proton trigger veto; delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      unique_TH1F d_Ex (new TH1F(("d_Ex"+thread_i_str).c_str(), "delayed good proton; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_ExP (new TH1F(("d_ExP"+thread_i_str).c_str(), "delayed good proton with prompt; delayed [keV]", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_ExP (new TH2F(("dd_ExP_"+thread_i_str).c_str(), "with correct excitation energy, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_C2_ExP (new TH2F(("dd_C2_ExP_"+thread_i_str).c_str(), "with correct excitation energy, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_ExSIP (new TH2F(("dd_ExSIP_"+thread_i_str).c_str(), "with best excitation energy, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      // constexpr static int bins_sum_Ge = 2500;
      // constexpr static int max_sum_Ge = 5000;
      // unique_TH2F d_VS_sum_C2 (new TH2F(("d_VS_sum_C2_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C2_P (new TH2F(("d_VS_sum_C2_P_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_clean_sum_C2_P (new TH2F(("d_VS_clean_sum_C2_P_"+thread_i_str).c_str(), "delayed Ge VS clean sum of two clean Ge, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_clean_sum_C2_pP (new TH2F(("d_VS_clean_sum_C2_pP_"+thread_i_str).c_str(), "delayed Ge VS clean sum of two clean Ge, particle and prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_clean_sum_C2_PDM2 (new TH2F(("d_VS_clean_sum_C2_PDM2_"+thread_i_str).c_str(), "delayed Ge VS clean sum of two clean Ge, prompt trigger and delayed M2;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_clean_sum_C2_PM5DM2 (new TH2F(("d_VS_clean_sum_C2_PM5DM2_"+thread_i_str).c_str(), "delayed Ge VS clean sum of two clean Ge, prompt trigger and prompt M<5 and delayed M2;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C2_pP (new TH2F(("d_VS_sum_C2_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, particle + prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C2_ExP (new TH2F(("d_VS_sum_C2_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, particle + good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C2_ExSIP (new TH2F(("d_VS_sum_C2_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F p_VS_sum_C_pP (new TH2F(("p_VS_sum_C_pP_"+thread_i_str).c_str(), "prompt Ge VS sum of all clean Ge, particle + prompt gamma;E sum _{delayed} [keV]; E#gamma_{prompt}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C_pP (new TH2F(("d_VS_sum_C_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C_ExP (new TH2F(("d_VS_sum_C_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_C_ExSIP (new TH2F(("d_VS_sum_C_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_Ge_pP (new TH2F(("d_VS_sum_Ge_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_Ge_ExP (new TH2F(("d_VS_sum_Ge_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F d_VS_sum_Ge_ExSIP (new TH2F(("d_VS_sum_Ge_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      // unique_TH2F dirty_d_VS_sum_C2 (new TH2F(("dirty_d_VS_sum_C2_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C2_P (new TH2F(("dirty_d_VS_sum_C2_P_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C2_pP (new TH2F(("dirty_d_VS_sum_C2_pP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, particle + prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C2_ExP (new TH2F(("dirty_d_VS_sum_C2_ExP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, particle + good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C2_ExSIP (new TH2F(("dirty_d_VS_sum_C2_ExSIP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C_pP (new TH2F(("dirty_d_VS_sum_C_pP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C_ExP (new TH2F(("dirty_d_VS_sum_C_ExP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dirty_d_VS_sum_C_ExSIP (new TH2F(("dirty_d_VS_sum_C_ExSIP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      // unique_TH2F Ex_vs_dC_sum (new TH2F(("Ex_vs_dC_sum_"+thread_i_str).c_str(), "Ex VS sum of all delayed clean Ge;E sum [keV]; E DSSD [keV]", bins_sum_Ge,0,max_sum_Ge, bins_Ex,0,max_Ex));
      // unique_TH2F Ep_vs_dC_sum_P (new TH2F(("Ep_vs_dC_sum_P_"+thread_i_str).c_str(), "E dssd VS sum of all delayed clean Ge;E sum [keV]; E DSSD [keV]", bins_sum_Ge,0,max_sum_Ge, bins_Ex,0,max_Ex));
      
      unique_TH2F p_VS_DC_ExSIP (new TH2F(("p_VS_DC_ExSIP_"+thread_i_str).c_str(), "prompt Ge VS delayed Ge, particle + best Ex for SI;Delayed calorimetry [keV]; E#gamma_{delayed}[keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_ExSIP (new TH2F(("d_VS_DC_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS delayed Ge, particle + best Ex for SI;Delayed calorimetry [keV]; E#gamma_{delayed}[keV]",2000,0,20000,  nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      unique_TH1F neutron_hit_pattern (new TH1F(("neutron_hit_pattern"+thread_i_str).c_str(), "neutron_hit_pattern", 1000,0,1000));
      unique_TH1F hit_pattern_2755 (new TH1F(("hit_pattern_2755"+thread_i_str).c_str(), "hit_pattern_2755", 1000,0,1000));
      
      std::string outFolder = "data/"+trigger+"/"+target+"/";
      Path::make(outFolder);
      std::string out_filename = outFolder+filename;

      Event event;
      event.reading(tree, "mltTEQ");

      CloversV2 pclovers;
      CloversV2 dclovers;

      // CloversV2 pclovers_raw;
      // CloversV2 dclovers_raw;

      // CloversV2 last_prompt_clovers;
      // CloversV2 last_delayed_clovers;

      SimpleParis pparis(&calibPhoswitches);
      SimpleParis dparis(&calibPhoswitches);

      WarsawDSSD dssd;

      // std::vector<double> prompt_phoswitch;
      // std::vector<double> delayed_phoswitch;

      // std::vector<double> prompt_phoswitch_label;
      // std::vector<double> delayed_phoswitch_label;

      // std::vector<double> prompt_paris_module;
      // std::vector<double> delayed_paris_module;

      // constexpr void addback = [&](){
        

      // }
      
      // std::vector<double> sector_energy;
      // std::vector<double> ring_energy;

      // std::vector<Label> sector_labels;
      // std::vector<Label> ring_labels;

        // auto isContaminant = [&](float const & nrj){
        //   return (
        //                         (nrj < 100)
        //         || (506 < nrj && nrj < 516)
        //         || (594 < nrj && nrj < 605)
        //         || (1777< nrj && nrj < 1784)
        //   );
        // };

        // auto hasContaminant = [&](CloversV2 const & clovers) {
        //   int nb_gamma_conta = 0;
        //   for (auto const & index : clovers.GeClean_id) if (isContaminant(clovers[index].nrj)) ++nb_gamma_conta;
        //   return nb_gamma_conta;
        // };

      // Handle the first hit : 
      int evt_i = 0;
      tree->GetEntry(evt_i);
      auto timestamp_init = event.stamp;

      for ( ;(evt_i < tree->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        double prompt_clover_calo = 0;

        if (evt_i>0 && evt_i%freq_hit_display == 0) print(nicer_double(evt_i, 0), "events");

        tree->GetEntry(evt_i);

        // pclovers_raw.clear();
        // dclovers_raw.clear();

        pclovers.clear();
        dclovers.clear();

        pparis.clear();
        dparis.clear();

        dssd.clear();

        // sector_energy.clear();
        // ring_energy.clear();

        // sector_labels.clear();
        // ring_labels.clear();

        auto const absolute_time_h = double_cast(event.stamp)*1.e-12/3600.;
        timestamp_hist_VS_run->Fill(absolute_time_h, run_number);

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          auto const & time =  event.times[hit_i];
          auto const & nrj = event.nrjs[hit_i];
          auto const & nrj2 = event.nrj2s[hit_i];

          if (nrj<20_keV) continue;

          // Remove bad Ge and overflow :
          //  if ((find_key(CloversV2::maxE_Ge, label) && nrj>CloversV2::maxE_Ge.at(label))) continue;
          if (label == 65 && run_number == 116) continue; // This detector's timing slipped in this run
          if ((label == 134 || label == 135 || label == 136) && time > 100_ns) continue; // These detectors have strange events after 100 ns

          auto const & det_name = detectors[label]; 
          if (bidim_by_run && !(Paris::is[label] && !Paris::pid_LaBr3(nrj,nrj2)))
          {
            E_vs_run[det_name]->Fill(run_number, nrj);
            T_vs_run[det_name]->Fill(run_number, time);
          }

          if (label == 252)
          {
            Fatima_E_dT->Fill(time, nrj);
          }

          // Paris :
          if (Paris::is[label])
          {
            if (found(blacklist, label)) continue;
            if (Paris::pid_LaBr3(nrj, nrj2)) E_dT_phoswitch->Fill(time, nrj);
            else if (Paris::pid_good_phoswitch(nrj, nrj2)) E_dT_phoswitch->Fill(time, calibPhoswitches.calibrate(label, nrj, nrj2));
            short_over_long_VS_time->Fill(time, nrj/nrj2);
            if (gate(-5_ns, time, 5_ns))
            {
              short_vs_long_prompt->Fill(nrj, nrj2); 
              pparis.fill(event, hit_i);
            }
            else if (gate(5_ns, time, 40_ns))
            {
              neutron_hit_pattern->Fill(label);
            }
            else if (gate(40_ns, time, 170_ns))
            {
              short_vs_long_delayed->Fill(nrj, nrj2);
              dparis.fill(event, hit_i);
            }
          }

          // Clovers:
          if (gate(-20_ns, time, 20_ns) )
          {
            if (CloversV2::isBGO(label)) prompt_clover_calo += nrj ;
            else if (CloversV2::isGe(label)) 
            {
              if (CloversIsBlacklist[label]) continue;
              // pclovers_raw.fill(event, hit_i);
              // Apply the run by run correction
              if (kCalibGe) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
              if (!gate(505_keV, nrj, 515_keV)) prompt_clover_calo += smear(nrj, random);
            }
            pclovers.fill(event, hit_i);
          }
          else if (gate(20_ns, time, 40_ns))
          {
            if (CloversIsBlacklist[label]) continue;
            // if (kCalibGe && CloversV2::isGe(label)) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
            n->Fill(nrj);
          }
          else if (gate(40_ns, time, 170_ns) )
          {
            if (CloversIsBlacklist[label]) continue;
            // dclovers_raw.fill(event, hit_i);
            // if (kCalibGe && CloversV2::isGe(label)) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
            dclovers.fill(event, hit_i);
          }

          // Dssd :
          if (gate(799u, label, 860u)) dssd.fill(event, hit_i);

        }// End hits loop
      
        //////////////
        // Analyse  //
        //////////////


        // First step, perform add-back and compton suppression :
        pclovers.analyze();
        dclovers.analyze();
        // pclovers_raw.analyze();
        // dclovers_raw.analyze();
        pparis.analyze();
        dparis.analyze();
        dssd.analyze();

        // -- Multiplicity -- //
        auto const & pcloverM = pclovers.all.size();
        auto const & pparisM = pparis.module_mult();
        auto const & PM = pcloverM + pparisM;

        auto const & dcloverM = dclovers.all.size();
        auto const & dparisM = dparis.module_mult();
        auto const & DM_raw = dcloverM + dparisM;

        // auto const & delayed_Ge_mult = delayed_clovers.Ge.size();
        // bool Gemult[10] = {false}; Gemult[delayed_Ge_mult] = true;
        // auto const & delayed_C_mult = delayed_clovers.GeClean_id.size();
        // bool Cmult[10] = {false}; Cmult[delayed_C_mult] = true;

        p_mult->Fill(PM);
        d_mult->Fill(DM_raw);
        dp_mult->Fill(PM, DM_raw);

        // -- Calorimetry -- //
        auto const & prompt_paris_calo = pparis.calorimetry();
        
        if (prompt_clover_calo > 0) p_calo_clover -> Fill(prompt_clover_calo);
        if (prompt_paris_calo  > 0) p_calo_phoswitch -> Fill(prompt_paris_calo);
        if (prompt_clover_calo > 0 && prompt_paris_calo > 0) p_calo_clover_VS_p_calo_phoswitch -> Fill(prompt_clover_calo, prompt_paris_calo);
        // if (delayed_clover_calo > 0 && delayed_paris_calo > 0) d_calo_clover_VS_d_calo_phoswitch -> Fill(delayed_clover_calo, delayed_paris_calo);

        auto const & PC = prompt_clover_calo + prompt_paris_calo;
        // auto const & DC = delayed_clover_calo + delayed_paris_calo;
        if (0 < PC) p_calo->Fill(PC);
        // if (0 < DC) d_calo->Fill(DC);
        // // if (PC > 0 && DC > 0) dp_calo->Fill(PC, DC);
        // if (DC > 0 && PM > 0) d_calo_pP->Fill(delayed_clover_calo);

        ////////// RAW Ge /////////
        // for (auto const & clover : pclovers_raw.clean) p_before_correction->Fill(clover->nrj);
        // for (auto const & clover : dclovers_raw.clean) d_before_correction->Fill(clover->nrj);

        
        //////////////// DSSD ///////////////////
        bool dssd_trigger = dssd.mult() > 0;
        // bool proton_trigger = 
        //       (dssd.sectors().mult == 1) 
        //   &&  (dssd.rings().mult < 3) 
        //   &&  (dssd.sectors()[0].nrj < 8_MeV) 
        //   &&  gate(-10_ns,dssd.sectors()[0].time, 10_ns);
        bool proton_trigger = true;
        for (auto sector : dssd.sectors()) if (sector.nrj < 8_MeV && gate(-10_ns,sector.time, 10_ns)) proton_trigger = false;
        
        auto const & Ex_U6 = (dssd.ok) ? Ex(dssd.nrj, dssd.ring_index) : ExcitationEnergy::bad_value;

        auto const & Emiss = Ex_U6-PC;

        if (Ex_U6 > 0_MeV) 
        {
          Ex_U6_histo->Fill(Ex_U6);
          Ex_U6_VS_ring->Fill(Ex_U6, dssd.ring_index);
          if (PM > 0)
          {
            Emiss__P->Fill(Emiss);
          }
        }

        /////// PROMPT CLEAN ///////

        for (size_t loop_i = 0; loop_i<pclovers.GeClean_id.size(); ++loop_i)
        {
          auto const & index_i = pclovers.GeClean_id[loop_i];
          auto const & clover_i = pclovers[index_i]; 
          p->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          p_VS_PM->Fill(PM, clover_i.nrj);

          if (proton_trigger) p_proton->Fill(clover_i.nrj);
          else p_proton_veto->Fill(clover_i.nrj);

          if (PC < 5_MeV) p_PC5->Fill(clover_i.nrj);
          if (PC < 3_MeV) p_PC3->Fill(clover_i.nrj);
          if (PC < 2_MeV) p_PC2->Fill(clover_i.nrj);

          // Prompt-prompt :
          for (size_t loop_j = loop_i+1; loop_j<pclovers.GeClean_id.size(); ++loop_j)
          {
            auto const & clover_j = pclovers[pclovers.GeClean_id[loop_j]];
            pp->Fill(clover_i.nrj, clover_j.nrj);
            pp->Fill(clover_j.nrj, clover_i.nrj);
            if (dssd_trigger) 
            {
              pp_p->Fill(clover_i.nrj, clover_j.nrj);
              pp_p->Fill(clover_j.nrj, clover_i.nrj);
            }

            if (proton_trigger) 
            {
              pp_proton->Fill(clover_i.nrj, clover_j.nrj);
              pp_proton->Fill(clover_j.nrj, clover_i.nrj);
            }
            else
            {
              pp_proton_veto->Fill(clover_i.nrj, clover_j.nrj);
              pp_proton_veto->Fill(clover_j.nrj, clover_i.nrj);
            }

            // Triple coincidence ppp :
            auto const & nrj_int = size_cast(clover_i.nrj);
            if (make_triple_coinc_ppp && 0 < nrj_int && nrj_int < ppp_gate_bin_max && ppp_gate_lookup[nrj_int])
            {
              for (size_t loop_j = 0; loop_j<pclovers.clean.size(); ++loop_j)
              {
                auto const & clover_j = *(pclovers.clean[loop_j]);
                if (clover_i.index() == clover_j.index()) continue;
                for (size_t loop_k = loop_j+1; loop_k<pclovers.clean.size(); ++loop_k)
                {
                  auto const & clover_k = *(pclovers.clean[loop_k]);
                  if (clover_i.index() == clover_k.index()) continue;
                  ppp_gated[ppp_id_gate_lkp[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                  ppp_gated[ppp_id_gate_lkp[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
                }
              }
            }
          }

          for (auto const & BGO_i : pclovers.BGOClean_id)
          {
            auto const & nrj_BGO = pclovers[BGO_i].nrjBGO;
            d_VS_pBGO->Fill(nrj_BGO, clover_i.nrj);
          }

          // Prompt-prompt background
          // for (auto const & index_j : last_prompt_clovers.GeClean_id)
          // {
          //   auto const & clover_j = last_prompt_clovers[index_j];
          //   if (index_i == index_j) continue;
          //   pp_bckg->Fill(clover_i.nrj, clover_j.nrj);
          //   pp_bckg->Fill(clover_j.nrj, clover_i.nrj);
          // }

          // Prompt VS paris
          for (auto const module : pparis.modules) p_VS_pparis_mod->Fill(module->nrj, clover_i.nrj);
          for (auto const module : dparis.modules) p_VS_dparis_mod->Fill(module->nrj, clover_i.nrj);
        }

        /////// DELAYED CLEAN ///////
        for (size_t loop_i = 0; loop_i<dclovers.clean.size(); ++loop_i)
        {
          auto const & clover_i = *(dclovers.clean[loop_i]);
          auto const & index_i = clover_i.index();
          d->Fill(clover_i.nrj);
          auto const & prompt_veto = (!dssd_trigger && PM == 0);
          if (prompt_veto) d_prompt_veto->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);

          // Find the coincident gamma to create the calorimetry :
          double DC = 0;
          int DM = 0;
          std::vector<const CloverModule*> coinc_clean;
          for (auto const & clover_j : dclovers.all)
          {
            if (std::abs(clover_i.time - clover_j->time) < bidimTimeWindow) 
            {
              if (clover_j->nbBGO == 0) DC+=smear(clover_j->calo, random);
              else DC+=clover_j->calo;
              ++DM;
              if (clover_j->isCleanGe() && clover_j!=&clover_i) coinc_clean.push_back(clover_j);
            }
          }

          auto const & CMult = coinc_clean.size()+1;

          if (CMult == 2)
          {
            auto const & clover_j = *(coinc_clean[0]);
            auto const & sum = clover_j.nrj + clover_i.nrj;
            d_sumC2->Fill(sum);
            d_VS_sumC2->Fill(sum, clover_j.nrj);
            d_VS_sumC2->Fill(sum, clover_i.nrj);
            if (prompt_veto) 
            {
              d_sumC2_prompt_veto->Fill(sum);
              d_VS_sumC2_prompt_veto->Fill(sum, clover_j.nrj);
              d_VS_sumC2_prompt_veto->Fill(sum, clover_i.nrj);
            }
            if (dssd_trigger)
            {
              d_VS_sumC2_p->Fill(sum, clover_j.nrj);
              d_VS_sumC2_p->Fill(sum, clover_i.nrj);
              if (prompt_veto)
              {
                d_VS_sumC2_p_prompt_veto->Fill(sum, clover_j.nrj);
                d_VS_sumC2_p_prompt_veto->Fill(sum, clover_i.nrj);
              }
            }
          }

          std::vector<const SimpleParisModule*> coincident_paris;
          for (auto const & module : dparis.modules)
          {
            if (std::abs(clover_i.time - module->time) < bidimTimeWindow) 
            {
              DC+=module->nrj;
              ++DM;
              coincident_paris.push_back(module);
            }
          }

          bool PM4DM4 = (DM>0 && PM>0 && PM<4 && DM<4);

          if (2740_keV < clover_i.nrj && clover_i.nrj < 2760_keV) hit_pattern_2755->Fill(index_i);
          
          d_VS_PM->Fill(PM, clover_i.nrj);
          d_VS_DM->Fill(DM, clover_i.nrj);
          
          if (PM > 1) 
          {
            d_VS_PC -> Fill(PC, clover_i.nrj);
          }

          d_VS_DC_VS_PC -> Fill(PC, DC, clover_i.nrj);
          d_VS_PM_VS_PC -> Fill(PC, PM, clover_i.nrj);
          d_VS_DM_VS_DC -> Fill(DC, DM, clover_i.nrj);
          d_VS_DM_VS_PM -> Fill(PM, DM, clover_i.nrj);

          if (prompt_veto)  d_VS_DM_VS_DC_particleveto -> Fill(DC, DM, clover_i.nrj);
          if (!dssd_trigger)
          {
            d_VS_DC_VS_PC_particleveto -> Fill(PC, DC, clover_i.nrj);
            d_VS_PM_VS_PC_particleveto -> Fill(PC, PM, clover_i.nrj);
            d_VS_DM_VS_PM_particleveto -> Fill(PM, DM, clover_i.nrj);
          }

          if (DM > 1) d_VS_DC -> Fill(DC, clover_i.nrj);

          if (PM > 0) 
          {
            d_P->Fill(clover_i.nrj);

            if (PC < 5_MeV) d_PC5->Fill(clover_i.nrj);
            if (PC < 3_MeV) d_PC3->Fill(clover_i.nrj);
            if (PC < 2_MeV) d_PC2->Fill(clover_i.nrj);
          }
          if (DC < 3_MeV) 
          {
            d_DC3->Fill(clover_i.nrj);
            if (PM > 0 && PC < 3_MeV) 
            {
              d_PC3DC3->Fill(clover_i.nrj);
              if (PM < 4 && DM < 4) d_PC3PM3DC3DM3->Fill(clover_i.nrj);
            }
            if (1_MeV < DC) 
            {
              d_DC1_3->Fill(clover_i.nrj);
              if (PM > 0 && PC < 3_MeV) d_PC3DC1_3->Fill(clover_i.nrj);
            }
          }
          if(proton_trigger) 
          {
            d_proton->Fill(clover_i.nrj);
            if (PM>0) d_proton_P->Fill(clover_i.nrj);
            if (PM>0 && PM<4) d_proton_PM1_3->Fill(clover_i.nrj);
          }
          if (PM==0 || !proton_trigger) d_proton_P_veto->Fill(clover_i.nrj);
          if(Ex_U6 > 0)  d_Ex->Fill(clover_i.nrj);
          if (PM > 0 && Ex_U6 > 0)
          {
            d_VS_Ex_U6__P->Fill(Ex_U6, clover_i.nrj);
            d_ExP->Fill(clover_i.nrj);
          }

          // Delayed-delayed :
          // for (auto const & clover_j_it : coinc_clean)
          for (size_t loop_j = loop_i+1; loop_j<dclovers.clean.size(); ++loop_j)
          {
            auto const & clover_j = *(dclovers.clean[loop_j]);
            // auto const & index_j = clover_j.index()();
            // auto const & clover_j = *clover_j_it;
            if (std::abs(clover_i.time-clover_j.time)>bidimTimeWindow) continue;
            dd->Fill(clover_i.nrj, clover_j.nrj);
            dd->Fill(clover_j.nrj, clover_i.nrj);

            ddt->Fill(clover_i.nrj, clover_j.nrj, clover_j.time);
            ddt->Fill(clover_j.nrj, clover_i.nrj, clover_j.time);

            if (prompt_veto)
            {
              ddt_veto->Fill(clover_i.nrj, clover_j.nrj, clover_j.time);
              ddt_veto->Fill(clover_j.nrj, clover_i.nrj, clover_j.time);
            }
            
            if (!dssd_trigger)
            {
              dd_particle_veto->Fill(clover_i.nrj, clover_j.nrj);
              dd_particle_veto->Fill(clover_j.nrj, clover_i.nrj);

              if (PM == 0)
              {
                dd_mega_prompt_veto->Fill(clover_i.nrj, clover_j.nrj);
                dd_mega_prompt_veto->Fill(clover_j.nrj, clover_i.nrj);
                dd_prompt_veto->Fill(clover_i.nrj, clover_j.nrj);
                dd_prompt_veto->Fill(clover_j.nrj, clover_i.nrj);
              }
              else
              {
                if (PM > 3 || PC > 4_MeV || DC > 4 || DM > 4)
                {
                  dd_mega_prompt_veto->Fill(clover_i.nrj, clover_j.nrj);
                  dd_mega_prompt_veto->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
            }
            
            if (dssd_trigger)
            {
              dd_p->Fill(clover_i.nrj, clover_j.nrj);
              dd_p->Fill(clover_j.nrj, clover_i.nrj);
              if (PM>0)
              {
                dd_pP->Fill(clover_i.nrj, clover_j.nrj);
                dd_pP->Fill(clover_j.nrj, clover_i.nrj);
                if (PM4DM4)
                {
                  dd_pPM4DM4->Fill(clover_i.nrj, clover_j.nrj);
                  dd_pPM4DM4->Fill(clover_j.nrj, clover_i.nrj);
                  if (PC<3)
                  {
                    dd_pPM4PC3DM4->Fill(clover_i.nrj, clover_j.nrj);
                    dd_pPM4PC3DM4->Fill(clover_j.nrj, clover_i.nrj);
                    if (Ex_U6>0)
                    {
                      dd_pPM4PC3DM4Ex->Fill(clover_i.nrj, clover_j.nrj);
                      dd_pPM4PC3DM4Ex->Fill(clover_j.nrj, clover_i.nrj);
                    }
                  }
                }
              }
              else 
              {
                dd_p_prompt_veto->Fill(clover_i.nrj, clover_j.nrj);
                dd_p_prompt_veto->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
            
            if (PM > 0)
            {
              dd_P->Fill(clover_i.nrj, clover_j.nrj);
              dd_P->Fill(clover_j.nrj, clover_i.nrj);
              if (PM4DM4)
              {
                dd_PM4DM4->Fill(clover_i.nrj, clover_j.nrj);
                dd_PM4DM4->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
            if (PM > 0 && PC < 5_MeV)
            {
              dd_PC5->Fill(clover_i.nrj, clover_j.nrj);
              dd_PC5->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (PM > 0 && PC < 3_MeV)
            {
              dd_PC3->Fill(clover_i.nrj, clover_j.nrj);
              dd_PC3->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (PM > 0 && PC < 2_MeV)
            {
              dd_PC2->Fill(clover_i.nrj, clover_j.nrj);
              dd_PC2->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (DC < 3_MeV)
            {
              dd_DC3->Fill(clover_i.nrj, clover_j.nrj);
              dd_DC3->Fill(clover_j.nrj, clover_i.nrj);
              if (1_MeV < DC)
              {
                dd_DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                dd_DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                if (PM > 0 && PC < 3_MeV)
                {
                  dd_PC3DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                  dd_PC3DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
              if (PM > 0 && PC < 3_MeV)
              {
                dd_PC3DC3->Fill(clover_i.nrj, clover_j.nrj);
                dd_PC3DC3->Fill(clover_j.nrj, clover_i.nrj);
                if (PM < 4 && DM < 4)
                {
                  dd_PC3PM3DC3DM3->Fill(clover_i.nrj, clover_j.nrj);
                  dd_PC3PM3DC3DM3->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
            }
            if(proton_trigger)
            {
              dd_proton->Fill(clover_i.nrj, clover_j.nrj);
              dd_proton->Fill(clover_j.nrj, clover_i.nrj);
              if (PM==0)
              {
                dd_proton_P->Fill(clover_i.nrj, clover_j.nrj);
                dd_proton_P->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
            if (PM==0 && !proton_trigger)
            {
                dd_proton_P_veto->Fill(clover_i.nrj, clover_j.nrj);
                dd_proton_P_veto->Fill(clover_j.nrj, clover_i.nrj);
            }
            if ((PM==0 || PM >3) && !proton_trigger)
            {
              dd_proton_PM1_3_veto->Fill(clover_i.nrj, clover_j.nrj);
              dd_proton_PM1_3_veto->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (PM > 0 && Ex_U6 > 0)
            {
              dd_ExP->Fill(clover_i.nrj, clover_j.nrj);
              dd_ExP->Fill(clover_j.nrj, clover_i.nrj);
              if (gate(4.0_MeV, Emiss, 6.5_MeV))
              {
                dd_ExSIP->Fill(clover_i.nrj, clover_j.nrj);
                dd_ExSIP->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
            if (gate(640_keV, clover_i.nrj, 644_keV))
            {
              if (gate(203_keV, clover_j.nrj, 207_keV)) 
              {
                dT_642_VS_205->Fill(clover_i.time-clover_j.time);
                for (auto const & clover_k : coinc_clean)
                {
                  if (clover_k->index()!=clover_j.index()
                   && clover_k->index()!=clover_i.index()
                   && gate(103_keV, clover_k->nrj, 107_keV))
                  {
                    dT_205_VS_104__d642->Fill(clover_i.time-clover_j.time);
                  }
                }
              }
              if (gate(307_keV, clover_j.nrj, 311_keV)) dT_642_VS_309->Fill(clover_i.time-clover_j.time);
              if (gate(277_keV, clover_j.nrj, 281_keV)) dT_642_VS_279->Fill(clover_i.time-clover_j.time);
              if (gate(242_keV, clover_j.nrj, 246_keV)) dT_642_VS_244->Fill(clover_i.time-clover_j.time);
              if (gate(299_keV, clover_j.nrj, 303_keV)) dT_642_VS_301->Fill(clover_i.time-clover_j.time);
              dT_642_VS_d->Fill(clover_j.nrj, clover_i.time-clover_j.time);
              if (dssd_trigger)
              {
                E_dT_p__642d->Fill(clover_j.time, clover_j.nrj);
              }
            }
          }

          // Background delayed-delayed :
          // for (auto const & clover_j : last_delayed_clovers.clean)
          // {
          //   dd_bckg->Fill(clover_i.nrj, clover_j->nrj);
          //   dd_bckg->Fill(clover_j->nrj, clover_i.nrj);
          // }

          // Gated delayed-delayed (=triple delayed coincidence)
          auto const & nrj_int = size_cast(clover_i.nrj);
          if (make_triple_coinc_ddd && 0 < nrj_int && nrj_int < ddd_gate_bin_max && ddd_gate_lookup[nrj_int])
          {
            for (size_t loop_j = 0; loop_j<coinc_clean.size(); ++loop_j)
            {
              auto const & clover_j = *(coinc_clean[loop_j]);
              if (clover_i.index() == clover_j.index() || std::abs(clover_i.time-clover_j.time) > bidimTimeWindow) continue;
              for (size_t loop_k = loop_j+1; loop_k<coinc_clean.size(); ++loop_k)
              {
                auto const & clover_k = *(coinc_clean[loop_k]);
                if (clover_i.index() == clover_k.index() || std::abs(clover_i.time-clover_k.time) > bidimTimeWindow) continue;
                ddd_gated[ddd_id_gate_lkp[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                ddd_gated[ddd_id_gate_lkp[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
              }
            }
          }

          // Gated prompt-prompt (=triple coincidence delayed prompt-prompt)
          if (make_triple_coinc_dpp && 0 < nrj_int && nrj_int < dpp_gate_bin_max && dpp_gate_lookup[nrj_int])
          {
            for (size_t loop_j = 0; loop_j<pclovers.clean.size(); ++loop_j)
            {
              auto const & clover_j = *(pclovers.clean[loop_j]);
              for (size_t loop_k = loop_j+1; loop_k<pclovers.clean.size(); ++loop_k)
              {
                auto const & clover_k = *(pclovers.clean[loop_k]);
                dpp_gated[dpp_id_gate_lkp[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                dpp_gated[dpp_id_gate_lkp[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
              }
            }
          }

          // Prompt-delayed :
          for (size_t loop_j = 0; loop_j<pclovers.GeClean_id.size(); ++loop_j)
          {
            auto const & clover_j = *(pclovers.clean[loop_j]);
            dp->Fill(clover_j.nrj, clover_i.nrj);
            dpt->Fill(clover_j.nrj, clover_i.nrj, clover_i.time);
            dpt_veto->Fill(clover_j.nrj, clover_i.nrj, clover_i.time);
            if (dssd_trigger) dp_p->Fill(clover_j.nrj, clover_i.nrj);
            else 
            {
              dp_particle_veto->Fill(clover_j.nrj, clover_i.nrj);
              if (PM>1) d_VS_p_VS_PC_particleveto -> Fill(PC, clover_j.nrj, clover_i.nrj);
            }
            if (PM>1) d_VS_p_VS_PC -> Fill(PC, clover_j.nrj, clover_i.nrj);
            // if (PM > 0 && PC < 5_MeV) dp_PC5->Fill(clover_j.nrj, clover_i.nrj);
            // if (PM > 0 && PC < 3_MeV) dp_PC3->Fill(clover_j.nrj, clover_i.nrj);
            // if (PM > 0 && PC < 2_MeV) dp_PC2->Fill(clover_j.nrj, clover_i.nrj);
            // if (DC < 3_MeV) dp_DC3->Fill(clover_j.nrj, clover_i.nrj);
            if (DC < 3_MeV) 
            {
            //   // dp_DC3->Fill(clover_j.nrj, clover_i.nrj);
            //   if (1_MeV < DC) 
            //   {
            //     // dp_DC1_3->Fill(clover_j.nrj, clover_i.nrj);
            //     // if (PM > 0 && PC < 3_MeV) dp_PC3DC1_3->Fill(clover_j.nrj, clover_i.nrj);
            //   }
              if (PM > 0 && PC < 3_MeV) dp_PC3DC3->Fill(clover_j.nrj, clover_i.nrj);
            }
          }

          if (PM > 0 && PC < 5_MeV)
          {
            d_VS_DC_PC5->Fill(DC, clover_i.nrj);
            d_VS_PM_PC5->Fill(PM, clover_i.nrj);
            d_VS_DM_PC5->Fill(DM, clover_i.nrj);
          }
          if (PM > 0 && PC < 3_MeV)
          {
            d_VS_DC_PC3->Fill(DC, clover_i.nrj);
            d_VS_PM_PC3->Fill(PM, clover_i.nrj);
            d_VS_DM_PC3->Fill(DM, clover_i.nrj);
          }
          if (PM > 0 && PC < 2_MeV)
          {
            d_VS_DC_PC2->Fill(DC, clover_i.nrj);
            d_VS_PM_PC2->Fill(PM, clover_i.nrj);
            d_VS_DM_PC2->Fill(DM, clover_i.nrj);
          }
          if (DC < 3_MeV)
          {
            d_VS_DC_DC3->Fill(DC, clover_i.nrj);
            d_VS_PM_DC3->Fill(PM, clover_i.nrj);
            d_VS_DM_DC3->Fill(DM, clover_i.nrj);
            if (1_MeV < DC)
            {
              d_VS_DC_DC1_3->Fill(DC, clover_i.nrj);
              d_VS_PM_DC1_3->Fill(PM, clover_i.nrj);
              d_VS_DM_DC1_3->Fill(DM, clover_i.nrj);
              if (PM > 0 && PC < 3_MeV)
              {
                d_VS_DC_PC3DC1_3->Fill(DC, clover_i.nrj);
                d_VS_PM_PC3DC1_3->Fill(PM, clover_i.nrj);
                d_VS_DM_PC3DC1_3->Fill(DM, clover_i.nrj);
                if (PM4DM4)
                {
                  d_pPC3MP3DC3MD4->Fill(clover_i.nrj);
                  if (Ex_U6>0 && Ex_U6<6.5_MeV) d_pPC3MP3DC3MD4Ex6_5->Fill(clover_i.nrj);
                } 
              }
            }
            if (PM > 0 && PC < 3_MeV)
            {
              if (DM > 1) d_VS_DC_PC3DC3->Fill(DC, clover_i.nrj);
              d_VS_PM_PC3DC3->Fill(PM, clover_i.nrj);
              d_VS_DM_PC3DC3->Fill(DM, clover_i.nrj);
            }
          }

          //// Clover VS PARIS ////

          for (auto const & module : dparis.modules) 
          {
            d_VS_dparis_mod->Fill(module->nrj, clover_i.nrj);
            if (module->addbacked()) d_VS_daddback_mod->Fill(module->nrj, clover_i.nrj);
            if (gate(1367_keV, clover_i.nrj, 1372_keV)) 
            {
              daddback_module_gate_1370_VS_run_paris->Fill(run_number, module->nrj);
              daddback_module_gate_1370_VS_index_paris->Fill(module->index(), module->nrj);
            }
          }

          for (auto const & dclean_phos : dparis.clean_phoswitches) 
          {
            d_VS_dclean_phos->Fill(dclean_phos->nrj, clover_i.nrj);
            if (dclean_phos->isLaBr3) 
            {
              d_VS_dLaBr3->Fill(clover_i.nrj, dclean_phos->qshort);
              dT_642_Clover_VS_dLabr3->Fill(dclean_phos->qshort, dclean_phos->time-clover_i.time);// TODO
            }
          }
          for (auto const & pclean_phoswitch : pparis.clean_phoswitches) 
          {
            d_VS_pclean_phos->Fill(pclean_phoswitch->nrj, clover_i.nrj);
          }
          
          //// Clover VS BGO ////
          for (auto const & clover : dclovers.cleanBGO)
          {
            d_VS_dBGO->Fill(clover->nrjBGO, clover_i.nrj);
          }
          for (auto const & clover : dclovers.rejected)
          {
            d_VS_dBGO_addback->Fill(clover->calo, clover_i.nrj);
          }

          if (dssd_trigger)
          {
            d_p->Fill(clover_i.nrj);
            E_dT_p->Fill(clover_i.time, clover_i.nrj);
            d_VS_PM_p->Fill(PM, clover_i.nrj);
            d_VS_DM_p->Fill(DM, clover_i.nrj);
            d_VS_PC_p->Fill(PC, clover_i.nrj);
            if (DM > 1) d_VS_DC_p->Fill(DC, clover_i.nrj);
            if (PM>0)
            {
              d_pP->Fill(clover_i.nrj);
              d_VS_PC_pP->Fill(PC, clover_i.nrj);
              d_VS_DM_pP->Fill(DM, clover_i.nrj);
              if (DM > 1) d_VS_DC_pP->Fill(DC, clover_i.nrj);
              if (4_MeV < Emiss && Emiss < 6.5_MeV)
              {
                p_VS_DC_ExSIP->Fill(DC, clover_i.nrj);
                d_VS_DC_ExSIP->Fill(DC, clover_i.nrj);
              }
            }
            else d_p_prompt_veto->Fill(clover_i.nrj);
            
            if (PM > 0 && PC < 5_MeV) 
            {
              d_pPC5->Fill(clover_i.nrj);
              if (PM4DM4) d_pPC5PM4DM4->Fill(clover_i.nrj);
            }
            if (PM > 0 && PC < 5_MeV && PM4DM4) d_pPC5PM4DM4->Fill(clover_i.nrj);
            if (PM > 0 && PC < 3_MeV) d_pPC3->Fill(clover_i.nrj);
            if (PM > 0 && PC < 3_MeV && PM4DM4) d_pPC3PM4DM4->Fill(clover_i.nrj);
            if (PM > 0 && PC < 2_MeV) d_pPC2->Fill(clover_i.nrj);
            if (PM > 0 && PC < 2_MeV && PM) d_pPC2_PM4DM4->Fill(clover_i.nrj);
            if (DC < 3_MeV)
            {
              d_pDC3->Fill(clover_i.nrj);
              if (PM > 0) d_pPDC3->Fill(clover_i.nrj);
              if (PC < 5_MeV) d_pPC5DC1_3->Fill(clover_i.nrj);
              if (DC > 1_MeV) 
              {
                d_pDC1_3->Fill(clover_i.nrj);
                if (PM > 0) d_pPDC1_3->Fill(clover_i.nrj);
                if (PC < 5_MeV) d_pPC5DC3->Fill(clover_i.nrj);
              }
            }
          }
        }


        if (dssd_trigger)
        {
          p_calo_p->Fill(PC);
          // d_calo_p->Fill(DC);
          if (PM > 0 && Ex_U6>0_MeV) 
          {
            Ex_VS_PC_p->Fill(PC, Ex_U6);
            // Ex_VS_DC_p__P->Fill(DC, Ex_U6);
          }
          for (size_t loop_i = 0; loop_i<pclovers.GeClean_id.size(); ++loop_i)
          {
            auto const & index_i = pclovers.GeClean_id[loop_i];
            auto const & clover_i = pclovers[index_i];
            auto const & nrj = clover_i.nrj;
            auto const & time = clover_i.time;
            p_p->Fill(nrj);
            E_dT_p->Fill(time, nrj);
            p_VS_PM_p->Fill(PM, nrj);
            // p_VS_DM_p->Fill(DM, nrj);
            if (PM>1) p_VS_PC_p->Fill(PC, nrj);
            // p_VS_DC_p->Fill(DC, nrj);
            if (PM > 0 && PC < 5_MeV) p_pPC5->Fill(nrj);
            if (PM > 0 && PC < 3_MeV) p_pPC3->Fill(nrj);
            if (PM > 0 && PC < 2_MeV) p_pPC2->Fill(nrj);
            // if (DC < 3_MeV)
            // {
            //   p_pDC3->Fill(nrj);
            //   if (DC > 1_MeV)
            //   {
            //     p_pDC1_3->Fill(nrj);
            //   }
            // }
            if (PM > 0 && Ex_U6 > 0_MeV)
            {
              p_VS_Ex_U6->Fill(Ex_U6, nrj);
            }
          }
        }

        // Clean Ge sum
        // if (Cmult[2])
        // {
        //   auto const & Ge0 = delayed_clovers[delayed_clovers.GeClean_id[0]];
        //   auto const & Ge1 = delayed_clovers[delayed_clovers.GeClean_id[1]];
        //   auto const & Esum = Ge0.nrj + Ge1.nrj;
        //   if (abs(Ge0.time - Ge1.time) > 40_ns) continue;
        //   d_VS_sum_C2->Fill(Esum, Ge0.nrj);
        //   d_VS_sum_C2->Fill(Esum, Ge1.nrj);
        //   dd_C2_ExP->Fill(Ge0.nrj, Ge1.nrj);
        //   dd_C2_ExP->Fill(Ge1.nrj, Ge0.nrj);
        //   if (PM < 0) continue;
        //   d_VS_sum_C2_P->Fill(Esum, Ge0.nrj);
        //   d_VS_sum_C2_P->Fill(Esum, Ge1.nrj);
        
        //   if (hasContaminant(delayed_clovers) == 0)
        //   {
        //     d_VS_clean_sum_C2_P->Fill(Esum, Ge0.nrj);
        //     d_VS_clean_sum_C2_P->Fill(Esum, Ge1.nrj);
        //     if (dssd_trigger)
        //     {
        //       d_VS_clean_sum_C2_pP->Fill(Esum, Ge0.nrj);
        //       d_VS_clean_sum_C2_pP->Fill(Esum, Ge1.nrj);
        //     }
        //     if (DM == 2)
        //     {
        //       d_VS_clean_sum_C2_PDM2->Fill(Esum, Ge0.nrj);
        //       d_VS_clean_sum_C2_PDM2->Fill(Esum, Ge1.nrj);
        //       if (PM < 5)
        //       {
        //         d_VS_clean_sum_C2_PM5DM2->Fill(Esum, Ge0.nrj);
        //         d_VS_clean_sum_C2_PM5DM2->Fill(Esum, Ge1.nrj);
        //       }
        //     }
        //   }
        //   if (dssd_trigger)
        //   {
        //     d_VS_sum_C2_pP->Fill(Esum, Ge0.nrj);
        //     d_VS_sum_C2_pP->Fill(Esum, Ge1.nrj);
        //     if (Ex_U6 > 0)
        //     {
        //       d_VS_sum_C2_ExP->Fill(Esum, Ge0.nrj);
        //       d_VS_sum_C2_ExP->Fill(Esum, Ge1.nrj);
        //       if (4_MeV < Ex_U6 && Ex_U6 < 6.5_MeV)
        //       {
        //         d_VS_sum_C2_ExSIP->Fill(Esum, Ge0.nrj);
        //         d_VS_sum_C2_ExSIP->Fill(Esum, Ge1.nrj);
        //       }
        //     }
        //   }
        // }

        // // If one of the three gammas is a contaminant, then the two others become a C2
        // if (Cmult[3] && PM > 0 && hasContaminant(delayed_clovers) == 1) 
        // {
        //   float Esum = 0;
        //   for (auto const & index : delayed_clovers.GeClean_id) if (!isContaminant(delayed_clovers[index].nrj)) ++Esum;
        //   for (auto const & index : delayed_clovers.GeClean_id) if (!isContaminant(delayed_clovers[index].nrj)) 
        //   {
        //     d_VS_clean_sum_C2_P->Fill(Esum, delayed_clovers[index].nrj); 
        //     d_VS_clean_sum_C2_pP->Fill(Esum, delayed_clovers[index].nrj); 
        //     if (DM == 2) d_VS_clean_sum_C2_PDM2->Fill(Esum, delayed_clovers[index].nrj);
        //     if (DM == 2) d_VS_clean_sum_C2_PM5DM2->Fill(Esum, delayed_clovers[index].nrj);
        //   }
        // }
        
        // if (PM > 0)
        // {
        //   double sum_clean_Ge = 0;
        //   for (auto const & clover : delayed_clovers.clean) sum_clean_Ge+=clover->nrj;
        //   double sum_all_Ge = 0;
        //   for (auto const & clover_index : delayed_clovers.Ge) sum_all_Ge+=delayed_clovers[clover_index].nrj;
        //   for (auto const & clover : delayed_clovers.clean) 
        //   {
        //     if (dssd_trigger) 
        //     {
        //       if (sum_clean_Ge>0) d_VS_sum_C_pP->Fill(sum_clean_Ge, clover->nrj);
        //       if (delayed_clovers.Ge.size()>1) d_VS_sum_Ge_pP->Fill(sum_all_Ge, clover->nrj);
        //     }
        //     if (Ex_U6>0)
        //     {
        //       if (sum_clean_Ge>0)
        //       {
        //         d_VS_sum_C_ExP->Fill(sum_clean_Ge, clover->nrj);
        //         if (4_MeV < Emiss && Emiss < 6.5_MeV) d_VS_sum_C_ExSIP->Fill(sum_clean_Ge, clover->nrj);
        //       }
        //       if (delayed_clovers.Ge.size()>1)
        //       {
        //         d_VS_sum_Ge_ExP->Fill(sum_all_Ge, clover->nrj);
        //         if (4_MeV < Emiss && Emiss < 6.5_MeV) d_VS_sum_Ge_ExSIP->Fill(sum_all_Ge, clover->nrj);
        //       }
        //     }
        //   }
        //   if (sum_clean_Ge>0)
        //   {
        //     if (Ex_U6 > 0) Ex_vs_dC_sum->Fill(sum_clean_Ge, Ex_U6);
        //     if (dssd_energy > 0) Ep_vs_dC_sum_P->Fill(sum_clean_Ge, dssd_energy);
        //   }
              
        //   for (auto const & clover : pclovers.clean) if (sum_clean_Ge>0) p_VS_sum_C_pP->Fill(sum_clean_Ge, clover->nrj);
        // }
        
        // // Dirty Ge sum
        // if (Gemult[2])
        // {
        //   auto const & Ge0 = delayed_clovers[delayed_clovers.Ge[0]];
        //   auto const & Ge1 = delayed_clovers[delayed_clovers.Ge[1]];
        //   auto const & Esum = Ge0.nrj + Ge1.nrj;
        //   dirty_d_VS_sum_C2->Fill(Esum, Ge0.nrj);
        //   dirty_d_VS_sum_C2->Fill(Esum, Ge1.nrj);
        //   if (PM > 0)
        //   {
        //     dirty_d_VS_sum_C2_P->Fill(Esum, Ge0.nrj);
        //     dirty_d_VS_sum_C2_P->Fill(Esum, Ge1.nrj);
        //   }
        //   if (dssd_trigger && PM > 0)
        //   {
        //     dirty_d_VS_sum_C2_pP->Fill(Esum, Ge0.nrj);
        //     dirty_d_VS_sum_C2_pP->Fill(Esum, Ge1.nrj);
        //     if (Ex_U6 > 0)
        //     {
        //       dirty_d_VS_sum_C2_ExP->Fill(Esum, Ge0.nrj);
        //       dirty_d_VS_sum_C2_ExP->Fill(Esum, Ge1.nrj);
        //       if (4_MeV < Ex_U6 && Ex_U6 < 6.5_MeV)
        //       {
        //         dirty_d_VS_sum_C2_ExSIP->Fill(Esum, Ge0.nrj);
        //         dirty_d_VS_sum_C2_ExSIP->Fill(Esum, Ge1.nrj);
        //       }
        //     }
        //   }
        // }
        
        // if (PM > 0 && delayed_Ge_mult > 1)
        // {
        //   double sum_all_Ge = 0;
        //   for (auto const & index : delayed_clovers.Ge) sum_all_Ge+=delayed_clovers[index].nrj;
        //   for (auto const & clover : delayed_clovers.clean) 
        //   {
        //     if (dssd_trigger) dirty_d_VS_sum_C_pP->Fill(sum_all_Ge, clover->nrj);
        //     if (Ex_U6>0)
        //     {
        //       dirty_d_VS_sum_C_ExP->Fill(sum_all_Ge, clover->nrj);
        //       if (4_MeV < Emiss && Emiss < 6.5_MeV) dirty_d_VS_sum_C_ExSIP->Fill(sum_all_Ge, clover->nrj);
        //     }
        //   }
        // }


        /////// PARIS //////////


          for (auto const & pclean_phos : pparis.clean_phoswitches) if (pclean_phos->isLaBr3)
          {
            pLaBr3_VS_label->Fill(Paris::index[pclean_phos->label], pclean_phos->nrj);
            LaBr3_prompt->Fill(pclean_phos->nrj);
          }
          for (auto const & dclean_phos : dparis.clean_phoswitches) if (dclean_phos->isLaBr3)
          {
            dLaBr3_VS_label->Fill(Paris::index[dclean_phos->label], dclean_phos->nrj);
            LaBr3_delayed->Fill(dclean_phos->nrj);
          }


        // double sum = 0;
        // for (auto const & id : pparis.back.modules_id)  phoswitches_prompt ->Fill(pparis.back.modules[id].nrj);
        // for (auto const & id : pparis.front.modules_id)  phoswitches_prompt ->Fill(pparis.front.modules[id].nrj);
        // for (auto const & id : dparis.back.modules_id)  
        // {
        //   auto const & nrj = dparis.back.modules[id].nrj;
        //   sum+=nrj;
        //   phoswitches_delayed->Fill(nrj);
        // }
        // for (auto const & id : dparis.front.modules_id)  
        // {
        //   auto const & nrj = dparis.front.modules[id].nrj;
        //   sum+=nrj;
        //   phoswitches_delayed->Fill(nrj);
        // }

        if (dparisM == 2)
        {
          double sum = 0;
          for (auto const & id : dparis.back.modules_id) paris_VS_sum_paris_M2->Fill(sum, dparis.back.modules[id].nrj);
          for (auto const & id : dparis.front.modules_id) paris_VS_sum_paris_M2->Fill(sum, dparis.front.modules[id].nrj);
          if (PM > 0)
          {
            for (auto const & id : dparis.back.modules_id) paris_VS_sum_paris_M2_P->Fill(sum, dparis.back.modules[id].nrj);
            for (auto const & id : dparis.front.modules_id) paris_VS_sum_paris_M2_P->Fill(sum, dparis.front.modules[id].nrj);
            if (dssd_trigger)
            {
              for (auto const & id : dparis.back.modules_id) paris_VS_sum_paris_M2_pP->Fill(sum, dparis.back.modules[id].nrj);
              for (auto const & id : dparis.front.modules_id) paris_VS_sum_paris_M2_pP->Fill(sum, dparis.front.modules[id].nrj);
              for (auto const & id : pparis.back.modules_id) prompt_paris_VS_sum_paris_M2_pP->Fill(sum, pparis.back.modules[id].nrj);
              for (auto const & id : pparis.front.modules_id) prompt_paris_VS_sum_paris_M2_pP->Fill(sum, pparis.front.modules[id].nrj);
            }
          }
        }

        // for (auto const & index : pparis.front().HitsClean) paris_prompt -> Fill(pparis.front().modules[index].nrj);
        // for (auto const & index : pparis.back().HitsClean) paris_prompt -> Fill(pparis.back().modules[index].nrj);
        // for (auto const & index : dparis.front().HitsClean) paris_delayed -> Fill(dparis.front().modules[index].nrj);
        // for (auto const & index : dparis.back().HitsClean) paris_delayed -> Fill(dparis.back().modules[index].nrj);
        
        // for (auto const & index : pparis.front().hits_LaBr3) LaBr3_prompt -> Fill(pparis.front().phoswitches[index].nrj);
        // for (auto const & index : pparis.back().hits_LaBr3) LaBr3_prompt -> Fill(pparis.back().phoswitches[index].nrj);
        // for (auto const & index : dparis.front().hits_LaBr3) LaBr3_delayed -> Fill(dparis.front().phoswitches[index].nrj);
        // for (auto const & index : dparis.back().hits_LaBr3) LaBr3_delayed -> Fill(dparis.back().phoswitches[index].nrj);

        // last_prompt_clovers = pclovers;
        // last_delayed_clovers = delayed_clovers;

      }// End events loop

      // tree->GetEntry(--evt_i);
      total_evts_number.fetch_add(evt_i, std::memory_order_relaxed);
      
      // Handle last hit :
      auto const & timestamp_final = event.stamp;
      double run_duration_s = Time_cast(timestamp_final-timestamp_init)*1E-12;

      // Writing the file (the mutex protects potential concurency issues)
      lock_mutex lock(write_mutex);
      time_runs[run_number] = run_duration_s;

      print("run of", run_duration_s, "s");

      total_time_of_beam_s+=run_duration_s;
      File Filename(out_filename); Filename.makePath();
      print("writing spectra in", out_filename, "...");

      print("Calculate additionnal spectra :");
      
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(), "recreate"));
        file->cd();

        timestamp_hist_VS_run->Write("timestamp_hist_VS_run", TObject::kOverwrite);

        print("write standard spectra");
        // Energy :
        pp->Write("pp", TObject::kOverwrite);
        // pp_bckg->Write("pp_bckg", TObject::kOverwrite);
        Fatima_E_dT->Write("Fatima_E_dT", TObject::kOverwrite);
        p_before_correction->Write("p_before_correction", TObject::kOverwrite);
        d_before_correction->Write("d_before_correction", TObject::kOverwrite);
        p->Write("p", TObject::kOverwrite);
        n->Write("n", TObject::kOverwrite);
        // dd_clean->Write();
        d->Write("d", TObject::kOverwrite);
        d_prompt_veto->Write("d_prompt_veto", TObject::kOverwrite);
        dd->Write("dd", TObject::kOverwrite);
        // dd_bckg->Write("dd_bckg", TObject::kOverwrite);
        dd_prompt_veto->Write("dd_prompt_veto", TObject::kOverwrite);
        dd_mega_prompt_veto->Write("dd_mega_prompt_veto", TObject::kOverwrite);
        E_dT->Write("E_dT", TObject::kOverwrite);
        E_dT_phoswitch->Write("E_dT_phoswitch", TObject::kOverwrite);
        p_VS_dT_LaBr3->Write("p_VS_dT_LaBr3", TObject::kOverwrite);
        dp->Write("dp", TObject::kOverwrite);
        ddt->Write("ddt", TObject::kOverwrite);
        ddt_veto->Write("ddt_veto", TObject::kOverwrite);
        dpt->Write("dpt", TObject::kOverwrite);
        dpt->Write("dpt", TObject::kOverwrite);
        dpt_veto->Write("dpt_veto", TObject::kOverwrite);
        dp_particle_veto->Write("dp_particle_veto", TObject::kOverwrite);
        if (make_triple_coinc_ddd) for (size_t gate_index = 0; gate_index<ddd_gates.size(); ++gate_index) 
          ddd_gated[gate_index]->Write(("ddd_gate_"+std::to_string(ddd_gates[gate_index])).c_str(), TObject::kOverwrite);
        if (make_triple_coinc_dpp) for (size_t gate_index = 0; gate_index<dpp_gates.size(); ++gate_index) 
          dpp_gated[gate_index]->Write(("dpp_gate_"+std::to_string(dpp_gates[gate_index])).c_str(), TObject::kOverwrite);
        if (make_triple_coinc_ppp) for (size_t gate_index = 0; gate_index<ppp_gates.size(); ++gate_index) 
          ppp_gated[gate_index]->Write(("ppp_gate_"+std::to_string(ppp_gates[gate_index])).c_str(), TObject::kOverwrite);

        p_D->Write("p_D", TObject::kOverwrite);
        d_P->Write("d_P", TObject::kOverwrite);
        dd_particle_veto->Write("dd_particle_veto", TObject::kOverwrite);
        dd_P->Write("dd_P", TObject::kOverwrite);
        
        print("write multiplicity spectra");
        // Multiplicity :
        dd_PM4DM4->Write("dd_PM4DM4", TObject::kOverwrite);
        p_mult->Write("p_mult", TObject::kOverwrite);
        d_mult->Write("d_mult", TObject::kOverwrite);
        dp_mult->Write("dp_mult", TObject::kOverwrite);
        p_VS_PM->Write("p_VS_PM", TObject::kOverwrite);
        p_VS_DM->Write("p_VS_DM", TObject::kOverwrite);
        d_VS_PM->Write("d_VS_PM", TObject::kOverwrite);
        d_VS_PM->Write("d_VS_PM", TObject::kOverwrite);
        d_VS_DM->Write("d_VS_DM", TObject::kOverwrite);

        print("write Calorimetry spectra");
        // Calorimetry :
        p_calo->Write("p_calo", TObject::kOverwrite);
        d_calo->Write("d_calo", TObject::kOverwrite);
        dp_calo->Write("dp_calo", TObject::kOverwrite);
        d_VS_PC->Write("d_VS_PC", TObject::kOverwrite);
        d_VS_DC->Write("d_VS_DC", TObject::kOverwrite);
        d_VS_DC->Write("d_VS_DC", TObject::kOverwrite);

        d_VS_p_VS_PC->Write("d_VS_p_VS_PC", TObject::kOverwrite);
        d_VS_PM_VS_PC->Write("d_VS_PM_VS_PC", TObject::kOverwrite);
        d_VS_DM_VS_DC->Write("d_VS_DM_VS_DC", TObject::kOverwrite);
        d_VS_DM_VS_PM->Write("d_VS_DM_VS_PM", TObject::kOverwrite);
        d_VS_DC_VS_PC->Write("d_VS_DC_VS_PC", TObject::kOverwrite);
        d_VS_p_VS_PC_particleveto->Write("d_VS_p_VS_PC_particleveto", TObject::kOverwrite);
        d_VS_PM_VS_PC_particleveto->Write("d_VS_PM_VS_PC_particleveto", TObject::kOverwrite);
        d_VS_DM_VS_DC_particleveto->Write("d_VS_DM_VS_DC_particleveto", TObject::kOverwrite);
        d_VS_DM_VS_PM_particleveto->Write("d_VS_DM_VS_PM_particleveto", TObject::kOverwrite);
        d_VS_DC_VS_PC_particleveto->Write("d_VS_DC_VS_PC_particleveto", TObject::kOverwrite);

        p_calo_clover->Write("p_calo_clover", TObject::kOverwrite);
        d_calo_clover->Write("d_calo_clover", TObject::kOverwrite);
        p_calo_phoswitch->Write("p_calo_phoswitch", TObject::kOverwrite);
        d_calo_phoswitch->Write("d_calo_phoswitch", TObject::kOverwrite);
        p_calo_clover_VS_p_calo_phoswitch->Write("p_calo_clover_VS_p_calo_phoswitch", TObject::kOverwrite);
        d_calo_clover_VS_d_calo_phoswitch->Write("d_calo_clover_VS_d_calo_phoswitch", TObject::kOverwrite);


        print("write Calorimetry gated spectra");
        p_PC5->Write("p_PC5", TObject::kOverwrite);
        // // pp_PC5->Write("pp_PC5", TObject::kOverwrite);
        d_PC5->Write("d_PC5", TObject::kOverwrite);
        dd_PC5->Write("dd_PC5", TObject::kOverwrite);
        // // dp_PC5->Write("dp_PC5", TObject::kOverwrite);
        d_VS_DC_PC5->Write("d_VS_DC_PC5", TObject::kOverwrite);
        d_VS_PM_PC5->Write("d_VS_PM_PC5", TObject::kOverwrite);
        d_VS_DM_PC5->Write("d_VS_DM_PC5", TObject::kOverwrite);

        p_PC3->Write("p_PC3", TObject::kOverwrite);
        // // pp_PC3->Write("pp_PC3", TObject::kOverwrite);
        d_PC3->Write("d_PC3", TObject::kOverwrite);
        dd_PC3->Write("dd_PC3", TObject::kOverwrite);
        // // dp_PC3->Write("dp_PC3", TObject::kOverwrite);
        d_VS_DC_PC3->Write("d_VS_DC_PC3", TObject::kOverwrite);
        d_VS_PM_PC3->Write("d_VS_PM_PC3", TObject::kOverwrite);
        d_VS_DM_PC3->Write("d_VS_DM_PC3", TObject::kOverwrite);

        p_PC2->Write("p_PC2", TObject::kOverwrite);
        // // pp_PC2->Write("pp_PC2", TObject::kOverwrite);
        d_PC2->Write("d_PC2", TObject::kOverwrite);
        dd_PC2->Write("dd_PC2", TObject::kOverwrite);
        // // dp_PC2->Write("dp_PC2", TObject::kOverwrite);
        d_VS_DC_PC2->Write("d_VS_DC_PC2", TObject::kOverwrite);
        d_VS_PM_PC2->Write("d_VS_PM_PC2", TObject::kOverwrite);
        d_VS_DM_PC2->Write("d_VS_DM_PC2", TObject::kOverwrite);

        p_DC3->Write("p_DC3", TObject::kOverwrite);
        // // pp_DC3->Write("pp_DC3", TObject::kOverwrite);
        d_DC3->Write("d_DC3", TObject::kOverwrite);
        dd_DC3->Write("dd_DC3", TObject::kOverwrite);
        // // dp_DC3->Write("dp_DC3", TObject::kOverwrite);
        d_VS_DC_DC3->Write("d_VS_DC_DC3", TObject::kOverwrite);
        d_VS_PM_DC3->Write("d_VS_PM_DC3", TObject::kOverwrite);
        d_VS_DM_DC3->Write("d_VS_DM_DC3", TObject::kOverwrite);

        p_DC1_3->Write("p_DC1_3", TObject::kOverwrite);
        // // pp_DC1_3->Write("pp_DC1_3", TObject::kOverwrite);
        d_DC1_3->Write("d_DC1_3", TObject::kOverwrite);
        dd_DC1_3->Write("dd_DC1_3", TObject::kOverwrite);
        // // dp_DC1_3->Write("dp_DC1_3", TObject::kOverwrite);
        d_VS_DC_DC1_3->Write("d_VS_DC_DC1_3", TObject::kOverwrite);
        d_VS_PM_DC1_3->Write("d_VS_PM_DC1_3", TObject::kOverwrite);
        d_VS_DM_DC1_3->Write("d_VS_DM_DC1_3", TObject::kOverwrite);

        p_PC3DC3->Write("p_PC3DC3", TObject::kOverwrite);
        // // pp_PC3DC3->Write("pp_PC3DC3", TObject::kOverwrite);
        d_PC3DC3->Write("d_PC3DC3", TObject::kOverwrite);
        d_PC3PM3DC3DM3->Write("d_PC3PM3DC3DM3", TObject::kOverwrite);
        dd_PC3DC3->Write("dd_PC3DC3", TObject::kOverwrite);
        dd_PC3PM3DC3DM3->Write("dd_PC3PM3DC3DM3", TObject::kOverwrite);
        dp_PC3DC3->Write("dp_PC3DC3", TObject::kOverwrite);
        d_VS_DC_PC3DC3->Write("d_VS_DC_PC3DC3", TObject::kOverwrite);
        d_VS_PM_PC3DC3->Write("d_VS_PM_PC3DC3", TObject::kOverwrite);
        d_VS_DM_PC3DC3->Write("d_VS_DM_PC3DC3", TObject::kOverwrite);

        p_PC3DC1_3->Write("p_PC3DC1_3", TObject::kOverwrite);
        // // pp_PC3DC1_3->Write("pp_PC3DC1_3", TObject::kOverwrite);
        d_PC3DC1_3->Write("d_PC3DC1_3", TObject::kOverwrite);
        dd_PC3DC1_3->Write("dd_PC3DC1_3", TObject::kOverwrite);
        // // dp_PC3DC1_3->Write("dp_PC3DC1_3", TObject::kOverwrite);
        d_VS_DC_PC3DC1_3->Write("d_VS_DC_PC3DC1_3", TObject::kOverwrite);
        d_VS_PM_PC3DC1_3->Write("d_VS_PM_PC3DC1_3", TObject::kOverwrite);
        d_VS_DM_PC3DC1_3->Write("d_VS_DM_PC3DC1_3", TObject::kOverwrite);
        d_pPC3MP3DC3MD4->Write("d_pPC3MP3DC3MD4", TObject::kOverwrite);
        d_pPC3MP3DC3MD4Ex6_5->Write("d_pPC3MP3DC3MD4Ex6_5", TObject::kOverwrite);

        print("write particule gated spectra");

        p_p->Write("p_p", TObject::kOverwrite);
        pp_p->Write("pp_p", TObject::kOverwrite);
        d_p->Write("d_p", TObject::kOverwrite);
        d_p_prompt_veto->Write("d_p_prompt_veto", TObject::kOverwrite);
        dd_p->Write("dd_p", TObject::kOverwrite);
        dd_p_prompt_veto->Write("dd_p_prompt_veto", TObject::kOverwrite);
        dp_p->Write("dp_p", TObject::kOverwrite);
        dd_pPM4DM4->Write("dd_pPM4DM4", TObject::kOverwrite);
        dd_pPM4PC3DM4->Write("dd_pPM4PC3DM4", TObject::kOverwrite);
        dd_pPM4PC3DM4Ex->Write("dd_pPM4PC3DM4Ex", TObject::kOverwrite);
        E_dT_p->Write("E_dT_p", TObject::kOverwrite);
        E_dT_p__642d->Write("E_dT_p__642d", TObject::kOverwrite);
        p_calo_p->Write("p_calo_p", TObject::kOverwrite);
        d_calo_p->Write("d_calo_p", TObject::kOverwrite);
        
        d_pP->Write("d_pP", TObject::kOverwrite);
        dd_pP->Write("dd_pP", TObject::kOverwrite);
        d_calo_pP->Write("d_calo_pP", TObject::kOverwrite);

        p_pPC5->Write("p_pPC5", TObject::kOverwrite);
        d_pPC5->Write("d_pPC5", TObject::kOverwrite);
        d_pPC5PM4DM4->Write("d_pPC5PM4DM4", TObject::kOverwrite);
        p_pPC3->Write("p_pPC3", TObject::kOverwrite);
        d_pPC3->Write("d_pPC3", TObject::kOverwrite);
        d_pPC3PM4DM4->Write("d_pPC3PM4DM4", TObject::kOverwrite);
        p_pPC2->Write("p_pPC2", TObject::kOverwrite);
        d_pPC2->Write("d_pPC2", TObject::kOverwrite);
        d_pPC2_PM4DM4->Write("d_pPC2_PM4DM4", TObject::kOverwrite);
        p_pDC3->Write("p_pDC3", TObject::kOverwrite);
        d_pDC3->Write("d_pDC3", TObject::kOverwrite);
        d_pPDC3->Write("d_pPDC3", TObject::kOverwrite);
        p_pDC1_3->Write("p_pDC1_3", TObject::kOverwrite);
        d_pDC1_3->Write("d_pDC1_3", TObject::kOverwrite);
        d_pPDC1_3->Write("d_pPDC1_3", TObject::kOverwrite);
        d_pPC5DC3->Write("d_pPC5DC3", TObject::kOverwrite);
        d_pPC5DC1_3->Write("d_pPC5DC1_3", TObject::kOverwrite);
        
        p_VS_PM_p->Write("p_VS_PM_p", TObject::kOverwrite);
        d_VS_PM_p->Write("d_VS_PM_p", TObject::kOverwrite);
        p_VS_DM_p->Write("p_VS_DM_p", TObject::kOverwrite);
        d_VS_DM_p->Write("d_VS_DM_p", TObject::kOverwrite);
        d_VS_DM_pP->Write("d_VS_DM_pP", TObject::kOverwrite);

        p_VS_PC_p->Write("p_VS_PC_p", TObject::kOverwrite);
        d_VS_PC_p->Write("d_VS_PC_p", TObject::kOverwrite);
        d_VS_PC_pP->Write("d_VS_PC_pP", TObject::kOverwrite);
        p_VS_DC_p->Write("p_VS_DC_p", TObject::kOverwrite);
        d_VS_DC_p->Write("d_VS_DC_p", TObject::kOverwrite);
        d_VS_DC_pP->Write("d_VS_DC_pP", TObject::kOverwrite);

        d_pPC3MP3DC3MD4->Write("d_pPC3MP3DC3MD4", TObject::kOverwrite);
        d_pPC3MP3DC3MD4Ex6_5->Write("d_pPC3MP3DC3MD4Ex6_5", TObject::kOverwrite);

        dT_642_VS_205->Write("dT_642_VS_205", TObject::kOverwrite);
        dT_642_VS_309->Write("dT_642_VS_309", TObject::kOverwrite);
        dT_642_VS_279->Write("dT_642_VS_279", TObject::kOverwrite);
        dT_642_VS_244->Write("dT_642_VS_244", TObject::kOverwrite);
        dT_642_VS_301->Write("dT_642_VS_301", TObject::kOverwrite);
        dT_642_VS_d->Write("dT_642_VS_d", TObject::kOverwrite);
        dT_642_VS_d->Write("dT_642_VS_d", TObject::kOverwrite);
        dT_642_Clover_VS_dLabr3->Write("dT_642_Clover_VS_dLabr3", TObject::kOverwrite);
        dT_642_Clover_VS_dLabr3->Write("dT_642_Clover_VS_dLabr3", TObject::kOverwrite);
        dT_642_VS_dLabr3->Write("dT_642_VS_dLabr3", TObject::kOverwrite);
        dT_205_VS_104__d642->Write("dT_205_VS_104__d642", TObject::kOverwrite);

        d_sumC2->Write("d_sumC2", TObject::kOverwrite);
        d_VS_sumC2->Write("d_VS_sumC2", TObject::kOverwrite);
        d_sumC2_prompt_veto->Write("d_sumC2_prompt_veto", TObject::kOverwrite);
        d_VS_sumC2_prompt_veto->Write("d_VS_sumC2_prompt_veto", TObject::kOverwrite);
        d_VS_sumC2_p->Write("d_VS_sumC2_p", TObject::kOverwrite);
        d_VS_sumC2_p_prompt_veto->Write("d_VS_sumC2_p_prompt_veto", TObject::kOverwrite);

        print("write sum spectra");
        // d_VS_sum_C2->Write("d_VS_sum_C2", TObject::kOverwrite);
        // d_VS_sum_C2_P->Write("d_VS_sum_C2_P", TObject::kOverwrite);
        // d_VS_clean_sum_C2_P->Write("d_VS_clean_sum_C2_P", TObject::kOverwrite);
        // d_VS_clean_sum_C2_pP->Write("d_VS_clean_sum_C2_pP", TObject::kOverwrite);
        // d_VS_clean_sum_C2_PDM2->Write(" d_VS_clean_sum_C2_PDM2", TObject::kOverwrite);
        // d_VS_clean_sum_C2_PM5DM2->Write(" d_VS_clean_sum_C2_PDM2", TObject::kOverwrite);
        // d_VS_sum_C2_pP->Write("d_VS_sum_C2_pP", TObject::kOverwrite);
        // d_VS_sum_C2_ExP->Write("d_VS_sum_C2_ExP", TObject::kOverwrite);
        // d_VS_sum_C2_ExSIP->Write("d_VS_sum_C2_ExSIP", TObject::kOverwrite);
        // p_VS_sum_C_pP->Write("p_VS_sum_C_pP", TObject::kOverwrite);
        // d_VS_sum_C_pP->Write("d_VS_sum_C_pP", TObject::kOverwrite);
        // d_VS_sum_C_ExP->Write("d_VS_sum_C_ExP", TObject::kOverwrite);
        // d_VS_sum_C_ExSIP->Write("d_VS_sum_C_ExSIP", TObject::kOverwrite);
        // d_VS_sum_Ge_pP->Write("d_VS_sum_Ge_pP", TObject::kOverwrite);
        // d_VS_sum_Ge_ExP->Write("d_VS_sum_Ge_ExP", TObject::kOverwrite);
        // d_VS_sum_Ge_ExSIP->Write("d_VS_sum_Ge_ExSIP", TObject::kOverwrite);

        // dirty_d_VS_sum_C2->Write("dirty_d_VS_sum_C2", TObject::kOverwrite);
        // dirty_d_VS_sum_C2_P->Write("dirty_d_VS_sum_C2_P", TObject::kOverwrite);
        // dirty_d_VS_sum_C2_pP->Write("dirty_d_VS_sum_C2_pP", TObject::kOverwrite);
        // dirty_d_VS_sum_C2_ExP->Write("dirty_d_VS_sum_C2_ExP", TObject::kOverwrite);
        // dirty_d_VS_sum_C2_ExSIP->Write("dirty_d_VS_sum_C2_ExSIP", TObject::kOverwrite);
        // dirty_d_VS_sum_C_pP->Write("dirty_d_VS_sum_C_pP", TObject::kOverwrite);
        // dirty_d_VS_sum_C_ExP->Write("dirty_d_VS_sum_C_ExP", TObject::kOverwrite);
        // dirty_d_VS_sum_C_ExSIP->Write("dirty_d_VS_sum_C_ExSIP", TObject::kOverwrite);

        // Ex_vs_dC_sum->Write("Ex_vs_dC_sum", TObject::kOverwrite);
        // Ep_vs_dC_sum_P->Write("Ep_vs_dC_sum_P", TObject::kOverwrite);
        
        p_VS_DC_ExSIP->Write("p_VS_DC_ExSIP", TObject::kOverwrite);
        d_VS_DC_ExSIP->Write("d_VS_DC_ExSIP", TObject::kOverwrite);

        // Paris :
        // paris_prompt->Write("paris_prompt", TObject::kOverwrite);
        // paris_delayed->Write("paris_delayed", TObject::kOverwrite);
        LaBr3_prompt->Write("LaBr3_prompt", TObject::kOverwrite);
        LaBr3_delayed->Write("LaBr3_delayed", TObject::kOverwrite);
        pLaBr3_VS_label->Write("pLaBr3_VS_label", TObject::kOverwrite);
        dLaBr3_VS_label->Write("dLaBr3_VS_label", TObject::kOverwrite);

        print("Write Ex spectra");
        Ex_U6_histo->Write("Ex_U6_histo", TObject::kOverwrite);
        Ex_U6_VS_ring->Write("Ex_U6_VS_ring", TObject::kOverwrite);
        p_VS_Ex_U6->Write("p_VS_Ex_U6", TObject::kOverwrite);
        d_VS_Ex_U6__P->Write("d_VS_Ex_U6__P", TObject::kOverwrite);
        Ex_VS_PC_p->Write("Ex_VS_PC_p", TObject::kOverwrite);
        Ex_VS_DC_p__P->Write("Ex_VS_DC_p__P", TObject::kOverwrite);


        p_proton->Write("p_proton", TObject::kOverwrite);
        pp_proton->Write("pp_proton", TObject::kOverwrite);
        p_proton_veto->Write("p_proton_veto", TObject::kOverwrite);
        pp_proton_veto->Write("pp_proton_veto", TObject::kOverwrite);
        d_proton->Write("d_proton", TObject::kOverwrite);
        dd_proton->Write("dd_proton", TObject::kOverwrite);
        d_proton_P->Write("d_proton_P", TObject::kOverwrite);
        dd_proton_P->Write("dd_proton_P", TObject::kOverwrite);
        d_proton_P_veto->Write("d_proton_P_veto", TObject::kOverwrite);
        dd_proton_P_veto->Write("dd_proton_P_veto", TObject::kOverwrite);
        d_proton_PM1_3->Write("d_proton_PM1_3", TObject::kOverwrite);
        dd_proton_PM1_3_veto->Write("dd_proton_PM1_3_veto", TObject::kOverwrite);
        
        d_Ex->Write("d_Ex", TObject::kOverwrite);
        d_ExP->Write("d_ExP", TObject::kOverwrite);
        dd_ExP->Write("dd_ExP", TObject::kOverwrite);
        dd_C2_ExP->Write("dd_C2_ExP", TObject::kOverwrite);
        dd_ExSIP->Write("dd_ExSIP", TObject::kOverwrite);
        Emiss__P->Write("Emiss__P", TObject::kOverwrite);
        Emiss_VS_Edelayed_p__P->Write("Emiss_VS_Edelayed_p__P", TObject::kOverwrite);

        print("write paris spectra");

        short_vs_long_prompt->Write("short_vs_long_prompt", TObject::kOverwrite);
        short_vs_long_delayed->Write("short_vs_long_delayed", TObject::kOverwrite);
        short_over_long_VS_time->Write("short_over_long_VS_time", TObject::kOverwrite);

        p_VS_pparis_mod->Write("p_VS_pparis_mod", TObject::kOverwrite);
        p_VS_dparis_mod->Write("p_VS_dparis_mod", TObject::kOverwrite);
        d_VS_dparis_mod->Write("d_VS_dparis_mod", TObject::kOverwrite);
        d_VS_dclean_phos->Write("d_VS_dclean_phos", TObject::kOverwrite);
        d_VS_pclean_phos->Write("d_VS_pclean_phos", TObject::kOverwrite);
        d_VS_daddback_mod->Write("d_VS_daddback_mod", TObject::kOverwrite);
        daddback_module_gate_1370_VS_run_paris->Write("daddback_module_gate_1370_VS_run_paris", TObject::kOverwrite);
        daddback_module_gate_1370_VS_index_paris->Write("daddback_module_gate_1370_VS_index_paris", TObject::kOverwrite);

        paris_VS_sum_paris_M2->Write("paris_VS_sum_paris_M2", TObject::kOverwrite);
        paris_VS_sum_paris_M2_P->Write("paris_VS_sum_paris_M2_P", TObject::kOverwrite);
        paris_VS_sum_paris_M2_pP->Write("paris_VS_sum_paris_M2_pP", TObject::kOverwrite);
        prompt_paris_VS_sum_paris_M2_pP->Write("prompt_paris_VS_sum_paris_M2_pP", TObject::kOverwrite);

        phoswitches_prompt->Write("phoswitches_prompt", TObject::kOverwrite);
        phoswitches_delayed->Write("phoswitches_delayed", TObject::kOverwrite);

        d_VS_pBGO->Write("d_VS_pBGO", TObject::kOverwrite);
        d_VS_dBGO->Write("d_VS_dBGO", TObject::kOverwrite);
        d_VS_dBGO_addback->Write("d_VS_dBGO_addback", TObject::kOverwrite);

        neutron_hit_pattern->Write("neutron_hit_pattern", TObject::kOverwrite);
        hit_pattern_2755->Write("hit_pattern_2755", TObject::kOverwrite);

        print("Write run by run spectra");
        if (bidim_by_run) for (size_t it = 0; it<detectors.names().size(); ++it)
        {
          std::string name = detectors.names()[it];
          if (name == "") continue;
          if (T_vs_run[name]->Integral() > 0) T_vs_run[name]->Write(("T_vs_run_"+name).c_str(), TObject::kOverwrite);
          if (E_vs_run[name]->Integral() > 0) E_vs_run[name]->Write(("E_vs_run_"+name).c_str(), TObject::kOverwrite);
        }

      file->Close();
      print(out_filename, "written");
    }
  });
  print("Total beam time :", total_time_of_beam_s);
  print("Reading speed of", files_total_size/timer.TimeSec(), "Mo/s | ", 1.e-6*total_evts_number/timer.TimeSec(), "M events/s");
  if (nb_files<0 
  #ifdef DEBUG
    || askUserYN("Do you want to merge the files ? (y/N)")
  #endif // DEBUG
  )
  {
    std::ofstream time_file("timefile.runs");
    for (size_t run_i = 0; run_i<time_runs.size(); ++run_i) if(time_runs[run_i] > 0) time_file << run_i << " " << time_runs[run_i] << std::endl;
    time_file.close();

    std::string dest = "data/merge_"+trigger+"_"+target+".root";
    std::string source = "data/"+trigger+"/"+target+"/run_*.root";
    std::string nb_threads_str = std::to_string(nb_threads);
    std::string command = "hadd -f -j "+ nb_threads_str+ " -d . "+ dest + " " + source;
    print(command);
    gSystem->Exec(command.c_str());

  //   print("Calculate additionnal spectra :");

  //   auto f = TFile::Open(dest.c_str(), "update");

  //   auto dd_clean = static_cast<TH2F*> (static_cast<TH2F*>(f->Get("dd"))->Clone("dd_clean"));
  //   CoLib::removeVeto(dd_clean, dd_prompt_veto.get(), 501, 521);
  //   dd_clean->Write();

  //   auto dd_p_clean = static_cast<TH2F*> (static_cast<TH2F*>(f->Get("dd_p"))->Clone("dd_p_clean"));
  //   CoLib::removeVeto(dd_p_clean, dd_p_prompt_veto.get(), 501, 521);
  //   dd_p_clean->Write();

  //   // Calculate the prompt-delayed background : //TODO
  //   // auto dp_pv = static_cast<TH2F*>(f->Get("dp_prompt_veto"));
  //   // auto proj_p_pv = dp_bckg->ProjectionX(); // Projection on prompt
  //   // auto proj_pv = dp->ProjectionY();
  //   // auto test = removeVeto(dp, (TH1F*)proj_p, (TH1F*)proj_d, 505,515);
  //   // test->Draw();
  //   // auto dp_clean = static_cast<TH2F*> (static_cast<TH2F*>(f->Get("dp"))->Clone("dp_clean"));
  }
  print("run time :", timer());
}

int main(int argc, char** argv)
{
       if (argc == 1) macro5();
  else if (argc == 2) macro5(std::stoi(argv[1]));
  else if (argc == 3) macro5(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macro5(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}
// g++ -g -o exec macro5.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro5.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17