#ifndef CLOVERSPECTRA_HPP
#define CLOVERSPECTRA_HPP

#include "../Analyse/Clovers.hpp"
#include "../Classes/CoincBuilder.hpp"
#include "Timeshifts.hpp"
#include "Calibration.hpp"

/**
 * @brief Make usefull calibrated spectra from data in .fast or .root format
 */
class CloverSpectra
{
public:
  CloverSpectra() = default;
  CloverSpectra(int argc, char** argv) {this -> Initialize(argc, argv);}
  void Initialize();
  void Initialize(int argc, char** argv);
  void printParameters();

  void loadTimeshifts(std::string const & dTfile)   {m_timeshifts.load(dTfile);}
  void loadCalibration(std::string const & calibFile)  {m_calibration.load(calibFile);}

  void setCalibration(Calibration const & calibration) {m_calibration = calibration;}
  void setTimeshifts(Timeshifts const & timeshifts) {m_timeshifts = timeshifts;}

  void setDataPath(std::string const & datapath) {m_dataPath = datapath;}
  void setOutName(std::string const & outname) {m_writeHisto = true; m_outName = outname;}
  void setNbFiles(int const & nb_files = -1) {m_nb_files = nb_files;}
  void setNbThreads(int const & nb_threads = -1) {m_nb_threads = nb_threads;}
  void setTimeWindow_ns(int const & timewindow = 1500) {m_timewindow = timewindow*1000;}

  void fillHisto(std::string const & datapath = "", int nb_files = -1);
  void writeHisto(std::string const & outName = "");
  // void loadHisto(File const & histoFile, std::string const & histoName);// TBD

private:
  void runFasterReader(Hit & hit, Alignator & tree);
  static void runMTFasterReader(Hit & hit, Alignator & tree, CloverSpectra & ce)
  {
    ce.runFasterReader(hit, tree);
  }

  Timeshifts m_timeshifts;
  Calibration m_calibration;

  Time m_timewindow = 100000;
  char dataKind = -1; // 0: faster 1: root not aligned 2: root aligned
  int m_nb_files = -1;
  int m_nb_threads = 1;
  bool m_throw_single = false;
  bool m_writeHisto = false;

  MTTHist<TH1F> m_clovers;
  MTTHist<TH1F> m_clovers_crystal;
  MTTHist<TH2F> m_clovers_bidim;
  // MTTHist<TH1F> m_paris;
  // MTTHist<TH2F> m_paris_bidim;
  // MTTHist<TH2F> m_paris_VS_clovers;

  Path m_dataPath;
  File m_outName;
};

inline void CloverSpectra::fillHisto(std::string const & datapath, int nb_files)
{
  if (datapath != "") m_dataPath = datapath;
  if (nb_files>0) m_nb_files = nb_files;
  if (!m_timeshifts) throw_error("no timeshifts !!");
  if (!m_calibration) throw_error("no calibration !!");
  if (m_dataPath.string().find(".fast/")!=std::string::npos)
  {
    dataKind = 0;
    MTFasterReader reader(m_dataPath, m_nb_files);
    reader.readAligned(m_timeshifts.get(), runMTFasterReader, *this);
  }
}

inline void CloverSpectra::writeHisto(std::string const & outName)
{
  if (outName!="") m_outName = outName;
  auto file = TFile::Open(m_outName.c_str(), "recreate");
  if (!file) throw_error("Output file" + m_outName + std::string("cannot be created"));
  file -> cd();
  m_clovers.Write();
  m_clovers_crystal.Write();
  m_clovers_bidim.Write();
  // m_paris.Write();
  // m_paris_bidim.Write();
  // m_paris_VS_clovers.Write();
  file->Write();
  file -> Close();
  delete file;
  print(m_outName, "written");
}

