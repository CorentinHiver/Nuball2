#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "../libCo.hpp"

#include "../Classes/Hit.hpp"
#include "../Classes/Detectors.hpp"
#include "../Classes/FilesManager.hpp"

#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"

// The following are declared at the end :
/**
 * @brief Allows one to manipulate the results of peaks
*/

class pic_fit_result
{
public:
  pic_fit_result(){};

  void resize(int size)
  {
    peaks.resize(size);
    cmeasures.resize(size);
    mean.resize(size);
    sigma.resize(size);
  };


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

private:
  Label m_label = 0;
  bool m_exist = false;
  bool m_enough_counts = false;
  bool m_peaks_found = false;
};

std::ofstream& operator<<(std::ofstream& fout, pic_fit_result const & fit)
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

using Fits = std::vector <pic_fit_result>;

class Calibration
{
public:

  struct histograms
  {
    // Binning raw spectra
    std::map<dAlias, NRJ> m_bins_raw   = {{labr_a,   1000.}, {ge_a,  20000.}, {bgo_a,   5000.}, {eden_a,      500.}, {paris_a,  10000.}, {dssd_a,   5000.}};
    std::map<dAlias, NRJ> m_min_raw    = {{labr_a,      0.}, {ge_a,      0.}, {bgo_a,      0.}, {eden_a,        0.}, {paris_a,      0.}, {dssd_a,      0.}};
    std::map<dAlias, NRJ> m_max_raw    = {{labr_a, 500000.}, {ge_a, 500000.}, {bgo_a,5000000.}, {eden_a,   500000.}, {paris_a, 500000.}, {dssd_a, 500000.}};
    
    std::map<dAlias, NRJ> m_bins_calib = {{labr_a,   1000.}, {ge_a,   6000.}, {bgo_a,    500.}, {eden_a,      500.}, {paris_a,   5000.},  {dssd_a,   500.}};
    std::map<dAlias, NRJ> m_min_calib  = {{labr_a,      0.}, {ge_a,      0.}, {bgo_a,      0.}, {eden_a,       -1.}, {paris_a,      0.},  {dssd_a,     0.}};
    std::map<dAlias, NRJ> m_max_calib  = {{labr_a,   3000.}, {ge_a,   3000.}, {bgo_a,   3000.}, {eden_a,       1.5}, {paris_a,  10000.},  {dssd_a, 20000.}};
    
    // Raw spectra :
    Vector_MTTHist<TH1F> raw_spectra;
    MTTHist<TH2F> all_raw_spectra;

    // Calibrated spectra :
    Vector_MTTHist<TH1F> calib_spectra;
    Vector_MTTHist<TH2F> all_calib;
    void Initialize(Calibration & calib);
    void setBins(std::string const & parameters);
  } m_histos;

  Calibration() {m_ok = false;}

  Calibration(Detectors const & detList) 
  {
    m_detectors = detList;
  }

  Calibration(Detectors const & detList, std::string const & loadfile) 
  {
    m_detectors = detList;
    load(loadfile);
  }

