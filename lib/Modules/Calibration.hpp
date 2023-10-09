#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "../libCo.hpp"

#include "../Classes/Hit.hpp"
#include "../Classes/Detectors.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/FilesManager.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"

// The following are declared at the end :
/**
 * @brief Allows one to manipulate the results of peaks
*/

class Fit
{
public:
  Fit(){};

  void resize(int size)
  {
    peaks.resize(size);
    cmeasures.resize(size);
    mean.resize(size);
    sigma.resize(size);
  };

  void clear()
  {
    peaks.clear();
    cmeasures.clear();

    mean.clear();
    sigma.clear();

    x.clear() ;
    y.clear() ;
    ex.clear();
    ey.clear();
  
    integral = -1.0;
    chi2 = -1.0;
    parameter0 = 0.0;
    parameter1 = 1;
    parameter2 = 0.0;
    parameter3 = 0.0;
    scalefactor = 0.0;
    keVperADC = 0.0;
    order = 0;

    name = "";

    m_label = 0;
    m_exist = false;
    m_enough_counts = false;
    m_peaks_found = false;
  }

  Label const & label() const {return m_label;}

  /// @brief Returns the number of peaks used for calibration
  size_t size() const {return peaks.size();};

  /// @brief Returns true if fit succeeded
  bool const & exists() const {return this->m_exist;}

  /// @brief Returns true if fit had enough counts
  bool enough_counts() const {return this -> m_enough_counts;}

  /// @brief Returns true if the fit found the peaks
  bool found_peaks() const {return this -> m_peaks_found;}


  /// @brief Set the label
  void setLabel(Label const & label) {m_label = label;}

  /// @brief Set if the fit succeeded :
  bool const & exists(bool exist) {return (m_exist = bool_cast(exist));}

  /// @brief Set if there was not enough counts :
  bool too_few_counts(bool few_counts) {return (m_enough_counts = bool_cast(!few_counts));}

  /// @brief Set if there was not enough counts :
  bool peaks_found(bool found) {return (m_peaks_found = bool_cast(found));}


  std::vector<double> peaks;
  std::vector<double> cmeasures;

  std::vector<double> mean;
  std::vector<double> sigma;

  std::vector<double> x ;
  std::vector<double> y ;
  std::vector<double> ex;
  std::vector<double> ey;


  double integral = -1.0;
  double chi2 = -1.0;
  double parameter0 = 0.0;
  double parameter1 = 1;
  double parameter2 = 0.0;
  double parameter3 = 0.0;
  double scalefactor = 0.0;
  double keVperADC = 0.0;
  uchar order = 0;

  std::string name;

private:
  Label m_label = 0;
  bool m_exist = false;
  bool m_enough_counts = false;
  bool m_peaks_found = false;
};

std::ofstream& operator<<(std::ofstream& fout, Fit const & fit)
{
  fout << fit.label();
  fout << std::setprecision(4);
  fout << " " << fit.parameter0;
  fout << std::setprecision(6);
  fout << " " << fit.parameter1;
  if (fit.parameter2!=0.0) 
  {
    fout << std::setprecision(8);
    fout << " " << fit.parameter2;
  }
  if (fit.parameter3!=0.0) 
  {
    fout << std::setprecision(10);
    fout << " " << fit.parameter3;
  }
  fout << std::endl;
  return fout;
}

using Fits = std::vector <Fit>;

/**
 * @brief 
 * @todo Calibration::verify() DO NOT SUPPORT MTObject !!
 */
class Calibration
{
public:

  struct histograms
  {
    // Binning raw spectra
    // std::map<dType, NRJ> m_bins_raw   = {{"labr",   1000.}, {"ge",  20000.}, {"bgo",   5000.}, {"eden",      500.}, {"paris",  10000.}, {"dssd",   5000.}};
    // std::map<dType, NRJ> m_min_raw    = {{"labr",      0.}, {"ge",      0.}, {"bgo",      0.}, {"eden",        0.}, {"paris",      0.}, {"dssd",      0.}};
    // std::map<dType, NRJ> m_max_raw    = {{"labr", 500000.}, {"ge", 500000.}, {"bgo",5000000.}, {"eden",   500000.}, {"paris", 500000.}, {"dssd", 500000.}};
    
