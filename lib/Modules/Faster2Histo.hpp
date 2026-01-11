#ifndef FASTER2HISTO_HPP
#define FASTER2HISTO_HPP

#include "../MTObjects/MTObject.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/CalibAndScale.hpp"
#include "../Classes/Timeshifts.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MultiHist.hpp"
#include "../MTObjects/MTList.hpp"

#define QDC1MAX

/**
 * @brief Convert faster data to histograms.
 * 
 * @details
 * 
 * @todo
 * event building, timeshifts
 * 
 */
class Faster2Histo
{
public:
  Faster2Histo() {TH1::AddDirectory(kFALSE);}
  /// @brief To launch the application at instanciation
  Faster2Histo(int argc, char** argv);
  ~Faster2Histo(){print(m_timer());}
  /// @brief To launch the application after instanciation
  bool load(int const & argc, char** argv);
  
  /// @brief Adds [nb_files] files to convert inside of [folder]
  void addFolder(Path folder, int const & nb_files = -1) 
  {
    m_files.addFolder(folder, nb_files);
    if (m_outFile == File("spectra.root")) // If the output name is still by default
    {
      auto path = folder.string(); path.pop_back();
      setOutFilename(rmPathAndExt(path)+"_spectra.root");
    }
  }

  /// @brief @todo UNUSED until event building is done
  void setTrigger(HitTrigger trigger) {m_trigger = trigger;}
  
  /////////
  // Run //
  /////////
  
  /// @brief Setup the threads and launched the application
  void multirun(int const & nb_threads = -1);
  /// @brief Treats the file with already configured Hit and FasterReader object
  void treatFile(Hit & hit, FasterReader & reader);
  /// @brief Initialize the histogram corresponding to a given detector's label
  void initializeHisto(Label const & label);
  /// @brief Handles each hit and fills the correct histogram
  void fillHisto(Hit const & hit);
  /// @brief Write down a [out_filename].root file containing the spectra of the data
  void write(std::string const & out_filename = "") noexcept;

  ///////////////
  // Interface //
  ///////////////

  /// @brief Adds one file to convert at the list. Option -f
  void addFile(std::string const & file) 
  {
    m_files.addFile(file);
    setOutFilename(rmPathAndExt(file)+"_spectra.root");
  }
  /// @brief Set the name of the output file
  void setOutFilename(std::string const & out_filename) noexcept {m_outFile = out_filename;}
  /// @brief Interface with the ROOT write mode. Default = "RECREATE". 
  /// @details If you want to use "APPEND" for instance, you have to use the overwrite option (-o)
  void setWriteMode(const char* mode) noexcept {delete[] m_write_mode; m_write_mode = mode;}
  /// @brief Loads the .align file needed. Option -a
  void loadAlignement(std::string const & alignement_file) {m_alignement.load(alignement_file);}
  /// @brief Loads the .calib file needed. Option -c
  void loadCalibration(std::string const & calibration_file) {m_calibration.load(calibration_file);}
  /// @brief Loads the .dT file needed. Option -t
  void loadTimeshifts(std::string const & timeshifts_filename) {m_timeshifts.load(timeshifts_filename);}
  /// @brief Set the number of files to read in the folder. Second argument of option -F
  void setNbFiles(int const & _nb_files) noexcept{m_nb_files = _nb_files;}
  /// @brief Set the number of threads to write down. Option -m
  void setNbThreads(int const & _nb_threads) noexcept {MTObject::setThreadsNb(_nb_threads);}
  /// @brief Set the full output path. Option --out_path
  void setOutPath(Path const & path) noexcept {m_outPath = path; m_outPath.make();}
  /// @brief Allows to overwrite the output file if it exists. Option -o
  void overwrite(bool const & _overwrite) noexcept {m_overwrite = _overwrite;}

  /////////////
  // Getters //
  /////////////

  bool const & ok() const {return m_ok;}
  operator bool() const & {return this->ok();}

private:
  bool m_ok = false;
  Timer m_timer;
  void printParameters() const noexcept;
  static void dispatch_threads(Hit & hit, FasterReader & reader, Faster2Histo & f2h) {f2h.treatFile(hit, reader);}
  void fill(Hit const & hit) {m_spectra.at(hit.label).Fill((m_calibration) ? hit.nrj : hit.adc);}
  MTFasterReader m_MTreader;
  FilesManager m_files;
  std::vector<Label> m_labels;
  Map_MultiHist<TH1F, Label> m_spectra;
  Map_MultiHist<TH2F, Label> m_bidim;
  const char* m_write_mode = "RECREATE";
  CalibAndScale m_alignement;
  Calibration m_calibration;
  Timeshifts m_timeshifts;
  HitTrigger m_trigger = [](Hit const & hit) {return true;};
  
