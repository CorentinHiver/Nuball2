#define MULTITHREAD 2

#ifdef MULTITHREAD
  #include "../lib/Classes/CoMT.hpp"
#endif // MULTITHREAD

#include "../lib/Classes/MThisto.hpp"

#include "../lib/Classes/Arguments.hpp"
#include "../lib/FasterReader/FasterRunReader.hpp"
#include "../lib/RootReader/RootReader.hpp"
#include "../lib/RootReader/TimeshiftCalculator.hpp"

using namespace Colib;
using namespace NSI136;
using namespace std;

void calculateTimeshifts(string filename, string outputPath);
constexpr bool C2trigger(Event const & event);
constexpr bool dC1trigger(Event const & event);
constexpr bool ptrigger(Event const & event);

constexpr array<Label, LUT_s> timeBlacklist = {70, 97, 800, 801, 840, 841, 849};
constexpr auto timeBlacklist_LUT = LUT<LUT_s>([](Label label) -> bool {return found(timeBlacklist, label);});
constexpr array<Label, LUT_s> spectroBlacklist = {70, 80, 92, 122, 129, 163};
constexpr auto spectroBlacklist_LUT = LUT<LUT_s>([](Label label) -> bool {return found(spectroBlacklist, label);});

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print("Usage :");
    print("First: : Parameters");
    print("-m || --multithreading  [nbThreads] : if compiled in MT mode, sets the number of threads");
    return 0;
  }
  static string data = "~/nuball2/N-SI-136/";
  static string output = "~/nuball2/N-SI-136_root/";
  auto const dataPath = nicerPath(data); 
  auto const outputPath = nicerPath(output); 
  auto tsPath = outputPath+"timeshifts/";
  auto const calibPath = outputPath+"calib/";
  vector<string> runs(findFilesWildcard(dataPath+"run*.fast/"));
#ifdef CoMT
  MT::Initialise(MULTITHREAD);
  auto dispatched_runs = MT::distribute(runs);