    // std::map<dType, NRJ> m_bins_calib = {{"labr",   1000.}, {"ge",   6000.}, {"bgo",    500.}, {"eden",      500.}, {"paris",   5000.},  {"dssd",   500.}};
    // std::map<dType, NRJ> m_min_calib  = {{"labr",      0.}, {"ge",      0.}, {"bgo",      0.}, {"eden",       -1.}, {"paris",      0.},  {"dssd",     0.}};
    // std::map<dType, NRJ> m_max_calib  = {{"labr",   3000.}, {"ge",   3000.}, {"bgo",   3000.}, {"eden",       1.5}, {"paris",  10000.},  {"dssd", 20000.}};
    
    // Raw spectra :
    Vector_MTTHist<TH1F> raw_spectra;
    MTTHist<TH2F> all_raw_spectra;

    // Calibrated spectra :
    Vector_MTTHist<TH1F> calib_spectra;
    Vector_MTTHist<TH2F> all_calib;

    // Other spectra :
    std::map<int, TH1F*> spectra;

    void Initialize();
    void setBins(std::string const & parameters);
    bool initialized = false;
  } m_histos;

  // ~Calibration() {for (auto & histo : m_histos.spectra) if (histo.second) delete histo.second;}

  Calibration() {m_ok = false;}

  /// @brief Copy constructor
  Calibration (Calibration const & otherCalib)
  {
    m_ok            = otherCalib.m_ok;
    m_nb_detectors  = otherCalib.m_nb_detectors;
    m_max_labels    = otherCalib.m_max_labels;
    m_order         = otherCalib.m_order;
    m_intercept     = otherCalib.m_intercept;
    m_slope         = otherCalib.m_slope;
    m_binom         = otherCalib.m_binom;
    m_trinom        = otherCalib.m_trinom;
  }

  /// @brief Copy operator
  Calibration const & operator=(Calibration const & otherCalib) 
  {
    m_ok            = otherCalib.m_ok;
    m_nb_detectors  = otherCalib.m_nb_detectors;
    m_max_labels    = otherCalib.m_max_labels;
    m_order         = otherCalib.m_order;
    m_intercept     = otherCalib.m_intercept;
    m_slope         = otherCalib.m_slope;
    m_binom         = otherCalib.m_binom;
    m_trinom        = otherCalib.m_trinom;
    return *this;
  }

  /// @brief Loading calibration from file name
  Calibration(std::string const & calibFileName) {load(calibFileName);}
  Calibration(const char * calibFileName) {load(calibFileName);}
  Calibration const & operator=(std::string const & calibFileName) {load(calibFileName); return *this;}
  bool load(std::string const & calibFileName);


  void calculate(std::string const & dataDir, int const & nb_files = -1, std::string const & source = "152Eu",std::string const & type = "fast");

  /// @brief Calculate calibration from .root histograms
  /// @attention TODO
  void calculate(std::string const & histograms, std::string const & source = "152Eu");

  void Initialize() {m_histos.Initialize(); m_fits.resize(1000);}

  void loadRootData(std::string const & dataDir, int const & nb_files = -1);
  static void loadRootDataThread(Calibration & calib, MTList & list);
  void fillRootDataHisto(std::string const & filename);

  void loadData(std::string const & dataDir, int const & nb_files = -1);
  static void fillHisto(Hit & hit, FasterReader & reader, Calibration & calib);
  void analyse(std::string const & source = "152Eu");
  void peakFinder(std::string const & source);
  void fitCalibration(Fits & fits);
  void writePosPeaks(std::string const & outfilename);
  void writeData(std::string const & outfilename);
  void writeRawRoot(std::string const & outfilename);

  void verify(std::string const & outfilename = "verify");
  void writeCalibratedRoot(std::string const & outfilename);

