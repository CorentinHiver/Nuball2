#ifndef CALIBRATOR_HPP
#define CALIBRATOR_HPP

#include "../libRoot.hpp"

#include "../Analyse/SpectraCo.hpp"

#include "../Classes/Hit.hpp"
#include "../Classes/Fit.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/Detectors.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/FilesManager.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTRootReader.hpp"
#include "../MTObjects/MTTHist.hpp"

/**
 * @brief Allows one to calculate and verify calibration coefficients for various sources and detectors for Nuball2
 * @todo Calibrator::verify() DO NOT SUPPORT multithreading with MTObject !!
 */
class Calibrator
{
public:

  Calibrator() = default;
  ~Calibrator() {if (readFile) delete readFile;}

  /// @brief Loading calibration from file name
  bool loadCalibration(Calibration const & calib) {return (m_calib = calib);}
  bool loadCalibration(std::string const & calib_file) {return (m_calib = calib_file);}

  void calculate(std::string const & dataDir, int nb_files = -1, std::string const & source = "152Eu", std::string const & type = "fast");

  /// @brief Calculate calibration from .root histograms
  /// @attention TODO
  void calculate(std::string const & histograms, std::string const & source = "152Eu");

  void calculate2(std::string const & histogramsFilename, std::vector<double> peaks, std::string const & fit_info_file = "fit_info.data");

  // void calculateInteractive(std::string const & histogramsFilename, std::string const & fit_info_file = "fit_info.data", std::vector<double> peaks);

  void Initialise() 
  {
    if (m_Initialised) return;
    m_histos.Initialise(); 
    m_fits.resize(1000); 
    if (!detectors) throw Detectors::Error(); 
    m_Initialised = true;
  }
  
  void loadRootData(std::string const & dataDir, int const & nb_files = -1);
  static void loadRootDataThread(Calibrator & calib, MTList & list);
  void fillRootDataHisto(std::string const & filename);

  void loadFasterData(std::string const & dataDir, int const & nb_files = -1);
  static void fillHisto(Hit & hit, FasterReader & reader, Calibrator & calib);
  void loadFitInfo(std::string const & fit_info_file);
  void analyse(std::string const & source = "152Eu");
  void analyse2(std::vector<double> peaks);
  void peakFinder(std::string const & source);
  void fitCalibration(Fits & fits);
  void writePosPeaks(std::string const & outfilename);
  void writeData(std::string const & outfilename);
  void writeRawRoot(std::string const & outfilename);

  void verify(std::string const & outfilename = "verify");
  void writeCalibratedHisto(std::string const & outfilename);

  void setSource(std::string const & source) {m_source = source;}
  void verbose(bool const & _verbose) {m_verbose = _verbose;}

  void loadRootHisto(std::string const & histograms);

  Calibration const & calibration() const {return m_calib;}
  Calibration & calibration() {return m_calib;}

  void calibrateFasterData(std::string const & folder, int const & nb_files = -1);
  void calibrateRootData(std::string const & folder, int const & nb_files = -1);
  bool const & calibrate_data() const {return m_calibrate_data;}
  bool const & calibrate_data() {return m_calibrate_data;}


  /// @brief @todo
  operator bool() const & {return (true);}

  auto const & calib() const {return m_calib;}
  auto & calib() {return m_calib;}

  static void treatOnlyParis(bool const & b = true) {m_treatOnlyParis = b;}
  static void treatOnlyGe(bool const & b = true) {m_treatOnlyGe = b;}

private:
  //Private methods :
  void printParameters();

  // Calibration data :
  Calibration m_calib;

  //Attributs for the calculations :
  bool      m_verbose     = false;
  bool      m_Initialised = false;
  bool      m_residus     = false;
  bool      m_outRoot_b   = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibrator/";

  static bool m_treatOnlyParis;
  static bool m_treatOnlyGe;
  bool m_calibrate_data = false;

  Fits m_fits = Fits(100);

  Path dataPath;

  TFile* readFile = nullptr;

  // FOR ANALYSE2 :
  
  // std::map<Label, int> m_rebin;
  std::map<int, int> m_rebin;
  // std::map<Label, double> m_threshold;
  std::map<int, double> m_threshold;
  // std::map<Label, int> m_nb_bins_below;
  std::map<int, int> m_nb_bins_below;

  bool m_histo_loaded = false;


public:
  struct histograms
  {
    // Raw spectra :
    Vector_MTTHist<TH1F> raw_spectra;
    MTTHist<TH2F> all_raw_spectra;

    // Calibrated spectra :
    Vector_MTTHist<TH1F> calib_spectra;
    Vector_MTTHist<TH2F> all_calib;

    // Other spectra :
    std::map<Label, SpectraCo> spectra;

