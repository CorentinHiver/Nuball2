#include "../lib/libRoot.hpp"
#include "../lib/Classes/RF_Manager.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "MyClovers.hpp"

void macro2(int nb_files = -1, double nb_hits_read = 1.e+200, int nb_threads = 10)
{
  // std::string trigger = "PrM1DeC1";
  std::string trigger = "C2";
  // std::string trigger = "P";

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::Initialise(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  
  MTObject::parallelise_function([&](){

    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    while(MTfiles.getNext(file))
    {
      // Simple spectra :
      std::unique_ptr<TH2F> pp_raw (new TH2F(("pp_raw_"+std::to_string(thread_i)).c_str(), "pp_raw", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> p_raw (new TH1F(("p_raw_"+std::to_string(thread_i)).c_str(), "p_raw", 4096,0,4096));
      std::unique_ptr<TH2F> dd_raw (new TH2F(("dd_raw_"+std::to_string(thread_i)).c_str(), "dd_raw", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d_raw (new TH1F(("d_raw_"+std::to_string(thread_i)).c_str(), "d_raw", 4096,0,4096));
      std::unique_ptr<TH2F> E_dT_raw (new TH2F(("E_dT_raw_"+std::to_string(thread_i)).c_str(), "E_dT_raw", 600,-100,200, 4096,0,4096));
      std::unique_ptr<TH2F> pp_add_back (new TH2F(("pp_add_back_"+std::to_string(thread_i)).c_str(), "pp_add_back", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> p_add_back (new TH1F(("p_add_back_"+std::to_string(thread_i)).c_str(), "p_add_back", 4096,0,4096));
      std::unique_ptr<TH2F> dd_add_back (new TH2F(("dd_add_back_"+std::to_string(thread_i)).c_str(), "dd_add_back", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d_add_back (new TH1F(("d_add_back_"+std::to_string(thread_i)).c_str(), "d_add_back", 4096,0,4096));
      std::unique_ptr<TH2F> E_dT_add_back (new TH2F(("E_dT_add_back_"+std::to_string(thread_i)).c_str(), "E_dT_add_back", 600,-100,200, 4096,0,4096));
      std::unique_ptr<TH2F> pp_clean (new TH2F(("pp_"+std::to_string(thread_i)).c_str(), "pp clean", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> p_clean (new TH1F(("p_"+std::to_string(thread_i)).c_str(), "p clean", 4096,0,4096));
      std::unique_ptr<TH2F> dd_clean (new TH2F(("dd_"+std::to_string(thread_i)).c_str(), "dd clean", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d_clean (new TH1F(("d_"+std::to_string(thread_i)).c_str(), "d clean", 4096,0,4096));
      std::unique_ptr<TH2F> E_dT_clean (new TH2F(("E_dT_"+std::to_string(thread_i)).c_str(), "E_dT clean", 600,-100,200, 4096,0,4096));
      std::unique_ptr<TH2F> pp_rejected (new TH2F(("pp_rejected_"+std::to_string(thread_i)).c_str(), "pp_rejected", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> p_rejected (new TH1F(("p_rejected_"+std::to_string(thread_i)).c_str(), "p_rejected", 4096,0,4096));
      std::unique_ptr<TH2F> dd_rejected (new TH2F(("dd_rejected_"+std::to_string(thread_i)).c_str(), "dd_rejected", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d_rejected (new TH1F(("d_rejected_"+std::to_string(thread_i)).c_str(), "d_rejected", 4096,0,4096));

      // Multiplicity :
      std::unique_ptr<TH1F> prompt_raw_mult (new TH1F(("prompt_raw_mult"+std::to_string(thread_i)).c_str(), "prompt raw multiplicity", 50,0,50));
      std::unique_ptr<TH1F> delayed_raw_mult (new TH1F(("delayed_raw_mult"+std::to_string(thread_i)).c_str(), "delayed raw multiplicity", 50,0,50));
      std::unique_ptr<TH1F> prompt_add_back_mult (new TH1F(("prompt_add_back_mult"+std::to_string(thread_i)).c_str(), "prompt add_back multiplicity", 50,0,50));
      std::unique_ptr<TH1F> delayed_add_back_mult (new TH1F(("delayed_add_back_mult"+std::to_string(thread_i)).c_str(), "delayed add_back multiplicity", 50,0,50));
      std::unique_ptr<TH1F> prompt_clean_mult (new TH1F(("prompt_clean_mult"+std::to_string(thread_i)).c_str(), "prompt clean multiplicity", 50,0,50));
      std::unique_ptr<TH1F> delayed_clean_mult (new TH1F(("delayed_clean_mult"+std::to_string(thread_i)).c_str(), "delayed clean multiplicity", 50,0,50));
      std::unique_ptr<TH1F> prompt_rejected_mult (new TH1F(("prompt_rejected_mult"+std::to_string(thread_i)).c_str(), "prompt rejected multiplicity", 50,0,50));
      std::unique_ptr<TH1F> delayed_rejected_mult (new TH1F(("delayed_rejected_mult"+std::to_string(thread_i)).c_str(), "delayed rejected multiplicity", 50,0,50));

      // Different trigger condition (always using clean germaniums): 
      std::unique_ptr<TH1F> d_VS_Clover_mult (new TH1F(("d_VS_Clover_mult_"+std::to_string(thread_i)).c_str(), "d_VS_Clover_mult", 4096,0,4096));
      std::unique_ptr<TH1F> d_VS_Module_mult (new TH1F(("d_VS_Module_mult_"+std::to_string(thread_i)).c_str(), "d_VS_Module_mult", 4096,0,4096));

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
      MyClovers all_clovers;
      MyClovers prompt_clovers;
      MyClovers delayed_clovers;

      for (int evt_i = 1; (evt_i < chain->GetEntries() && evt_i < nb_hits_read); evt_i++)
      {
        if (evt_i%((int)1.e+6) == 0) print(nicer_double(evt_i, 0), "events");

        chain->GetEntry(evt_i);

        all_clovers.reset();
        prompt_clovers.reset();
        delayed_clovers.reset();

        int prompt_rawMult = 0;
        int delayed_rawMult = 0;

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label_i = event.labels[hit_i];
          auto const & time_i =  event.times[hit_i];

          // Raw analysis :
          if (isGe(label_i)) 
          {
            auto const & nrj_i = event.nrjs[hit_i];
            E_dT_raw->Fill(time_i, nrj_i);
            if (-10_ns < time_i && time_i < 10_ns) 
            {
              ++prompt_rawMult;
              p_raw->Fill(nrj_i);
            }
            else if (60_ns < time_i && time_i < 170_ns) 
            {
              ++delayed_rawMult;
              d_raw->Fill(nrj_i);
            }
            for (int hit_j = hit_i+1; hit_j < event.mult; ++hit_j)
            {
              auto const & label_j = event.labels[hit_j];
              auto const & time_j = event.times[hit_j];
              if (isGe(label_j)) 
              {
                auto const & nrj_j = event.nrjs[hit_j];
                if (-10_ns < time_i && time_i < 10_ns && -10_ns < time_j && time_j < 10_ns) 
                {
                  pp_raw->Fill(nrj_i, nrj_j);
                  pp_raw->Fill(nrj_j, nrj_i);
                }
                if (60_ns < time_i && time_i < 170_ns && 60_ns < time_j && time_j < 170_ns) 
                {
                  dd_raw->Fill(nrj_i, nrj_j);
                  dd_raw->Fill(nrj_j, nrj_i);
                }
              }
            }
          }

          // Fill Clovers :
          all_clovers.fill(event, hit_i);
          if (-10_ns < time_i && time_i < 10_ns) prompt_clovers.fill(event, hit_i);       // Prompt
          else if (60_ns < time_i && time_i < 170_ns) delayed_clovers.fill(event, hit_i); // Delayed
        }// End hits loop

        /////////////////////
        // Analyse Clovers //
        /////////////////////
        all_clovers.analyze();
        prompt_clovers.analyze();
        delayed_clovers.analyze();

        //////// MULTIPLICITY ////////
        prompt_raw_mult->Fill(prompt_rawMult);
        delayed_raw_mult->Fill(delayed_rawMult);
        prompt_add_back_mult->Fill(prompt_clovers.Ge.size());
        delayed_add_back_mult->Fill(delayed_clovers.Ge.size());
        prompt_clean_mult->Fill(prompt_clovers.GeClean.size());
        delayed_clean_mult->Fill(delayed_clovers.GeClean.size());
        prompt_rejected_mult->Fill(prompt_clovers.Rejected.size());
        delayed_rejected_mult->Fill(delayed_clovers.Rejected.size());

        /////// ALL ADD-BACK ///////
        for (int loop_i = 0; loop_i<all_clovers.Ge.size(); ++loop_i)
        {
          auto const & clover_i = all_clovers[all_clovers.Ge[loop_i]];
          E_dT_add_back->Fill(clover_i.time, clover_i.nrj);
        }

        /////// PROMPT ADD-BACK ///////
        for (int loop_i = 0; loop_i<prompt_clovers.Ge.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.Ge[loop_i]];
          p_add_back->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<prompt_clovers.Ge.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.Ge[loop_j]];
            pp_add_back->Fill(clover_i.nrj, clover_j.nrj);
            pp_add_back->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// DELAYED ADD-BACK ///////
        for (int loop_i = 0; loop_i<delayed_clovers.Ge.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.Ge[loop_i]];
          d_add_back->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<delayed_clovers.Ge.size(); ++loop_j)
          {
            auto const & clover_j = delayed_clovers[delayed_clovers.Ge[loop_j]];
            dd_add_back->Fill(clover_i.nrj, clover_j.nrj);
            dd_add_back->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// ALL CLEAN ///////
        for (int loop_i = 0; loop_i<all_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = all_clovers[all_clovers.GeClean[loop_i]];
          E_dT_clean->Fill(clover_i.time, clover_i.nrj);
        }

        /////// PROMPT CLEAN ///////
        for (int loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.GeClean[loop_i]];
          p_clean->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            pp_clean->Fill(clover_i.nrj, clover_j.nrj);
            pp_clean->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// DELAYED CLEAN ///////
        for (int loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.GeClean[loop_i]];
          d_clean->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = delayed_clovers[delayed_clovers.GeClean[loop_j]];
            dd_clean->Fill(clover_i.nrj, clover_j.nrj);
            dd_clean->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// PROMPT REJECTED ///////
        for (int loop_i = 0; loop_i<prompt_clovers.Rejected.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.Rejected[loop_i]];
          p_rejected->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<prompt_clovers.Rejected.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.Rejected[loop_i]];
            pp_rejected->Fill(clover_i.nrj, clover_j.nrj);
            pp_rejected->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// DELAYED REJECTED ///////
        for (int loop_i = 0; loop_i<delayed_clovers.Rejected.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.Rejected[loop_i]];
          d_rejected->Fill(clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<delayed_clovers.Rejected.size(); ++loop_j)
          {
            auto const & clover_j = delayed_clovers[delayed_clovers.Rejected[loop_j]];
            dd_rejected->Fill(clover_i.nrj, clover_j.nrj);
            dd_rejected->Fill(clover_j.nrj, clover_i.nrj);
          }
        }
      }// End events loop
      lock_mutex lock(MTObject::mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
        file->cd();

        // Energy :
        pp_raw->Write("pp_raw", TObject::kOverwrite);
        p_raw->Write("p_raw", TObject::kOverwrite);
        dd_raw->Write("dd_raw", TObject::kOverwrite);
        d_raw->Write("d_raw", TObject::kOverwrite);
        E_dT_raw->Write("E_dT_raw", TObject::kOverwrite);
        pp_add_back->Write("pp_add_back", TObject::kOverwrite);
        p_add_back->Write("p_add_back", TObject::kOverwrite);
        dd_add_back->Write("dd_add_back", TObject::kOverwrite);
        d_add_back->Write("d_add_back", TObject::kOverwrite);
        E_dT_add_back->Write("E_dT_add_back", TObject::kOverwrite);
        pp_clean->Write("pp", TObject::kOverwrite);
        p_clean->Write("p", TObject::kOverwrite);
        dd_clean->Write("dd", TObject::kOverwrite);
        d_clean->Write("d", TObject::kOverwrite);
        E_dT_clean->Write("E_dT", TObject::kOverwrite);
        pp_rejected->Write("pp_rejected", TObject::kOverwrite);
        p_rejected->Write("p_rejected", TObject::kOverwrite);
        dd_rejected->Write("dd_rejected", TObject::kOverwrite);
        d_rejected->Write("d_rejected", TObject::kOverwrite);

        // Multiplicity :
        prompt_raw_mult->Write("prompt_raw_mult", TObject::kOverwrite);
        delayed_raw_mult->Write("delayed_raw_mult", TObject::kOverwrite);
        prompt_add_back_mult->Write("prompt_add_back_mult", TObject::kOverwrite);
        delayed_add_back_mult->Write("delayed_add_back_mult", TObject::kOverwrite);
        prompt_clean_mult->Write("prompt_clean_mult", TObject::kOverwrite);
        delayed_clean_mult->Write("delayed_clean_mult", TObject::kOverwrite);
        prompt_rejected_mult->Write("prompt_rejected_mult", TObject::kOverwrite);
        delayed_rejected_mult->Write("delayed_rejected_mult", TObject::kOverwrite);

        d_VS_Clover_mult->Write("d_VS_Clover_mult", TObject::kOverwrite);
        d_VS_Module_mult->Write("d_VS_Module_mult", TObject::kOverwrite);

      file->Close();
      print(out_filename, "written");
      // mutex freed
    }
  });
}

#ifndef __CINT__
int main(int argc, char** argv)
{
       if (argc == 1) macro2();
  else if (argc == 2) macro2(std::stoi(argv[1]));
  else if (argc == 3) macro2(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macro2(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}
#endif //__CINT__
// g++ -g -o exec macro2.C ` root-config --cflags` `root-config --glibs` -lSpectrum
// g++ -O2 -o exec macro2.C ` root-config --cflags` `root-config --glibs` -lSpectrum