  void setSource(std::string const & source) {m_source = source;}
  void verbose(bool const & _verbose) {m_verbose = _verbose;}

  void loadRootHisto(std::string const & histograms);

  /// @brief avoid using this one
  void  calibrate(Hit & hit) const;

  /// @brief calibrate the ADC value using the parameters extracted from the calibration data
  // NRJ calibrate(ADC const & adc, Label const & label) const ;

  /// @brief calibrate the nrj value using the parameters extracted from the calibration data
  NRJ calibrate(NRJ const & nrj, Label const & label) const;

  void calibrateData(std::string const & folder, int const & nb_files = -1);
  bool const & calibrate_data() const {return m_calibrate_data;}
  bool const & calibrate_data() {return m_calibrate_data;}
  void writeCalibratedData(std::string const & outfilename);

  
  /// @brief Call for calibrate method
  template<class... ARGS>
  auto operator()(ARGS &&... args) const {return calibrate(std::forward<ARGS>(args)...);}

  bool const & isFilled() const {return m_ok;}

  void Print();

  operator bool() const & {return m_ok;}

  //DEV :
  // void calibrate_fast(Hit & hit){}
  // void calibrate_fast(Label label, ADC energy, NRJ energy_calibrated){}
  void setCalibrationTables();
  //!DEV

  auto const & size() const {return m_nb_detectors;}

  // Accessors to the calibration vectors :
  std::vector<NRJ> operator[](Label const & label) const 
  {
    switch (m_order[label])
    {
      case 0 : return {0};
      case 1 : return {m_intercept[label], m_slope[label]};
      case 2 : return {m_intercept[label], m_slope[label], m_binom[label]};
      case 3 : return {m_intercept[label], m_slope[label], m_binom[label], m_trinom[label]};
     default : return {0};
    }
  }

private:
  //Private methods :
  void set(Label label, NRJ intercept, NRJ slope, NRJ binom, NRJ trinom);

  //Attributs for the calculations :
  bool      m_verbose   = false;
  bool      m_residus   = false;
  bool      m_outRoot_b = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibration/";

  Fits m_fits;

  //Attributs for the tables :
  bool m_ok = false;
  bool m_calibrate_data = false;
  Label m_nb_detectors = 0;
  Label m_max_labels = 0;
  std::vector<char>  m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<NRJ> m_intercept;
  std::vector<NRJ> m_slope;
  std::vector<NRJ> m_binom;
  std::vector<NRJ> m_trinom;
  std::vector<std::vector<std::vector<NRJ>>> calibration_tables;
};

