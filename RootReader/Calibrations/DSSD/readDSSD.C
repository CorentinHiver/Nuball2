#include "../../../lib/libRoot.hpp"
#include "../../../lib/Classes/Detectors.hpp"
#include "../../../lib/Classes/SpectraAlignator.hpp"
#include "TPad.h"

  std::vector<std::string> names = 
  {
    "S1_0_DSSD",
    "S1_1_DSSD",
    "S1_2_DSSD",
    "S1_3_DSSD",
    "S1_4_DSSD",
    "S1_5_DSSD",
    "S1_6_DSSD",
    "S1_7_DSSD",
    "S1_8_DSSD",
    "S1_9_DSSD",
    "S1_10_DSSD",
    "S1_11_DSSD",
    "S1_12_DSSD",
    "S1_13_DSSD",
    "S1_14_DSSD",
    "S1_15_DSSD",
    "S2_1_DSSD",
    "S2_2_DSSD",
    "S2_5_DSSD",
    "S2_6_DSSD",
    "S2_8_DSSD",
    "S2_9_DSSD",
    "S2_10_DSSD",
    "S2_11_DSSD",
    "S2_12_DSSD",
    "S2_13_DSSD",
    "S2_14_DSSD",
    "S2_15_DSSD"
  };

/**
 * @brief Returns the position of the peak. Carefull, you need to zoom in the zone of interest first.
 */
double calib(TH1* histo, bool const & verbose = false, bool const & draw = false)
{
  std::string name (histo->GetName());

  // Inititalise the parameters :
  auto const & constante = histo->GetMaximum();
  auto const & mean = histo->GetXaxis()->GetBinCenter(histo->GetMaximumBin());
  auto const & sigma = (histo->GetXaxis()->GetBinCenter(histo -> FindLastBinAbove(constante/2))
                        - histo->GetXaxis()->GetBinCenter(histo -> FindFirstBinAbove(constante/2)))/2.35;

  // Simple gaussian fit :
  TF1*  gaus(new TF1("gaus","gaus"));     
  gaus -> SetRange(mean-10*sigma, mean+10*sigma);
  gaus -> SetParameter(0, constante);
  gaus -> SetParameter(1, mean);
  gaus -> SetParameter(2, sigma);
  histo -> Fit(gaus,"RQN+");

  auto const & constante_1 = gaus -> GetParameter(0);
  auto const & mean_1 = gaus -> GetParameter(1);
  auto const & sigma_1 = gaus -> GetParameter(2);

  // Gaussian + background :
  TF1* gaus_pol0(new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)"));
  gaus_pol0 -> SetRange(mean_1-10*sigma_1, mean_1+10*sigma_1);
  gaus_pol0 -> SetParameter(0, gaus -> GetParameter(0));
  gaus_pol0 -> SetParameter(1, gaus -> GetParameter(1));
  gaus_pol0 -> SetParameter(2, gaus -> GetParameter(2));
  histo -> Fit(gaus_pol0,"RQ+");

  if (draw)
  {
    auto c = new TCanvas(("canvas_"+name).c_str(), name.c_str());
    c->cd();
    histo->Draw();
    c->Update();
    c->WaitPrimitive();
  }

  if (verbose)
  {
    print(constante, mean, sigma);
    print(constante_1, mean_1, sigma_1);
    print(gaus_pol0 -> GetParameter(0), gaus_pol0 -> GetParameter(1), gaus_pol0 -> GetParameter(2));
  }

  return gaus -> GetParameter(1);
}