    void Initialise();
    void setTypeBins(std::string const & parameters);
    bool Initialised = false;
  } m_histos;
};

bool Calibrator::m_treatOnlyParis = false;
bool Calibrator::m_treatOnlyGe = false;

void Calibrator::histograms::Initialise()
{
  if (Initialised) return;

  if (detectors)
  {
    auto const & max_label = detectors.size();
    if (max_label == 0) 
    {
      print("Error using Detector class in Calibrator module."); 
      throw std::runtime_error(error_message["DEV"]);
    }
    // All the detectors spectra in one plot :
    all_raw_spectra.reset("All_detectors", "All detectors", 
        max_label,0,max_label, detectors.ADCBin("default").bins,0,detectors.ADCBin("default").min);

    // All the calibrated detectors spectra of each type in one plot :
    all_calib.resize(detectors.nbTypes());
    for (auto const & type : detectors.types())
    {
      auto nb_detectors = detectors.nbOfType(type);
      auto const & binning = detectors.energyBidimBin(type);
      all_calib[detectors.typeIndex(type)].reset(("All_"+type+"_spectra").c_str(), ("All "+type+" spectra").c_str(), 
          nb_detectors,0,nb_detectors, binning.bins,binning.min,binning.max);
    }

    // All the raw and/or calibrated spectra in a separate spectra :
    calib_spectra.resize(max_label);
    raw_spectra.resize(max_label);
    for (auto const & type : detectors.types())
    {
      if (type == "null" || type == "RF") continue;
      for (int index = 0; index<detectors.nbOfType(type); index++)
      {
        auto const & name = detectors.name(type, index);
        auto const & label = detectors.label(type, index);
        auto const & bin_raw = detectors.ADCBin(type);
        auto const & bin_calib = detectors.energyBin(type);
        raw_spectra[label].reset((name+"_raw").c_str(), (name+" raw spectra").c_str(), bin_raw.bins, bin_raw.min, bin_raw.max);
        calib_spectra[label].reset((name+"_calib").c_str(), (name+" calibrated spectra").c_str(), bin_calib.bins, bin_calib.min, bin_calib.max);
      }
    }
    Initialised = true;
  }
}

void Calibrator::histograms::setTypeBins(std::string const & parameters)
{
  std::istringstream param(parameters);
  auto & ADCbins = detectors.getADCBin();
  auto & Energybins = detectors.getEnergyBin();
  for (std::string line; std::getline(param,line);)
  {
    std::istringstream is(line);
    std::string type;
    std::string which_histo;
    is>>type;
    if (type == "null") {throw_error(type+"type is not recognized for binning in Calibrator");}
    is >> which_histo;
         if (which_histo == "raw"  ) is >> ADCbins[type].bins >> ADCbins[type].min >> ADCbins[type].max;
    else if (which_histo == "calib") is >> Energybins[type].bins >> Energybins[type].min >> Energybins[type].max;
    else {throw_error(which_histo+" histo of Calibrator module not recognized ");}
  }
}

void Calibrator::calculate(std::string const & dataDir, int nb_files, std::string const & source, std::string const & type)
{
  print ("Calculating calibrations from raw data in", dataDir);

  if(type == "fast") this -> loadFasterData(dataDir, nb_files);
  else if (type == "root") this -> loadRootData(dataDir, nb_files); // UNTESTED
  else {print(type, "unkown data format"); return;}

  this -> analyse(source);
  this -> writeData(source+".calib");
  this -> writeRawRoot(source+".root");
}

/**
 * @brief Calculate the calibration coefficients from already calculated histograms
 * @attention The name of the histograms must correspond either to the name of the
 * detector or the label declared in the ID file
 */
void Calibrator::calculate(std::string const & histograms, std::string const & source)
{
  print ("Calculating calibration coefficients from histogram data in", histograms);
  this -> Initialise();
  this -> loadRootHisto(histograms);
  this -> analyse(source);
  this -> writeData(source+".calib");
  this -> writeRawRoot(source+".root");
}

void Calibrator::loadFitInfo(std::string const & fit_info_file)
{
  std::ifstream file(fit_info_file, std::ios::in);
  if (!file.good()) throw_error(concatenate("CANT OPEN ", fit_info_file));
  std::string line;
  getline(file, line);
  auto m_header = getList(line, ' ');

  std::string name;
  int rebin = 0;
  double threshold = 0;
  int nb_bins_below = 0;

  while(getline(file, line))
  {
    std::istringstream iss(line);
    iss >> name;
    auto label = detectors[name];
    iss >> rebin >> threshold >> nb_bins_below;
    m_rebin[label] = rebin;
    m_threshold[label] = threshold;
    m_nb_bins_below[label] = nb_bins_below;
  }
}