void Calibration::histograms::Initialize()
{
  if (initialized) return;
  auto const & max_label = detectors.size();

  if (max_label == 0) 
  {
    print("Error using Detector class in Calibration module."); 
    throw std::runtime_error(error_message["DEV"]);
  }

  // All the detectors spectra in one plot :
  all_raw_spectra.reset("All_detectors", "All detectors", 
      max_label,0,max_label, detectors.ADCBin("default").bins,0,detectors.ADCBin("default").min);

  // All the calibrated detectors spectra of each type in one plot :
  all_calib.resize(detectors.nbTypes());
  for (auto const & type : detectors.types())
  {
    print(type);
    auto nb_detectors = detectors.nbOfType(type);
    auto const & binning = detectors.energyBidimBin(type);
    all_calib[detectors.typeIndex(type)].reset(("All_"+type+"_spectra").c_str(), ("All "+type+" spectra").c_str(), 
        nb_detectors,0,nb_detectors, binning.bins,binning.min,binning.max);
  }

  // All the raw and/or calibrated spectra in a separate spectra :
  calib_spectra.resize(max_label);
  raw_spectra.resize(max_label);
  // for (Label label = 0; label<max_label; label++)
  // {
  //   auto const & name = detectors[label];
  //   auto const & type = detectors.type(label);
  //   if (type == "null" || type == "RF")
  //   {
  //     calib_spectra[label].reset((std::to_string(label)+"_calib").c_str(), (std::to_string(label)+" calibrated spectra").c_str(), 1000, 0, 1000);
  //     raw_spectra[label].reset((std::to_string(label)+"_raw").c_str(), (std::to_string(label)+" raw spectra").c_str(), 1000, 0, 1000);
  //   }
  //   else 
  //   {
  //     auto const & bin_raw = detectors.energyBin(type);
  //     auto const & bin_calib = detectors.energyBin(type);
  //     raw_spectra[label].reset((name+"_raw").c_str(), (name+" raw spectra").c_str(), bin_raw.bins, bin_raw.min, bin_raw.max);
  //     calib_spectra[label].reset((name+"_calib").c_str(), (name+" calibrated spectra").c_str(), bin_calib.bins, bin_calib.min, bin_calib.max);
  //   }
  // }
  for (auto const & type : detectors.types())
  {
    if (type == "null" || type == "RF") continue;
    for (size_t index = 0; index<detectors.nbOfType(type); index++)
    {
      auto const & name = detectors.name(type, index);
      print(name);
      auto const & label = detectors.label(type, index);
      print(label);
      auto const & bin_raw = detectors.energyBin(type);
      auto const & bin_calib = detectors.energyBin(type);
      raw_spectra[label].reset((name+"_raw").c_str(), (name+" raw spectra").c_str(), bin_raw.bins, bin_raw.min, bin_raw.max);
      calib_spectra[label].reset((name+"_calib").c_str(), (name+" calibrated spectra").c_str(), bin_calib.bins, bin_calib.min, bin_calib.max);
    }
  }
  initialized = true;
}

void Calibration::histograms::setBins(std::string const & parameters)
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
    if (type == "null") {throw_error(type+"type is not recognized for binning in Calibration");}
    is >> which_histo;
         if (which_histo == "raw"  ) is >> ADCbins[type].bins >> ADCbins[type].min >> ADCbins[type].max;
    else if (which_histo == "calib") is >> Energybins[type].bins >> Energybins[type].min >> Energybins[type].max;
    else {throw_error(which_histo+" histo of Calibration module not recognized ");}
  }
}

void Calibration::calculate(std::string const & dataDir, int const & nb_files, std::string const & source, std::string const & type)
{
  print ("Calculating calibrations from raw data in", dataDir);

  if(type == "fast") this -> loadData(dataDir, nb_files);
  else if (type == "root") this -> loadRootData(dataDir, nb_files);
  else {print(type, "unkown data format"); return;}

  this -> analyse(source);

  this -> writeData(source+".calib");

  this -> writeRawRoot(source+".root");
}

// TBD
void Calibration::calculate(std::string const & histograms, std::string const & source)
{
  print ("Calculating calibrations from histogram data in", histograms);
  this -> Initialize();
  // this -> loadRootHisto(file); TBD
  this -> analyse(source);
  this -> writeData(source+".calib");
  this -> writeRawRoot(source+".root");
}

void Calibration::loadRootHisto(std::string const & histograms)
{
  unique_TFile file(TFile::Open(histograms.c_str()));
  if (!file.get()->IsOpen()) throw_error("Can't open"+histograms);
  
  TIter nextKey(file->GetListOfKeys());
  TKey* key = nullptr;

  while ((key = dynamic_cast<TKey*>(nextKey()))) 
  {
    TObject* obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom(TH1::Class())) 
    {
      if (obj->IsA()->InheritsFrom(TH1F::Class())) 
      {
        auto hist = dynamic_cast<TH1F*>(obj);
        std::string name = hist -> GetName();
        for (auto const & _name : detectors)
        {
          if (_name!="" && name.find(_name) != std::string::npos)
          {
            auto const & label = detectors.getLabel(_name);
            if (name.find("_raw")) m_histos.raw_spectra[label] = hist;
            else if (name.find("_calib")) m_histos.calib_spectra[label] = dynamic_cast<TH1F*> (hist->Clone(_name.c_str()));
            else m_histos.spectra[label] = dynamic_cast<TH1F*> (hist->Clone(_name.c_str()));
            m_histos.raw_spectra[label].Print();
            break;
          }
        }
      }
    }
  }

}

