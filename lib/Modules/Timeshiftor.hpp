#ifndef TIMESHIFTOR_HPP
#define TIMESHIFTOR_HPP

#include "../libRoot.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/FasterReader.hpp"
#include "../Classes/FilesManager.hpp"
#include "../Classes/Hit.hpp"
#include "../Classes/RF_Manager.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/CoincBuilder.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/Performances.hpp"

#include "../MTObjects/MTList.hpp"
#include "../MTObjects/MultiHist.hpp"


/// @brief Casts a number into unsigned Time
template<typename T,  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline Time Timeshift_cast(T const & t) {return static_cast<Time>(t);}

/**
 * @brief Calculates the time difference between all detectors and a reference detector
 * @details
 *  * The timestamps and timeshifts are in picoseconds
 * Goal 1 : calculate the time shifts between detectors
 * How : 
 *   Use one detector as reference, preferentially with good time resolution
 *   Create coincidences using the CoincBuilder event builder
 *   Calculate the difference in timestamp as follow : ref_time - detector_time
 *   Finally, output the (.dT) file containing the time shifts of all the detectors
 *   Also produces a .root file containing the raw and corrected time spectra.
 *   This module includes multithreading management using MTObject::parallelise_function(function, parameters...)
 * Goal 2 : read a timeshift file (.dT) previously calculated by this module
 * Goal 3 : apply the timeshifts by calling the operator[] : time_correct = time_shifted + timeshifts[label];
 * Timeshiftor ts;
 * detectors.load("index_129.list");
 * ts.calculate()
 * 
 * @todo This code has a lot of legacy parts and is very confusing, I feel sorry for anyone wanting to read it, cleaning it might be a good idea
 */
class Timeshiftor
{
public:

  Timeshiftor() {}

  ///@brief Call the Timeshiftor::load() method to load the timeshifts from a .dT file 
  Timeshiftor(std::string const & filename) : m_filename(filename) {this -> load(m_filename); this -> Initialise();}

  ///@brief Call the Timeshiftor::load() method to load the timeshifts from a .dT file 
  Timeshiftor(const char * filename) : m_filename(filename) {this -> load(m_filename);}

  /**
   * @brief For faster to root conversion only : Timeshiftor::load() loads the timeshifts from a .dT file 
   * @param path: The path of the out root directory (/path/to/run_name)
   * @param name: The name of the .dT file without the extension (e.g. run_10)
   * @details Will read a file in /path/Timeshitfs/name.dT
  */
  Timeshiftor(std::string const & path, std::string const & name) : m_filename(path+"Timeshiftor/"+name+".dT") {this -> load(m_filename);}

  /// @brief Copy constructor
  Timeshiftor(Timeshiftor const & timeshifts) : m_timeshifts(timeshifts.m_timeshifts) {}
  
  Timeshiftor& operator=(Timeshiftor const & timeshifts)
  {
    m_timeshifts = timeshifts.m_timeshifts;
    m_ok = timeshifts.m_ok;
    return *this;
  }

  /// @brief Use this method to setup the parameters from a string in order to calculate the timeshifts
  bool setParameters(std::string const & parameter);

  /// @brief Use this method to load timeshifts from a .dT file
  bool load(std::string const & filename);

  /// @brief Setup the time window for the coincidence in ns
  bool setTimeWindow_ns(Time_ns const & timewindow_ns);

  /**
   * @brief Set the time window in ns
   * @note It is better to keep it at its default value, otherwise you can miss some detectors
   * 
   * @param timewindow_ns: default value 1500.f ns
  */
  bool setTimeWindow_ns(std::string const & timewindow_ns_str);

  /// @brief Set the time reference label
  /// @note Mandatory only if calculate timeshifts
  bool setTimeReference(Label const & timeRef_label);

  /**
   * @brief Set the time reference.
   * @note Mandatory only if calculate timeshifts
   * 
   * Either the full name, or the label number
   * 
  */
  bool setTimeReference(std::string const & timeRef_name);

  ///@brief Set the output directory (full path);
  ///@note Mandatory only if calculate timeshifts
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
   * @brief Use setName instead. Set output root file containing the raw and corrected time spectra
   * @note Mandatory only if calculate timeshifts
  */
  void setOutRoot(std::string const & outRoot);

  /**
   * @brief Use setName instead. Set output file name of the timeshifts data. 
   * @note Mandatory only if calculate timeshifts
   * 
   * If not set, it is automatically named after the root file.
  */
  void setOutData(std::string const & outData);

  /**
   * @brief Set output file name of the timeshifts data and root file.
   * @note Mandatory only if calculate timeshifts
  */
  void setOutName(std::string const & outData);

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
  void setMult(int const & min_mult, int const & max_mult) {m_min_mult = min_mult; m_max_mult = max_mult;}

  /**
   * @brief Set maximum hits to be read in a file
   * @note Use only if calculate timeshifts
  */
  void setMaxHits(ulong const & max_hits = -1) {m_max_hits = max_hits;}

  /**
   * @brief Set RF offset (synonymous to Timeshiftor::setRFShift)
   * @note Use only if calculating timeshifts with RF
   * @param offset: default 50000 ps
  */
  void setRFOffset(Time const & offset_ps) {RF_Manager::setOffset(offset_ps);}

  /**
   * @brief Set RF offset (synonymous to Timeshiftor::setRFShift)
   * @note Use only if calculating timeshifts with RF
   * @param offset: default 50 ns
  */
  void setRFOffset_ns(Time_ns const & offset_ns) {RF_Manager::setOffset_ns(offset_ns);}

  bool calculate(std::string const & folder, int const & nb_files = -1);

  bool verify(std::string const & folder, int const & nb_files = -1);