#endif // CoMT

  enum Trigger{dC1, p, C2}; // The default trigger is placed at the end so that it is used if the user input is erroneous.
  constexpr vector<string> trig_names = {"dC1", "p", "C2"};
  vector<string> trig_outputs = [&](){vector<string> out; for (auto const & trig : trig_names) out.push_back(outputPath+"run_"+trig+"/");}
  int trigger = C2;


  Arguments args(argc, argv);
  while(args.next())
  {
    ////////////////////////////
    // Time shift calculation //
    ////////////////////////////
    if (args == "-m" || args == "--multithreading")
    {
    #ifdef CoMT
      MT::Initialise(args.load<int>());
    #endif // CoMT
    }

    else if (args == "--ts" || args == "--timeshifs")
    {
      auto sub_arg = args.load<string>();

      // -- First, extract the reference-gated events -- //

      if (sub_arg == "make" || sub_arg == "all")
      {
        print("Creating time-reference-gated data");
      #ifdef CoMT
        MT::parallelise_function([&](){
        for (auto const & run : dispatched_runs[MT::getThreadIndex()])
      #else // NO CoMT
        for (auto const & run : runs)
      #endif // CoMT
        {
          FasterRunReader reader;
          reader.addFiles(run+"*");
          reader.setOutputPath(outputPath+"ref252/");
          reader.setHitsBufferSize(500_ki);
          // reader.setMaxFilesMemory((getHome() == "/home/faster") ? 5 : 2);
          reader.setOverwrite(false);
          reader.setMerge(true);
          reader.setRef(252);

          reader.run();
        }

      #ifdef CoMT
        });
      #endif // CoMT
      }

      // -- Then, calculate the time difference -- //

      if (sub_arg == "calc" || sub_arg == "all")
      {
        vector<string> runsGated(findFilesWildcard(outputPath+"ref252/run*_ref_252.root"));
      #ifdef CoMT
        MT::parallelise_function([&](){
        for (auto const & run : dispatched_runs[MT::getThreadIndex()])
      #else // NO CoMT
        for (auto const & run : runsGated)
      #endif // CoMT

          calculateTimeshifts(run, tsPath);

      #ifdef CoMT
        });
      #endif // CoMT
      }

      // -- Check the result -- //

      if (sub_arg == "check" || sub_arg == "all")
      {
        print("Testing timeshifts");
        for (auto const & gatedRootFile : findFilesWildcard(outputPath+"ref252/*_ref_252.root"))
        {
          string tsFilename = tsPath+removePath(setExtension(removeAll(gatedRootFile, "_ref_252"), "dT"));
          Timeshifts ts; ts.load(tsFilename);
          auto outputPathCorr = nicerPath(outputPath+"tcorr/");
          string outputCorr = outputPathCorr + removePath(gatedRootFile);
          RootInterface interface;
          interface.openRootFile(outputCorr);
          interface.initializeTree("Nuball2", "Nuball2_Events_ts_corr");
          RootReader reader(gatedRootFile);
          reader.setMaxHits(1e6);
          reader.getEvent().writing(interface.getTree());
          while(reader.readNext())
          {
            reader.printLoadingPercents();
            auto & event = reader.getEvent();
            for (int i = 0; i<event.mult; ++i) event.times[i] += ts[event.labels[i]];
            interface.getTree()->Fill();
          }
          interface.writeTree();
          print(outputCorr, "written");
        }
      }
      
      // -- Extract the histograms from the result -- //
      
      if (sub_arg == "checkhisto" || sub_arg == "all")
      {
        auto filename = outputPath+"tcorr/ref252_dT.root";
        bool overwrite = true;
        for (auto const & corrRootFile : findFilesWildcard(outputPath+"tcorr/run*"))
        {
          auto histoName = "dT"+runName(removeAll(removeAll(corrRootFile, "_ref_252"), "tcorr"));
          auto bidim = new TH2F(histoName.c_str(), histoName.c_str(), 1000, 0, 1000, 2500, -1250000, 1250000);
          {
            RootReader reader(corrRootFile);
            auto tree = reader.getTree();
            tree->SetBranchStatus("*", false);
            for (auto const & branch : {"mult", "time", "label"}) tree->SetBranchStatus(branch, true);
            auto const & event = reader.getEvent();
            while(reader.readNext()) for (int hit_i = 0; hit_i<event.mult; ++hit_i)
              bidim->Fill(event.labels[hit_i], event.times[hit_i]);
            bidim->SetDirectory(nullptr);
          }
          auto outFile = TFile::Open(filename.c_str(), (overwrite) ? "recreate" : "update");
          overwrite = false;
          outFile->cd();
          bidim->Write();
          outFile->Close();
        }
        print(filename, "written");
      }

      // -- The results will be disapointing for some detectors because there isn't enough coincidence statistics -- //
      // -- Use the beam pulse that is now synchronized to extract events with good pulse timing. Downscaled ! -- //
      if (sub_arg == "checkpulse" || sub_arg == "all")
      {
        print("check synchronization with pulsation");
        bool overwrite = true;
      #ifdef CoMT
        MT::parallelise_function([&](){
        for (auto const & run : dispatched_runs[MT::getThreadIndex()])
      #else // NO CoMT
        for (auto const & run : runs)
      #endif // CoMT
        {        
          string tsFilename = tsPath+setExtension(runName(run), "dT");
          Timeshifts ts(tsFilename);

          // First, load a part of the data.
          FasterRootInterface reader;
          reader.setTotMaxHits(5e7);
          reader.loadTimeshifts(ts);
          while(reader.loadDatafiles(findFilesWildcard(run+"*"), 10));
          reader.timeSorting();

          string name = "refRFtime"+runName(run);

          // First step, check the RF timing
          auto refRF = new TH1F(name.c_str(), name.c_str(), 3000, -100_ns, 200_ns);
          RF_Manager rf;
          rf.label = 251;
          rf.setOffset(50_ns);
          bool b = rf.findFirst(reader.data(), reader.sortedIDs());
          if (!b) throw_error("FasterRootInterface::buildEventWithRF() :  no RF hit with label "+to_string(251));
          for (auto const & hit_i : reader.sortedIDs())
          {
            auto const & hit = reader.data()[hit_i];
            if (rf.setHit(hit)) continue;
            if (hit.label == 252) refRF->Fill(rf.relTime(hit));
          }
          auto max_T = abs(refRF->GetXaxis()->GetBinLowEdge(refRF->GetMaximumBin()));
          if (max_T < 2_ns) ts.set(252, max_T);
          {
#ifdef CoMT
            lock_mutex lock(MT::mutex);
#endif // CoMT
            auto file = TFile::Open("checkpulse_dT.root", (overwrite) ? "recreate" : "update");
            refRF->Write();
            file->Close();
            overwrite = false;
          }
          auto refRFall = new TH2F((name+"_all").c_str(), (name+"_all").c_str(), 1000,0,1000, 3000, -100_ns, 200_ns);
          rf.findFirst(reader.data(), reader.sortedIDs());
          for (auto const & hit_i : reader.sortedIDs())
          {
            auto const & hit = reader.data()[hit_i];
            if (rf.setHit(hit)) continue;
            refRFall->Fill(hit.label, rf.relTime(hit));
          }
          {
#ifdef CoMT
            lock_mutex lock(MT::mutex);
#endif // CoMT
            auto file = TFile::Open("checkpulse_dT.root", "update");
            refRFall->Write();
            file->Close();
            overwrite = false;
          }
        }
        
      #ifdef CoMT
        });
      #endif // CoMT
      }
      
      else if (sub_arg == "file")
      {
        calculateTimeshifts(args.load<string>(), tsPath);
      }
    }

    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }

    else if (args == "--spectra")
    {
      auto sub_arg = args.load<string>();
      if (sub_arg == "runs")
      {
      #ifdef CoMT
        MT::parallelise_function([&](){
        for (auto const & run : dispatched_runs[MT::getThreadIndex()])
      #else // NO CoMT
        for (auto const & run : runs)
      #endif // CoMT
        {
          auto run_name = runName(run);
          string name = "histo"+runName(run);
          Calibration calib(calibPath+"temp_136.calib");
          auto adc   = new TH2F((name+"_adc").c_str(), (name+"_adc").c_str(), 1000,0,1000, 3000, -100_ns, 200_ns);
          auto nrj   = new TH2F((name+"_nrj").c_str(), (name+"_nrj").c_str(), 1000,0,1000, 3000, -100_ns, 200_ns);
          for (auto const & filename : findFilesWildcard(run+"*"))
          {
            FasterRootInterface reader(filename);
            auto & hit = reader.getHit();
            while(reader.loadNextRootHit())
            {
              adc->Fill(hit.label, hit.adc);
              calib.calibrate(hit);
              nrj->Fill(hit.label, hit.nrj);
            }
          }
          auto outFile = TFile::Open(name.c_str(), "recreate");
          adc -> Write();
          nrj -> Write();
          outFile -> Close();
        }
      #ifdef CoMT
        });
      #endif // CoMT
      }
      else if (sub_arg == "run")
      {
        auto files = findFilesWildcard(nicerPath(args.load<string>())+"*");

        MT::Histo<TH2F> allSpectra   ("allSpectra"  , "allSpectra;label;nrj[keV]"       , 1000,0,1000, 10_ki,0,10_MeV);
        MT::Histo<TH2F> GeSpectra    ("GeSpectra"   , "GeSpectra;GeLabel;nrj[keV]"      , 100 ,0, 100, 10_ki,0,10_MeV);
        MT::Histo<TH2F> BGOSpectra   ("BGOSpectra"  , "BGOSpectra;BGOLabel;nrj[keV]"    , 30  ,0,  30,  1_ki,0,10_MeV);
        MT::Histo<TH2F> ParisSpectra ("ParisSpectra", "ParisSpectra;ParisLabel;nrj[keV]", 100 ,0, 100,  2_ki,0,10_MeV);
        MT::Histo<TH2F> DSSDSpectra  ("DSSDSpectra" , "DSSDSpectra;DSSDLabel;nrj[keV]"  , 50  ,0,  50,  1_ki,0,30_MeV);

        auto calibFile = calibPath+"temp_136.calib";
        if (0 < args.nbRemainingArgs()) while(args.next())
        {
          if (args == "-c") calibFile = args.load<string>();
          else throw_error("in --spectra run,"+args.getArg()+"argument is not known");
        }
        Calibration calib(calibFile);
      #ifdef CoMT
        auto dispatchesd_files = MT::distribute(files);
      MT::parallelise_function([&](){
        auto thread_str = std::to_string(MT::getThreadIndex());
        auto files_to_read = dispatchesd_files[MT::getThreadIndex()];
      #else //no CoMT
        auto files_to_read = files;
        std::string thread_str = "";
      #endif // CoMT
        size_t cursor = 0;
        for (auto const & file : files_to_read)
        {
          printsln(thread_str, getFilename(file));
          ++cursor;
          FasterRootInterface reader(file);
          while(reader.loadNextHit())
          {
            auto & hit = reader.getHit();
          #ifdef CoMT
            if (MT::isKilled()) break;
          #endif //CoMT
            // if (cursor%int(1e6) == 0) printsln(getFilename(file), Colib::nicer_double(cursor, 2), Colib::nicer_seconds(hit.getTimestamp_s()));

            if (isGe[hit.label && hit.adc<NSI136::GeOverflow]) continue;
            calib.calibrate(hit);

            allSpectra->Fill(hit.label, hit.nrj);
            switch (detectorType[hit.label])
            {
              case Detector::Ge    : GeSpectra    -> Fill(GeIndex   [hit.label], hit.nrj); break;
              case Detector::BGO   : BGOSpectra   -> Fill(BGOIndex  [hit.label], hit.nrj); break;
              case Detector::Paris : ParisSpectra -> Fill(ParisIndex[hit.label], hit.nrj); break;
              case Detector::DSSD  : DSSDSpectra  -> Fill(DSSDIndex [hit.label], hit.nrj); break;
              default: continue;
            }
          }
        }
      #ifdef CoMT
        });
      #endif // CoMT

        std::string name = "test.root";
        auto outFile = TFile::Open(name.c_str(), "recreate");
        allSpectra   -> Write();
        GeSpectra    -> Write();
        BGOSpectra   -> Write();
        ParisSpectra -> Write();
        DSSDSpectra  -> Write();
        outFile -> Close();
        print(name, "written");
      }
    }
    else if (args == "--run")
    {
      bool overwrite{false};
      while(args.next())
      {
             if (args == "-t"  || args == "--trigger") trigger = findIndex(trig_names, args.load<string>());
        
        // else if (args == "-ts" || args == "--timeshifts") tsPath = args.load<string>();
        // else if (args == "-c" || args == "--calibration") calibPath = args.load<string>();
        else if (args == "-h"  || args == "--help:")
        {
          print("--run : convert the pulsed runs from .fast to .root with calibration, time synchonization, event building, data reduction...");
          print(" -t  || --trigger: choose which data reduction trigger to use between dC1, C2, p (1 delayed Clean Ge, 2 clean Ge, particle). Default: C2.");
          // print(" -ts || --timeshifts: Path to the timeshift files to use. Default:", tsPath);
          // print(" -c  || --calibration: Calibration file to use. Default:", tsPath);
          print(" -h  || --help: display this help");
          print(" -o || --overwrite : overwrite the output") overwrite = true;
        }
        else throw_error("in --spectra run,"+args.getArg()+"argument is not known");
      }
      auto runs_to_convert = runs;
      if (!overwrite) 
      {
        // Manually check for already calculated runs if we don't want to overwrite them
        // We also set FasterRunReader::setOverwrite for safety reason
        vector<string> rootruns(findFilesWildcard(trig_outputs[trigger]+"run*.root"));
        for (auto const & root_file : rootruns)
      }

    #ifdef CoMT
      dispatched_runs = 
      MT::parallelise_function([&](){
      for (auto const & run : dispatched_runs[MT::getThreadIndex()])
    #else // NO CoMT
      for (auto const & run : runs)
    #endif // CoMT
      {
        auto run_name = runName(run);
        printsln(run, run_name);
        if (runNumber(run_name) < 13) continue;
        string tsFilename = tsPath+setExtension(run_name, "dT");
        string calibFilename = calibPath+"temp_136.calib";

        auto outputPathConversion = trig_outputs[trigger];

        FasterRunReader reader;
        reader.addFiles(run+"/*");
        reader.setOutputPath(outputPathConversion);
        // reader.setMaxFilesMemory((getHome() == "/home/faster") ? 5 : 2);
        reader.setOverwrite(overwrite);
        reader.setMerge(true);
        reader.setRF(251);
        reader.setTimeShifts(tsFilename);
        reader.setCalibration(calibFilename);
        
        switch(trigger)
        {
          case (dC1): reader.setEventTrigger(dC1trigger); break;
          case (p  ): reader.setEventTrigger(ptrigger); break;
          case (C2 ): default:    
                      reader.setEventTrigger(C2trigger);
        }
        reader.run();
      }
    
    #ifdef CoMT
      });
    #endif // CoMT

    }
    else if (args == "--comp" || args == "--compare")
    {
      try
      {
        auto file_1 = args.load<string>();
        print(file_1);
        auto Tfile1 = TFile::Open(file_1.c_str(), "READ");
        if (!Tfile1) throw_error(file_1+" not found !!");
        auto tree1 = Tfile1->Get<TTree>("Nuball2");
        if (!tree1) throw_error(file_1+" has no Nuball2 tree !!");
        auto N1 = tree1->GetEntries();
        RootEvent event_1; event_1.reading(tree1);
  
        auto file_2 = args.load<string>();
        print(file_2);
        auto Tfile2 = TFile::Open(file_2.c_str(), "READ");
        if (!Tfile2) throw_error(file_2+" not found !!");
        auto tree2 = Tfile2->Get<TTree>("Nuball2");
        if (!tree2) throw_error(file_2+" has no Nuball2 tree !!");
        auto N2 = tree2->GetEntries();
        RootEvent event_2; event_2.reading(tree2);
  
        if (N1 != N2) throw_error(concatenate("1st file", to_string(N1), "events and 2nd file,", to_string(N1)));
  
        auto const N = N1;
        int nbDiff = 0;
        for (long n = 0; n<N; n++)
        {
          Tfile1->cd();
          tree1->GetEntry(n);
          Tfile2->cd();
          tree2->GetEntry(n);
          if (event_1 != event_2) 
          {
            error(event_1);
            error("different from");
            error(event_2);
            ++nbDiff;
          }
        }
        if (nbDiff == 0) print(Color::GREEN, "Files identical !!", Color::RESET);
      }
      catch(Arguments::MissingArg const & arg)
      {
        print(arg.message);
      }
    }
    else if (args == "analyse")
    {
      trigger = findIndex(trig_names, args.load<string>());
      vector<string> rootruns(findFilesWildcard(trig_outputs[trigger]+"run*.root"));
      if (sub_arg == "assess")
      {
#ifdef CoMT
          dispatched_rootruns = MT::distribute(rootruns)
          MT::parallelise_function([&](){
          for (auto const & run : dispatched_rootruns[MT::getThreadIndex()])
#else // NO CoMT
          for (auto const & run : dispatched_rootruns)
#endif // CoMT
          {
            string test = "test";
            MT::Histo<TH2F> pp(test+"pp", test+"pp", 4096,0,4096, 4096,0,4096);
            MT::Histo<TH2F> dd(test+"dd", test+"dd", 4096,0,4096, 4096,0,4096);
          }
#ifdef CoMT
          });
#endif // CoMT
      }

    }
    else throw_error(args.getArg() + " unkown !!");
  }
  return 1;
}

