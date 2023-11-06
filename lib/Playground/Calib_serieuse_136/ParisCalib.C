#include "../../libRoot.hpp"
#include "../../Classes/Detectors.hpp"
#include "../../Modules/Timeshifts.hpp"
#include "../../Classes/FilesManager.hpp"
#include "../../Classes/CoincBuilder.hpp"
#include "../../MTObjects/MTFasterReader.hpp"
#include "../../MTObjects/MTTHist.hpp"

void GetPoint(TVirtualPad * vpad, double& x, double& y)
{
  auto pad = dynamic_cast<TPad*>(vpad);
  pad->Update();
	auto cutg = static_cast<TMarker*> (pad->WaitPrimitive("TMarker","Marker"));
  if (!cutg) return;
	x = cutg->GetX();
	y = cutg->GetY();
	delete cutg;
  print(x, y);
}

class ParisPeakFitter
{
public:
  ParisPeakFitter(TH1F* histo, double low_edge, double high_edge)
  {
    auto const & mean0 = (high_edge+low_edge)/2;
    auto const & constante0 = histo->GetBinContent(mean0);
    auto const & sigma0 = (high_edge-low_edge)/5.;

    TF1* gaus0(new TF1("gaus0","gaus"));
    gaus0 -> SetRange(low_edge, high_edge);
    gaus0 -> SetParameter(0, constante0);
    gaus0 -> SetParameter(1, mean0);
    gaus0 -> SetParameter(2, sigma0);
    histo -> Fit(gaus0,"RQN+");

    TF1* gaus1(new TF1("gaus1","gaus(0)+pol1(3)"));
    gaus1 -> SetRange(low_edge, high_edge);
    gaus1 -> SetParameter(0, gaus0->GetParameter(0));
    gaus1 -> SetParameter(1, gaus0->GetParameter(1));
    gaus1 -> SetParameter(2, gaus0->GetParameter(2));
    histo -> Fit(gaus1,"RQN+");

    TF1* gaus2(new TF1("gaus2","gaus(0)+pol2(3)"));
    gaus2 -> SetRange(low_edge, high_edge);
    gaus2 -> SetParameter(0, gaus1->GetParameter(0));
    gaus2 -> SetParameter(1, gaus1->GetParameter(1));
    gaus2 -> SetParameter(2, gaus1->GetParameter(2));
    histo -> Fit(gaus2,"RQN+");

    gaus2->Draw("same");
    final_fit = gaus2;
  }

  auto getConstante() const {return final_fit->GetParameter(0);}
  auto getMean() const {return final_fit->GetParameter(1);}
  auto getSigma() const {return final_fit->GetParameter(2);}

private:
  TF1* final_fit = nullptr;
};

class FittedPeak
{
public:
  FittedPeak(double const & constante, double const & mean, double const & sigma) :
    m_constante (constante),
    m_mean      (mean),
    m_sigma     (sigma)
  {
  }

  FittedPeak(double const & energy, double const & constante, double const & mean, double const & sigma) :
    m_energy    (energy),
    m_constante (constante),
    m_mean      (mean),
    m_sigma     (sigma)
  {
  }

  FittedPeak(ParisPeakFitter const & fit) :
    m_constante (fit.getConstante()),
    m_mean      (fit.getMean()),
    m_sigma     (fit.getSigma())
  {
  }

  FittedPeak(double const & energy, ParisPeakFitter const & fit) :
    m_energy    (energy),
    m_constante (fit.getConstante()),
    m_mean      (fit.getMean()),
    m_sigma     (fit.getSigma())
  {
  }

  auto const & energy() const {return m_energy;}
  auto const & mean() const {return m_mean;}
  auto const & sigma() const {return m_sigma;}
  auto const & constante() const {return m_constante;}

private:
  double m_energy = -1;
  double m_constante = -1;
  double m_mean = -1;
  double m_sigma = -1;

};

class AnalysedSpectra
{
public:
  AnalysedSpectra(TH1F* histo) : m_histo(histo){}

  auto operator->() {return m_histo;}