  /**
   * @brief Used to create @todo caption
   * 
   */
  bool const & verification(bool const & verif = true) {return (m_verification = verif);}

  // void Print() {print(m_timeshifts);}

  TH1F* shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit = "ps");

  Time const & operator[] (int const & i) const {return m_timeshifts[i];}

  void operator() (Hit & hit) const {hit.stamp += m_timeshifts[hit.label];}

  operator bool() const & {return m_ok;}

  std::vector<Time> const & get() const {return m_timeshifts;}
  std::vector<Time> const & data() const {return m_timeshifts;}
  Time const & get(int const & i) const {return m_timeshifts[i];}

  void write    (std::string const & name);
  void writeRoot(std::string const & name);
  void writeData(std::string const & name);

  void setEminADC(ADC const & EminADC) {m_EMin_ADC = EminADC;}

  void dT_with_RF(dType const & type) {m_RF_preferred[type] = true;}
  void dT_with_RF(Label const & label) {InitialisePreferencesArrays(); m_RF_preferred_label[label] = true;}
  void dT_with_raising_edge(dType const & type) {m_edge_preferred[type] = true;}
  void dT_with_raising_edge(Label const & label) {InitialisePreferencesArrays(); m_edge_preferred_label[label] = true;}
  void dT_with_biggest_peak_finder(Label const & label) {InitialisePreferencesArrays(); m_biggest_peak_finder_label[label] = true;}
  void dT_with_biggest_peak_finder() {m_biggest_peak_finder = true;}

  /**
   * @brief 
   * 
   * @param check: default true.
   */
  void checkForPreprompt(bool const & check = true) {m_check_preprompt = check;}

  bool CheckParameters()
  {
    if (!detectors.exists(m_time_ref_label)) 
    {
      print("Time reference detector", m_time_ref_name, "with label", m_time_ref_label, "do not exists in ID file ...");
      return false;
    }
    return true;
  }

  std::vector<bool>  m_RF_preferred_label; // Used to force RF measurement for specific labels;
  std::vector<bool>  m_edge_preferred_label; // Used to force RF measurement for specific labels;
  std::vector<bool>  m_biggest_peak_finder_label; // Use to use BiggestPeakFitter - best suited for well binned and low background peaks for which the traditionnal peak fitter fails
  bool  m_biggest_peak_finder = false; // Use to use BiggestPeakFitter - best suited for well binned and low background peaks for which the traditionnal peak fitter fails
  std::vector<int> m_nb_shifts_RF_peak;  // If the dT peak used with the RF is not within [+- RF_period] then one need to shift it accordingly

  void rebin(std::string const & detector, Time_ns const & bin_size_ns)
  {
    if (!find_key(m_bins_per_ns, detector)) throw_error("in Timeshiftor::rebin(std::string detector, float bin_size_ns) : detector not known !!");
    m_bins_per_ns[detector] = bin_size_ns;
  }

  void periodRF_ns(int const & period_ns) {m_use_rf = true; m_rf_period = period_ns;}

  // TODO :
  // void rebin(Label const & label, Time_ns const & bin_size_ns)
  // {
  //   m_bins_per_ns_labels[label] = bin_size_ns;
  // }

// private:

  void InitialisePreferencesArrays();
  bool m_pref_arrays_init = false;
  bool Initialise(bool const & InitialiseRaw = false, bool const & InitialiseCorrected = false);
  bool InitialiseRaw();
  bool InitialiseCorrected();
  static void treatFilesMT(Timeshiftor & ts, MTList & files_MT);
  void treatFolder(std::string const & folder, int const & nb_files = -1);
  void treatFile(std::string const & filename);
  void treatRootFile(std::string const & filename);
  void treatFasterFile(std::string const & filename);
  void Fill(Event const & event, RF_Manager & rf);
  // void FillRF(Hit const & hit, RF_Manager & rf);
  void analyse();

  bool m_ok = false;
  bool m_verbose = false;
  bool m_verification = true;
  bool m_corrected = false;
  bool m_folder_treated = false;

  bool m_Initialised = false;
  bool m_energySpectraInitialised = false;
  bool m_InitialisedRaw = false;
  bool m_InitialisedCorrected = false;

  bool m_check_preprompt = false;

  FilesManager m_files;

  std::string m_filename = "";

  std::vector<Time> m_timeshifts;
