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
#include "../lib/Classes/CalibAndScale.hpp"
#include "../lib/Classes/Timer.hpp"
#include "CoefficientCorrection.hpp"
#include "Utils.h"

/// V3 : use of alignement

#define MaxMult 10

#ifdef KISOMER
  #define ParticleTrigger
  #define MaxMultDelayed 6
  #define MaxCaloDelayed 3_MeV
  #define MinMultDelayed 1
  #define FILE_SUFFIXE "Kisomer"
#endif // KISOMER

#ifdef SHAPEISOMER
  #define MaxCaloPrompt 3_MeV
  #define MinMultDelayed 1
  #define MaxMultDelayed 5
  #define MinCaloDelayed 1.2_MeV
  #define MaxCaloDelayed 3.5_MeV
  #define FILE_SUFFIXE "ShapeIsomer"
#endif //SHAPEISOMER

#ifdef PROMPT_ONLY
  #define MaxCaloPrompt 3_MeV
  #define MaxMultPrompt 10
  #define MaxMultDelayed 0
  #define FILE_SUFFIXE "PromptOnly"
#endif //PROMPT_ONLY

constexpr Time coincTw = 40_ns;

void macro6(int nb_files = -1, long long nbEvtMax = -1, int nb_threads = 10)
{
  CloversV2::setBlacklist({46, 55, 64, 69, 70, 80, 92, 97, 122, 129, 142, 163});

  std::string target  = "U" ;
  std::string dataset = "C2";

  TH1::AddDirectory(false);

  Timer timer;
  long long freqEvtDisplay = nbEvtMax/10;
  if (nbEvtMax<0) 
  {
    nbEvtMax = Colib::big<long long>();
    freqEvtDisplay = 1.e+7;
  }

  if (nb_threads==0) Colib::throw_error("Need at least 1 thread, not" + std::to_string(nb_threads));
  if (nb_threads>1) MTObject::Initialise(nb_threads);
  
  detectors.load("../136/index_129.list");
  
  // Get the calibration parameters :
    // Paris rotation angles :
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  // Alignement :

  CalibAndScales::verbose(0);
  static std::unordered_map<int, CalibAndScales> alignements;
  for (auto const & detector : detectors)
  {
    if (detector == "") continue;
    auto const & label = detectors[detector];
    alignements.emplace(label, "Alignement/"+detector+".align");
  }
  
  auto align = [](Event & event, int const & hit_i, int const & run_number) -> bool
  {
    if (!key_found(alignements, run_number)) return false;
    auto const & alignement = alignements.at(event.labels[hit_i]);
    if (!alignement.hasRun(run_number)) return false;
    event.nrjs[hit_i] = alignement[run_number].linear_inv_calib(event.nrjs[hit_i]);
    return true;
  };

  // Data files :

  Path data_path("~/nuball2/N-SI-136-root_"+dataset+"/merged/");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());

  // Prepare parallelisation :

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

      TFile* infile = nullptr;
      TTree* tree   = nullptr;

      { // Have to lock the multithreading because of ROOT internally-managed pointers
        lock_mutex lock(read_mutex);
        print("reading", file);
        infile = TFile::Open(file.c_str(), "READ");
        if (!infile || infile->IsZombie()){error("Can't read valid", file); continue;}
        tree = static_cast<TTree*>(infile->Get("Nuball2"));
        if (!tree || tree->IsZombie()) {error("Can't find valid Nuball2 tree in", filename); continue;}
      }

      // -- Creating analysis modules -- //
      Event event;
      event.reading(tree, "mltTEQp");

      CloversV2 pclovers;
      CloversV2 dclovers;
  
      SimpleParis pparis(&calibPhoswitches);
      SimpleParis nparis(&calibPhoswitches);
      SimpleParis dparis(&calibPhoswitches);
      std::vector<SimpleParisModule*> parisModules;
  
      DSSD::Simple dssd;

      // -- Output file : -- //

      std::string outFolder = "data/"+dataset+"/"+target+"/";

    #ifdef FILE_SUFFIXE
      outFolder+=FILE_SUFFIXE;
    #endif //FILE_SUFFIXE

      Path::make(outFolder);
      std::string out_filename = outFolder+removeExtension(filename)+"_v3.root";
      File Filename(out_filename); Filename.makePath();
      TFile* outfile = TFile::Open(out_filename.c_str(), "recreate");
      // std::unique_ptr<TFile> outfile (TFile::Open(out_filename.c_str(), "recreate"));
      
      TH2F* pp = new TH2F(("pp"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd = new TH2F(("dd"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_PM1 = new TH2F(("dd_PM1"+thread_i_str).c_str(), "gamma-gamma delayed Prompt Multiplicity #supeq 1;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dp = new TH2F(("dp"+thread_i_str).c_str(), "gamma-gamma prompt-delayed;Prompt#gamma[keV];Delayed#gamma[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_pveto = new TH2F(("dd_pveto"+thread_i_str).c_str(), "gamma-gamma delayed pveto;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      
      TH2F* dd_PC3DC3 = new TH2F(("dd_PC3DC3"+thread_i_str).c_str(), "PC3DC3 gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_PM1_PC3DC3 = new TH2F(("dd_PM1_PC3DC3"+thread_i_str).c_str(), "PC3DC3 gamma-gamma delayed Prompt Multiplicity #supeq 1;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dp_PC3DC3 = new TH2F(("dp_PC3DC3"+thread_i_str).c_str(), "PC3DC3 gamma-gamma prompt-delayed;Prompt#gamma[keV];Delayed#gamma[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_pveto_PC3DC3 = new TH2F(("dd_pveto_PC3DC3"+thread_i_str).c_str(), "PC3DC3 gamma-gamma delayed pveto;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);

      TH2F* d_tParis = new TH2F(("d_tParis"+thread_i_str).c_str(), "gamma VS time paris;E1[keV];time[ps]", 4096,0,4096, 210,-25_ns,185_ns);
      
      ////////////////////////
      // -- PROCESS DATA -- //
      ////////////////////////

      auto const Nevt = tree->GetEntries();
  
      for (int evt_i = 0; (evt_i < Nevt && evt_i < nbEvtMax); evt_i++)
      {
        if (evt_i>0 && evt_i%freqEvtDisplay == 0) print(Colib::nicer_double(evt_i, 0), "events");
  
        tree->GetEntry(evt_i);

        // -- Clear modules -- //

        pclovers.clear();
        dclovers.clear();
  
        pparis.clear();
        nparis.clear();
        dparis.clear();
        parisModules.clear();
  
        dssd.clear();

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
                if (gate(-10_ns, time,  10_ns)) 
            {
              if (!align(event, hit_i, run_number)) continue;
              pparis.fill(event, hit_i);
            }
            else if (gate(10_ns, time,  40_ns)) 
            {
              if (!align(event, hit_i, run_number)) continue;
              nparis.fill(event, hit_i);
            }
            else if (gate( 40_ns, time, 180_ns)) 
            {
              if (!align(event, hit_i, run_number)) continue;
              dparis.fill(event, hit_i);
            }
          }
          else if (CloversV2::is[label])
          {
                if (gate(-10_ns, time,  10_ns)) 
            {
              if (!align(event, hit_i, run_number)) continue;
              pclovers.fill(event, hit_i);
            }

            else if (gate( 40_ns, time, 180_ns)) 
            {
              if (!align(event, hit_i, run_number)) continue;
              dclovers.fill(event, hit_i);
            }

          }
          else if (DSSD::is[label]) dssd.fill(event, hit_i);
        }

        // -- Analyse modules -- //
  

        pclovers.analyze();
        dclovers.analyze();
        pparis.analyze();
        nparis.analyze();
        dparis.analyze();
        // Easily accessible paris modules :
        for (auto & pmodule : pparis.modules) parisModules.push_back(pmodule);
        for (auto & pmodule : nparis.modules) parisModules.push_back(pmodule);
        for (auto & pmodule : dparis.modules) parisModules.push_back(pmodule);
        dssd.analyze();

        ////////////////////////
        // -- Multiplicity -- //
        ////////////////////////
        
          // -- Prompt multiplicity -- //
        auto const & PMclover = pclovers.all.size();
        auto const & PMparis  = pparis.module_mult();
        auto const & PM = PMclover + PMparis;
  
      #ifdef MaxMultPrompt
        if (PM > MaxMultPrompt) continue;
      #endif //MaxMultPrompt

          // -- Delayed multiplicity -- //
        auto const & DMclover = dclovers.all.size();
        auto const & DMparis  = dparis.module_mult();
        auto const & DM = DMclover + DMparis;

      #ifdef MaxMultDelayed
        if (DM > MaxMultDelayed) continue;
      #endif //MaxMultDelayed
      #ifdef MinMultDelayed
        if (DM < MinMultDelayed) continue;
      #endif //MinMultDelayed

          // -- Total multiplicity -- //
        auto const & Mtot = PM + DM;

      #ifdef MaxMult
        if (Mtot > MaxMult) continue;
      #endif //MaxMult
      #ifdef MinMult
        if (Mtot < MinMult) continue;
      #endif //MinMult

        ///////////////////////
        // -- Calorimetry -- //
        ///////////////////////

          // -- Prompt calorimetry -- //
        auto const & PC_clover = pclovers.calorimetryTotal;
        auto const & PC_paris  = pparis.calorimetry();
        auto const & PC = PC_clover + PC_paris;

      #ifdef MaxCaloPrompt
        if (PC > MaxCaloPrompt) continue;
      #endif //MaxCaloPrompt

          // -- Delayed calorimetry -- //
        auto const & DC_clover = dclovers.calorimetryTotal;
        auto const & DC_paris  = dparis.calorimetry();
        auto const & DC = DC_clover + DC_paris;

      #ifdef MaxCaloDelayed
        if (DC > MaxCaloDelayed) continue;
      #endif //MaxCaloDelayed
      #ifdef MinCaloDelayed
        if (DC < MinCaloDelayed) continue;
      #endif //MinCaloDelayed

          // -- Total calorimetry -- //

        auto const & Ctot = PC + DC;

      #ifdef MaxCalo
        if (Ctot > MaxCalo) continue;
      #endif //MaxCalo
      #ifdef MinCalo
        if (Ctot < MinCalo) continue;
      #endif //MinCalo

        /////////////////////
        // -- PARTICLES -- //
        /////////////////////

        bool dssdTrigger = dssd.mult() > 0;

      #ifdef ParticleTrigger
        if (!dssdTrigger) continue;
      #endif //ParticleTrigger

        /////////////////////
        // -- ANALYSIS -- //
        /////////////////////

        // -- Prompt -- //
        for (size_t loop_i = 0; loop_i<pclovers.clean.size(); ++loop_i)
        {
          auto const & pclover0 = pclovers.clean[loop_i];

          // -- Prompt-prompt -- //
          for (size_t loop_j = loop_i+1; loop_j<pclovers.clean.size(); ++loop_j)
          {
            auto const & pclover1 = pclovers.clean[loop_j];
            pp->Fill(pclover0->nrj, pclover1->nrj);
            pp->Fill(pclover1->nrj, pclover0->nrj);
          }

          // -- Prompt-delayed -- //
          for (auto dclover : dclovers.clean)
          {
            dp->Fill(pclover0->nrj, dclover->nrj);
          }
        }

        // -- Delayed -- //
        for (size_t loop_i = 0; loop_i<dclovers.clean.size(); ++loop_i)
        {
          auto const & dclover0 = dclovers.clean[loop_i];

          // -- Delayed-delayed -- //
          for (size_t loop_j = loop_i+1; loop_j<dclovers.clean.size(); ++loop_j)
          {
            auto const & dclover1 = dclovers.clean[loop_j];
            if (Colib::abs(dclover0->time - dclover1->time) > coincTw) continue;
            dd->Fill(dclover0->nrj, dclover1->nrj);
            dd->Fill(dclover1->nrj, dclover0->nrj);
            
            // -- Prompt veto -- //
            if (PM == 0)
            {
              dd_pveto->Fill(dclover0->nrj, dclover1->nrj);
              dd_pveto->Fill(dclover1->nrj, dclover0->nrj);
            }
            else
            {
              dd_PM1->Fill(dclover0->nrj, dclover1->nrj);
              dd_PM1->Fill(dclover1->nrj, dclover0->nrj);
            }
          }

          // -- Delayed VS PARIS -- //
          for (auto const & module1 : parisModules) d_tParis->Fill(dclover0->nrj, module1->time);
        }
      
        if (PC < 3_MeV && DC < 3_MeV)
        {        
          // -- Prompt -- //
          for (size_t loop_i = 0; loop_i<pclovers.clean.size(); ++loop_i)
          {
            auto const & pclover0 = pclovers.clean[loop_i];

            // -- Prompt-prompt -- //
            for (size_t loop_j = loop_i+1; loop_j<pclovers.clean.size(); ++loop_j)
            {
              auto const & pclover1 = pclovers.clean[loop_j];
              pp_PC3DC3->Fill(pclover0->nrj, pclover1->nrj);
              pp_PC3DC3->Fill(pclover1->nrj, pclover0->nrj);
            }

            // -- Prompt-delayed -- //
            for (auto dclover : dclovers.clean)
            {
              dp_PC3DC3->Fill(pclover0->nrj, dclover->nrj);
            }
          }

          // -- Delayed -- //
          for (size_t loop_i = 0; loop_i<dclovers.clean.size(); ++loop_i)
        {
          auto const & dclover0 = dclovers.clean[loop_i];

          // -- Delayed-delayed -- //
          for (size_t loop_j = loop_i+1; loop_j<dclovers.clean.size(); ++loop_j)
          {
            auto const & dclover1 = dclovers.clean[loop_j];
            if (Colib::abs(dclover0->time - dclover1->time) > coincTw) continue;
            dd_PC3DC3->Fill(dclover0->nrj, dclover1->nrj);
            dd_PC3DC3->Fill(dclover1->nrj, dclover0->nrj);
            
            // -- Prompt veto -- //
            if (PM == 0)
            {
              dd_pveto_PC3DC3->Fill(dclover0->nrj, dclover1->nrj);
              dd_pveto_PC3DC3->Fill(dclover1->nrj, dclover0->nrj);
            }
            else
            {
              dd_PM1_PC3DC3->Fill(dclover0->nrj, dclover1->nrj);
              dd_PM1_PC3DC3->Fill(dclover1->nrj, dclover0->nrj);
            }
          }

          // -- Delayed VS PARIS -- //
          for (auto const & module1 : parisModules) d_tParis->Fill(dclover0->nrj, module1->time);
        }
        }
      }

      // -- Write the data (monothread) -- //
      
      lock_mutex lock(write_mutex);
      
      print("writing spectra in", out_filename, "...");

      outfile->cd();


        pp -> Write("pp", TObject::kOverwrite);
        dd -> Write("dd", TObject::kOverwrite);
        dp -> Write("dp", TObject::kOverwrite);
        dd_pveto -> Write("dd_pveto", TObject::kOverwrite);
        dd_PM1 -> Write("dd_PM1", TObject::kOverwrite);
        
        pp_PC3DC3 -> Write("pp_PC3DC3", TObject::kOverwrite);
        dd_PC3DC3 -> Write("dd_PC3DC3", TObject::kOverwrite);
        dp_PC3DC3 -> Write("dp_PC3DC3", TObject::kOverwrite);
        dd_pveto_PC3DC3 -> Write("dd_pveto_PC3DC3", TObject::kOverwrite);
        dd_PM1_PC3DC3 -> Write("dd_PM1_PC3DC3", TObject::kOverwrite);
        
        d_tParis -> Write("d_tParis", TObject::kOverwrite);

      outfile->Close();
      delete outfile;

      infile->Close();
      delete infile;


      delete pp;
      delete dd;
      delete dp;
      delete dd_pveto;
      delete dd_PM1;

      delete d_tParis;

      print(out_filename, "written"); 
    }
  // Multithreading stops here. 
  });

  // -- Merging the files from each run -- //
  std::string dest = "data/merge_"+dataset+"_"+target;
#ifdef FILE_SUFFIXE
  dest+=FILE_SUFFIXE;
#endif //FILE_SUFFIXE  
  dest+="_v3.root";

  std::string source = "data/"+dataset+"/"+target;
#ifdef FILE_SUFFIXE
  dest+=FILE_SUFFIXE;
#endif //FILE_SUFFIXE  
  dest+="/run_*_v3.root";

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
// g++ -Og -g -o exec macro6.C ` root-config --cflags` `root-config --glibs` -DDEBUG -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro6.C ` root-config --cflags` `root-config --glibs` -std=c++17