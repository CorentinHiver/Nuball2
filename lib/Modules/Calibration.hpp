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

  //Getter :
  size_t size() const {return peaks.size();};

  //Getter & setter :
  Bool_t exists(char const & _exist = -1)
  {
    if (_exist == -1)  return this->exist; // Getter mode
    else if (_exist<2) return (exist = static_cast<bool>(_exist));// Setter mode
    else return false;
  };

  Bool_t too_few_counts(char const &  _few_counts = -1)
  {
    if (_few_counts == -1)  return this->few_counts; //Getter mode
    else if (_few_counts<2) return (few_counts = static_cast<bool> (_few_counts)); // Setter mode
    else return false;
  };

  Int_t label = 0;

  std::vector<Float_t> peaks;
  std::vector<Float_t> cmeasures;

  std::vector<Float_t> mean;
  std::vector<Float_t> sigma;

  Int_t   nb_hits = -1;
  Float_t chi2 = -1;
  Float_t parameter0 = 0;
  Float_t parameter1 = 1;
  Float_t parameter2 = 0.f;
  Float_t parameter3 = 0.f;
  Float_t scalefactor = 0.f;
  Float_t keVperADC = 0.f;

private:
  Bool_t exist = false;
  Bool_t few_counts = false;
};
using Fits = std::vector <pic_fit_result>;

class Calibration
{
public:

  struct histograms
  {
    // Binning raw spectra
    std::map<dAlias, float> m_bins_raw   = {{dAlias::labr,   1000.}, {dAlias::ge,   3000.}, {dAlias::bgo,    500.}, {dAlias::eden,      500.}, {dAlias::paris,    1000.}, {dAlias::dssd,    500.}};
    std::map<dAlias, float> m_min_raw    = {{dAlias::labr,      0.}, {dAlias::ge,      0.}, {dAlias::bgo,      0.}, {dAlias::eden,        0.}, {dAlias::paris,       0.}, {dAlias::dssd,      0.}};
    std::map<dAlias, float> m_max_raw    = {{dAlias::labr, 500000.}, {dAlias::ge, 500000.}, {dAlias::bgo, 500000.}, {dAlias::eden,   500000.}, {dAlias::paris, 5000000.}, {dAlias::dssd, 500000.}};
    
    std::map<dAlias, float> m_bins_calib = {{dAlias::labr,   1000.}, {dAlias::ge,   3000.}, {dAlias::bgo,    500.}, {dAlias::eden,      500.}, {dAlias::paris,    1000.},  {dAlias::dssd,   500.}};
    std::map<dAlias, float> m_min_calib  = {{dAlias::labr,      0.}, {dAlias::ge,      0.}, {dAlias::bgo,      0.}, {dAlias::eden,       -1.}, {dAlias::paris,       0.},  {dAlias::dssd,     0.}};
    std::map<dAlias, float> m_max_calib  = {{dAlias::labr,   3000.}, {dAlias::ge,   3000.}, {dAlias::bgo,   3000.}, {dAlias::eden,       1.5}, {dAlias::paris,   30000.},  {dAlias::dssd, 20000.}};
    
    Vector_MTTHist<TH1F> raw_spectra;
    Vector_MTTHist<TH1F> calib_spectra;
    void Initialize(Calibration & calib);
    void setBins(std::string const & parameters);
  } histos;

  Calibration() {m_ok = false;}

  Calibration(Detectors const & detList, std::string const & datafile = "") 
  {
    m_detectors = detList;
    if (datafile != "") load(datafile);
  }

  /// @brief Copy constructor
  Calibration (Calibration const & calibration)
  {
    m_detectors = calibration.m_detectors;
  }

  Calibration(std::string const & calibFileName) {load(calibFileName);}
  Calibration& operator=(std::string const & calibFileName) {load(calibFileName); return *this;}
  bool load(std::string const & calibFileName);


  /// @brief Calculate calibration from raw .fast files
  void calculate(std::string const & dataDir, int const & nb_files = -1, std::string const & source = "152Eu");

  /// @brief Calculate calibration from .root histograms
  void calculate(std::string const & histograms, std::string const & source = "152Eu");

  void Initialize() {histos.Initialize(*this);}
  static void fillRawHisto(Hit & hit, FasterReader & reader, Calibration & calib);
  void analyse();

  void setDetectorsList(Detectors const & ID_file) {m_detectors = ID_file;}
  void setDetectorsList(Detectors *ID_file) {m_detectors = *ID_file;}
  void setSource(std::string const & source) {m_source = source;}

  /// @brief avoid using this one
  void  calibrate(Hit & hit) const;

  /// @brief calibrate
  float calibrate(float const & nrj, Label const & label) const;

  
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

private:
  //Private methods :
  void set(Label label, float intercept, float slope, float binom, float trinom);

