#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/Paris.hpp"
#include "../lib/Classes/Hit.hpp"
#include "ExcitationEnergy.hpp"

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

std::unordered_set<Label> CloversV2::blacklist = {46, 55, 69, 70, 80, 92, 97, 122, 129, 142, 163};
std::unordered_map<Label, double> CloversV2::maxE_Ge = 
{
  {25, 12600 }, {26, 13600 }, {27, 10500 }, {28, 7500  }, 
  {31, 11500 }, {32, 11400 }, {33, 8250  }, {34, 9000  }, 
  {37, 11000 }, {38, 11100 }, {39, 11500 }, {40, 11300 }, 
  {43, 12600 }, {44, 11900 }, {45, 11550 }, {46, 9200  }, 
  {49, 14300 }, {50, 12800 }, {51, 13500 }, {52, 12400 }, 
  {55, 5500  }, {56, 5800  }, 
                {68, 7100  }, {69, 15500 }, {70, 9500  },
  {73, 11650 }, {74, 11600 }, {75, 11800 }, {76, 11600 }, 
  {79, 11500 }, {80, 8000  }, {81, 18200 },
  {85, 7700  }, {86, 12000 }, {87, 12000 }, {88, 11600 }, 
  {91, 7900  }, {92, 10000 }, {93, 11500 }, {94, 11000 }, 
  {97, 11400 }, {98, 11400 }, {99, 11250 }, {100, 8900 }, 
  {103, 11400 }, {104, 11600 }, {105, 11600 }, {106, 11500 }, 
  {109, 12800 }, {110, 1800  }, {111, 13000 }, {112, 11300 }, 
  {115, 12800 }, {116, 11500 }, {117, 10500 }, {118, 11400 }, 
  {121, 12400 }, {122, 20000 }, {123, 10700 }, {124, 20000 }, 
  {127, 11600 }, {128, 11700 }, {129, 10000 }, {130, 11200 }, 
  {133, 11200 }, {134, 9350  }, {135, 9400  }, {136, 9500  }, 
  {139, 13200 }, {140, 12400 }, {141, 12900 }, {142, 4500  }, 
  {145, 8200  }, {146, 9600  }, {147, 9100  }, {148, 10900 }, 
  {151, 11900 }, {152, 12200 }, {153, 11300 }, {154, 12000 }, 
  {157, 9110  }, {158, 9120  }, {159, 9110  }, {160, 11700 }, 
  {163, 11000 }, {164, 11600 }, {165, 11600 }, {166, 11600 }, 
};