void Calibrator::calculate2(std::string const & histogramsFilename, std::vector<double> peaks, std::string const & fit_info_file)
{
  print ("Calculating calibration coefficients from histogram data in", histogramsFilename, "version 2");
  this -> Initialise();
  this -> loadFitInfo(fit_info_file);
  this -> loadRootHisto(histogramsFilename);
  this -> analyse2(peaks);
  this -> writeData("calibration.calib");
  this -> writeRawRoot("histograms.root");
}

/**
 * @brief Loads non-calibrated spectra
 * 
 * @param histograms 
 */
void Calibrator::loadRootHisto(std::string const & histograms)
{
  this -> Initialise();
  m_histo_loaded = true;
  
  print("Loading histograms ...");
  
  readFile = TFile::Open(histograms.c_str());
  readFile->cd();
  if (!readFile || !readFile->IsOpen()) throw_error("Can't open"+histograms);
  auto histos (loadFormattedTH1F(readFile));
  for (auto const & it : histos) 
  {
    auto const & label = it.first;
    auto const & histo = it.second;
    // m_histos.raw_spectra[label] = histo;
    m_histos.spectra[label].load(histo);
  }
  // for (auto const & spectra : m_histos.spectra) print(spectra);
}

void Calibrator::loadFasterData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());

  // First initialises the histograms :
  this -> Initialise();

  // Then create the data reader :
  MTFasterReader *mt_reader = new MTFasterReader();
  //Setup the files to read :
  mt_reader->addFolder(dataDir, nb_files);
  // Gives the reader the function to use on each hit :
  mt_reader->readRaw(fillHisto, *this);
  delete mt_reader;
  print("Data loaded");
}

void Calibrator::loadRootData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialise();
  FilesManager files;
  files.addFolder(dataDir, nb_files);
  MTList list(files.getListFiles());
  MTObject::parallelise_function(loadRootDataThread, *this, list);
  print("Data loaded");
}

void Calibrator::loadRootDataThread(Calibrator & calib, MTList & list)
{
  std::string filename = "";
  while(list.getNext(filename))
  {
    calib.fillRootDataHisto(filename);
  }
}

void Calibrator::fillRootDataHisto(std::string const & filename)
{
  unique_TFile file(TFile::Open(filename.c_str(), "READ"));
  std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball2"));
  if (!tree.get()) {print("NO Nuball2 found in", filename); return;}

  Event event;
  event.reading(tree.get(), "leq");

  print("Reading", filename);

  auto const & nb_hits = tree->GetEntries();

  if (m_calibrate_data) for (long evt = 0; evt<nb_hits; evt++)
  {
    tree->GetEntry(evt);
    for (int hit = 0; hit<event.mult; hit++)
    {
      auto const & label = event.labels[hit];
      if (label == 0) continue;
      if (isParis[label] && event.qdc2s[hit]!=0)
      {
        auto const & ratio = (event.qdc2s[hit]-event.adcs[hit])/event.qdc2s[hit];
        if (ratio<-0.1 || ratio>0.2) continue;
      }
      auto const nrjcal = m_calib(event.adcs[hit], label);
      m_histos.calib_spectra[label].Fill(nrjcal);
      m_histos.all_calib[detectors.typeIndex(label)].Fill(compressedLabel[label], nrjcal);
    }
  }
  else for (long evt = 0; evt<nb_hits; evt++)
  {
    tree->GetEntry(evt);
    for (int hit = 0; hit<event.mult; hit++)
    {
      auto const & label = event.labels[hit];
      if (label == 0) continue;
      m_histos.raw_spectra[label].Fill(event.adcs[hit]);
    }
  }
}

/**
 * @brief Fills histograms
 * @details
 * There are two modes : 
 *    You can either fill the histograms with raw values
 *    Or you can use the loaded or calculated calibration factors to fille calibrated histograms
 * 
 * @param hit 
 * @param reader 
 * @param calib 
 */
void Calibrator::fillHisto(Hit & hit, FasterReader & reader, Calibrator & calib)
{
  // If the option to calibrate histograms is on :
  if (calib.calibrate_data()) while(reader.Read()) 
  {
    if (calib.calibration().getOrder()[hit.label]<1) continue;
    auto const & nrj_cal = calib.calibration()(hit.adc, hit.label);
    calib.m_histos.calib_spectra[hit.label].Fill(nrj_cal);
    calib.m_histos.all_calib[detectors.typeIndex(hit.label)].Fill(compressedLabel[hit.label], nrj_cal);
  }
  // Fills raw values :
  else while(reader.Read()) {calib.m_histos.raw_spectra[hit.label].Fill(hit.adc);}
}