void calculateTimeshifts(string filename, string outputPath)
{
  TimeshiftCalculator ts_cal(filename);
  ts_cal.setOutputName(runName(filename)+"_dT.root");
  string tsFilename = removePath(setExtension(removeAll(filename, "_ref_252"), "dT"));
  ts_cal.setSimpleMax(251);
  vector<Label> weirdDSSD = {800, 801, 841, 849};
  for (auto const & label : labelsDSSD) 
  {
    if (found(weirdDSSD, label)) 
    {
      ts_cal.setSimpleMax(251);
      ts_cal.rebin(label, 200);
    }
    else 
    {
      ts_cal.setRaisingEdge(label);
      ts_cal.rebin(label, 20);
    }
  }
  for (auto const & label : labelsClover) ts_cal.rebin(label, 20);
  ts_cal.rebin(251, 20);
  ts_cal.makeHisto(252, true, outputPath, tsFilename);
}

constexpr bool ptrigger(Event const & event)
{
  if (10 < event.mult) return false;
  for (int hit_i = 0; hit_i<event.mult; ++hit_i) if (isDSSD[event.labels[hit_i]]) return true;
  return false;
}

constexpr bool C2trigger(Event const & event)
{
  if (10 < event.mult) return false;

  bitset<24> Ge;
  bitset<24> BGO;

  for (int hit_i = 0; hit_i<event.mult; ++hit_i)
  {
    auto const & label = event.labels[hit_i];
    if (timeBlacklist_LUT[label] || spectroBlacklist_LUT[label]) continue;

         if (isBGO[label]) BGO.set(cloverIndex[label]);
    else if (isGe [label]) Ge .set(cloverIndex[label]);
  }

  int nb_C = 0;

  for (int clover_i = 0; clover_i<24; ++clover_i)
    if (Ge.test(clover_i) && !BGO.test(clover_i)) ++nb_C;

  return 1 < nb_C;
}

