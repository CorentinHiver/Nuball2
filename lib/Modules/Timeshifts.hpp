#ifndef TIMESHIFTS_H
#define TIMESHIFTS_H

#include "../libCo.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/FasterReader.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/Hit.hpp"
#include "../Classes/RF_Manager.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/CoincBuilder2.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/Performances.hpp"
#include "../Classes/DetectorsList.hpp"

#include "../MTObjects/MTObject.hpp"
#include "../MTObjects/MTList.hpp"
#include "../MTObjects/MTTHist.hpp"

/* 
 * Author : Corentin Hiver
 * Module part of the NearLine software
 * The timestamps are in picoseconds, so are the timeshifts
 * Goal 1 : calculate the time shifts between detectors
 * How : 
 * * Use one detector as reference, preferencially with good time resolution
 * * Create coincidences using the CoincBuilder event builder
 * * Calculate the difference in timestamp as follow : ref_time - detector_time
 * * Finally, output the (.dT) file containing the time shifts of all the detectors
 * * Also produces a .root file containing the raw and corrected time spectra.
 * * This module includes multithreading management using MTObject::parallelise_function(function, parameters...)
 * Goal 2 : read a timeshift file (.dT) previously calculated by this module
 * Goal 3 : apply the timeshifts by calling the operator[] : time_correct = time_shifted + timeshifts[label];
 * Example at the end
*/

using Shift_t = Long64_t;

class Timeshifts
{
public:

  Timeshifts() {Detectors::Initialize();}

  /**
   * @brief Call the Timeshifts::load() method to load the timeshifts from a .dT file 
  */
  Timeshifts(std::string const & filename) : m_filename(filename) {this -> load(filename);}

  /**
   * @brief For faster to root conversion only : Timeshifts::load() loads the timeshifts from a .dT file 
   * @param path: The path of the out root directory (/path/to/run_name)
   * @param name: The name of the .dT file without the extension (e.g. run_10)
   * @details Will read a file in /path/Timeshitfs/name.dT
  */
  Timeshifts(std::string const & path, std::string const & name) : m_filename(path+"Timeshifts/"+name+".dT") {this -> load(m_filename);}

  /**
   * @brief Copy constructor
  */
  Timeshifts(Timeshifts const & timeshifts) : m_timeshifts(timeshifts.m_timeshifts) {}

  /**
   * @brief Use this method to setup the parameters from a string in order to calculate the timeshifts
  */
  bool setParameters(std::string const & parameter);

  /**
   * @brief Use this method to load timeshifts from a .dT file
  */
  bool load(std::string const & filename);

  /**
   * @brief Set the detector list
   * @note Mandatory only if calculate timeshifts
  */
  void setDetectorsList(DetectorsList const & detList) {m_detList = detList;}

  bool setTimeWindow(int const & timewindow_ns);

  /**
   * @brief Set the time window in ns
   * @note It is better to keep it at its default value, otherwise you can miss some detectors
   * 
   * @param timewindow_ns: default value 1500ns. 
  */
  bool setTimeWindow(std::string const & timewindow_ns);

  /**
   * @brief Set the time reference label
   * @note Mandatory only if calculate timeshifts
  */
  bool setTimeReference(int const & timeRef_label);

  /**
   * @brief Set the time reference.
   * @note Mandatory only if calculate timeshifts
   * 
   * Either the full name, or the label number
   * 
  */
  bool setTimeReference(std::string const & timeRef_name);

  /**
   * @brief Set the output directory (full path);
   * @note Mandatory only if calculate timeshifts
  */
  void setOutDir(std::string const & outDir);

  /**
   * @brief Set verbosity level. 
   * @note Mandatory only if calculate timeshifts
   * 
   * It will print the fit parameters in the terminal
   * 
  */
  void verbose(bool const & _verbose = true) {m_verbose = _verbose;}
  
  /**
   * @brief Set output root file containing the raw and corrected time spectra
   * @note Mandatory only if calculate timeshifts
  */
  void setOutRoot(std::string const & outroot);

