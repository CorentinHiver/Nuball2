#ifndef ANALYSEDSPECTRA_HPP
#define ANALYSEDSPECTRA_HPP

#include "../libRoot.hpp"
#include "../Classes/Fit.hpp"

class PeakFitter
{
public:
  PeakFitter(TH1F* histo, double low_edge, double high_edge)
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

class BGOPeakFitter
{

};

class GePeakFitter
{
  
};

class FittedPeak
{
public:
  FittedPeak(double const & energy, double const & constante, double const & mean, double const & sigma = -1) :
    m_energy    (energy),
    m_constante (constante),
    m_mean      (mean),
    m_sigma     (sigma)
  {
  }

  FittedPeak(PeakFitter const & fit) :
    m_constante (fit.getConstante()),
    m_mean      (fit.getMean()),
    m_sigma     (fit.getSigma())
  {
  }

  FittedPeak(double const & energy, PeakFitter const & fit) :
    m_energy    (energy),
    m_constante (fit.getConstante()),
    m_mean      (fit.getMean()),
    m_sigma     (fit.getSigma())
  {
  }

  auto const & energy   () const {return m_energy   ;}
  auto const & mean     () const {return m_mean     ;}
  auto const & sigma    () const {return m_sigma    ;}
  auto const & constante() const {return m_constante;}

private:
  double m_energy     = -1;
  double m_constante  = -1;
  double m_mean       = -1;
  double m_sigma      = -1;

};

/**
 * @brief Allows one to select which peaks to fit on a spectra
 * 
 */
class AnalysedSpectra
{
public:
  AnalysedSpectra() noexcept = default;
  AnalysedSpectra(TH1F* histo) noexcept : m_histo(histo) {}
  void load(TH1F* const histo) 
  {
    m_histo = histo;
    range_min = histo->GetXaxis() -> GetXmin();
    range_max = histo->GetXaxis() -> GetXmax();
  }

  auto operator->() {return m_histo;}

  void setKevPerBin(double const & kpb) {m_kpb = kpb;}
  void setChannelsPerBin(double const & cpb) {m_cpb = cpb;}
  void cd(TVirtualPad * vpad) {m_pad = static_cast<TPad*> (vpad); m_pad->cd();}
  void autoCanvas()
  {
    print("Pad created automatically for peak finding");
    m_pad = new TCanvas("canvas","canvas");
    m_pad -> cd();
    m_pad -> Update();
    m_pad -> SetCrosshair(1);
  }
  auto getCanvas() {return m_pad;}
  auto const getCanvas() const {return m_pad;}

  void choosePeaks(std::vector<double> const & peaks);

  double selectX(std::string const & instructions)
  {
    double x = 0; double y = 0;
    m_pad->SetTitle(instructions.c_str());
    m_histo->SetTitle(instructions.c_str());
    GetPoint(m_pad->cd(), x, y);
    m_pad->Update();
    return x;
  }

  void setMinimumRange(double const & _min) 
  {
    m_histo->GetXaxis()->SetRangeUser((range_min = _min), range_max);
  }

  void setMaximumRange(double const & _max) 
  {
    m_histo->GetXaxis()->SetRangeUser(range_min, (range_max = _max));
  }

  std::vector<FittedPeak> & fitPeaks(std::vector<double> peaks, bool const & adjustRange = false);

  auto const & getPeaks() const {return m_peaks;}
  void clearPeaks() {m_peaks.clear();}

private:
  TPad* m_pad = nullptr;
  TH1F* m_histo = nullptr;
  double m_kpb = 1; // keV per bin
  double m_cpb = 1; // canal per bin
  std::string detector = "PARIS";

  double range_min = 0;
  double range_max = 0;

