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
  return random->Gaus(nrj, nrj*((300.0/sqrt(nrj))/100.0)/2.35);
}

constexpr static bool kCalibGe = false;
constexpr static double E_gate_length = 2;

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

  // If too much bidims, need to reduce the number of threads (hardcode!)
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

      
      static const std::unordered_map<std::string, std::pair<double, double>> coinc_isotopes = {
        {"97ZR", {162, 1103}},
        {"Te132", {697, 974}},
        {"Te133", {126, 1151}},
        {"Te134", {297, 1279}}
      };

      std::unordered_map<std::string, unique_TH1F> time_coincs; 
      std::unordered_map<std::string, unique_TH1F> IYRs;   
      std::unordered_map<std::string, unique_TH2F> HigherE_coinc_E_VS_time;  
      std::unordered_map<std::string, unique_TH2F> LowerE_coinc_E_VS_time;  

      for (auto const & it : coinc_isotopes)
      {
        auto const & name = it.first;
        auto const & energies = it.second;
        time_coincs.emplace(name, new TH1F((name+"_coinc_"+thread_i_str).c_str(), "Zr97_coinc", 1000, -500_ns, 500_ns));
        IYRs.emplace(name, new TH1F((name+"_IYR_"+thread_i_str).c_str(), "Zr97_IYR", 1000, -500_ns, 500_ns));
        LowerE_coinc_E_VS_time.emplace(name, new TH2F((name+"_"+std::to_string(int(energies.first))+"_coinc_E_VS_time_"+thread_i_str).c_str(), "Zr97_1279coin_VS_time", 
          1000,-500_ns,500_ns, 4000,0,4000));
        HigherE_coinc_E_VS_time.emplace(name, new TH2F((name+"_"+std::to_string(int(energies.second))+"_coinc_E_VS_time_"+thread_i_str).c_str(), "Zr97_1279coin_VS_time", 
          1000,-500_ns,500_ns, 4000,0,4000));
      }

      std::string outFolder = "data/"+trigger+"/"+target+"/";
      Path::make(outFolder);
      std::string out_filename = outFolder+filename;
      print(out_filename);

      Event event;
      event.reading(tree, "mltTEQ");

      CloversV2 clovers;

      // Handle the first hit : 
      int evt_i = 0;
      tree->GetEntry(evt_i);
      auto timestamp_init = event.stamp;

      for ( ;(evt_i < tree->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        double prompt_clover_calo = 0;

        if (evt_i>0 && evt_i%freq_hit_display == 0) print(Colib::nicer_double(evt_i, 0), "events");

        tree->GetEntry(evt_i);

        clovers.clear();

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event[hit_i].label;
          auto const & time = event[hit_i].time;
          if (event[hit_i].nrj < 20_keV) continue;

          // Remove bad Ge and overflow :
          if (label == 65 && run_number == 116) continue; // This detector's timing slipped in this run
          if ((label == 134 || label == 135 || label == 136) && time > 100_ns) continue; // These detectors have strange events after 100 ns

          clovers.fill(event, hit_i);

        }// End hits loop
      
        //////////////
        // Analyse  //
        //////////////

        // First step, perform add-back and compton suppression :
        clovers.analyze();

        if (clovers.clean.size() < 3) continue;

        auto fillHistos = [&](std::string const & name, CloverModule const & clover_lowE, CloverModule const & clover_highE)
        {
          time_coincs.at(name) -> Fill(clover_lowE .time - clover_highE.time);
          time_coincs.at(name) -> Fill(clover_highE.time - clover_lowE .time);

          IYRs.at(name) -> Fill(clover_lowE .time);
          IYRs.at(name) -> Fill(clover_highE.time);

        };

        for (size_t loop_i = 0; loop_i<clovers.GeClean_id.size(); ++loop_i)
        {
          auto const & index_i = clovers.GeClean_id[loop_i];
          auto const & clover_i = clovers[index_i];   

          for (auto const & coinc_isotope : coinc_isotopes)
          {
            auto const & name = coinc_isotope.first;
            auto const & low_energy = coinc_isotope.second.first;
            auto const & high_energy = coinc_isotope.second.second;
            if (gate(low_energy-E_gate_length, clover_i.nrj, low_energy+E_gate_length))
            {
              for (size_t loop_j = loop_i+1; loop_j<clovers.GeClean_id.size(); ++loop_j)
              {
                auto const & clover_j = clovers[clovers.GeClean_id[loop_j]];

                LowerE_coinc_E_VS_time .at(name) -> Fill(clover_j.time, clover_j.nrj);

                if (gate(high_energy-E_gate_length, clover_j.nrj, high_energy+E_gate_length)) 
                  fillHistos(name, clover_i, clover_j);
              }
            }
            else if (gate(high_energy-E_gate_length, clover_i.nrj, high_energy+E_gate_length))
            {
              for (size_t loop_j = loop_i+1; loop_j<clovers.GeClean_id.size(); ++loop_j)
              {
                auto const & clover_j = clovers[clovers.GeClean_id[loop_j]];

                HigherE_coinc_E_VS_time.at(name) -> Fill(clover_j.time, clover_j.nrj);

                if (gate(low_energy-E_gate_length, clover_j.nrj, low_energy+E_gate_length))
                  fillHistos(name, clover_j, clover_i);
              }
            }
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
      
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(), "recreate")); file->cd();

        for (auto & it : time_coincs              ) it.second->Write();
        for (auto & it : IYRs                     ) it.second->Write();
        for (auto & it : HigherE_coinc_E_VS_time  ) it.second->Write();
        for (auto & it : LowerE_coinc_E_VS_time   ) it.second->Write();
      
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

  }
  print("analysis time :", timer());
}

int main(int argc, char** argv)
{
       if (argc == 1) test1();
  else if (argc == 2) test1(std::stoi(argv[1]));
  else if (argc == 3) test1(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) test1(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}

// g++ -g -o exec test1.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec test1.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17