//   std::vector<Time> m_timeshifts;
  // std::vector<Timestamp> mt_ref_time;

  Time_ns m_timewindow_ns = 1500;
  Time m_timewindow = 1500_ns;
  int m_min_mult = 2;
  int m_max_mult = 2;
  Label m_time_ref_label = 252;
  std::string m_time_ref_name = "R1A9_FATIMA_LaBr3";
  ulong m_max_hits = -1;
  double m_EMin_ADC = 0.;
  int m_nb_detectors = 0;

  // RF : 
  bool m_use_rf = false;
  Time m_rf_period = 0;

  // This map holds the number of bins per ns (e.g. for LaBr3 there is one bin every 100 ps)
  std::map<dType, Time_ns> m_bins_per_ns = 
  { 
    {"labr",  10.}, 
    {"ge",     1.}, 
    {"bgo",    2.}, 
    {"eden",   2.}, 
    {"RF",    10.}, 
    {"paris",  5.}, 
    {"dssd",  0.5}
  };

  // Idem, but allows to handle individual labels; TODO
  // std::map<dType, Time_ns> m_bins_per_ns_labels;

  // This map tell if the timeshift is preferentially taken as the 
  std::map<dType, bool> m_RF_preferred = 
  { 
    {"labr",  false}, 
    {"ge",    false}, 
    {"bgo",   false}, 
    {"eden",  false}, 
    {"RF",    false}, 
    {"paris", false}, 
    {"dssd",  false}
  };

  // This map tells if the timeshift is preferentially taken as the mean of the peak (false) or the raising edge (true)
  std::map<dType, bool> m_edge_preferred = 
  { 
    {"labr",  false}, 
    {"ge",    false}, 
    {"bgo",   false}, 
    {"eden",  false}, 
    {"RF",    false}, 
    {"paris", false}, 
    {"dssd",  false}
  };

  std::string m_outDir = "";
  std::string m_ts_outDir = "Timeshifts/";
  Path m_outPath;
  std::string m_outRoot = "timeshifts.root";
  std::string m_outData = "";

  // Histograms :
  MultiHist<TH1F> m_EnergyRef; // Energy spectra of the time reference detector
  MultiHist<TH2F> m_EnergyRef_bidim; // Energy VS Time (unused)

  Vector_MultiHist<TH1F> m_time_spectra; // Time spectra from coincidence with the time reference detector, one TH1F for each detector
  MultiHist<TH2F> m_time_spectra_bidim; // Time spectra from coincidence with the time reference detector, X axis label, Y axis time spectra, after timeshift
  Vector_MultiHist<TH1F> m_time_spectra_corrected; // Time spectra from coincidence with the time reference detector, one TH1F for each detector, after timeshift
  MultiHist<TH2F> m_time_spectra_corrected_bidim; // Time spectra from coincidence with the time reference detector, X axis label, Y axis time spectra, after timeshift

  std::unordered_map<std::string, TH1F*> integratedHistos;

  MultiHist<TH1F> m_histo_ref_VS_RF; // RF time spectra of the time reference detector
  MultiHist<TH2F> m_histo_ref_vs_RF_VS_mult; // RF time spectra VS multiplicity of the time reference detector
  MultiHist<TH1F> m_time_spectra_reference_RF_corrected; // RF time spectra VS multiplicity of the time reference detector, after timeshift

  Vector_MultiHist<TH1F> m_histograms_VS_RF; // RF time spectra, one TH1F for each detector
  Vector_MultiHist<TH1F> m_time_spectra_corrected_RF; // RF time spectra, one TH1F for each detector, after timeshift
  MultiHist<TH2F> m_time_spectra_corrected_bidim_RF; // RF time spectra, X axis label, Y axis time spectra, after timeshift

public:
  class NotFoundError
  {
  public:
    NotFoundError(std::string const & filename) : m_filename(filename) {error("Timeshiftor::NotFoundError", filename);}
    auto const & getFilename() const {return m_filename;}
  private:
    std::string m_filename;
  };

  class EmptyError
  {
    public:
      EmptyError() {error("No timeshift provided");}
  };
};

void Timeshiftor::InitialisePreferencesArrays()
{
  if (!m_pref_arrays_init)
  {
    m_RF_preferred_label.resize(detectors.size(), false);
    m_edge_preferred_label.resize(detectors.size(), false);
    m_biggest_peak_finder_label.resize(detectors.size(), false);
    m_pref_arrays_init = true;
  }
}

bool Timeshiftor::load(std::string const & filename)
{
  std::ifstream inputFile(filename, std::ifstream::in);
  if (!inputFile.good()) {throw NotFoundError(filename);}
  else if (file_is_empty(inputFile)) {print("TIMESHIFT FILE", filename, "EMPTY !");return false;}
  std::string line = ""; // Reading buffer
  Label label = 0; // Reading buffer

  // ----------------------------------------------------- //
  // First extract the maximum label
  Label size = 0;
  // Infer the number of detectors from the higher label in calbration file
  while (getline(inputFile, line))
  { 
    std::istringstream iss(line);
    iss >> label;
    if (size<label) size = label;
  }
  size++; // The size of the vector must be label_max+1
  // Ensure there is no mismatch with the detectors module :
  if (detectors && size<detectors.size()) size = detectors.size();
  inputFile.clear(); 
  inputFile.seekg(0, inputFile.beg);

  // ----------------------------------------------------- //
  // Now fill the vector
  m_timeshifts.resize(size, 0);
  Time shift = 0;
  while (getline(inputFile, line))
  { // Then fill the array
    m_nb_detectors++;
    std::istringstream iss(line);
    iss >> label >> shift;
    m_timeshifts[label] = shift;
    shift = 0;
  }
  inputFile.close();
  print("Timeshifts extracted from", filename);
  return (m_ok = true);
}

bool Timeshiftor::setParameters(std::string const & parameter)
{
  std::string temp;
  std::istringstream is(parameter);
  while(is >> temp)
  {
    if (temp == "timewindow:" || temp == "time_window:")
    {
      is >> m_timewindow_ns;
      if (!this -> setTimeWindow_ns(m_timewindow_ns)) return false;
    }
    else if (temp == "time_reference:" || temp == "time_ref:")
    {
      is >> m_time_ref_name;
      if (!this -> setTimeReference(m_time_ref_name)) return false;
    }
    else if (temp == "outDir:")  {is >> m_outDir; setOutDir(m_outDir);}
    else if (temp == "outRoot:") {is >> m_outRoot; setOutRoot(m_outRoot);}
    else if (temp == "outData:") {is >> m_outData; setOutData(m_outData);}
    else if (temp == "mult:")    {is >> m_min_mult >> m_max_mult; setMult(m_min_mult, m_max_mult);} //by default
    else if (temp == "verbose")  {verbose(true);}
    else {print("\nATTENTION, parameter", temp, "not recognized !\n\n"); return false;}
  }
  if (m_outData == "") 
  {
    m_outData = removeExtension(m_outRoot)+".dT";
    print("Name of the output timeshifts file automatically set to", m_outData);
  }
  return true;
}

