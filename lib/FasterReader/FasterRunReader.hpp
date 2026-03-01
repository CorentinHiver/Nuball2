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
  void setMaxHits       (int                 nb         ) {m_reader.setMaxHits     (nb)                ;}
  void setTotMaxHits    (int                 nb         ) {m_reader.setTotMaxHits  (nb)                ;}
  // void setHitTrigger    (HitTrigger          trigger    ) {m_reader.setHitTrigger  (trigger)           ;}
  void setEventTrigger  (EventTrigger        trigger    ) {m_reader.setEventTrigger(trigger)           ;}
  void setTimeWindow    (Time                time_window) {m_reader.setTimeWindow  (time_window)       ;}
  void setTimeShifts    (std::string const & tsFile     ) {m_reader.loadTimeshifts (tsFile)            ;}
  void setTimeShifts    (Timeshifts       && tshifts    ) {m_reader.loadTimeshifts (std::move(tshifts));}
  void setCalibration   (Calibration      && calib      ) {m_reader.setCalibration (std::move(calib  ));}

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
    setRF();
    m_rfLabel = rfLabel;
  }
  void buildEvents(bool b = true) {m_buildEvents = b;}

  // Data processing :

  static std::string renameOutputRef(std::string const & m_outputFile, Label m_refLabel) {
    return Colib::removeExtension(m_outputFile)+"_ref_"+std::to_string(m_refLabel)+".root";
  }

  static std::string renameOutputRF(std::string const & m_outputFile) {
    return Colib::removeExtension(m_outputFile)+"_rf.root";
  }

  void processData(std::string const & outFile)
  {
    m_outputFile = outFile;
    if (m_useRF) 
    {
      m_outputFile = renameOutputRF(m_outputFile);
      m_reader.writeEventsWithRF (m_outputFile, m_rfLabel);
    }
    else if (m_useRef) 
    {
      m_outputFile = renameOutputRef(m_outputFile, m_refLabel);
      m_reader.writeEventsWithRef(m_outputFile, m_refLabel);
    }
    else if (m_buildEvents) m_reader.writeEvents       (m_outputFile);
    else                    m_reader.writeHits         (m_outputFile);
  }

#if defined (MULTITHREAD) && defined (DEV)
// Work in progress
// Maintenant il faut inclure des pools de threads pour lire les données en serial mais traiter et écrire les données en parallèle
  void processDataMT(std::string const & outFile)
  {
    if (!Colib::MT::isMasterThread()) return processData(outFile);

    m_outputFile = outFile;
    static thread_local RootInterface writer;
    writer.setData(std::move(m_reader.data()));
    if (m_useRF) 
    {
      m_outputFile = renameOutputRF(m_outputFile);
      writer.writeEventsWithRF (m_outputFile, m_rfLabel);
    }
    else if (m_useRef) 
    {
      m_outputFile = renameOutputRef(m_outputFile, m_refLabel);
      writer.writeEventsWithRef(m_outputFile, m_refLabel);
    }
    else if (m_buildEvents) writer.writeEvents       (m_outputFile);
    else                    writer.writeHits         (m_outputFile);
  }

#endif // CoMT

  void run()
  {
    auto const & files   = RunReader::p_files;   // Aliasing for clarity
    auto const & outPath = RunReader::p_outPath; // Aliasing for clarity

    if (files.empty()) Colib::throw_error("No files");
    std::string outFile;

    if (m_merge)
    {
      outFile = Colib::removeLastPart(Colib::removePath(files[0]), '_')+".root";
      std::string outFilePath;
           if (m_useRef) outFilePath = renameOutputRef(outPath + outFile, m_refLabel);
      else if (m_useRF ) outFilePath = renameOutputRF (outPath + outFile);
      else outFilePath = outPath + outFile;
      
      print("m_useRef", nicer_bool(m_useRef), renameOutputRef(outPath + outFile, m_refLabel));
      print("m_useRF", nicer_bool(m_useRF), renameOutputRF (outPath + outFile));
      print(outFilePath);
      if (!checkOutput(outFilePath)) return;

      std::string tempOutFilePath = outPath + "temp_" + outFile;

    #ifdef CoMT

  #ifdef DEV
      auto my_producer = [&]() mutable -> std::optional<std::vector<Hit>> {
        if (m_reader.loadDatafiles(files, m_maxFilesInMemory)) return m_reader.data() ;
        else return std::nullopt;
      };
      auto my_consumer = [&](std::vector<Hit> && data) {
        static thread_local RootInterface writer;
        auto thread_filename = outPath + Colib::removeExtension(outFile)+std::to_string(Colib::MT::getThreadIndex())+".root";
        writer.setData(std::move(data));
        if (m_useRF) 
        {
          thread_filename = renameOutputRF(thread_filename);
          writer.writeEventsWithRF (thread_filename, m_rfLabel);
        }
        else if (m_useRef) 
        {
          thread_filename = renameOutputRef(thread_filename, m_refLabel);
          writer.writeEventsWithRef(thread_filename, m_refLabel);
        }
        else if (m_buildEvents) writer.writeEvents       (thread_filename);
        else                    writer.writeHits         (thread_filename);
      };

      Colib::MT::parallelise_stream(my_producer, my_consumer);
  #else // NO DEV
      while(m_reader.loadDatafiles(files, m_maxFilesInMemory)) processData(tempOutFilePath);
  #endif // DEV

    #else // NO CoMT
      while(m_reader.loadDatafiles(files, m_maxFilesInMemory)) processData(tempOutFilePath);
    #endif // CoMT

      std::rename(tempOutFilePath.c_str(), outFilePath.c_str());
    }
    else
    {
      for (auto const & dataFile : files)
      {
        outFile = outPath + Colib::removeExtension(Colib::removePath(dataFile))+".root";
        if(!checkOutput(outFile)) return;
        m_reader.loadDatafile(dataFile);
        processData(outFile);
      }
    }
  }

  auto getOutputFilename() const {return (m_merge) ? m_outputFile : "No merged output";}

protected:

  FasterRootInterface m_reader;

  std::string m_outputFile = {};

  // Other convenience attributes:
  int m_maxFilesInMemory = {1};
  bool m_buildEvents = true;
  bool m_merge = false;
  bool m_useRef = false;
  bool m_useRF = false;
  Label m_refLabel = 252;
  Label m_rfLabel = 251;
};