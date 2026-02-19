#include "../../lib/Analyse/AnalysedSpectra.hpp"
#include "../../lib/Analyse/SpectraCo.hpp"

#define NSI129

/**
 * @brief first step : uses the calibration on the run data
 * and perform a manual peak finding. The result is 136_ajustation.calib
 * calibration file, that links the 152Eu calibration data to in-beam calibration.
 * Now you have to copy the data in a file named 136_ajustation_checked.calib and correct
 * the eventual issues by looking at the spectra in 136_recal.root
 * 
 */
void recal(std::string choice_detectors = "PARIS", std::vector<double> peaks = {})
{
  std::vector<double> Eu152 = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
  std::vector<double> Th232 = {238.632, 338.32, 510.770, 583.191, 911.204, 2614.533};
  if (choice_detectors == "DSSD") Colib::throw_error("Can't be used for DSSD");
  detectors.load("index_129.list");
#ifdef NSI129
  // std::string infile = "~/faster_data/N-SI-129-run_histo/merged/fused_histo.root";
  std::string infile = "~/faster_data/N-SI-129-source_histo/Th232_both_sides.root";
  peaks = Th232;
  Calibration calib("../129_2024.calib");
#else
  std::string infile = "test_calib.root";
#endif //NSI129
  auto file(TFile::Open(infile.c_str()));
  file->cd();
  auto histos = get_TH1F_names(file);
  Calibration out_calib;

  information("Histos loaded from", infile);
  std::map<Label, SpectraCo> spectra;
  TF1* fit_linear = new TF1("linear", "pol1");

  for (auto name : histos)
  {
    auto hist = file->Get<TH1F>(name.c_str());
    if (hist->Integral()<100) continue;

    remove(name, "_calib");
    auto const & label = detectors[name];

    int rebin = 2;
    int smooth = 10;
    if (found(name, "PARIS")) 
    {
      if (choice_detectors!="PARIS") continue;
#ifdef NSI129
      rebin = 2; 
      smooth = 20; 
#else
      rebin = 20; 
      smooth = 20; 
#endif //NSI129
      if (peaks.size() == 0) peaks = {511, 1779};
    }
    else if (found(name, "red") || found(name, "blue") || found(name, "green") || found(name, "black")) 
    {
      if (choice_detectors!="GE") continue;
      rebin = 10; 
      smooth = 10; 
    }
    else if (found(name, "BGO")) 
    {
      if (choice_detectors!="BGO") continue;
      rebin = 40; 
      smooth = 10; 
    }
    else continue;
    
    information("treating", name);

    // Load the spectra in a SpectraCo to remove its background
    spectra.emplace(label, hist);
    auto & spectrum = spectra.at(label);
  #ifdef NSI129
    spectrum.calibrate(calib, detectors.label(name));
  #endif //NSI129
    spectrum.rebin(rebin);
    spectrum.removeBackground(smooth);
    spectrum.resizeBin(spectrum.getBin(10000));

    // Use AnalysedSpectra to use the online manual peak finder
    AnalysedSpectra spectraA(spectrum.createTH1F(name+"_analyse", name));
    auto peaks_found = spectraA.fitPeaks(peaks);
    std::vector<double> peaks_ADC;
    for (auto const & peak : peaks_found) peaks_ADC.push_back(peak.mean());

    // Fit the calibration curve :
    auto graph = new TGraph(peaks_ADC.size(), peaks_ADC.data(), peaks.data());
    graph->Fit(fit_linear);

    // Save the calibration coefficient and use them to calibrated the spectra :
    out_calib.set(label, fit_linear->GetParameter(0),  fit_linear->GetParameter(1));
    spectrum.calibrate(out_calib, label);
  }

  file->Close();

#ifdef NSI129
  std::string outname = "129_recal.root";
#else
  std::string outname = "136_recal.root";
#endif //NSI129
  auto outFile = TFile::Open(outname.c_str(), "recreate");
  outFile->cd();
  for (auto it : spectra) it.second.write();
  outFile->Write();
  outFile->Close();
  print(outname, "written");
  
#ifdef NSI129
  out_calib.write("129_ajustation"); 
#else
  out_calib.write("136_ajustation");
#endif //NSI129

}