bool isTripleAlpha(std::string const & source_name)
{
  return (source_name == "3-alpha" || source_name == "3alpha"  ||
          source_name == "triple-alpha" || source_name == "triplealpha");
}

void Calibrator::peakFinder(std::string const & source)
{
  int nb_pics = 0;
  double E_right_pic = 0.f;
  for (Label label = 0; label<detectors.size(); label++)
  {
    if (!detectors.exists(label)) continue;

    auto & histo = m_histos.raw_spectra[label];
    auto const & name = detectors[label];
    auto const & type = detectors.type(label);

    if (histo.Integral()==0)
    {
      information(name, "has no data in this run");
      continue;
    }

    histo.Merge();
    
    auto & fit = m_fits[label];
    fit.setLabel(label);
    print(name);

    if (m_verbose) {print(); print(name);}

    // Initialise algorithm parameters :
   
    double integral_ratio_threshold = 0.f;  // The threshold used to tag the peak.
    int ADC_threshold = 0; // A threshold used in order to reject any potential electrical noise below like 500 ADC
    int window_1 = 0, window_2 = 0; // The three windows width (in keV)

    auto & peaks = fit.peaks;
    int minHitsToCalibrate = 50000;

    // Handles the triple alpha
    if (isTripleAlpha(source) && type!="dssd") continue;
    else if (!isTripleAlpha(source) && type=="dssd") continue;

    if (m_treatOnlyParis && type!="paris") continue;
    if (m_treatOnlyGe && type!="ge") continue;
    
    if (type == "ge")
    {// For Clovers
      window_1 = 10, window_2 = 10;
      if (source == "152Eu")
      {
        nb_pics = 5;
        peaks.resize(nb_pics);
        peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.011f;
        if (label == 69 || label == 70) integral_ratio_threshold = 0.009f;
        ADC_threshold = 100;
      }
      else if (source == "232Th")
      {
        nb_pics = 4;
        peaks.resize(nb_pics);
        peaks = {238, 583, 910, 2614};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.0049f;
        ADC_threshold = 100;
      }
      else if (source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        peaks.resize(nb_pics);
        peaks = {1172, 1333};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.06f;
        ADC_threshold = 100;
      }
    }
    else if (type == "labr")
    {// For labr
      window_1 = 70, window_2 = 50;
      if (source == "152Eu")
      {
        nb_pics = 5;
        peaks.resize(nb_pics);
        peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.012f;
        ADC_threshold = 500;
      }
      else if (source == "232Th")
      {
        nb_pics = 4;
        peaks.resize(nb_pics);
        peaks = {238, 583, 911, 2614};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.0025f;
        ADC_threshold = 500;
      }
      else if (source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        peaks.resize(nb_pics);
        peaks = {1172, 1333};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.1f;
        ADC_threshold = 100;
      }
    }
    else if (type == "paris")
    {// For paris
      window_1 = 80, window_2 = 50;
      if (source == "152Eu")
      {
        // if (name.find("FR1")) 
        // {
        //   nb_pics = 4;
        //   peaks.resize(nb_pics);
        //   peaks = {344.2760, 778.9030, 964.1310, 1408.0110};
        //   E_right_pic = peaks.back();
        //   integral_ratio_threshold = 0.030f;
        // }
        // else
        // {
        nb_pics = 5;
        peaks.resize(nb_pics);
        peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.011f;
        // }
        ADC_threshold = 500;
      }
      else if (source == "232Th")
      {//never tested yet - but should be similar to fatima
        nb_pics = 4;
        peaks.resize(nb_pics);
        peaks = {238, 583, 911, 2614};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.0023f;
        ADC_threshold = 500;
      }
    }
    else if (type == "dssd")
    {
      if (isTripleAlpha(source))
      {
        window_1 = 100, window_2 = 80;
        nb_pics = 3;
        peaks.resize(nb_pics);
        peaks = {5150, 5480, 5800};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.05f;
        ADC_threshold = 500;
        minHitsToCalibrate = 0;
      }
      else
      {
        if (m_verbose) {print("DSSD not to be taken in this calibration file");}
      }
    }
    else if (type == "bgo")
    {
      
      if (source == "152Eu")
      {
        nb_pics = 0;
        E_right_pic = 1408.0110;
        integral_ratio_threshold = 0.027f;
        ADC_threshold = 500;
      }
      else if (source == "232Th")
      {//never tested yet - but should be similar to fatima
        nb_pics = 0;
        E_right_pic = 2614;
        integral_ratio_threshold = 0.02f;// TBD
        ADC_threshold = 500;
      }
    }
    else {if (m_verbose) print("Detector", name, "not handled !"); continue;}
    fit.resize(nb_pics);// Resize the intern vectors of the detector's Fit

    int vmaxchan = 0;// Position of the right pic in bins (e.g. the 1408 keV in Eu)
    double & scalefactor = fit.scalefactor; // To adapt to the binnin of the histogram (ADC/bin)
    double & kpc = fit.keVperADC; // keV per bin (=channel)

    // -----------------------
    // OPERATE ON THE SPECTRUM
    // -----------------------
    auto const & nbins = histo->GetXaxis()->GetNbins();// The number of bins in the spectra

    // NB: It supposes that the spectra starts at 0 !!
    scalefactor = histo->GetXaxis()->GetXmax()/nbins;

    auto ADC_threshold_scaled = ADC_cast(ADC_threshold/scalefactor); 
    auto sum=histo->Integral(ADC_threshold_scaled, nbins-1); // The total integral of the spectra
    fit.integral = sum;
    if (sum < minHitsToCalibrate)
    {// No calibration if not enough counts in the spectra
      if (m_verbose)
      {
        fit.too_few_counts(true);
        fit.exists(false);
        std::cout << "Too few counts to calibrate : " << sum << std::endl;
      }
      continue;
    }
    if (m_verbose)
    {
      std::cout << "Integral = " << sum << " counts; Number of bins = " <<  nbins << std::endl;
      std::cout << "scale factor = " << scalefactor << std::endl;
    }

    AddTH1(m_histos.all_raw_spectra.get(), histo.get(), label);
    
    // Initialises vmaxchan at the last bin (right of the spectra)
    vmaxchan=nbins;
    double integral=0; // Holds the surface below the spectra from the highest bin to the current bin in the following loop

    // We start at nbins-2 because the nbin'th bin is the overflow, so to be secure we start at nbin-2
    for (int j=nbins-2; j > ADC_threshold_scaled; j--)
    {
      integral+=histo->GetBinContent(j);
      if ((integral/sum) > integral_ratio_threshold) {vmaxchan=j; break;}
    }

    if (vmaxchan==ADC_threshold_scaled) { if (m_verbose) print("Could not fit this spectrum "); continue;}
    if (m_verbose) print("right pic found at channel ", vmaxchan*scalefactor, " ADC");
    kpc=E_right_pic/vmaxchan;
    if (m_verbose) print("kev per channel = ", kpc);

    if (type == "bgo")
    {
      // For BGOs, already "gain matched", a simple affine relation roughly calibrates
      fit.exists(true);
      fit.order = 1;
      fit.parameter0 = 0;
      fit.parameter1 = kpc/scalefactor;
      continue;
    }
    // for (int j=0; j<nb_pics; j++) //Starting with the lower energy pic
    auto xaxis = histo->GetXaxis();
    for (int j=nb_pics-1; j>-1; j--) //Starting with the higher energy pic
    {
      if (m_verbose) print("Pic : " , peaks[j]);
      // 1st window :
      double p=peaks[j]; // Energy of the peak (in keV)
      int cguess_low  = int_cast((p-window_1)/kpc); // Low  edge of the window (in bins)
      int cguess_high = int_cast((p+window_1)/kpc); // High edge of the window (in bins)
      if (cguess_low<ADC_threshold_scaled) cguess_low = ADC_threshold_scaled; //cannot read the spectra below the ADC threshold !
      xaxis->SetRange(cguess_low, cguess_high); // Setup the window to the spectra
      double cmc = 0.5 + histo->GetMaximumBin();
      if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cmc*scalefactor);

      // 2nd window :
      cguess_high = int_cast(0.5 + cmc + window_2/kpc);// +0.5 in order to take the centre of the bin
      cguess_low  = int_cast(0.5 + cmc - window_2/kpc);
      xaxis->SetRange(cguess_low, cguess_high);
      double cm  = histo->GetMean(); //in ADC (scalefactor*bins)
      cmc = cm/scalefactor;
      if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cm);

      // // 3rd window :
      // cguess_high = int_cast(cmc );
      // cguess_low  = int_cast(cmc );
      // xaxis->SetRange(cguess_low, cguess_high);
      // cm  = histo->GetMean(); //in ADC
      // cmc = cm/scalefactor; //in bins
      // if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cm);
      fit.cmeasures[j]=cmc; //The measured channel number

      // Fit of the peak :
      // !!! ATTENTION : THE FIT IS TO BE IMPROVED  !!!
      double constante = histo -> GetMaximum();
      double mean = cmc;
      double sigma = (histo -> FindLastBinAbove(constante/2) - histo -> FindFirstBinAbove(constante/2))/2.35;
      TF1*  gaus(new TF1("gaus","gaus"));
      gaus -> SetRange(cguess_low*scalefactor, cguess_high*scalefactor);
      gaus -> SetParameter(0, constante);
      gaus -> SetParameter(1, mean);
      gaus -> SetParameter(2, sigma);
      histo -> Fit(gaus,"RQN+");
      fit.mean [j] = gaus -> GetParameter(1);
      fit.sigma[j] = gaus -> GetParameter(2);
      TF1* gaus_pol0(new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)"));
      gaus_pol0 -> SetRange(cguess_low*scalefactor, cguess_high*scalefactor);
      gaus_pol0 -> SetParameter(0, gaus -> GetParameter(0));
      gaus_pol0 -> SetParameter(1, gaus -> GetParameter(1));
      gaus_pol0 -> SetParameter(2, gaus -> GetParameter(2));
      // gaus_pol0 -> SetParameter(3, histo -> GetBinContent(0));
      histo -> Fit(gaus_pol0,"RQ+");
      if (label == 25) print(kpc, gaus_pol0 -> GetParameter(1), gaus_pol0 -> GetParameter(2)/gaus_pol0 -> GetParameter(1));
      // histo -> Fit(gaus_pol0,"RIQE");
      // fit.mean [j] = gaus_pol0 -> GetParameter(1);
      // fit.sigma[j] = gaus_pol0 -> GetParameter(2);
      // print(label, "kpc : ", kpc, " scale : ", scalefactor, " pic : ", p, " -> ", cguess_low*kpc, " : ", cguess_high*kpc);
      // print(label, " pic : ", p, " -> ", cm, " ", histo -> GetBinContent(0));

      // If the peak is the higher in energy, then use it as a little bit more precise rough linear calibration :
      if (j == nb_pics) kpc = cmc; 
    }

    xaxis -> UnZoom();
  }
}

