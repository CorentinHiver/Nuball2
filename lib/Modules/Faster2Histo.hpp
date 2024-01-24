#ifndef FASTER2HISTO_HPP
#define FASTER2HISTO_HPP

#include "../Classes/Detectors.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/Calibration.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"
#include "../MTObjects/MTList.hpp"

#define QDC1MAX

class Faster2Histo
{
public:
  Faster2Histo() {}
  Faster2Histo(int argc, char** argv);
  void load(int const & argc, char** argv);
  
  void addFolder(Path folder, int const & nb_files = -1) 
  {
    m_files.addFolder(folder, nb_files);
    if (m_outFile == File("spectra.root")) // If the output name is still by default
    {
      auto path = folder.string(); path.pop_back();
      setOutFilename(rmPathAndExt(path)+"_spectra.root");
    }
  }
  
  void addFile(std::string const & file) 
  {
    m_files.addFile(file);
    setOutFilename(rmPathAndExt(file)+"_spectra.root");
  }

  void setTrigger(TriggerHit trigger) {m_trigger = trigger;}

  void multirun(int const & nb_threads = -1);
  void treatFile(Hit & hit, FasterReader & reader);
  void fillHisto(Hit const & hit);
  void write(std::string const & out_filename = "") noexcept;

  void setOutFilename(std::string const & out_filename) noexcept {m_outFile = out_filename;}
  void setWriteMode(const char* mode) noexcept {delete[] write_mode; write_mode = mode;}
  void loadCalibration(std::string const & calibration_file) {m_calibration.load(calibration_file);}
  void setNbFiles(int const & _nb_files) noexcept{m_nb_files = _nb_files;}
  void setNbThreads(int const & _nb_threads) {MTObject::setThreadsNb(_nb_threads);}
  void setOutPath(Path const & path) noexcept {m_outPath = path;}
  void overwrite(bool const & _overwrite) noexcept {m_overwrite = _overwrite;}

private:
  void printParameters() const noexcept;
  static void dispatch_threads(Hit & hit, FasterReader & reader, Faster2Histo & f2h) {f2h.treatFile(hit, reader);}
  MTFasterReader m_MTreader;
  FilesManager m_files;
  std::vector<Label> m_labels;
  std::unordered_map<Label, MTTHist<TH1F>> m_spectra;
  const char* write_mode = "RECREATE";
  Calibration m_calibration;
  TriggerHit m_trigger = [](Hit const & hit) {return true;};
  
  Path m_outPath = Path::pwd();
  File m_outFile = "spectra.root";
  int m_nb_files = -1;
  int m_overwrite = false;

  int m_nb_bins = 2000000;
  double m_ADC_per_bin = 100;
};

Faster2Histo::Faster2Histo(int argc, char** argv)
{
  this -> load(argc, argv);
  this -> multirun();
  this -> write();
}

void Faster2Histo::printParameters() const noexcept
{
  print("");
  print("Usage of Faster2Histo : [[parameters]]");
  print("Note : The output file will be written in the folder from which the executable is run");
  print("");
  print("parameters :");
  print("");
  print("-c [calibration_file]  : Loads the calibration file");
  // print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  // print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-f [file_name]              : add a new .fast file");
  print("-F [folder_name] [nb_files] : add a new folder with a certain amount of .fast files (-1 to take all of them)");
  print("-i [ID_file]                : Load ID file and set the name of the histograms accordingly");
  print("-n [hits_number]            : Choose the number of hits to read inside a file");
  print("-m [threads_number]         : Choose the number of files to treat in parallel");
  print("-O [outputFile]             : Set the name of the output file");
  print("-o                          : Overwrite the output");
  print("--out_path [output_path]    : Set the output path");
  // print("-t [time_window_ns]    : Loads timeshift data");
  // print("--throw-single         : If you are not interested in single hits");
  // print("--trigger [filename]   : Load a trigger file (look at documentation)");
  exit(1);
}

