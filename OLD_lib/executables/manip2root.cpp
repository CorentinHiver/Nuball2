#include <libRoot.hpp>
#include <Manip.hpp>
#include <Faster2Root.hpp>
#include <CloversV2.hpp>

/**
 * @brief DO NOT WORK NOW  Uses the Faster2Root class to convert a bunch of runs in the same folder.
 * 
 * @todo shaping it better based on work done on faster2root
 */
int main(int argc, char ** argv)
{
  if (argc < 3) 
  {
    print("");
    print("Usage of manip2root : /path/to/manip/ [-O /output/path/] [-F [[options of faster2root]]]");
    print("");
    print("First parameter options : /path/to/manip/ or /path/to/manip/wantedRun* or /path/to/manip/wantedRun1 /path/to/manip/wantedRun2 or /path/to/wantedRunsList.list (inclusive or)");
    print("");
    Faster2Root::printParameters(); // In order to print the options of faster2root
  }
  else
  {
    print(argv_to_string(argv));

    int argi = 1;
    Manip manip;
    std::string data;
    constexpr auto defaultOutpath =  "./";
    std::string outpathStr = defaultOutpath;
    while(argi<argc && (data = std::string(argv[argi++])) != "-F")
    {
      if (extension(data) == "list") manip.addListFile(data);
      else if (data == "-O") outpathStr = argv[argi++];
      else manip.addPath(data);
    }
    if (!manip) Colib::throw_error(concatenate("manip", manip, " empty !"));
    
    if (outpathStr == defaultOutpath)
    {
      printC("Enter the path of the output root files (default : ", outpathStr, ").");
      std::getline(std::cin, outpathStr);
    }
    
    Path outpath (outpathStr, true);

    std::string run;
    while(manip >> run)
    {
      Path runPath(run, false);
      if (!runPath.exists()) {error(concatenate(runPath, " doesn't exist !")); continue;}

      File output (outpath, removeExtension(runPath.folder().name())+".root");

      auto arguments = string_to_argv(concatenate("faster2root ", runPath, " ", output.string(), argv_to_string(argv, argi)));
      auto const & new_argc = getList(argv_to_string(arguments), " ", true).size();
      print(argv_to_string(arguments), new_argc); 
      
    #ifdef TRIGGER_PPC
      thread_local CloversV2 clovers;
      try {
        Faster2Root(new_argc, arguments, [](Event const & event) 
        {// Example of a front-back DSSD coincidence
          for (int hit_i = 0; hit_i<event.mult; hit_i++)
          {
            auto const & label0 = event.labels[hit_i];
            if (label0 > 799 && label0 < 820 ) // If there is a ring
            {
              for (int hit_j = 0; hit_j<event.mult; hit_j++)
              {
                auto const & label1 = event.labels[hit_j];
                if (label1 > 819 && label1 < 856 ) // If there is a sector
                {
                  clovers.loadRaw(event);
                  if (clovers.clean.size() > 0) return true; // If there is at least a clean Ge
                }
              }
            }
          }
          return false;
        });
      }
    #else
      try {Faster2Root(new_argc, arguments);}
    #endif //TRIGGER_PPC
    catch(FilesManager::FolderEmpty const & error) {continue;} // If the input folder is empty
    catch(OverwriteError const & error) {continue;} // If the output file already exists then move to the next folder to histogram
      
      delete_argv(arguments);
    }
  }
  return 1;
}

// libPath=../lib g++ -o manip2root manip2root.cpp $(root-config --cflags) $(pkg-config --cflags libfasterac) -I$libPath/Modules -I$libPath -I$libPath/Classes -I$libPath/Analyse $(root-config --libs) $(pkg-config --libs libfasterac) -Wall -Wextra -O2