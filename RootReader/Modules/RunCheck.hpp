#ifndef RUNCHECK_H
#define RUNCHECK_H
#include <libRoot.hpp>
#include "../../lib/MTObjects/MultiHist.hpp"
#include "../Classes/Parameters.hpp"
#include "../../lib/Analyse/HistoAnalyse.hpp"
#include <DSSD.hpp>
// #include <Paris.hpp>
#include <Clovers.hpp>

template<class T>
using BinMap       = std::unordered_map < Int_t   , T           > ;

class RunCheck
{
public:

  RunCheck(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  static void run(Parameters & p, RunCheck & runcheck);
  void InitialiseManip();
  void InitialiseRun(std::string const & run);
  void FillRaw(Event const & event);
  void FillSorted(Clovers const & clovers, DSSD const & dssd, Event const & event);
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
  std::string m_outDir  = "136/RunCheck/";
  std::string m_outRoot = "ManipCheck.root";

  // ---- Histograms ---- //
  // Histograms for each run :
  MultiHist<TH2F> TimeSpectra;
  MultiHist<TH2F> CloverTimeSpectra;
  MultiHist<TH2F> GeRunSpectra;
  // Bidims for each run :
  MultiHist<TH2F> R3A1_red_vs_clover;
  MultiHist<TH2F> R3A1_BGO1_vs_clover;
  MultiHist<TH2F> Paris_LaBr3_vs_clover;
  MultiHist<TH2F> Paris_NaI_vs_clover;

  // Histograms for the whole runs :
  MultiHist<TH2F> GeSpectraManip;
  MultiHist<TH1F> GeSpectraTotal;
  Vector_MTTHist<TH2F> TimeEachDetector;
  Vector_MTTHist<TH2F> EnergyEachDetector;
  Vector_MTTHist<TH2F> EnergyClover;
};

bool RunCheck::launch(Parameters & p)
{
  debug("loading parameters");
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitialiseManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> AnalyseManip();
  this -> WriteManip();
  return true;
}

void RunCheck::InitialiseManip()
{
  auto const & nbThreads = MTObject::getThreadsNb();
  m_name_run.resize(nbThreads);
  print("Initialise the manip histograms...");
  GeSpectraManip.reset("Each_Clover_Spectra", "Each Clover Spectra", 24,0,24, 10000,0,10000);
  GeSpectraTotal.reset("Ge_Spectra_Total", "Ge Spectra Total", 10000,0,10000);
  TimeEachDetector.resize(g_detectors.size());
  EnergyEachDetector.resize(g_detectors.size());
  EnergyClover.resize(24);
  for (size_t i = 0; i<EnergyClover.size(); i++)
    EnergyClover[i].reset(("Clover_"+std::to_string(i)).c_str(), ("Clover "+std::to_string(i)+" spectra over runs").c_str(),
        24,0,24, 14000,0,7000);
  std::string name;
  BinMap<int> nrj_bins;
  BinMap<Float_t> nrj_bin_min;
  BinMap<Float_t> nrj_bin_max;
  for (size_t i = 0; i<g_detectors.size(); i++)
  {
    name = g_detectors[i];
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
    print(_run);
    Timer timer;
    runcheck.InitialiseRun(_run);
    auto const & listFilesRun = p.getRunFiles(_run);
    int run_size = 0;
    Clovers clovers;
    DSSD dssd;
    RF_Manager rf;
    int loop = 0;
    int file_number = 0;
    for (auto const & rootfile : listFilesRun)
    {
      file_number++;
      print(rootfile);
      unique_TFile file (TFile::Open(rootfile.c_str(), "READ"));
      if (!file) {print(rootfile, "doesn't exists !"); continue;}
      if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
      unique_tree tree (file->Get<TTree>("Nuball2"));
      if (!tree || tree -> IsZombie()) {print("NO Nuball2 FOUND"); continue;}
      Event event(tree.get(), "lTE");
      size_t events = tree->GetEntries();
      p.totalCounter+=events;

      auto const & filesize = size_file(rootfile, "Mo");
      p.totalFilesSize+=filesize;
      run_size+=filesize;

      for (size_t i = 0; i<events; i++)
      {
      if (loop % (int)(5.e+6) == 0) print(loop, file_number);

        tree->GetEntry(i);
        if (event.read.t && !event.read.T)
        {
          if (event.labels[0] == RF_Manager::label)
          {
            rf.setEvent(event);
            continue;
          }
          else rf.align_RF_ns(event);
        }
        clovers.SetEvent(event);
        dssd.SetEvent(event);
        // event_s.sortEvent(event);
        runcheck.FillSorted(clovers, dssd, event);
        // runcheck.FillRaw(event);
        loop++;
        // if (loop > (int)(2.e+6)) break;
      } // End event loop
      // if (loop > (int)(2.e+6)) break;
    }
    runcheck.AnalyseRun();
    runcheck.WriteRun(_run);

    auto const & time = timer();
    print(_run, time, timer.unit(), ":", run_size/timer.TimeSec(), "Mo/s");
  } // End runs loop
}

void RunCheck::InitialiseRun(std::string const & run_name)
{
  m_name_run[MTObject::getThreadIndex()] = run_name;
  TimeSpectra.reset("Times"+run_name,"Relative Timestamp", 900,0,900, 500,-100,400);
  CloverTimeSpectra.reset("Clover E VS Times"+run_name,"E VS Time Clovers", 500,-100,400, 2000,0,4000);
  R3A1_red_vs_clover.reset("R3A1_red_vs_clover_"+run_name,"R3A1_red VS Clover", 10000,0,10000, 10000,0,10000);
  R3A1_BGO1_vs_clover.reset("R3A1_BGO1_vs_clover"+run_name,"R3A1_BGO1 VS Clover", 10000,0,10000, 250,0,5000);
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
    TimeSpectra.Fill(label, time);
    TimeEachDetector[label].Fill(run_nb, time);
    EnergyEachDetector[label].Fill(run_nb, nrj);
    // Fill whole manip :
  }
}