bool Timeshiftor::InitialiseRaw()
{
  if (m_InitialisedRaw) return true;// To prevent multiple initializations

  // Raw Timeshiftor spectra :
  m_time_spectra.resize(detectors.size());

  if (m_use_rf)
  {
    // Raw RF Timeshiftor spectra :
    m_histograms_VS_RF.resize(detectors.size());
    m_histo_ref_VS_RF.reset( "Ref_RF_calculated", "Reference RF calculated;Time[ps];#", m_timewindow_ns*m_bins_per_ns["RF"], -m_timewindow/2, m_timewindow/2);
    m_histo_ref_vs_RF_VS_mult.reset( "Ref_RF_VS_mult", "Reference RF VS multiplicity;Time[ps];Multiplicity", m_timewindow_ns*m_bins_per_ns["RF"],-m_timewindow/2,m_timewindow/2, 10,0,10 );
  }

  m_time_spectra_bidim.reset("raw_spectra_VS_label", "raw_spectra_VS_label", detectors.size(),0,detectors.size(), 1000,-m_timewindow/2, m_timewindow/2);

  for (ushort label = 0; label<detectors.size(); label++)
  {
    if (!detectors.exists(label)) continue;
    auto const & name = detectors[label];
    // auto const & type = detectors.type(label);
    std::string type;
    if (label<200) type = (((label-23)%6 > 1)) ? "ge" : "bgo";
    else if (label == 212) type = "RF";
    else if (label == 252) type = "labr";
    else if (label > 300 && label < 800) type = "paris";
    else if (label >= 800) type="dssd";

    m_time_spectra[label].reset(name.c_str(), name.c_str(), m_timewindow_ns*m_bins_per_ns[type], -m_timewindow/2, m_timewindow/2);

    if (m_use_rf)
    {
      m_histograms_VS_RF[label].reset((name+"_RF").c_str(), (name+"_RF").c_str(), m_timewindow_ns*m_bins_per_ns[type], -m_timewindow/2, m_timewindow/2);
      if (type == "RF")
      {
        m_time_spectra[label].reset(name.c_str(), name.c_str(), 1000, -m_timewindow/2, m_timewindow/2);
      }
    }
  }
  return (m_InitialisedRaw = true);
}

bool Timeshiftor::InitialiseCorrected()
{
  if (m_InitialisedCorrected) return true;// To prevent multiple initializations

  // Corrected timeshifts spectra :
  m_time_spectra_corrected.resize(detectors.size());
  m_time_spectra_corrected_bidim.reset("All_corrected_time_spectra", "All corrected time spectra;Channel;Time[ps];#", 
          detectors.size(), 0, detectors.size(), 1000, -m_timewindow/2, m_timewindow/2);

  
  float borne_inf = -100000.f;
  float borne_sup = (float_cast(m_rf_period)+100.f)*1000.f;
  float RF_time_window = borne_sup-borne_inf;
  float RF_time_window_ns = RF_time_window/1000.f;

  if(m_use_rf)
  {
    // Corrected RF timeshifts spectra :
    m_time_spectra_corrected_RF.resize(detectors.size());// Initialise the vector
    m_time_spectra_reference_RF_corrected.reset( "Ref_RF_corrected", "Reference RF corrected;Time[ps];#", RF_time_window_ns*m_bins_per_ns["RF"], borne_inf, borne_sup);
    m_time_spectra_corrected_bidim_RF.reset("All_corrected_time_spectra_RF", "All corrected time spectra VS RF;Channel;Time[ps];#", 
            detectors.size(), 0, detectors.size(), RF_time_window_ns*2, borne_inf, borne_sup);
  }

  for (ushort label = 0; label<detectors.size(); label++)
  {
    if (!detectors.exists(label)) continue;
    auto const & name = detectors[label];
    auto const & type = detectors.type(label);

    m_time_spectra_corrected[label].reset((name+"_corrected").c_str(), (name+" corrected;Time[ps];#").c_str(), m_timewindow_ns*m_bins_per_ns[type], -m_timewindow/2, m_timewindow/2);
  
    if(m_use_rf) m_time_spectra_corrected_RF[label].reset((name+"_RF_corrected").c_str(), (name+" RF corrected;Time[ps];#").c_str(), 
                    RF_time_window_ns*m_bins_per_ns[type], borne_inf, borne_sup);
  }
  return (m_InitialisedCorrected = true);
}

bool Timeshiftor::Initialise(bool const & InitialiseRaw, bool const & InitialiseCorrected)
{
  Timeshiftor::InitialisePreferencesArrays();

  if (InitialiseRaw) this -> InitialiseRaw();

  if (InitialiseCorrected) this -> InitialiseCorrected();    

  if (!m_energySpectraInitialised && (InitialiseRaw || InitialiseCorrected))
  {
    // Energy spectra :
    m_EnergyRef.reset("Energy_spectra", "Energy_spectra;ADC;#", 10000, 0, 1000000);
    m_energySpectraInitialised = true;
  }

  if (m_Initialised) return true; // To prevent multiple initializations

  // mt_ref_time.resize(MTObject::getThreadsNumber(), 0);

  // Check the Detectors module :
  auto const & label_max = detectors.size();
  if (label_max == 0)
  {
    print("PLEASE LOAD THE ID FILE");
    return (m_ok = m_Initialised = false);
  }

  m_timeshifts.resize(label_max, 0);
  m_RF_preferred_label.resize(label_max, false);
  m_edge_preferred_label.resize(label_max, false);
  m_nb_shifts_RF_peak.resize(label_max, 0);

  return (m_ok = m_Initialised = true);
}

