#include <libRoot.hpp>
#include <Manip.hpp>
#include <Faster2Root.hpp>

/**
 * @brief Uses the Faster2Root class to convert a bunch of runs in the same folder.
 * @todo shaping it better based on work done on faster2root
 */
int main(int argc, char ** argv)
{
  if (argc < 3) 
  {
    print("Usage of manip2root : /path/to/manip/ /path/to/output/ [[options of faster2root]]");
    // print("Usage of manip2root : /path/to/manip/ /path/to/output/ [[options of manip]] [[options of faster2root]]");
    // print("manip2root options : ");
    // print("-l [list_runs] : gives a list of the runs to convert if you don't want to convert everything in the folder");
    // print("-r [list_runs] : gives a list of the runs NOT to convert if you don't want to convert everything in the folder");
    system("./faster2root"); // Here is printed as well the options of faster2root
  }
  else
  {
    Manip manip;

    for (int i = 3; i<argc; i++)
    {
      std::string option = argv[i];

           if (option == "-l") manip.readFile(argv[++i]);
      // else if (option == "-r")
      // {
      //   manip.readFolder(argv[++i]);
      // }
      
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
        auto arguments = string_to_argv(concatenate("faster2manip", argv_to_string(argv, 4), " -F ", input, " -1 -O ", output));
        print(argv_to_string(arguments));
        try {Faster2Histo(argc+2, arguments);}
        catch(OverwriteError const & error) {continue;} // If the output file already exist then move to the next folder to histogram
        delete_argv(arguments);
      }

    // Manip manip(argv[1]);
    // std::string parameters; for (int i = 4; i<argc; i++) parameters += std::string(" ")+argv[i];
    // std::string run;
    // while(manip >> run)
    // {
    //   Path input (argv[2]+run);
    //   Path output(argv[3]+removeExtension(run), true);
    //   std::string command = std::string("./faster2root ") + input.string() + std::string(" ") + output.string() + parameters;
    //   print(command);
    //   if (input) system(command.c_str());
    //   else print(input.string(), "NOT FOUND");
    // }
    }
  }
  return 1;
  
}