#pragma once

#include "TFileMerger.h"
#include "TSystem.h"

#include "../Classes/RunReader.hpp"

#include "FasterRootInterface.hpp"

class FasterRunReader : public RunReader
{
public:
  FasterRunReader() noexcept = default;

  void printArgs()
  {
    print("Additionnal arguments of FasterRunReader :");
    print();  
    // print("-a [alignement_file]        : Loads the run-by-run calibration correction file");
    // print("-c [calibration_file]       : Loads the calibration file");
    print("-n       [hits_number]        : Choose the number of hits to read inside a file (default: all)");
    print("-nf      [nb_files_in_memory] : In merge mode : sets the maximum number of files to load at once");
    print("--hits                        : Do not perform event building (skipped in --ref and --rf modes)");
    print("--merge                       : merge the outputs");
    print("--ref    [label]              : use a detector as time reference and build events around it (usefull for time shift calculations)");
    print("--rf     [label]              : use rf with downscaled pulse registered at the given label. Frequency is automatically extracted from the data");
    // print("-m [threads_number]        : Choose the number of files to treat in multithread mode (default: 1)");
    print("-T [time_window]              : Not used if --hits activated. Sets the event building time window in ns (default : 2000 ns). In RF mode : number of pulses (TODO!)");
    // print("--out_path [output_path]   : Set the output path (default ./)");
    print();
  };
  bool processArg(Arguments & args)
  {
    if (args.size() == 0) 
    {
      RunReader::printArgs();
      printArgs();
      return false;
    }

    else while(args.next()) if (!RunReader::processArg(args))
    {
           if (args == "-nf")
      {
        m_maxFilesInMemory = args.load<int>();
      }
      else if (args == "--hits")
      {
        buildEvents(false);
      }
      else if (args == "--merge")
      {
        setMerge(true);
      }
      else if (args == "--rf")
      {
        Label label; if (!args.load(label)) error("Option --rf needs a label");
        setRF(label);
      }
      else if (args == "--ref")
      {
        Label label; if (!args.load(label)) error("Option --ref needs a label");
        setRef(label);
      }
      else if (args == "-T")
      {
        setTimeWindow(static_cast<Time>(args.load<double>()));
      }
      else
      {
        Colib::throw_error(args.getArg() + " unkown parameter");
      }
    }
    setTimeShifts(std::move(RunReader::p_timeshift));
    setMaxHits(p_nbMaxHits);
    return true;
  }
  
  FasterRunReader(Arguments & args) : RunReader()
  {
    processArg(args);
  }
  
  FasterRunReader(int argc, char** argv) : RunReader()
  {
    Arguments args(argc, argv);
    processArg(args);
  }

  void loadConfig() noexcept override
  {
    
  }

  // Interface with FasterRootInterface
  void setMaxHits       (int                 nb         ) {for (auto & reader : m_readers) reader.setMaxHits     (nb)                ;}
  void setHitTrigger    (HitTrigger          trigger    ) {for (auto & reader : m_readers) reader.setHitTrigger  (trigger)           ;}
  void setEventTrigger  (EventTrigger        trigger    ) {for (auto & reader : m_readers) reader.setEventTrigger(trigger)           ;}
  void setTimeWindow    (Time                time_window) {for (auto & reader : m_readers) reader.setTimeWindow  (time_window)       ;}
  void setTimeShifts    (std::string const & tsFile     ) {for (auto & reader : m_readers) reader.loadTimeshifts (tsFile)            ;}
  void setTimeShifts    (Timeshifts        && tshifts   ) {for (auto & reader : m_readers) reader.loadTimeshifts (std::move(tshifts));}

  // Other interface :
  void setMerge(bool b = true) {m_merge = b;}
  void setRef(Label refLabel)
  {
    m_useRef = true;
    m_refLabel = refLabel;
  }
  void setRF(Label rfLabel)
  {
    m_useRF = true;
    m_rfLabel = rfLabel;
  }
  void buildEvents(bool b = true) {m_buildEvents = b;}

