#ifdef MULTITHREAD
  #include "../CoMT/CoMT.hpp"
#endif //MULTITHREAD

#include "../libCo.hpp"
#include "../Classes/Arguments.hpp"
#include "../Classes/Timeshifts.hpp"
#include "../Classes/Calibration.hpp"

class RunReader
{
public:
  RunReader() noexcept = default;
  void printArgs()
  {
    print("Arguments of RunReader :");
    print();  
  // #ifdef MULTITHREAD
  //   print("-m [threads_number]     : Choose the number of threads (default 1)");
  // #endif //MULTITHREAD
    print("-c [filename.calib]     : Loads the calibration file");
    print("-n [hits_number]        : Choose the number of hits to read inside a file (default: all)");
    print("-N [hits_number]        : Choose the total number of hits to read (default: all)");
    print("-f [file_name.fast]     : add a new file. You can use wildcards (* or ?) to load all matching pattern, but you need to add \"\" around the name.");
    print("-F [pattern] [nb_files] : add nb_files of new files matching the wildcard pattern.");
    print("-i [ID_file]            : Load detector's ID file and set the name of the histograms accordingly");
    print("-O [outputPath]         : Set the path of the output file");
    print("-o                      : Overwrite the output");
    print("-t [filename.dT]        : Loads the timeshifts (default : no timeshits)");
    print();
  }
  bool processArg(Arguments & args)
  {
    if (args.size() == 0) {printArgs(); return false;}

  // #ifdef MULTITHREAD
  //   if (args == "-m")
  //   {
  //     p_nbThreads = args.load<size_t>();
  //   }
  // #endif //MULTITHREAD

    if (args == "-c")
    {
      p_calib.load(args.load<std::string>());
    }
    else if (args == "-n")
    {
      p_nbMaxHits = static_cast<uint64_t>(args.load<double>());
    }
    else if (args == "-N")
    {
      p_nbTotMaxHits = static_cast<uint64_t>(args.load<double>());
    }
    else if (args == "-f")
    {
      addFiles(args.load<std::string>());
    }
    else if (args == "-F")
    {
      addFiles(args.load<std::string>(), args.load<int>());
    }
    else if (args == "-o")
    {
      setOverwrite(true);
    }
    else if (args == "-O")
    {
      if (!args.next()) error("Option -f needs an argument");
      setOutputPath(args);
    }
    else if (args == "-t")
    {
      p_timeshift.load(args.load<std::string>());
    }
    else return false; // Not found
    return true;      // Found
  }
  void addFiles(std::string const & files, int nb = -1)
  {
    auto const & filenames = Colib::findFilesWildcard(files);
    debug(filenames);
    if(filenames.empty()) return;
    size_t const nbFiles = (nb<1) ? std::min(filenames.size(), size_t(nb)) : nb;
    for (size_t file_i = 0; file_i < nbFiles; ++file_i) p_files.push_back(filenames[file_i]);
  }
  void setOverwrite(bool b = true) {p_overwrite = b;}
  void setOutputPath(std::string const & path) 
  {
    p_outPath = path;
    if (p_outPath.back() != '/') p_outPath.push_back('/');
    Colib::ensurePath(p_outPath, true);
  }

  bool checkOutput(std::string const & outFile)
  {
    if (Colib::fileExists(outFile))
    {
      if (p_overwrite) fs::remove(outFile);
      else
      {
        error(outFile, "already exists ! Use -o mode or RunReader::setOverwrite()");
        return false;
      }
    }
    return true;
  }
  
#ifdef MULTITHREAD
  
void distributeFiles()
{
    if (p_nbThreads <= 1) return;

    const size_t total = p_files.size();
    const size_t chunk_size = total / p_nbThreads;
    const size_t remainder  = total % p_nbThreads;

    size_t offset = 0;

    for (size_t thread_i = 0; thread_i < p_nbThreads; ++thread_i)
    {
      size_t this_chunk = chunk_size + (thread_i < remainder ? 1 : 0);

      auto& files_for_thread = p_distributedFiles[thread_i];
      files_for_thread.clear();
      files_for_thread.reserve(this_chunk);

      for (size_t i = 0; i < this_chunk; ++i) files_for_thread.push_back(p_files[offset + i]);

      offset += this_chunk;
    }
}
#endif //MULTITHREAD

  virtual bool loadConfig(std::string) noexcept = 0; 

protected:
  
  std::vector<std::string> p_files;
  std::string p_outPath = "./";
  bool p_overwrite = false;
  uint64_t p_nbMaxHits = -1;
  uint64_t p_nbTotMaxHits = Colib::big<uint64_t>();
  
  Timeshifts p_timeshift;
  Calibration p_calib;
  
#ifdef MULTITHREAD
  inline static constexpr size_t p_nbThreads = MULTITHREAD;
  std::array<std::vector<std::string>, p_nbThreads> p_distributedFiles;
#else // !MULTITHREAD
  inline static constexpr size_t p_nbThreads = 1;
#endif // MULTITHREAD

};