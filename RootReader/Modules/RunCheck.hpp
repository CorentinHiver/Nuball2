#ifndef RUNCHECK_H
#define RUNCHECK_H
#include <libRoot.hpp>
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"
#include "../../lib/Analyse/HistoAnalyse.hpp"

class RunCheck
{
public:

  RunCheck(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  static void run(Parameters & p, RunCheck & runcheck);
  void InitializeManip();
  void InitializeRun(std::string const & run);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void AnalyseRun();
  void WriteRun(std::string const & _run);
  void AnalyseManip();
  void WriteManip();
private:

  // ---- Parameters ---- //
  std::string param_string = "Run Check";
  std::vector<std::string> m_name_run;
  friend class MTObject;

  // ---- Variables ---- //
  std::string m_outDir  = "129/RunCheck/";
  std::string m_outRoot = "ManipCheck.root";

  // ---- Histograms ---- //
  // Histograms for each run :
  MTTHist<TH2F> TimeSpectra;
  MTTHist<TH2F> CloverTimeSpectra;
  MTTHist<TH2F> GeRunSpectra;
  // Histograms for the whole runs :
  MTTHist<TH2F> GeSpectraManip;
  MTTHist<TH1F> GeSpectraTotal;
  Vector_MTTHist<TH2F> TimeEachDetector;
  Vector_MTTHist<TH2F> EnergyEachDetector;
  Vector_MTTHist<TH2F> EnergyClover;
};

bool RunCheck::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> AnalyseManip();
  this -> WriteManip();
  return true;
}

void RunCheck::InitializeManip()
{
  auto const & nbThreads = MTObject::getThreadsNb();
  m_name_run.resize(nbThreads);
  print("Initialize the manip histograms...");
  GeSpectraManip.reset("Each_Clover_Spectra", "Each Clover Spectra", 24,0,24, 5000,0,5000);
  GeSpectraTotal.reset("Ge_Spectra_Total", "Ge Spectra Total", 5000,0,5000);
  TimeEachDetector.resize(g_labelToName.size());
  EnergyEachDetector.resize(g_labelToName.size());
  EnergyClover.resize(24);
  for (size_t i = 0; i<EnergyClover.size(); i++)
    EnergyClover[i].reset(("Clover_"+std::to_string(i)).c_str(), ("Clover "+std::to_string(i)+" spectra over runs").c_str(),
        24,0,24, 14000,0,7000);
  std::string name;
  BinMap<int> nrj_bins;
  BinMap<Float_t> nrj_bin_min;
  BinMap<Float_t> nrj_bin_max;
  for (size_t i = 0; i<g_labelToName.size(); i++)
  {
    name = g_labelToName[i];
    if (name.size()>1)
    {
      TimeEachDetector[i].reset((name+" time each run").c_str(), (name+" time each run").c_str(), 82,20,102, 250,-100,400);

           if (isGe   [i]) { nrj_bins[i] = 7000; nrj_bin_min[i] = 0; nrj_bin_max[i] =  7000; }
      else if (isBGO  [i]) { nrj_bins[i] =   350; nrj_bin_min[i] = 0; nrj_bin_max[i] =  7000; }
      else if (isParis[i]) { nrj_bins[i] =  3500; nrj_bin_min[i] = 0; nrj_bin_max[i] =  7000; }
      else if (isLaBr3[i]) { nrj_bins[i] =  3500; nrj_bin_min[i] = 0; nrj_bin_max[i] =  7000; }
      else if (isDSSD [i]) { nrj_bins[i] =   600; nrj_bin_min[i] = 0; nrj_bin_max[i] = 30000; }
      else                 { nrj_bins[i] =     1; nrj_bin_min[i] = 0; nrj_bin_max[i] =     1; }
      EnergyEachDetector[i].reset((name+" nrj each run").c_str(), (name+" nrj each run").c_str(),
          82,20,102, nrj_bins[i],nrj_bin_min[i],nrj_bin_max[i]);
    }
  }
}

