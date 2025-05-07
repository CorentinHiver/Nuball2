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
// #include "CoefficientCorrection.hpp"
#include "../Data_verification/Utils.h"

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((300.0/sqrt(nrj))/100.0)/2.35);
}

constexpr static bool kCalibGe = false;

static std::vector<Label> blacklist = {501};

void test1(int nb_files = -1, double nb_hits_read = -1, int nb_threads = 10)
{
  if (nb_hits_read<0) nb_hits_read = 1.e+50;
  // std::string target = "Th";
  std::string target = "U";
  // std::string trigger = "P";
  // std::string trigger = "dC1";
  std::string trigger = "C2";

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

  // If too much bidims, need to reduce the number of threads (hardcode!)
  if (make_triple_coinc_ddd || make_triple_coinc_dpp || make_triple_coinc_ppp) nb_threads = std::min(8, nb_threads);
  if (bidim_by_run) nb_threads =  std::min(5, nb_threads);

  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_evts_number(0);
  std::vector<double> time_runs(1000, 0);
  double total_time_of_beam_s = 0;
  int freq_hit_display = (nb_hits_read > 1.e+100) ? 1.e+7 : nb_hits_read/10;

  // Calibration calibNaI("../136/coeffs_NaI.calib");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  // CoefficientCorrection calibGe("../136/GainDriftCoefficients.dat");
  ExcitationEnergy Ex("../136/Excitation_energy", "U5", "d", "p", "10umAl");

  // std::mutex init_mutex;
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
      auto const & run_name = removeExtension(filename);
      auto const & run_name_vector = split(run_name, '_');
      auto const & run_number_str = run_name_vector[1];
      int const & run_number = std::stoi(run_number_str);
      if (target == "Th" && run_number>72) continue;
      if (target == "U" && run_number<74) continue;
      Nuball2Tree tree(file);
      if (!tree) continue;
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);

      int nb_bins_Ge_singles = 10000;
      int max_bin_Ge_singles = 10000;
      int nb_bins_Ge_bidim = 4096;
      double max_bin_Ge_bidim = 4096;

      unique_TH1F Te134_IYR (new TH1F(("Te134_IYR_"+thread_i_str).c_str(), "Te134_IYR", 1000, -500_ns, 500_ns));

      std::string outFolder = "data/"+trigger+"/"+target+"/";
      Path::make(outFolder);
      std::string out_filename = outFolder+filename+"_iyr";
      print(out_filename);

      Event event;
      event.reading(tree, "mltTEQ");

      CloversV2 pclovers;
      CloversV2 dclovers;

      // CloversV2 pclovers_raw;
      // CloversV2 dclovers_raw;

      // CloversV2 last_prompt_clovers;
      // CloversV2 last_delayed_clovers;

      SimpleParis pparis(&calibPhoswitches);
      SimpleParis nparis(&calibPhoswitches);
      SimpleParis dparis(&calibPhoswitches);

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
        nparis.clear();
        dparis.clear();

        auto const absolute_time_h = double_cast(event.stamp)*1.e-12/3600.;
        // timestamp_hist_VS_run->Fill(absolute_time_h, run_number);

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
            // E_vs_run[det_name]->Fill(run_number, nrj);
            // T_vs_run[det_name]->Fill(run_number, time);
          }

          if (label == 252)
          {
            // Fatima_E_dT->Fill(time, nrj);
          }

          // Paris :
          if (Paris::is[label])
          {
            if (found(blacklist, label)) continue;
            // if (Paris::pid_LaBr3(nrj, nrj2)) E_dT_phoswitch->Fill(time, nrj);
            // else if (Paris::pid_good_phoswitch(nrj, nrj2)) E_dT_phoswitch->Fill(time, calibPhoswitches.calibrate(label, nrj, nrj2));
            // short_over_long_VS_time->Fill(time, nrj/nrj2);
            if (gate(-5_ns, time, 5_ns))
            {
              // short_vs_long_prompt->Fill(nrj, nrj2); 
              pparis.fill(event, hit_i);
            }
            else if (gate(5_ns, time, 40_ns))
            {
              // neutron_hit_pattern->Fill(label);
              nparis.fill(event, hit_i);
            }
            else if (gate(40_ns, time, 170_ns))
            {
              // short_vs_long_delayed->Fill(nrj, nrj2);
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
              // if (kCalibGe) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
              if (!gate(505_keV, nrj, 515_keV)) prompt_clover_calo += smear(nrj, random);
            }
            pclovers.fill(event, hit_i);
          }
          else if (gate(20_ns, time, 40_ns))
          {
            if (CloversIsBlacklist[label]) continue;
            // if (kCalibGe && CloversV2::isGe(label)) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
            // n->Fill(nrj);
          }
          else if (gate(40_ns, time, 170_ns) )
          {
            if (CloversIsBlacklist[label]) continue;
            // dclovers_raw.fill(event, hit_i);
            // if (kCalibGe && CloversV2::isGe(label)) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
            dclovers.fill(event, hit_i);
          }

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
        nparis.analyze();
        dparis.analyze();

        // -- Multiplicity -- //
        auto const & pcloverM = pclovers.all.size();
        auto const & pparisM = pparis.module_mult();
        auto const & PM = pcloverM + pparisM;

        auto const & dcloverM = dclovers.all.size();
        auto const & dparisM = dparis.module_mult();
        auto const & DM_raw = dcloverM + dparisM;

        for (size_t loop_i = 0; loop_i<pclovers.GeClean_id.size(); ++loop_i)
        {
          auto const & index_i = pclovers.GeClean_id[loop_i];
          auto const & clover_i = pclovers[index_i];

          // Prompt-prompt :
          for (size_t loop_j = loop_i+1; loop_j<pclovers.GeClean_id.size(); ++loop_j)
          {
            auto const & clover_j = pclovers[pclovers.GeClean_id[loop_j]];

            if (gate(295., clover_i.nrj, 300.) && gate(1277., clover_j.nrj, 1281.))
              Te134_IYR->Fill(clover_i.time - clover_j.time);
            else if (gate(295., clover_j.nrj, 300.) && gate(1277., clover_i.nrj, 1281.))
              Te134_IYR->Fill(clover_j.time - clover_i.time);
          }
        }
      }
      total_evts_number.fetch_add(evt_i, std::memory_order_relaxed);
        
        // Handle last hit :
        auto const & timestamp_final = event.stamp;
        double run_duration_s = Time_cast(timestamp_final-timestamp_init)*1E-12;
  
        // Writing the file (the mutex protects potential concurency issues)
        lock_mutex lock(write_mutex);
        time_runs[run_number] = run_duration_s;
  
        print("run of", nicer_seconds(run_duration_s));
  
        total_time_of_beam_s+=run_duration_s;
        File Filename(out_filename); Filename.makePath();
        print("writing spectra in", out_filename, "...");
  
        print("Calculate additionnal spectra :");
        
        std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(), "recreate"));
          file->cd();
          Te134_IYR->Write("d_calo_p", TObject::kOverwrite);
        file->Close();
        print(out_filename, "written");
    };
  });

  print("Total beam time :", nicer_seconds(total_time_of_beam_s));
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
  print("analysis time :", timer());
}