  /// @brief Copy constructor
  Calibration (Calibration const & otherCalib)
  {
    m_detectors     = otherCalib.m_detectors;
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
    m_detectors     = otherCalib.m_detectors;
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
  Calibration const & operator=(std::string const & calibFileName) {load(calibFileName); return *this;}
  bool load(std::string const & calibFileName);


  void calculate(std::string const & dataDir, int const & nb_files = -1, std::string const & source = "152Eu");

  /// @brief Calculate calibration from .root histograms
  /// @attention TODO
  void calculate(std::string const & histograms, std::string const & source = "152Eu");

  void Initialize() {m_histos.Initialize(*this);m_fits.resize(1000);}
  void loadData(std::string const & dataDir, size_t const & nb_files = -1);
  static void fillHisto(Hit & hit, FasterReader & reader, Calibration & calib);
  void analyse(std::string const & source = "152Eu");
  void writePosPeaks(std::string const & outfilename);
  void writeData(std::string const & outfilename);
  void writeRawRoot(std::string const & outfilename);

  void verify(std::string const & outfilename = "verify_calib.root");
  void writeCalibratedRoot(std::string const & outfilename);

  void setDetectorsList(Detectors const & ID_file) {m_detectors = ID_file;}
  void setDetectorsList(Detectors *ID_file) {m_detectors = *ID_file;}
  void setSource(std::string const & source) {m_source = source;}
  void verbose(bool const & _verbose) {m_verbose = _verbose;}

  /// @brief avoid using this one
  void  calibrate(Hit & hit) const;

  /// @brief calibrate the ADC value using the parameters extracted from the calibration data
  // NRJ calibrate(ADC const & adc, Label const & label) const ;

  /// @brief calibrate the nrj value using the parameters extracted from the calibration data
  NRJ calibrate(NRJ const & nrj, Label const & label) const;

  void calibrateData(std::string const & folder, size_t const & nb_files = -1);
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

  auto const & detectors() const {return m_detectors;}

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

  Detectors m_detectors;
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

void Calibration::histograms::Initialize(Calibration & calib)
{
  auto const & nb_det = calib.m_detectors.size();

  if (nb_det == 0) 
  {
    print("Error using Detector class in Calibration module."); 
    throw std::runtime_error(error_message["DEV"]);
  }

  // Bidims :
  auto bins_bidim_raw = get_max_value(m_bins_raw);
  auto max_bin_raw = get_max_value(m_max_raw);
  all_raw_spectra.reset("All_detectors", std::string("All_detectors").c_str(), nb_det,0,nb_det, bins_bidim_raw,0,max_bin_raw);

  all_calib.resize(calib.m_detectors.nb_aliases());
  for (auto const & pair : m_bins_calib) // Loop through the bins that has been set
  {
    auto alias = pair.first;
    auto const & type = Detector::alias_str[alias];
    if (!(find_key(m_min_calib, alias) && find_key(m_max_calib, alias))) continue; // Reject aliases with badly set bins
    auto nb_detectors = calib.m_detectors.nb_det_in_alias(alias);
    if (nb_detectors<1) continue;// Reject aliases without detectors
    all_calib[alias].reset(("All_"+type+"_spectra").c_str(), ("All "+type+" spectra").c_str(), 
        nb_detectors,0,nb_detectors, m_bins_calib[alias],m_min_calib[alias],m_max_calib[alias]);
  }

  calib_spectra.resize(nb_det);
  raw_spectra.resize(nb_det);

  for (Label label = 0; label<nb_det; label++)
  {
    auto const & name = calib.m_detectors[label];
    auto const & alias = Detectors::alias(label);
    if (alias == dAlias::null || alias == dAlias::RF) continue;
    calib_spectra[label].reset((name+"_calib").c_str(), (name+" calibrated spectra").c_str(), m_bins_calib[alias], m_min_calib[alias], m_max_calib[alias]);
    raw_spectra[label].reset((name+"_raw").c_str(), (name+" raw spectra").c_str(), m_bins_raw[alias], m_min_raw[alias], m_max_raw[alias]);
  }
}

void Calibration::histograms::setBins(std::string const & parameters)
{
  std::istringstream param(parameters);
  for (std::string line; std::getline(param,line);)
  {
    std::istringstream is(line);
    std::string type;
    std::string which_histo;
    is>>type;
    auto alias = Detector::getAlias(type);
    if (alias == dAlias::null) {throw std::runtime_error(type+"type is not recognized for binning in Calibration");continue;}
    is >> which_histo;
         if (which_histo == "raw"  ) is >> m_bins_raw  [alias] >> m_min_raw  [alias] >> m_max_raw  [alias];
    else if (which_histo == "calib") is >> m_bins_calib[alias] >> m_min_calib[alias] >> m_max_calib[alias];
    else {throw std::runtime_error(which_histo+"histo of Calibraiton module not recognized ");continue;}
  }
}

void Calibration::calculate(std::string const & dataDir, int const & nb_files, std::string const & source)
{
  print ("Calculating calibrations from raw data in", dataDir);

  this -> loadData(dataDir, nb_files);

  this -> analyse(source);

  this -> writeData(source+".calib");

  this -> writeRawRoot(source+".root");
}

// TBD
void Calibration::calculate(std::string const & histograms, std::string const & source)
{
  print ("Calculating calibrations from histogram data in", histograms);
  // this -> loadRootHisto(file); TBD
  this -> analyse(source);
  this -> writeData(source+".calib");
  this -> writeRawRoot(source+".root");
}

void Calibration::loadData(std::string const & dataDir, size_t const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialize();
  MTFasterReader mt_reader;
  mt_reader.addFolder(dataDir, nb_files);
  mt_reader.execute (fillHisto, *this);
  print("Data loaded");
}

void Calibration::fillHisto(Hit & hit, FasterReader & reader, Calibration & calib)
{
  if (calib.calibrate_data()) while(reader.Read()) 
  {
    auto nrj_cal = calib.calibrate(hit.adc, hit.label);
    calib.m_histos.calib_spectra[hit.label].Fill(nrj_cal);
    calib.m_histos.all_calib[calib.detectors().alias(hit.label)].Fill(compressedLabel[hit.label], nrj_cal);
  }
  else while(reader.Read()) 
  {
    calib.m_histos.raw_spectra[hit.label].Fill(hit.adc);
  }
}

bool isTripleAlpha(std::string const & source_name)
{
  return (source_name == "3-alpha" || source_name == "3alpha"  ||
          source_name == "triple-alpha" || source_name == "triplealpha");
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
  int nb_pics = 0;
  double E_right_pic = 0.f;
  for (Label label = 0; label<m_detectors.size(); label++)
  {
    if (!m_detectors.exists[label]) continue;

    auto & histo = m_histos.raw_spectra[label];
    auto const & name = m_detectors[label];
    auto const & alias = m_detectors.alias(label);

    if (histo.Integral()==0)
    {
      if (m_verbose) print(name, "have no data in this run");
      continue;
    }
    
    pic_fit_result & fit = m_fits[label];
    fit.setLabel(label);

    if (m_verbose) {print(); print(name);}

    // Initialize algorithm parameters :
   
    double integral_ratio_threshold = 0.f;  // The threshold used to tag the peak.
    int ADC_threshold = 0; // A threshold used in order to reject any potential electrical noise below like 500 ADC
    int window_1 = 0, window_2 = 0; // The three windows width (in keV)

    auto & peaks = fit.peaks;

    // Handles the triple alpha
    if (isTripleAlpha(source) && alias!=dssd_a) continue;
    else if (alias==dssd_a) continue;

    if (alias == ge_a)
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
    else if (alias == labr_a)
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
    else if (alias == paris_a)
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
          integral_ratio_threshold = 0.022f;
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
    else if (alias == dssd_a)
    {
      if (isTripleAlpha(source))
      {
        window_1 = 150, window_2 = 100;
        nb_pics = 3;
        peaks.resize(nb_pics);
        peaks = {5150, 5480, 5800};
        E_right_pic = peaks.back();
        integral_ratio_threshold = 0.05f;
        ADC_threshold = 500;
      }
      else
      {
        if (m_verbose) {print("DSSD not to be taken in this calibration file");}
      }
    }
    else if (alias == bgo_a)
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
    fit.resize(nb_pics);// Resize the intern vectors of the detector's pic_fit_result

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
    if (sum < 50000)
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

    if (alias == bgo_a)
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

    auto & x  = fit.x ;
    auto & y  = fit.y ;
    auto & ex = fit.ex;
    auto & ey = fit.ey;
    
    x .resize(nb_pics);
    y .resize(nb_pics);
    ex.resize(nb_pics);
    ey.resize(nb_pics);

    for (int j=0; j < nb_pics; j++)
    {
      if (m_verbose) std::cout << "Energy = " << peaks[j] << " Channel = " << fit.cmeasures[j]*scalefactor << std::endl;
      x [j]=fit.cmeasures[j]*scalefactor;
      y [j]=peaks[j];
      ex[j]=0;
      ey[j]=0;
    }

    // If faudrait aussi revoir ce fit ici !
    auto c1 = TCanvas(("c_"+name).c_str());
    TGraphErrors* gr = new TGraphErrors(nb_pics,x.data(),y.data(),ex.data(),ey.data());
    gr -> SetName((name+"_gr").c_str());
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
  if (m_detectors) size = m_detectors.size();
  else 
  {// If no ID file loaded, infer the number of detectors from the higher label in calbration file (unsafe)
    while (getline(inputfile, line))
    {
      std::istringstream iss (line);
      iss >> label;
      if (label>size) size = label;
    }
    inputfile.clear();                 // clear fail and eof bits
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
  for (auto const & fit : m_fits) if (m_detectors.exists[fit.label()]) outfile << fit;
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
  std::vector<size_t> nb_det_filled(m_detectors.nb_aliases(), 0);
  print("Verification of the calibration");
  for (Label label = 0; label<m_histos.raw_spectra.size(); label++) 
  {
    auto & raw_histo = m_histos.raw_spectra[label];
    if (raw_histo.Integral()<1) continue;
    auto & calib_histo = m_histos.calib_spectra[label];
    auto const & fit = m_fits[label];
    if (!fit.exists()) {nb_det_filled[m_detectors.alias(label)]++; continue;}

    // Extract useful information : 
    auto const & alias = m_detectors.alias(label);

    auto xaxis_raw = raw_histo -> GetXaxis();
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
        m_histos.all_calib[alias]->Fill(nb_det_filled[alias], nrj);
      }
    }
    nb_det_filled[alias]++;
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

void Calibration::calibrateData(std::string const & folder, size_t const & nb_files )
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
  auto const & det = calib.detectors();
  for (Label label = 0; label<det.size(); label++) if (det.exists[label])
  {
     cout << label << " " << calib[label] << std::endl;
  }
  return cout;
}

#endif //CALIBRATION_H