void RunCheck::FillSorted(Clovers const & clovers, DSSD const & dssd, Event const & event)
{
  auto const & run_name = m_name_run[MTObject::getThreadIndex()];
  size_t run_nb = stoi(lastPart(run_name, '_'));
  for(auto const & clover_i : clovers.CleanGe)
  {
    auto const & clover = clovers[clover_i];
    GeSpectraManip.Fill(clover.label(), clover.nrj);
    GeSpectraTotal.Fill(clover.nrj);
    EnergyClover[clover_i].Fill(run_nb, clover.nrj);
    for (auto const & ge_crystal : clovers.cristaux)
    {
      if (ge_crystal == 0 && clover.label()!=ge_crystal/4) 
      {
        // print(ge_crystal, clover.nrj, clovers.cristaux_nrj[ge_crystal]);
        // print(R3A1_red_vs_clover.Integral());
        // pauseCo();
        R3A1_red_vs_clover.Fill(clover.nrj, clovers.cristaux_nrj[ge_crystal]);
      }
    }
    for (auto const & bgo_crystal : clovers.cristaux_BGO)
    {
      if (bgo_crystal == 0) R3A1_BGO1_vs_clover.Fill(clover.nrj, clovers.cristaux_nrj_BGO[bgo_crystal]);
    }
  }
}
// void RunCheck::FillSorted(Sorted_Event const & event_s, Event const & event)
// {
//   auto const & thread_i = MTObject::getThreadIndex();
//   auto const & run_name = m_name_run[thread_i];
//   size_t run_nb = stoi(lastPart(run_name, '_'));

//   for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//   {
//     auto const & clover_i = event_s.clover_hits[loop_i];

//     auto const & nrj_i = event_s.nrj_clover[clover_i];
//     auto const & time_i = event_s.time_clover[clover_i];

//     // Fill each run :
//     CloverTimeSpectra.Fill(time_i, nrj_i);
//     // Fill whole manip :
//     GeSpectraManip.Fill(clover_i, nrj_i);
//     GeSpectraTotal.Fill(nrj_i);
//     EnergyClover[clover_i].Fill(run_nb, nrj_i);
//   }
// }

void RunCheck::AnalyseRun()
{

}

void RunCheck::WriteRun(std::string const & _run)
{
  // auto const & threadNb = MTObject::getThreadIndex();
  create_folder_if_none(m_outDir);
  unique_TFile outfile(TFile::Open((m_outDir+_run+".root").c_str(),"recreate"));
  outfile->cd();
  TimeSpectra.Write();
  CloverTimeSpectra.Write();
  R3A1_red_vs_clover.Write();
  R3A1_BGO1_vs_clover.Write();
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
  unique_TFile outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
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
      else if (temp == "activated");
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