inline void CloverSpectra::printParameters()
{
  print("");
  print("Usage of CloverSpectra : /path/to/data outputFile [[parameters]]");
  print("");
  print("parameters :");
  print("");
  print("-c [calibration_file]  : Loads the calibration file");
  print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-i [ID_file]           : Load ID file");
  print("-n [hits_number]       : Choose the number of hits to read inside each file");
  print("-m [threads_number]    : Choose the number of files to treat in parallel");
  print("-t [time_window_ns]    : Loads timeshift data");
  print("--throw-singles        : Throw the single hits, i.e. no coincidence");
}

inline void CloverSpectra::Initialize(int argc, char** argv)
{
  if (argc<2) 
  {
    print("Not enough parameters !!!");
    printParameters();
    throw_error("Parameters");
  }

  setDataPath(argv[1]);

  for (int i = 2; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-e") setTimeWindow_ns(std::stoi(argv[++i]));
    else if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    else if (command == "-i") detectors.load(argv[++i]);
    else if (command == "-m") setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-t") loadTimeshifts(argv[++i]);
    else if (command == "--throw-singles") m_throw_single = true;
    else if (command == "-w") setOutName(argv[++i]);
    else {print("Unkown command", command);}
  }

  this -> Initialize();

  this -> fillHisto();

  if (m_writeHisto) this -> writeHisto();
}

inline void CloverSpectra::Initialize()
{
  if (m_nb_threads > 1) MTObject::Initialize(m_nb_threads);
  if (!m_throw_single) Builder::keepSingles();
  Clovers::timePs(true);

  m_clovers.reset("Clovers", "Clovers", 2000, 0, 2000);
  m_clovers_crystal.reset("Clovers crystals", "Clovers crystals", 2000, 0, 2000);
  m_clovers_bidim.reset("Clovers_bidim", "Clovers gamma-gamma", 2000,0,2000, 2000,0,2000);
  // m_paris.reset("Paris", "Paris", 2000, 0, 2000);
  // m_paris_bidim.reset("Paris_bidim", "Paris gamma-gamma", 2000,0,2000, 2000,0,2000);
  // m_paris_VS_clovers.reset("Paris VS Clovers", "Paris VS Clovers", 2000,0,2000, 2000,0,2000);
}

inline void CloverSpectra::runFasterReader(Hit &hit, Alignator &tree)
{
  // Clover only
  Event cloversEvent;
  CoincBuilder cloversEventBuilder(&cloversEvent, m_timewindow);
  Clovers clovers;

  // Paris only
  // Event cloversEvent;
  // CoincBuilder cloversEventBuilder(&cloversEvent, m_timewindow);
  // Paris paris;

  // Whole Calorimeter 
  // Event cloversEvent;
  // CoincBuilder cloversEventBuilder(&cloversEvent, m_timewindow);
  // Clovers cloversNuball;
  // Paris parisNuball;

  auto const & nb_hits = tree->GetEntries();
  for (int hit_i = 0; hit_i<nb_hits; hit_i++)
  {
    tree.GetEntry(hit_i);
    m_calibration(hit);
    if (isClover[hit.label])
    {
      if (isGe[hit.label] && hit.nrj>5) m_clovers_crystal.Fill(hit.nrj);
      if (cloversEventBuilder.build(hit))
      {
        clovers.SetEvent(hit);
        for (size_t loop_i = 0; loop_i<clovers.Ge.size(); loop_i++)
        {
          auto const & clover_i = clovers.Ge[loop_i];
          auto const & clover = clovers[clover_i];
          m_clovers.Fill(clover.nrj);
          if (clovers.Ge.size()>1) print(clovers.Ge.size());
          for (size_t loop_j = loop_i+1; loop_j<clovers.Ge.size(); loop_j++)
          {
            m_clovers_bidim.Fill(clover.nrj, clovers[clovers.Ge[loop_j]].nrj);
          }
        }
      }
    }
  }
  // m_paris.Fill(clover.nrj);
  // m_paris_bidim.Fill(clover.nrj);
  // m_paris_VS_clovers.Fill(clover.nrj);
}

#endif //CLOVERSPECTRA_HPP