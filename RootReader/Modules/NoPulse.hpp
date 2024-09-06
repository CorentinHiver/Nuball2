#ifndef NOPULSE_H
#define NOPULSE_H
#include <libRoot.hpp>
#include "../../lib/MTObjects/MultiHist.hpp"
#include "../Classes/Parameters.hpp"


class NoPulse
{
public:

  NoPulse(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitialiseManip();
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
  MultiHist<TH1F> pulse_DSSD;
  MultiHist<TH1F> pulse_first_DSSD;
  MultiHist<TH2F> Time_VS_pulse_DSSD;
  MultiHist<TH2F> Time_VS_pulse_DSSD_ref_dssd;
  MultiHist<TH2F> ref_dssd_VS_ref_any_DSSD;
  MultiHist<TH2F> ref_dssd_VS_ref_any;
  MultiHist<TH2F> Time_VS_pulse_ParisFront;
  MultiHist<TH2F> Time_VS_pulse_ParisBack;
};

bool NoPulse::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitialiseManip();
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

void NoPulse::InitialiseManip()
{
  print("Initialise histograms");
  pulse_DSSD.reset("pulse_DSSD", "pulse DSSD", 3500,-200,500);
  pulse_first_DSSD.reset("pulse_first_DSSD", "pulse first DSSD", 3500,-200,500);
  Time_VS_pulse_DSSD.reset("Time_VS_pulse_DSSD", "Time VS pulse DSSD ref any", 3500,-200,500, 3500,-200,500);
  Time_VS_pulse_DSSD_ref_dssd.reset("Time_VS_pulse_DSSD_ref_dssd", "DSSD : time relative to first DSSD of the event VS pulse ", 3500,-200,500, 3500,-200,500);
  ref_dssd_VS_ref_any_DSSD.reset("ref_dssd_VS_ref_any_DSSD", "ref DSSD VS ref any - DSSD", 3500,-200,500, 3500,-200,500);
  ref_dssd_VS_ref_any.reset("ref_dssd_VS_ref_any", "ref DSSD VS ref any", 3500,-200,500, 3500,-200,500);
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
  auto const & time_ref = event_s.times[0];
  auto const & dssd_ref = event_s.times[event_s.DSSD_hits[0]];
  ref_dssd_VS_ref_any.Fill(dssd_ref, time_ref);
  pulse_DSSD.Fill(time_ref);
  pulse_first_DSSD.Fill(time_ref);
  for (size_t loop_i = 1; loop_i<event_s.DSSD_hits.size(); loop_i++)
  {
    auto const & dssd_i = event_s.DSSD_hits[loop_i];

    auto const & time_i = event_s.times[dssd_i];

    pulse_DSSD.Fill(time_i);

    Time_VS_pulse_DSSD.Fill(time_i-time_ref, time_i);
    Time_VS_pulse_DSSD_ref_dssd.Fill(time_i-dssd_ref, time_i);
    ref_dssd_VS_ref_any_DSSD.Fill(time_i-dssd_ref, time_i-time_ref);
  }
}

void NoPulse::Analyse()
{

}

void NoPulse::Write()
{
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  outfile->cd();

  pulse_DSSD.Write();
  pulse_first_DSSD.Write();

  Time_VS_pulse_DSSD.Merge();
  Time_VS_pulse_DSSD_ref_dssd.Merge();
  ref_dssd_VS_ref_any_DSSD.Merge();
  ref_dssd_VS_ref_any.Merge();

  TH2F* Time_VS_pulse_DSSD_ref_dssd__sub__Time_VS_pulse_DSSD = static_cast<TH2F*> (Time_VS_pulse_DSSD_ref_dssd->Clone("test"));
  Time_VS_pulse_DSSD_ref_dssd__sub__Time_VS_pulse_DSSD->Add(Time_VS_pulse_DSSD.get(),-1 );

  Time_VS_pulse_DSSD.Write();
  Time_VS_pulse_DSSD_ref_dssd.Write();
  ref_dssd_VS_ref_any_DSSD.Write();
  Time_VS_pulse_DSSD_ref_dssd__sub__Time_VS_pulse_DSSD->Write();
  ref_dssd_VS_ref_any->Write();

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
