#include "../../../lib/libRoot.hpp"
#include "../../../lib/Classes/FilesManager.hpp"
#include "../../../lib/Classes/Calibration.hpp"
#include "../../../lib/Analyse/AnalysedSpectra.hpp"


void read_evolution(Path folder = "")
{
  Calibration calib()
  print("start");
  if (folder == Path::pwd()) folder = Path("~/faster_data/N-SI-136-U_histo");
  if (!folder) throw_error(concatenate("Folder ", folder, " do not exist"));
  FilesManager files(folder);
  // auto c = new TCanvas("coucou");
  int i = 0; 
  for (auto const & filename : files)
  {
    print(filename);
    auto file = TFile::Open(filename.c_str());
    auto list_names = getTH1FNames(filename);
    for (auto const & name : list_names) if (found(name, "PARIS"))
    {
      print(name);
      auto spectra = file->Get<TH1F>(name.c_str());
      if (!spectra) continue;
      spectra->Rebin(10);
      AnalysedSpectra histo(spectra);
      histo.fitPeaks({511, 1778});
      auto peaks = histo.getPeaks();
    }
    i++;
  }
}