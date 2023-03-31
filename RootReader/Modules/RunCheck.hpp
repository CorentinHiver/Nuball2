#ifndef RUNCHECK_H
#define RUNCHECK_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"


class RunCheck
{
public:

  RunCheck(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  static void run(Parameters & p, RunCheck & runcheck);
  void Initialize();
  void InitializeRun(std::string const & run);
  void TreatRun(std::string const & run, Parameters & p);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void AnalyseRun();
  void WriteRun(std::string const & _run);
  void WriteManip();
private:

  // ---- Parameters ---- //
  std::string param_string = "RunCheck";
  size_t nbThreads = 1.;
  friend class MTObject;

  // ---- Variables ---- //
  std::string m_outDir  = "129/RunCheck/";
  std::string m_outRoot = "RunCheck.root";

  // ---- Histograms ---- //
  MTTHist<TH1F> testRun;
  MTTHist<TH1F> testManip;
};

bool RunCheck::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> Initialize();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  return true;
}

void RunCheck::Initialize()
{
  testManip.reset("test Manip", "test", 100, 0, 100);
}

void RunCheck::run(Parameters & p, RunCheck & runcheck)
{
  std::string _run;
  while(p.getNextRun(_run))
  {
    runcheck.TreatRun(_run, p);
  } // End files loop
}

void RunCheck::TreatRun(std::string const & _run, Parameters & p)
{
  this -> InitializeRun(_run);
  std::vector<std::string> listFilesRun = p.getRunFiles(_run);
  Sorted_Event event_s;
  Timer timer;
  int run_size = 0;
  for (auto const & rootfile : listFilesRun)
  {
    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
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
      this -> FillSorted(event_s, event);
      this -> FillRaw(event);
    } // End event loop
  }
  auto const & time = timer();
  print(_run, time, timer.unit(), ":", run_size/timer.TimeSec(), "Mo/s");
  this -> WriteRun(_run);
}

void RunCheck::InitializeRun(std::string const & run_name)
{
  // auto const & threadNb = MTObject::getThreadIndex();
  testRun.reset("test_"+run_name,"test",100,0,100);
}

void RunCheck::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void RunCheck::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  // auto const & thread_i = MTObject::getThreadIndex();
   for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
   {
     auto const & clover_i = event_s.clover_hits[loop_i];
     auto const & nrj_i = event_s.nrj_clover[clover_i];
     testRun.Fill(nrj_i);
     testManip.Fill(nrj_i);
   }
}

void RunCheck::AnalyseRun()
{

}

void RunCheck::WriteRun(std::string const & _run)
{
  auto const & threadNb = MTObject::getThreadIndex();
  create_folder_if_none(outPath);
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+_run+m_outRoot).c_str(),"recreate"));
  if (testRun[threadNb]) testRun[threadNb]->Write();
  outfile->Write();
  outfile->Close();
  print("Writting ", outPath+m_outRoot);
}

void WriteManip()
{
  print("Writting histograms ...");
  std::unique_ptr<TFile> outfile(TFile::Open((outPath+m_outRoot).c_str(),"recreate"));

}

bool RunCheck::setParameters(std::vector<std::string> const & parameters)
{
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