  /**
   * @brief Set output file name of the timeshifts data.
   * @note Mandatory only if calculate timeshifts
   * 
   * If not set, it is automatically named after the root file.
  */
  void setOutData(std::string const & outdata);

  /**
   * @brief Set the multiplicity gate for the events used for coincidence timeshift calculation
   * @note Mandatory only if calculate timeshifts
   * 
   * min_mult and max_mult included.
   * 
   * example : multiplicity = 2 : setMult(2, 2)
   * 
   * example : multiplicity = {2;3;4} : setMult(2, 4)
   * 
   * @param min_mult: default min_mult = 2
   * @param max_mult: default max_mult = 2
   * 
  */
  void setMult(uchar const & min_mult, uchar const & max_mult) {m_min_mult = min_mult; m_max_mult = max_mult;}

  /**
   * @brief Set maximum hits to be read in a file
   * @note Use only if calculate timeshifts
  */
  void setMaxHits(ulong const & max_hits = -1) {m_max_hits = max_hits;}

  /**
   * @brief Set RF shift (synonymous to Timeshifts::setRFOffset)
   * @note Use only if calculating timeshifts with RF
   * @param shift: default 50 ns
  */
  void setRFShift(Long64_t const & shift) {RF_Manager::set_offset(shift);}

  /**
   * @brief Set RF offset (synonymous to Timeshifts::setRFShift)
   * @note Use only if calculating timeshifts with RF
   * @param offset: default 50 ns
  */
  void setRFOffset(Long64_t const & shift) {RF_Manager::set_offset(shift);}

  bool calculate(std::string const & folder, int const & nb_files = -1);
  void Print() {print(m_timeshifts);}

  TH1F* shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit = "ps");

  Shift_t const & operator[] (int const & i) const {return m_timeshifts[i];}
  operator bool() const & {return m_ok;}

  std::vector<Shift_t> const & get() const {return m_timeshifts;}
  Shift_t const & get(int const & i) const {return m_timeshifts[i];}

private:

  static void treatFilesMT(Timeshifts & ts, MTList<std::string> & files_MT);
  void treatFile(std::string const & filename);
  void treatRootFile(std::string const & filename);
  void treatFasterFile(std::string const & filename);
  void Fill(Event const & event);
  void analyse();
  void write();

  bool m_ok = false;
  bool m_initialized = false;
  bool m_verbose = false;

  DetectorsList m_detList;
  FilesManager m_files;
  Performances m_perf;

  std::string m_filename = "";

  std::vector<Shift_t> m_timeshifts;

  float m_timewindow = 1500000;
  float m_timewindow_ns = 1500;
  uchar m_max_mult = 2;
  uchar m_min_mult = 2;
  ushort m_timeRef_label = 252;
  std::string m_timeRef_name = "R1A9_FATIMA_LaBr3";
  ulong m_max_hits = -1;

  bool InitParameters();

  // This map holds the bin width of the histograms in ps (e.g. for LaBr3 there is one bin every 100 ps)
  std::map<std::string, float> m_rebin = { {"LaBr3", 100}, {"Ge", 1000}, {"BGO", 500}, {"EDEN", 500}, {"RF", 100}, {"paris", 100}, {"dssd", 1000}};

  std::string m_outDir = "";
  std::string m_ts_outdir = "Timeshifts/";
  std::string m_outPath;
  std::string m_outRoot = "timeshifts.root";
  std::string m_outData = "";


  Vector_MTTHist<TH1F> m_histograms;
  std::vector<std::unique_ptr<TH1F>> m_histograms_corrected;
#ifdef USE_RF
  MTTHist<TH1F> m_histo_ref_VS_RF;
  Vector_MTTHist<TH1F> m_histograms_VS_RF;
  std::vector<std::unique_ptr<TH1F>> m_histograms_corrected_RF;
  std::unique_ptr<TH1F> m_histo_RF_corrected;
#endif //USE_RF
  MTTHist<TH1F> m_EnergyRef;
  MTTHist<TH2F> m_EnergyRef_bidim;
};

