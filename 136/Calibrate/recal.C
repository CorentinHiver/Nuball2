#include "../../lib/Analyse/AnalysedSpectra.hpp"
#include "../../lib/Analyse/SpectraCo.hpp"

/**
 * @brief first step : uses the 152Eu calibration on the run data
 * and perform a manual peak finding. The result is 136_ajustation.calib
 * calibration file, that links the 152Eu calibration data to in-beam calibration.
 * Now you have to copy the data in a file named 136_ajustation_checked.calib and correct
 * the eventual issues by looking at the spectra in 136_recal.root
 * 
 */
void recal(std::string detectors = "PARIS")
{
  detectors.load("index_129.list");
  std::string infile = "test_calib.root";
  auto file(TFile::Open(infile.c_str()));
  file->cd();
  auto histos = get_TH1F_names(file);
  Calibration calib;

  information("Histos loaded from", infile);
  std::map<Label, SpectraCo> spectra;
  TF1* fit_linear = new TF1("linear", "pol1");

  for (auto name : histos)
  {
    auto hist = file->Get<TH1F>(name.c_str());
    if (hist->Integral()<1) continue;

    remove(name, "_calib");
    auto const & label = detectors[name];

    int rebin = 2;
    int smooth = 10;
    std::vector<double> peaks;
    if (found(name, "PARIS")) 
    {
      rebin = 20; 
      smooth = 20; 
      peaks = {511, 1779};
      if (detectors!="PARIS") continue;
    }
    else if (found(name, "red") || found(name, "blue") || found(name, "green") || found(name, "black")) 
    {
      rebin = 10; 
      smooth = 10; 
      if (detectors!="GE") continue;
    }
    else if (found(name, "BGO")) 
    {
      rebin = 40; 
      smooth = 10; 
      if (detectors!="BGO") continue;
    }
    else continue;
    
    information("treating", name);

    // Load the spectra in a SpectraCo to remove its background
    spectra.emplace(label, hist);
    auto & spectrum = spectra.at(label);
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
    calib.set(label, fit_linear->GetParameter(0),  fit_linear->GetParameter(1));
    spectrum.calibrate(calib, label);
  }

  file->Close();

  std::string outname = "136_recal.root";
  auto outFile = TFile::Open(outname.c_str(), "recreate");
  outFile->cd();
  for (auto it : spectra) it.second.write();
  outFile->Write();
  outFile->Close();
  print(outname, "written");

  calib.write("136_ajustation");


}