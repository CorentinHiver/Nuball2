#include "TROOT.h"
#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Detectors.hpp"
// #include "../lib/Classes/Calibration.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Analyse/SimpleDSSD.hpp"
// #include "../lib/Analyse/WarsawDSSD.hpp"
#include "../lib/Classes/Timer.hpp"
#include "CoefficientCorrection.hpp"
#include "Utils.h"

#define MaxMult 10
// #define MaxPM 10
// #define MaxDM 10
// #define MaxCalo 10_MeV
// #define MaxPC 10_MeV
#define MaxDC 3_MeV

#define ParticleTrigger

constexpr Time coincTw = 40_ns;

void macro6(int nb_files = -1, long long nbEvtMax = -1, int nb_threads = 10)
{
  std::string target = "U";
  std::string trigger = "C2";

  TH1::AddDirectory(false);

  Timer timer;
  long long freqEvtDisplay = nbEvtMax/10;
  if (nbEvtMax<0) 
  {
    nbEvtMax = Colib::big<long long>();
    freqEvtDisplay = 1.e+7;
  }

  if (nb_threads==0) throw_error("Need at least 1 thread, not" + std::to_string(nb_threads));
  if (nb_threads>1) MTObject::Initialise(nb_threads);
  
  detectors.load("../136/index_129.list");
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());

  std::mutex read_mutex;
  std::mutex write_mutex;
  
  MTObject::parallelise_function([&]()
  {
    
    // Multithreading starts here : 

    auto const & thread_i = MTObject::getThreadIndex();
    auto const & thread_i_str = std::to_string(thread_i);

    std::string file;

    while(MTfiles.getNext(file))
    {
      // -- Input file : -- //
      auto const & filename = removePath(file);
      auto const & run_name = removeExtension(filename);
      auto const & run_name_vector = split(run_name, '_');
      auto const & run_number_str = run_name_vector[1];
      int const & run_number = std::stoi(run_number_str);
      if (target == "Th" && run_number>72) continue;
      if (target == "U" && run_number<74) continue;

      // Nuball2Tree tree(file);
      // std::unique_ptr<TFile> infile; infile.reset(TFile::Open(file.c_str(), "READ"));
    read_mutex.lock();
      print("reading", file);
      TFile* infile = TFile::Open(file.c_str(), "READ");
      if (!infile || infile->IsZombie()){error("Can't read valid", file); continue;}
      TTree* tree = static_cast<TTree*>(infile->Get("Nuball2"));
      if (!tree || tree->IsZombie()) {error("Can't find valid Nuball2 tree in", filename); continue;}
    read_mutex.unlock();

      // -- Creating analysis modules -- //
      Event event;
      event.reading(tree, "mltTEQp");

      CloversV2 pclovers;
      CloversV2 dclovers;
  
      SimpleParis pparis(&calibPhoswitches);
      SimpleParis dparis(&calibPhoswitches);
  
      DSSD::Simple dssd;
      // WarsawDSSD dssd;

      // -- Output file : -- //

      std::string outFolder = "data/"+trigger+"/"+target+"/";
      Path::make(outFolder);
      std::string out_filename = outFolder+removeExtension(filename)+"_v2.root";
      File Filename(out_filename); Filename.makePath();
      TFile* outfile = TFile::Open(out_filename.c_str(), "recreate");
      // std::unique_ptr<TFile> outfile (TFile::Open(out_filename.c_str(), "recreate"));
  
      TH2F* pp = new TH2F(("pp"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd = new TH2F(("dd"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dp = new TH2F(("dp"+thread_i_str).c_str(), "gamma-gamma prompt-delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_pveto = new TH2F(("dd_pveto"+thread_i_str).c_str(), "gamma-gamma delayed pveto;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      
      // -- Start processing the data -- //

      auto const Nevt = tree->GetEntries();
  
      for (int evt_i = 0; (evt_i < Nevt && evt_i < nbEvtMax); evt_i++)
      {
        if (evt_i>0 && evt_i%freqEvtDisplay == 0) print(nicer_double(evt_i, 0), "events");
  
        tree->GetEntry(evt_i);

        // -- Clear modules -- //
  
        pclovers.clear();
        dclovers.clear();
  
        pparis.clear();
        dparis.clear();
  
        dssd.clear();

        std::vector<Hit> hits;

        // -- Fill modules -- //

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          auto const & time =  event.times[hit_i];
          auto const & nrj = event.nrjs[hit_i];
          auto const & nrj2 = event.nrj2s[hit_i];
  
          if (nrj < 20_keV) continue;
          if (nrj2 < 0) continue;
  
          if (label == 65 && run_number == 116) continue; // This detector's timing slipped in this run
          if ((label == 134 || label == 135 || label == 136) && time > 100_ns) continue; // These detectors have strange events after 100 ns
  
               if (Paris::is[label])
          {
                 if (gate(-10_ns, time,  10_ns)) pparis.fill(event, hit_i);
            else if (gate( 40_ns, time, 180_ns)) dparis.fill(event, hit_i);
          }
          else if (CloversV2::is[label])
          {
                 if (gate(-10_ns, time,  10_ns)) pclovers.fill(event, hit_i);
            else if (gate( 40_ns, time, 180_ns)) dclovers.fill(event, hit_i);
          }
          else if (DSSD::is[label]) dssd.fill(event, hit_i);
          hits.push_back(event[hit_i]);
        }

        // -- Analyse modules -- //
  
        pclovers.analyze();
        dclovers.analyze();
        pparis.analyze();
        dparis.analyze();
        dssd.analyze();

        // -- Multiplicity -- //
  
        auto const & PMclover = pclovers.all.size();
        auto const & PMparis  = pparis.module_mult();
        auto const & PM = PMclover + PMparis;
  
        auto const & DMclover = dclovers.all.size();
        auto const & DMparis  = dparis.module_mult();
        auto const & DM = DMclover + DMparis;

        auto const & Mtot = PM + DM;

      #ifdef MaxMult
        if (Mtot > MaxMult) continue;
      #endif //MaxMult

        // -- Calorimetry -- //

        auto const & PC_clover = pclovers.calorimetryTotal;
        auto const & PC_paris  = pparis.calorimetry();
        auto const & PC = PC_clover + PC_paris;

        auto const & DC_clover = dclovers.calorimetryTotal;
        auto const & DC_paris  = dparis.calorimetry();
        auto const & DC = DC_clover + DC_paris;

        auto const & Ctot = PC + DC;

      #ifdef MaxDC
        if (DC > MaxDC) continue;
      #endif //MaxDC

        bool dssdTrigger = dssd.mult() > 0;

      #ifdef ParticleTrigger
        if (!dssdTrigger) continue;
      #endif //ParticleTrigger

        // -- Prompt -- //
        for (size_t loop_i = 0; loop_i<pclovers.clean.size(); ++loop_i)
        {
          auto const & clover0 = pclovers.clean[loop_i];

          // -- Prompt-prompt -- //
          for (size_t loop_j = loop_i+1; loop_j<pclovers.clean.size(); ++loop_j)
          {
            auto const & clover1 = pclovers.clean[loop_j];
            pp->Fill(clover0->nrj, clover1->nrj);
            pp->Fill(clover1->nrj, clover0->nrj);
          }

          // -- Prompt-delayed -- //
          for (size_t loop_j = 0; loop_j<dclovers.clean.size(); ++loop_j)
          {
            auto const & dclover = dclovers.clean[loop_j];
            dp->Fill(clover0->nrj, dclover->nrj);
          }
        }

        // -- Delayed -- //
        for (size_t loop_i = 0; loop_i<dclovers.clean.size(); ++loop_i)
        {
          auto const & clover0 = dclovers.clean[loop_i];
          // -- Delayed-delayed -- //
          for (size_t loop_j = loop_i+1; loop_j<dclovers.clean.size(); ++loop_j)
          {
            auto const & clover1 = dclovers.clean[loop_j];
            if (Colib::abs(clover0->time - clover1->time) > coincTw) continue;
            dd->Fill(clover0->nrj, clover1->nrj);
            dd->Fill(clover1->nrj, clover0->nrj);
            
            // -- Prompt veto -- /:
            if (PM == 0)
            {
              dd_pveto->Fill(clover0->nrj, clover1->nrj);
              dd_pveto->Fill(clover1->nrj, clover0->nrj);
            }
          }
        }
      }

      
      lock_mutex lock(write_mutex);
      
      print("writing spectra in", out_filename, "...");

      outfile->cd();
        pp -> Write("pp", TObject::kOverwrite);
        dd -> Write("dd", TObject::kOverwrite);
        dp -> Write("dp", TObject::kOverwrite);
        dd_pveto -> Write("dd_pveto", TObject::kOverwrite);

      outfile->Close();
      delete outfile;

      infile->Close();
      delete infile;

      delete pp;
      delete dd;
      delete dp;
      delete dd_pveto;

      print(out_filename, "written"); 
    }
  // Multithreading stops here. 
  });

  // -- Merging the files from each run -- //
  std::string dest = "data/merge_"+trigger+"_"+target+"_v2.root";
  std::string source = "data/"+trigger+"/"+target+"/run_*_v2.root";
  std::string nb_threads_str = std::to_string(nb_threads);
  std::string command = "hadd -f -j "+ nb_threads_str+ " -d . "+ dest + " " + source;
  print(command);
  gSystem->Exec(command.c_str());
  print(timer());
}

int main(int argc, char** argv)
{
       if (argc == 1) macro6();
  else if (argc == 2) macro6(std::stoi(argv[1]));
  else if (argc == 3) macro6(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macro6(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}
// g++ -Og -g -o exec macro6.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro6.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17