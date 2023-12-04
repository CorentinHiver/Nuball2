// #include <Faster2Root.hpp>
#include <libRoot.hpp>
#include <Manip.hpp>
#define NORMAL
// #define TRIGGER

/**
 * You can use manip2root to convert a bunch of runs in the same folder given a list of runs.
 * 
 */
int main(int argc, char ** argv)
{
  if (argc < 4) 
  {
    print("Usage of manip2root : runs.list /path/to/manip/ /path/to/output/ [[parameters]]");
    system("./faster2root"); // Here is printed as well the parameters of faster2root
  }
  else
  {
    Manip manip(argv[1]);
    std::string run;
    std::string parameters; for (int i = 4; i<argc; i++) parameters += std::string(" ")+argv[i];
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