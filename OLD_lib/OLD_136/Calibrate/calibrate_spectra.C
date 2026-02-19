#include "../../lib/Classes/Calibration.hpp"
#include "../../lib/Analyse/SpectraCo.hpp"

void calibrate_spectra
(
  std::string const & histo_file = "~/faster_data/N-SI-136-U_histo/total/fused_histo.root", 
  std::string const & calibration_file = "../conversion_200ns/136_2024.calib",
  std::string const & out_filename = "test.root"
)

{
  // // for (Label label = 0; label<detectors.size(); label++) print(label, detectors.exists(label));
  // Calibrator calib_test;
  // calib_test.loadCalibration(calibration_file);
  // calib_test.verbose(true);
  // calib_test.loadRootHisto(histo_file);
  // calib_test.verify(out_filename);

  // print("Now verification time : you have to look at the calibrated spectra in test_calib.root "
  // "in order to correct for any huge error. To do so, copy 136.calib to 136_checked.calib and make"
  // "the appropriate changes there. Now run recal.C to adjust the coefficients using some run peaks.");
  
  detectors.load("index_129.list");
  Calibration calib(calibration_file);

  auto file(TFile::Open(histo_file.c_str(), "READ"));
  file->cd();
  std::vector<std::string> names;
  auto spectra(get_TH1F_map(file, names));

  std::map<std::string, TH1F*> calib_spectra;
  std::map<dType, TH2F*> each_detector;
  each_detector["ge"]   = new TH2F("each_ge"   , "Each Germanium Spectra;index;Energy[keV]", 96,0,96, 10000,0,10000);
  each_detector["bgo"]  = new TH2F("each_BGO"  , "Each BGO Spectra;index;Energy[keV]"      , 24,0,24, 10000,0,10000);
  each_detector["paris"]= new TH2F("each_paris", "Each Paris Spectra;index;Energy[keV]"    , 70,0,70, 10000,0,10000);
  each_detector["dssd"] = new TH2F("each_dssd" , "Each DSSD Spectra;index;Energy[keV]"     , 70,0,70, 10000,0,10000);
  
  std::map<dType, TH1F*> all_detector;
  all_detector["ge"]   = new TH1F("all_ge"   , "All Germanium Spectra;index;Energy[keV]", 10000,0,10000);
  all_detector["bgo"]  = new TH1F("all_BGO"  , "All BGO Spectra;index;Energy[keV]"      , 10000,0,10000);
  all_detector["paris"]= new TH1F("all_paris", "All Paris Spectra;index;Energy[keV]"    , 10000,0,10000);
  all_detector["dssd"] = new TH1F("all_dssd" , "All DSSD Spectra;index;Energy[keV]"     , 10000,0,10000);

  std::vector<dType> used_types = {"ge" ,"bgo" ,"paris" ,"dssd"};
  std::map<dType, int> rebin = {{"ge", 4}, {"bgo", 100}, {"paris", 20}, {"dssd", 10}};
  std::map<dType, int> smooths = {{"ge", 20}, {"bgo", 10}, {"paris", 10}, {"dssd", 10}};

  for (auto const & name : names)
  {
    auto spectrum = spectra[name];
    auto const & label = detectors[name];
    auto const & type = detectors.type(label);
    if (!found(used_types, type)) continue;
    print(name);
    calib.calibrateAxis(spectrum, label);
    spectrum->SetDirectory(nullptr);
    spectrum->Rebin(rebin[type]);
    if (type!="paris") CoAnalyse::removeBackground(spectrum, smooths[type]);

    if(type == "ge") {spectrum->GetXaxis()->SetRange(0, spectrum->FindLastBinAbove(1)*0.90);}
    else             {spectrum->GetXaxis()->SetRange(0, spectrum->FindLastBinAbove(1)     );}

    calib_spectra.emplace(name, spectrum);

    AddTH1(all_detector[type], spectrum);

    print(name, label, compressedLabel[label]);

    AddTH1ByValue(each_detector[type], histo, compressedLabel[label]);
  }

  file->Close();

  print("Writting detectors");
  auto outfile (TFile::Open(out_filename.c_str(), "RECREATE"));
  outfile->cd();
  // for (auto & it : all_detector) it.second->Write();
  for (auto & name : names)
  {
    if (!found(used_types, detectors.type(detectors[name]))) continue;
    auto & spectrum = calib_spectra.at(name);
    spectrum->SetDirectory(outfile);
    spectrum->Write();
  }
  outfile -> Write();
  outfile -> Close();
  print(out_filename, "written");
}