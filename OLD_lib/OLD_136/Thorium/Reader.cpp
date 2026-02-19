// g++ -g -o reader Reader.cpp ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o reader Reader.cpp ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17
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
#include "CoefficientCorrection.hpp"
#include "Utils.h"

// Requires c++17, may have issues with c++11, contact me if it is the case
// Since I no longer will have access to my ijclab mail, here is my personal mail : corentin.hiver@free.fr
// Miscalleneous information : 
// There are two compilation lines at the beginning of the file. Use the first in case you want to debug with gdb, and the other if you want to run the code faster
// The arguments to run the code are ./reader [nb_files_to_read] [nb_hits_to_read(can be like 10e8)] [nb_threads(unused so far)]
// In CloversV2.hpp is a list of blacklisted detectors : CloversV2::blacklist

void Reader(int nb_files = -1, double nb_hits_read = -1, int nb_threads = 10)
{
  Timer timer;// A simple timer

  // Parameters :
  CloversV2::blacklist = {46, 55, 69, 70, 80, 92, 97, 122, 129, 142, 163}; // Blacklist the Ge crystal labels with poor behavior
  PhoswitchCalib calibPhoswitches("NaI_136_2024.angles"); // Rotation angles to calibrate Paris
  static constexpr Time bidimTimeWindow = 40_ns; // Suggested time coincidence time window

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/"); // To modify to match your repository
  FilesManager files(data_path.string(), nb_files); // Holds the list of the root files

  // Declare the histograms here

  std::string filename;// Variable to fill the full path and name of the file
  while(files.getNext(filename)) // Loop over the files
  {
    auto const & filename = removePath(file); // The file name without the path
    auto const & run_name = removeExtension(filename); // The file name without path or extension
    auto const & run_name_vector = split(run_name, '_'); // Working vector
    auto const & run_number_str = run_name_vector[1]; // Run number extracted from the file name, string
    auto const & run_number = std::stoi(run_number_str); // Run number integer

    Nuball2Tree tree(file); // Reads the root tree
    if (!tree) {error (file, "has no Nuball2 tree !!"); continue;}// If no root tree continue

    Event event; // Instanciate the event reader
    event.reading(tree, "mltTEQ"); // Link the event to the tree

    CloversV2 pclovers; // Prompt clovers
    CloversV2 dclovers; // Delayed clovers

    SimpleParis pparis(&calibPhoswitches); // Prompt paris
    SimpleParis dparis(&calibPhoswitches); // Delayed paris
    
    WarsawDSSD dssd; // DSSD (prompt because there is no delayed particles)

    for ( ;(evt_i < tree->GetEntries() && evt_i < nb_hits_read); evt_i++)
    {
      tree->GetEntry(evt_i); // Load the Event object with the next event
      
      // Clear all detectors :
      pclovers.clear();
      dclovers.clear();
      pparis.clear();
      dparis.clear();
      dssd.clear();

      // Loop over each raw hit and fill the detectors :
      for (int hit_i = 0; hit_i < event.mult; hit_i++)
      {
        auto const & label = event.labels[hit_i]; // Reading-only label variable
        auto const & time =  event.times[hit_i]; // Reading-only time variable
        auto const & nrj = event.nrjs[hit_i]; // Reading-only nrj variable

        if (nrj<20_keV) continue;

        // if (label == 252) // This label is the FATIMA LaBr3 used for time reference

        if (Paris::is[label])
        {
          if (gate(-5_ns, time, 5_ns)) // Prompt time gate
          {
            // Here you can apply calibration
            pparis.fill(event, hit_i);
          }
          else if (gate(40_ns, time, 170_ns)) // Delayed time gate
          {
            // Here you can apply calibration
            dparis.fill(event, hit_i);
          }
        }

        else if (CloversV2::isClover(label)) // note : this line is susceptible to change in a later version of the code
        {
          if (gate(-20_ns, time, 20_ns) )
          {
            // Here you can apply calibration
            pclovers.fill(event, hit_i);
          }
          else if (gate(40_ns, time, 170_ns) )
          {
            dclovers.fill(event, hit_i);
          }
        }

        else if (DSSD::is[label]) dssd.fill(event, hit_i);
      }

      // Perform detector analysis techniques : add-back, compton rejection...
      pclovers.analyze();
      dclovers.analyze();
      pparis.analyze();
      dparis.analyze();
      dssd.analyze();

      // -- Multiplicity -- //

      auto const & pcloverM = pclovers.all.size(); // Prompt clover multiplicity
      auto const & pparisM = pparis.module_mult(); // Prompt paris multiplicity
      auto const & PM = pcloverM + pparisM; // Prompt multiplicity

      auto const & dcloverM = dclovers.all.size(); // Delayed clover multiplicity
      auto const & dparisM = dparis.module_mult(); // Delayed paris multiplicity
      auto const & DM_raw = dcloverM + dparisM; // Delayed multiplicity

      // -- Calorimetry -- //

      auto const & PC = pclovers.calorimetryTotal + pparis.calorimetry(); // Prompt calorimetry
      auto const & DC = dclovers.calorimetryTotal + dparis.calorimetry(); // Delayed calorimetry

      // -- DSSD -- //
      bool dssd_trigger = dssd.mult() > 0; // Enough for a simple dssd trigger
      bool proton_trigger = false; // I did not study this trigger in details, contact me if you want to investigate it
      for (auto sector : dssd.sectors()) if (sector->nrj < 8_MeV && gate(-10_ns,sector->time, 10_ns)) proton_trigger = true;
    
      // -- Prompt clean clovers analysis -- //      
      for (size_t loop_i = 0; loop_i<pclovers.clean.size(); ++loop_i)
      {
        auto const & clover_i = pclovers.clean[loop_i];
        // I use read-only variables here for you to know how to use the class, but you can use for instance clover_i.nrj if you prefer
        auto const & index_i = clover_i.index();
        auto const & nrj_i = clover_i.nrj;
        auto const & time_i = clover_i.time;
        // p->Fill(nrj_i);
        
        // -- Germanium gamma-gamma -- //
        for (size_t loop_j = loop_i+1; loop_j<pclovers.clean.size(); ++loop_j)
        {
          // pp->Fill(clover_i.nrj, clover_j.nrj);
          // pp->Fill(clover_j.nrj, clover_i.nrj);
          if (dssd_trigger && PC < 5_MeV) // For example
          {
            // pp_p->Fill(clover_i.nrj, clover_j.nrj);
            // pp_p->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        // -- Paris-Germanium gamma-gamma -- //
        for (auto const module : pparis.modules) 
        {
          auto const & pparis_nrj = module->nrj;
          auto const & pparis_time = module->time;
          // p_VS_pparis->Fill(module->nrj, clover_i.nrj);
        }
      }

      // -- Delayed clean clovers analysis -- //      
      for (size_t loop_i = 0; loop_i<dclovers.clean.size(); ++loop_i)
      {
        auto const & clover_i = dclovers.clean[loop_i];
        // I use read-only variables here for you to know how to use the class, but you can use for instance clover_i.nrj if you prefer
        auto const & index_i = clover_i.index();
        auto const & nrj_i = clover_i.nrj;
        auto const & time_i = clover_i.time;
        // d->Fill(nrj_i);
        
        // -- Germanium gamma-gamma -- //
        for (size_t loop_j = loop_i+1; loop_j<dclovers.clean.size(); ++loop_j)
        {
          // dd->Fill(clover_i.nrj, clover_j.nrj);
          // dd->Fill(clover_j.nrj, clover_i.nrj);
          if (dssd_trigger && PC < 5_MeV) // For example
          {
            // dd_p->Fill(clover_i.nrj, clover_j.nrj);
            // dd_p->Fill(clover_j.nrj, clover_i.nrj);
          }

          if (PM == 0) // The prompt-veto technique : the prompt-vetoed matrix is created this way
          {
            // dd_prompt_veto->Fill(clover_i.nrj, clover_j.nrj);
            // dd_prompt_veto->Fill(clover_j.nrj, clover_i.nrj);
          }
        }

        // -- Paris-Germanium gamma-gamma -- //
        for (auto const module : pparis.modules) 
        {
          // p_VS_pparis->Fill(module->nrj, clover_i.nrj);
        }
      }
    }
  }
  // Write the histograms here
  print("analysis time :", timer);
}

int main(int argc, char** argv)
{
       if (argc == 1) macro5();
  else if (argc == 2) macro5(std::stoi(argv[1]));
  else if (argc == 3) macro5(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macro5(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}