// Technical point : the run method must be static in order to be called by MTObject::parallelise_function
// Therefore, it needs to have a RunCheck class as parameter
void RunCheck::run(Parameters & p, RunCheck & runcheck)
{
  std::string _run;
  while(p.getNextRun(_run))
  {
    Timer timer;
    runcheck.InitializeRun(_run);
    auto const & listFilesRun = p.getRunFiles(_run);
    Sorted_Event event_s;
    int run_size = 0;
    for (auto const & rootfile : listFilesRun)
    {
      std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
      if (!file) {print(rootfile, "doesn't exists !"); continue;}
      if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
      std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
      Event event(tree.get(), "lTn");

      size_t events = tree->GetEntries();
      p.totalCounter+=events;

      auto const & filesize = size_file(rootfile, "Mo");
      p.totalFilesSize+=filesize;
      run_size+=filesize;

      for (size_t i = 0; i<events; i++)
      {
        tree->GetEntry(i);
        event_s.sortEvent(event);
        runcheck.FillSorted(event_s, event);
        runcheck.FillRaw(event);
      } // End event loop
    }

    runcheck.AnalyseRun();
    runcheck.WriteRun(_run);

    auto const & time = timer();
    print(_run, time, timer.unit(), ":", run_size/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void RunCheck::InitializeRun(std::string const & run_name)
{
  auto const & thread_i = MTObject::getThreadIndex();
  m_name_run[thread_i] = run_name;
  TimeSpectra.reset("Times"+run_name,"Relative Timestamp", 900,0,900, 500,-100,400);
  CloverTimeSpectra.reset("Clover E VS Times"+run_name,"E VS Time Clovers", 500,-100,400, 2000,0,4000);
}

void RunCheck::FillRaw(Event const & event)
{
  auto const & thread_i = MTObject::getThreadIndex();
  auto const & run_name = m_name_run[thread_i];
  size_t run_nb = stoi(lastPart(run_name, '_'));

  for (size_t i = 0; i<event.size(); i++)
  {
    auto const & label = event.labels[i];
    auto const & time = event.time2s[i];
    auto const & nrj = event.nrjs[i];

    if (nrj<20) continue;
    if (isBGO[label] && nrj<100) continue;

    // Fill each run :
    TimeSpectra[thread_i]->Fill(label, time);
    TimeEachDetector[label][thread_i]->Fill(run_nb, time);
    EnergyEachDetector[label][thread_i]->Fill(run_nb, nrj);
    // Fill whole manip :
  }
}

void RunCheck::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  auto const & thread_i = MTObject::getThreadIndex();
  auto const & run_name = m_name_run[thread_i];
  size_t run_nb = stoi(lastPart(run_name, '_'));

  for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
  {
    auto const & clover_i = event_s.clover_hits[loop_i];

    auto const & nrj_i = event_s.nrj_clover[clover_i];
    auto const & time_i = event_s.time_clover[clover_i];

    // Fill each run :
    CloverTimeSpectra[thread_i]->Fill(time_i, nrj_i);
    // Fill whole manip :
    GeSpectraManip[thread_i]->Fill(clover_i, nrj_i);
    GeSpectraTotal[thread_i]->Fill(nrj_i);
    EnergyClover[clover_i][thread_i]->Fill(run_nb, nrj_i);
  }
}

void RunCheck::AnalyseRun()
{

}

void RunCheck::WriteRun(std::string const & _run)
{
  // auto const & threadNb = MTObject::getThreadIndex();
  create_folder_if_none(m_outDir);
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+_run+".root").c_str(),"recreate"));
  outfile->cd();
  TimeSpectra.Write();
  CloverTimeSpectra.Write();
  outfile->Write();
  outfile->Close();
}

void RunCheck::AnalyseManip()
{
  HistoAnalyse GeSpectraManip_a(GeSpectraManip);
  GeSpectraManip_a.normalizeY(5);

  for (auto & histo : EnergyEachDetector)
  {
    HistoAnalyse analyse(histo);
    analyse.normalizeY(1);
  }

  for (auto & histo : TimeEachDetector)
  {
    HistoAnalyse analyse(histo);
    analyse.normalizeY(1);
  }
}

void RunCheck::WriteManip()
{
  print("Writting histograms ...");
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  outfile->cd();
  GeSpectraManip.Write();
  for (auto & histo : EnergyEachDetector) if (histo) histo.Write();
  for (auto & histo : TimeEachDetector)   if (histo) histo.Write();
  outfile->Write();
  outfile->Close();
  print("Writting manip analysis in ", m_outDir);
}

bool RunCheck::setParameters(std::vector<std::string> const & parameters)
{
  if (parameters.size()<1){print("No Parameters read for "+param_string+" !!"); return false;}
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> m_outDir;
      else if (temp == "outRoot:")  is >> m_outRoot;
      else
      {
        print("Parameter", temp, "for RunCheck unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //RUNCHECK_H
