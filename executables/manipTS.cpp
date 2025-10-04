#include <MTObject.hpp>
#include <Timeshifts.hpp>
#include <MTFasterReader.hpp>
#include <Manip.hpp>

#define QDC1MAX

/**
 * @brief Uses the manip2histo executable to calculate time shifts between detectors of a bunch of runs in the same folder, given a list of runs.
 * @details
 * 
 * Tip : to easily create a runs.list file, simply do the following : 
 * $ cd /path/to/manip
 * $ ls -d * / >> runs.list (the * and the / are together, not separate)
 */
int main(int argc, char ** argv)
{
  if (argc < 3) 
  {
    print("Usage of manipTS (time shifts calculator) : /path/to/manip [[]]");
    print("Arguments :");
    print("-n : number of files per manip - default=all");
    print("-m : number of threads");
  }
  else
  {
    int nb_files = -1;
    int nb_threads = -1;
    for (int i = 3; i<argc; i++)
    {
      std::string command = argv[i];
           if (command == "-m") nb_threads = std::stoi(argv[++i]);
      else if (command == "-n") nb_files = std::stoi(argv[++i]);
      else Colib::throw_error(concatenate("Unkown command", command));
    }

    MTObject::Initialise(nb_threads);
    MTObject::adjustThreadsNumber(nb_files);

    Manip manip(argv[1]);
    Path datapath(argv[2]);
    Path outpath("Timeshifts");
    if (!datapath.exists()) Colib::throw_error(concatenate(datapath, " doesn't exist !"));
    
    std::string run;
    while(manip >> run)
    {
      std::string run_name = rmPathAndExt(run);
      Path input (concatenate(datapath, run_name, ".fast"));
      Timeshifts ts;
      ts.calculate(input.string(), nb_files);
      ts.write(run_name);
    }
  }
  return 1;
}