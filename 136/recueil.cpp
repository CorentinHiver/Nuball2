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
      TimeshiftCalculator ts(args.load<std::string>());
      ts.calculate();
    }
    else if (args == "--fast" || args == "--faster-reader")
    {
      FasterRunReader reader(args);
      reader.run();
    }
  }



  return 1;
}