void Calibration::loadData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialize();
  MTFasterReader mt_reader;
  mt_reader.addFolder(dataDir, nb_files);
  mt_reader.execute (fillHisto, *this);
  print("Data loaded");
}

void Calibration::loadRootData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialize();
  FilesManager files;
  files.addFolder(dataDir, nb_files);
  MTList list(files.getListFiles());
  MTObject::parallelise_function(loadRootDataThread, *this, list);
  print("Data loaded");
}

void Calibration::loadRootDataThread(Calibration & calib, MTList & list)
{
  std::string filename = "";
  while(list.getNext(filename))
  {
    calib.fillRootDataHisto(filename);
  }
}

void Calibration::fillRootDataHisto(std::string const & filename)
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
        if (ratio<-0.2 || ratio>0.2) continue;
      }
      auto const nrjcal = calibrate(event.adcs[hit], label);
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

void Calibration::fillHisto(Hit & hit, FasterReader & reader, Calibration & calib)
{
  if (calib.calibrate_data()) while(reader.Read()) 
  {
    if (calib.m_order[hit.label]<1) continue;
    auto nrj_cal = calib.calibrate(hit.adc, hit.label);
    calib.m_histos.calib_spectra[hit.label].Fill(nrj_cal);
    calib.m_histos.all_calib[detectors.typeIndex(hit.label)].Fill(compressedLabel[hit.label], nrj_cal);
  }
  else while(reader.Read())
  {
    auto const & type = detectors.type(hit.label);
    if (type ==  "null" || type ==  "RF") continue;
    calib.m_histos.raw_spectra[hit.label].Fill(hit.adc);
  }
}

bool isTripleAlpha(std::string const & source_name)
{
  return (source_name == "3-alpha" || source_name == "3alpha"  ||
          source_name == "triple-alpha" || source_name == "triplealpha");
}

void Calibration::peakFinder(std::string const & source)
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
      if (m_verbose) print(name, "has no data in this run");
      continue;
    }

    histo.Merge();
    
    auto & fit = m_fits[label];
    fit.setLabel(label);
    fit.name = name;

    if (m_verbose) {print(); print(name);}

    // Initialize algorithm parameters :
   
    double integral_ratio_threshold = 0.f;  // The threshold used to tag the peak.
    int ADC_threshold = 0; // A threshold used in order to reject any potential electrical noise below like 500 ADC
    int window_1 = 0, window_2 = 0; // The three windows width (in keV)

    auto & peaks = fit.peaks;
    int minHitsToCalibrate = 50000;

    // Handles the triple alpha
    if (isTripleAlpha(source) && type!="dssd") continue;
    else if (!isTripleAlpha(source) && type=="dssd") continue;
    
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
        if (name.find("FR1")) 
        {
          nb_pics = 4;
          peaks.resize(nb_pics);
          peaks = {344.2760, 778.9030, 964.1310, 1408.0110};
          E_right_pic = peaks.back();
          integral_ratio_threshold = 0.030f;
        }
        else
        {
          nb_pics = 5;
          peaks.resize(nb_pics);
          peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
          E_right_pic = peaks.back();
          integral_ratio_threshold = 0.012f;
        }
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

    AddTH1(m_histos.all_raw_spectra, histo, label);
    
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

void Calibration::fitCalibration(Fits & fits)
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

    if(isGe[label] || isDSSD[label])
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
void Calibration::analyse(std::string const & source)
{
  print("Exctracting calibration parameters from spectra");
  // -----------------------------
  // Parameterize the pics to fit :
  // -----------------------------
  peakFinder(source);
  fitCalibration(m_fits);
}

//DEV :
void Calibration::setCalibrationTables()
{
  print("creating calibration tables");
  calibration_tables.resize(m_max_labels);
  std::vector<std::vector<NRJ>> *calib_vec;
  for (Label i = 0; i<m_max_labels; i++)
  {
    calib_vec = &calibration_tables[i];
    calib_vec->resize(200000);
  }
  print("Done !");
  print();
}