bool Timeshifts::load(std::string const & filename)
{
  std::ifstream inputfile(filename, std::ifstream::in);
  if (!inputfile.is_open())
  {
    print("Could not open the time shifts file - '", filename, "'");
    return (m_ok = false);
  }
  else if (file_is_empty(inputfile))
  {
    print("Time shift file - '", filename, "' is empty !");
    return (m_ok = false);
  }
  // ----------------------------------------------------- //
  UShort_t size = 0; std::string line = ""; int label = 0; 
  while (getline(inputfile, line))
  { // First extract the maximum label
    std::istringstream iss(line);
    iss >> label;
    if (size<label) size = label;
  }
  // ----------------------------------------------------- //
  // Second reading : fill the vector
  m_timeshifts.resize(size+1);
  inputfile.clear(); inputfile.seekg(0, inputfile.beg);
  int deltaT = 0;
  while (getline(inputfile, line))
  { // Then fill the array
    std::istringstream iss(line);
    iss >> label >> deltaT;
    m_timeshifts[label] = deltaT;
  }
  inputfile.close();
  print("Timeshifts extracted from", filename);
  return (m_ok = true);
}

bool Timeshifts::setParameters(std::string const & parameter)
{
  std::string temp;
  std::istringstream is(parameter);
  while(is >> temp)
  {
    if (temp == "timewindow:" || temp == "time_window:")
    {
      is >> m_timewindow_ns;
      if (!this -> setTimeWindow(m_timewindow_ns)) return false;
    }
    else if (temp == "time_reference:" || temp == "time_ref:")
    {
      is >> m_timeRef_name;
      if (!this -> setTimeReference(m_timeRef_name)) return false;
    }
    else if (temp == "outDir:")  {is >> m_outDir; setOutDir(m_outDir);}
    else if (temp == "outRoot:") {is >> m_outRoot; setOutRoot(m_outRoot);}
    else if (temp == "outData:") {is >> m_outData; setOutData(m_outData);}
    else if (temp == "mult:")    {is >> m_min_mult >> m_max_mult; setMult(m_min_mult, m_max_mult);} //by default
    else if (temp == "verbose")  {verbose(true);}
    else {std::cout << std::endl << "ATTENTION, parameter " << temp << " not recognized !" << std::endl << std::endl; return false;}
  }
  if (m_outData == "") 
  {
    m_outData = removeExtension(m_outRoot)+".dT";
    print("Name of the output timeshifts file automatically set to", m_outData);
  }
  return true;
}

bool Timeshifts::InitParameters()
{
  if (m_initialized) return true; // To prevent multiple initializations
  m_outPath = m_outDir+m_ts_outdir;
  create_folder_if_none(m_outPath);
  if (!folder_exists(m_outPath, true)) return (m_ok = m_initialized = false);
  if (extension(m_outData) != "dT") m_outData = removeExtension(m_outData)+".dT";
  if (m_detList.size() == 0)
  {
    print("PLEASE LOAD THE ID FILE IN THE TIMESHIFT MODULE");
    return (m_ok = m_initialized = false);
  }

  m_EnergyRef.reset("Energy_spectra", "Energy_spectra", 10000, 0, 1000000);
  m_EnergyRef_bidim.reset("Energy_VS_time", "Energy VS time", 100,-1,1, 1000,0,1000000);

  m_timeshifts.resize(m_detList.size(), 0);
  m_histograms_corrected.resize(m_detList.size());
  m_histograms.resize(m_detList.size());

#ifdef USE_RF
  m_histograms_VS_RF.resize(m_detList.size());
  m_histograms_corrected_RF.resize(m_detList.size());
#endif //USE_RF

  for (ushort label = 0; label<m_detList.size(); label++)
  {
    auto const & name = m_detList[label];
    if (name == "") continue;

    auto const & type = Detectors::type(label);

  #ifdef USE_RF
    if (label == 251) 
    {
      m_histo_ref_VS_RF.reset( "RF_calculated", "RF_calculated", m_timewindow/m_rebin["RF"], -m_timewindow/2, m_timewindow/2);
    }
    m_histograms_VS_RF[label].reset((name+"_RF").c_str(), (name+"_RF").c_str(), m_timewindow/m_rebin[type], -m_timewindow/2, m_timewindow/2);
  #endif //USE_RF
    m_histograms[label].reset(name.c_str(), name.c_str(), m_timewindow/m_rebin[type], -m_timewindow/2, m_timewindow/2);
  }
  return (m_ok = m_initialized = true);
}

