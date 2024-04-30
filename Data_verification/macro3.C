#include "../lib/libRoot.hpp"
#include "../lib/Classes/RF_Manager.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "MyClovers.hpp"
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

  MTObject::parallelise_function([&](){

    TRandom* random = new TRandom();
    random->SetSeed(time(0));

    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    while(MTfiles.getNext(file))
    {
      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);
      // Simple spectra :
      std::unique_ptr<TH2F> pp (new TH2F(("pp_"+std::to_string(thread_i)).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> p (new TH1F(("p_"+std::to_string(thread_i)).c_str(), "prompt", 4096,0,4096));
      std::unique_ptr<TH2F> dd (new TH2F(("dd_"+std::to_string(thread_i)).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096));
      std::unique_ptr<TH1F> d (new TH1F(("d_"+std::to_string(thread_i)).c_str(), "delayed", 4096,0,4096));
      std::unique_ptr<TH2F> E_dT (new TH2F(("E_dT_"+std::to_string(thread_i)).c_str(), "E_dT clean", 600,-100,200, 4096,0,4096));

      // Multiplicity :
      std::unique_ptr<TH1F> prompt_Module_mult (new TH1F(("prompt_Module_mult_"+std::to_string(thread_i)).c_str(), "prompt_Module_mult", 20,0,20));
      std::unique_ptr<TH1F> delayed_Module_mult (new TH1F(("delayed_Module_mult"+std::to_string(thread_i)).c_str(), "delayed_Module_mult", 20,0,20));
      std::unique_ptr<TH2F> d_VS_prompt_Module_mult (new TH2F(("d_VS_prompt_Module_mult_"+std::to_string(thread_i)).c_str(), "d_VS_prompt_Module_mult", 20,0,20, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_delayed_Module_mult (new TH2F(("d_VS_delayed_Module_mult"+std::to_string(thread_i)).c_str(), "d_VS_delayed_Module_mult", 20,0,20, 4096,0,4096));
      
      // Calorimetry :
      std::unique_ptr<TH1F> prompt_calorimetry (new TH1F(("prompt_calorimetry_"+std::to_string(thread_i)).c_str(), "prompt_calorimetry", 1000,0,10000));
      std::unique_ptr<TH1F> delayed_calorimetry (new TH1F(("delayed_calorimetry_"+std::to_string(thread_i)).c_str(), "delayed_calorimetry", 1000,0,10000));
      std::unique_ptr<TH2F> d_VS_prompt_calorimetry (new TH2F(("d_VS_prompt_calorimetry_"+std::to_string(thread_i)).c_str(), "d_VS_prompt_calorimetry", 1000,0,10000, 4096,0,4096));
      std::unique_ptr<TH2F> d_VS_delayed_calorimetry (new TH2F(("d_VS_delayed_calorimetry_"+std::to_string(thread_i)).c_str(), "d_VS_delayed_calorimetry", 1000,0,10000, 4096,0,4096));

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

      MyClovers prompt_clovers;
      MyClovers delayed_clovers;

      Paris prompt_paris;
      Paris delayed_paris;

      Calibration NaI_calib("../136/coeffs_NaI.calib");

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

        // Calorimetry :
        auto const & prompt_calo_clover = prompt_clovers.calorimetryBGO + smear(prompt_clovers.calorimetryGe, random);
        auto const & delayed_calo_clover = delayed_clovers.calorimetryBGO + smear(delayed_clovers.calorimetryGe, random);
        auto const & prompt_calo_paris = prompt_paris.NaI_calorimetry() + smear(prompt_paris.LaBr3_calorimetry(), random);
        auto const & delayed_calo_paris = delayed_paris.NaI_calorimetry() + smear(delayed_paris.LaBr3_calorimetry(), random);
        auto const & prompt_calo = prompt_calo_clover + prompt_calo_paris;
        auto const & delayed_calo = delayed_calo_clover + delayed_calo_paris;
        prompt_calorimetry->Fill(prompt_calo);
        delayed_calorimetry->Fill(delayed_calo);

        /////// PROMPT CLEAN ///////
        for (int loop_i = 0; loop_i<prompt_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = prompt_clovers[prompt_clovers.GeClean[loop_i]];
          p->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          for (int loop_j = loop_i+1; loop_j<prompt_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = prompt_clovers[prompt_clovers.GeClean[loop_j]];
            pp->Fill(clover_i.nrj, clover_j.nrj);
            pp->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        /////// DELAYED CLEAN ///////
        for (int loop_i = 0; loop_i<delayed_clovers.GeClean.size(); ++loop_i)
        {
          auto const & clover_i = delayed_clovers[delayed_clovers.GeClean[loop_i]];
          d->Fill(clover_i.nrj);
          E_dT->Fill(clover_i.time, clover_i.nrj);
          d_VS_delayed_Module_mult->Fill(prompt_mult, clover_i.nrj);
          d_VS_prompt_Module_mult->Fill(delayed_mult, clover_i.nrj);
          
          d_VS_prompt_calorimetry -> Fill(prompt_calo, clover_i.nrj);
          d_VS_delayed_calorimetry -> Fill(delayed_calo, clover_i.nrj);
          
          for (int loop_j = loop_i+1; loop_j<delayed_clovers.GeClean.size(); ++loop_j)
          {
            auto const & clover_j = delayed_clovers[delayed_clovers.GeClean[loop_j]];
            dd->Fill(clover_i.nrj, clover_j.nrj);
            dd->Fill(clover_j.nrj, clover_i.nrj);
          }
        }
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

        // Multiplicity :

        prompt_Module_mult->Write("prompt_Module_mult", TObject::kOverwrite);
        delayed_Module_mult->Write("delayed_Module_mult", TObject::kOverwrite);
        d_VS_prompt_Module_mult->Write("d_VS_prompt_Module_mult", TObject::kOverwrite);
        d_VS_delayed_Module_mult->Write("d_VS_delayed_Module_mult", TObject::kOverwrite);

        prompt_calorimetry->Write("prompt_calorimetry", TObject::kOverwrite);
        delayed_calorimetry->Write("delayed_calorimetry", TObject::kOverwrite);
        d_VS_prompt_calorimetry->Write("d_VS_prompt_calorimetry", TObject::kOverwrite);
        d_VS_delayed_calorimetry->Write("d_VS_delayed_calorimetry", TObject::kOverwrite);

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
// g++ -g -o exec macro3.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum
// g++ -O2 -o exec macro3.C ` root-config --cflags` `root-config --glibs` -lSpectrum