#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/Paris.hpp"
#include "../lib/Classes/Hit.hpp"

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

void macro3(int nb_files = -1, double nb_hits_read = 1.e+200, int nb_threads = 10)
{
  // std::string trigger = "PrM1DeC1";
  std::string trigger = "C2";
  // std::string trigger = "P";
  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_hits_number(0);
  int freq_hit_display= (nb_hits_read < 2.e+7) ? 1.e+6 : 1.e+7;

  Calibration calibNaI("../136/coeffs_NaI.calib");
  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::Initialise(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  
  std::mutex write_mutex;

  size_t gate_bin_size = 2; // Take 2 keV

  auto const & dd_gate_bin_max = maximum(dd_gates)+gate_bin_size+1;
  std::vector<bool> dd_gate_lookup; dd_gate_lookup.reserve(dd_gate_bin_max);
  std::vector<int> dd_index_gate_lookup; dd_index_gate_lookup.reserve(dd_gate_bin_max);
  size_t temp_bin = 0;
  for (size_t gate_index = 0;  gate_index<dd_gates.size(); ++gate_index)
  {
    for (; temp_bin<size_cast(dd_gate_bin_max); ++temp_bin) 
    {
      auto const & gate = dd_gates[gate_index];
      if (temp_bin<gate-gate_bin_size) 
      {
        dd_gate_lookup.push_back(false);
        dd_index_gate_lookup.push_back(0);
      }
      else if (temp_bin<gate+gate_bin_size+1) 
      {
        dd_gate_lookup.push_back(true);
        dd_index_gate_lookup.push_back(gate_index);
      }
      else break;
    }
  }
  auto const & pp_gate_bin_max = maximum(pp_gates)+gate_bin_size+1;
  std::vector<bool> pp_gate_lookup; pp_gate_lookup.reserve(pp_gate_bin_max);
  temp_bin = 0;
  for (auto gate : pp_gates) for (; temp_bin<pp_gate_bin_max; ++temp_bin) 
  {
    if (temp_bin<gate-gate_bin_size) pp_gate_lookup.push_back(false);
    else if (temp_bin<gate+gate_bin_size+1) pp_gate_lookup.push_back(true);
    else break;
  }

  MTObject::parallelise_function([&](){

    TRandom* random = new TRandom();
    random->SetSeed(time(0));

    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    while(MTfiles.getNext(file))
    {
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);
      
      std::unique_ptr<TH1F> LaBr3_cristal_prompt (new TH1F(("LaBr3_cristal_prompt_"+std::to_string(thread_i)).c_str(), "LaBr3_cristal_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> NaI_cristal_prompt (new TH1F(("NaI_cristal_prompt_"+std::to_string(thread_i)).c_str(), "NaI_cristal_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> nrj2_vs_nrj (new TH1F(("nrj2_vs_nrj_"+std::to_string(thread_i)).c_str(), "nrj2_vs_nrj;keV", 10000,0,20000));
      std::unique_ptr<TH1F> nrj2_vs_nrj (new TH1F(("nrj2_vs_nrj_"+std::to_string(thread_i)).c_str(), "nrj2_vs_nrj;keV", 10000,0,20000));
      std::unique_ptr<TH1F> paris_prompt (new TH1F(("paris_prompt_"+std::to_string(thread_i)).c_str(), "paris_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> paris_delayed (new TH1F(("paris_delayed_"+std::to_string(thread_i)).c_str(), "paris_delayed;keV", 10000,0,20000));
      std::unique_ptr<TH1F> LaBr3_prompt (new TH1F(("LaBr3_prompt_"+std::to_string(thread_i)).c_str(), "LaBr3_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> LaBr3_delayed (new TH1F(("LaBr3_delayed_"+std::to_string(thread_i)).c_str(), "LaBr3_delayed;keV", 10000,0,20000));

      std::unique_ptr<TH2F> ge_VS_LaBr3_delayed (new TH2F(("ge_VS_LaBr3_delayed_"+std::to_string(thread_i)).c_str(), "ge_VS_LaBr3_delayed;LaBr3 [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      std::unique_ptr<TH2F> ge_VS_LaBr3_prompt (new TH2F(("ge_VS_LaBr3_prompt_"+std::to_string(thread_i)).c_str(), "ge_VS_LaBr3_prompt;LaBr3 [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      std::unique_ptr<TH2F> ge_VS_BGO_delayed (new TH2F(("ge_VS_BGO_delayed_"+std::to_string(thread_i)).c_str(), "ge_VS_BGO_delayed;BGO [keV]; Ge [keV]", 2000,0,20000, 10000,0,10000));
      

      auto const & filename = removePath(file);
      auto const & run_name = removeExtension(filename);
      TChain* chain = new TChain("Nuball2");
      chain->Add(file.c_str());
      print("Reading", file);

      std::string outFolder = "data/"+trigger+"/";
      std::string out_filename = outFolder+filename;

      Event event;
      event.reading(chain, "ltTEQ");

      CloversV2 prompt_clovers;
      CloversV2 delayed_clovers;

      Paris prompt_paris;
      Paris delayed_paris;

      bool dssd_trigger = false;

      int evt_i = 1;
      for ( ;(evt_i < chain->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        if (evt_i%freq_hit_display == 0) print(nicer_double(evt_i, 0), "events");

        chain->GetEntry(evt_i);

        prompt_clovers.reset();
        delayed_clovers.reset();

        prompt_paris.reset();
        delayed_paris.reset();

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label_i = event.labels[hit_i];
          auto const & time_i =  event.times[hit_i];

          // Calibrate the NaI :
          if (Paris::is[label_i] && ParisPhoswitch::simple_pid(event.nrjs[hit_i], event.nrj2s[hit_i]))
          {
            event.nrjs[hit_i] = calibNaI(event.nrjs[hit_i], label_i);
            event.nrj2s[hit_i] = calibNaI(event.nrj2s[hit_i], label_i);
          }
        }// End hits loop

        //////////////
        // Analyse  //
        //////////////
        // First step, perform add-back and compton suppression :
        prompt_clovers.analyze();
        delayed_clovers.analyze();
        prompt_paris.analyze();
        delayed_paris.analyze();

        // Multiplicity :
        auto const & prompt_clover_mult = prompt_clovers.Hits.size();
        auto const & delayed_clover_mult = delayed_clovers.Hits.size();
        auto const & prompt_paris_mult = prompt_paris.Hits.size();
        auto const & delayed_paris_mult = delayed_paris.Hits.size();
        auto const & prompt_mult = prompt_clover_mult + prompt_paris_mult;
        auto const & delayed_mult = delayed_clover_mult + delayed_paris_mult;

        // p_mult->Fill(prompt_mult);
        // d_mult->Fill(delayed_mult);
        // dp_mult->Fill(prompt_mult, delayed_mult);

        // Calorimetry :
        auto const & prompt_calo_clover = prompt_clovers.calorimetryBGO + smear(prompt_clovers.calorimetryGe, random);
        auto const & delayed_calo_clover = delayed_clovers.calorimetryBGO + smear(delayed_clovers.calorimetryGe, random);
        auto const & prompt_calo_paris = prompt_paris.NaI_calorimetry() + smear(prompt_paris.LaBr3_calorimetry(), random);
        auto const & delayed_calo_paris = delayed_paris.NaI_calorimetry() + smear(delayed_paris.LaBr3_calorimetry(), random);
        auto const & prompt_calo = prompt_calo_clover + prompt_calo_paris;
        auto const & delayed_calo = delayed_calo_clover + delayed_calo_paris;

        // p_calo->Fill(prompt_calo);
        // d_calo->Fill(delayed_calo);
        // if (prompt_calo>0 && delayed_calo>0) dp_calo->Fill(prompt_calo, delayed_calo);

        /////// PROMPT CLEAN ///////
        for (size_t loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.GeClean[loop_i]];
          if (delayed_calo < 3_MeV) 
          {
            // p_DC3->Fill(clover_i.nrj);
          }
          // prompt-prompt :
          for (auto const & index : prompt_paris.back().CleanLaBr3)
          {
            auto const & nrj_paris = prompt_paris.back().modules_pureLaBr[index].nrj;
            // ge_VS_LaBr3_prompt->Fill(nrj_paris, clover_i.nrj);
          }
        }

        /////// DELAYED CLEAN ///////
        for (size_t loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.GeClean[loop_i]];

          //// Clover VS PARIS ////
          for (auto const & index : delayed_paris.back().CleanLaBr3)
          {
            auto const & nrj_paris = delayed_paris.back().modules_pureLaBr[index].nrj;
            // ge_VS_LaBr3_delayed->Fill(nrj_paris, clover_i.nrj);
          }
        }

        /////// PARIS //////////
        // for (auto const & index : prompt_paris.front().HitsClean) LaBr3_cristal_prompt -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.front().HitsClean) NaI_cristal_prompt -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.front().HitsClean) nrj2_vs_nrj -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.front().HitsClean) nrj2_vs_nrj -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.front().HitsClean) paris_prompt -> Fill(prompt_paris.front().modules[index].nrj);
        // for (auto const & index : prompt_paris.back().HitsClean) paris_prompt -> Fill(prompt_paris.back().modules[index].nrj);
        // for (auto const & index : delayed_paris.front().HitsClean) paris_delayed -> Fill(delayed_paris.front().modules[index].nrj);
        // for (auto const & index : delayed_paris.back().HitsClean) paris_delayed -> Fill(delayed_paris.back().modules[index].nrj);
        
        // for (auto const & index : prompt_paris.front().CleanLaBr3) LaBr3_prompt -> Fill(prompt_paris.front().modules_pureLaBr[index].nrj);
        // for (auto const & index : prompt_paris.back().CleanLaBr3) LaBr3_prompt -> Fill(prompt_paris.back().modules_pureLaBr[index].nrj);
        // for (auto const & index : delayed_paris.front().CleanLaBr3) LaBr3_delayed -> Fill(delayed_paris.front().modules_pureLaBr[index].nrj);
        // for (auto const & index : delayed_paris.back().CleanLaBr3) LaBr3_delayed -> Fill(delayed_paris.back().modules_pureLaBr[index].nrj);

      }// End events loop

      total_hits_number.fetch_add(evt_i, std::memory_order_relaxed);

      // Writing the file (the mutex protects potential concurency issues)
      lock_mutex lock(write_mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
        file->cd();


        // Paris :
        LaBr3_cristal_prompt->Write("LaBr3_cristal_prompt", TObject::kOverwrite);
        LaBr3_cristal_prompt->Write("LaBr3_cristal_prompt", TObject::kOverwrite);
        NaI_cristal_prompt->Write("NaI_cristal_prompt", TObject::kOverwrite);
        NaI_cristal_prompt->Write("NaI_cristal_prompt", TObject::kOverwrite);
        nrj2_vs_nrj->Write("nrj2_vs_nrj", TObject::kOverwrite);
        nrj2_vs_nrj->Write("nrj2_vs_nrj", TObject::kOverwrite);
        paris_prompt->Write("paris_prompt", TObject::kOverwrite);
        paris_delayed->Write("paris_delayed", TObject::kOverwrite);
        LaBr3_prompt->Write("LaBr3_prompt", TObject::kOverwrite);
        LaBr3_delayed->Write("LaBr3_delayed", TObject::kOverwrite);

        ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        ge_VS_LaBr3_prompt->Write("ge_VS_LaBr3_prompt", TObject::kOverwrite);
        
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