void Calibrator::fitCalibration(Fits & fits)
{
  for (Label label = 0; label<detectors.size(); label++)
  {
    Fit & fit = fits[label];
    auto const & nb_pics = fit.peaks.size();
    auto & x  = fit.x ;
    auto & y  = fit.y ;
    auto & ex = fit.ex;
    auto & ey = fit.ey;
    
    x .resize(nb_pics);
    y .resize(nb_pics);
    ex.resize(nb_pics);
    ey.resize(nb_pics);

    for (size_t j=0; j < nb_pics; j++)
    {
      if (m_verbose) std::cout << "Energy = " << fit.peaks[j] << " Channel = " << fit.cmeasures[j]*fit.scalefactor << std::endl;
      x [j]=fit.cmeasures[j]*fit.scalefactor;
      y [j]=fit.peaks[j];
      ex[j]=0;
      ey[j]=0;
    }

    // If faudrait aussi revoir ce fit ici ! Et éventuellement les erreurs
    auto c1 = new TCanvas(("c_"+fit.name).c_str());
    c1->cd(1);
    TGraphErrors* gr = new TGraphErrors(nb_pics,x.data(),y.data(),ex.data(),ey.data());
    gr -> SetName((fit.name+"_gr").c_str());
    TF1* linear(new TF1("lin","pol1")); //Range and number of fit parameters
    gr->Fit(linear,"q");
    TF1* binom (new TF1("pol", "pol2"));

    if(isGe[label] || isDSSD[label] || isParis[label])
    {//First order fit
      fit.order = 1;
      fit.parameter0 = linear -> GetParameter(0);
      fit.parameter1 = linear -> GetParameter(1);
      fit.chi2       = linear -> GetChisquare( );
      fit.exists(true);
    }
    else if (isLaBr3[label] || isParis[label])
    {// Second order fit
      fit.order = 2;
      binom -> SetParameters(0, linear -> GetParameter(0));
      binom -> SetParameters(1, linear -> GetParameter(1));
      gr -> Fit(binom,"q");
      fit.parameter0 = binom -> GetParameter(0);
      fit.parameter1 = binom -> GetParameter(1);
      fit.parameter2 = binom -> GetParameter(2);
      fit.chi2       = binom -> GetChisquare( );
      fit.exists(true);
    }
    else {
      fit.exists(false);
    }
  }
}

