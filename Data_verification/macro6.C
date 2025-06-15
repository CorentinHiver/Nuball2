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

///
/// V3 : use of alignement

#ifdef KISOMER
  #define MaxMult 10
  #define MaxMultDelayed 6
  #define MaxCaloDelayed 3_MeV
  #define MinMultDelayed 1
  #define ParticleTrigger
#endif // KISOMER

#ifdef SHAPEISOMER
  #define MaxMult 10
  #define MinMultDelayed 1
  #define MaxMultDelayed 5
  #define MinMultDelayed 2_MeV
#endif //SHAPEISOMER

constexpr Time coincTw = 40_ns;

void macro6(int nb_files = -1, long long nbEvtMax = -1, int nb_threads = 10)
{
  CloversV2::setBlacklist({46, 55, 64, 69, 70, 80, 92, 97, 122, 129, 142, 163});

  CloversV2::setOverflow( 
  {
    {25, 12600 }, {26, 13600 }, {27, 10500 }, {28, 7500  }, 
    {31, 11500 }, {32, 11400 }, {33, 8250  }, {34, 9000  }, 
    {37, 11000 }, {38, 11100 }, {39, 11500 }, {40, 11300 }, 
    {43, 12600 }, {44, 11900 }, {45, 11550 }, {46, 9200  }, 
    {49, 14300 }, {50, 12800 }, {51, 13500 }, {52, 12400 }, 
    {55, 5500  }, {56, 5500  }, 
                  {68, 7100  }, {69, 15500 }, {70, 9500  },
    {73, 11650 }, {74, 11600 }, {75, 11800 }, {76, 11600 }, 
    {79, 11500 }, {80, 8000  }, {81, 18200 },
    {85, 7700  }, {86, 12000 }, {87, 12000 }, {88, 11600 }, 
    {91, 7900  }, {92, 10000 }, {93, 11500 }, {94, 11000 }, 
    {97, 11400 }, {98, 11400 }, {99, 11250 }, {100, 8900 }, 
    {103, 11400 }, {104, 11600 }, {105, 11600 }, {106, 11500 }, 
    {109, 12800 }, {110, 1800  }, {111, 13000 }, {112, 11300 }, 
    {115, 12800 }, {116, 11500 }, {117, 10500 }, {118, 11400 }, 
    {121, 12400 }, {122, 20000 }, {123, 10700 }, {124, 20000 }, 
    {127, 11600 }, {128, 11700 }, {129, 10000 }, {130, 11200 }, 
    {133, 11200 }, {134, 9350  }, {135, 9400  }, {136, 9500  }, 
    {139, 13200 }, {140, 12400 }, {141, 12900 }, {142, 4500  }, 
    {145, 8200  }, {146, 9600  }, {147, 9100  }, {148, 10900 }, 
    {151, 11900 }, {152, 12200 }, {153, 11300 }, {154, 12000 }, 
    {157, 9110  }, {158, 9120  }, {159, 9110  }, {160, 11700 }, 
    {163, 11000 }, {164, 11600 }, {165, 11600 }, {166, 11600 }, 
  });

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
  
  // Get the calibration parameters :
    // Paris rotation angles :
  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  // Alignement
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

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
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

      // Nuball2Tree tree(file);
      // std::unique_ptr<TFile> infile; infile.reset(TFile::Open(file.c_str(), "READ"));

      TFile* infile = nullptr;
      TTree* tree = nullptr;

      {
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

      CloversV2 pclovers_before;
      CloversV2 dclovers_before;
  
      SimpleParis pparis_before(&calibPhoswitches);
      SimpleParis dparis_before(&calibPhoswitches);

      CloversV2 pclovers;
      CloversV2 dclovers;
  
      SimpleParis pparis(&calibPhoswitches);
      SimpleParis dparis(&calibPhoswitches);
  
      DSSD::Simple dssd;

      // -- Output file : -- //

      std::string outFolder = "data/"+trigger+"/"+target+"/";
      Path::make(outFolder);
      std::string out_filename = outFolder+removeExtension(filename)+"_v3.root";
      File Filename(out_filename); Filename.makePath();
      TFile* outfile = TFile::Open(out_filename.c_str(), "recreate");
      // std::unique_ptr<TFile> outfile (TFile::Open(out_filename.c_str(), "recreate"));
      
      TH1F* p_before = new TH1F(("p_before"+thread_i_str).c_str(), "gamma delayed before alignement;E1[keV];E2[keV]", 20000,0,20000);
      TH1F* p_after = new TH1F(("p_after"+thread_i_str).c_str(), "gamma delayed after alignement;E1[keV];E2[keV]", 20000,0,20000);
      
      TH1F* d_before = new TH1F(("d_before"+thread_i_str).c_str(), "gamma delayed before alignement;E1[keV];E2[keV]", 20000,0,20000);
      TH1F* d_after = new TH1F(("d_after"+thread_i_str).c_str(), "gamma delayed after alignement;E1[keV];E2[keV]", 20000,0,20000);
  
      TH2F* pp_before = new TH2F(("pp_before"+thread_i_str).c_str(), "gamma-gamma prompt before alignement;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_before = new TH2F(("dd_before"+thread_i_str).c_str(), "gamma-gamma delayed before alignement;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dp_before = new TH2F(("dp_before"+thread_i_str).c_str(), "gamma-gamma prompt-delayed before alignement;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_pveto_before = new TH2F(("dd_pveto_before"+thread_i_str).c_str(), "gamma-gamma delayed pveto before alignement;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      
      TH2F* pp = new TH2F(("pp"+thread_i_str).c_str(), "gamma-gamma prompt;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd = new TH2F(("dd"+thread_i_str).c_str(), "gamma-gamma delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dp = new TH2F(("dp"+thread_i_str).c_str(), "gamma-gamma prompt-delayed;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      TH2F* dd_pveto = new TH2F(("dd_pveto"+thread_i_str).c_str(), "gamma-gamma delayed pveto;E1[keV];E2[keV]", 4096,0,4096, 4096,0,4096);
      
      ////////////////////////
      // -- PROCESS DATA -- //
      ////////////////////////

      auto const Nevt = tree->GetEntries();
  
      for (int evt_i = 0; (evt_i < Nevt && evt_i < nbEvtMax); evt_i++)
      {
        if (evt_i>0 && evt_i%freqEvtDisplay == 0) print(nicer_double(evt_i, 0), "events");
  
        tree->GetEntry(evt_i);

        // -- Clear modules -- //
  
        pclovers_before.clear();
        dclovers_before.clear();
  
        pparis_before.clear();
        dparis_before.clear();

        pclovers.clear();
        dclovers.clear();
  
        pparis.clear();
        dparis.clear();
  
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
              pparis_before.fill(event, hit_i);
              if (!align(event, hit_i, run_number)) continue;
              pparis.fill(event, hit_i);
            }
            else if (gate( 40_ns, time, 180_ns)) 
            {
              dparis_before.fill(event, hit_i);
              if (!align(event, hit_i, run_number)) continue;
              dparis.fill(event, hit_i);
            }
          }
          else if (CloversV2::is[label])
          {
                if (gate(-10_ns, time,  10_ns)) 
            {
              pclovers_before.fill(event, hit_i);
              if (!align(event, hit_i, run_number)) continue;
              pclovers.fill(event, hit_i);
            }

            else if (gate( 40_ns, time, 180_ns)) 
            {
              dclovers_before.fill(event, hit_i);
              if (!align(event, hit_i, run_number)) continue;
              dclovers.fill(event, hit_i);
            }

          }
          else if (DSSD::is[label]) dssd.fill(event, hit_i);
        }

        // -- Analyse modules -- //
  
        pclovers_before.analyze();
        dclovers_before.analyze();
        pparis_before.analyze();
        dparis_before.analyze();

        pclovers.analyze();
        dclovers.analyze();
        pparis.analyze();
        dparis.analyze();
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
      #ifdef MinMultPrompt
        if (PM < MinMultPrompt) continue;
      #endif //MinMultPrompt

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
      #ifdef MinCaloPrompt
        if (PC < MinCaloPrompt) continue;
      #endif //MinCaloPrompt

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
        if (DC > MaxCalo) continue;
      #endif //MaxCalo
      #ifdef MinCalo
        if (DC < MinCalo) continue;
      #endif //MinCalo

        /////////////////////
        // -- PARTICLES -- //
        /////////////////////

        bool dssdTrigger = dssd.mult() > 0;

      #ifdef ParticleTrigger
        if (!dssdTrigger) continue;
      #endif //ParticleTrigger

        //////////////////////////////////////
        // -- ANALYSIS BEFORE ALIGNEMENT -- //
        //////////////////////////////////////

        // -- Prompt -- //
        for (size_t loop_i = 0; loop_i<pclovers_before.clean.size(); ++loop_i)
        {
          auto const & pclover0 = pclovers_before.clean[loop_i];
          p_before->Fill(pclover0->nrj);

          // -- Prompt-prompt -- //
          for (size_t loop_j = loop_i+1; loop_j<pclovers_before.clean.size(); ++loop_j)
          {
            auto const & pclover1 = pclovers_before.clean[loop_j];
            pp_before->Fill(pclover0->nrj, pclover1->nrj);
            pp_before->Fill(pclover1->nrj, pclover0->nrj);
          }

          // -- Prompt-delayed -- //
          for (auto dclover : dclovers_before.clean)
          {
            dp_before->Fill(pclover0->nrj, dclover->nrj);
          }
        }

        // -- Delayed -- //
        for (size_t loop_i = 0; loop_i<dclovers_before.clean.size(); ++loop_i)
        {
          auto const & dclover0 = dclovers_before.clean[loop_i];
          d_before->Fill(dclover0->nrj);
          
          // -- Delayed-delayed -- //
          for (size_t loop_j = loop_i+1; loop_j<dclovers_before.clean.size(); ++loop_j)
          {
            auto const & dclover1 = dclovers_before.clean[loop_j];
            if (Colib::abs(dclover0->time - dclover1->time) > coincTw) continue;
            dd_before->Fill(dclover0->nrj, dclover1->nrj);
            dd_before->Fill(dclover1->nrj, dclover0->nrj);
            
            // -- Prompt veto -- //
            if (PM == 0)
            {
              dd_pveto_before->Fill(dclover0->nrj, dclover1->nrj);
              dd_pveto_before->Fill(dclover1->nrj, dclover0->nrj);
            }
          }
        }

        /////////////////////
        // -- ANALYSIS -- //
        /////////////////////

        // -- Prompt -- //
        for (size_t loop_i = 0; loop_i<pclovers.clean.size(); ++loop_i)
        {
          auto const & pclover0 = pclovers.clean[loop_i];
          p_after->Fill(pclover0->nrj);
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
          d_after->Fill(dclover0->nrj);
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
          }
        }
      }

      // -- Write the data (monothread) -- //
      
      lock_mutex lock(write_mutex);
      
      print("writing spectra in", out_filename, "...");

      outfile->cd();
        p_before -> Write("p_before", TObject::kOverwrite);
        d_before -> Write("d_before", TObject::kOverwrite);
        pp_before -> Write("pp_before", TObject::kOverwrite);
        dd_before -> Write("dd_before", TObject::kOverwrite);
        dp_before -> Write("dp_before", TObject::kOverwrite);
        dd_pveto_before -> Write("dd_pveto_before", TObject::kOverwrite);

        p_after -> Write("p_after", TObject::kOverwrite);
        d_after -> Write("d_after", TObject::kOverwrite);
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
  std::string dest = "data/merge_"+trigger+"_"+target+"_v3.root";
  std::string source = "data/"+trigger+"/"+target+"/run_*_v3.root";
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