void readDSSD(char choix = 0)
{

  if (choix == 0)
  {// Read the initial DSSD_bidims.root and removes the background
    std::vector<TH2F*> matrixes;
    auto file = TFile::Open("DSSD_bidims.root", "READ");
    for (auto const & name : names)
    {
      matrixes.emplace_back(file->Get<TH2F>(name.c_str()));
      auto & matrix = matrixes.back();
      CoAnalyse::removeRandomBidim(matrix, 20);
    }

    auto out = TFile::Open("DSSD_final.root", "recreate");
    out->cd();
    for (auto & matrix : matrixes) if (matrix) matrix->Write();
    out->Write();
    out->Close();  
    print("DSSD_final.root written");
  }

  else if (choix == 1 || choix == 2)
  {// Reads the DSSD_final.root
    detectors.load("../index_129.list");
    ofstream outFile("DSSD_calib_169.data");
    auto file = TFile::Open("DSSD_final.root", "READ");
    std::vector<TH2F*> matrixes;
    std::vector<TH1D*> proj169;
    for (auto const & name : names)
    {
      auto const & label = detectors.getLabel(name);
      matrixes.emplace_back(file->Get<TH2F>(name.c_str()));
      if (!matrixes.back()) continue;
      matrixes.back()->RebinY(4);
      proj169.emplace_back(new TH1D());
      auto & histo = proj169.back();
      CoAnalyse::projectY(matrixes.back(), histo, 167., 171.);

      if (histo->Integral()<200)
      {
        print("CAN'T CALIBRATE", name, ": only", histo->Integral(), "counts");
        outFile << label << " " << 5700 << " " << "N/A" << std::endl;
        continue;
      }

      ////////////////////
      // Fit the peak : //
      ////////////////////

      auto mean = (choix == 2) ? calib(histo, true, true) : calib(histo);

      // Write down the center value :
      outFile << label << " " << 5700 << " " << mean << std::endl;
    }
    outFile.close();
  }

  else if (choix == 3 || choix == 4)
  {// Reads the DSSD_final.root
    detectors.load("../index_129.list");
    ofstream outFile("DSSD_calib_546.data");
    auto file = TFile::Open("DSSD_final.root", "READ");
    std::vector<TH2F*> matrixes;
    std::vector<TH1D*> proj545;
    for (auto const & name : names)
    {
      auto const & label = detectors.getLabel(name);
      matrixes.emplace_back(file->Get<TH2F>(name.c_str()));
      matrixes.back()->RebinY(4);
      if (!matrixes.back()) continue;
      proj545.emplace_back(new TH1D());
      auto & histo = proj545.back();
      CoAnalyse::projectY(matrixes.back(), histo, 544., 547.);

      if (histo->Integral()<100)
      {
        print("CAN'T CALIBRATE", name, ": only", histo->Integral(), "counts");
        outFile << label << " " << 9000 << " " << "N/A" << std::endl;
        continue;
      }

      ////////////////////
      // Fit the peak : //
      ////////////////////

      auto mean = (choix == 4) ? calib(histo, true, true) : calib(histo);

      // Write down the center value :
      outFile << label << " " << 9000 << " " << mean << std::endl;
    }
    outFile.close();
  }

  else if (choix == 5 || choix == 6)
  {// Reads the DSSD_final.root
    detectors.load("../index_129.list");
    ofstream outFile("DSSD_calib_3000.data");
    auto file = TFile::Open("DSSD_final.root", "READ");
    std::vector<TH2F*> matrixes;
    std::vector<TH1D*> proj3000;
    for (auto const & name : names)
    {
      auto const & label = detectors.getLabel(name);
      matrixes.emplace_back(file->Get<TH2F>(name.c_str()));
      matrixes.back()->RebinY(6);
      if (!matrixes.back()) continue;
      proj3000.emplace_back(new TH1D());
      auto & histo = proj3000.back();
      CoAnalyse::projectY(matrixes.back(), histo, 2980., 3020.);

      if (histo->Integral()<50)
      {
        print("CAN'T CALIBRATE", name, ": only", histo->Integral(), "counts");
        outFile << label << " " << 5250 << " " << "N/A" << std::endl;
        continue;
      }

      ////////////////////
      // Fit the peak : //
      ////////////////////

      auto mean = (choix == 6) ? calib(histo, true, true) : calib(histo);

      // Write down the center value :
      outFile << label << " " << 5250 << " " << mean << std::endl;
    }
    outFile.close();
  }

  /**
   * @brief Choice 7 : load the run_75 raw adc spectra straight out of manip2histo for energy calibration
   */
  else if (choix == 7)
  {
    detectors.load("../index_129.list");
    ofstream outFile("DSSD_calib_choice7.data");

    auto c1 = new TCanvas("c1");
    auto file = TFile::Open("run_75_spectra.root", "READ");
    auto list = file->GetListOfKeys();
    size_t nb_histos = 0;
    for (auto&& keyAsObj : *list)
    {
      std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
      std::string className =  key->GetClassName();
      if(className == "TH1F")
      {
        std::unique_ptr<TObject> obj (key->ReadObj());
        auto histo = dynamic_cast<TH1*>(obj.get());
        std::string name = histo->GetName();
        remove(name, " adc");
        auto const & label = detectors[name];

        histo->Rebin(8);

        nb_histos++;
      }
    }
  }
}