/**
 * @brief Uses the second version of the peak finder to extract calibration coefficients
 * 
 * @details
 * Uses the second derivative spectra in order to find the peaks.
 * To do so, uses an input file to set the three main parameter for each detector : 
 *  The number of bins to use
 *  The threshold for peak detection
 *  The number of bins below threshold (by default 2)
 * 
 * The spectra's name must be the detector's name (ex R3A1_red)
 * 
 * @param source 
 */
void Calibrator::analyse2(std::vector<double> peaks)
{
  for (auto const & it : detectors.labels())
  {
    auto const & name = it.first;
    auto const & label = it.second;

    debug("coucou1");

    if (!find_key(m_histos.spectra, label)) {printC(GREY, name, " spectra not found ", RESET); continue;}
    debug("coucou2");

    auto const & rebin = m_rebin[label];
    auto const & threshold = m_threshold[label];
    auto const & nb_bins_below = m_nb_bins_below[label];

    print("Treating", name);

    auto & spectraCo = m_histos.spectra[label];
    spectraCo.removeBackground(rebin); // Allows for a cleaner peak finding
    spectraCo.derivate2(rebin); // Calculate the second derivative spectra
    debug("coucou3");

    // Finds the peaks and store them in points (vector of pair<int, double>) :
    auto points = spectraCo.findPeaks(threshold, nb_bins_below); 

    if (points.size()<1) {print("NO PEAK FOR", name); continue;}// Check that points have been found

    // Each point contains the bin number of the peak and the height of the peak (value)
    std::vector<int> peaks_bins; std::vector<double> peaks_value; 
    unpack(points, peaks_bins, peaks_value);

    // Order the peaks from highest to smallest
    std::vector<int> m_ordered_indexes; 
    bubble_sort(peaks_value, m_ordered_indexes); // Ordered from lower to higher value
    invert(m_ordered_indexes);

    debug("coucou5");
    // Extracts the five highest peaks :
    std::vector<int> five_max_peaks;
    for (size_t index_i = 0; (index_i<peaks.size() && index_i<peaks_bins.size()); index_i++)
    {
      auto const & peak_i = m_ordered_indexes[index_i];
      auto const & bin = peaks_bins[peak_i];
      print(
        spectraCo.getX(bin),
        spectraCo[bin]
      );
      five_max_peaks.push_back(bin);
    }
    
    debug("coucou6");
    // Finds the higher energy peak (hep)
    auto const & hep_bin = maximum(five_max_peaks); // highest energy peak bin
    auto const & hep_ADC = spectraCo.getX(hep_bin); // highest energy peak ADC
    auto const & hep_keV = maximum(peaks);          // highest energy peak energy

    // First rough linear calibration :
    auto const & ADC_to_keV = hep_ADC/hep_keV; // Rough conversion from ADC to energy in keV
    auto const & keV_to_ADC = 1/ADC_to_keV; // Rough conversion from energy in keV to ADC

    // Some helpers : 
    auto const & nb_peaks = peaks.size();

    std::vector<double> ADC; ADC.reserve(nb_peaks);
    debug("coucou7");
    // print(five_max_peaks);

    // for (auto const & peak : peaks)
    // {
    //   auto const & dumb_adc = peak/ADC_to_keV;
    //   print(five_max_peaks);
    // }
  }
}