bool Timeshifts::calculate(std::string const & folder, int const & nb_files)
{
  Timer timer;
  if (!InitParameters()) 
  {
    print("Someting went wrong in the timeshifts initialization...");
    return false;
  }

  m_files.addFolder(folder, nb_files);

  if (MTObject::ON)
  {// If multithreading, treat each data file of the folder in parallel
    print("Calculating timeshifts with", MTObject::getThreadsNb(),"threads");
    // The FileManager object isn't thread safe. 
    // That is why one has to encapsulate the files list inside a MTList (Multi-Threaded List) :
    MTList<std::string> files_MT(m_files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, *this, files_MT);
  }
  else
  {// If no multithreading, treat each data file sequentially
    print("Calculating timeshifts without multithreading");
    std::string filename;
    while (m_files.nextFileName(filename))
    {
      treatFile(filename);
    }
  }

  // Once the histograms have been filled, fit the peaks
  this -> analyse();

  // Write down the timeshitfs file in a .dT file, and the root histograms in a .root file
  write();

  print("Timeshifts calculated in", timer(), timer.unit());

  return true;
}

void Timeshifts::treatFilesMT(Timeshifts & ts, MTList<std::string> & files_MT)
{
  // This method has to be static in order to be multithreaded.
  // Thus, the only way it can be "link" to the class (i.e. having access to its public members) is to pass the class itself as an argument.
  // This is done by passing "Timeshifts & ts" in arguments and fill this argument with "*this" in the piece of code that calls it
  // Finally, we can use the same function as the not multithreaded piece of code.
  std::string filename;
  while (files_MT.getNext(filename))
  {
    ts.treatFile(filename);
  }
}

void get_first_RF_of_file(FasterReader & reader, Hit & hit, RF_Manager & rf)
{
  // int i = 0;
  while (reader.Read() && !(hit.label == RF_Manager::label)) continue;
  rf.setHit(hit);
  // reader.Reset(); TBD !!!
}

void Timeshifts::treatFile(std::string const & filename)
{
  if (extension(filename) == "root") { treatRootFile(filename);}
  else if (extension(filename) == "fast") { treatFasterFile(filename);}
}

void Timeshifts::treatRootFile(std::string const & filename) 
{
  std::unique_ptr<TFile> file (TFile::Open(filename.c_str(), "READ"));
  std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
  Event event(tree.get(), "lt");
  auto const nb_events = tree -> GetEntries();
  int event_i = 0;
  while(event_i<nb_events)
  {
    tree -> GetEntry(event_i);
    Fill(event);
  }
}

void Timeshifts::treatFasterFile(std::string const & filename)
{
  Hit hit;
  FasterReader reader(&hit, filename);
  Event event;
  CoincBuilder2 coincBuilder(&event, m_timewindow);

  uint counter = 0;
  
  bool maxHitsToRead = (m_max_hits>0);

#ifdef USE_RF
  RF_Manager rf;
  get_first_RF_of_file(reader, hit, rf);
#endif //USE_RF

  while(reader.Read() && ((maxHitsToRead) ? (counter<m_max_hits) : true) )
  {
    if ( (counter%static_cast<uint>(1.E+6)) == 0 ) print(counter);
    counter++;

  #ifdef USE_RF
    if(isRF[hit.label]) {rf.last_hit = hit.time; rf.period = hit.nrj;}
    else if (hit.label == m_timeRef_label)
    {
      hit.nrjcal = hit.nrj;
      m_histo_ref_VS_RF.Fill(rf.pulse_ToF(hit.time));
    }
    else
    {
      m_histograms_VS_RF[hit.label].Fill(rf.pulse_ToF(hit.time, USE_RF));
    }
  #endif //USE_RF

    if (coincBuilder.build(hit))
    {
      Fill(event);
    }
  }
}