constexpr bool dC1trigger(Event const & event)
{
  if (10 < event.mult) return false;

  bitset<24> dGe;
  bitset<24> dBGO;

  for (int hit_i = 0; hit_i<event.mult; ++hit_i)
  {
    auto const & label = event.labels[hit_i];
    if (timeBlacklist_LUT[label] || spectroBlacklist_LUT[label]) continue;

    auto const & time = event.times[hit_i];
    if (40_ns<time && time<180_ns)
    {
      if (isBGO[label]) dBGO.set(cloverIndex[label]);
      else if (isGe [label]) dGe .set(cloverIndex[label]);
    }
  }

  int nb_dC = 0;

  for (int clover_i = 0; clover_i<24; ++clover_i)
    if (dGe.test(clover_i) && !dBGO.test(clover_i)) ++nb_dC;

  return 0 < nb_dC;
}

// auto dTbinning = [](Label label) -> Time {
//                if (isGe [label])   return   2_ns;
//           else if (isBGO[label])   return   1_ns;
//           else if (label == 251)           return 100_ps;
//           else if (label == 252)           return 100_ps;
//           else if (isParis[label]) return 100_ps;
//           else if (isDSSD[label])  return   2_ns;
//           else                             return  10_ns;
// };


// else if (sub_arg == "pulsecalc")
// {
//   vector<string> runsGated(findFilesWildcard(outputPath+"ref252/run*_ref_252.root"));
// #ifdef CoMT
//   auto const dispatched_runs = MT::distribute(runsGated);
//   MT::parallelise_function([&](){
//   for (auto const & run : dispatched_runs[MT::getThreadIndex()])
// #else // NO CoMT
//   for (auto const & run : runsGated)
// #endif // CoMT

//     calculateTimeshifts(run, tsPath);

// #ifdef CoMT
//   });
// #endif // CoMT
// }
// else if (sub_arg == "pulsecheck")
// {
//   print("Testing timeshifts after pulse correction");
//   for (auto const & gatedRootFile : findFilesWildcard(outputPath+"dTpulse/*.root"))
//   {
//     string tsFilename = tsPath+removePath(setExtension(gatedRootFile, "dT"));
//     Timeshifts ts; ts.load(tsFilename);
//     auto outputPathCorr = nicerPath(outputPath+"tcorrpulse/");
//     string outputCorr = outputPathCorr + removePath(gatedRootFile);
//     RootInterface interface;
//     interface.openRootFile(outputCorr);
//     interface.initializeTree("Nuball2", "Nuball2_Events_ts_corr");
//     RootReader reader(gatedRootFile);
//     reader.getEvent().writing(interface.getTree());
//     while(reader.readNext())
//     {
//       reader.printLoadingPercents();
//       auto & event = reader.getEvent();
//       for (int i = 0; i<event.mult; ++i) event.times[i] += ts[event.labels[i]];
//       interface.getTree()->Fill();
//     }
//     interface.writeTree();
//     print(outputCorr, "written");
//   }
// }