void Timeshiftor::treatFolder(std::string const & folder, int const & nb_files)
{
  Path path(folder);
  FilesManager files;
  files.addFolder(folder, nb_files);

  if (MTObject::ON)
  {// If multithreading, treat each data file of the folder in parallel
    print(Colib::Color::BRIGHTBLUE, "Calculating timeshifts with", MTObject::getThreadsNb(),"threads", Colib::Color::RESET);
    // The FileManager object isn't thread safe. 
    // That is why one has to encapsulate the files list inside a MTList (Multi-Threaded List) :
    MTList files_MT(files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, *this, files_MT);
  }
  else
  {// If no multithreading, treat each data file sequentially
    print(Colib::Color::BRIGHTBLUE, "Calculating timeshifts without multithreading", Colib::Color::RESET);
    std::string filename;
    while (files.nextFileName(filename))
    {
      treatFile(filename);
    }
  }
  m_folder_treated = true;
}

bool Timeshiftor::calculate(std::string const & folder, int const & nb_files)
{
  Timer timer;
  if (!this -> Initialise(true,false)) 
  {
    print("Something went wrong in the timeshifts calculation initialization...");
    return false;
  }

  if (!this -> CheckParameters()) return false;
  

  // Fill the time spectra with data from data files :
  this -> treatFolder(folder, nb_files);

  // Once the histograms have been filled, fit the peaks :
  this -> analyse();

  print("Timeshiftor calculated in", timer());

  return true;
}

/**
 * @brief To verify an already calculated timeshift .dT data file
 * 
 */
bool Timeshiftor::verify(std::string const & folder, int const & nb_files)
{
  Timer timer;
  if (!this -> Initialise(false, true)) 
  {
    print("Something went wrong in the timeshifts initialization...");
    return false;
  }

  if (!m_ok) throw EmptyError();

  if (!detectors) throw Detectors::EmptyError();

  // if (m_folder_treated)
  // {
  //   print("\nTimeshifts calculated, now verification time !\n");
  //   for (auto const & label : detectors.labels())
  //   {
  //     if (!m_time_spectra[label].Merged()) continue;
  //     m_time_spectra_corrected[label] = static_cast<TH1F*>(m_time_spectra[label].get()->Clone(concatenate(m_time_spectra[label].name(), "_corrected").c_str()));
  //     shiftX(m_time_spectra_corrected[label].get(), -double_cast(m_timeshifts[label]));
  //     if (m_use_rf) 
  //     {
  //       if (!m_histograms_VS_RF[label].Merged()) continue;
  //       m_time_spectra_corrected_RF[label] = static_cast<TH1F*>(m_histograms_VS_RF[label].get()->Clone(concatenate(m_histograms_VS_RF[label].name(), "_corrected").c_str()));
  //       shiftX(m_time_spectra_corrected_RF[label].get(), double_cast(m_timeshifts[label]));
  //     }
  //   }
  // }
  // else // If no time spectra already exist :
  // {
    print();
    print("Timeshiftor loaded, now verification time !");
    print();
    m_corrected = true;
    this -> treatFolder(folder, nb_files);
    m_corrected = false;
  // }

  print("Timeshiftor verified in", timer());

  return true;
}

void Timeshiftor::treatFilesMT(Timeshiftor & ts, MTList & files_MT)
{
  // This method has to be static in order to be multithreaded.
  // Thus, the only way it can be "link" to the class (i.e. having access to its public members) is to pass the class itself as an argument.
  // This is done by passing "Timeshiftor & ts" in arguments and fill this argument with "*this" in the piece of code that calls it
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
  // TO BE DONE : reset the reader to the beginning of the file
}

void get_first_RF_of_file(TTree * tree, Event & event, RF_Manager & rf)
{
  int event_i = 0;
  bool done = false;
  while (event_i<tree->GetEntries() && done == false)
  {
    tree -> GetEntry(event_i);
    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      if (rf.setHit(event,hit_i)) 
      {
        done = true;
        break;
      }
    }
  }
}

void Timeshiftor::treatFile(std::string const & filename)
{
  if (extension(filename) == "root") { treatRootFile(filename);}
  else if (extension(filename) == "fast") { treatFasterFile(filename);}
}

void Timeshiftor::treatRootFile(std::string const & filename) 
{
  std::unique_ptr<TFile> file (TFile::Open(filename.c_str(), "READ"));
  std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball2"));
  print("reading", filename);
  Event event(tree.get());
  auto const nb_events = tree -> GetEntries();
  int event_i = 0;
  RF_Manager rf;
  if (m_use_rf) get_first_RF_of_file(tree.get(), event, rf);
  for(;event_i<nb_events; ++event_i)
  {
    tree -> GetEntry(event_i);
    for (int hit = 0; hit<event.mult; hit++) 
    {
      if (m_corrected) event.times[event.labels[hit]] += m_timeshifts[event.labels[hit]];
    }
    this -> Fill(event, rf);
  }
}

void Timeshiftor::treatFasterFile(std::string const & filename)
{
  Hit hit;
  FasterReader reader(&hit, filename);
  Event event;
  CoincBuilder coincBuilder(&event, m_timewindow);

  uint counter = 0;  
  bool const maxHitsToRead = (m_max_hits>0);

  RF_Manager rf;

  if(m_use_rf)
  {
    rf.setPeriod_ns(m_rf_period);
    get_first_RF_of_file(reader, hit, rf);
  }

  // Handle the first hit :
  reader.Read(); 
  // mt_ref_time[MTObject::getThreadIndex()] = hit.stamp;


  while(reader.Read() && ((maxHitsToRead) ? (counter<m_max_hits) : true) )
  {
  #ifdef DEBUG
    if ( (counter%static_cast<uint>(1.E+6)) == 0 ) print(counter);
  #endif //DEBUG
    counter++;

    if (m_corrected) hit.stamp += m_timeshifts[hit.label];

    // This is used to put the energy value of the time reference in the Event (used in the Fill method) :
    if (hit.label == m_time_ref_label) hit.nrj = NRJ_cast(hit.adc); 
    else hit.nrj = 0;

    if(m_use_rf) if(isRF[hit.label]) {rf.last_downscale_timestamp = hit.stamp; rf.period = hit.adc;}

    if (coincBuilder.build(hit)) {this -> Fill(event, rf);}
  }
}

