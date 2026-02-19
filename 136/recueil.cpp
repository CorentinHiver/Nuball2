#include "../lib/Classes/Arguments.hpp"
#include "../lib/FasterReader/FasterRunReader.hpp"
#include "../lib/RootReader/RootReader.hpp"
#include "../lib/RootReader/TimeshiftCalculator.hpp"

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print("Usage :");
    return 0;
  }

  Arguments args(argc, argv);
  while(args.next())
  {
    if (args == "--ts" || args == "--timeshifs")
    {
      auto sub_arg = args.load<std::string>();
      if (sub_arg == "all")
      {
        std::string dataPath = "~/nuball2/N-SI-136/run*.fast/";
        std::string outputPath = "~/nuball2/N-SI-136_root/dT/";
        std::vector<std::string> runs (Colib::findFilesWildcard(dataPath));
        for (auto const & run : runs)
        {
          print("Calculating dT output for", run);
          FasterRunReader reader;
          reader.addFiles(run+"*");
          reader.setOutputPath(outputPath);
          reader.setMaxFilesMemory(2); // NOMBRE A CHANGER
          reader.setOverwrite(false);
          reader.setMerge(true);
          reader.setRef(252);

          reader.run();

          print();
        }

        for (auto const & dTfilename : Colib::findFilesWildcard(outputPath+"*"))
        {
          TimeshiftCalculator ts_cal(dTfilename);
          ts_cal.setBins<NSI136::maxLabel>([](Label label) -> Time {
                 if (NSI136::isGe [label])   return    2_ns;
            else if (NSI136::isBGO[label])   return   1_ns;
            else if (label == 251)           return 100_ps;
            else if (label == 252)           return 100_ps;
            else if (NSI136::isParis[label]) return 100_ps;
            else if (NSI136::isDSSD[label])  return   2_ns;
            else                             return  10_ns;
          });
          ts_cal.calculate();
        }
      }
      else if (sub_arg == "-f")
      {
        TimeshiftCalculator ts_cal(args.load<std::string>());
        ts_cal.setBins<NSI136::maxLabel>([](Label label) -> Time {
               if (NSI136::isGe [label])   return  2_ns;
          else if (NSI136::isBGO[label])   return  1_ns;
          else if (label == 251)           return 100_ps;
          else if (label == 252)           return 100_ps;
          else if (NSI136::isParis[label]) return 100_ps;
          else if (NSI136::isDSSD[label])  return   2_ns;
          else                             return  10_ns;
        });
        ts_cal.calculate();
      }
    }
    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }
    else Colib::throw_error(args.getArg() + " unkown !!");
  }



  return 1;
}