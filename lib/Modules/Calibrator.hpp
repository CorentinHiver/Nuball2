#ifndef CALIBRATOR_HPP
#define CALIBRATOR_HPP

#include "../libRoot.hpp"
#include "../Classes/Hit.hpp"

#include "../Classes/Fit.hpp"
#include "../Classes/Calibration.hpp"
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

  Calibrator(int argc, char** argv);

  /// @brief Loading calibration from file name
  bool loadCalibration(Calibration const & calib) {return (m_calib = calib);}

  void calculate(std::string const & dataDir, int nb_files = -1, std::string const & source = "152Eu", std::string const & type = "fast");

  /// @brief Calculate calibration from .root histograms
  /// @attention TODO
  void calculate(std::string const & histograms, std::string const & source = "152Eu");

  void Initialize() {m_histos.Initialize(); m_fits.resize(1000);}

  void loadRootData(std::string const & dataDir, int const & nb_files = -1);
  static void loadRootDataThread(Calibrator & calib, MTList & list);
  void fillRootDataHisto(std::string const & filename);

  void loadFasterData(std::string const & dataDir, int const & nb_files = -1);
  static void fillHisto(Hit & hit, FasterReader & reader, Calibrator & calib);
  void analyse(std::string const & source = "152Eu");
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
  void writeCalibratedData(std::string const & outfilename);

  bool const & isFilled() const {return m_ok;}

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
  bool      m_verbose   = false;
  bool      m_residus   = false;
  bool      m_outRoot_b = false;
  std::string m_source    = "";
  std::string m_outRoot   = "calibration.root";
  std::string m_outCalib  = "";
  std::string m_outDir    = "Calibrator/";

  static bool m_treatOnlyParis;
  static bool m_treatOnlyGe;

  Fits m_fits;

  Path dataPath;

  //Attributs for the tables :
  bool m_ok = false;
  bool m_calibrate_data = false;
  Label m_nb_detectors = 0;
  Label m_size = 0;
  std::vector<char> m_order; //1, 2 or 3 | 0 -> no calibration
  std::vector<NRJ> m_intercept;
  std::vector<NRJ> m_slope;
  std::vector<NRJ> m_binom;
  std::vector<NRJ> m_trinom;
  std::vector<std::vector<std::vector<NRJ>>> calibration_tables;

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
    std::map<int, TH1F*> spectra;

    void Initialize();
    void setBins(std::string const & parameters);
    bool initialized = false;
  } m_histos;
};

bool Calibrator::m_treatOnlyParis = false;
bool Calibrator::m_treatOnlyGe = false;

void Calibrator::printParameters()
{
  print("Usages of Calibrator : ");
  print("");
  print("parameters :");
  print("-F [[nb_files]]  : Read data files, either .fast or already converted .root file with a root tree.");
  print("-f               : Reads a .root file containing the histograms you want to use as a calibration run.");
  print("-o               : Overwrite the ouput file if it already exists.");
  print("-O               : Setup the name of the output.");
  exit();
}

Calibrator::Calibrator(int argc, char** argv)
{
  if (argc<2) 
  {
    print("Not enough parameters for Calibrator");
    printParameters();
  }
  
}

void Calibrator::histograms::Initialize()
{
  if (initialized) return;
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
    for (size_t index = 0; index<detectors.nbOfType(type); index++)
    {
      auto const & name = detectors.name(type, index);
      auto const & label = detectors.label(type, index);
      auto const & bin_raw = detectors.ADCBin(type);
      auto const & bin_calib = detectors.energyBin(type);
      raw_spectra[label].reset((name+"_raw").c_str(), (name+" raw spectra").c_str(), bin_raw.bins, bin_raw.min, bin_raw.max);
      calib_spectra[label].reset((name+"_calib").c_str(), (name+" calibrated spectra").c_str(), bin_calib.bins, bin_calib.min, bin_calib.max);
    }
  }
  initialized = true;
}

void Calibrator::histograms::setBins(std::string const & parameters)
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

// TBD
void Calibrator::calculate(std::string const & histograms, std::string const & source)
{
  print ("Calculating calibrations from histogram data in", histograms);
  this -> Initialize();
  // this -> loadRootHisto(file); TBD
  this -> analyse(source);
  this -> writeData(source+".calib");
  this -> writeRawRoot(source+".root");
}

void Calibrator::loadRootHisto(std::string const & histograms)
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

void Calibrator::loadFasterData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialize();
  MTFasterReader *mt_reader = new MTFasterReader();
  mt_reader->addFolder(dataDir, nb_files);
  mt_reader->readRaw(fillHisto, *this);
  delete mt_reader;
  print("Data loaded");
}

void Calibrator::loadRootData(std::string const & dataDir, int const & nb_files)
{
  print("Loading the data from", Path(dataDir).folder());
  this -> Initialize();
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

void Calibrator::fillHisto(Hit & hit, FasterReader & reader, Calibrator & calib)
{
  if (calib.calibrate_data()) while(reader.Read()) 
  {
    auto const & type = detectors.type(hit.label);
    if (type ==  "null" || type ==  "RF") continue;
    if (calib.m_order[hit.label]<1) continue;
    auto const & nrj_cal = calib.calibration()(hit.adc, hit.label);
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

void Calibrator::verify(std::string const & outfilename)
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
  writeCalibratedHisto(outfilename+"_calib.root");
}

void Calibrator::writeCalibratedHisto(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : m_histos.all_calib) histo.Write();
  for (auto & histo : m_histos.calib_spectra)  histo.Write();
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

void Calibrator::writeCalibratedData(std::string const & outfilename)
{
  unique_TFile outFile(TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile->cd();
  for (auto & histo : m_histos.all_calib) histo.Write();
  for (auto & histo : m_histos.calib_spectra)  histo.Write();
  print("Calibrated root spectra written to", outfilename);
}

#endif //CALIBRATOR_HPP