/**
 * @brief 
 * 
 * @param event 
 */
void Timeshiftor::Fill(Event const & event, RF_Manager & rf)
{
  auto const & mult = event.mult;
  
  if (mult >= m_max_mult &&  mult <= m_min_mult) return;

  // There are 2 imbricated loops : the first one fills the time spectra and looks for the time reference.
  // If found, it opens another loop to fill the coincidence time spectra

  for (int loop_i = 0; loop_i < mult; loop_i++)
  {
    if(m_use_rf)
    {
      auto const & label = event.labels[loop_i];
      auto const & time = event.times[loop_i];
      auto const & nrj = event.nrjs[loop_i];
      auto const & ToF = rf.relTime(event.stamp+time);

      if (m_corrected)
      {
        if (label == m_time_ref_label && nrj>m_EMin_ADC && mult>5) m_time_spectra_reference_RF_corrected.Fill(ToF);
        m_time_spectra_corrected_RF[label].Fill(ToF);
        m_time_spectra_corrected_bidim_RF.Fill(label, ToF);
      }
      else 
      {// Raw data :
        if (label == m_time_ref_label && nrj>m_EMin_ADC) m_histo_ref_VS_RF.Fill(ToF);
        else
        {
          m_histograms_VS_RF[label].Fill(ToF);
        }
      }
    }

    // We require the reference detector in the event :
    if (event.labels[loop_i] == m_time_ref_label)
    {
      // print(event);
      // pauseCo();
      // Extract informations of the reference :
      auto const & refPos = loop_i; // Position of the reference in the event
      auto const & refE = event.nrjs[refPos]; // Energy deposited in the reference detector
      auto const & refT = event.times[refPos];// Timestamp of the hit in the reference detector


      // Handle reference detector :
      if (refE < m_EMin_ADC) return;
      m_EnergyRef.Fill(refE);

      // Loop over the other hits of the event :
      for (int loop_j = 0; loop_j<mult; loop_j++)
      {
        auto const & label = event.labels[loop_j];

        if (label == m_time_ref_label) continue; // To reject the time spectra of the time reference

        auto deltaT = static_cast<Long64_t>( refT - event.times[loop_j] );

        if (m_corrected)
        {
          m_time_spectra_corrected[label].Fill(deltaT);
          m_time_spectra_corrected_bidim.Fill(label, deltaT);
        }
        else 
        {
          m_time_spectra[label].Fill(deltaT);
          m_time_spectra_bidim.Fill(label, deltaT);
        }
        // pauseCo();
      }
    }
  }
}

double getRFGammaPrompt(TH1F * hist, bool const & check_preprompt)
{
  auto xAxis = hist -> GetXaxis();

  auto const & maxBin = hist -> GetMaximumBin();
  auto const & maxBin_ps = hist -> GetBinCenter(maxBin);

  if(check_preprompt)
  {
    // The goal is to detect the moments the preprompt has been spotted
    // Is we got the preprompt, then the following 10 ns has not a lot of hits, 
    // compared with the real gamma prompt.
    // Therefore, we calculate the ratio between the supposedly prompt and the 10 following ns
    // If that ratio is too small then we take the next higher peak.
    // CAREFULL : the delayed neutron peak must not exceed the hight of the gamma prompt

    // Integral of the peak : 
    xAxis -> SetRangeUser(maxBin_ps-5000, maxBin_ps+5000);
    auto const & integral_peak = hist -> Integral();

    // Integral of the 10 ns after the peak : 
    xAxis -> SetRangeUser(maxBin_ps+5000, maxBin_ps+15000);
    auto const & integral_10_ns = hist -> Integral();

    // Ratio between both integrals :
    auto const ratio = integral_peak/integral_10_ns;

    // If too big then move to next :
    if (ratio>2)
    {
      xAxis -> SetRangeUser(maxBin_ps+15000, xAxis -> GetXmax());
      auto const & second_maxBin = hist -> GetMaximumBin();
      auto const & second_maxBin_ps = hist -> GetBinCenter(second_maxBin);
      return second_maxBin_ps;
    }
    else return maxBin_ps;
  }
  else return maxBin_ps;
}