  void setKevPerBin(double const & kpb) {m_kpb = kpb;}
  void setChannelsPerBin(double const & cpb) {m_cpb = cpb;}
  void cd(TVirtualPad * vpad) {m_pad = static_cast<TPad*> (vpad); m_pad->cd();}
  void autoCanvas()
  {
    print("Pad created automatically for peak finding");
    m_pad = new TCanvas("canvas","canvas");
    m_pad -> cd();
    m_pad -> SetLogy();
    m_pad -> Update();
    m_pad -> SetCrosshair(1);
  }
  auto getCanvas() {return m_pad;}
  auto const getCanvas() const {return m_pad;}

  void fitPeaks(std::vector<double> peaks)
  {
    if (m_pad == nullptr) autoCanvas();
    m_histo->Draw();
    double x = 0; double y = 0;
    int xmax = 0;

    // double power = -0.8;
    double sigma_peakMax = 0;
    double mean_peakMax = 0;

    // Fit the maximum peak :
    {
      auto const & peak = peaks.back();
      m_pad->SetTitle(("INSTRUCTION : Select the low edge of maximum peak"+ std::to_string(peak)).c_str());
      m_pad->Update();
      GetPoint(m_pad->cd(), x, y);
      m_pad->Update();

      auto low_edge = x;

      m_pad->SetTitle(("INSTRUCTION : Select the high edge of maximum peak"+ std::to_string(peak)).c_str());
      m_pad->Update();
      GetPoint(m_pad->cd(), x, y);
      m_pad->Update();

      auto high_edge = x;
      xmax = high_edge;

      m_histo->GetXaxis()->SetRangeUser(0, xmax);

      ParisPeakFitter fit(m_histo, low_edge, high_edge);

      mean_peakMax  = fit.getMean ();
      sigma_peakMax = fit.getSigma();
    }

    m_kpb = peaks.back()/mean_peakMax; // keV per bin
    m_cpb = m_histo->GetXaxis()->GetBinLowEdge(mean_peakMax)/xmax; // canal per bin
    print("keV per bin :", m_kpb, "canal per bin : ", m_cpb);

    auto const & FWHMr = 2.35*sigma_peakMax/mean_peakMax;// relative FWHM 
    auto const & FWHMp = 100*FWHMr;                      // relative FWHM in percentage
    // auto const & C = sigma_peakMax*pow(mean_peakMax, power);
    print("sigma:", sigma_peakMax*m_kpb, "keV at", peaks.back(), "kev -> FWHM=", FWHMp, "%");

    for(size_t peak_i = 0; peak_i<peaks.size(); peak_i++)
    {
      auto const & peak = peaks[peak_i];

      m_pad->SetTitle(("Select peak "+std::to_string((int)peak)).c_str());
      m_pad->Update();

      GetPoint(m_pad->cd(), x, y);
      auto low_edge = x;
      GetPoint(m_pad->cd(), x, y);
      auto high_edge = x;

      if (x*m_kpb<5) continue; // 5 keV threshold

      ParisPeakFitter fit(m_histo, low_edge, high_edge);

      m_peaks.emplace_back(
        peak,
        fit.getConstante(),
        fit.getMean(),
        fit.getSigma());
    }
    
    std::cout << "E   ";
    for (auto const & peak : m_peaks) std::cout << peak.energy()  << " ";
    std::cout << std::endl;

    std::cout << "ADC ";
    for (auto const & peak : m_peaks) std::cout << peak.mean() << " ";
    std::cout << std::endl;
  }

  auto const & getPeaks() const {return m_peaks;}

private:
  TPad* m_pad = nullptr;
  TH1F* m_histo = nullptr;
  double m_kpb = 1; // keV per bin
  double m_cpb = 1; // canal per bin

  // Fitting : 
  std::vector<FittedPeak> m_peaks;
};

std::ostream& operator<<(std::ostream& out, AnalysedSpectra const & as)
{
  out << "E   ";
  for (auto const & peak : as.getPeaks()) out << peak.energy()  << " ";
  out << std::endl;

  out << "ADC ";
  for (auto const & peak : as.getPeaks()) out << peak.mean() << " ";
  out << std::endl;

  return out;
}

void analyseSpectra(std::string const & filename = "152_Eu_center_after.root", std::string const & source = "152Eu")
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