void Faster2Histo::load(int const & argc, char** argv)
{
  if (argc<2)
  {
    print("Not enough parameters !!!");
    printParameters();
  }

  // Other parameters :
  for (int i = 1; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-f") {this -> addFile(argv[++i]);}
    else if (command == "-F") 
    {
      auto const & folder = argv[++i];
      auto const & nb_files = std::stoi(argv[++i]);
      this -> addFolder(folder, nb_files);
    }
    else if (command == "-i") detectors.load(argv[++i]);
    else if (command == "-m") this -> setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-O") this -> setOutFilename(argv[++i]);
    else if (command == "-o") this -> overwrite(true);
    else if (command == "--out_path") this -> setOutPath(argv[++i]);
    else {throw_error("Unkown command " + command);}
  }

  // Checking the number of files to treat :
  if (m_files.size()<1) throw_error("NO FILE TO READ !!!");
  // Checking the output filename is not already used :
  if (!m_overwrite && File(m_outFile).exists()) {throw_error(m_outFile.string()+" already exists ! Use option -o or method Faster2Histo::overwrite(true)");} 

}

inline void Faster2Histo::multirun(int const & nb_threads)
{
  // Initialise the multithreading. Either initialise with previously set number or 1 if not, or with nb_threads
  (nb_threads<0) ? MTObject::Initialize() : MTObject::Initialize(nb_threads);

  // Set the files to the reader
  m_MTreader.files() = m_files;
  
  // Dispatch Faster2Histo::treatFile() in each thread, i.e. in each file being processed in parallel
  m_MTreader.readRaw(dispatch_threads, *this);
}



void Faster2Histo::treatFile(Hit & _hit, FasterReader & reader)
{
  Timer timer;
  // If the calibration loaded then calibrate the hits :
  if (m_calibration) while(reader.Read())
  {// Loop through the hits of the .fast file
    // The following has been added to calibrate faster (the classical way uses a random generator to calibrate)
    auto const & slope = m_calibration.slope(_hit.label);
    auto const & intercept = m_calibration.intercept(_hit.label);
    _hit.nrj = (_hit.adc+random_uniform())*slope + intercept;
    // m_calibration(_hit);
    fillHisto(_hit);
  }
  else while(reader.Read()) fillHisto(_hit); // With no calibration
  
  print(timer(), timer.unit(), File(reader.getFilename()).size("Mo")/timer.TimeSec(), "Mo/s");// Print performances
}

inline void Faster2Histo::fillHisto(Hit const & hit)
{
  try {
      m_spectra.at(hit.label).Fill((m_calibration) ? hit.nrj : hit.adc); // Fill the spectra
    }
    catch(std::out_of_range const & error)
    {// Add a new detector label to the map if it doesn't exist
      lock_mutex lock(MTObject::mutex);
      if (!found(m_labels, hit.label))// Check if another thread did not already created it
      {
        m_labels.push_back(hit.label);
        std::string name = (detectors) ? detectors[hit.label] : std::to_string(hit.label);
        std::string title = (detectors) ? detectors[hit.label] : std::to_string(hit.label);
        title += (m_calibration) ? " E" : " adc";
        if (m_calibration) 
        {
          m_nb_bins = 1000000;
          m_ADC_per_bin = 0.1;
        }
        m_spectra.emplace(hit.label, MTTHist<TH1F>(name.c_str(), title.c_str(), m_nb_bins, 0, m_nb_bins*m_ADC_per_bin));
        m_spectra[hit.label].Fill(hit.adc);
      }
    }
}

void Faster2Histo::write(std::string const & out_filename) noexcept
{
  if (out_filename != "") m_outFile = out_filename;
  File file(m_outFile);
  if (!file.path())file.makePath();

  auto outFile = TFile::Open(m_outFile.c_str(), write_mode);
  outFile->cd();
  // Ordering of the unordered_map keys :
  auto ordered_labels_indexes = bubble_sort(m_labels);
  // Write down with ordered labels :
  for (auto & label_index : ordered_labels_indexes) 
  {
    auto & spectra = m_spectra[m_labels[label_index]];
    // spectra.Merge();
    // spectra->GetXaxis()->SetRange(0, spectra->FindLastBinAbove(1));
    spectra.Write();
  }
  outFile->Write();
  outFile->Close();
  print(m_outFile, "written");
}

#endif // FASTER2HISTO_HPP