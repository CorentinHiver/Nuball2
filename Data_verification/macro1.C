#include "../lib/libRoot.hpp"
#include "../lib/Classes/RF_Manager.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/MTObjects/MTList.hpp"

using Label = unsigned short;
inline bool isGe(Label const & label) {return (label < 200 && (label+1)%6 > 1);}
void macro1(int choice = 0, int nb_files = -1, int nb_threads = 10)
{
  bool oldVersion = (choice == 0);
  if (oldVersion) print("Treating OLD"); 
  else if (choice == 1) print("Treating new"); 
  else if (choice == 2) 
  {
    print("Treating new in one TChain");
    nb_files = 1;
    nb_threads = 1;
  }

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
      unique_ptr<TH2F> gg (new TH2F(("gg_"+std::to_string(thread_i)).c_str(), "gg", 4096,0,4096, 4096,0,4096));
      unique_ptr<TH2F> pp (new TH2F(("pp_"+std::to_string(thread_i)).c_str(), "pp", 4096,0,4096, 4096,0,4096));
      unique_ptr<TH2F> dd (new TH2F(("dd_"+std::to_string(thread_i)).c_str(), "dd", 4096,0,4096, 4096,0,4096));
      unique_ptr<TH2F> E_dT (new TH2F(("E_dT_"+std::to_string(thread_i)).c_str(), "E_dT", 600,-100,200, 4096,0,4096));

      auto const & filename = File(file).filename();
      auto const & run_name = filename.shortName();
      TChain* chain = new TChain("Nuball2");
      if (oldVersion) 
      {
        auto files = ("~/nuball2/N-SI-136-root_P/OLD/" + run_name + "/"+run_name+"*.root");
        print("Loading", files);
        chain->Add(files.c_str());
      }
      else if (choice == 1)
      {
        chain->Add(file.c_str());
        print("Reading", file);
      }
      else if (choice == 2)
      {
        auto const & filesChain = data_path.string()+"run*.root";
        chain->Add(filesChain.c_str());
        print("Reading", filesChain);
      }

      Folder outFolder;
      if (oldVersion) outFolder = "OLD2/";
      else if (choice == 1) outFolder = "new2/";
      else if (choice == 2) outFolder = "new3/";
      std::string out_filename = outFolder.string()+filename.string();

      int mult = 0;
      ULong64_t timestamp[255];
      Label label[255];
      Long64_t time[255];
      float energy[255];
      if (oldVersion)
      {
        chain -> SetBranchAddress("mult"  , &mult);
        chain -> SetBranchAddress("label" , &label);
        chain -> SetBranchAddress("time" , &timestamp);
        chain -> SetBranchAddress("nrjcal"  , &energy);
      }

      Event event;
      if (!oldVersion) event.reading(chain, "mltTEQ");
      RF_Manager rf;
      bool rf_found = false;

      for (int evt_i = 1; evt_i < chain->GetEntries(); evt_i++)
      {
        if (evt_i%((int)1.e+7) == 0) std::cout << evt_i/1.e+6 << "M events" << std::endl;
        chain->GetEntry(evt_i);

        auto const & multiplicity = (oldVersion) ? mult : event.mult;

        for (int hit_i = 0; hit_i < multiplicity; hit_i++)
        {
          auto const & label_i = (oldVersion) ? label[hit_i] : event.labels[hit_i];
          if (oldVersion)
          {
            if (label_i == 251) {rf.set(timestamp[hit_i], energy[hit_i]); rf_found = true; continue; }
            if (!rf_found) continue;
          }
          auto const & time_i = (oldVersion) ? rf.relTime_ns(timestamp[hit_i]) : event.times[hit_i]/1000.f;
          if (isGe(label_i)) 
          {
            auto const & nrj_i = (oldVersion) ? energy[hit_i] :  event.nrjs[hit_i];
            E_dT->Fill(time_i, nrj_i);
            for (int hit_j = hit_i+1; hit_j < multiplicity; ++hit_j)
            {
              auto const & label_j = (oldVersion) ? label[hit_j] : event.labels[hit_j];
              auto const & time_j = (oldVersion) ? rf.relTime_ns(timestamp[hit_j])  : event.times[hit_j]/1000.f;
              if (isGe(label_j)) 
              {
                auto const & nrj_j = (oldVersion) ? energy[hit_j] :  event.nrjs[hit_j];
                gg->Fill(nrj_i, nrj_j);
                gg->Fill(nrj_j, nrj_i);
                if (-10 < time_i && time_i < 10 && -10 < time_j && time_j < 10) 
                {
                  pp->Fill(nrj_i, nrj_j);
                  pp->Fill(nrj_j, nrj_i);
                }
                if (60 < time_i && time_i < 150 && 60 < time_j && time_j < 150) 
                {
                  dd->Fill(nrj_i, nrj_j);
                  dd->Fill(nrj_j, nrj_i);
                }
              }
            }
          }
        }// End hits loop
      }// End events loop
      lock_mutex lock(MTObject::mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
        file->cd();
        gg->Write("gg", TObject::kOverwrite);
        pp->Write("pp", TObject::kOverwrite);
        dd->Write("dd", TObject::kOverwrite);
        E_dT->Write("E_dT", TObject::kOverwrite);
      file->Close();
      // mutex freed
    }
  });
}