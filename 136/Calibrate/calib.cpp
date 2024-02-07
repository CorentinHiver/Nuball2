#include "../libRoot.hpp"
#include <SpectraCo.hpp>

#define NSI129

//argv parameters : 1 -> only the source, 2 -> only the runs, default -> both
int main(int argc, char ** argv)
{
  detectors.load("index_129.list");
  Calibration calib;
  std::map<Label, SpectraCo> spectra;
  // ----------------------------------------------------------- //
  // This part uses the 152Eu run to calibrate the BGO and PARIS
  // ----------------------------------------------------------- //
  std::vector<double> Eu152 = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
  std::vector<double> Th232 = {238.632, 338.32, 510.770, 583.191, 911.204, 2614.533};

#ifdef NSI129
  auto peaks_th = Th232;
  auto file (TFile::Open("~/faster_data/N-SI-129-source_histo/Th232_both_sides.root", "READ"));
#else 
  auto peaks_th = Eu152;
  auto file (TFile::Open("~/faster_data/N-SI-136-sources_histo/152Eu_center.root", "READ"));
#endif //NSI129
  auto histo_map (get_TH1F_map(file));
  gROOT -> cd();

  double energy = peaks_th.back();


  // Peak finder parameters :
  int rebin = 20;  // Smooth parameter used to calculate the second derivative
  double threshold = 20; // Threshold below wich the spectrum must go to detect a peak
  int nb_below_threshold = 5; // Number of bins the spectrum must stay below the thrshold for the peak to be validated
  
  for (auto const & it : histo_map)
  {
    if (argc>1 && std::stoi(argv[1]) == 2) continue;
    // Load the spectra :
    auto name  = it.first ;
    auto histo = it.second;
    auto const & label = detectors[name];

    if (found(name, "BGO")) {histo->Rebin(20); rebin = 50; threshold = 4; nb_below_threshold = 1;} 
    else if (found(name, "PARIS"))
    {
      histo->Rebin(10);
      if (found(name, "FR")) {rebin = 10; threshold = 250; nb_below_threshold = 5;}
      else                   {rebin = 9;  threshold = 250; nb_below_threshold = 5;}
    }
    else if (found(name, "red") || found(name, "green") || found(name, "black") || found(name, "blue"))
    {
      rebin = 20;  threshold = 500; nb_below_threshold = 5;
      continue;
    }
    else continue;

    // Put the spectra inside a map of SpectraCo :
    spectra.emplace(label, histo);
    auto & spectrum = spectra[label]; // Alias of the spectrum
    spectrum.setActualRange(); // Find the actual end of the spectra, i.e. without energy

    // Find peak and extract the peaks :
    auto const & peaks = spectrum.findPeaks(threshold, nb_below_threshold, rebin);
    std::vector<int> bins; std::vector<double> heights;
    unpack(peaks, bins, heights);
    if (bins.size() < 1) {warning("Detector", name, "(n°", label, ") has no peaks !"); continue;}

    // Take the high energy peak for a first raw calibration :
    auto const & max_peak_bin = maximum(bins);
    auto const & max_peak_ADC = spectrum.getX(max_peak_bin);
    auto adc_to_keV = energy/max_peak_ADC;
    // print(GREY, name, detectors[name], max_peak_ADC, adc_to_keV, RESET);
    
  #ifdef NSI129
    std::vector<double> ADCs(Th232.size());
  #else 
    std::vector<double> ADCs(peaks_th.size());
  #endif //NSI129


    if (found(name, "BGO")) 
    {
      calib.set(label, 0, adc_to_keV);
      print(label, adc_to_keV);
    }
    else if (found(name, "PARIS") || found(name, "red") || found(name, "green") || found(name, "black") || found(name, "blue"))
    {// For paris, try to find the other peaks for a better fit
      std::vector<int> ordered_peaks;
      bubble_sort(peaks_th, ordered_peaks);
      invert(ordered_peaks); // Start from the heigher energy peak
      for (auto const & peak_i : ordered_peaks)
      {
        auto guess_ADC = peaks_th[peak_i]/adc_to_keV;

        auto derivative2 = *(spectrum.derivative2());
        auto rebin_keV = derivative2.getX(rebin);

        guess_ADC = derivative2.meanInRange(guess_ADC-5*rebin_keV, guess_ADC+5*rebin_keV);

        guess_ADC = derivative2.meanInRange(guess_ADC-2*rebin_keV, guess_ADC+2*rebin_keV);
        auto minimum = minimum_in_range(derivative2.data(), derivative2.getBin(guess_ADC-rebin_keV), derivative2.getBin(guess_ADC+rebin_keV));
        
        guess_ADC = spectrum.getX(minimum.first);

        ADCs[peak_i] = guess_ADC;
      }
      auto graph( new TGraph(peaks_th.size(), ADCs.data(), peaks_th.data()));
      TF1* linear(new TF1("lin","pol1"));
      graph->Fit(linear,"q");
      calib.set(label, linear->GetParameter(0), linear->GetParameter(1));
      print(label, linear->GetParameter(0), linear->GetParameter(1));
      delete graph; 
      delete linear;
    }
  }

  file->Close();

  // ---------------------------------------------------- //
  // This part uses the Uranium run to calibrate the DSSD
  // ---------------------------------------------------- //

  // Load the run spectra in order to calibrate the DSSD : 
  energy = 11000; // the higher energy peak is the elastic 11Mev deuteron peak
#ifdef NSI129
  auto runs (TFile::Open("~/faster_data/N-SI-129-run_histo/merged/fused_histo.root", "READ"));
#else 
  auto runs (TFile::Open("~/faster_data/N-SI-136-U_histo/total/fused_histo.root", "READ"));
#endif //NSI129
  auto histo_map_runs (get_TH1F_map(runs));

  for (auto const & it : histo_map_runs)
  {
  if (argc>1 && std::stoi(argv[1]) == 1) continue;
    // Load the spectra :
    auto name  = it.first ;
    auto histo = it.second;
    auto const & label = detectors[name];
    print(name, label);

    if (found(name, "DSSD")) 
    { 
      print("found");print();
      histo->Rebin(2);
      rebin = 100; threshold = 100; nb_below_threshold = 20;

      spectra.emplace(label, histo);
      auto & spectrum = spectra[label]; // Alias of the spectrum
      spectrum.setActualRange(); // Find the actual end of the spectra, i.e. without energy
      spectrum.removeBackground(20);
      spectrum.derivate2(100);

      // Find peaks and extract the peaks :
      auto const & peaks = spectrum.findPeaks(threshold, nb_below_threshold);
      std::vector<int> bins; std::vector<double> heights;
      unpack(peaks, bins, heights);
      if (bins.size() < 1) {warning("Detector", name, "(n°", label, ") has no peaks !"); continue;}

      // Take the high energy peak for a first raw calibration :
      auto const & max_peak_bin = maximum(bins);
      auto const & max_peak_ADC = spectrum.getX(max_peak_bin);
      auto adc_to_keV = energy/max_peak_ADC;

      calib.set(label, 0, adc_to_keV);
    }
  }

  runs->Close();

  // Calibrate the spectra :
  for (auto & it : spectra) it.second.calibrate(calib, it.first);

  // Write the calibrated spectra
  std::string outname = "out.root";
  auto outfile(TFile::Open(outname.c_str(),"recreate"));
  outfile->cd();
  for (auto & it : spectra) it.second.derivative2()->write();
  for (auto & it : spectra) it.second.write();
  outfile->Write();
  outfile->Close();
  print(outname, "written");

  // Write the first raw calibration data
  #ifdef NSI129
  calib.write("129");
#else 
  calib.write("136");
#endif //NSI129


  return 0;
}
