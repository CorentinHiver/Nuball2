#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/Paris.hpp"
#include "../lib/Classes/Hit.hpp"

void macro3(int nb_files = -1, double nb_hits_read = 1.e+200, int nb_threads = 10)
{
  // std::string trigger = "PrM1DeC1";
  std::string trigger = "dC1";
  // std::string trigger = "C2";
  // std::string trigger = "P";
  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_hits_number(0);
  int freq_hit_display= (nb_hits_read < 2.e+7) ? 1.e+6 : 1.e+7;
  Time time_window = 10_ns;
  Time distance_max = 2;
  bool prompt_each_bidim = true;
  bool delayed_each_bidim = true;
  bool print_array = false;
  bool verbose = false;

  Calibration calibNaI("../136/coeffs_NaI.calib");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::Initialise(nb_threads);
  MTObject::adjustThreadsNumber(files.size());

  Paris::InitialiseArrays();
  if (print_array) ParisCluster<Paris::cluster_size>::printArrays();
  
  std::mutex write_mutex;

  MTObject::parallelise_function([&](){
    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    while(MTfiles.getNext(file))
    {
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);
      auto const & thread_i_str = std::to_string(thread_i);

      std::unordered_map<Label, unique_TH2F> prompt_each_nrjcal_VS_nrj;
      std::unordered_map<Label, unique_TH2F> delayed_each_nrjcal_VS_nrj;

      std::unordered_map<Label, unique_TH1F> prompt_each_phoswitch;
      std::unordered_map<Label, unique_TH1F> delayed_each_phoswitch;
      std::unordered_map<Label, unique_TH1F> prompt_each_module;
      std::unordered_map<Label, unique_TH1F> delayed_each_module;

      for (Label label = 300; label<800; label++) if (Paris::is[label])
      {
        auto const & label_str = std::to_string(label);
        if (prompt_each_bidim)  prompt_each_nrjcal_VS_nrj .emplace(label, new TH2F(("prompt_each_nrjcal_VS_nrj_" +thread_i_str+label_str).c_str(), 
          ("energy phoswitch vs qshort prompt"+label_str+";keV").c_str(), 1000,0,10000, 1000,0,10000));
        if (delayed_each_bidim) delayed_each_nrjcal_VS_nrj.emplace(label, new TH2F(("delayed_each_nrjcal_VS_nrj_"+thread_i_str+label_str).c_str(), 
          ("energy phoswitch vs qshort delayed"+label_str+";keV").c_str(), 1000,0,10000, 1000,0,10000));

        prompt_each_phoswitch .emplace(label, new TH1F(("prompt_each_phoswitch" +thread_i_str+"_"+label_str).c_str(), ("prompt_each_phoswitch_"+label_str).c_str(), 1000,0,10000));
        delayed_each_phoswitch.emplace(label, new TH1F(("delayed_each_phoswitch"+thread_i_str+"_"+label_str).c_str(), ("delayed_each_phoswitch_"+label_str).c_str(), 1000,0,10000));

        prompt_each_module .emplace(label, new TH1F(("prompt_each_module" +thread_i_str+"_"+label_str).c_str(), ("prompt_each_module_"+label_str).c_str(), 1000,0,10000));
        delayed_each_module.emplace(label, new TH1F(("delayed_each_module"+thread_i_str+"_"+label_str).c_str(), ("delayed_each_module_"+label_str).c_str(), 1000,0,10000));
      }

      // unique_TH1F LaBr3_cristal_prompt (new TH1F(("LaBr3_cristal_prompt_"+thread_i_str).c_str(), "LaBr3_cristal_prompt;keV", 10000,0,20000));
      // unique_TH1F NaI_cristal_prompt   (new TH1F(("NaI_cristal_prompt_"+thread_i_str).c_str()  , "NaI_cristal_prompt;keV"  , 10000,0,20000));
      
      // unique_TH2F nrj2_vs_nrj_prompt  (new TH2F(("nrj2_vs_nrj_prompt_"+thread_i_str).c_str() , "nrj2_vs_nrj_prompt;keV" , 2000,0,20000, 2000,0,20000));
      // unique_TH2F nrj2_vs_nrj_delayed (new TH2F(("nrj2_vs_nrj_delayed_"+thread_i_str).c_str(), "nrj2_vs_nrj_delayed;keV", 2000,0,20000, 2000,0,20000));

      // unique_TH1F paris_prompt  (new TH1F(("paris_prompt_"+thread_i_str).c_str() , "paris_prompt;keV" , 10000,0,20000));
      // unique_TH1F paris_delayed (new TH1F(("paris_delayed_"+thread_i_str).c_str(), "paris_delayed;keV", 10000,0,20000));
      // unique_TH1F LaBr3_prompt  (new TH1F(("LaBr3_prompt_"+thread_i_str).c_str() , "LaBr3_prompt;keV" , 10000,0,20000));
      // unique_TH1F LaBr3_delayed (new TH1F(("LaBr3_delayed_"+thread_i_str).c_str(), "LaBr3_delayed;keV", 10000,0,20000));

      // unique_TH2F ge_VS_LaBr3_delayed (new TH2F(("ge_VS_LaBr3_delayed_"+thread_i_str).c_str(), "ge_VS_LaBr3_delayed;LaBr3 [keV]; Ge [keV]", 5000,0,20000, 10000,0,10000));
      // unique_TH2F ge_VS_LaBr3_prompt  (new TH2F(("ge_VS_LaBr3_prompt_"+thread_i_str).c_str() , "ge_VS_LaBr3_prompt;LaBr3 [keV]; Ge [keV]" , 5000,0,20000, 10000,0,10000));
      

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

      std::vector<double> prompt_phoswitch_front_energy;
      std::vector<Label>  prompt_phoswitch_front_label;
      std::vector<Time>   prompt_phoswitch_front_time;
      std::vector<double> prompt_phoswitch_back_energy;
      std::vector<Label>  prompt_phoswitch_back_label;
      std::vector<Time>   prompt_phoswitch_back_time;
      std::vector<double> delayed_phoswitch_front_energy;
      std::vector<Label>  delayed_phoswitch_front_label;
      std::vector<Time>   delayed_phoswitch_front_time;
      std::vector<double> delayed_phoswitch_back_energy;
      std::vector<Label>  delayed_phoswitch_back_label;
      std::vector<Time>   delayed_phoswitch_back_time;

      std::vector<double> prompt_paris_module_front_energy;
      std::vector<uchar>  prompt_paris_module_front_index;
      std::vector<Time>  prompt_paris_module_front_time;
      std::vector<double> prompt_paris_module_back_energy;
      std::vector<uchar>  prompt_paris_module_back_index;
      std::vector<Time>  prompt_paris_module_back_time;
      std::vector<double> delayed_paris_module_front_energy;
      std::vector<uchar>  delayed_paris_module_front_index;
      std::vector<Time>  delayed_paris_module_front_time;
      std::vector<double> delayed_paris_module_back_energy;
      std::vector<uchar>  delayed_paris_module_back_index;
      std::vector<Time>  delayed_paris_module_back_time;

      std::vector<size_t> hits_added;

      int evt_i = 1;
      for ( ;(evt_i < chain->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        if (evt_i%freq_hit_display == 0) print(nicer_double(evt_i, 0), "events");

        chain->GetEntry(evt_i);

        prompt_phoswitch_front_energy.clear();
        prompt_phoswitch_front_label.clear();
        prompt_phoswitch_front_time.clear();
        prompt_phoswitch_back_energy.clear();
        prompt_phoswitch_back_label.clear();
        prompt_phoswitch_back_time.clear();
        delayed_phoswitch_front_energy.clear();
        delayed_phoswitch_front_label.clear();
        delayed_phoswitch_front_time.clear();
        delayed_phoswitch_back_energy.clear();
        delayed_phoswitch_back_label.clear();
        delayed_phoswitch_back_time.clear();

        prompt_paris_module_front_energy.clear();
        prompt_paris_module_front_index.clear();
        prompt_paris_module_front_time.clear();
        prompt_paris_module_back_energy.clear();
        prompt_paris_module_back_index.clear();
        prompt_paris_module_back_time.clear();
        delayed_paris_module_front_energy.clear();
        delayed_paris_module_front_index.clear();
        delayed_paris_module_front_time.clear();
        delayed_paris_module_back_energy.clear();
        delayed_paris_module_back_index.clear();
        delayed_paris_module_back_time.clear();

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          auto const & time  = event.times [hit_i];
          auto const & nrj   = event.nrjs  [hit_i];
          auto const & nrj2  = event.nrj2s [hit_i];

          if (Paris::is[label])
          {
            // Calibrate the phoswitch :
            auto const & nrjcal = calibPhoswitches.calibrate(label, nrj, nrj2);
            if (-5_ns < time && time < 5_ns) // Prompt
            {
              prompt_each_phoswitch[label]->Fill(nrjcal);
              prompt_each_nrjcal_VS_nrj[label]->Fill(nrj, nrjcal);
              if (Paris::cluster[label] == 0)
              {
                prompt_phoswitch_back_energy.push_back(nrjcal);
                prompt_phoswitch_back_label.push_back(label);
                prompt_phoswitch_back_time.push_back(time);
              }
              else if (Paris::cluster[label] == 1)
              {
                prompt_phoswitch_front_energy.push_back(nrjcal);
                prompt_phoswitch_front_label.push_back(label);
                prompt_phoswitch_front_time.push_back(time);
              }
              else throw_error(concatenate("error Paris::cluster lookup table ", label, " : ", (int)Paris::cluster[label]));
            }
            else if (40_ns < time && time < 170_ns) // Delayed
            {
              delayed_each_phoswitch[label]->Fill(nrjcal);
              delayed_each_nrjcal_VS_nrj[label]->Fill(nrj, nrjcal);
              if (Paris::cluster[label] == 0)
              {
                delayed_phoswitch_back_energy.push_back(nrjcal);
                delayed_phoswitch_back_label.push_back(label);
                delayed_phoswitch_back_time.push_back(time);
              }
              else if (Paris::cluster[label] == 1)
              {
                delayed_phoswitch_front_energy.push_back(nrjcal);
                delayed_phoswitch_front_label.push_back(label);
                delayed_phoswitch_front_time.push_back(time);
              }
              else throw_error(concatenate("error Paris::cluster lookup table ", label, " : ", (int)Paris::cluster[label]));
            }
          }

        }// End hits loop

        //////////////
        // Analyse  //
        //////////////

        auto addback = [&](std::vector<double> const & phoswitch_energy, std::vector<Label> const & phoswitch_label, std::vector<Time> const & phoswitch_time,
                          std::vector<double> & module_energy, std::vector<uchar> & module_index, std::vector<Time> & module_time)
        {
          auto const & phoswitch_mult = phoswitch_energy.size();
          hits_added.resize(phoswitch_mult, false);

          // 1. Order the hits from the highest to lowest energy deposit
          std::vector<size_t> paris_hits_ordered; paris_hits_ordered.reserve(phoswitch_mult);
          for (size_t hit_i = 0; hit_i<phoswitch_mult; ++hit_i) paris_hits_ordered[hit_i] = hit_i;
          std::sort(paris_hits_ordered.begin(), paris_hits_ordered.end(), [&phoswitch_energy](int hit_i, int hit_j)
          {
            return phoswitch_energy[hit_i]>phoswitch_energy[hit_j];
          });

          // 2. Perform the add-back
          for (size_t ordered_loop_i = 0; ordered_loop_i<phoswitch_mult; ++ordered_loop_i)
          {
            auto const & hit_i = paris_hits_ordered[ordered_loop_i]; // Starts with the highest energy deposit
            if (hits_added[hit_i]) continue; // If this hit has already been used for add-back with a previous hit then discard it

            auto const & nrj_i   = phoswitch_energy[hit_i];
            auto const & time_i  = phoswitch_time  [hit_i];
            auto const & label_i = phoswitch_label [hit_i]; // The detector's global label

            auto const & index_i = Paris::index[label_i]; // The index of the detector in its cluster (see Paris and ParisCluster class)

            module_energy.push_back(nrj_i);
            module_time.push_back(time_i);
            module_index.push_back(index_i);

            // Test each other detector in the event :
            for (size_t ordered_loop_j = ordered_loop_i+1; ordered_loop_j<phoswitch_mult; ordered_loop_j++)
            {
              auto const & hit_j = paris_hits_ordered[ordered_loop_j]; 
              auto const & time_j = phoswitch_time[hit_j];
              // Timing : if they are not simultaneous then they don't belong to the same event
              if (abs(time_j-time_i) > time_window) continue; 

              // Distance : if the phoswitches are physically too far away they are unlikely to be a Compton scattering of the same gamma
              auto const & label_j = phoswitch_label[hit_j]; 
              auto const & index_j = Paris::index[label_j];
              auto const & distance_ij = ParisCluster<Paris::cluster_size>::distances[index_i][index_j];
              if (distance_ij > distance_max) continue;

              // If the two hits meets the criteria they are add-backed :
              module_energy.back()+=phoswitch_energy[hit_j];
              hits_added[hit_j] = true;
            } // End j loop
          }

          if (phoswitch_mult>0 && verbose)
          {
            print("______________");
            print("event", evt_i, event);
            print("phoswitches : energy, label, time");
            for (size_t i = 0; i<phoswitch_mult; i++) print(phoswitch_energy[i], phoswitch_label[i], phoswitch_time[i]);
            print("modules : energy, index, time");
            for (size_t i = 0; i<module_energy.size(); i++) print(module_energy[i], module_index[i], module_time[i]);
            pauseCo();
          }
        }; // End of addback definition
        
        // Perform add-back :
        addback(prompt_phoswitch_back_energy, prompt_phoswitch_back_label, prompt_phoswitch_back_time, prompt_paris_module_back_energy, prompt_paris_module_back_index, prompt_paris_module_back_time);
        addback(prompt_phoswitch_front_energy, prompt_phoswitch_front_label, prompt_phoswitch_front_time, prompt_paris_module_front_energy, prompt_paris_module_front_index, prompt_paris_module_front_time);
        addback(delayed_phoswitch_back_energy, delayed_phoswitch_back_label, delayed_phoswitch_back_time, delayed_paris_module_back_energy, delayed_paris_module_back_index, delayed_paris_module_back_time);
        addback(delayed_phoswitch_front_energy, delayed_phoswitch_front_label, delayed_phoswitch_front_time, delayed_paris_module_front_energy, delayed_paris_module_front_index, delayed_paris_module_front_time);
        
        for (size_t hit_i = 0; hit_i<prompt_paris_module_back_energy.size(); ++hit_i) prompt_each_module[Paris::label(0, prompt_paris_module_back_index[hit_i])]->Fill(prompt_paris_module_back_energy[hit_i]);
        for (size_t hit_i = 0; hit_i<prompt_paris_module_front_energy.size(); ++hit_i) prompt_each_module[Paris::label(1, prompt_paris_module_front_index[hit_i])]->Fill(prompt_paris_module_front_energy[hit_i]);
        for (size_t hit_i = 0; hit_i<delayed_paris_module_back_energy.size(); ++hit_i) delayed_each_module[Paris::label(0, delayed_paris_module_back_index[hit_i])]->Fill(delayed_paris_module_back_energy[hit_i]);
        for (size_t hit_i = 0; hit_i<delayed_paris_module_front_energy.size(); ++hit_i) delayed_each_module[Paris::label(1, delayed_paris_module_front_index[hit_i])]->Fill(delayed_paris_module_front_energy[hit_i]);
      }// End events loop

      total_hits_number.fetch_add(evt_i, std::memory_order_relaxed);

      // Writing the file (the mutex protects potential concurency issues)
      lock_mutex lock(write_mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
        file->cd();


        // // Paris :
        // LaBr3_cristal_prompt->Write("LaBr3_cristal_prompt", TObject::kOverwrite);
        // LaBr3_cristal_prompt->Write("LaBr3_cristal_prompt", TObject::kOverwrite);
        // NaI_cristal_prompt->Write("NaI_cristal_prompt", TObject::kOverwrite);
        // NaI_cristal_prompt->Write("NaI_cristal_prompt", TObject::kOverwrite);
        // nrj2_vs_nrj_prompt->Write("nrj2_vs_nrj_prompt", TObject::kOverwrite);
        // nrj2_vs_nrj_delayed->Write("nrj2_vs_nrj_delayed", TObject::kOverwrite);
        // paris_prompt->Write("paris_prompt", TObject::kOverwrite);
        // paris_delayed->Write("paris_delayed", TObject::kOverwrite);
        // LaBr3_prompt->Write("LaBr3_prompt", TObject::kOverwrite);
        // LaBr3_delayed->Write("LaBr3_delayed", TObject::kOverwrite);

        // ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        // ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        // ge_VS_LaBr3_prompt->Write("ge_VS_LaBr3_prompt", TObject::kOverwrite);
        // ge_VS_LaBr3_delayed->Write("ge_VS_LaBr3_delayed", TObject::kOverwrite);
        // ge_VS_LaBr3_prompt->Write("ge_VS_LaBr3_prompt", TObject::kOverwrite);

        for (Label label = 300; label<800; label++) if (Paris::is[label])
        {
          auto const & label_str = std::to_string(label);
          delayed_each_nrjcal_VS_nrj.at(label)->Write(("delayed_nrjcal_VS_nrj_"+label_str).c_str(), TObject::kOverwrite);
          prompt_each_phoswitch.at(label)->Write(("prompt_phoswitch_"+label_str).c_str(), TObject::kOverwrite);
          prompt_each_module.at(label)->Write(("prompt_module_"+label_str).c_str(), TObject::kOverwrite);
          prompt_each_nrjcal_VS_nrj.at(label)->Write(("prompt_nrjcal_VS_nrj_"+label_str).c_str(), TObject::kOverwrite);
          delayed_each_phoswitch.at(label)->Write(("delayed_phoswitch_"+label_str).c_str(), TObject::kOverwrite);
          delayed_each_module.at(label)->Write(("delayed_module_"+label_str).c_str(), TObject::kOverwrite);
        }
        
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
// g++ -g -o macroParisVerif macroParisVerif.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o macroParisVerif macroParisVerif.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17