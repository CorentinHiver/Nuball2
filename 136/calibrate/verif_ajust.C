#include "../../lib/Analyse/AnalysedSpectra.hpp"
#include "../../lib/Analyse/SpectraCo.hpp"

#define NSI129

/**
 * @brief third step : apply the ajusted calibration to the initial spectram.
 * This creates a final root file to check the final calibration.
 * 
 */
void verif_ajust()
{
  detectors.load("index_129.list");
#ifdef NSI129
  Calibration calib("129_ajusted.calib");
#else
  Calibration calib("136_ajusted.calib");
#endif //NSI129
  std::string filename = "~/faster_data/N-SI-136-U_histo/total/fused_histo.root";
  auto file = TFile::Open(filename.c_str());
  auto spectra_map = get_TH1F_map(file);
  for (auto & it : spectra_map)it.second->SetDirectory(nullptr);
  file->Close();
  gROOT->cd();
  std::map<Label, TH1F*> spectra;

  for (auto & it : spectra_map)
  {
    auto & name  = it.first;
    auto const & label = detectors.label(name);

    SpectraCo spectrum(it.second);
    spectrum.rebin(20);
    spectrum.removeBackground(10);
    spectrum.calibrate(calib, label);
    spectrum.resizeBin(spectrum.getBin(20000));
    spectra.emplace(label, spectrum.createTH1F(name+"_recalibrated"));
  }

#ifdef NSI129
  std::string outname = "129_ajusted.root";
#else
  std::string outname = "136_ajusted.root";
#endif //NSI129

  auto outFile = TFile::Open(outname.c_str(), "recreate");
  outFile->cd();
  for (auto it :  spectra) 
  {
    auto & spectrum = it.second;
    if (spectrum && !spectrum->IsZombie() && spectrum->Integral()>1) spectrum->Write();
  }
  outFile->Write();
  outFile->Close();
  print(outname, "written");
}