/**
 * @brief Analyse the spectra to extract calibration coefficients
 * @details
 * The peak finding follows the following principle : 
 * We start from the bin at the very right side of the spectra.
 * Then we add the value of the bin to the integral counter.
 * Then we add the value of the next bin on the left, then the next, etc.. 
 * That is, we integrate the spectra from right to left
 * The moment the first peak is found, the higher energy one, the integral will suddenly increase
 * Then we have to determine a threshold above which we say "we have found the first peak"
 * From this we determine a really rough first linear calibration.
 * This allows us to find, for each other peak, an energy windows in which it should be.
 * Once this window established, we find its centroid.
 * Then we create a smaller window and find again the centroid.
 * A third window (which may not me important ?) event narrower is set around the peak.
 * Then the peak is fitted and the mean value of the gaussian fit added to the calibration curve.
 * Finally, the fit of the calibration curve gives the calibration coefficients.
 * 
 * The threshold is taken as the ratio between the integral and the total integral of the spectra, 
 * so that the process do not depend neither on different counting rates nor on different calibration duration.
 * Only issue : it depends on the kind of detector and to some extend to the geometry. That is, this calibration
 * is not well suited for paris detectors... Also, if a peak is absent due to for instance high energy threshold 
 * of the detector (typically 121keV of 152Eu is absent in some noisy channels) then the calibration will fail
 * 
 * @attention The most difficult part is to find the value of the threshold, wich must be different for each kind of detector. 
 * If it is different for differents detectors of the same type (e.g. paris) then the calibration requires additionnal work.
 * 
 * @attention Take care of the binning of the spectra. If there is too much or not enough bins then the peak fitting will fail, 
 * if the maximum ADC value is lower than the higher energy peak then the peak findind will fail. Also, everything supposes the
 * minimum bin corresponds to 0, otherwise it might fail.
 * 
 */
void Calibrator::analyse(std::string const & source)
{
  print("Exctracting calibration parameters from spectra");
  // -----------------------------
  // Parameterize the pics to fit :
  // -----------------------------
  peakFinder(source);
  fitCalibration(m_fits);
}