// NRJ Calibration::calibrate(ADC const & adc, Label const & label) const 
// {
//   return calibrate(NRJ_cast(adc), label);
// }
inline NRJ Calibration::calibrate(NRJ const & nrj, Label const & label) const 
{
  auto nrj_r = nrj+random_uniform();
  // auto nrj_r = nrj+gRandom->Uniform(0,1);
  switch(m_order[label])
  {
    case 0: return nrj_r;
    case 1: return m_intercept[label] + m_slope[label]*nrj_r; 
    case 2: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r;
    case 3: return m_intercept[label] + m_slope[label]*nrj_r + m_binom[label]*nrj_r*nrj_r + m_trinom[label]*nrj_r*nrj_r*nrj_r;
   default: return nrj_r;
  }
}

inline void Calibration::calibrate(Hit & hit) const
{
  auto const & label = hit.label;
  if (label > m_max_labels) return;

#ifdef LICORNE
  if (is_EDEN(label))
  {
    if (hit.qdc2==0) hit.qdc2 = 1;
    hit.nrj2 = NRJ_cast(hit.qdc2)/NRJ_cast(hit.adc);
  }
#endif //LICORNE
  if (isParis[label]) hit.nrj2 = calibrate(hit.qdc2, label);
  else hit.nrj = calibrate(hit.adc, label);
}

void Calibration::set(Label _label, NRJ _intercept = 0.f, NRJ _slope = 1.f, NRJ _binom = 0.f, NRJ _trinom = 0.f)
{
  if (_slope == 1.f && _intercept == 0.f) {m_order[_label] = 0;}
  else if (_binom == 0.f)
  {
    m_order     [_label] = 1;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
  }
  else if (_trinom == 0.f)
  {
    m_order     [_label] = 2;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
  }
  else
  {
    m_order     [_label] = 3;
    m_intercept [_label] = _intercept;
    m_slope     [_label] = _slope;
    m_binom     [_label] = _binom;
    m_trinom    [_label] = _trinom;
  }
}

bool Calibration::load(std::string const & calibFileName)
{
  std::ifstream inputfile(calibFileName, std::ifstream::in);

  if (!inputfile.good()) {print("CAN'T OPEN THE CALIBRATION FILE " + calibFileName); throw std::runtime_error("CALIBRATION");}
  else if (file_is_empty(inputfile)) {print("CALIBRATION FILE", calibFileName, "EMPTY !"); throw std::runtime_error("CALIBRATION");}
  std::string line = "";
  Label size = 0;
  Label label = 0;
  // ----------------------------------------------------- //
  // First extract the maximum label
  if (detectors) size = detectors.size();
  else 
  {// If no ID file loaded, infer the number of detectors from the higher label in calibration file (less safe)
    while (getline(inputfile, line))
    {
      std::istringstream iss (line);
      iss >> label;
      if (label>size) size = label;
    }
    inputfile.clear();
    inputfile.seekg(0, std::ios::beg); // back to the start of the file
  }
  // ----------------------------------------------------- //
  // Now fill the vectors
  m_order    .resize(size,-1);
  m_intercept.resize(size, 0.f);
  m_slope    .resize(size, 1.f);
  m_binom    .resize(size, 0.f);
  m_trinom   .resize(size, 0.f);
  NRJ slope = 1.f, binom = 0.f, trinom = 0.f, intercept = 0.f;
  while (getline(inputfile, line))
  {
    m_nb_detectors++;
    std::istringstream iss (line);
    iss >> label >> intercept >> slope >> binom >> trinom;
    this -> set(label, intercept, slope, binom, trinom);
    intercept = 0.f; slope = 1.f; binom = 0.f; trinom = 0.f;
  }
  print("Calibration extracted from", calibFileName);
  return (m_ok = true);
}