void Timeshiftor::analyse()
{
  bool has_RF = false; // TODO check this variable

  m_histo_ref_VS_RF.Merge();
  if(m_use_rf)
  {
    has_RF = (m_histo_ref_VS_RF.Integral() > 0);
    if (has_RF)
    {
      // Calculating the RF time shift :
      m_timeshifts[RF_Manager::label] = Time_cast(getRFGammaPrompt(m_histo_ref_VS_RF.get(), m_check_preprompt));
    }
    else 
    {
      print("ATTENTION : THIS RUN DOES NOT APPEAR TO CONTAIN ANY RF");
      print("RF label is :", RF_Manager::label);
      print("Timing reference is :", m_time_ref_label);
    }
  }

  // Loop over all the channels time spectra :
  for (Label label = 0; label<detectors.size(); label++)
  {
    if (!detectors.exists(label)) continue; // Reject unused detectors
    if (label == m_time_ref_label) {m_timeshifts[label] = 0; continue;} // Force the reference detector to be at 0;
    
    auto const & type = detectors.type(label); // Get the detector's type (dssd, ge, ...)
    auto const & name = detectors[label];// Get the name ("R3A11_blue", ...)

    if (m_verbose) print(detectors[label]);

    if (type == "eden" || type == "RF") continue;

    m_time_spectra[label].Merge();
   if (m_use_rf) m_histograms_VS_RF[label].Merge();

    // A. If RF, one can decide to use the RF time spectra to calculate the time shifts.
    // Attention !!! This works only if the normal dT peak is bewteen 0 and the RF period, otherwise there will be a shift
    if (m_use_rf && (m_RF_preferred[type] || m_RF_preferred_label[label]) && has_RF)
    {
      auto const & RF_zero = m_timeshifts[RF_Manager::label];
      if (m_histograms_VS_RF[label].Integral() < 50 ) {print("Not a lot of hits : only", m_histograms_VS_RF[label].Integral(), "for", name); continue;}
      
      if (m_edge_preferred[type] || m_edge_preferred_label[label])
      { // Not taking the maximum but the rising edge of the time spectra :
        auto const & amppic = m_histograms_VS_RF[label] -> GetMaximum();
        auto const & peak_begin = m_histograms_VS_RF[label] -> FindFirstBinAbove(amppic*0.8);
        auto const & peak_begin_ps = m_histograms_VS_RF[label]->GetBinCenter(peak_begin); // In ps
        m_timeshifts[label] = RF_zero - Timeshift_cast(peak_begin_ps);
        if (m_verbose) print( "Edge :", m_timeshifts[label], "with max =", int_cast(amppic), "counts.");
      }

      else if (m_biggest_peak_finder || m_biggest_peak_finder_label[label])
      {
        Colib::BiggestPeakFitter fitter(m_histograms_VS_RF[label].get());
        m_timeshifts[label] = RF_zero-Timeshift_cast(fitter.getMean());
        if (m_verbose) print( "Mean :", m_timeshifts[label], "with max =", int_cast(m_histograms_VS_RF[label] -> GetMaximum()), "counts.");
      }

      else
      {
        double mean = 0.;
        if (Colib::getMeanPeak(m_histograms_VS_RF[label].get(), mean)) m_timeshifts[label] = RF_zero-Timeshift_cast(mean);
        if (m_verbose) print( "Mean :", m_timeshifts[label], "with max =", int_cast(m_histograms_VS_RF[label] -> GetMaximum()), "counts.");
      }
    }

    // B. Using normal coincidence time spectra : 
    else
    {
      if (m_time_spectra[label].Integral() < 10000 ) 
      {
        print("Not a lot of hits : only", m_time_spectra[label].Integral(), "for", name);
        if (m_time_spectra[label].get())
        {
          print("Falling back to classic integration");
          auto integratedHisto = Colib::integrate(m_time_spectra[label].get());
          integratedHistos.emplace(m_time_spectra[label].get()->GetName(), integratedHisto);
          auto const & threshold = integratedHisto->GetMaximum() / 2;
          m_timeshifts[label] = Timeshift_cast(integratedHisto->GetBinCenter(integratedHisto->FindFirstBinAbove(threshold)));
        }
        else print("Skip");
        continue;
      }

      if (m_edge_preferred[type])
      {
        auto const & amppic     = m_time_spectra[label] -> GetMaximum();
        auto const & peak_begin = m_time_spectra[label] -> FindFirstBinAbove(amppic*0.8);
        auto const & peak_bins  = m_time_spectra[label] -> GetBinCenter(peak_begin);
        m_timeshifts[label] = Timeshift_cast(peak_bins); // In ps
        if (m_verbose) print( "Edge :", m_timeshifts[label], "with max =", int_cast(m_time_spectra[label] -> GetMaximum()), "counts.");
      }

      else if (m_biggest_peak_finder || m_biggest_peak_finder_label[label])
      {
        Colib::BiggestPeakFitter fitter(m_time_spectra[label].get());
        m_timeshifts[label] = Timeshift_cast(fitter.getMean());
        if (m_verbose) print( "Mean :", m_timeshifts[label], "with max =", int_cast(m_time_spectra[label] -> GetMaximum()), "counts.");
      }

      else
      {// For all the other detectors :
        double mean = 0.;
        if (Colib::getMeanPeak(m_time_spectra[label].get(), mean)) m_timeshifts[label] = Timeshift_cast(mean); else m_timeshifts[label] = 0;
        if (m_verbose) print( "Mean :", m_timeshifts[label], "with max =", int_cast(m_time_spectra[label] -> GetMaximum()), "counts.");
      }
    }
  }
}

/**
 * @brief To write down the histograms and the data
 * 
 * @param name: Will automatically create the following name as output : 
 * (name+".dT") and (name+"_dT.root")
 */
void Timeshiftor::write(std::string const & name)
{
  writeData(name);
  writeRoot(name);
}

/**
 * @brief To write down the data
 * 
 * @param name: Name of the output data file : (name+".dT")
 */
void Timeshiftor::writeData(std::string const & name)
{
  m_outPath = Path (m_outDir+m_ts_outDir, true); // /path/to/output/directory/Timeshiftor/, create it if needed
  if (!m_outPath) {m_ok = false; return;}

  File outData (m_outPath+name);
  outData.setExtension(".dT");

  std::ofstream outTimeshiftsFile(outData, std::ios::out);
  
  for (Label label = 0; label<m_timeshifts.size(); label++)
  {
    auto const & dT = m_timeshifts[label];
    if (dT != 0) outTimeshiftsFile << label << "\t" << dT << std::endl;
  }

  outTimeshiftsFile.close();

  print("Timeshifts data written to", outData);
}

/**
 * @brief To write down the histograms
 * 
 * @param name: Name of the output data file : (name+"_dT.root")
 */
