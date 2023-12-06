#include <libRoot.hpp>
#include <Manip.hpp>

/**
 * @brief Uses the manip2histo executable to make energy spectra a bunch of runs in the same folder, given a list of runs.
 * @details That is, all you need is to compile faster2histo to your convenience before running this executable
 * Read the README to know how to choose which executable to compile
 */
int main(int argc, char ** argv)
{
  if (argc < 2) 
  {
    print("Usage of manip2histo : runs.list /path/to/manip /path/to/output [[parameters_of_faster2histo]]");
    system("./faster2histo"); // Here is printed as well the parameters of faster2root
  }
  else
  {
    Manip manip(argv[1]);
    std::string parameters; for (int i = 4; i<argc; i++) parameters += std::string(" ")+argv[i];
    std::string run;
    while(manip >> run)
    {
      Path input (Path(argv[2])+run);
      Path output(Path(argv[3])+removeExtension(run), true);
      // std::string command = "./faster2histo"+parameters+" -F "+;
      std::string command = concatenate("./faster2histo", parameters, " -F ", input.string(), " -1 -O ", output.string());
      // print(command.c_str());
      system(command.c_str());
    }
  }
  return 1;
  
}