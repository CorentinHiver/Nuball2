#ifndef ANALYSERAW_H
#define ANALYSERAW_H

#include <libRoot.hpp>
#include <MTTHist.hpp>
#include <RF_Manager.hpp>

#include "../Classes/Parameters.hpp"

/**
 * @brief This class is used only at the beginning of the conversion, to check that the conversioncd .. went ok
*/
using namespace Detector;
class AnalyseRaw
{
public:

  AnalyseRaw(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void treatFile(Parameters & p, AnalyseRaw & analyseraw);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();

private:
  // ---- Parameters ---- //
  std::string param_string = "Analyse Raw";
  friend class MTObject;

  // ---- Variables ---- //
  RF_Manager rf;
  std::string m_outDir  = "136/AnalyseRaw/";
  std::string m_outRoot = "AnalyseRaw.root";

  // Energy binning between 0 and 3000 keV, number of bins per 10 keV
  std::map<Detector::det_alias, float> m_energy_binning = { {LaBr3, 1.}, {Ge, 20.}, {BGO, 0.5}, {Eden, 2.}, {RF, 10.}, {paris, 1.}, {null, 1.} };
  // Timing binning between -100 and 300 ns, number of bins per nanoseconds
  std::map<Detector::det_alias, float> m_timing_binning = { {LaBr3, 10.}, {Ge, 1.}, {BGO, 2.}, {Eden, 2.}, {RF, 10.}, {paris, 10.}, {dssd, 1.}, {null, 1.} };

  // ---- Histograms ---- //
  Vector_MTTHist<TH1F> timing_spectra;
  Vector_MTTHist<TH1F> energy_spectra;
  Vector_MTTHist<TH2F> energy2_spectra;
};

bool AnalyseRaw::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  MTObject::parallelise_function(treatFile, p, *this);
  this -> Write();
  return true;
}

void AnalyseRaw::treatFile(Parameters & p, AnalyseRaw & analyseraw)
{
  std::string rootfile;
  Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (!file.get()) {print("CAN'T OPEN", rootfile,"!!"); continue;}
    if (file->IsZombie()) {print(rootfile, "is a Zombie !"); continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    if (!tree.get()) tree.reset(file->Get<TTree>("Nuball2"));
    if (!tree.get()) {print("Nuball or Nuball2 tree not in file", rootfile);}

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    Event event(tree.get(), "ltnN");
    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      analyseraw.FillRaw(event);
      // event_s.sortEvent(event);
      // analyseraw.FillSorted(event_s,event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void AnalyseRaw::InitializeManip()
{
  RF_Manager::set_offset(50);
  print("Initialize histograms");
  timing_spectra.resize(g_detectors.size());
  energy_spectra.resize(g_detectors.size());
  energy2_spectra.resize(g_detectors.size());
  for (uint label = 0; label<g_detectors.size(); label++) 
  {
    auto const & name = g_detectors[label];
    if (name == "") continue;
    auto const & alias = Detectors::alias(label);

    if (alias == Detector::det_alias::null) print(name, Detector::det_str[alias], "check Detectors object");

    timing_spectra[label].reset((name+" time spectra").c_str(), (name+" time spectra;time [ns];# counts").c_str(), 500*m_timing_binning[alias], -100, 400);

    if (isDSSD[label]) energy_spectra[label].reset((name+" energy spectra").c_str(), (name+" energy spectra;energy [keV]; # counts").c_str(), 500, 0, 20000);
    else  energy_spectra[label].reset((name+" energy spectra").c_str(), (name+" energy spectra;energy [keV]; # counts").c_str(), 300*m_energy_binning[alias], 0, 3000);

    if (isParis[label] || isEden[label]) energy2_spectra[label].reset((name+" energy2 spectra").c_str(), (name+" nrj2 VS nrj spectra;nrj [u.a.];nrj2 [u.a.]").c_str(), 
      300*m_energy_binning[alias], 0, 3000, 300*m_energy_binning[alias], 0, 3000);
  }
}

void AnalyseRaw::FillRaw(Event const & event)
{
  for (size_t i = 0; i<event.size(); i++)
  {
    auto const & label = event.labels[i];
    auto const & time  = event.times [i];
    auto const & nrj   = event.nrjs  [i];
    auto const & nrj2  = event.nrj2s [i];

    if (label == RF_Manager::label)
    {
      rf.set(time, nrj);
      continue;
    }

    if (nrj2 != 0)
    {
      if (isParis[label])
      {
        energy2_spectra[label].Fill(nrj, nrj2);
        if ( (nrj/nrj2) > 0.5 ) 
        {
          energy_spectra[label].Fill(nrj);
          timing_spectra[label].Fill(rf.pulse_ToF(time)/_ns);
        }
      }
      else
      {
        energy_spectra[label].Fill(nrj);
        timing_spectra[label].Fill(rf.pulse_ToF(time)/_ns);
      }
    }
    else
    {
      energy_spectra[label].Fill(nrj);
      timing_spectra[label].Fill(rf.pulse_ToF(time)/_ns);
    }
  }
}

void AnalyseRaw::FillSorted(Sorted_Event const & event_s, Event const & event)
{
//    for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//    {
//    }
}

void AnalyseRaw::Analyse()
{
  
}

void AnalyseRaw::Write()
{
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  outfile->cd();
  print("Writting histograms ...");
  // Write here the histograms :
  for (auto & hist : timing_spectra)  hist.Write();
  for (auto & hist : energy_spectra)  hist.Write();
  for (auto & hist : energy2_spectra) hist.Write();
  outfile->Write();
  outfile->Close();
  print("Writting analysis in", m_outDir+m_outRoot);
}

bool AnalyseRaw::setParameters(std::vector<std::string> const & parameters)
{
  if (parameters.size()<1){print("No parameters for "+param_string+" !!"); return false;}  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "activated") continue;
      if (temp == "outDir:")  is >> m_outDir;
      else if (temp == "outRoot:")  is >> m_outRoot;
      else
      {
        print("Parameter", temp, "for", param_string, "unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //ANALYSERAW_H
