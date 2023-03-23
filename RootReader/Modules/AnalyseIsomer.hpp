#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

#include "../../lib/utils.hpp"

#include "../../lib/Analyse/HistoAnalyse.hpp"

#include "../../lib/MTObjects/MTTHist.hpp"

#include "../../lib/Classes/Gate.hpp"
#include "../../lib/Classes/RWMat.hxx"

#include "../Classes/Parameters.hpp"

std::string param_string = "Isomer";

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const &  param);
  void InitializeManip();
  static void run(Parameters & p, AnalyseIsomer & ai);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();

private:
  // Parameters
  friend class MTObject;
  std::string outDir  = "129/Analyse/Isomer/";
  std::string outRoot = "ai.root";

  Gate Ge_prompt_gate;
  Gate Ge_delayed_gate;
  Gate LaBr3_prompt_gate;
  Gate LaBr3_delayed_gate;

  MTTHist<TH1F> Ge_spectra;
  MTTHist<TH1F> Ge_spectra_prompt;
  MTTHist<TH1F> Ge_spectra_delayed;
  MTTHist<TH2F> GePrompt_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed;
  MTTHist<TH2F> GeDelayed_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_SumGeDelayed;
  MTTHist<TH2F> Ge_Time_VS_Spectra;
  MTTHist<TH2F> raw_Ge_Time_VS_Spectra;

  MTTHist<TH2F> Ge_VS_DSSD;
  MTTHist<TH2F> GePrompt_VS_DSSD;
  MTTHist<TH2F> GeDelayed_VS_DSSD;
};