  // Fitting : 
  std::vector<FittedPeak> m_peaks;
};


std::vector<FittedPeak> & AnalysedSpectra::fitPeaks(std::vector<double> peaks, bool const & adjustRange = false)
{
  if (m_pad == nullptr) autoCanvas();
  m_histo->Draw();
  double x = 0; double y = 0;
  int xmax = 0;// Position of the maximum peak in bins

  double sigma_peakMax = 0;
  double mean_peakMax = 0;

  // Fit the maximum peak :
  {
    auto const & peak = peaks.back();
    m_pad -> SetLogy(true);
    
    setMaximumRange(selectX(concatenate("VIEW RANGE : Select ABOVE peak ", (int)peak)));
    setMinimumRange(selectX(concatenate("VIEW RANGE : Select BELOW peak ", (int)peak)));
    if (adjustRange)
    {
      setMinimumRange(selectX(concatenate("VIEW RANGE : Select BELOW peak ", (int)peak)));
      setMaximumRange(selectX(concatenate("VIEW RANGE : Select ABOVE peak ", (int)peak)));
    }

    m_pad -> SetLogy(false);
    
    auto beginPeak = selectX(concatenate("FIT : Select the BEGINNING of peak ",(int)peak));
    auto endPeak   = selectX(concatenate("FIT : Select the END peak "         ,(int)peak));

    PeakFitter fit(m_histo, beginPeak, endPeak);

    xmax = (beginPeak+endPeak)/2;

    mean_peakMax  = fit.getMean ();
    sigma_peakMax = fit.getSigma();
  }

  m_kpb = peaks.back()/mean_peakMax; // keV per bin
  m_cpb = m_histo->GetXaxis()->GetBinLowEdge(mean_peakMax)/xmax; // canal per bin
  print("keV per bin :", m_kpb, "canal per bin : ", m_cpb);

  auto const & FWHMr = 2.35*sigma_peakMax/mean_peakMax;// relative FWHM 
  auto const & FWHMp = 100*FWHMr;                      // relative FWHM in percentage
  print("sigma:", sigma_peakMax*m_kpb, "keV at", peaks.back(), "kev -> FWHM=", FWHMp, "%");

  for(size_t peak_i = 0; peak_i<peaks.size(); peak_i++)
  {
    auto const & peak = peaks[peak_i];
    auto const & peak_min_view_range = peak*0.75;
    auto const & peak_max_view_range = peak*1.25;
    setMinimumRange(peak_min_view_range);
    setMaximumRange(peak_max_view_range);

    if (adjustRange)
    {
      setMinimumRange(selectX(concatenate("VIEW RANGE : Select BELOW peak ", (int)peak)));
      setMaximumRange(selectX(concatenate("VIEW RANGE : Select ABOVE peak ", (int)peak)));
    }

    auto low_edge = selectX("FIT : Select the BEGINNING of peak "+ std::to_string((int)peak));
    auto high_edge = selectX("FIT : Select the END peak "+ std::to_string((int)peak));

    PeakFitter fit(m_histo, low_edge, high_edge);

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

  return m_peaks;
}

void AnalysedSpectra::choosePeaks(std::vector<double> const & peaks)
{
  if (m_pad == nullptr) autoCanvas();
  m_histo->Draw();
  double x = 0; double y = 0;
  int xmax = 0;

  double sigma_peakMax = 0;
  double mean_peakMax = 0;

  // Setup the window :
  m_pad->SetTitle("INSTRUCTION : Select the maximum of energy window");
  m_histo->SetTitle("INSTRUCTION : Select the maximum of energy window");
  m_pad->Update();
  GetPoint(m_pad->cd(), x, y);
  m_histo->GetXaxis()->SetRangeUser(0, x);
  m_pad->Update();

  for(size_t peak_i = 0; peak_i<peaks.size(); peak_i++)
  {
    auto const & peak = peaks[peak_i];

    m_pad->SetTitle(("Select peak "+std::to_string((int)peak)).c_str());
    m_pad->Update();

    GetPoint(m_pad->cd(), x, y);
    auto pos = x;
    print(x);
    m_peaks.emplace_back(peak, m_histo->GetBinContent(pos), pos);
  }

  std::cout << "E   ";
  for (auto const & peak : m_peaks) std::cout << peak.energy()  << " ";
  std::cout << std::endl;

  std::cout << "ADC ";
  for (auto const & peak : m_peaks) std::cout << peak.mean() << " ";
  std::cout << std::endl;
}

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

class PeaksCalibrator
{
public:
  PeaksCalibrator(std::vector<FittedPeak> const & peaks, int const & order = 0) :
    m_size(peaks.size()),
    m_order(order)
  {
    std::vector<double> ADCs; ADCs.reserve(m_size);
    std::vector<double> energies; energies.reserve(m_size);

    for (auto const & peak : peaks)
    {
      ADCs.push_back(peak.mean());
      energies.push_back(peak.energy());
    }

    m_graph = new TGraph(m_size, ADCs.data(), energies.data());
    m_graph->Fit(linear_fit);
    if (order < 1) m_fit.parameter0 = linear_fit->GetParameter(0);
    m_fit.parameter1 = linear_fit->GetParameter(1);
  }

  ~PeaksCalibrator()
  {
    delete m_graph;
  }

  Fit const & fit() const {return m_fit;}

  void Draw(std::string const & opt) {if (m_graph) m_graph->Draw(opt.c_str());}

private :
  size_t m_size = 0;
  int m_order = 0;

  TGraph* m_graph;
  static TF1* linear_fit;
  Fit m_fit;
};

TF1* PeaksCalibrator::linear_fit = new TF1("linear_PeaksCalibrator", "pol1");

#endif // ANALYSEDSPECTRA_HPP