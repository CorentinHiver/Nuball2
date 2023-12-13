#include "CaLib.hpp"

void analyseSpectra(std::string const & filename = "PARIS/152_Eu_center_after.root", std::string const & source = "152Eu")
{
  detectors.load("index_129.list");
  // if (!detectors) detectors.load("index_129.list");
  auto file = TFile::Open(filename.c_str(), "READ");
  std::unordered_map<std::string, TH1F*> histos;
  auto list = file->GetListOfKeys();
  std::vector<std::string> names;

  std::string out = removeExtension(filename)+".calpoints";
  std::ofstream outFile(out, std::ios::out);

  size_t nb_histos = 0;
  print("Reading", filename);
  for (auto&& keyAsObj : *list)
  {
    std::unique_ptr<TKey> key (static_cast<TKey*>(keyAsObj));
    std::string className =  key->GetClassName();
    if(className == "TH1F")
    {
      auto histo = static_cast<TH1F*>(key->ReadObj());
      std::string name = histo->GetName();
      remove(name, "_cal");
      remove(name, "_raw");
      histos.emplace(name, histo);
      names.emplace_back(name);
    }
    nb_histos++;
  }
  
  std::vector<double> peaks;
  if (source == "152Eu") peaks= {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
  outFile << "name";
  for (auto const & peak : peaks) outFile << ";" << peak;
  outFile << std::endl;

  auto c1 = new TCanvas("canvas","canvas");
  c1->cd();
  c1->SetLogy();
  c1->Update();
  c1->SetCrosshair(1);
  c1->ToggleEventStatus();

  for (auto & name : names)
  {
    print();
    print(name);
    AnalysedSpectra histo(histos[name]);
    // auto const & label = detectors.label(name);

    histo.cd(c1);
    histo.fitPeaks(peaks);

    outFile << name << ";";
    for (auto const & peak : histo.getPeaks())
    {
      if (found(peaks, peak.energy())) outFile << peak.mean() << ";";
      else outFile << "  ";
    }
    outFile << std::endl;
  }
}

Time_ns timewindow_ns = 100; // in ns
Vector_MTTHist<TH1F> raw_spectra;

void fillRawSpectra(Hit & hit, Alignator & tree)
// void fillRawSpectra(Hit & hit, Alignator & tree, Vector_MTTHist<TH1F> raw_spectra)
{
  Event event;
  CoincBuilder eventBuilder(&event);
  eventBuilder.setTimeWindow_ns(timewindow_ns);
  auto nb_hits = tree->GetEntries();

  for (int loop = 0; loop<nb_hits; loop++)
  {
    tree.GetEntry(loop);
    if (eventBuilder.build(hit))
    {
      if (event.mult > 1 && event.mult < 4)
      for (int c = 0; c<event.mult; c++)
      {
        auto const & label = event.labels[c];
        if (!isParis[label]) continue;
        auto const & index = detectors.index(label);
        auto const & adc = event.adcs[c];
        raw_spectra[index].Fill(adc);
      }
    }
  }
}

void ParisCalib
(
  std::string const & folder = "/home/corentin/faster_data/N-SI-136/152_Eu_center_after.fast/",
  int const & nb_files = -1,
  int const & nb_threads = 1,
  std::string const & source = "152Eu",
  std::string const & type = "paris"
)
{
  // Writting down the data : 
  auto const & rootFileName = removeExtension(Path(folder).folder().name())+".root";

  detectors.load("index_129.list");

  MTObject::Initialize(nb_threads);
  raw_spectra.resize(detectors.nbOfType(type));
  auto const & binning = detectors.ADCBin(type);
  for (size_t index = 0; index<detectors.nbOfType(type); index++)
  {
    auto const & name = detectors.name(type, index);
    raw_spectra[index].reset(name.c_str(), (name+" raw spectra").c_str(), binning.bins, binning.min, binning.max);
  }

  MTFasterReader reader(folder, nb_files);
  Timeshifts ts("../136.dT");
  reader.setTimeshifts(ts.get());
  reader.readAligned(fillRawSpectra);

  auto file = TFile::Open(rootFileName.c_str(), "recreate");
  file->cd();
  for (auto & histo : raw_spectra)
  {
    // histo.Merge();
    // std::string name = histo.name();
    histo.Write();
  }
  file->Write();
  file->Close();

  print(rootFileName, "written");

  analyseSpectra(rootFileName);
}