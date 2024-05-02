#include "../lib/libRoot.hpp"
#include "../lib/Classes/RF_Manager.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/Paris.hpp"

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

void macro3(int nb_files = -1, double nb_hits_read = 1.e+200, int nb_threads = 10)
{
  std::string trigger = "PrM1DeC1";
  // std::string trigger = "C2";
  // std::string trigger = "P";
  Timer timer;
  std::atomic<int> files_total_size(0);

  Calibration calibNaI("../136/coeffs_NaI.calib");
  Path data_path("~/faster_data/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::Initialise(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  
  std::mutex write_mutex;

  int gate_bin_size = 2; // Take 2 keV
    std::vector<int> dd_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991}; // keV
    std::vector<int> pp_gates = {205, 222, 244, 279, 301, 309, 642, 688, 699, 903, 921, 942, 966, 991}; // keV

    auto const & dd_gate_bin_max = maximum(dd_gates)+gate_bin_size+1;
    std::vector<bool> dd_gate_lookup; dd_gate_lookup.reserve(dd_gate_bin_max);
    std::vector<int> dd_index_gate_lookup; dd_index_gate_lookup.reserve(dd_gate_bin_max);
    size_t temp_bin = 0;
    for (int gate_index = 0;  gate_index<dd_gates.size(); ++gate_index)
    {
      for (; temp_bin<dd_gate_bin_max; ++temp_bin) 
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
      // Simple spectra :
      std::unique_ptr<TH1F> p (new TH1F(("p_"+std::to_string(thread_i)).c_str(), "prompt", 4096,0,4096));
      std::unique_ptr<TH2F> pp (new TH2F(("pp_"+std::to_string(thread_i)).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d (new TH1F(("d_"+std::to_string(thread_i)).c_str(), "delayed", 4096,0,4096));
      std::unique_ptr<TH2F> dd (new TH2F(("dd_"+std::to_string(thread_i)).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH2F> dp (new TH2F(("dp_"+std::to_string(thread_i)).c_str(), "delayed VS prompt;Prompt [keV];Delayed [keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH2F> E_dT (new TH2F(("E_dT_"+std::to_string(thread_i)).c_str(), "E_dT clean", 600,-100_ns,200_ns, 4096,0,4096));
      std::vector<TH2I*> dd_gated;
      for (auto const & gate : dd_gates) dd_gated.push_back(new TH2I(("dd_gated_"+std::to_string(gate)+"_"+std::to_string(thread_i)).c_str(), 
                                        ("gamma-gamma delayed gated on "+std::to_string(gate)+";E1 [keV];E2 [keV]").c_str(), 4096,0,4096, 4096,0,4096));
      // std::vector<TH2I*> pp_gated;
      // for (auto const & gate : pp_gates) pp_gated.push_back(new TH2I(("pp_gated_"+std::to_string(gate)+"_"+std::to_string(thread_i)).c_str(), 
      //                                   ("gamma-gamma prompt gated on "+std::to_string(gate)+";E1 [keV];E2 [keV]").c_str(), 4096,0,4096, 4096,0,4096));

      // Multiplicity :
      std::unique_ptr<TH1F> p_mult (new TH1F(("p_mult_"+std::to_string(thread_i)).c_str(), "p_mult", 20,0,20));
      std::unique_ptr<TH1F> d_mult (new TH1F(("d_mult"+std::to_string(thread_i)).c_str(), "d_mult", 20,0,20));
      std::unique_ptr<TH2F> dp_mult (new TH2F(("dp_mult"+std::to_string(thread_i)).c_str(), "dp_mult", 20,0,20, 20,0,20));
      std::unique_ptr<TH2F> d_VS_prompt_mult (new TH2F(("d_VS_prompt_mult_"+std::to_string(thread_i)).c_str(), "d_VS_prompt_mult", 20,0,20, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_delayed_mult (new TH2F(("d_VS_delayed_mult_"+std::to_string(thread_i)).c_str(), "d_VS_delayed_mult", 20,0,20, 4096,0,4096));
      
      // Calorimetry :
      std::unique_ptr<TH1F> p_calo (new TH1F(("prompt_calorimetry_"+std::to_string(thread_i)).c_str(), "p_calo", 2000,0,20000));
      std::unique_ptr<TH1F> d_calo (new TH1F(("delayed_calorimetry_"+std::to_string(thread_i)).c_str(), "d_calo", 2000,0,20000));
      std::unique_ptr<TH2F> dp_calo (new TH2F(("prompt_VS_delayed_calorimetry_"+std::to_string(thread_i)).c_str(), "dp_calo", 1000,0,10000, 1000,0,10000));
      std::unique_ptr<TH2F> d_VS_prompt_calorimetry (new TH2F(("d_VS_prompt_calorimetry_"+std::to_string(thread_i)).c_str(), "d_VS_prompt_calorimetry", 1000,0,10000, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_delayed_calorimetry (new TH2F(("d_VS_delayed_calorimetry_"+std::to_string(thread_i)).c_str(), "d_VS_delayed_calorimetry", 1000,0,10000, 4096,0,4096));

      // Condition Prompt Calorimetry < 3 MeV (code PC3):
      std::unique_ptr<TH2F> d_VS_delayed_calorimetry_PC3 (new TH2F(("d_VS_delayed_calorimetry_PC3_"+std::to_string(thread_i)).c_str(), "d_VS_delayed_calorimetry_PC3", 1000,0,10000, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_prompt_mult_PC3 (new TH2F(("d_VS_prompt_mult_PC3_"+std::to_string(thread_i)).c_str(), "d_VS_prompt_mult_PC3", 20,0,20, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_delayed_mult_PC3 (new TH2F(("d_VS_delayed_mult_PC3_"+std::to_string(thread_i)).c_str(), "d_VS_delayed_mult_PC3", 20,0,20, 4096,0,4096));
      std::unique_ptr<TH1F> p_PC3 (new TH1F(("p_PC3_"+std::to_string(thread_i)).c_str(), "prompt PC3", 4096,0,4096));
      std::unique_ptr<TH2F> pp_PC3 (new TH2F(("pp_PC3_"+std::to_string(thread_i)).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d_PC3 (new TH1F(("d_PC3_"+std::to_string(thread_i)).c_str(), "delayed PC3", 4096,0,4096));
      std::unique_ptr<TH2F> dd_PC3 (new TH2F(("dd_PC3_"+std::to_string(thread_i)).c_str(), "gamma-gamma delayed PC3;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH2F> dp_PC3 (new TH2F(("dp_PC3_"+std::to_string(thread_i)).c_str(), "delayed VS prompt PC3;Prompt [keV];Delayed [keV]", 4096,0,4096, 4096,0,4096));

      // Paris
      std::unique_ptr<TH1F> paris_prompt (new TH1F(("paris_prompt_"+std::to_string(thread_i)).c_str(), "paris_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> paris_delayed (new TH1F(("paris_delayed_"+std::to_string(thread_i)).c_str(), "paris_delayed;keV", 10000,0,20000));
      std::unique_ptr<TH1F> LaBr3_prompt (new TH1F(("LaBr3_prompt_"+std::to_string(thread_i)).c_str(), "LaBr3_prompt;keV", 10000,0,20000));
      std::unique_ptr<TH1F> LaBr3_delayed (new TH1F(("LaBr3_delayed_"+std::to_string(thread_i)).c_str(), "LaBr3_delayed;keV", 10000,0,20000));

      std::unique_ptr<TH2F> ge_VS_LaBr3_delayed (new TH2F(("ge_VS_LaBr3_delayed_"+std::to_string(thread_i)).c_str(), "ge_VS_LaBr3_delayed;LaBr3 [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      // std::unique_ptr<TH1F> LaBr3_delayed (new TH1F(("LaBr3_delayed_"+std::to_string(thread_i)).c_str(), "LaBr3_delayed;keV", 10000,0,20000));


      auto const & filename = removePath(file);
      auto const & run_name = removeExtension(filename);
      TChain* chain = new TChain("Nuball2");
      chain->Add(file.c_str());
      print("Reading", file);

      std::string outFolder = "data/";
      std::string out_filename = outFolder+filename;

      Event event;
      event.reading(chain, "ltTEQ");

      RF_Manager rf;
      bool rf_found = false;

      CloversV2 prompt_clovers;
      CloversV2 delayed_clovers;

      Paris prompt_paris;
      Paris delayed_paris;

      for (int evt_i = 1; (evt_i < chain->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        if (evt_i%((int)1.e+6) == 0) print(nicer_double(evt_i, 0), "events");

        chain->GetEntry(evt_i);

        prompt_clovers.reset();
        delayed_clovers.reset();

        prompt_paris.reset();
        delayed_paris.reset();

        int prompt_rawMult = 0;
        int delayed_rawMult = 0;

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
          if (-10_ns < time_i && time_i < 10_ns) prompt_clovers.fill(event, hit_i);       // Prompt clovers
          else if (60_ns < time_i && time_i < 170_ns) delayed_clovers.fill(event, hit_i); // Delayed clovers
          if (-5_ns < time_i && time_i < 5_ns) prompt_paris.fill(event, hit_i);           // Prompt paris
          else if (60_ns < time_i && time_i < 170_ns) delayed_paris.fill(event, hit_i);   // Delayed paris
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
        p_mult->Fill(prompt_mult);
        d_mult->Fill(delayed_mult);
        dp_mult->Fill(prompt_mult, delayed_mult);

        // Calorimetry :
        auto const & prompt_calo_clover = prompt_clovers.calorimetryBGO + smear(prompt_clovers.calorimetryGe, random);
        auto const & delayed_calo_clover = delayed_clovers.calorimetryBGO + smear(delayed_clovers.calorimetryGe, random);
        auto const & prompt_calo_paris = prompt_paris.NaI_calorimetry() + smear(prompt_paris.LaBr3_calorimetry(), random);
        auto const & delayed_calo_paris = delayed_paris.NaI_calorimetry() + smear(delayed_paris.LaBr3_calorimetry(), random);
        auto const & prompt_calo = prompt_calo_clover + prompt_calo_paris;
        auto const & delayed_calo = delayed_calo_clover + delayed_calo_paris;
        p_calo->Fill(prompt_calo);
        d_calo->Fill(delayed_calo);
        if (prompt_calo>0 && delayed_calo>0) dp_calo->Fill(prompt_calo, delayed_calo);

        /////// PROMPT CLEAN ///////
        for (int loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.GeClean[loop_i]];
          p->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          if (prompt_calo < 3000) p_PC3->Fill(clover_i.nrj);
          // prompt-prompt :
          for (int loop_j = loop_i+1; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            pp->Fill(clover_i.nrj, clover_j.nrj);
            pp->Fill(clover_j.nrj, clover_i.nrj);
            if (prompt_calo < 3000)
            {
              pp_PC3->Fill(clover_i.nrj, clover_j.nrj);
              pp_PC3->Fill(clover_j.nrj, clover_i.nrj);
            }
          }
        }

        /////// DELAYED CLEAN ///////
        for (int loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.GeClean[loop_i]];
          d->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          d_VS_prompt_mult->Fill(prompt_mult, clover_i.nrj);
          d_VS_delayed_mult->Fill(delayed_mult, clover_i.nrj);
          
          d_VS_prompt_calorimetry -> Fill(prompt_calo, clover_i.nrj);
          d_VS_delayed_calorimetry -> Fill(delayed_calo, clover_i.nrj);
          if (prompt_calo < 3000) d_PC3->Fill(clover_i.nrj);
          
          
          // Delayed-delayed :
          for (int loop_j = loop_i+1; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = delayed_clovers[delayed_clovers.GeClean[loop_j]];
            dd->Fill(clover_i.nrj, clover_j.nrj);
            dd->Fill(clover_j.nrj, clover_i.nrj);
            
            if (prompt_calo < 3000)
            {
              dd_PC3->Fill(clover_i.nrj, clover_j.nrj);
              dd_PC3->Fill(clover_j.nrj, clover_i.nrj);
            }
          }

          // Gated delayed-delayed (=triple coincidence)
          auto const & nrj_int = int_cast(clover_i.nrj);
          if (0 < nrj_int && nrj_int < dd_gate_bin_max && dd_gate_lookup[nrj_int])
          {
            for (int loop_j = 0; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
            {
              if (loop_i == loop_j) continue;
              auto const & clover_j = delayed_clovers[delayed_clovers.GeClean[loop_j]];
              for (int loop_k = loop_j+1; loop_k<delayed_clovers.GeClean.size(); ++loop_k)
              {
              if (loop_i == loop_k) continue;
                auto const & clover_k = delayed_clovers[delayed_clovers.GeClean[loop_k]];
                dd_gated[dd_index_gate_lookup[nrj_int]]->Fill(clover_j.nrj, clover_k.nrj);
                dd_gated[dd_index_gate_lookup[nrj_int]]->Fill(clover_k.nrj, clover_j.nrj);
              }
            }
          }

          // Prompt-delayed :
          for (int loop_j = 0; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            dp->Fill(clover_j.nrj, clover_i.nrj);
            if (prompt_calo < 3000) dp_PC3->Fill(clover_j.nrj, clover_i.nrj);
          }

          if (prompt_calo < 3000)
          {
            d_VS_delayed_calorimetry_PC3->Fill(delayed_calo, clover_i.nrj);
            d_VS_prompt_mult_PC3->Fill(prompt_mult, clover_i.nrj);
            d_VS_delayed_mult_PC3->Fill(delayed_mult, clover_i.nrj);
          }

          //// Clover VS PARIS ////
          for (auto const & index : delayed_paris.back().CleanLaBr3)
          {
            auto const & nrj_paris = delayed_paris.back().modules_pureLaBr[index].nrj;
            ge_VS_LaBr3_delayed->Fill(nrj_paris, clover_i.nrj);
          }
        }

        /////// PARIS //////////
        for (auto const & index : prompt_paris.front().HitsClean) paris_prompt -> Fill(prompt_paris.front().modules[index].nrj);
        for (auto const & index : prompt_paris.back().HitsClean) paris_prompt -> Fill(prompt_paris.back().modules[index].nrj);
        for (auto const & index : delayed_paris.front().HitsClean) paris_delayed -> Fill(delayed_paris.front().modules[index].nrj);
        for (auto const & index : delayed_paris.back().HitsClean) paris_delayed -> Fill(delayed_paris.back().modules[index].nrj);
        
        for (auto const & index : prompt_paris.front().CleanLaBr3) LaBr3_prompt -> Fill(prompt_paris.front().modules_pureLaBr[index].nrj);
        for (auto const & index : prompt_paris.back().CleanLaBr3) LaBr3_prompt -> Fill(prompt_paris.back().modules_pureLaBr[index].nrj);
        for (auto const & index : delayed_paris.front().CleanLaBr3) LaBr3_delayed -> Fill(delayed_paris.front().modules_pureLaBr[index].nrj);
        for (auto const & index : delayed_paris.back().CleanLaBr3) LaBr3_delayed -> Fill(delayed_paris.back().modules_pureLaBr[index].nrj);

      }// End events loop

      // Writing the file (the mutex protects potential concurency issues)
      lock_mutex lock(write_mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
        file->cd();

        // Energy :
        pp->Write("pp", TObject::kOverwrite);
        p->Write("p", TObject::kOverwrite);
        dd->Write("dd", TObject::kOverwrite);
        d->Write("d", TObject::kOverwrite);
        E_dT->Write("E_dT", TObject::kOverwrite);
        dp->Write("dp", TObject::kOverwrite);
        for (size_t gate_index = 0; gate_index<dd_gates.size(); ++gate_index) 
          dd_gated[gate_index]->Write(("dd_gate_"+std::to_string(dd_gates[gate_index])).c_str(), TObject::kOverwrite);

        // Multiplicity :
        p_mult->Write("p_mult", TObject::kOverwrite);
        d_mult->Write("d_mult", TObject::kOverwrite);
        dp_mult->Write("dp_mult", TObject::kOverwrite);
        d_VS_prompt_mult->Write("d_VS_prompt_mult", TObject::kOverwrite);
        d_VS_delayed_mult->Write("d_VS_delayed_mult", TObject::kOverwrite);

        p_calo->Write("p_calo", TObject::kOverwrite);
        d_calo->Write("d_calo", TObject::kOverwrite);
        dp_calo->Write("dp_calo", TObject::kOverwrite);
        d_VS_prompt_calorimetry->Write("d_VS_prompt_calorimetry", TObject::kOverwrite);
        d_VS_delayed_calorimetry->Write("d_VS_delayed_calorimetry", TObject::kOverwrite);

        p_PC3->Write("p_PC3", TObject::kOverwrite);
        pp_PC3->Write("pp_PC3", TObject::kOverwrite);
        d_PC3->Write("d_PC3", TObject::kOverwrite);
        dd_PC3->Write("dd_PC3", TObject::kOverwrite);
        dp_PC3->Write("dp_PC3", TObject::kOverwrite);
        d_VS_delayed_calorimetry_PC3->Write("d_VS_delayed_calorimetry_PC3", TObject::kOverwrite);
        d_VS_prompt_mult_PC3->Write("d_VS_prompt_mult_PC3", TObject::kOverwrite);
        d_VS_delayed_mult_PC3->Write("d_VS_delayed_mult_PC3", TObject::kOverwrite);

        paris_prompt->Write("paris_prompt", TObject::kOverwrite);
        paris_delayed->Write("paris_delayed", TObject::kOverwrite);
        LaBr3_prompt->Write("LaBr3_prompt", TObject::kOverwrite);
        LaBr3_delayed->Write("LaBr3_delayed", TObject::kOverwrite);

        ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        
      file->Close();
      print(out_filename, "written");
      // mutex freed
    }
  });
  print("Reading speed of", files_total_size/timer.TimeSec(), "Mo/s");
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
// g++ -g -o exec macro3.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17
// g++ -O2 -o exec macro3.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17