bool AnalyseIsomer::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void AnalyseIsomer::run(Parameters & p, AnalyseIsomer & ai)
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
      ai.FillSorted(event_s,event);
      ai.FillRaw(event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void AnalyseIsomer::InitializeManip()
{
  print("Initialize histograms");
  Ge_spectra.reset("Ge spectra","Ge spectra", 14000,0,7000);
  Ge_spectra_prompt.reset("Ge spectra prompt","Ge spectra prompt", 14000,0,7000);
  Ge_spectra_delayed.reset("Ge spectra delayed","Ge spectra delayed", 14000,0,7000);
  Ge_Time_VS_Spectra.reset("Ge spectra VS Time","Ge spectra VS Time",
      14000,0,7000, 1000,-100,400);
  raw_Ge_Time_VS_Spectra.reset("Raw_Ge_spectra_VS_Time","Raw Ge spectra VS Time",
      14000,0,7000, 1000,-100,400);
  GePrompt_VS_GePrompt.reset("Ge_bidim_prompt","Ge bidim prompt",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GeDelayed.reset("Ge_bidim_delayed","Ge bidim delayed",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GePrompt.reset("Ge_delayed_VS_prompt","Ge delayed VS prompt",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_SumGeDelayed.reset("Ge_delayed_VS_sum_Ge_delayed","Ge delayed VS sum Ge delayed",
      4096,0,4096, 4096,0,4096);
  Ge_VS_DSSD.reset("Ge VS DSSD","Ge VS DSSD",
      2000,0,20000, 14000,0,7000);
  GePrompt_VS_DSSD.reset("Ge Prompt VS DSSD","Ge VS DSSD",
      2000,0,20000, 14000,0,7000);
  GeDelayed_VS_DSSD.reset("Ge Delayed VS DSSD","Ge VS DSSD",
      2000,0,20000, 14000,0,7000);
}

void AnalyseIsomer::FillRaw(Event const & event)
{
  for (size_t i = 0; i<event.size(); i++)
  {
    auto const & label = event.labels[i];
    if (isGe[label])
    {
      auto const & nrj = event.nrjs[i];
      auto const & Time = event.Times[i];
      raw_Ge_Time_VS_Spectra.Fill(nrj,Time);
    }
  }
}

void AnalyseIsomer::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
  {
    auto const & clover_i = event_s.clover_hits[loop_i];
    auto const & nrj_i = event_s.nrj_clover[clover_i];
    auto const & Time_i = event_s.time_clover[clover_i];
    auto const delayed_i = Time_i>50 && Time_i<150;
    auto const prompt_i =  Time_i>-10 && Time_i<20;

    if (event_s.BGO[clover_i] || nrj_i<5) continue;

    Ge_Time_VS_Spectra.Fill(nrj_i, Time_i);
    Ge_spectra.Fill(nrj_i);
    if (delayed_i) Ge_spectra_delayed.Fill(nrj_i);
    else if (prompt_i) Ge_spectra_prompt.Fill(nrj_i);

    for (size_t loop_j = loop_i+1; loop_j<event_s.clover_hits.size(); loop_j++)
    {
      auto const & clover_j = event_s.clover_hits[loop_j];
      auto const & nrj_j = event_s.nrj_clover[clover_j];
      auto const & delayed_j = event_s.delayed_Ge[clover_j];
      auto const & prompt_j = event_s.delayed_Ge[clover_j];
      auto const & Time_j = event_s.time_clover[clover_j];

      if (event_s.BGO[clover_j] || nrj_j<5) continue;

      if (prompt_i)
      {
        if (prompt_j)
        {
          GePrompt_VS_GePrompt . Fill(nrj_i,nrj_j);
          GePrompt_VS_GePrompt . Fill(nrj_j,nrj_i);
        }
        else if (delayed_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_i,nrj_j);
        }
      }
      else if (delayed_i)
      {
        if (delayed_j && abs(Time_i-Time_j)<60)
        {
          GeDelayed_VS_GeDelayed . Fill(nrj_i, nrj_j);
          GeDelayed_VS_GeDelayed . Fill(nrj_j, nrj_i);
          GeDelayed_VS_SumGeDelayed.Fill(nrj_i, nrj_i+nrj_j);
          GeDelayed_VS_SumGeDelayed.Fill(nrj_j, nrj_i+nrj_j);
        }
        else if (prompt_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
        }
      }
    }
    for (auto const & dssd : event_s.DSSD_hits)
    {
      auto const & dssd_nrj = event.nrjs[dssd];
      // auto const dssd_label = event.labels[dssd] - 800;
      if (event_s.DSSDRingMult == event_s.DSSDSectorMult)
      {
        Ge_VS_DSSD.Fill(dssd_nrj,nrj_i);
        if (prompt_i) GePrompt_VS_DSSD.Fill(dssd_nrj,nrj_i);
        if (delayed_i)GeDelayed_VS_DSSD.Fill(dssd_nrj,nrj_i);
      }
    }
  }
}

void AnalyseIsomer::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  Ge_spectra.Write();
  Ge_spectra_delayed.Write();
  Ge_spectra_prompt.Write();
  print("Writting bidims ...");
  Ge_Time_VS_Spectra.Write();
  raw_Ge_Time_VS_Spectra.Write();
  GePrompt_VS_GePrompt.Write();
  GeDelayed_VS_GeDelayed.Write();
  GeDelayed_VS_GePrompt.Write();
  GeDelayed_VS_SumGeDelayed.Write();
  Ge_VS_DSSD.Write();
  GePrompt_VS_DSSD.Write();
  GeDelayed_VS_DSSD.Write();
  oufile->Write();
  oufile->Close();

  RWMat RW_prompt_prompt(GePrompt_VS_GePrompt); RW_prompt_prompt.Write();
  RWMat RW_del_del(GeDelayed_VS_GeDelayed); RW_del_del.Write();

  print("Writting analysis in", outDir+outRoot);
}

bool AnalyseIsomer::setParameters(std::vector<std::string> const & parameters)
{
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> outDir;
      else if (temp == "outRoot:")  is >> outRoot;
      else if (temp == "gate:")
      {
        is >> temp;
        if (temp == "prompt")
        {
          is >> temp;
          if (temp == "Ge")
          {
            is >> Ge_prompt_gate.start >> Ge_prompt_gate.stop;
            print("Prompt Ge gate : ", Ge_prompt_gate.start, Ge_prompt_gate.stop);
          }
          else if (temp == "LaBr3")
          {
            is >> LaBr3_prompt_gate.start >> LaBr3_prompt_gate.stop;
            print("Prompt LaBr3 gate : ", LaBr3_prompt_gate.start, LaBr3_prompt_gate.stop);
          }
          else {print("Parameter ", temp, "unkown for prompt gate...");}
        }
        if (temp == "delayed")
        {
          is >> temp;
          if (temp == "Ge")
          {
            is >> Ge_delayed_gate.start >> Ge_delayed_gate.stop;
            print("Delayed Ge gate : ", Ge_delayed_gate.start, Ge_delayed_gate.stop);
          }
          else if (temp == "LaBr3")
          {
            is >> LaBr3_delayed_gate.start >> LaBr3_delayed_gate.stop;
            print("Delayed LaBr3 gate : ", LaBr3_delayed_gate.start, LaBr3_delayed_gate.stop);
          }
          else {print("Parameter ", temp, "unkown for prompt gate...");return false;}
        }
      }
      else
      {
        print("Parameter", temp, "for AnalyseIsomer unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //ANALYSEISOMER_H