  Path m_outPath = Path::pwd();
  File m_outFile = "spectra.root";
  int m_nb_files = -1;
  int m_overwrite = false;
  int m_maximum_hits = -1;

  int m_nb_bins = -1;
  double m_bin_max = -1;

  bool m_bidim_paris = false;
  int m_nb_bins_paris = -1;
  double m_bin_max_paris = -1;
};

Faster2Histo::Faster2Histo(int argc, char** argv)
{
  TH1::AddDirectory(kFALSE);
  if (this -> load(argc, argv))
  {
    this -> multirun();
    this -> write();
  }
}

void Faster2Histo::printParameters() const noexcept
{
  print("");
  print("Usage of Faster2Histo : [[parameters]]");
  print("Note : The output file will be written in the folder from which the executable is run");
  print("");
  print("parameters :");
  print("");
  print("-a [alignement_file]        : Loads the run-bu-run calibration correction file");
  print("-c [calibration_file]       : Loads the calibration file");
  print("-f [file_name]              : add a new .fast file");
  print("-F [folder_name] [nb_files] : add a new folder with a certain amount of .fast files (-1 to take all of them)");
  print("-i [ID_file]                : Load detector's ID file and set the name of the histograms accordingly");
  print("-n [hits_number]            : Choose the number of hits to read inside a file (default: all)");
  print("-m [threads_number]         : Choose the number of files to treat in multithread mode (default: 1)");
  print("-O [outputFile]             : Set the name of the output file (default: \"spectra.root\")");
  print("-o                          : Overwrite the output (default: no)");
  print("-t                          : Loads the timeshifts (default : no timeshits)");
  print("--out_path [output_path]    : Set the output path (default ./)");
  print("--bins     [nb_bins]        : Set the number of bins (default: 50000)");
  print("--bin_max  [bin_max]        : Set the maximum bin of the spectra (default: 50000)");
  print("--paris_bidim               : For paris, do QDC1 VS QDC2 bidim.\n"
        "                              Requires index file loaded and \"PARIS\" in the name of the paris detectors.\n"
        "                              ATTENTION : make sure you have enough RAM, especially if using multithreading !!!");
  print("--paris_bins    [nb_bins]   : For paris, set the number of bins on both axis of the bidim (default: 1000)");
  print("--paris_bin_max [bin_max]   : For paris, set the maximum bin on both axis of the bidim (default : 3000 if calibrated, else 2e5)");
  exit(1);
}

bool Faster2Histo::load(int const & argc, char** argv)
{
  if (argc<2)
  {
    print("Not enough parameters !!!");
    printParameters();
  }

  for (int i = 1; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-a") {loadAlignement (argv[++i]);}
    else if (command == "-c") {loadCalibration(argv[++i]);}
    else if (command == "-f") {this -> addFile(argv[++i]);}
    else if (command == "-F") 
    {
      auto const & folder = argv[++i];
      auto const & nb_files = std::stoi(argv[++i]);
      this -> addFolder(folder, nb_files);
    }
    else if (command == "-i") {detectors.load(argv[++i]);}
    else if (command == "-m") {this -> setNbThreads(std::stoi(argv[++i]));}
    else if (command == "-n") {FasterReader::setMaxHits(std::stoi(argv[++i]));}
    else if (command == "-N") {m_maximum_hits = std::stoi(argv[++i]);}
    else if (command == "-O") {this -> setOutFilename(argv[++i]);}
    else if (command == "-o") {this -> overwrite(true);}
    else if (command == "-t") loadTimeshifts(argv[++i]);
    else if (command == "--out_path") {this -> setOutPath(argv[++i]);}
    else if (command == "--bins")     {m_nb_bins = std::stoi(argv[++i]);}
    else if (command == "--bin_max")  {m_bin_max = std::stod(argv[++i]);}
    else if (command == "--paris_bidim") { m_bidim_paris = true;}
    else if (command == "--paris_bins")    {m_nb_bins_paris = std::stoi(argv[++i]);}
    else if (command == "--paris_bin_max") {m_bin_max_paris = std::stod(argv[++i]);} 
    else {Colib::throw_error("Unkown command " + command);}
  }

  // Checking the number of files to treat :
  if (!m_files)
  {
    error("NO FILE TO READ !!!");
    return (m_ok = false);
  }
  // Checking the output filename is not already used :
  if (!m_overwrite && File(m_outFile).exists()) 
  {
    throw OverwriteError(m_outFile.string()+" already exists ! Use option -o or method Faster2Histo::overwrite(true)");
  }
  return (m_ok = true);
}

