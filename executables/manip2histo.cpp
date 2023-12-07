#include <Faster2Histo.hpp>
#include <Manip.hpp>

/**
 * @brief Uses the manip2histo executable to make energy spectra a bunch of runs in the same folder, given a list of runs.
 */
int main(int argc, char ** argv)
{
  if (argc < 4) 
  {
    print("Usage of manip2histo : runs.list /path/to/manip /path/to/output [[parameters_of_faster2histo]]");
    Faster2Histo(0, argv);
  }
  else
  {
    Manip manip(argv[1]);
    Path datapath(argv[2]);
    Path outpath(argv[3]);
    if (!datapath.exists()) throw_error(concatenate(datapath, " doesn't exist !"));
    std::string run;
    while(manip >> run)
    {
      std::string run_name = rmPathAndExt(run);
      Path input (concatenate(datapath, run_name, ".fast"));
      File output (outpath, run_name+".root");
      if (!input.exists()) continue;
      char** arguments = string_to_argv(concatenate("faster2histo", argv_to_string(argv, 4), " -F ", input, " -1 -O ", output));
      print(argv_to_string(arguments));
      Faster2Histo(argc+2, arguments);
      delete_argv(arguments);
    }
  }
  return 1;
}
// ./manip2histo parameters/Uranium.list ~/faster_data/N-SI-136 ../Playground/testFolder/