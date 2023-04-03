#ifndef RUNCHECK_H
#define RUNCHECK_H
#include "../../lib/utils.hpp"
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
  size_t nbThreads = 1.;
  friend class MTObject;

  // ---- Variables ---- //
  std::string m_outDir  = "129/RunCheck/";
  std::string m_outRoot = "RunCheck.root";

  // ---- Histograms ---- //
  // Histograms for each run :
  MTTHist<TH2F> TimeSpectra;
  // Histograms for the whole runs :
  MTTHist<TH2F> GeSpectraManip;
  MTTHist<TH1F> GeSpectraTotal;
  Vector_MTTHist<TH1F> TimeEachDetector;
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
  print("Initialize the manip histograms...");
  GeSpectraManip.reset("EachCloverSpectra", "Each Clover Spectra", 24,0,24, 5000,0,5000);
  GeSpectraTotal.reset("GeSpectraTotal", "GeSpectraTotal", 5000,0,5000);
  TimeEachDetector.resize(1000);
  std::string name;
  for (int i = 0; i<1000; i++)
  {
    name = g_labelToName[i];
    if (name.size()>1)
    {
      name = name+" time each run";
      TimeEachDetector.reset(name.c_str(), name.c_str(), 500, -100, 400);
    }
  }
}

// Technical point : run must be static in order to be called by MTObject::parallelise_function
// Therefore, it needs to have a RunCheck as parameter
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
    // runcheck.TreatRun(_run, p);
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
      tree.reset(nullptr);
      file.reset(nullptr);
    }

    runcheck.AnalyseRun();
    runcheck.WriteRun(_run);
    auto const & time = timer();
    print(_run, time, timer.unit(), ":", run_size/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void RunCheck::InitializeRun(std::string const & run_name)
{
  TimeSpectra.reset("Times"+run_name,"Relative Timestamp", 900,0,900, 500,-100,400);
}

void RunCheck::FillRaw(Event const & event)
{
  for (size_t i = 0; i<event.size(); i++)
  {
    auto const & label = event.labels[i];
    auto const & time = event.Times[i];

    // Fill each run :
    TimeSpectra.Fill(label, time);

    // Fill whole manip :
  }
}

void RunCheck::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  // auto const & thread_i = MTObject::getThreadIndex();
   for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
   {
     auto const & clover_i = event_s.clover_hits[loop_i];

     auto const & nrj_i = event_s.nrj_clover[clover_i];
     // auto const & time_i = event_s.time_clover[clover_i];

     // Fill each run :

     // Fill whole manip :
     GeSpectraManip.Fill(clover_i, nrj_i);
     GeSpectraTotal.Fill(nrj_i);
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
  outfile->Write();
  outfile->Close();
}

void RunCheck::AnalyseManip()
{
  HistoAnalyse test_a (GeSpectraManip);
  test_a.NormalizeY(5);
}

void RunCheck::WriteManip()
{
  print("Writting histograms ...");
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  outfile->cd();
  GeSpectraManip.Write();
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
