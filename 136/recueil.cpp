#define MULTITHREAD 2

#ifdef MULTITHREAD
  #include "../lib/CoMT/CoMT.hpp"
#endif // MULTITHREAD

#include "../lib/Classes/Arguments.hpp"
#include "../lib/FasterReader/FasterRunReader.hpp"
#include "../lib/RootReader/RootReader.hpp"
#include "../lib/RootReader/TimeshiftCalculator.hpp"

using namespace Colib;
using namespace std;

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print("Usage :");
    return 0;
  }
  static string dataPath = "~/nuball2/N-SI-136/";
  static string outputPath = "~/nuball2/N-SI-136_root/";

  auto dTbinning = [](Label label) -> Time {
                 if (NSI136::isGe [label])   return   2_ns;
            else if (NSI136::isBGO[label])   return   1_ns;
            else if (label == 251)           return 100_ps;
            else if (label == 252)           return 100_ps;
            else if (NSI136::isParis[label]) return 100_ps;
            else if (NSI136::isDSSD[label])  return   2_ns;
            else                             return  10_ns;
  };

  Arguments args(argc, argv);
  while(args.next())
  {
    if (args == "--ts" || args == "--timeshifs")
    {
      auto sub_arg = args.load<string>();
      if (sub_arg == "all")
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
          if (run == "/home/faster/nuball2/N-SI-136/run_39.fast") continue;
          FasterRunReader reader;
          reader.addFiles(run+"/*");
          reader.setOutputPath(outputPath+"ref252/");
          reader.setMaxFilesMemory((getHome() == "/home/faster") ? 5 : 2);
          reader.setOverwrite(false);
          reader.setMerge(true);
          reader.setRef(252);

          reader.run();
          print();
        }

      #ifdef CoMT
        });
      #endif // CoMT

        for (auto const & dTfilename : findFilesWildcard(outputPath+"ref252/*"))
        {
          TimeshiftCalculator ts_cal(dTfilename);
          ts_cal.setBins<NSI136::maxLabel>(dTbinning);
          ts_cal.calculate();
        }
      }
      else if (sub_arg == "-f")
      {
        TimeshiftCalculator ts_cal(args.load<string>());
        ts_cal.setBins<NSI136::maxLabel>(dTbinning);
        ts_cal.calculate();
      }
    }
    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }
    else if (args == "--run")
    {
      auto sub_arg = args.load<string>();
      vector<string> runs(findFilesWildcard(dataPath+"run*.fast"));
      if (sub_arg == "U")
      {
        vector<string> real_runs;
        // for (auto const & run: runs) if ()
      }
    }
    else if (args == "--comp" || args == "--compare")
    {
      try
      {
        auto file_1 = args.load<string>();
        print(file_1);
        auto Tfile1 = TFile::Open(file_1.c_str(), "READ");
        if (!Tfile1) Colib::throw_error(file_1+" not found !!");
        auto tree1 = Tfile1->Get<TTree>("Nuball2");
        if (!tree1) Colib::throw_error(file_1+" has no Nuball2 tree !!");
        auto N1 = tree1->GetEntries();
        RootEvent event_1; event_1.reading(tree1);
  
        auto file_2 = args.load<string>();
        print(file_2);
        auto Tfile2 = TFile::Open(file_2.c_str(), "READ");
        if (!Tfile2) Colib::throw_error(file_2+" not found !!");
        auto tree2 = Tfile2->Get<TTree>("Nuball2");
        if (!tree2) Colib::throw_error(file_2+" has no Nuball2 tree !!");
        auto N2 = tree2->GetEntries();
        RootEvent event_2; event_2.reading(tree2);
  
        if (N1 != N2) Colib::throw_error(concatenate("1st file", to_string(N1), "events and 2nd file,", to_string(N1)));
  
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
        if (nbDiff == 0) print(Colib::Color::GREEN, "Files identical !!", Colib::Color::RESET);
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