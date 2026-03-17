#define MULTITHREAD 2

#ifdef MULTITHREAD
  #include "../lib/CoMT/CoMT.hpp"
#endif // MULTITHREAD

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

constexpr array<Label, LUT_s> timeBlacklist = {70, 97, 800, 801, 840, 841, 849};
constexpr auto timeBlacklist_LUT = LUT<LUT_s>([&timeBlacklist](Label label) -> bool {return found(timeBlacklist, label);});
constexpr array<Label, LUT_s> spectroBlacklist = {70, 80, 92, 122, 129, 163};
constexpr auto spectroBlacklist_LUT = LUT<LUT_s>([&spectroBlacklist](Label label) -> bool {return found(spectroBlacklist, label);});

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print("Usage :");
    return 0;
  }
  static string data = "~/nuball2/N-SI-136/";
  static string output = "~/nuball2/N-SI-136_root/";
  auto const dataPath = getPath(data); 
  auto const outputPath = getPath(output); 
  auto tsPath = outputPath+"timeshifts/";
  auto const calibPath = outputPath+"calib/";

  auto dTbinning = [](Label label) -> Time {
                 if (isGe [label])   return   2_ns;
            else if (isBGO[label])   return   1_ns;
            else if (label == 251)           return 100_ps;
            else if (label == 252)           return 100_ps;
            else if (isParis[label]) return 100_ps;
            else if (isDSSD[label])  return   2_ns;
            else                             return  10_ns;
  };

  Arguments args(argc, argv);
  while(args.next())
  {
    if (args == "--ts" || args == "--timeshifs")
    {
      auto sub_arg = args.load<string>();
      if (sub_arg == "make")
      {
        print("Creating time-reference-gated data");
        vector<string> runs(findFilesWildcard(dataPath+"run*.fast"));
      #ifdef CoMT
        MT::Initialise(min(static_cast<int>(runs.size()), MULTITHREAD));
        auto const dispatchesd_runs = MT::distribute(runs);
        MT::parallelise_function([&](){
        for (auto const & run : dispatchesd_runs[MT::getThreadIndex()])
      #else // NO CoMT
        for (auto const & run : runs)
      #endif // CoMT
        {
          FasterRunReader reader;
          reader.addFiles(run+"/*");
          reader.setOutputPath(outputPath+"ref252/");
          reader.setMaxFilesMemory((getHome() == "/home/faster") ? 5 : 2);
          reader.setOverwrite(false);
          reader.setMerge(true);
          reader.setRef(252);

          reader.run();
        }

      #ifdef CoMT
        });
      #endif // CoMT
      }
      else if (sub_arg == "file")
      {
        calculateTimeshifts(args.load<string>(), tsPath);
      }
      else if (sub_arg == "calc")
      {
        for (auto const & gatedRootFile : findFilesWildcard(outputPath+"ref252/run*_ref_252.root"))
          calculateTimeshifts(gatedRootFile, tsPath);
      }
      else if (sub_arg == "check")
      {
        print("Testing timeshifts");
        for (auto const & gatedRootFile : findFilesWildcard(outputPath+"ref252/*_ref_252.root"))
        {
          string tsFilename = tsPath+removePath(setExtension(removeAll(gatedRootFile, "_ref_252"), "dT"));
          Timeshifts ts; ts.load(tsFilename);
          auto outputPathCorr = getPath(outputPath+"tcorr/");
          string outputCorr = outputPathCorr + removePath(gatedRootFile);
          RootInterface interface;
          interface.openRootFile(outputCorr);
          interface.initializeTree("Nuball2", "Nuball2_Events_ts_corr");
          RootReader reader(gatedRootFile);
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
    }
    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }

    else if (args == "--run")
    {
      enum Trigger{dC1, C2}; // The default trigger is placed at the end so that it is used if the user input is erroneous.
      vector<string> trig_names = {"dC1", "C2"}; 
      int trigger = C2;

      while(args.next())
      {
             if (args == "-t"  || args == "--trigger") trigger = findIndex(trig_names, args.load<string>());
        else if (args == "-ts" || args == "--timeshifts") tsPath = args.load<string>();
        // else if (args == "-c" || args == "--calibration") calibPath = args.load<string>();
        else if (args == "-h"  || args == "--help:")
        {
          print("--run : convert the pulsed runs from .fast to .root with calibration, time synchonization, event building, data reduction...");
          print(" -t  || --trigger: choose which data reduction trigger to use between dC1, C2. Default: C2.");
          print(" -ts || --timeshifts: Path to the timeshift files to use. Default:", tsPath);
          // print(" -c  || --calibration: Calibration file to use. Default:", tsPath);
          print(" -h  || --help: display this help");
        }
      }

      vector<string> runs(findFilesWildcard(dataPath+"run*.fast"));
    #ifdef CoMT
      MT::Initialise(min(int_cast(runs.size()), MULTITHREAD));
      auto const dispatchesd_runs = MT::distribute(runs);
      MT::parallelise_function([&](){
      for (auto const & run : dispatchesd_runs[MT::getThreadIndex()])
    #else // NO CoMT
      for (auto const & run : runs)
    #endif // CoMT
      {
        auto run_name = runName(run);
        string tsFilename = tsPath+setExtension(run_name, "dT");
        string calibFilename = calibPath+"temp_136.calib";

        auto outputPathConversion = getPath(outputPath+"_"+trig_names[trigger]+"/");

        FasterRunReader reader;
        reader.addFiles(run+"/*");
        reader.setOutputPath(outputPathConversion);
        reader.setMaxFilesMemory((getHome() == "/home/faster") ? 5 : 2);
        reader.setOverwrite(false);
        reader.setMerge(true);
        reader.setRF(251);
        reader.setTimeShifts(tsFilename);
        reader.setCalibration(calibFilename);
        
        switch(trigger)
        {
          case (dC1): reader.setEventTrigger(dC1trigger); break;
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
    else throw_error(args.getArg() + " unkown !!");
  }
  return 1;
}

void calculateTimeshifts(string filename, string outputPath)
{
  TimeshiftCalculator ts_cal(filename);
  string tsFilename = removePath(setExtension(removeAll(filename, "_ref_252"), "dT"));
  ts_cal.setSimpleMax(251);
  for (auto const & label : labelsDSSD) ts_cal.setRaisingEdge(label);
  ts_cal.makeHisto(252, true, outputPath, tsFilename);
}

constexpr bool C2trigger(Event const & event)
{
  if (10 < event.mult) return false;

  bitset<24> Ge;
  bitset<24> BGO;

  if (timeBlacklist_LUT[label] || spectroBlacklist_LUT[label]) continue;

  for (int hit_i = 0; hit_i<event.mult; ++hit_i)
  {
    auto const & label = event.labels[hit_i];
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
    if (40<time && time<180)
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