  void processData(std::string const & outFile)
  {
  #ifdef MULTITHREAD
    auto & reader = m_readers[Colib::MT::getThreadIndex()];
  #else // !MULTITHREAD
    auto & reader = m_readers.front();
  #endif// MULTITHREAD
    reader.timeSorting();
         if (m_useRF      ) reader.writeEventsWithRF (m_refLabel, outFile);
    else if (m_useRef     ) reader.writeEventsWithRef(m_rfLabel , outFile);
    else if (m_buildEvents) reader.writeEvents       (outFile);
    else                    reader.writeHits         (outFile);
  }

#ifdef MULTITHREAD  
  void run()
  {
    distributeFiles();
    auto const & files   = RunReader::p_distributedFiles;   // Aliasing for clarity
    auto const & outPath = RunReader::p_outPath; // Aliasing for clarity

    if (files.empty()) Colib::throw_error("No files");
    std::string outFile;
    
    if (m_merge)
    {
      outFile = outPath + Colib::removeLastPart(Colib::removePath(files[0][0]), '_') + ".root";
      std::vector<std::string> tempFiles;
      for (size_t thread_i = 0; thread_i<p_nbThreads; ++thread_i) 
        tempFiles.push_back(Colib::appendFilename(outFile, std::to_string(thread_i)));

      if(!checkOutput(outFile)) return;

      Colib::MT::parallelise_function(p_nbThreads, [this, &files, &tempFiles]()
      {// Parallel section:
        auto const thread_i = Colib::MT::getThreadIndex();
        while(m_readers[thread_i].loadDatafiles(files[thread_i], m_maxFilesInMemory)) 
          processData(tempFiles[thread_i]);
      });

      Colib::pause();

      print("Merging the output");

      TFileMerger mergingFile;
      mergingFile.OutputFile(outFile.c_str(), "RECREATE");
      for (auto const & file : tempFiles) mergingFile.AddFile(file.c_str());

      if (mergingFile.Merge())
      {
        std::string rmCommand = "rm ";
        for (auto const & file : tempFiles) rmCommand += file+" ";
        gSystem->Exec(rmCommand.c_str());
      }
      else
      {
        error("In FasterRunReader::run() - in multithread mode, final merge failed (try to use hadd instead)");
      }
    }
    else
    {
      Colib::MT::parallelise_function(p_nbThreads, [this, &files, &outPath, &outFile]()
      {// Parallel section:
        auto const thread_i = Colib::MT::getThreadIndex();
        for (auto const & dataFile : files[thread_i])
        {
          outFile = outPath + Colib::removeExtension(Colib::removePath(dataFile))+".root";
          if(!checkOutput(outFile)) return;
          m_readers[thread_i].loadDatafile(dataFile);
          processData(outFile);
        }
      });
    }
  }
#else //!MULTITHREAD
  void run()
  {
    auto const & files   = RunReader::p_files;   // Aliasing for clarity
    auto const & outPath = RunReader::p_outPath; // Aliasing for clarity

    if (files.empty()) Colib::throw_error("No files");
    std::string outFile;

    if (m_merge)
    {
      outFile = outPath + Colib::removeLastPart(Colib::removePath(files[0]), '_')+".root";
      if(!checkOutput(outFile)) return;
      while(m_readers[0].loadDatafiles(files, m_maxFilesInMemory)) processData(outFile);
    }
    else
    {
      for (auto const & dataFile : files)
      {
        outFile = outPath + Colib::removeExtension(Colib::removePath(dataFile))+".root";
        if(!checkOutput(outFile)) return;
        m_readers[0].loadDatafile(dataFile);
        processData(outFile);
      }
    }
  }
#endif //MULTITHREAD

protected:

  std::array<FasterRootInterface, p_nbThreads> m_readers;

  // Other convenience attributes:
  int m_maxFilesInMemory = 0;
  bool m_buildEvents = true;
  bool m_merge = false;
  bool m_useRef = false;
  bool m_useRF = false;
  Label m_refLabel = 251;
  Label m_rfLabel = 252;
};