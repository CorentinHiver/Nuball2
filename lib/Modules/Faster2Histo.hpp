#ifndef FASTER2HISTO_HPP
#define FASTER2HISTO_HPP

#include "../Classes/FilesManager.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"
#include "../MTObjects/MTList.hpp"
#include "../Classes/Timer.hpp"

class Faster2Histo
{
public:
  Faster2Histo() {}
  Faster2Histo(int argc, char** argv);
  
  void addFolder(std::string const & folder, int const & nb_files = -1) 
  {
    m_reader.addFolder(folder, nb_files);
    setOutFilename(rmPathAndExt(folder)+".root");
  }
  void addFile(std::string const & file) 
  {
    m_reader.addFile(file);
    setOutFilename(rmPathAndExt(file)+".root");
  }

  void multirun(std::string const & outfilename = "", int const & nb_threads = -1);
  void treatFile(Hit & hit, FasterReader & reader);
  void write(std::string const & outfilename = "");

  void setOutFilename(std::string const & out_filename) noexcept {m_outFilename = out_filename;}
  void setWriteMode(const char* mode) noexcept {delete[] write_mode; write_mode = mode;}

  void setNbFiles(int const & _nb_files) {m_nb_files = _nb_files;}
  void setNbThreads(int const & _nb_threads) {MTObject::Initialize(_nb_threads);}
  void overwrite(bool const & _overwrite) {m_overwrite = _overwrite;}

private:
  void printParameters() const noexcept;
  static void dispatch_threads(Hit & hit, FasterReader & reader, Faster2Histo & f2h) {f2h.treatFile(hit, reader);}
  MTFasterReader m_reader;
  std::vector<Label> m_labels;
  std::unordered_map<Label, MTTHist<TH1F>> m_spectra;
  const char* write_mode = "RECREATE";
  File m_outFilename;

  int m_nb_files = -1;
  int m_overwrite = false;
};

void Faster2Histo::printParameters() const noexcept
{
  print("");
  print("Usage of Faster2Histo : /path/to/data /path/to/output [[parameters]]");
  print("");
  print("parameters :");
  print("");
  // print("-c [calibration_file]  : Loads the calibration file");
  // print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  // print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-f [file_name]         : add a new file");
  print("-F [folder_name]       : add a new folder");
  // print("-i [ID_file]           : Load ID file (necessary if trigger is used)");
  print("-n [hits_number]       : Choose the number of hits to read inside a file");
  print("-m [threads_number]    : Choose the number of files to treat in parallel");
  print("-o                     : Overwrite the output");
  // print("-t [time_window_ns]    : Loads timeshift data");
  // print("--throw-single         : If you are not interested in single hits");
  // print("--trigger [filename]   : Load a trigger file (look at documentation)");
  exit(1);
}

Faster2Histo::Faster2Histo(int argc, char** argv)
{
  if (argc<2)
  {
    print("Not enough parameters !!!");
    printParameters();
  }

  // Other parameters :
  for (int i = 3; i<argc; i++)
  {
    std::string command = argv[i];
        //  if (command == "-c") loadCalibration(argv[++i]); // TBD
        //  if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    // else if (command == "-i") detectors.load(argv[++i]); // A voir
         if (command == "-m") setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-o") overwrite(true);
    else if (command == "-f") // TBD
    // else if (command == "--trigger") loadTriggerFile(argv[++i]); // TBD
    else {throw_error("Unkown command" + command);}
  }
}

void Faster2Histo::multirun(std::string const & outfilename, int const & nb_threads)
{

  if (!m_overwrite && File(outfilename).exists()) {print(outfilename, "already exists ! Use option -o or method Faster2Histo::overwrite(true)"); return;}
  MTObject::Initialize(nb_threads);
  m_reader.readRaw(dispatch_threads, *this);
}

void Faster2Histo::treatFile(Hit & _hit, FasterReader & reader)
{
  Timer timer;
  while(reader.Read())
  {
    auto const & hit = _hit;
    try {
      m_spectra.at(hit.label).Fill(hit.adc);
    }
    catch(std::out_of_range const & error)
    {// Add a new detector label to the map
      m_labels.push_back(hit.label);
      m_spectra.emplace(hit.label, MTTHist<TH1F>(std::to_string(hit.label), (std::to_string(hit.label)+" adc").c_str(), 200000, 0, 2000000));
      m_spectra[hit.label].Fill(hit.adc);
    }
  }
  print(timer(), timer.unit(), File(reader.getFilename()).size("Mo")/timer.TimeSec(), "Mo/s");
}

void Faster2Histo::write(std::string const & out_filename)
{
  if (out_filename == "") {if (m_outFilename.get() == "") m_outFilename = "spectra.root";}
  else m_outFilename = out_filename;

  auto outFile = TFile::Open(m_outFilename.c_str(), write_mode);
  outFile->cd();
  // Ordering of the unordered_map keys :
  auto ordered_labels_indexes = bubbleSort(m_labels);
  // Write down with ordered labels :
  for (auto & label_index : ordered_labels_indexes) 
  {
    m_spectra[m_labels[label_index]].verbosity();
    m_spectra[m_labels[label_index]].Write();
  }
  outFile->Write();
  outFile->Close();
  print(m_outFilename, "written");
}

#endif // FASTER2HISTO_HPP