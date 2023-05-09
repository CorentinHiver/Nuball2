#ifndef NOPULSE_H
#define NOPULSE_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"


class NoPulse
{
public:

  NoPulse(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void run(Parameters & p, NoPulse & nopulse);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();
private:

  // ---- Parameters ---- //
  std::string param_string = "NoPulse";
  friend class MTObject;
  // ---- Variables ---- //
  std::string m_outDir  = "129/NoPulse/";
  std::string m_outRoot = "NoPulse.root";
  // ---- Histograms ---- //
};

bool NoPulse::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void NoPulse::run(Parameters & p, NoPulse & nopulse)
{
  std::string rootfile;
  Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "lTn");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      nopulse.FillSorted(event_s,event);
      nopulse.FillRaw(event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void NoPulse::InitializeManip()
{
  print("Initialize histograms");
}

void NoPulse::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void NoPulse::FillSorted(Sorted_Event const & event_s, Event const & event)
{
//    for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//    {
//    }
}

void NoPulse::Analyse()
{

}

void NoPulse::Write()
{
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  outfile->cd();
  outfile->Write();
  outfile->Close();
  print("Writting analysis in", m_outDir+m_outRoot);
}

bool NoPulse::setParameters(std::vector<std::string> const & parameters)
{
  if (parameters.size()<1){print("No Parameters for "+param_string+" !!"); return false;}  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
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

#endif //NOPULSE_H