void Calibrator::writePosPeaks(std::string const & outfilename)
{
  File outFile(outfilename);
  outFile.setExtension("peaks");
  outFile.makePath(); // Create the path if it doesn't already exist

  std::ofstream fout(outFile);

  for (auto const & fit : m_fits) if (fit.found_peaks()) 
  {
    fout << fit.label() << " ";
    for (size_t i = 0; i<fit.x.size(); i++) 
      fout << fit.x[i] << " " << fit.y[i] << " " << fit.ex[i] << " " << fit.ey[i] << std::endl;
  }

  print("Peaks position written to", outfilename);
}

void Calibrator::writeData(std::string const & outfilename)
{
  File outFile(outfilename);
  outFile.setExtension("calib");
  outFile.makePath(); // Create the path if it doesn't already exist

  std::ofstream outfile(outFile);
  for (auto const & fit : m_fits) if (detectors.exists(fit.label())) outfile << fit;
  print("Data written to", outfilename);
}

void Calibrator::writeRawRoot(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  m_histos.all_raw_spectra.Write();
  for (auto & histo : m_histos.raw_spectra) {histo.Write();}
  print("Raw root spectra written to", outfilename);
}

/**
 * @brief if some spectra and calibration coefficients have been loaded,
 * or after the calibration coefficients have been calculated, this
 * allows one to check the calibration
 * 
 * @param outfilename 
 */
void Calibrator::verify(std::string const & outfilename)
{
  if (!detectors) throw Detectors::Error();
  std::vector<size_t> nb_det_filled(detectors.nbTypes(), 0);
  print("Verification of the calibration");
  for (auto const & it : detectors.labels()) 
  {
    auto const & name = it.first;
    auto const & label = it.second;

    if (!m_histo_loaded)
    {// If the raw spectra come from the calibration calculation
      auto & raw_histo = m_histos.raw_spectra[label];
      raw_histo.Merge();
      m_histos.spectra[label].load(raw_histo.get());
    }

    auto & spectrum = m_histos.spectra[label];
    print(spectrum);

    if (spectrum.integral()<1) {if (m_verbose) information(name, "has no hit"); continue;}
    auto & fit = m_fits[label];

    // Extract useful information : 
    auto const & type = detectors.type(label);
    auto const & type_index = detectors.typeIndex(type);
    if (!fit.exists()) 
    {
      if (m_verbose) information(name, "has no fit"); 
      if (m_calib && m_calib.size()>=label) 
      {
        information("reading Calibration module");
        information(label, (int)m_calib.order(label), m_calib.intercept(label), m_calib.slope(label));
        if (fit.order<0) continue;
        fit.order = m_calib.order(label);
        fit.parameter0 = m_calib.intercept(label);
        fit.parameter1 = m_calib.slope(label);
        fit.parameter2 = m_calib.binom(label);
      }
      else continue;
    }

    spectrum.name() = spectrum.name() + "_calib";
    spectrum.calibrate(m_calib, label);
    double type_id = double_cast(compressedLabel[label]);
    auto & sum_histo =  m_histos.all_calib[type_index];
    for (int bin = 0; bin<sum_histo->GetNbinsY(); bin++)
    { // Target is sum_histo, source is spectrum
      auto const & target_X_value = sum_histo->GetYaxis()->GetBinCenter(bin);
      auto const & source_bin = spectrum.getBin(target_X_value);
      sum_histo->SetBinContent(type_id, bin, spectrum.interpolate(source_bin));
    }
    nb_det_filled[type_index]++;
  }
  auto file = TFile::Open((outfilename+"_calib.root").c_str(), "recreate");
  file->cd();
  for (auto & histo : m_histos.all_calib) if (histo && !histo->IsZombie() && histo->Integral() > 0) histo->Write();
  for (auto & it : m_histos.spectra) 
  {
    it.second.setActualRange();
    it.second.write(file);
  }
  file->Write();
  file->Close();
  print(outfilename+"_calib.root", "written");
}

void Calibrator::writeCalibratedHisto(std::string const & outfilename)
{
  auto outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : m_histos.all_calib) if (histo.get()) {histo->Write();}
  for (auto & histo : m_histos.calib_spectra) if (histo.get()) {print("writting", histo.name()); histo->Write();}
  outFile->Write();
  outFile->Close();
  print("Calibrated root spectra written to", outfilename);
}

void Calibrator::calibrateFasterData(std::string const & folder, int const & nb_files )
{
  m_calibrate_data = true;
  this -> loadFasterData(folder, nb_files);
}

void Calibrator::calibrateRootData(std::string const & folder, int const & nb_files)
{
  m_calibrate_data = true;
  this -> loadRootData(folder, nb_files);
}

#endif //CALIBRATOR_HPP
