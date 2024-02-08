#include "../../lib/Analyse/SpectraCo.hpp"
#include "../../lib/Analyse/AnalysedSpectra.hpp"

int find_max_peak(TH1F* spectrum, double ratio_threshold, double ADC_threshold = 500)
{
  auto const & total_sum = spectrum->Integral();

  auto const & bin_threshold = spectrum->FindBin(ADC_threshold);
  int sum = 0;
  int bin = spectrum->GetNbinsX();
  for (; bin>bin_threshold; --bin)
  {
    sum+=spectrum->GetBinContent(bin);
    if (sum/total_sum > ratio_threshold) return bin;
  }
  return -1;
}



void calibBGO129(bool draw = false)
{
  auto const & ADC_threshold = 500;
  auto const & ratio_threshold = 0.025;
  int rebin = 50;
  int background_smooth = 5;
  detectors.load("../index_129.list");
  bool skip_draw_Th = false;

  std::vector<double> peaks = {511, 1274.5, 2614.533};
  std::map<std::string, double> points_511;
  std::map<std::string, double> points_1274;
  std::map<std::string, double> points_2614;
  std::map<std::string, double> kpbs;
  std::vector<std::string> names;
  for (auto const & name : detectors.names()) if (found(name, "BGO")) names.push_back(name);
  std::map<std::string, AnalysedSpectra> spectraA;
  std::map<std::string, SpectraCo> spectra_calib;
  Calibration calib;

    // int i = 0;

  auto file(TFile::Open("~/faster_data/N-SI-129-source_histo/Th232_both_sides.root", "READ"));
  {
    file->cd();
    auto spectra(get_TH1F_map(file));

    for (auto const & name : names)
    {
      print(name, spectra.count(name));
      if (spectra.count(name) != 1) continue;
      auto const & spectrum = spectra.at(name);
      if (!spectrum) continue;
      spectrum->SetDirectory(nullptr);
      auto const & label = detectors.label(name);

      // if (i++ > 1) break;

      spectrum->Rebin(rebin);
      spectrum->GetXaxis()->SetRange(spectrum->FindBin(ADC_threshold), spectrum->FindLastBinAbove(5));

      // Fit 2614
      // auto const & peak_ADC = spectrum->GetBinCenter(find_max_peak(spectrum, ratio_threshold));
      // CoAnalyse::removeBackground(spectrum, background_smooth);
      // spectrum->GetXaxis()->SetRangeUser(peak_ADC*0.8, peak_ADC*1.2);

      // auto const & mean = spectrum->GetMean();


      // PeakFitter fit(spectrum, mean*0.9, mean*1.1);

      if (draw && !skip_draw_Th)
      {
        gPad->SetTitle(concatenate("2614 ", name).c_str());
        // spectrum->GetXaxis()->SetRangeUser(peak_ADC*0.5, peak_ADC*1.5);
        spectrum->Draw();
        // gPad->WaitPrimitive();
        double x; double y;
        GetPoint(gPad, x, y);
        points_2614[name] = x;
        kpbs[name] = peaks[2]/x;
        calib.set(label, kpbs[name]);
        calib.calibrateAxis(spectrum, label);
        continue;
      }
      
      // points_2614[name] = fit.getMean();
      // kpbs[name] = peaks[2]/fit.getMean();

      // spectraA[name] = spectrum;
      // auto const & peak_ADC = spectraA[name].fitPeak(peaks[2], false);
      // points_2614[name] = peak_ADC.mean();
      // kpbs[name] = peaks[2]/points_2614[name];
      // print(name, kpbs[name]);
    }
  }
  // for (auto const & name : names) print(name, points_2614[name]);
  file->Close();

  std::string const & filename = "test.root";
  file = TFile::Open(filename.c_str(), "RECREATE");
  file->cd();
  for (auto & name : names)
  {
    auto & spectrum = spectra.at(name);
    spectrum->SetDirectory(file);
    spectrum->Write();
  }
  file -> Write();
  file -> Close();
  print(filename);

  calib.write("129_BGO.calib");

//   file = TFile::Open("~/faster_data/N-SI-129-source_histo/Na22.root", "READ");
//   {
//     file->cd();
//     auto spectra(get_TH1F_map(file));
// // i = 0;
//     for (auto const & name : names)
//     {
//       if (spectra.count(name) != 1) continue;
//       auto const & spectrum = spectra.at(name);
//       if (!spectrum) continue;
//       // if (i++ > 1) break;

//       spectrum->Rebin(rebin);
//       spectrum->GetXaxis()->SetRange(spectrum->FindBin(ADC_threshold), spectrum->FindLastBinAbove(5)); // Set range to actual spectrum
//       CoAnalyse::removeBackground(spectrum, background_smooth);

//       // Fit 1274
//       auto const & ADC_1274 = peaks[1]/kpbs[name];
//       print(kpbs[name], peaks[1], ADC_1274, peaks[0], peaks[0]/kpbs[name]);

//       if (draw)
//       {
//         spectrum->GetXaxis()->SetRangeUser(0, ADC_1274*1.5);
//         double x; double y;
//         gPad->SetLogy();
//         spectrum->Draw();
//         gPad->SetTitle(concatenate("1279 ", name).c_str());
//         GetPoint(gPad, x, y);
//         points_1274[name] = x;
//         gPad->SetTitle(concatenate("511 ", name).c_str());
//         GetPoint(gPad, x, y);
//         points_511[name] = x;
//       }

//       // spectrum->GetXaxis()->SetRangeUser(ADC_1274*0.8, ADC_1274*1.2);

//       // auto const & mean_1274 = spectrum->GetMean();

//       // PeakFitter fit_1274(spectrum, mean_1274*0.9, mean_1274*1.1);

//       // // kpbs[name] = peaks[1]/fit_1274.getMean();


//       // // Fit 511
//       // auto const & ADC_511 = peaks[0]/kpbs[name];

//       // auto const & mean_511 = spectrum->GetMean();

//       // PeakFitter fit_511(spectrum, mean_511*0.9, mean_511*1.1);

//       // points_1274[name] = fit_1274.getMean();
//       // points_511[name] = fit_511.getMean();


//       // spectrum->GetXaxis()->SetRangeUser(ADC_511*0.5, ADC_511*1.5);
//       // if (draw)
//       // {
//         // print(ADC_511*0.5, ADC_511*1.5);
//         // gPad->SetTitle("511");
//         // spectrum->Draw();
//         // gPad->WaitPrimitive();
//         // gPad->SetTitle("511");
//       // }



//       // spectraA[name] = spectrum;
//       // spectraA[name].setRatioRange(0.5, 1.5);
//       // auto const & peaks_fit = spectraA[name].fitPeaks({peaks[0], peaks[1]}, false);
//       // points_1274[name] = peaks_fit[0].mean();
//       // points_511[name] = peaks_fit[0].mean();
//       // auto fitted_peaks (spectraA[name].getPeaks());
//       // for (auto const & p : fitted_peaks) std::cout << p << std::endl;
//       // print(points_1274[name], points_511[name]);
//     }
//   }
//   file->Close();

//   Calibration calib;
//   TF1* linear(new TF1("lin","pol1"));
//   std::map<std::string, TGraph*> graphs;
//   // for (auto const & name : names) print(name, points_511[name], points_1274[name], points_2614[name]);
//   for (auto const & name : names)
//   {
//     // print(name);
//     auto const & label = detectors.label(name);
//     std::vector<double> ADCs(3, 0);
//     ADCs[0] = points_511[name];
//     ADCs[1] = points_1274[name];
//     ADCs[2] = points_2614[name];

//     // for (auto const & e : peaks) std::cout << e << " ";
//     // print();
//     // for (auto const & e : ADCs) std::cout << e << " ";

//     auto graph( new TGraph(peaks.size(), ADCs.data(), peaks.data()));
//     // graph->Draw();
//     // gPad->WaitPrimitive();
//     graph->Fit(linear,"q");
//     calib.set(label, linear->GetParameter(0), linear->GetParameter(1));
//     graph->SetName(name.c_str());
//     graphs.emplace(name, graph);
//     // print(label, linear->GetParameter(0), linear->GetParameter(1));
//   }

//   std::map<std::string, SpectraCo> spectra_clean;
//   TH2F* all_BGOs = new TH2F("all_BGOs", "All BGOs;BGO n°;E [keV]", 24,0,24,  500,0,5000);

//   file = TFile::Open("~/faster_data/N-SI-129-source_histo/Na22.root", "READ");
//   // file = TFile::Open("~/faster_data/N-SI-129-source_histo/60Co_center_spectra.root", "READ");
//   {
//     file->cd();
//     auto spectra(get_TH1F_map(file));
//     int histos = 0;
// // i = 0;
//     for (auto const & name : names)
//     {
//       if (spectra.count(name) != 1) continue;
//       auto const & spectrum = spectra.at(name);
//       if (!spectrum) continue;
//       auto const & label = detectors.label(name);
//       // if (i++ > 1) break;

//       spectrum->GetXaxis()->SetRange(0, spectrum->FindLastBinAbove(5));

//       spectra_clean.emplace(name, spectrum);
//       auto & spectrum_clean = spectra_clean[name];
//       spectrum_clean.name() += "_calib";
//       spectrum_clean.calibrate(calib, label);
//       for (int bin = 0; bin<all_BGOs->GetYaxis()->GetNbins(); bin++)
//       {
//         auto const & keV_value = all_BGOs->GetYaxis()->GetBinCenter(bin);
//         all_BGOs->SetBinContent(histos, bin, spectrum_clean.interpolate(spectrum_clean.getBin(keV_value)));
//       }
//       histos++;
//     }
//   }

//   file->Close();

//   all_BGOs->Draw("colz");


}