void Timeshifts::Fill(Event const & event)
{
  auto mult = event.size();
  if (mult >= m_max_mult &&  mult <= m_min_mult) return;
  for (size_t i = 0; i < mult; i++)
  {
    if ( event.labels[i] == m_timeRef_label ) // if the ref detector triggered
    {
      auto const & refPos = i;
      Shift_t deltaT = 0;
      m_EnergyRef.Fill(event.nrjs[refPos]);
      for (uchar j = 0; j<mult; j++)
      {
        if (event.labels[j] == m_timeRef_label) continue; // To reject the time spectra of the time reference
        deltaT = event.times[refPos]-event.times[j];
        m_histograms[event.labels[j]].Fill(deltaT); //stored in ps
      }
    }
  }
}

/**
 * @brief Get which bin holds the X = 0
*/
int getBin0(TH1F* spectra)
{
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  std::vector<double> lowEdges(bins);
  spectra -> GetXaxis() -> GetLowEdge(lowEdges.data());
  int bin0 = 0;
  while(lowEdges[bin0] < 0) bin0++;
  return bin0;
}

/**
 * @brief Get the mean of the peak of a histogram with one nice single peak
*/
bool getMeanPeak(TH1F* spectra, double & mean)
{
  // Declaration :
  std::unique_ptr<TF1> gaus_pol0;
  std::unique_ptr<TF1> fittedPic;
  float pospic, amppic, dump_sigma;
  float cte, Mean, sigma;

  // Histogram characteristics :
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  auto const xmax = spectra -> GetXaxis() -> GetXmax();
  auto const xmin = spectra -> GetXaxis() -> GetXmin();

  auto const xPerBin = (xmax-xmin)/bins;
  auto const bin0 = getBin0(spectra);
  
  // Extract dump parameters :
  amppic = spectra -> GetMaximum();
  pospic = static_cast<double>( (spectra->GetMaximumBin() - bin0)*xPerBin );
  dump_sigma = static_cast<double>( (spectra->FindLastBinAbove(amppic/2) - spectra->FindFirstBinAbove(amppic/2)) * xPerBin/2 );

  // Fits the peak :
  gaus_pol0.reset(new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*dump_sigma,pospic+20*dump_sigma));
  gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
  gaus_pol0 -> SetRange(pospic-dump_sigma*20,pospic+dump_sigma*20);
  spectra -> Fit(gaus_pol0.get(),"R+q");

  // Extracts the fitted parameters :
  fittedPic.reset (spectra -> GetFunction("gaus+pol0"));
  if (!fittedPic) return false; // Eliminate non existing fits, when not enough statistics fit doesn't converge
  cte = fittedPic -> GetParameter(0);
  mean = Mean = fittedPic -> GetParameter(1);
  sigma = fittedPic -> GetParameter(2);

  if (false) print("cte", cte, "Mean", Mean, "sigma", sigma);

  return true;
}