inline void Faster2Histo::multirun(int const & nb_threads)
{
  // Initialise the multithreading. Either initialise with previously set number or 1 if not, or with nb_threads
  (nb_threads<0) ? MTObject::Initialise() : MTObject::Initialise(nb_threads);

  // Set the files to the reader
  m_MTreader.files() = m_files;

  // Dispatch Faster2Histo::treatFile() in each thread, i.e. in each file being processed in parallel
  m_MTreader.readRaw(dispatch_threads, *this);
}

void Faster2Histo::treatFile(Hit & _hit, FasterReader & reader)
{
  Timer timer;
  // If the calibration loaded then calibrate the hits :
  if (m_calibration) 
  {
    if (m_alignement) while(reader.Read()) // With calibration and alignement
    { // Loop through the hits of the data file
      m_calibration(_hit);
      _hit.nrj = m_alignement.linear_inv_calib(_hit.nrj);
      fillHisto(_hit);
    }
    else while(reader.Read()) // With calibration
    { // Loop through the hits of the data file
      m_calibration(_hit);
      fillHisto(_hit);
    }
  }
  else while(reader.Read()) fillHisto(_hit); // With no calibration
  
  print(timer(), File(reader.getFilename()).size("Mo")/timer.TimeSec(), "Mo/s");// Print performances
}

inline void Faster2Histo::initializeHisto(Label const & label)
{
  lock_mutex lock(MTObject::mutex);

  // Check if another thread did not already create it between the try and the catch :
  if (!found(m_labels, label))
  {
    std::string name = (detectors) ? detectors[label] : std::to_string(label);
    if (name == "") return;

    m_labels.push_back(label);
    std::string title = (detectors) ? detectors[label] : std::to_string(label);
    if (m_calibration) 
    {
      title += " E";
      if (m_nb_bins<0) m_nb_bins = 50000;
      if (m_bin_max<0) m_bin_max = 50000;
    }
    else
    {
      title += " adc";
      if (m_nb_bins<0) m_nb_bins = int_cast(1e+6);
      if (m_bin_max<0) m_bin_max = 2e+7;
    }

    emplaceMultiHist(m_spectra, label, name.c_str(), title.c_str(), m_nb_bins, 0, m_bin_max);

    if (m_bidim_paris && detectors)
    {
      if (m_nb_bins_paris<0) m_nb_bins_paris = 1000;
      if (m_bin_max_paris<0) m_bin_max_paris = (m_calibration) ? 3000 : 2.e+5;
      if (found(detectors[label], "PARIS"))
      {
        auto const & name_bidim  = name +"_bidim";
        auto const & title_bidim = title+" Q_long VS Q_short;Q_short [ADC];Q_long [ADC]";
        MultiHist<TH2F> bidim(name_bidim.c_str(), title_bidim.c_str(), 
        m_nb_bins_paris,0,m_bin_max_paris, m_nb_bins_paris,0,m_bin_max_paris);
        m_bidim.emplace(label, std::move(bidim)); // To change with addToMap
      }
    }
  }
}

inline void Faster2Histo::fillHisto(Hit const & hit)
{
  // m_spectra[hit.label].Fill(hit.nrj);
  try
  { // Try to fill the histogram
    this->fill(hit);
  }
  catch(std::out_of_range const & error)
  { // If no histogram is found, create a new one
    initializeHisto(hit.label);
    this->fill(hit);
  }

  // if (m_bidim_paris && detectors && found(detectors[hit.label], "PARIS"))
  // {
  //   if (m_calibration) m_bidim[hit.label].Fill(hit.nrj, hit.nrj2);
  //   else               m_bidim[hit.label].Fill(hit.adc, hit.qdc2);
  // }
}

void Faster2Histo::write(std::string const & out_filename) noexcept
{
  if (out_filename != "") m_outFile = out_filename;
  print("-----------------");
  // File file(m_outPath.string()+m_outFile.filename().string());
  print("-----------------");
  if (!m_outFile.path()) m_outFile.makePath();
  m_outFile.setExtension("root");

  auto outFile = TFile::Open(m_outFile.c_str(), m_write_mode);
  outFile->cd();
  // Ordering of the unordered_map keys (stored in m_labels):
  auto ordered_labels_indexes = bubble_sort(m_labels);
  // Write down with ordered labels :
  for (auto & label_index : ordered_labels_indexes) m_spectra[m_labels[label_index]].Write();
  if (m_bidim_paris) for (auto & label_index : ordered_labels_indexes) m_bidim[m_labels[label_index]].Write();
  // outFile->Write();
  outFile->Close();
  print(m_outFile, "written");
}

#endif // FASTER2HISTO_HPP
// ./faster2histo -c ../136/136_2024_corrected.calib -t ../136_run.dT -F ~/nuball2/N-SI-136/run_75.fast/ -1 -O to_throw.root -i parameters/index_129.list --bins 20000 --bin_max 20000 -m 3 -o