void macro3(int nb_files = -1, double nb_hits_read = 1.e+200, int nb_threads = 10)
{
  std::string trigger = "dC1";
  // std::string trigger = "PrM1DeC1";
  // std::string trigger = "C2";
  // std::string trigger = "P";
  bool make_triple_coinc_ddd = false;
  bool make_triple_coinc_dpp = false;
  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_hits_number(0);
  int freq_hit_display= (nb_hits_read < 2.e+7) ? 1.e+6 : 1.e+7;

  // Calibration calibNaI("../136/coeffs_NaI.calib");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  ExcitationEnergy Ex("../136/dssd_table.dat");
  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::Initialise(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  
  std::mutex write_mutex;

  size_t gate_bin_size = 2; // Take 2 keV
  std::vector<int> ddd_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991, 1750, 1836, 2115, 1846, 2125}; // keV
  std::vector<int> dpp_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991, 1750, 1836, 2115, 1846, 2125}; // keV
  std::vector<int> ppp_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991}; // keV

  auto const & ddd_gate_bin_max = maximum(ddd_gates)+gate_bin_size+1;
  std::vector<bool> ddd_gate_lookup; ddd_gate_lookup.reserve(ddd_gate_bin_max);
  std::vector<int> ddd_index_gate_lookup; ddd_index_gate_lookup.reserve(ddd_gate_bin_max);
  size_t temp_bin = 0;
  for (size_t gate_index = 0;  gate_index<ddd_gates.size(); ++gate_index)
  {
    for (; temp_bin<size_cast(ddd_gate_bin_max); ++temp_bin) 
    {
      auto const & gate = ddd_gates[gate_index];
      if (temp_bin<gate-gate_bin_size) 
      {
        ddd_gate_lookup.push_back(false);
        ddd_index_gate_lookup.push_back(0);
      }
      else if (temp_bin<gate+gate_bin_size+1) 
      {
        ddd_gate_lookup.push_back(true);
        ddd_index_gate_lookup.push_back(gate_index);
      }
      else break;
    }
  }
  auto const & dpp_gate_bin_max = maximum(dpp_gates)+gate_bin_size+1;
  std::vector<bool> dpp_gate_lookup; dpp_gate_lookup.reserve(dpp_gate_bin_max);
  temp_bin = 0;
  for (auto gate : dpp_gates) for (; temp_bin<dpp_gate_bin_max; ++temp_bin) 
  {
    if (temp_bin<gate-gate_bin_size) dpp_gate_lookup.push_back(false);
    else if (temp_bin<gate+gate_bin_size+1) dpp_gate_lookup.push_back(true);
    else break;
  }

  MTObject::parallelise_function([&](){

    TRandom* random = new TRandom();
    random->SetSeed(time(0));

    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    auto const & thread_i_str = std::to_string(thread_i);
    while(MTfiles.getNext(file))
    {
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);
      int nb_bins_Ge_singles = 10000;
      int max_bin_Ge_singles = 10000;
      int nb_bins_Ge_bidim = 4096;
      double max_bin_Ge_bidim = 4096;
      // Simple spectra :
      unique_TH1F p (new TH1F(("p_"+thread_i_str).c_str(), "prompt", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp (new TH2F(("pp_"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F pp_bckg (new TH2F(("pp_bckg_"+thread_i_str).c_str(), "gamma-gamma prompt background;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d (new TH1F(("d_"+thread_i_str).c_str(), "delayed", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd (new TH2F(("dd_"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_bckg (new TH2F(("dd_bckg_"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp (new TH2F(("dp_"+thread_i_str).c_str(), "delayed VS prompt;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F E_dT (new TH2F(("E_dT_"+thread_i_str).c_str(), "E_dT clean", 600,-100_ns,200_ns, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F E_dT_phoswitch (new TH2F(("E_dT_phoswitch_"+thread_i_str).c_str(), "E_dT_phoswitch clean", 600,-100_ns,200_ns, 2000,0,10000));
      std::vector<std::unique_ptr<TH2I>> ddd_gated;
      if (make_triple_coinc_ddd) for (auto const & gate : ddd_gates) ddd_gated.push_back(std::unique_ptr<TH2I>(new TH2I(concatenate("ddd_gated_on_", gate, "_", (thread_i) + " delayed").c_str(), 
                                        ("gamma-gamma delayed gated on "+std::to_string(gate)+" delayed; delayed [keV];delayed [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim)));
      std::vector<std::unique_ptr<TH2I>> dpp_gated;
      if (make_triple_coinc_dpp) for (auto const & gate : dpp_gates) dpp_gated.push_back(std::unique_ptr<TH2I>(new TH2I(concatenate("dpp_gated_on_", gate, "_", (thread_i) + " delayed").c_str(),
                                        ("gamma-gamma prompt gated on "+std::to_string(gate)+"delayed;prompt [keV];prompt [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim)));
      // std::vector<TH2I*> pp_gated;
      // for (auto const & gate : ppp_gates) pp_gated.push_back(new TH2I(("pp_gated_"+std::to_string(gate)+"_"+thread_i_str).c_str(), 
      //                                   ("gamma-gamma prompt gated on "+std::to_string(gate)+";E1 [keV];E2 [keV]").c_str(), nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      // General codes : P = prompt, D = delayed, p = particle
      // Multiplicity : (codes : PM = prompt multiplicity ; DM = delayed multiplicity)
      unique_TH1F p_D (new TH1F(("p_D"+thread_i_str).c_str(), "prompt with delayed; mult", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_P (new TH1F(("d_P"+thread_i_str).c_str(), "delayed with prompt; mult", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_P (new TH2F(("dd_P_"+thread_i_str).c_str(), "gamma-gamma delayed with prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F p_mult (new TH1F(("p_mult_"+thread_i_str).c_str(), "prompt multiplicity; mult", 20,0,20));
      unique_TH1F d_mult (new TH1F(("d_mult"+thread_i_str).c_str(), "delayed multiplicity; mult", 20,0,20));
      unique_TH2F dp_mult (new TH2F(("dp_mult"+thread_i_str).c_str(), "delayed VS prompt multiplicity; prompt mult; delayed mult", 20,0,20, 20,0,20));
      unique_TH2F p_VS_PM (new TH2F(("p_VS_PM_"+thread_i_str).c_str(), "prompt Ge VS prompt multiplicity;prompt mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM (new TH2F(("d_VS_PM_"+thread_i_str).c_str(), "delayed Ge VS prompt multiplicity;prompt mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_DM (new TH2F(("p_VS_DM_"+thread_i_str).c_str(), "prompt Ge VS delayed multiplicity;delayed mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM (new TH2F(("d_VS_DM_"+thread_i_str).c_str(), "delayed Ge VS delayed multiplicity;delayed mult; E[keV]", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
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

      // Condition Prompt Calorimetry < 5 MeV (code PC5):
      unique_TH1F p_PC5 (new TH1F(("p_PC5_"+thread_i_str).c_str(), "prompt Ge PC5", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC5 (new TH2F(("pp_PC5_"+thread_i_str).c_str(), "gamma-gamma prompt PC5;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC5 (new TH1F(("d_PC5_"+thread_i_str).c_str(), "delayed PC5", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_PC5 (new TH2F(("dd_PC5_"+thread_i_str).c_str(), "gamma-gamma delayed PC5;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC5 (new TH2F(("dp_PC5_"+thread_i_str).c_str(), "delayed VS prompt PC5;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC5 (new TH2F(("d_VS_DC_PC5_"+thread_i_str).c_str(), "d_VS_DC_PC5", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC5 (new TH2F(("d_VS_PM_PC5_"+thread_i_str).c_str(), "d_VS_PM_PC5", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC5 (new TH2F(("d_VS_DM_PC5_"+thread_i_str).c_str(), "d_VS_DM_PC5", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition Prompt Calorimetry < 3 MeV (code PC3):
      unique_TH1F p_PC3 (new TH1F(("p_PC3_"+thread_i_str).c_str(), "prompt PC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3 (new TH2F(("pp_PC3_"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3 (new TH1F(("d_PC3_"+thread_i_str).c_str(), "delayed PC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_PC3 (new TH2F(("dd_PC3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC3 (new TH2F(("dp_PC3_"+thread_i_str).c_str(), "delayed VS prompt PC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3 (new TH2F(("d_VS_DC_PC3_"+thread_i_str).c_str(), "d_VS_DC_PC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3 (new TH2F(("d_VS_PM_PC3_"+thread_i_str).c_str(), "d_VS_PM_PC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3 (new TH2F(("d_VS_DM_PC3_"+thread_i_str).c_str(), "d_VS_DM_PC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition Prompt Calorimetry < 2 MeV (code PC2):
      unique_TH1F p_PC2 (new TH1F(("p_PC2_"+thread_i_str).c_str(), "prompt PC2", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC2 (new TH2F(("pp_PC2_"+thread_i_str).c_str(), "gamma-gamma prompt PC2;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC2 (new TH1F(("d_PC2_"+thread_i_str).c_str(), "delayed PC2", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_PC2 (new TH2F(("dd_PC2_"+thread_i_str).c_str(), "gamma-gamma delayed PC2;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC2 (new TH2F(("dp_PC2_"+thread_i_str).c_str(), "delayed VS prompt PC2;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC2 (new TH2F(("d_VS_DC_PC2_"+thread_i_str).c_str(), "d_VS_DC_PC2", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC2 (new TH2F(("d_VS_PM_PC2_"+thread_i_str).c_str(), "d_VS_PM_PC2", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC2 (new TH2F(("d_VS_DM_PC2_"+thread_i_str).c_str(), "d_VS_DM_PC2", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      
      // Condition Delayed Calorimetry < 3 MeV (code DC3):
      unique_TH1F p_DC3 (new TH1F(("p_DC3_"+thread_i_str).c_str(), "prompt DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_DC3 (new TH2F(("pp_DC3_"+thread_i_str).c_str(), "gamma-gamma prompt DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_DC3 (new TH1F(("d_DC3_"+thread_i_str).c_str(), "delayed DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_DC3 (new TH2F(("dd_DC3_"+thread_i_str).c_str(), "gamma-gamma delayed DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_DC3 (new TH2F(("dp_DC3_"+thread_i_str).c_str(), "delayed VS prompt DC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_DC3 (new TH2F(("d_VS_DC_DC3_"+thread_i_str).c_str(), "d_VS_DC_DC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_DC3 (new TH2F(("d_VS_PM_DC3_"+thread_i_str).c_str(), "d_VS_PM_DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_DC3 (new TH2F(("d_VS_DM_DC3_"+thread_i_str).c_str(), "d_VS_DM_DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      
      // Condition 1 < Delayed Calorimetry < 3 MeV (code DC1_3):
      unique_TH1F p_DC1_3 (new TH1F(("p_DC1_3_"+thread_i_str).c_str(), "prompt DC1_3", nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F pp_DC1_3 (new TH2F(("pp_DC1_3_"+thread_i_str).c_str(), "gamma-gamma prompt DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_DC1_3 (new TH1F(("d_DC1_3_"+thread_i_str).c_str(), "delayed DC1_3", nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dd_DC1_3 (new TH2F(("dd_DC1_3_"+thread_i_str).c_str(), "gamma-gamma delayed DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_DC1_3 (new TH2F(("dp_DC1_3_"+thread_i_str).c_str(), "delayed VS prompt DC1_3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_DC1_3 (new TH2F(("d_VS_DC_DC1_3_"+thread_i_str).c_str(), "d_VS_DC_DC1_3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_DC1_3 (new TH2F(("d_VS_PM_DC1_3_"+thread_i_str).c_str(), "d_VS_PM_DC1_3", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DM_DC1_3 (new TH2F(("d_VS_DM_DC1_3_"+thread_i_str).c_str(), "d_VS_DM_DC1_3", 20,0,20, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      // Condition Delayed Calorimetry < 3 MeV AND Prompt Calorimetry < 3 MeV (code PC3DC3):
      unique_TH1F p_PC3DC3 (new TH1F(("p_PC3DC3_"+thread_i_str).c_str(), "prompt PC3DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3DC3 (new TH2F(("pp_PC3DC3_"+thread_i_str).c_str(), "gamma-gamma prompt PC3DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3DC3 (new TH1F(("d_PC3DC3_"+thread_i_str).c_str(), "delayed PC3DC3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_PC3DC3 (new TH2F(("dd_PC3DC3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3DC3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp_PC3DC3 (new TH2F(("dp_PC3DC3_"+thread_i_str).c_str(), "delayed VS prompt PC3DC3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3DC3 (new TH2F(("d_VS_DC_PC3DC3_"+thread_i_str).c_str(), "d_VS_DC_PC3DC3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3DC3 (new TH2F(("d_VS_PM_PC3DC3_"+thread_i_str).c_str(), "d_VS_PM_PC3DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3DC3 (new TH2F(("d_VS_DM_PC3DC3_"+thread_i_str).c_str(), "d_VS_DM_PC3DC3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Condition 1 < Delayed Calorimetry < 3 MeV AND Prompt Calorimetry < 3 MeV (code PC3DC1_3):
      unique_TH1F p_PC3DC1_3 (new TH1F(("p_PC3DC1_3_"+thread_i_str).c_str(), "prompt PC3DC1_3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F pp_PC3DC1_3 (new TH2F(("pp_PC3DC1_3_"+thread_i_str).c_str(), "gamma-gamma prompt PC3DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_PC3DC1_3 (new TH1F(("d_PC3DC1_3_"+thread_i_str).c_str(), "delayed PC3DC1_3", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      // unique_TH2F dd_PC3DC1_3 (new TH2F(("dd_PC3DC1_3_"+thread_i_str).c_str(), "gamma-gamma delayed PC3DC1_3;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      // unique_TH2F dp_PC3DC1_3 (new TH2F(("dp_PC3DC1_3_"+thread_i_str).c_str(), "delayed VS prompt PC3DC1_3;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_PC3DC1_3 (new TH2F(("d_VS_DC_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_DC_PC3DC1_3", 1000,0,10000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_PM_PC3DC1_3 (new TH2F(("d_VS_PM_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_PM_PC3DC1_3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_DM_PC3DC1_3 (new TH2F(("d_VS_DM_PC3DC1_3_"+thread_i_str).c_str(), "d_VS_DM_PC3DC1_3", 20,0,20, nb_bins_Ge_singles,0,max_bin_Ge_singles));

      // Paris
      // unique_TH1F paris_prompt (new TH1F(("paris_prompt_"+thread_i_str).c_str(), "paris_prompt;keV", 10000,0,20000));
      // unique_TH1F paris_delayed (new TH1F(("paris_delayed_"+thread_i_str).c_str(), "paris_delayed;keV", 10000,0,20000));
      // unique_TH1F LaBr3_prompt (new TH1F(("LaBr3_prompt_"+thread_i_str).c_str(), "LaBr3_prompt;keV", 10000,0,20000));
      // unique_TH1F LaBr3_delayed (new TH1F(("LaBr3_delayed_"+thread_i_str).c_str(), "LaBr3_delayed;keV", 10000,0,20000));

      unique_TH2F short_vs_long_prompt (new TH2F(("short_vs_long_prompt_"+thread_i_str).c_str(), "short_vs_long_prompt;keV", 1000,0,10000, 1000,0,10000));
      unique_TH2F short_vs_long_delayed (new TH2F(("short_vs_long_delayed_"+thread_i_str).c_str(), "short_vs_long_delayed;keV", 1000,0,10000, 1000,0,10000));
      unique_TH1F phoswitches_prompt (new TH1F(("phoswitches_prompt_"+thread_i_str).c_str(), "Phoswitches_prompt;keV", 10000,0,20000));
      unique_TH1F phoswitches_delayed (new TH1F(("phoswitches_delayed_"+thread_i_str).c_str(), "Phoswitches_delayed;keV", 10000,0,20000));

      unique_TH2F ge_VS_phoswitch_delayed (new TH2F(("ge_VS_LaBr3_delayed_"+thread_i_str).c_str(), "ge_VS_phoswitch_delayed;Phoswitch [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F ge_VS_phoswitch_prompt (new TH2F(("ge_VS_phoswitch_prompt_"+thread_i_str).c_str(), "ge_VS_phoswitch_prompt;Phoswitch [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F p_VS_phoswitch_delayed (new TH2F(("p_VS_phoswitch_delayed_"+thread_i_str).c_str(), "p_VS_phoswitch_delayed;Phoswitch_{delayed} [keV]; E#gamma_{prompt} [keV]", 5000,0,20000, 10000,0,10000));
      unique_TH2F d_VS_phoswitch_prompt (new TH2F(("d_VS_phoswitch_prompt_"+thread_i_str).c_str(), "d_VS_phoswitch_prompt;Phoswitch_{prompt} [keV]; E#gamma_{delayed} [keV]", 5000,0,20000, 10000,0,10000));
      
      // BGO
      unique_TH2F ge_VS_BGO_prompt (new TH2F(("ge_VS_BGO_prompt_"+thread_i_str).c_str(), "ge_VS_BGO_prompt;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));
      unique_TH2F ge_VS_BGO_delayed (new TH2F(("ge_VS_BGO_delayed_"+thread_i_str).c_str(), "ge_VS_BGO_delayed;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));

      // Particle trigger (code p)
      unique_TH1F p_p (new TH1F(("p_p_"+thread_i_str).c_str(), "prompt particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F pp_p (new TH2F(("pp_p_"+thread_i_str).c_str(), "gamma-gamma prompt particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_p (new TH1F(("d_p_"+thread_i_str).c_str(), "delayed particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_p (new TH2F(("dd_p_"+thread_i_str).c_str(), "gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dp_p (new TH2F(("dp_p_"+thread_i_str).c_str(), "delayed VS prompt particle trigger;Prompt [keV];Delayed [keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F E_dT_p (new TH2F(("E_dT_p_"+thread_i_str).c_str(), "E vs time particle trigger", 600,-100_ns,200_ns, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_calo_p (new TH1F(("p_calo_p_"+thread_i_str).c_str(), "prompt calorimetry particle trigger", 2000,0,20000));
      unique_TH1F d_calo_p (new TH1F(("d_calo_p_"+thread_i_str).c_str(), "prompt calorimetry particle trigger", 2000,0,20000));

      // Prompt trigger and particle (code pP)

      unique_TH1F d_pP (new TH1F(("d_pP_"+thread_i_str).c_str(), "with prompt gamma, delayed particle trigger;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F dd_pP (new TH2F(("dd_pP_"+thread_i_str).c_str(), "with prompt gamma, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH1F d_calo_pP (new TH1F(("d_calo_pP_"+thread_i_str).c_str(), "with prompt gamma, prompt calorimetry particle trigger", 2000,0,20000));

      unique_TH1F p_pPC5 (new TH1F(("p_pPC5_"+thread_i_str).c_str(), "prompt particle trigger PC5;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5 (new TH1F(("d_pPC5_"+thread_i_str).c_str(), "delayed particle trigger PC5;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pPC3 (new TH1F(("p_pPC3_"+thread_i_str).c_str(), "prompt particle trigger PC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC3 (new TH1F(("d_pPC3_"+thread_i_str).c_str(), "delayed particle trigger PC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pPC2 (new TH1F(("p_pPC2_"+thread_i_str).c_str(), "prompt particle trigger PC2;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC2 (new TH1F(("d_pPC2_"+thread_i_str).c_str(), "delayed particle trigger PC2;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pDC3 (new TH1F(("p_pDC3_"+thread_i_str).c_str(), "prompt particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pDC3 (new TH1F(("d_pDC3_"+thread_i_str).c_str(), "delayed particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPDC3 (new TH1F(("d_pPDC3_"+thread_i_str).c_str(), "delayed particle trigger DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F p_pDC1_3 (new TH1F(("p_pDC1_3_"+thread_i_str).c_str(), "prompt particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pDC1_3 (new TH1F(("d_pDC1_3_"+thread_i_str).c_str(), "delayed particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPDC1_3 (new TH1F(("d_pPDC1_3_"+thread_i_str).c_str(), "delayed particle trigger DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5DC3 (new TH1F(("d_pPC5DC3_"+thread_i_str).c_str(), "delayed particle trigger PC5DC3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F d_pPC5DC1_3 (new TH1F(("Ex_"+thread_i_str).c_str(), "delayed particle trigger PC5DC1_3;keV", nb_bins_Ge_singles,0,max_bin_Ge_singles));

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

      // DSSD
      // Excitation energy trigger (code Ex, ExSI = 4 MeV < Ex < 6.5 MeV)
      constexpr static int bins_Ex = 200;
      constexpr static double max_Ex = 10000;
      unique_TH2F Ex_p_VS_ring (new TH2F(("Ex_p_VS_ring_"+thread_i_str).c_str(), "excitation energy proton VS ring number;Excitation energy [keV]" , 20,0,20, bins_Ex,0,max_Ex));
      unique_TH2F p_VS_Ex_p (new TH2F(("p_VS_Ex_p_"+thread_i_str).c_str(), "prompt Ge VS excitation energy proton;Excitation energy [keV];keV" , bins_Ex,0,max_Ex, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH2F d_VS_Ex_p__P (new TH2F(("d_VS_Ex_p__P_"+thread_i_str).c_str(), "delayed Ge VS excitation energy proton;Excitation energy [keV];keV", bins_Ex,0,max_Ex, nb_bins_Ge_singles,0,max_bin_Ge_singles));
      unique_TH1F Ex_p_histo (new TH1F(("Ex_p_histo_"+thread_i_str).c_str(), "Excitation energy;keV", bins_Ex,0,max_Ex));
      unique_TH2F Ex_VS_PC_p (new TH2F(("Ex_VS_PC_p_"+thread_i_str).c_str(), "excitation energy proton VS prompt calorimetry;Calorimetry prompt [keV];Excitation energy [keV]" ,2000,0,20000,  bins_Ex,0,max_Ex));
      unique_TH2F Ex_VS_DC_p__P (new TH2F(("Ex_VS_DC_p__P_"+thread_i_str).c_str(), "excitation energy proton VS delayed calorimetry;Calorimetry delayed [keV];Excitation energy [keV]", 2000,0,20000, bins_Ex,0,max_Ex));
      constexpr static int bins_Emiss = 200;
      constexpr static double max_Emis = 10000;
      unique_TH1F Emiss__P (new TH1F(("Emiss__P_"+thread_i_str).c_str(), "Missing energy;Missing energy [keV];", bins_Emiss,0,max_Emis));
      unique_TH2F Emiss_VS_Edelayed_p__P (new TH2F(("Emiss_VS_Edelayed_p__P_"+thread_i_str).c_str(), "Missing energy VS delayed energy;Calorimetry delayed [keV];Missing energy [keV]", 2000,0,20000, bins_Emiss,0,max_Emis));

      unique_TH2F dd_ExP (new TH2F(("dd_ExP_"+thread_i_str).c_str(), "with correct excitation energy, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dd_ExSIP (new TH2F(("dd_ExSIP_"+thread_i_str).c_str(), "with best excitation energy, gamma-gamma delayed particle trigger;E1[keV];E2[keV]", nb_bins_Ge_bidim,0,max_bin_Ge_bidim, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      constexpr static int bins_sum_Ge = 2500;
      constexpr static int max_sum_Ge = 5000;
      unique_TH2F d_VS_sum_C2 (new TH2F(("d_VS_sum_C2_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C2_P (new TH2F(("d_VS_sum_C2_P_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C2_pP (new TH2F(("d_VS_sum_C2_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, particle + prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C2_ExP (new TH2F(("d_VS_sum_C2_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, particle + good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C2_ExSIP (new TH2F(("d_VS_sum_C2_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of two clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F p_VS_sum_C_pP (new TH2F(("p_VS_sum_C_pP_"+thread_i_str).c_str(), "prompt Ge VS sum of all clean Ge, particle + best Ex for SI;E sum _{delayed} [keV]; E#gamma_{prompt}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C_pP (new TH2F(("d_VS_sum_C_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C_ExP (new TH2F(("d_VS_sum_C_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_C_ExSIP (new TH2F(("d_VS_sum_C_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_Ge_pP (new TH2F(("d_VS_sum_Ge_pP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_Ge_ExP (new TH2F(("d_VS_sum_Ge_ExP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_sum_Ge_ExSIP (new TH2F(("d_VS_sum_Ge_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      unique_TH2F dirty_d_VS_sum_C2 (new TH2F(("dirty_d_VS_sum_C2_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C2_P (new TH2F(("dirty_d_VS_sum_C2_P_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C2_pP (new TH2F(("dirty_d_VS_sum_C2_pP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, particle + prompt trigger;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C2_ExP (new TH2F(("dirty_d_VS_sum_C2_ExP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, particle + good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C2_ExSIP (new TH2F(("dirty_d_VS_sum_C2_ExSIP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of two clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C_pP (new TH2F(("dirty_d_VS_sum_C_pP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, particle + prompt;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C_ExP (new TH2F(("dirty_d_VS_sum_C_ExP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, good Ex;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F dirty_d_VS_sum_C_ExSIP (new TH2F(("dirty_d_VS_sum_C_ExSIP_"+thread_i_str).c_str(), "dirty delayed Ge VS sum of all clean Ge, best Ex for SI;E sum [keV]; E#gamma_{delayed}[keV]", bins_sum_Ge,0,max_sum_Ge, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      
      unique_TH2F Ex_vs_dC_sum (new TH2F(("Ex_vs_dC_sum_"+thread_i_str).c_str(), "Ex VS sum of all delayed clean Ge;E sum [keV]; E DSSD [keV]", bins_sum_Ge,0,max_sum_Ge, bins_Ex,0,max_Ex));
      unique_TH2F Ep_vs_dC_sum_P (new TH2F(("Ep_vs_dC_sum_P_"+thread_i_str).c_str(), "E dssd VS sum of all delayed clean Ge;E sum [keV]; E DSSD [keV]", bins_sum_Ge,0,max_sum_Ge, bins_Ex,0,max_Ex));
      
      unique_TH2F p_VS_DC_ExSIP (new TH2F(("p_VS_DC_ExSIP_"+thread_i_str).c_str(), "prompt Ge VS delayed Ge, particle + best Ex for SI;Delayed calorimetry [keV]; E#gamma_{delayed}[keV]", 2000,0,20000, nb_bins_Ge_bidim,0,max_bin_Ge_bidim));
      unique_TH2F d_VS_DC_ExSIP (new TH2F(("d_VS_DC_ExSIP_"+thread_i_str).c_str(), "delayed Ge VS delayed Ge, particle + best Ex for SI;Delayed calorimetry [keV]; E#gamma_{delayed}[keV]",2000,0,20000,  nb_bins_Ge_bidim,0,max_bin_Ge_bidim));

      unique_TH1F neutron_hit_pattern (new TH1F(("neutron_hit_pattern"+thread_i_str).c_str(), "neutron_hit_pattern", 1000,0,1000));
      unique_TH1F hit_pattern_2755 (new TH1F(("hit_pattern_2755"+thread_i_str).c_str(), "hit_pattern_2755", 1000,0,1000));
      
      auto const & filename = removePath(file);
      auto const & run_name = removeExtension(filename);
      int const & run_number = std::stoi(split(filename, '_')[1]);
      TChain* chain = new TChain("Nuball2");
      chain->Add(file.c_str());
      print("Reading", file);

      std::string outFolder = "data/"+trigger+"/";
      std::string out_filename = outFolder+filename;

      Event event;
      event.reading(chain, "ltTEQ");

      CloversV2 prompt_clovers;
      CloversV2 delayed_clovers;

      CloversV2 last_prompt_clovers;
      CloversV2 last_delayed_clovers;

      Paris prompt_paris;
      Paris delayed_paris;

      std::vector<double> prompt_phoswitch;
      std::vector<double> delayed_phoswitch;

      // std::vector<double> prompt_phoswitch_label;
      // std::vector<double> delayed_phoswitch_label;

      // std::vector<double> prompt_paris_module;
      // std::vector<double> delayed_paris_module;

      // constexpr void addback = [&](){
        

      // }
      
      std::vector<double> sector_energy;
      std::vector<double> ring_energy;

      std::vector<Label> sector_labels;
      std::vector<Label> ring_labels;


      int evt_i = 1;
      for ( ;(evt_i < chain->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        double prompt_clover_calo = 0;
        double delayed_clover_calo = 0;

        double prompt_phoswitch_calo = 0;
        double delayed_phoswitch_calo = 0;

        bool dssd_trigger = false;

        if (evt_i%freq_hit_display == 0) print(nicer_double(evt_i, 0), "events");

        chain->GetEntry(evt_i);

        // print(event);

        prompt_clovers.reset();
        delayed_clovers.reset();

        // prompt_paris.reset();
        // delayed_paris.reset();

        prompt_phoswitch.clear();
        delayed_phoswitch.clear();
        
        sector_energy.clear();
        ring_energy.clear();

        sector_labels.clear();
        ring_labels.clear();

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          auto const & time =  event.times[hit_i];
          auto const & nrj = event.nrjs[hit_i];
          auto const & nrj2 = event.nrj2s[hit_i];

          if (nrj<5) continue;

          // Remove bad Ge and overflow :
           if ((find_key(CloversV2::maxE_Ge, label) && nrj>CloversV2::maxE_Ge.at(label))) continue;
           if (label == 65 && run_number == 116) continue; // This detector's timing slipped in this run
           if ((label == 134 || label == 135 || label == 136) && time > 100) continue; // These detectors have strange events after 100 ns

          // Paris :
          if (Paris::is[label])
          {
            // if (500 < label && label < 600) continue; // Reject paris front inner ring
            // Calibrate the phoswitch :
            auto const & nrjcal = calibPhoswitches.calibrate(label, nrj, nrj2);
            E_dT_phoswitch->Fill(time, nrjcal);
            if (-5_ns < time && time < 5_ns)
            {
              short_vs_long_prompt->Fill(nrj, nrj2);
              prompt_phoswitch.push_back(nrjcal);
            }
            else if (10_ns < time && time < 40_ns)
            {
              neutron_hit_pattern->Fill(label);
            }
            else if (40_ns < time && time < 170_ns) 
            {
              short_vs_long_delayed->Fill(nrj, nrj2);
              delayed_phoswitch.push_back(nrjcal);
            }
          }

          // Clovers:
          if (-10_ns < time && time < 10_ns) 
          {
            prompt_clovers.fill(event, hit_i);
            if (CloversV2::isBGO(label)) prompt_clover_calo += nrj ;
            else if (CloversV2::isGe(label)) 
            {
              // Rejecting the 511 keV in the calorimetry;
              if (nrj < 506 && 516 < nrj) prompt_clover_calo += smear(nrj, random);
            }
          }
          else if (40_ns < time && time < 170_ns) 
          {
            delayed_clovers.fill(event, hit_i);
            if (CloversV2::isBGO(label)) delayed_clover_calo += nrj ;
            else if (CloversV2::isGe(label)) 
            {
              // Rejecting the 511 keV in the calorimetry;
              if (nrj < 506 && 516 < nrj) delayed_clover_calo += smear(nrj, random) ;
            }
          }
          // if (-5_ns < time && time < 5_ns) prompt_paris.fill(event, hit_i);           // Prompt paris
          // else if (40_ns < time && time < 170_ns) delayed_paris.fill(event, hit_i);   // Delayed paris

          // Dssd :
          if (799 < label && label < 860) 
          {
            dssd_trigger = true;
            if (799 < label && label < 840) 
            {
              sector_labels.push_back(label);
              sector_energy.push_back(nrj);
            }
            if (839 < label && label < 860) 
            {
              ring_labels.push_back(label-840);
              ring_energy.push_back(nrj);
            }
          }
        }// End hits loop

        //////////////
        // Analyse  //
        //////////////
        // First step, perform add-back and compton suppression :
        prompt_clovers.analyze();
        delayed_clovers.analyze();
        // prompt_paris.analyze();
        // delayed_paris.analyze();

        // -- Multiplicity -- //
        auto const & prompt_clover_mult = prompt_clovers.Hits.size();
        auto const & delayed_clover_mult = delayed_clovers.Hits.size();
        auto const & prompt_paris_mult = prompt_phoswitch.size();
        auto const & delayed_paris_mult = delayed_phoswitch.size();
        // auto const & prompt_paris_mult = prompt_paris.Hits.size();
        // auto const & delayed_paris_mult = delayed_paris.Hits.size();
        auto const & prompt_mult = prompt_clover_mult + prompt_paris_mult;
        auto const & delayed_mult = delayed_clover_mult + delayed_paris_mult;

        auto const & delayed_Ge_mult = delayed_clovers.Ge.size();
        bool Gemult[10] = {false}; Gemult[delayed_Ge_mult] = true;
        auto const & delayed_C_mult = delayed_clovers.GeClean.size();
        bool Cmult[10] = {false}; Cmult[delayed_C_mult] = true;

        p_mult->Fill(prompt_mult);
        d_mult->Fill(delayed_mult);
        dp_mult->Fill(prompt_mult, delayed_mult);

        // -- Calorimetry -- //
        // print(prompt_phoswitch);
        // print(delayed_phoswitch);
        // pauseCo();
        prompt_phoswitch_calo = sum(prompt_phoswitch);
        delayed_phoswitch_calo = sum(delayed_phoswitch);
        auto const & prompt_calo_clover = prompt_clover_calo;
        auto const & delayed_calo_clover = delayed_clover_calo;
        // auto const & prompt_calo_clover = prompt_clovers.calorimetryBGO + prompt_clovers.calorimetryGe + prompt_phoswitch_calo;
        // auto const & delayed_calo_clover = delayed_clovers.calorimetryBGO + delayed_clovers.calorimetryGe + delayed_phoswitch_calo;
        // auto const & prompt_calo_paris = prompt_paris.NaI_calorimetry() + smear(prompt_paris.LaBr3_calorimetry(), random);
        // auto const & delayed_calo_paris = delayed_paris.NaI_calorimetry() + smear(delayed_paris.LaBr3_calorimetry(), random);
        
        if (prompt_calo_clover     > 0) p_calo_clover -> Fill(prompt_calo_clover);
        if (delayed_calo_clover    > 0) d_calo_clover -> Fill(delayed_calo_clover);
        if (prompt_phoswitch_calo  > 0) p_calo_phoswitch -> Fill(prompt_phoswitch_calo);
        if (delayed_phoswitch_calo > 0) d_calo_phoswitch -> Fill(delayed_phoswitch_calo);
        if (prompt_calo_clover     > 0 && prompt_phoswitch_calo  > 0) p_calo_clover_VS_p_calo_phoswitch -> Fill(prompt_calo_clover, prompt_phoswitch_calo);
        if (delayed_calo_clover    > 0 && delayed_phoswitch_calo > 0) d_calo_clover_VS_d_calo_phoswitch -> Fill(delayed_calo_clover, delayed_phoswitch_calo);

        auto const & prompt_calo = prompt_calo_clover + prompt_phoswitch_calo;
        auto const & delayed_calo = delayed_calo_clover + delayed_phoswitch_calo;
        if (0 < prompt_calo) p_calo->Fill(prompt_calo);
        if (0 < delayed_calo) d_calo->Fill(delayed_calo);
        if (prompt_calo > 0 && delayed_calo > 0) dp_calo->Fill(prompt_calo, delayed_calo);
        if (delayed_calo > 0 && prompt_mult > 0) d_calo_pP->Fill(delayed_calo_clover);

        /////// PROMPT CLEAN ///////
        for (size_t loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
        {
          auto const & index_i = prompt_clovers.GeClean[loop_i];
          auto const & clover_i = prompt_clovers[index_i];
          p->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          p_VS_PM->Fill(prompt_mult, clover_i.nrj);
          p_VS_DM->Fill(delayed_mult, clover_i.nrj);

          if (delayed_mult > 0) p_D->Fill(clover_i.nrj);

          if (prompt_calo < 5_MeV) p_PC5->Fill(clover_i.nrj);
          if (prompt_calo < 3_MeV) p_PC3->Fill(clover_i.nrj);
          if (prompt_calo < 2_MeV) p_PC2->Fill(clover_i.nrj);
          if (delayed_calo < 3_MeV) 
          {
            p_DC3->Fill(clover_i.nrj);
            if (1_MeV < delayed_calo) 
            {
              p_DC1_3->Fill(clover_i.nrj);
              if (prompt_calo < 3_MeV) p_PC3DC1_3->Fill(clover_i.nrj);
            }
            if (prompt_calo < 3_MeV) p_PC3DC3->Fill(clover_i.nrj);
          }
          // Prompt-prompt :
          for (size_t loop_j = loop_i+1; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            pp->Fill(clover_i.nrj, clover_j.nrj);
            pp->Fill(clover_j.nrj, clover_i.nrj);
            if (dssd_trigger) 
            {
              pp_p->Fill(clover_i.nrj, clover_j.nrj);
              pp_p->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_calo < 5_MeV)
            {
              // pp_PC5->Fill(clover_i.nrj, clover_j.nrj);
              // pp_PC5->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_calo < 3_MeV)
            {
              // pp_PC3->Fill(clover_i.nrj, clover_j.nrj);
              // pp_PC3->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_calo < 2_MeV)
            {
              // pp_PC2->Fill(clover_i.nrj, clover_j.nrj);
              // pp_PC2->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (delayed_calo < 3_MeV)
            {
              // pp_DC3->Fill(clover_i.nrj, clover_j.nrj);
              // pp_DC3->Fill(clover_j.nrj, clover_i.nrj);
              if (1_MeV < delayed_calo)
              {
                // pp_DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                // pp_DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                if (prompt_calo < 3_MeV)
                {
                  // pp_PC3DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                  // pp_PC3DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
              if (prompt_calo < 3_MeV)
              {
                // pp_PC3DC3->Fill(clover_i.nrj, clover_j.nrj);
                // pp_PC3DC3->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
          }
          for (auto const & nrj_paris : prompt_phoswitch)
          {
            ge_VS_phoswitch_prompt->Fill(nrj_paris, clover_i.nrj);
          }
          for (auto const & nrj_paris : delayed_phoswitch)
          {
            p_VS_phoswitch_delayed->Fill(nrj_paris, clover_i.nrj);
          }
          for (auto const & BGO_i : prompt_clovers.BGOClean)
          {
            auto const & nrj_BGO = prompt_clovers[BGO_i].nrj_BGO;
            ge_VS_BGO_prompt->Fill(nrj_BGO, clover_i.nrj);
          }

          // prompt-prompt background
          for (auto const & index_j : last_prompt_clovers.GeClean)
          {
            auto const & clover_j = last_prompt_clovers[index_j];
            if (index_i == index_j) continue;
            pp_bckg->Fill(clover_i.nrj, clover_j.nrj);
            pp_bckg->Fill(clover_j.nrj, clover_i.nrj);
          }
          
        }

        /////// DELAYED CLEAN ///////
        for (size_t loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
        {
          auto const & index_i = delayed_clovers.GeClean[loop_i];
          auto const & clover_i = delayed_clovers[index_i];
          if (2750 < clover_i.nrj && clover_i.nrj < 2760) hit_pattern_2755->Fill(index_i);
          d->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          
          d_VS_PM->Fill(prompt_mult, clover_i.nrj);
          d_VS_DM->Fill(delayed_mult, clover_i.nrj);
          
          if (prompt_calo > 10) d_VS_PC -> Fill(prompt_calo, clover_i.nrj);
          if (delayed_mult > 1) d_VS_DC -> Fill(delayed_calo, clover_i.nrj);

          if (prompt_mult > 0) d_P->Fill(clover_i.nrj);

          if (prompt_mult > 0 && prompt_calo < 5_MeV) d_PC5->Fill(clover_i.nrj);
          if (prompt_mult > 0 && prompt_calo < 3_MeV) d_PC3->Fill(clover_i.nrj);
          if (prompt_mult > 0 && prompt_calo < 2_MeV) d_PC2->Fill(clover_i.nrj);
          if (delayed_calo < 3_MeV) 
          {
            d_DC3->Fill(clover_i.nrj);
            if (prompt_mult > 0 && prompt_calo < 3_MeV) d_PC3DC3->Fill(clover_i.nrj);
            if (1_MeV < delayed_calo) 
            {
              d_DC1_3->Fill(clover_i.nrj);
              if (prompt_mult > 0 && prompt_calo < 3_MeV) d_PC3DC1_3->Fill(clover_i.nrj);
            }
          }
          for (auto const & index_j : last_delayed_clovers.GeClean)
          {
            auto const & clover_j = delayed_clovers[index_j];
            dd_bckg->Fill(clover_i.nrj, clover_j.nrj);
            dd_bckg->Fill(clover_j.nrj, clover_i.nrj);
          }
          // Delayed-delayed :
          for (size_t loop_j = loop_i+1; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_index_j = delayed_clovers.GeClean[loop_j];
            auto const & clover_j = delayed_clovers[clover_index_j];
            dd->Fill(clover_i.nrj, clover_j.nrj);
            dd->Fill(clover_j.nrj, clover_i.nrj);
            
            if (dssd_trigger)
            {
              dd_p->Fill(clover_i.nrj, clover_j.nrj);
              dd_p->Fill(clover_j.nrj, clover_i.nrj);
              if (prompt_mult>0)
              {
                dd_pP->Fill(clover_i.nrj, clover_j.nrj);
                dd_pP->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
            if (prompt_mult > 0)
            {
              dd_P->Fill(clover_i.nrj, clover_j.nrj);
              dd_P->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_mult > 0 && prompt_calo < 5_MeV)
            {
              // dd_PC5->Fill(clover_i.nrj, clover_j.nrj);
              // dd_PC5->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_mult > 0 && prompt_calo < 3_MeV)
            {
              // dd_PC3->Fill(clover_i.nrj, clover_j.nrj);
              // dd_PC3->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (prompt_mult > 0 && prompt_calo < 2_MeV)
            {
              // dd_PC2->Fill(clover_i.nrj, clover_j.nrj);
              // dd_PC2->Fill(clover_j.nrj, clover_i.nrj);
            }
            if (delayed_calo < 3_MeV)
            {
              // dd_DC3->Fill(clover_i.nrj, clover_j.nrj);
              // dd_DC3->Fill(clover_j.nrj, clover_i.nrj);
              if (1_MeV < delayed_calo)
              {
                // dd_DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                // dd_DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                if (prompt_mult > 0 && prompt_calo < 3_MeV)
                {
                  // dd_PC3DC1_3->Fill(clover_i.nrj, clover_j.nrj);
                  // dd_PC3DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
              if (prompt_mult > 0 && prompt_calo < 3_MeV)
              {
                  // dd_PC3DC3->Fill(clover_i.nrj, clover_j.nrj);
                  // dd_PC3DC3->Fill(clover_j.nrj, clover_i.nrj);
              }
            }
          }

          // Gated delayed-delayed (=triple delayed coincidence)
          auto const & nrj_int = size_cast(clover_i.nrj);
          if (make_triple_coinc_ddd && 0 < nrj_int && nrj_int < ddd_gate_bin_max && ddd_gate_lookup[nrj_int])
          {
            for (size_t loop_j = 0; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
            {
              if (loop_i == loop_j) continue;
              auto const & clover_j = delayed_clovers[delayed_clovers.GeClean[loop_j]];
              for (size_t loop_k = loop_j+1; loop_k<delayed_clovers.GeClean.size(); ++loop_k)
              {
              if (loop_i == loop_k) continue;
                auto const & clover_k = delayed_clovers[delayed_clovers.GeClean[loop_k]];
                ddd_gated[ddd_index_gate_lookup[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                ddd_gated[ddd_index_gate_lookup[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
              }
            }
          }

          // Gated delayed-delayed (=triple coincidence delayed prompt-prompt)
          if (make_triple_coinc_dpp && 0 < nrj_int && nrj_int < dpp_gate_bin_max && dpp_gate_lookup[nrj_int])
          {
            for (size_t loop_j = 0; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
            {
              auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
              for (size_t loop_k = loop_j+1; loop_k<prompt_clovers.GeClean.size(); ++loop_k)
              {
                auto const & clover_k = prompt_clovers[prompt_clovers.GeClean[loop_k]];
                dpp_gated[ddd_index_gate_lookup[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                dpp_gated[ddd_index_gate_lookup[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
              }
            }
          }

          // Prompt-delayed :
          for (size_t loop_j = 0; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            dp->Fill(clover_j.nrj, clover_i.nrj);
            if (dssd_trigger) dp_p->Fill(clover_j.nrj, clover_i.nrj);
            // if (prompt_mult > 0 && prompt_calo < 5_MeV) dp_PC5->Fill(clover_j.nrj, clover_i.nrj);
            // if (prompt_mult > 0 && prompt_calo < 3_MeV) dp_PC3->Fill(clover_j.nrj, clover_i.nrj);
            // if (prompt_mult > 0 && prompt_calo < 2_MeV) dp_PC2->Fill(clover_j.nrj, clover_i.nrj);
            // if (delayed_calo < 3_MeV) dp_DC3->Fill(clover_j.nrj, clover_i.nrj);
            if (delayed_calo < 3_MeV) 
            {
              // dp_DC3->Fill(clover_j.nrj, clover_i.nrj);
              if (1_MeV < delayed_calo) 
              {
                // dp_DC1_3->Fill(clover_j.nrj, clover_i.nrj);
                // if (prompt_mult > 0 && prompt_calo < 3_MeV) dp_PC3DC1_3->Fill(clover_j.nrj, clover_i.nrj);
              }
              if (prompt_mult > 0 && prompt_calo < 3_MeV) dp_PC3DC3->Fill(clover_j.nrj, clover_i.nrj);
            }
          }
          // Prompt phoswitch delayed Ge
          for (auto const & nrj_paris : prompt_phoswitch)
          {
            d_VS_phoswitch_prompt->Fill(nrj_paris, clover_i.nrj);
          }

          if (prompt_mult > 0 && prompt_calo < 5_MeV)
          {
            if (delayed_mult > 1) d_VS_DC_PC5->Fill(delayed_calo, clover_i.nrj);
            d_VS_PM_PC5->Fill(prompt_mult, clover_i.nrj);
            d_VS_DM_PC5->Fill(delayed_mult, clover_i.nrj);
          }
          if (prompt_mult > 0 && prompt_calo < 3_MeV)
          {
            if (delayed_mult > 1) d_VS_DC_PC3->Fill(delayed_calo, clover_i.nrj);
            d_VS_PM_PC3->Fill(prompt_mult, clover_i.nrj);
            d_VS_DM_PC3->Fill(delayed_mult, clover_i.nrj);
          }
          if (prompt_mult > 0 && prompt_calo < 2_MeV)
          {
            if (delayed_mult > 1) d_VS_DC_PC2->Fill(delayed_calo, clover_i.nrj);
            d_VS_PM_PC2->Fill(prompt_mult, clover_i.nrj);
            d_VS_DM_PC2->Fill(delayed_mult, clover_i.nrj);
          }
          if (delayed_calo < 3_MeV)
          {
            if (delayed_mult > 1) d_VS_DC_DC3->Fill(delayed_calo, clover_i.nrj);
            d_VS_PM_DC3->Fill(prompt_mult, clover_i.nrj);
            d_VS_DM_DC3->Fill(delayed_mult, clover_i.nrj);
            if (1_MeV < delayed_calo)
            {
              if (delayed_mult > 1) d_VS_DC_DC1_3->Fill(delayed_calo, clover_i.nrj);
              d_VS_PM_DC1_3->Fill(prompt_mult, clover_i.nrj);
              d_VS_DM_DC1_3->Fill(delayed_mult, clover_i.nrj);
              if (prompt_mult > 0 && prompt_calo < 3_MeV)
              {
                if (delayed_mult > 1) d_VS_DC_PC3DC1_3->Fill(delayed_calo, clover_i.nrj);
                d_VS_PM_PC3DC1_3->Fill(prompt_mult, clover_i.nrj);
                d_VS_DM_PC3DC1_3->Fill(delayed_mult, clover_i.nrj);
              }
            }
            if (prompt_mult > 0 && prompt_calo < 3_MeV)
            {
              if (delayed_mult > 1) d_VS_DC_PC3DC3->Fill(delayed_calo, clover_i.nrj);
              d_VS_PM_PC3DC3->Fill(prompt_mult, clover_i.nrj);
              d_VS_DM_PC3DC3->Fill(delayed_mult, clover_i.nrj);
            }
          }

          //// Clover VS PARIS ////
          for (auto const & nrj_paris : delayed_phoswitch)
          {
            ge_VS_phoswitch_delayed->Fill(nrj_paris, clover_i.nrj);
          }
          
          //// Clover VS BGO ////
          for (auto const & BGO_i : delayed_clovers.BGOClean)
          {
            auto const & nrj_BGO = delayed_clovers[BGO_i].nrj_BGO;
            ge_VS_BGO_delayed->Fill(nrj_BGO, clover_i.nrj);
          }
        }

        //////////////// DSSD ///////////////////
        double Ex_p = ExcitationEnergy::bad_value; // d stands for proton detected
        auto const & sector_mult = sector_labels.size();
        auto const & ring_mult = ring_labels.size();
        auto ring_label = 0;
        auto dssd_energy = 0;


        // As many rings and sector fired
        if (sector_mult == ring_mult)
        {
          if (sector_mult == 1)
          {
            ring_label = ring_labels[0];
            dssd_energy = sector_energy[0];
            Ex_p = Ex(dssd_energy, ring_label);
          }
          // else; // Maybe to be improved
        }

        if (sector_mult == 2*ring_mult)
        {
          if (sector_mult == 1 && abs(int_cast(ring_labels[0]-ring_labels[1])))
          {
            ring_label = ring_labels[0]; // To be improved maybe
            dssd_energy = sector_energy[0];
            Ex_p = (Ex(dssd_energy, ring_labels[0]) + Ex(dssd_energy, ring_labels[1]))/2;
            // Might want to reject cases when the sum energy of both rings isn't equal to the one of the sector
          }
          // else; // Maybe to be improved
        }

        if (sector_mult == 0)
        { // If the sector is dead then only rings fired
          if (ring_mult == 1)
          {
            dssd_energy = ring_energy[0];
            ring_label = ring_labels[0];
            Ex_p = Ex(dssd_energy, ring_label);
          }
          else if (ring_mult == 2)
          {
            dssd_energy = ring_energy[0]+ring_energy[1];
            ring_label = (ring_energy[0] > ring_energy[1]) ? ring_labels[0] : ring_labels[1];
            Ex_p = Ex(dssd_energy, ring_label);
          }
        }

        auto const & Emiss = Ex_p-prompt_calo;
        if (Ex_p>0) 
        {
          Ex_p_histo->Fill(Ex_p);
          Ex_p_VS_ring->Fill(Ex_p, ring_label);
          if (prompt_mult > 0)
          {
            Emiss__P->Fill(Emiss);
            if (delayed_calo>0) Emiss_VS_Edelayed_p__P->Fill(delayed_calo, Emiss);
          }
        }

        if (dssd_trigger)
        {
          p_calo_p->Fill(prompt_calo);
          d_calo_p->Fill(delayed_calo);
          if (prompt_mult > 0 && Ex_p>0) 
          {
            Ex_VS_PC_p->Fill(prompt_calo, Ex_p);
            Ex_VS_DC_p__P->Fill(delayed_calo, Ex_p);
          }
          for (size_t loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
          {
            auto const & index_i = prompt_clovers.GeClean[loop_i];
            auto const & clover_i = prompt_clovers[index_i];
            auto const & nrj = clover_i.nrj;
            auto const & time = clover_i.time;
            p_p->Fill(nrj);
            E_dT_p->Fill(time, nrj);
            p_VS_PM_p->Fill(prompt_mult, nrj);
            p_VS_DM_p->Fill(delayed_mult, nrj);
            p_VS_PC_p->Fill(prompt_calo, nrj);
            p_VS_DC_p->Fill(delayed_calo, nrj);
            if (prompt_mult > 0 && prompt_calo < 5_MeV) p_pPC5->Fill(nrj);
            if (prompt_mult > 0 && prompt_calo < 3_MeV) p_pPC3->Fill(nrj);
            if (prompt_mult > 0 && prompt_calo < 2_MeV) p_pPC2->Fill(nrj);
            if (delayed_calo < 3_MeV)
            {
              p_pDC3->Fill(nrj);
              if (delayed_calo > 1_MeV)
              {
                p_pDC1_3->Fill(nrj);
              }
            }
            if (prompt_mult > 0 && Ex_p > 0)
            {
              p_VS_Ex_p->Fill(Ex_p, nrj);
            }
          }
          for (size_t loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
          {
            auto const & index_i = delayed_clovers.GeClean[loop_i];
            auto const & clover_i = delayed_clovers[index_i];
            auto const & nrj = clover_i.nrj;
            auto const & time = clover_i.time;
            d_p->Fill(nrj);
            E_dT_p->Fill(time, nrj);
            d_VS_PM_p->Fill(prompt_mult, nrj);
            d_VS_DM_p->Fill(delayed_mult, nrj);
            d_VS_PC_p->Fill(prompt_calo, nrj);
            if (delayed_mult > 1) d_VS_DC_p->Fill(delayed_calo, nrj);
            if (prompt_mult>0)
            {
              d_pP->Fill(nrj);
              d_VS_PC_pP->Fill(prompt_calo, nrj);
              d_VS_DM_pP->Fill(delayed_mult, nrj);
              if (delayed_mult > 1) d_VS_DC_pP->Fill(delayed_calo, nrj);
              if (4_MeV < Emiss && Emiss < 6.5_MeV)
              {
                p_VS_DC_ExSIP->Fill(delayed_calo, nrj);
                d_VS_DC_ExSIP->Fill(delayed_calo, nrj);
              }
            }
            if (prompt_mult > 0 && prompt_calo < 5_MeV) d_pPC5->Fill(nrj);
            if (prompt_mult > 0 && prompt_calo < 3_MeV) d_pPC3->Fill(nrj);
            if (prompt_mult > 0 && prompt_calo < 2_MeV) d_pPC2->Fill(nrj);
            if (delayed_calo < 3_MeV)
            {
              d_pDC3->Fill(nrj);
              if (prompt_mult > 0) d_pPDC3->Fill(nrj);
              if (prompt_calo < 5_MeV) d_pPC5DC1_3->Fill(nrj);
              if (delayed_calo > 1_MeV) 
              {
                d_pDC1_3->Fill(nrj);
                if (prompt_mult > 0) d_pPDC1_3->Fill(nrj);
                if (prompt_calo < 5_MeV) d_pPC5DC3->Fill(nrj);
              }
            }
            if (prompt_mult > 0 && Ex_p > 0)
            {
              d_VS_Ex_p__P->Fill(Ex_p, nrj);
              for (size_t loop_j = loop_i+1; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
              {
                auto const & index_j = delayed_clovers.GeClean[loop_j];
                auto const & clover_j = delayed_clovers[index_j];
                dd_ExP->Fill(clover_i.nrj, clover_j.nrj);
                dd_ExP->Fill(clover_j.nrj, clover_i.nrj);
                if (4_MeV < Emiss && Emiss < 6.5_MeV)
                {
                  dd_ExSIP->Fill(clover_i.nrj, clover_j.nrj);
                  dd_ExSIP->Fill(clover_j.nrj, clover_i.nrj);
                }
              }
            }
          }
        }

        // Clean Ge sum
        if (Cmult[2])
        {
          auto const & Ge0 = delayed_clovers[delayed_clovers.GeClean[0]];
          auto const & Ge1 = delayed_clovers[delayed_clovers.GeClean[1]];
          auto const & Esum = Ge0.nrj + Ge1.nrj;
          d_VS_sum_C2->Fill(Esum, Ge0.nrj);
          d_VS_sum_C2->Fill(Esum, Ge1.nrj);
          if (prompt_mult > 0)
          {
            d_VS_sum_C2_P->Fill(Esum, Ge0.nrj);
            d_VS_sum_C2_P->Fill(Esum, Ge1.nrj);
          }
          if (dssd_trigger && prompt_mult > 0)
          {
            d_VS_sum_C2_pP->Fill(Esum, Ge0.nrj);
            d_VS_sum_C2_pP->Fill(Esum, Ge1.nrj);
            if (Ex_p > 0)
            {
              d_VS_sum_C2_ExP->Fill(Esum, Ge0.nrj);
              d_VS_sum_C2_ExP->Fill(Esum, Ge1.nrj);
              if (4_MeV < Ex_p && Ex_p < 6.5_MeV)
              {
                d_VS_sum_C2_ExSIP->Fill(Esum, Ge0.nrj);
                d_VS_sum_C2_ExSIP->Fill(Esum, Ge1.nrj);
              }
            }
          }
        }
        
        if (prompt_mult > 0)
        {
          double sum_clean_Ge = 0;
          for (auto const & clover : delayed_clovers.clean) sum_clean_Ge+=clover->nrj;
          double sum_all_Ge = 0;
          for (auto const & clover_index : delayed_clovers.Ge) sum_all_Ge+=delayed_clovers[clover_index].nrj;
          for (auto const & clover : delayed_clovers.clean) 
          {
            if (dssd_trigger) 
            {
              if (sum_clean_Ge>0) d_VS_sum_C_pP->Fill(sum_clean_Ge, clover->nrj);
              if (sum_all_Ge>0)   d_VS_sum_Ge_pP->Fill(sum_all_Ge, clover->nrj);
            }
            if (Ex_p>0)
            {
              if (sum_clean_Ge>0)
              {
                d_VS_sum_C_ExP->Fill(sum_clean_Ge, clover->nrj);
                if (4_MeV < Emiss && Emiss < 6.5_MeV) d_VS_sum_C_ExSIP->Fill(sum_clean_Ge, clover->nrj);
              }
              if (sum_all_Ge>0)
              {
                d_VS_sum_Ge_ExP->Fill(sum_all_Ge, clover->nrj);
                if (4_MeV < Emiss && Emiss < 6.5_MeV) d_VS_sum_Ge_ExSIP->Fill(sum_all_Ge, clover->nrj);
              }
            }
          }
          if (sum_clean_Ge>0)
          {
            if (Ex_p > 0) Ex_vs_dC_sum->Fill(sum_clean_Ge, Ex_p);
            if (dssd_energy > 0) Ep_vs_dC_sum_P->Fill(sum_clean_Ge, dssd_energy);
          }
              
          for (auto const & clover : prompt_clovers.clean) if (sum_clean_Ge>0) p_VS_sum_C_pP->Fill(sum_clean_Ge, clover->nrj);
        }
        
        // Dirty Ge sum
        if (Gemult[2])
        {
          auto const & Ge0 = delayed_clovers[delayed_clovers.Ge[0]];
          auto const & Ge1 = delayed_clovers[delayed_clovers.Ge[1]];
          auto const & Esum = Ge0.nrj + Ge1.nrj;
          dirty_d_VS_sum_C2->Fill(Esum, Ge0.nrj);
          dirty_d_VS_sum_C2->Fill(Esum, Ge1.nrj);
          if (prompt_mult > 0)
          {
            dirty_d_VS_sum_C2_P->Fill(Esum, Ge0.nrj);
            dirty_d_VS_sum_C2_P->Fill(Esum, Ge1.nrj);
          }
          if (dssd_trigger && prompt_mult > 0)
          {
            dirty_d_VS_sum_C2_pP->Fill(Esum, Ge0.nrj);
            dirty_d_VS_sum_C2_pP->Fill(Esum, Ge1.nrj);
            if (Ex_p > 0)
            {
              dirty_d_VS_sum_C2_ExP->Fill(Esum, Ge0.nrj);
              dirty_d_VS_sum_C2_ExP->Fill(Esum, Ge1.nrj);
              if (4_MeV < Ex_p && Ex_p < 6.5_MeV)
              {
                dirty_d_VS_sum_C2_ExSIP->Fill(Esum, Ge0.nrj);
                dirty_d_VS_sum_C2_ExSIP->Fill(Esum, Ge1.nrj);
              }
            }
          }
        }
        
        if (prompt_mult > 0 && delayed_Ge_mult > 1)
        {
          double sum_all_Ge = 0;
          for (auto const & index : delayed_clovers.Ge) sum_all_Ge+=delayed_clovers[index].nrj;
          for (auto const & clover : delayed_clovers.clean) 
          {
            if (dssd_trigger) dirty_d_VS_sum_C_pP->Fill(sum_all_Ge, clover->nrj);
            if (Ex_p>0)
            {
              dirty_d_VS_sum_C_ExP->Fill(sum_all_Ge, clover->nrj);
              if (4_MeV < Emiss && Emiss < 6.5_MeV) dirty_d_VS_sum_C_ExSIP->Fill(sum_all_Ge, clover->nrj);
            }
          }
        }


        /////// PARIS //////////
        for (auto const & paris_nrj : prompt_phoswitch)  phoswitches_prompt ->Fill(paris_nrj);
        for (auto const & paris_nrj : delayed_phoswitch) phoswitches_delayed->Fill(paris_nrj);
        // for (auto const & index : prompt_paris.front().HitsClean) paris_prompt -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.back().HitsClean) paris_prompt -> Fill(prompt_paris.back().modules[index].nrj);
        // for (auto const & index : delayed_paris.front().HitsClean) paris_delayed -> Fill(delayed_paris.front().modules[index].nrj);
        // for (auto const & index : delayed_paris.back().HitsClean) paris_delayed -> Fill(delayed_paris.back().modules[index].nrj);
        
        // for (auto const & index : prompt_paris.front().hits_LaBr3) LaBr3_prompt -> Fill(prompt_paris.front().phoswitches[index].nrj);
        // for (auto const & index : prompt_paris.back().hits_LaBr3) LaBr3_prompt -> Fill(prompt_paris.back().phoswitches[index].nrj);
        // for (auto const & index : delayed_paris.front().hits_LaBr3) LaBr3_delayed -> Fill(delayed_paris.front().phoswitches[index].nrj);
        // for (auto const & index : delayed_paris.back().hits_LaBr3) LaBr3_delayed -> Fill(delayed_paris.back().phoswitches[index].nrj);

        last_prompt_clovers = prompt_clovers;
        last_delayed_clovers = delayed_clovers;

      }// End events loop

      total_hits_number.fetch_add(evt_i, std::memory_order_relaxed);

      // Writing the file (the mutex protects potential concurency issues)
      lock_mutex lock(write_mutex);
      File Filename(out_filename); Filename.makePath();
      print("writing spectra in", out_filename, "...");
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(), "recreate"));
        file->cd();

      print("write standard spectra");
        // Energy :
        pp->Write("pp", TObject::kOverwrite);
        pp_bckg->Write("pp_bckg", TObject::kOverwrite);
        p->Write("p", TObject::kOverwrite);
        dd->Write("dd", TObject::kOverwrite);
        dd_bckg->Write("dd_bckg", TObject::kOverwrite);
        d->Write("d", TObject::kOverwrite);
        E_dT->Write("E_dT", TObject::kOverwrite);
        E_dT_phoswitch->Write("E_dT_phoswitch", TObject::kOverwrite);
        dp->Write("dp", TObject::kOverwrite);
        if (make_triple_coinc_ddd) for (size_t gate_index = 0; gate_index<ddd_gates.size(); ++gate_index) 
          ddd_gated[gate_index]->Write(("ddd_gate_"+std::to_string(ddd_gates[gate_index])).c_str(), TObject::kOverwrite);
        if (make_triple_coinc_dpp) for (size_t gate_index = 0; gate_index<dpp_gates.size(); ++gate_index) 
          dpp_gated[gate_index]->Write(("dpp_gate_"+std::to_string(dpp_gates[gate_index])).c_str(), TObject::kOverwrite);

        print("write multiplicity spectra");
        // Multiplicity :
        p_D->Write("p_D", TObject::kOverwrite);
        d_P->Write("d_P", TObject::kOverwrite);
        dd_P->Write("dd_P", TObject::kOverwrite);
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
        // // dd_PC5->Write("dd_PC5", TObject::kOverwrite);
        // // dp_PC5->Write("dp_PC5", TObject::kOverwrite);
        d_VS_DC_PC5->Write("d_VS_DC_PC5", TObject::kOverwrite);
        d_VS_PM_PC5->Write("d_VS_PM_PC5", TObject::kOverwrite);
        d_VS_DM_PC5->Write("d_VS_DM_PC5", TObject::kOverwrite);

        p_PC3->Write("p_PC3", TObject::kOverwrite);
        // // pp_PC3->Write("pp_PC3", TObject::kOverwrite);
        d_PC3->Write("d_PC3", TObject::kOverwrite);
        // // dd_PC3->Write("dd_PC3", TObject::kOverwrite);
        // // dp_PC3->Write("dp_PC3", TObject::kOverwrite);
        d_VS_DC_PC3->Write("d_VS_DC_PC3", TObject::kOverwrite);
        d_VS_PM_PC3->Write("d_VS_PM_PC3", TObject::kOverwrite);
        d_VS_DM_PC3->Write("d_VS_DM_PC3", TObject::kOverwrite);

        p_PC2->Write("p_PC2", TObject::kOverwrite);
        // // pp_PC2->Write("pp_PC2", TObject::kOverwrite);
        d_PC2->Write("d_PC2", TObject::kOverwrite);
        // // dd_PC2->Write("dd_PC2", TObject::kOverwrite);
        // // dp_PC2->Write("dp_PC2", TObject::kOverwrite);
        d_VS_DC_PC2->Write("d_VS_DC_PC2", TObject::kOverwrite);
        d_VS_PM_PC2->Write("d_VS_PM_PC2", TObject::kOverwrite);
        d_VS_DM_PC2->Write("d_VS_DM_PC2", TObject::kOverwrite);

        p_DC3->Write("p_DC3", TObject::kOverwrite);
        // // pp_DC3->Write("pp_DC3", TObject::kOverwrite);
        d_DC3->Write("d_DC3", TObject::kOverwrite);
        // // dd_DC3->Write("dd_DC3", TObject::kOverwrite);
        // // dp_DC3->Write("dp_DC3", TObject::kOverwrite);
        d_VS_DC_DC3->Write("d_VS_DC_DC3", TObject::kOverwrite);
        d_VS_PM_DC3->Write("d_VS_PM_DC3", TObject::kOverwrite);
        d_VS_DM_DC3->Write("d_VS_DM_DC3", TObject::kOverwrite);

        p_DC1_3->Write("p_DC1_3", TObject::kOverwrite);
        // // pp_DC1_3->Write("pp_DC1_3", TObject::kOverwrite);
        d_DC1_3->Write("d_DC1_3", TObject::kOverwrite);
        // // dd_DC1_3->Write("dd_DC1_3", TObject::kOverwrite);
        // // dp_DC1_3->Write("dp_DC1_3", TObject::kOverwrite);
        d_VS_DC_DC1_3->Write("d_VS_DC_DC1_3", TObject::kOverwrite);
        d_VS_PM_DC1_3->Write("d_VS_PM_DC1_3", TObject::kOverwrite);
        d_VS_DM_DC1_3->Write("d_VS_DM_DC1_3", TObject::kOverwrite);

        p_PC3DC3->Write("p_PC3DC3", TObject::kOverwrite);
        // // pp_PC3DC3->Write("pp_PC3DC3", TObject::kOverwrite);
        d_PC3DC3->Write("d_PC3DC3", TObject::kOverwrite);
        // // dd_PC3DC3->Write("dd_PC3DC3", TObject::kOverwrite);
        dp_PC3DC3->Write("dp_PC3DC3", TObject::kOverwrite);
        d_VS_DC_PC3DC3->Write("d_VS_DC_PC3DC3", TObject::kOverwrite);
        d_VS_PM_PC3DC3->Write("d_VS_PM_PC3DC3", TObject::kOverwrite);
        d_VS_DM_PC3DC3->Write("d_VS_DM_PC3DC3", TObject::kOverwrite);

        p_PC3DC1_3->Write("p_PC3DC1_3", TObject::kOverwrite);
        // // pp_PC3DC1_3->Write("pp_PC3DC1_3", TObject::kOverwrite);
        d_PC3DC1_3->Write("d_PC3DC1_3", TObject::kOverwrite);
        // // dd_PC3DC1_3->Write("dd_PC3DC1_3", TObject::kOverwrite);
        // // dp_PC3DC1_3->Write("dp_PC3DC1_3", TObject::kOverwrite);
        d_VS_DC_PC3DC1_3->Write("d_VS_DC_PC3DC1_3", TObject::kOverwrite);
        d_VS_PM_PC3DC1_3->Write("d_VS_PM_PC3DC1_3", TObject::kOverwrite);
        d_VS_DM_PC3DC1_3->Write("d_VS_DM_PC3DC1_3", TObject::kOverwrite);

        print("write particule gated spectra");

        p_p->Write("p_p", TObject::kOverwrite);
        pp_p->Write("pp_p", TObject::kOverwrite);
        d_p->Write("d_p", TObject::kOverwrite);
        dd_p->Write("dd_p", TObject::kOverwrite);
        dp_p->Write("dp_p", TObject::kOverwrite);
        E_dT_p->Write("E_dT_p", TObject::kOverwrite);
        p_calo_p->Write("p_calo_p", TObject::kOverwrite);
        d_calo_p->Write("d_calo_p", TObject::kOverwrite);
        
        d_pP->Write("d_pP", TObject::kOverwrite);
        dd_pP->Write("dd_pP", TObject::kOverwrite);
        d_calo_pP->Write("d_calo_pP", TObject::kOverwrite);

        p_pPC5->Write("p_pPC5", TObject::kOverwrite);
        d_pPC5->Write("d_pPC5", TObject::kOverwrite);
        p_pPC3->Write("p_pPC3", TObject::kOverwrite);
        d_pPC3->Write("d_pPC3", TObject::kOverwrite);
        p_pPC2->Write("p_pPC2", TObject::kOverwrite);
        d_pPC2->Write("d_pPC2", TObject::kOverwrite);
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

        print("write sum spectra");
        d_VS_sum_C2->Write("d_VS_sum_C2", TObject::kOverwrite);
        d_VS_sum_C2_P->Write("d_VS_sum_C2_P", TObject::kOverwrite);
        d_VS_sum_C2_pP->Write("d_VS_sum_C2_pP", TObject::kOverwrite);
        d_VS_sum_C2_ExP->Write("d_VS_sum_C2_ExP", TObject::kOverwrite);
        d_VS_sum_C2_ExSIP->Write("d_VS_sum_C2_ExSIP", TObject::kOverwrite);
        p_VS_sum_C_pP->Write("p_VS_sum_C_pP", TObject::kOverwrite);
        d_VS_sum_C_pP->Write("d_VS_sum_C_pP", TObject::kOverwrite);
        d_VS_sum_C_ExP->Write("d_VS_sum_C_ExP", TObject::kOverwrite);
        d_VS_sum_C_ExSIP->Write("d_VS_sum_C_ExSIP", TObject::kOverwrite);
        d_VS_sum_Ge_pP->Write("d_VS_sum_Ge_pP", TObject::kOverwrite);
        d_VS_sum_Ge_ExP->Write("d_VS_sum_Ge_ExP", TObject::kOverwrite);
        d_VS_sum_Ge_ExSIP->Write("d_VS_sum_Ge_ExSIP", TObject::kOverwrite);

        dirty_d_VS_sum_C2->Write("dirty_d_VS_sum_C2", TObject::kOverwrite);
        dirty_d_VS_sum_C2_P->Write("dirty_d_VS_sum_C2_P", TObject::kOverwrite);
        dirty_d_VS_sum_C2_pP->Write("dirty_d_VS_sum_C2_pP", TObject::kOverwrite);
        dirty_d_VS_sum_C2_ExP->Write("dirty_d_VS_sum_C2_ExP", TObject::kOverwrite);
        dirty_d_VS_sum_C2_ExSIP->Write("dirty_d_VS_sum_C2_ExSIP", TObject::kOverwrite);
        dirty_d_VS_sum_C_pP->Write("dirty_d_VS_sum_C_pP", TObject::kOverwrite);
        dirty_d_VS_sum_C_ExP->Write("dirty_d_VS_sum_C_ExP", TObject::kOverwrite);
        dirty_d_VS_sum_C_ExSIP->Write("dirty_d_VS_sum_C_ExSIP", TObject::kOverwrite);

        Ex_vs_dC_sum->Write("Ex_vs_dC_sum", TObject::kOverwrite);
        Ep_vs_dC_sum_P->Write("Ep_vs_dC_sum_P", TObject::kOverwrite);
        
        p_VS_DC_ExSIP->Write("p_VS_DC_ExSIP", TObject::kOverwrite);
        d_VS_DC_ExSIP->Write("d_VS_DC_ExSIP", TObject::kOverwrite);

        // Paris :
        // paris_prompt->Write("paris_prompt", TObject::kOverwrite);
        // paris_delayed->Write("paris_delayed", TObject::kOverwrite);
        // LaBr3_prompt->Write("LaBr3_prompt", TObject::kOverwrite);
        // LaBr3_delayed->Write("LaBr3_delayed", TObject::kOverwrite);

        print("Write Ex spectra");
        Ex_p_histo->Write("Ex_p_histo", TObject::kOverwrite);
        Ex_p_VS_ring->Write("Ex_p_VS_ring", TObject::kOverwrite);
        p_VS_Ex_p->Write("p_VS_Ex_p", TObject::kOverwrite);
        d_VS_Ex_p__P->Write("d_VS_Ex_p__P", TObject::kOverwrite);
        Ex_VS_PC_p->Write("Ex_VS_PC_p", TObject::kOverwrite);
        Ex_VS_DC_p__P->Write("Ex_VS_DC_p__P", TObject::kOverwrite);
        dd_ExP->Write("dd_ExP", TObject::kOverwrite);
        dd_ExSIP->Write("dd_ExSIP", TObject::kOverwrite);
        Emiss__P->Write("Emiss__P", TObject::kOverwrite);
        Emiss_VS_Edelayed_p__P->Write("Emiss_VS_Edelayed_p__P", TObject::kOverwrite);

        print("write paris spectra");

        short_vs_long_prompt->Write("phoswitches_prompt", TObject::kOverwrite);
        short_vs_long_delayed->Write("phoswitches_delayed", TObject::kOverwrite);

        ge_VS_phoswitch_prompt->Write("ge_VS_phoswitch_prompt", TObject::kOverwrite);
        ge_VS_phoswitch_delayed->Write("ge_VS_phoswitch_delayed", TObject::kOverwrite);
        p_VS_phoswitch_delayed->Write("p_VS_phoswitch_delayed", TObject::kOverwrite);
        d_VS_phoswitch_prompt->Write("d_VS_phoswitch_prompt", TObject::kOverwrite);

        phoswitches_prompt->Write("phoswitches_prompt", TObject::kOverwrite);
        phoswitches_delayed->Write("phoswitches_delayed", TObject::kOverwrite);

        ge_VS_BGO_prompt->Write("ge_VS_BGO_prompt", TObject::kOverwrite);
        ge_VS_BGO_delayed->Write("ge_VS_BGO_delayed", TObject::kOverwrite);

        neutron_hit_pattern->Write("neutron_hit_pattern", TObject::kOverwrite);
        hit_pattern_2755->Write("hit_pattern_2755", TObject::kOverwrite);

      file->Close();
      print(out_filename, "written");
      // mutex freed
    }
  });
  print("Reading speed of", files_total_size/timer.TimeSec(), "Mo/s | ", 1.e-6*total_hits_number/timer.TimeSec(), "M events/s");
}

#ifndef __CINT__
int main(int argc, char** argv)
{
       if (argc == 1) macro3();
  else if (argc == 2) macro3(std::stoi(argv[1]));
  else if (argc == 3) macro3(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macro3(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}
#endif //__CINT__
// g++ -g -o exec macro3.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro3.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17