void Timeshiftor::writeRoot(std::string const & name)
{
  m_outPath = Path (m_outDir+m_ts_outDir, true); // /path/to/output/directory/Timeshiftor/, create it if needed
  if (!m_outPath) {m_ok = false; return;}

  auto const outRoot = m_outPath+name+"_dT.root";

  std::unique_ptr<TFile> outFile(TFile::Open((outRoot).c_str(),"RECREATE"));
  if (outFile.get() == nullptr) {print("Cannot open file ", outRoot, " !!!\nAbort !");return;}

  m_EnergyRef.Write();
  m_time_spectra_bidim.Write();
  m_time_spectra_corrected_bidim.Write();

  if (m_use_rf)
  {
    m_histo_ref_vs_RF_VS_mult.Write();
    m_histo_ref_VS_RF.Write();
    m_time_spectra_corrected_bidim_RF.Write();
  }
  
  for (auto & histo : m_time_spectra          ) histo.Write();
  for (auto & histo : m_time_spectra_corrected) histo.Write();

  if (m_use_rf)
  {
    for (auto & histo : m_histograms_VS_RF          ) histo.Write();
    for (auto & histo : m_time_spectra_corrected_RF ) histo.Write();
  }

  for (auto const & it : integratedHistos) it.second->Write();

  outFile -> Write();
  outFile -> Close();

  print("Timeshiftor root file saved to", outRoot);
}

bool Timeshiftor::setTimeWindow_ns(Time_ns const & timewindow_ns) 
{
  if (timewindow_ns == 0) {print("NO TIME WINDOW !!"); return false; }
  m_timewindow_ns = timewindow_ns;
  m_timewindow = Time_cast(m_timewindow_ns*1000);
  print("Extracting timeshifts with a time window of ", m_timewindow_ns, " ns");
  return true;
}

bool Timeshiftor::setTimeWindow_ns(std::string const & timewindow_string) 
{
  auto const timewindow_ns = Time_ns_cast(stod(timewindow_string));
  return setTimeWindow_ns(timewindow_ns);
}

bool Timeshiftor::setTimeReference(Label const & timeRef_label)
{
  m_time_ref_label = timeRef_label;
  m_time_ref_name = detectors[m_time_ref_label];
  printC("Reference detector set to be ", m_time_ref_name, " (n°", m_time_ref_label, ")");
  return true;
}

bool Timeshiftor::setTimeReference(std::string const & timeRef_name)
{
  m_time_ref_name = timeRef_name;
  if (timeRef_name == "") {print("NO TIME REFERENCE !!"); return false; }
  // If the string contains the label number then convert the string to int (stoi)
  // If the string contains the detector name then extract the label number from the detectors list (detectors)
  return setTimeReference( Label_cast((isNumber(timeRef_name)) ? std::stoi(timeRef_name) : detectors.getLabel(timeRef_name)));
}

void Timeshiftor::setOutDir(std::string const & outDir)
{
  m_outDir = outDir;
  push_back_if_none(m_outDir, '/');
}

void Timeshiftor::setOutRoot(std::string const & outRoot) 
{
  m_outRoot = outRoot+"dT"; 
  if (extension(m_outRoot) != "root") 
  {
    m_outRoot = removeExtension(m_outRoot)+".root";
  }
}

void Timeshiftor::setOutData(std::string const & outData)
{
  m_outData = outData;
  if (extension(m_outData) != "dT") 
  {
    m_outData = removeExtension(m_outData)+".dT";
  }
}

void Timeshiftor::setOutName(std::string const & outname)
{
  m_outData = outname+".dT";
  m_outRoot = outname+"_dT.root";
}

TH1F* Timeshiftor::shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit)
{
  std::string name = histo->GetName();
  if (name == "")  return (new TH1F());
  
  auto axis = histo -> GetXaxis();

  auto bins    = axis -> GetNbins();
  auto timeMax = axis -> GetXmax ();
  auto timeMin = axis -> GetXmin ();

  auto binToTime = (timeMax-timeMin)/bins;
  Time time_to_ps = 1;

  if (unit != "ps")
  {
    if (unit == "ns") time_to_ps*=1000;
    else if (unit == "clock") time_to_ps*=128; // Deprecated
    else { print("UNKOWN TIME UNIT", unit); return nullptr;}
  }

  auto corrected_histo = static_cast<TH1F*>(histo -> Clone((name+"_corrected").c_str()));

  for (int bin = 1; bin<bins; bin++)
  {
    auto const time_ps = time_to_ps*Timeshift_cast(binToTime*bin + timeMin); // time_to_ps * (coeff*bin + offset_time)
    auto const new_time_ps = time_ps + m_timeshifts[label];
    auto const new_time = new_time_ps/time_to_ps;
    auto const newbin = int_cast(( double_cast(new_time) - timeMin ) / binToTime); // ( (time_ps/time_to_ps) - offset_time) / coeff

    if( newbin>0 && newbin < bins )
    {
      corrected_histo -> SetBinContent(newbin, histo->GetBinContent(bin));
    }
  }
  return corrected_histo;
}

std::ostream& operator<<(std::ostream& out, Timeshiftor const & ts)
{
  out << ts.get();
  return out;
}


#endif //TIMESHIFTOR_HPP

/*
// Now create the correctly shifted time spectra :
// for (size_t label = 0; label<m_time_spectra.size(); label++)
// {
//   auto & histo = m_time_spectra[label];
//   if (histo) 
//   {
//     m_time_spectra_corrected[label].reset( shiftTimeSpectra(histo, label) );
//     AddTH1(m_time_spectra_corrected_bidim.get(), m_time_spectra_corrected[label].get(), label, true);
//   }

//   auto & histo_RF = m_histograms_VS_RF[label];
//   if (histo_RF) 
//   {
//     m_time_spectra_corrected_RF[label].reset( shiftTimeSpectra(histo_RF, label) );
//     AddTH1(m_time_spectra_corrected_bidim_RF.get(), m_time_spectra_corrected_RF[label].get(), label, true);
//   }
// }
*/