void Timeshifts::analyse()
{
  print("Get the timeshifts from the time spectra");
  std::vector<float> array_labels(m_detList.size());
  for (size_t i = 0; i<array_labels.size(); i++) array_labels[i] = i;
  std::ofstream outDeltaTfile(m_outPath+m_outData, std::ios::out);

#ifdef USE_RF
  // Calculate the RF timeshift first :
  bool has_RF = (m_histo_ref_VS_RF.Integral() > 0);

  if (has_RF)
  {
    m_histo_ref_VS_RF.Merge();
    m_timeshifts[RF_Manager::label] = static_cast<Shift_t>( (m_histo_ref_VS_RF->GetMaximumBin() - (m_histo_ref_VS_RF -> GetNbinsX()/2)) * m_rebin["RF"] );
  }
  else 
  {
    print("ATTENTION : THIS RUN DOES NOT APPEAR TO CONTAIN ANY RF");
    print("RF label is :", RF_Manager::label);
    print("Timing reference is :", m_timeRef_label);
  }
#endif //USE_RF

  for (size_t label = 0; label<m_detList.size(); label++)
  {
    auto const & type = Detectors::type(label);
    if (type == "null") continue;
    m_histograms[label].Merge();

  #ifdef USE_RF
    if (has_RF)
    {
      m_histograms_VS_RF[label].Merge();
      if (label == RF_Manager::label)
      {
        if (m_verbose) print("RF :", m_timeshifts[RF_Manager::label], "with", m_histo_ref_VS_RF->GetMaximum(), "bins in peak");
        m_histo_RF_corrected.reset(shiftTimeSpectra(m_histo_ref_VS_RF, 251));
        outDeltaTfile << label << "\t" << m_timeshifts[RF_Manager::label] << std::endl;
        continue;
      }
    }
  #endif //USE_RF

    if (!m_histograms[label].exists()) continue; // Eliminate empty histograms
    if (m_verbose) print(m_detList[label]);

  #ifdef LICORNE
    if (type == "Eden")
    {
      outDeltaTfile << label << "\t" << 0;
      continue;
    }
  #endif //LICORNE

  #ifdef USE_DSSD
    if (type == "dssd")
    {
    #ifdef USE_RF
      if (has_RF)
      {
        auto amppic = m_histograms_VS_RF[label] -> GetMaximum();
        float zero = ( m_histograms_VS_RF[label] -> FindFirstBinAbove(amppic/2) - (m_histograms_VS_RF[label] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
        outDeltaTfile << label << "\t" << static_cast<float>(m_timeshifts[RF_Manager::label]-zero) << std::endl;
        if (m_verbose) print("Edge :", zero, "with", static_cast<int>(m_histograms_VS_RF[label] -> GetMaximum()), "counts in peak");
      }
      else 
      {
        auto amppic = m_histograms[label] -> GetMaximum();
        auto pospic = ( m_histograms[label] -> FindLastBinAbove(amppic*0.8) - (m_histograms[label] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
        outDeltaTfile << label << "\t" << pospic << std::endl;
        if (m_verbose) print( "mean : ", m_histograms[label] -> GetMean(), "with", (int) m_histograms[label] -> GetMaximum(), "counts in peak");
      }

    #else //NO USE_RF
      auto amppic = m_histograms[label] -> GetMaximum();
      auto pospic = ( m_histograms[label] -> FindLastBinAbove(amppic*0.8) - (m_histograms[label] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
      outDeltaTfile << label << "\t" << pospic << std::endl;
      if (m_verbose) print( "mean : ", m_histograms[label] -> GetMean(), "with", (int) m_histograms[label] -> GetMaximum(), "counts in peak");
    #endif //USE_RF
    
    continue;
    }
  #endif

    if (label == m_timeRef_label) m_timeshifts[label] = 0; // Force the reference detector to be at 0;
    else
    {
      double mean = 0;
      if (getMeanPeak(m_histograms[label], mean)) m_timeshifts[label] = static_cast<Shift_t> (-mean);
    }

    outDeltaTfile << label << "\t" << m_timeshifts[label] << std::endl;
  
  }
  // Now create the corrected time spectra :
  for (size_t label = 0; label<m_histograms.size(); label++)
  {
    auto & histo = m_histograms[label];
    if (histo) 
    {
      m_histograms_corrected[label].reset( shiftTimeSpectra(histo, label) );
    }

  #ifdef USE_RF
    auto & histo_RF = m_histograms_VS_RF[label];
    if (histo_RF) m_histograms_corrected_RF[label].reset( shiftTimeSpectra(histo_RF, label) );
  #endif //USE_RF
  }

  outDeltaTfile.close();

  std::cout << "Timeshifts data written to " << m_outPath+m_outData << std::endl;
}

void Timeshifts::write()
{
  std::unique_ptr<TFile> outFile(TFile::Open((m_outPath+m_outRoot).c_str(),"RECREATE"));
  if (outFile.get() == nullptr) {print("Cannot open file ", m_outPath+m_outRoot, " !!!\nAbort !");return;}

  m_EnergyRef.Write();

#ifdef USE_RF
  m_histo_ref_VS_RF.Write();
  if (THist_exists(m_histo_RF_corrected.get())) m_histo_RF_corrected->Write();
#endif //USE_RF

  for (auto & histo : m_histograms) histo.Write();
  for (auto & histo : m_histograms_corrected) if (THist_exists(histo.get())) histo->Write();

#ifdef USE_RF
  for (auto & histo : m_histograms_VS_RF) histo.Write();
  for (auto & histo : m_histograms_corrected_RF) if (THist_exists(histo.get())) histo->Write();
#endif //USE_RF

  outFile -> Write();
  outFile -> Close();

  std::cout << "Timeshifts root file saved to " << m_outPath+m_outRoot << std::endl;
}

bool Timeshifts::setTimeWindow(int const & timewindow_ns) 
{
  if (timewindow_ns == 0) {std::cout << "NO TIME WINDOW !!" << std::endl; return false; }
  m_timewindow_ns = timewindow_ns;
  m_timewindow = m_timewindow_ns*1000;
  print("Extracting timeshifts with a time window of ", m_timewindow_ns, " ns");
  return true;
}

bool Timeshifts::setTimeWindow(std::string const & timewindow_string) 
{
  auto const timewindow_ns = stoi(timewindow_string);
  return setTimeWindow(timewindow_ns);
}

bool Timeshifts::setTimeReference(int const & timeRef_label)
{
  m_timeRef_label = timeRef_label;
  m_timeRef_name = m_detList[m_timeRef_label];
  std::cout << "Reference detector set to be " << m_timeRef_name << " (n°" << m_timeRef_label << ")" << std::endl;
  return true;
}

bool Timeshifts::setTimeReference(std::string const & timeRef_name)
{
  m_timeRef_name = timeRef_name;
  if (timeRef_name == "") {std::cout << "NO TIME REFERENCE !!" << std::endl; return false; }
  // If the string contains the label number then convert the string to int (stoi)
  // If the string contains the detector name then extract the label number from the detectors list (m_detList)
  return setTimeReference( (isNumber(timeRef_name)) ? std::stoi(timeRef_name) : m_detList.getLabel(timeRef_name));
}

void Timeshifts::setOutDir(std::string const & outDir)
{
  m_outDir = outDir;
  push_back_if_none(m_outDir, '/');
}

void Timeshifts::setOutRoot(std::string const & outroot) 
{
  m_outRoot = outroot+"dT"; 
  if (extension(m_outRoot) != "root") 
  {
    m_outRoot = removeExtension(m_outRoot)+"root";
  }
}

void Timeshifts::setOutData(std::string const & outdata)
{
  m_outData = outdata;
  if (extension(m_outData) != "dT") 
  {
    m_outData = removeExtension(m_outData)+".dT";
  }
}

TH1F* Timeshifts::shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit)
{
  std::string name = histo->GetName();
  if (name == "")  return (new TH1F());
  
  auto axis = histo -> GetXaxis();

  auto bins    = axis -> GetNbins();
  auto timeMax = axis -> GetXmax ();
  auto timeMin = axis -> GetXmin ();

  auto binToTime = (timeMax-timeMin)/bins;
  auto time_to_ps = 1;

  if (unit != "ps")
  {
    if (unit == "ns") time_to_ps*=1000;
    else if (unit == "ticks") time_to_ps*=128; // Deprecated
    else print("UNKOWN TIME UNIT", unit);
    return nullptr;
  }

  auto corrected_histo = static_cast<TH1F*>(histo -> Clone((name+"_corrected").c_str()));

  for (int bin = 1; bin<bins; bin++)
  {
    auto const time_ps = time_to_ps*(binToTime*bin + timeMin); // time_to_ps * (coeff*bin + offset_time)
    auto const new_time_ps = time_ps + m_timeshifts[label];
    auto const new_time = new_time_ps/time_to_ps;
    int const newbin = ( new_time - timeMin ) / binToTime; // ( (time_ps/time_to_ps) - offset_time) / coeff

    if( newbin>0 && newbin < bins )
    {
      corrected_histo -> SetBinContent(newbin, histo->GetBinContent(bin));
    }
  }
  return corrected_histo;
}


#endif //TIMESHIFTS_H