void Calibration::Print()
{
  for (Label label = 0; label<m_max_labels; label++)
  {
    if (m_order[label] < 0) continue;
    std::cout << label << " : ";
    std::cout << m_intercept[label];
    if (m_order[label] > 0)
    {
      std::cout << " " << m_slope[label];
      if (m_order[label] > 1)
      {
        std::cout << " " << m_binom[label];
        if (m_order[label] > 2)
        {
          std::cout << " " << m_trinom[label];
        }
      }
    }
    std::cout << std::endl;
  }
}

void Calibration::writePosPeaks(std::string const & outfilename)
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

void Calibration::writeData(std::string const & outfilename)
{
  File outFile(outfilename);
  outFile.setExtension("calib");
  outFile.makePath(); // Create the path if it doesn't already exist

  std::ofstream outfile(outFile);
  for (auto const & fit : m_fits) if (detectors.exists(fit.label())) outfile << fit;
  print("Data written to", outfilename);
}

void Calibration::writeRawRoot(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  m_histos.all_raw_spectra.Write();
  for (auto & histo : m_histos.raw_spectra) {histo.Write();}
  print("Raw root spectra written to", outfilename);
}

void Calibration::verify(std::string const & outfilename)
{
  std::vector<size_t> nb_det_filled(detectors.nbTypes(), 0);
  print("Verification of the calibration");
  for (Label label = 0; label<m_histos.raw_spectra.size(); label++) 
  {
    if (!detectors.exists(label)) continue;
    auto const & name = detectors[label];
    auto & raw_histo = m_histos.raw_spectra[label];
    if (raw_histo.Integral()<1) {if (m_verbose) print(name, "has no hit"); continue;}
    raw_histo.Merge();
    auto & calib_histo = m_histos.calib_spectra[label];
    calib_histo.Merge();
    auto const & fit = m_fits[label];

    // Extract useful information : 
    auto const & type = detectors.type(label);
    auto const & type_index = detectors.typeIndex(type);
    if (!fit.exists()) {nb_det_filled[type_index]++; if (m_verbose) print(name, "has no fit"); continue;}

    auto const xaxis_raw = raw_histo -> GetXaxis();
    auto const & nb_bins = xaxis_raw -> GetNbins();
    auto const bin_width = xaxis_raw -> GetXmax() / nb_bins;
    for(int bin = 0; bin<nb_bins; bin++)
    {
      auto const & counts_in_bin = raw_histo -> GetBinContent(bin);
      ADC const raw_ADC = xaxis_raw -> GetBinCenter(bin);
      for (int hit = 0; hit<counts_in_bin; hit++)
      {
        // auto const nrj = calibrate(raw_ADC, label);
        auto nrj = NRJ_cast(raw_ADC + bin_width*gRandom->Uniform(0,1));
        switch(fit.order)
        {
          case 1 : nrj = fit.parameter0 + nrj*fit.parameter1; break;
          case 2 : nrj = fit.parameter0 + nrj*fit.parameter1 + nrj*nrj*fit.parameter2; break;
        }
        calib_histo -> Fill(nrj);
        m_histos.all_calib[type_index]->Fill(double_cast(nb_det_filled[type_index]), nrj);
      }
    }
    nb_det_filled[type_index]++;
  }
  writeCalibratedRoot(outfilename+"_calib.root");
}

void Calibration::writeCalibratedRoot(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : m_histos.all_calib) histo.Write();
  for (auto & histo : m_histos.calib_spectra)  histo.Write();
  print("Calibrated root spectra written to", outfilename);
}

void Calibration::calibrateData(std::string const & folder, int const & nb_files )
{
  m_calibrate_data = true;
  this -> loadData(folder, nb_files);
}

void Calibration::writeCalibratedData(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : m_histos.all_calib) histo.Write();
  for (auto & histo : m_histos.calib_spectra)  histo.Write();
  print("Calibrated root spectra written to", outfilename);
}

std::ostream& operator<<(std::ostream& cout, Calibration const & calib)
{
  for (Label label = 0; label<detectors.size(); label++) 
  {
    if (detectors && !detectors.exists(label)) continue;
     cout << label << " " << calib[label] << std::endl;
  }
  return cout;
}

#endif //CALIBRATION_H
