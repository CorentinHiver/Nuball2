#define MULTITHREAD 4

#ifdef MULTITHREAD
  #include "../CoMT/CoMT.hpp"
#endif //MULTITHREAD

#include "../lib/Classes/Arguments.hpp"
#include "../lib/FasterReader/FasterRunReader.hpp"
#include "../lib/RootReader/RootReader.hpp"
#include "../lib/RootReader/TimeshiftCalculator.hpp"

using namespace Colib;

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print("Usage :");
    return 0;
  }
  static std::string dataPath = "~/nuball2/N-SI-136/";
  static std::string outputPath = "~/nuball2/N-SI-136_root/";
  constexpr static std::pair<int, int> runsU = {75, 123};
  constexpr static std::pair<int, int> runsTh = {13, 74};

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
      auto sub_arg = args.load<std::string>();
      if (sub_arg == "all")
      {
        std::vector<std::string> runs(findFilesWildcard(dataPath+"run*.fast"));
      #ifdef MULTITHREAD
      MT::Initialise(std::min(static_cast<int>(runs.size()), MULTITHREAD));
        auto const dispatchesd_runs = MT::distribute(runs);
        MT::parallelise_function([&](){
        for (auto const & run : dispatchesd_runs[MT::getThreadIndex()])
      #else // NO MULTITHREAD
        for (auto const & run : runs)
      #endif // MULTITHREAD
        {
          print("Creating time-reference-gated data for", run);
          FasterRunReader reader;
          reader.addFiles(run+"/*");
          reader.setOutputPath(outputPath+"ref252/");
          reader.setMaxFilesMemory((getHome() == "/home/corentin") ? 2 : 5);
          reader.setOverwrite(false);
          reader.setMerge(true);
          reader.setRef(252);
          // reader.setMaxHits(10000);

          reader.run();

          print();
        }

      #ifdef MULTITHREAD
        });
      #endif // MULTITHREAD

        for (auto const & dTfilename : findFilesWildcard(outputPath+"ref252/*"))
        {
          TimeshiftCalculator ts_cal(dTfilename);
          ts_cal.setBins<NSI136::maxLabel>(dTbinning);
          ts_cal.calculate();
        }
      }
      else if (sub_arg == "-f")
      {
        TimeshiftCalculator ts_cal(args.load<std::string>());
        ts_cal.setBins<NSI136::maxLabel>(dTbinning);
        ts_cal.calculate();
      }
    }
    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }
    else if ("--run")
    {
      auto sub_arg = args.load<std::string>();
      std::vector<std::string> runs(findFilesWildcard(dataPath+"run*.fast"));
      // if (sub_arg == "U")
    }
    else throw_error(args.getArg() + " unkown !!");
  }



  return 1;
}