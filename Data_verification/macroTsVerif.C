#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
void macroTsVerif()
{
  FilesManager files(Path::home()+"nuball2/N-SI-136-root_all/Timeshifts/");
  TH2F* dT_per_run = new TH2F("dT_per_run", "dT_per_run", 100,50,150, 1000,-750000,750000);
  TH2F* dT_per_run_RF = new TH2F("dT_per_run_RF", "dT_per_run_RF", 100,50,150, 800,-100000,300000);
  for (auto const & file : files)
  {
    auto run_number = std::stoi(split(rmPathAndExt(file), '_')[1]);
    auto tFile(TFile::Open(file.c_str(), "READ"));

    auto dT_per_run_file = tFile->Get<TH2F>("All corrected time spectra");
    auto dT_per_run_RF_file = tFile->Get<TH2F>("All_corrected_time_spectra_RF");

    auto dT_per_run_proj = dT_per_run_file->ProjectionY();
    auto dT_per_run_RF_proj = dT_per_run_file->ProjectionY();

    for (int bin = 0; bin<1000; bin++) dT_per_run->SetBinContent(bin, run_number, dT_per_run_proj->GetBinContent(bin));
    for (int bin = 0; bin<800; bin++) dT_per_run_RF->SetBinContent(bin, run_number, dT_per_run_RF_proj->GetBinContent(bin));
  }
  new TCanvas;
  dT_per_run->Draw("colz");
  new TCanvas;
  dT_per_run_RF->Draw("colz");
}