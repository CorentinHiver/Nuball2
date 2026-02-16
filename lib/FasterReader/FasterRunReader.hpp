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
    // print("-n       [hits_number]        : Choose the number of hits to read inside a file (default: all)");
    print("-nf      [nb_files_in_memory] : In merge mode : sets the maximum number of files to load at once");
    print("--hits                        : Do not perform event building (skipped in --ref and --rf modes)");
    print("--merge                       : merge the outputs");
    print("--ref    [label]              : use a detector as time reference and build events around it (usefull for time shift calculations)");
    print("--rf                          : use rf with downscaled pulse. Frequency is automatically extracted from the data");
    print("--rf-label                    : Sets the label of the channel where the rf is downscaled");
    // print("-m [threads_number]        : Choose the number of files to treat in multithread mode (default: 1)");
    print("-T [time_window]              : Not used if --hits activated. Sets the event building time window in ns (default : 2000 ns). In RF mode : number of pulses (TODO!)");
    // print("--out_path [output_path]   : Set the output path (default ./)");
    print();
  };
  bool processArg(Arguments & args)
  {
    try{
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
          setMaxFilesMemory(args.load<int>());
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
          setRF();
        }
        else if (args == "--rf-label")
        {
          setRF(args.load<Label>());
        }
        else if (args == "--ref")
        {
          setRef(args.load<Label>());
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
    }
    catch(Arguments::MissingArg & error)
    {
      error.print();
      _exit(43);
    }
    if (p_files.empty())
    {
      error("No files !");
      return false;
    }
    setTimeShifts(std::move(RunReader::p_timeshift));
    setCalibration(std::move(RunReader::p_calib));
    setMaxHits(p_nbMaxHits);
    setTotMaxHits(p_nbTotMaxHits);
    return true;
  }
  
  FasterRunReader(Arguments & args) : RunReader()
  {
    if (!processArg(args)) Colib::throw_error("Argument processing failed !");
  }
  
  FasterRunReader(int argc, char** argv) : RunReader()
  {
    Arguments args(argc, argv);
    if (!processArg(args)) Colib::throw_error("Argument processing failed !");
  }

  bool loadConfig(std::string config) noexcept override
  {
    config = "to_skip "+config;
    auto argv = Colib::string_to_argv(config);
    Arguments args(config.size()+1, argv);
    auto const ret = processArg(args);
    Colib::delete_argv(argv);
    return ret;
  }

  // Interface with FasterRootInterface :
  void setMaxHits       (int                 nb         ) {for (auto & reader : m_readers) reader.setMaxHits     (nb)                ;}
  void setTotMaxHits    (int                 nb         ) {for (auto & reader : m_readers) reader.setTotMaxHits  (nb)                ;}
  void setHitTrigger    (HitTrigger          trigger    ) {for (auto & reader : m_readers) reader.setHitTrigger  (trigger)           ;}
  void setEventTrigger  (EventTrigger        trigger    ) {for (auto & reader : m_readers) reader.setEventTrigger(trigger)           ;}
  void setTimeWindow    (Time                time_window) {for (auto & reader : m_readers) reader.setTimeWindow  (time_window)       ;}
  void setTimeShifts    (std::string const & tsFile     ) {for (auto & reader : m_readers) reader.loadTimeshifts (tsFile)            ;}
  void setTimeShifts    (Timeshifts       && tshifts    ) {for (auto & reader : m_readers) reader.loadTimeshifts (std::move(tshifts));}
  void setCalibration   (Calibration      && calib      ) {for (auto & reader : m_readers) reader.setCalibration (std::move(calib  ));}

  // Other interface :
  void setMaxFilesMemory(int nb) {m_maxFilesInMemory = nb;}
  void setMerge(bool b = true) {m_merge = b;}
  void setRef(Label refLabel)
  {
    m_useRef = true;
    m_refLabel = refLabel;
  }
  void setRF()
  {
    m_useRF = true;
  }
  void setRF(Label rfLabel)
  {
    m_rfLabel = rfLabel;
    m_useRF = true;
  }
  void buildEvents(bool b = true) {m_buildEvents = b;}

  // Data processing :

  void processData(std::string const & outFile)
  {
  #ifdef MULTITHREAD
    auto & reader = m_readers[Colib::MT::getThreadIndex()];
  #else // !MULTITHREAD
    auto & reader = m_readers.front();
  #endif// MULTITHREAD
    reader.timeSorting();
         if (m_useRF      ) reader.writeEventsWithRF (outFile, m_rfLabel);
    else if (m_useRef     ) reader.writeEventsWithRef(outFile, m_refLabel);
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
  int m_maxFilesInMemory = 1;
  bool m_buildEvents = true;
  bool m_merge = false;
  bool m_useRef = false;
  bool m_useRF = false;
  Label m_refLabel = 252;
  Label m_rfLabel = 251;
};