  //Attributs for the calculations :
  bool      m_verbose   = false;
  bool      m_residus   = false;
  bool      m_outRoot_b = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibration/";

  Detectors m_detectors;
  FilesManager files;
  Fits fits;

  //Attributs for the tables :
  bool m_ok = false;
  Label m_nb_detectors = 0;
  Label m_max_labels = 0;
  std::vector<char>  m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<float> m_intercept;
  std::vector<float> m_slope;
  std::vector<float> m_binom;
  std::vector<float> m_trinom;
  std::vector<std::vector<std::vector<float>>> calibration_tables;
};

void Calibration::histograms::Initialize(Calibration & calib)
{
  auto const & nb_det = calib.m_detectors.size();

  if (nb_det == 0)
  {
    throw std::runtime_error("dAlias list in Calibration module do not exist.");
  }


  raw_spectra.resize(nb_det);
  calib_spectra.resize(nb_det);

  for (uint label = 0; label<nb_det; label++)
  {
    auto const & name = calib.m_detectors[label];
    auto const & alias = Detectors::alias(label);
    if (alias == dAlias::null || alias == dAlias::RF) continue;
    raw_spectra[label].reset((name+"_raw").c_str(), (name+"_raw").c_str(), m_bins_raw[alias], m_min_raw[alias], m_max_raw[alias]);
    calib_spectra[label].reset((name+"_calib").c_str(), (name+"_calib").c_str(), m_bins_calib[alias], m_min_calib[alias], m_max_calib[alias]);
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
  m_source = source;
  this -> Initialize();

  print("Loading the data");
  MTFasterReader mt_reader;
  mt_reader.addFolder(dataDir, nb_files);
  mt_reader.execute (fillRawHisto, *this);

  this -> analyse();

}

void Calibration::calculate(std::string const & histograms, std::string const & source)
{
  print ("Calculating calibrations from histogram data in", histograms);
  m_source = source;

  
}

void Calibration::fillRawHisto(Hit & hit, FasterReader & reader, Calibration & calib)
{
  while(reader.Read())
  {
    calib.histos.raw_spectra[hit.label].Fill(hit.nrj);
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
void Calibration::analyse()
{
  // -----------------------------
  // Parameterize the pics to fit :
  // -----------------------------
  Int_t nb_pics = 0;
  Float_t E_right_pic = 0.f;
  for (uint label = 0; label<m_detectors.size(); label++)
  {
    if (!m_detectors.exists[label]) continue;
    auto histo = histos.raw_spectra[label];
    pic_fit_result & fit = fits[label];
    auto const & name = m_detectors[label];
    auto const & type = m_detectors.type(label);
    if (m_verbose) std::cout << std::endl << name << std::endl;

    // Initialize algorithm parameters :
    // The threshold used to tag the peak.
    Float_t integral_ratio_threshold = 0.f;
    // A threshold is used in order to reject any potential electrical noise above like 500 ADC
    Int_t ADC_threshold = 0; 
    // The three windows width (in keV)
    Int_t window_1 = 0, window_2 = 0, window_3 = 0; 

    // Reject the triple alpha
    if (isTripleAlpha(m_source) && type!=dAlias::dssd) continue;
    else if (type==dAlias::dssd) continue;

    if (type == dAlias::ge)
    {// For Clovers
      window_1 = 10, window_2 = 8, window_3 = 4;
      if (m_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.011;
        ADC_threshold = 100;
      }
      else if (m_source == "232Th")
      {
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 910, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0049;
        ADC_threshold = 100;
      }
      else if (m_source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        fit.peaks.resize(nb_pics);
        fit.peaks = {1172, 1333};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.06;
        ADC_threshold = 100;
      }
    }
    else if (type == dAlias::labr)
    {// For labr
      window_1 = 70, window_2 = 50, window_3 = 20;
      if (m_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.012;
        ADC_threshold = 500;
      }
      else if (m_source == "232Th")
      {
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 911, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0025;
        ADC_threshold = 500;
      }
      else if (m_source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        fit.peaks.resize(nb_pics);
        fit.peaks = {1172, 1333};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.1;
        ADC_threshold = 100;
      }
    }
    else if (type == dAlias::paris)
    {// For paris
      window_1 = 70, window_2 = 50, window_3 = 25;
      if (m_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.012;
        ADC_threshold = 500;
      }
      else if (m_source == "232Th")
      {//never tested yet - but should be similar to fatima
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 911, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0025;
        ADC_threshold = 500;
      }
    }
    else if (type == dAlias::dssd)
    {
      if (isTripleAlpha(m_source))
      {
        window_1 = 150, window_2 = 100, window_3 = 50;
        nb_pics = 3;
        fit.peaks.resize(nb_pics);
        fit.peaks = {5150, 5480, 5800};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.05;
        ADC_threshold = 500;
      }
      else
      {
        if (m_verbose) {print("DSSD not to be taken in this calibration file");}
      }
    }
    else {if (m_verbose) std::cout << name << " not a handled detector" << std::endl; continue;}
    fit.resize(nb_pics);// Resize the intern vectors of the detector's pic_fit_result

    int vmaxchan = 0;// Position of the right pic in bins (e.g. 1408 keV in Eu)
    double scalefactor = 0.; // To adapt to the binnin of the histogram (ADC/bin)
    double kpc = 0.; // keV per bin (=channel)

    // -----------------------
    // OPERATE ON THE SPECTRUM
    // -----------------------
    int nbins = histo->GetXaxis()->GetNbins();// The number of bins in the spectra

    // It supposes that the spectra starts at 0 !!
    fit.scalefactor = scalefactor = histo->GetXaxis()->GetXmax()/nbins;

    Int_t ADC_threshold_scaled = ADC_threshold/scalefactor; 
    double sum=histo->Integral(ADC_threshold_scaled, nbins-1); // The total integral of the spectra
    fit.nb_hits = sum;
    if (sum < 50000)
    {// No calibration if not enough counts in the spectra
      if (m_verbose)
      {
        fit.too_few_counts(true);
        std::cout << "Too few counts to calibrate : " << sum << std::endl;
      }
      continue;
    }
    if (m_verbose)
    {
      std::cout << "Integral = " << sum << " counts; Number of bins = " <<  nbins << std::endl;
      std::cout << "scale factor = " << scalefactor << std::endl;
    }
    
    // Initialises vmaxchan at the last bin (right of the spectra)
    vmaxchan=nbins;
    double integral=0; // Integral

    // We start at nbins-2 because the nbin'th bin is the overflow, so to be secure we start at nbin-2
    for (int j=nbins-2; j > ADC_threshold_scaled; j--)
    {
      integral+=histo->GetBinContent(j);
      if ((integral/sum) > integral_ratio_threshold) {vmaxchan=j; break;}
    }

    if (vmaxchan==ADC_threshold_scaled) { if (m_verbose) print("Could not fit this spectrum "); continue;}
    if (m_verbose) print("right pic found at channel ", vmaxchan*scalefactor, " ADC");
    kpc=E_right_pic/vmaxchan;
    fit.keVperADC = kpc;
    if (m_verbose) print("kev per channel = ", kpc);
    // for (int j=0; j<nb_pics; j++) //Starting with the lower energy pic
    for (int j=nb_pics-1; j>-1; j--) //Starting with the higher energy pic
    {
      if (m_verbose) print("Pic : " , fit.peaks[j]);
      // 1st window :
      double p=fit.peaks[j]; // Energy of the peak (in keV)
      double cguess_low  = (p-window_1)/kpc; // Low  edge of the window (in bins)
      double cguess_high = (p+window_1)/kpc; // High edge of the window (in bins)
      if (cguess_low<ADC_threshold_scaled) cguess_low = ADC_threshold_scaled; //cannot read the spectra below the ADC threshold !
      histo->GetXaxis()->SetRange(cguess_low, cguess_high); // Setup the window to the spectra
      double cmc = 0.5 + histo->GetMaximumBin();
      if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cmc*scalefactor);

      // 2nd window :
      cguess_high = 0.5 + cmc + window_2/kpc;
      cguess_low  = 0.5 + cmc - window_2/kpc;
      histo->GetXaxis()->SetRange(cguess_low, cguess_high);
      double cm  = histo->GetMean(); //in ADC (scalefactor*bins)
      cmc = cm/scalefactor;
      if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cm);

      // 3rd window :
      cguess_high = cmc + window_3/kpc;
      cguess_low  = cmc - window_3/kpc;
      histo->GetXaxis()->SetRange(cguess_low, cguess_high);
      cm  = histo->GetMean(); //in ADC
      cmc = cm/scalefactor; //in bins
      fit.cmeasures[j]=cmc; //The measured channel number
      if (m_verbose) print("[", cguess_low*scalefactor , " , " , cguess_high*scalefactor , "] First mean = " , cm);

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
      histo -> Fit(gaus,"RQ+");
      fit.mean [j] = gaus -> GetParameter(1);
      fit.sigma[j] = gaus -> GetParameter(2);
      // TF1* gaus_pol0(new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)"));
      // gaus_pol0 -> SetRange(cguess_low*scalefactor, cguess_high*scalefactor);
      // gaus_pol0 -> SetParameter(0, gaus_pol0 -> GetParameter(0));
      // gaus_pol0 -> SetParameter(1, gaus_pol0 -> GetParameter(1));
      // gaus_pol0 -> SetParameter(2, gaus_pol0 -> GetParameter(2));
      // gaus_pol0 -> SetParameter(3, histo -> GetBinContent(0));
      // histo -> Fit(gaus_pol0,"RIQE");
      // fit.mean [j] = gaus_pol0 -> GetParameter(1);
      // fit.sigma[j] = gaus_pol0 -> GetParameter(2);
      // print(label, "kpc : ", kpc, " scale : ", scalefactor, " pic : ", p, " -> ", cguess_low*kpc, " : ", cguess_high*kpc);
      // print(label, " pic : ", p, " -> ", cm, " ", histo -> GetBinContent(0));

      // If the peak is the higher in energy, then use it as a little bit more precise rough linear calibration :
      if (j == nb_pics) kpc = cmc; 
    }

    std::vector<Double_t> x (nb_pics);
    std::vector<Double_t> y (nb_pics);
    std::vector<Double_t> ex(nb_pics);
    std::vector<Double_t> ey(nb_pics);
    for (int j=0; j < nb_pics; j++)
    {
      if (m_verbose) std::cout << "Energy = " << fit.peaks[j] << " Channel = " << fit.cmeasures[j]*scalefactor << std::endl;
      x [j]=fit.cmeasures[j]*scalefactor;
      y [j]=fit.peaks[j];
      ex[j]=0;
      ey[j]=0;
    }
    auto c1 = TCanvas(("c_"+name).c_str());
    TGraphErrors* gr = new TGraphErrors(nb_pics,x.data(),y.data(),ex.data(),ey.data());
    gr -> SetName((name+"_gr").c_str());
    TF1* linear(new TF1("lin","pol1")); //Range and number of fit parameters
    gr->Fit(linear,"q");
    TF1* binom (new TF1("pol", "pol2"));

    if(isGe[label] || isDSSD[label])
    {//First order fit
      fit.parameter0 = linear -> GetParameter(0);
      fit.parameter1 = linear -> GetParameter(1);
      fit.chi2       = linear -> GetChisquare( );
      fit.exists(true);
    }
    else if (isLaBr3[label] || isParis[label])
    {// Second order fit
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

  // {
  //   outRootFile -> cd();
  //   auto c1 = new TCanvas;
  //   c1 -> cd();
  //   gr -> Draw("A*");
  //   c1 -> Write();
  //   delete c1;
  //   gROOT -> cd();
  // }
}

//DEV :
void Calibration::setCalibrationTables()
{
  print("creating calibration tables");
  calibration_tables.resize(m_max_labels);
  std::vector<std::vector<float>> *calib_vec;
  for (Label i = 0; i<m_max_labels; i++)
  {
    calib_vec = &calibration_tables[i];
    calib_vec->resize(200000);
  }
  print("Done !");
  print();
}
//!DEV

inline float Calibration::calibrate(float const & nrj, Label const & label) const 
{
  auto nrj_r = nrj+gRandom->Uniform(0,1);
  switch(m_order[label])
  {
    case 0:
      return nrj_r;

    case 1:
      return m_intercept[label]
             + m_slope[label] * nrj_r;

    case 2:
      return m_intercept[label]
             + m_slope[label] * nrj_r
             + m_binom[label] * nrj_r * nrj_r;

    case 3:
      return m_intercept[label]
             + m_slope [label] * nrj_r
             + m_binom [label] * nrj_r * nrj_r
             + m_trinom[label] * nrj_r * nrj_r * nrj_r;

    default:
      return nrj;
  }
}

inline void Calibration::calibrate(Hit & hit) const
{
  auto const & label = hit.label;
  if (label > m_max_labels) return;

#if defined (LICORNE)
  if (is_EDEN(label))
  {
    if (hit.nrj2==0) hit.nrj2 = 1;
    hit.nrjcal = static_cast<float>(hit.nrj2)/hit.nrj;
  }
#elif defined (PARIS)
  if (isParis[label]) hit.nrj2 = calibrate(hit.nrj2, label);
#endif

  if (isBGO[label]) hit.nrjcal = (hit.nrj+gRandom->Uniform(0,1))/100;
  else hit.nrjcal = calibrate(hit.nrj, label);
}

void Calibration::set(Label _label, float _intercept = 0.f, float _slope = 1.f, float _binom = 0.f, float _trinom = 0.f)
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
  auto const & label_max = m_detectors.size();
  std::ifstream inputfile;
  inputfile.open(calibFileName);
  if (!inputfile.good()) return false;
  std::string line = "";
  Label label = 0;
  m_order    .resize(label_max,-1.f);
  m_intercept.resize(label_max, 0.f);
  m_slope    .resize(label_max, 1.f); //Fill with 1
  m_binom    .resize(label_max, 0.f);
  m_trinom   .resize(label_max, 0.f);
  float intercept = 0.f, slope = 1.f, binom = 0.f, trinom = 0.f;
  while (getline(inputfile, line))
  {
    m_nb_detectors++;
    std::istringstream iss(line);
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

#endif //CALIBRATION_H
