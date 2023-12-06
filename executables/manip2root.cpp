#include <libRoot.hpp>
#include <Manip.hpp>

/**
 * @brief Uses the manip2root executable to convert a bunch of runs in the same folder, given a list of runs.
 * @details That is, all you need is to compile faster2root to your convenience before running this executable
 * Read the README to know how to choose which file to compile
 */
int main(int argc, char ** argv)
{
  if (argc < 4) 
  {
    print("Usage of manip2root : runs.list /path/to/manip/ /path/to/output/ [[parameters_of_faster2root]]");
    system("./faster2root"); // Here is printed as well the parameters of faster2root
  }
  else
  {
    Manip manip(argv[1]);
    std::string parameters; for (int i = 4; i<argc; i++) parameters += std::string(" ")+argv[i];
    std::string run;
    while(manip >> run)
    {
      Path input (argv[2]+run);
      Path output(argv[3]+removeExtension(run), true);
      std::string command = std::string("./faster2root ") + input.string() + std::string(" ") + output.string() + parameters;
      print(command);
      if (input) system(command.c_str());
      else print(input.string(), "NOT FOUND");
    }
  }
  return 1;
  
}