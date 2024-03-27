#include "../../lib/Analyse/SpectraCo.hpp"
#include "../../lib/Analyse/AnalysedSpectra.hpp"
#include "../../lib/Analyse/Paris.hpp"

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

void calib129(std::string detector = "", int choice = 0)
{
  if (detector == "") print("Detectors are BGO, FR and BR (last 2 for PARIS FRONT and PARIS BACK). Choices are 0 (232Th) or 1 (22Na)");
  auto const & ADC_threshold = 500;
  auto const & ratio_threshold = 0.025;
  int background_smooth = 5;
  detectors.load("../index_129.list");
  bool skip_draw_Th = false;

  std::vector<double> peaks = {511, 1274.5, 2614.533};
  std::map<std::string, double> points_511;
  std::map<std::string, double> points_1274;
  std::map<std::string, double> points_2614;
  std::map<std::string, double> kpbs;
  std::vector<std::string> names;
  for (auto const & name : detectors.names()) if (found(name, detector)) names.push_back(name);
  std::map<std::string, AnalysedSpectra> spectraA;
  std::map<std::string, SpectraCo> spectra_calib;
  Calibration calib;

  std::map <std::string, std::string> filenames;
  filenames["Th"] = "~/faster_data/N-SI-129-source_histo/Th232_both_sides.root";
  filenames["Na"] = "~/faster_data/N-SI-129-source_histo/Na22.root";

  if (detector == "BGO")
  {
    int rebin = 50;
    if (choice == 0)
    {
      auto file(TFile::Open(filenames["Th"].c_str(), "READ"));
      file->cd();
      auto spectra(get_TH1F_map(file));

      new TCanvas;
      gPad->SetLogy();
      for (auto const & name : names)
      {
        print(name, spectra.count(name));
        if (spectra.count(name) != 1) continue;
        auto const & spectrum = spectra.at(name);
        if (!spectrum) continue;
        spectrum->SetDirectory(nullptr);
        auto const & label = detectors.label(name);

        spectrum->Rebin(rebin);
        spectrum->GetXaxis()->SetRange(spectrum->FindBin(ADC_threshold), spectrum->FindLastBinAbove(20));

        gPad->SetTitle(concatenate("2614 ", name).c_str());
        spectrum->Draw();
        double x; double y;
        GetPoint(gPad, x, y);
        points_2614[name] = x;
        kpbs[name] = peaks[2]/x;
        calib.set(label, 0, kpbs[name]);
        calib.calibrateAxis(spectrum, label);
      }
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
    }

    else if (choice == 1)
    {
      calib.load("129_BGO.calib");
      auto file(TFile::Open(filenames["Na"].c_str(), "READ"));
      file->cd();
      auto spectra(get_TH1F_map(file));
      std::map<std::string, PeaksCalibrator*> calibs;
      int order_fit = 2;


      new TCanvas();
      for (auto const & name : names)
      {
        gPad->SetLogy();
        if (spectra.count(name) != 1) continue;
        auto const & spectrum = spectra.at(name);
        if (!spectrum) continue;
        spectrum->SetDirectory(nullptr);
        auto const & label = detectors.label(name);

        calib.calibrateAxis(spectrum, label);
        spectrum->Rebin(rebin);
        spectrum->GetXaxis()->SetRange(spectrum->FindBin(calib(ADC_threshold, label)), spectrum->FindLastBinAbove(100));

        double x; double y;
        std::vector<double> peaks_ADC(3, 0);

        spectrum->Draw();
        gPad->SetTitle(concatenate("1274 ", name).c_str());
        GetPoint(gPad, x, y);
        peaks_ADC[1] = x;

        gPad->SetTitle(concatenate("511 ", name).c_str());
        GetPoint(gPad, x, y);
        peaks_ADC[0] = x;


        // Fill the 2614 point :
        peaks_ADC[2] = peaks[2];
        
        calibs.emplace(name, new PeaksCalibrator(peaks, peaks_ADC, order_fit));
        auto & calibrator = calibs.at(name);

        auto const & prev_slope = calib.slope(label);

        print(calibrator->fit().parameter0, calibrator->fit().parameter1);

        calib.set(label, calibrator->fit().parameter0, calibrator->fit().parameter1);
        calib.Print(label);

        // gPad->Clear();
        // gPad->SetLogy(false);
        // calibs[name]->graph()->Draw();
        // gPad->Update();
        // gPad->WaitPrimitive();
        calib.calibrateAxis(spectrum, label);
        calib.slope(label)*=prev_slope;

        auto calib_label = calib[label];
        calib.Print(label);
      }

      file->Close();
      delete gPad;

      std::string const & filename = "test_calib.root";
      file = TFile::Open(filename.c_str(), "RECREATE");
      file->cd();
      for (auto & name : names)
      {
        auto & spectrum = spectra.at(name);
        spectrum->SetDirectory(file);
        spectrum->Write();
      }
      for (auto & name : names) calibs[name]->graph()->Write();
      file -> Write();
      file -> Close();
      print(filename);

      calib.write("129_BGO_bis.calib");
      for (auto & it : calibs) delete it.second;
    }
  }

  if (detector == "BR" || "FR")
  {
    int rebin = 5;
    if (choice == 0)
    {
      auto file(TFile::Open(filenames["Th"].c_str(), "READ"));
      std::vector<double> Th232 = {238.632, 338.32, 510.770, 583.191, 911.204, 2614.533};
      file->cd();
      auto spectra(get_TH1F_map(file));

      new TCanvas;
      gPad->SetLogy();

      
      for (auto const & name : names)
      {
        print(name, spectra.count(name));
        if (spectra.count(name) != 1) continue;
        auto spectrum = spectra.at(name);
        if (!spectrum) continue;
        spectrum->SetDirectory(nullptr);
        auto const & label = detectors.label(name);

        spectrum->Rebin(rebin);
        auto last_bin = spectrum->FindLastBinAbove(20);
        spectrum->GetXaxis()->SetRange(last_bin/2, last_bin);

        gPad->SetTitle(concatenate("2614 ", name).c_str());
        spectrum->Draw();
        gStyle->SetOptStat(0);
        gPad->Update();
        double x; double y;
        GetPoint(gPad, x, y);
        points_2614[name] = x;
        kpbs[name] = peaks[2]/x;
        calib.set(label, 0, kpbs[name]);
        calib.calibrateAxis(spectrum, label);
      }
      file->Close();

      std::string const & filename = "test.root";
      file = TFile::Open(filename.c_str(), "RECREATE");
      file->cd();
      for (auto & name : names)
      {
        print(name);
        if (spectra.count(name) != 1) continue;
        auto & spectrum = spectra.at(name);
        spectrum->SetDirectory(file);
        spectrum->Write();
      }
      file -> Write();
      file -> Close();
      print(filename);

      calib.write(concatenate("129_",detector,".calib"));
    }

    else if (choice == 1)
    {
      calib.load(concatenate("129_",detector,".calib"));
      auto file(TFile::Open(filenames["Na"].c_str(), "READ"));
      file->cd();
      auto spectra(get_TH1F_map(file));
      std::map<std::string, PeaksCalibrator*> calibs;
      int order_fit = 2;


      new TCanvas("myCanvas", "title", 0,0, 1920, 1080);
      for (auto const & name : names)
      {
        gPad->SetLogy();
        print(name, spectra.count(name));
        if (spectra.count(name) != 1) continue;
        auto spectrum = spectra.at(name);
        if (!spectrum || spectrum->Integral()<500) continue;
        spectrum->SetDirectory(nullptr);
        auto const & label = detectors.label(name);

        calib.calibrateAxis(spectrum, label);

        spectrum->Rebin(rebin);
        auto last_bin = spectrum->FindLastBinAbove(10);
        spectrum->GetXaxis()->SetRange(0, last_bin);

        double x; double y;
        std::vector<double> peaks_ADC(3, 0);

        // spectrum->Draw();
        // gPad->SetTitle(concatenate("1274 ", name).c_str());
        // GetPoint(gPad, x, y);
        // peaks_ADC[1] = x;

        // gPad->SetTitle(concatenate("511 ", name).c_str());
        // GetPoint(gPad, x, y);
        // peaks_ADC[0] = x;


        // Fill the 2614 point :
        // peaks_ADC[2] = peaks[2];
        
        // calibs.emplace(name, new PeaksCalibrator(peaks, peaks_ADC, order_fit));
        // auto & calibrator = calibs.at(name);

        // auto const & prev_slope = calib.slope(label);

        // print(calibrator->fit().parameter0, calibrator->fit().parameter1);

        // calib.set(label, calibrator->fit().parameter0, calibrator->fit().parameter1);
        // calib.Print(label);

        // gPad->Clear();
        // gPad->SetLogy(false);
        // calibs[name]->graph()->Draw();
        // gPad->Update();
        // gPad->WaitPrimitive();
        // calib.calibrateAxis(spectrum, label);
        // calib.slope(label)*=prev_slope;

        // auto calib_label = calib[label];
        // calib.Print(label);
      }

      file->Close();
      delete gPad;

      std::string const & filename = "test_calib.root";
      auto outfile (TFile::Open(filename.c_str(), "RECREATE"));
      outfile->cd();
      for (auto & name : names)
      {
        print(name);
        if (spectra.count(name) != 1) continue;
        auto & spectrum = spectra.at(name);
        if (spectrum && spectrum->Integral()>2) 
        {
          spectrum->SetDirectory(outfile);
          spectrum->Write();
        }
      }
      // for (auto & name : names) 
      // {
      //   if (spectra.count(name) != 1) continue;
      //   if (calibs[name]->graph()) calibs[name]->graph()->Write();
      // }
      outfile -> Write();
      outfile -> Close();
      print(filename);

      calib.write(concatenate("129_",detector,"_bis.calib"));
      for (auto & it : calibs) delete it.second;
    }
  }

}