#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

#include <MTObject.hpp>
#include <libRoot.hpp>

#include "../../lib/MTObjects/MTTHist.hpp"

#include "../../lib/Analyse/HistoAnalyse.hpp"


#include "../../lib/Classes/Gate.hpp"
#include "../../lib/Classes/RWMat.hxx"

#include "../Classes/Parameters.hpp"
#include <RF_Manager.hpp>


class AnalyseIsomer
{
public:
  AnalyseIsomer(){}
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const &  param);
  void InitializeManip();
  static void run(Parameters & p, AnalyseIsomer & ai);
  void FillRaw(Event const & event);
  void FillClover(Clovers & clovers, Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();

private:
  std::string param_string = "Isomer";
  // Parameters
  friend class MTObject;
  std::string outDir  = "129/Analyse/Isomer/";
  std::string outRoot = "ai.root";

  Gate Ge_prompt_gate;
  Gate Ge_delayed_gate;
  Gate Ge_reject_gate;
  Gate LaBr3_prompt_gate;
  Gate LaBr3_delayed_gate;

  MTTHist<TH1F> Ge_spectra;
  MTTHist<TH1F> Ge_spectra_prompt;
  MTTHist<TH1F> Ge_spectra_delayed;
  MTTHist<TH2F> Ge_VS_Ge;
  MTTHist<TH2F> GePrompt_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed_time;
  MTTHist<TH2F> GeDelayed_VS_GePrompt;
  MTTHist<TH2F> Ge_Time_VS_Spectra;
  MTTHist<TH2F> raw_Ge_Time_VS_Spectra;

  MTTHist<TH2F> each_Ge_VS_Time;

  MTTHist<TH2F> Ge_VS_DSSD;
  MTTHist<TH2F> GePrompt_VS_DSSD;
  MTTHist<TH2F> GeDelayed_VS_DSSD;
  MTTHist<TH2F> DSSD_TW;
  MTTHist<TH2F> Ge_VS_DSSD_Time;

  MTTHist<TH2F> BGO_R3A1_1_VS_Ge;

  // MTTHist<TH2F> mult_VS_Time; In order to see the evolution of Multiplicity over time. To do it, take a moving 50ns time window to group events
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
  // Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}

    // TTree* tree = file->Get<TTree>("Nuball");
    // if (!tree) tree = file->Get<TTree>("Nuball2");
    // if (!tree) {print("Nuball or Nuball2 trees not found in",rootfile ); continue;}
    // Event event(tree, "ltE");
    
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    if (!tree.get()) tree.reset(file->Get<TTree>("Nuball2"));
    if (!tree.get()) {print("Nuball or Nuball2 trees not found in",rootfile ); continue;}
    Event event(tree.get(), "lTE");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    RF_Manager rf;
    rf.set_offset_ns(40);

    Clovers clovers;

    for (size_t i = 0; i<events; i++)
    {
    // #ifdef DEBUG
      // if (i%(int)(1.E+5) == ba0) print(i/1000000.,"Mevts");
    // #endif //DEBUG
      tree->GetEntry(i);
      // if (event.labels[0] == RF_Manager::label)
      // {
      //   rf.last_hit = event.times[0];
      //   rf.period = event.nrjcals[0];
      //   continue;
      // }
      // for (uint loop = 0; loop<event.size(); loop++) event.time2s[loop] = rf.pulse_ToF_ns(event.times[loop]);
      // print(event);
      ai.FillRaw(event);
      ai.FillClover(clovers, event);
      // int klj;
      // std::cin >> klj;
      // event_s.sortEvent(event);
      // ai.FillSorted(event_s,event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void AnalyseIsomer::InitializeManip()
{
  print("Initialize histograms");

  Ge_spectra.reset("Ge spectra","Ge spectra", 14000,0,7000);
  Ge_spectra_prompt.reset("Ge spectra prompt","Ge spectra prompt;Energy [keV]", 14000,0,7000);
  Ge_spectra_delayed.reset("Ge spectra delayed","Ge spectra delayed;Energy [keV]", 14000,0,7000);
  Ge_Time_VS_Spectra.reset("Ge spectra VS Time","Ge spectra VS Time;Energy [keV];Time [ns]",
      14000,0,7000, 250,-50,200);
  raw_Ge_Time_VS_Spectra.reset("Raw_Ge_spectra_VS_Time","Raw Ge spectra VS Time;Energy [keV];Time [ns]",
      14000,0,7000, 250,-50,200);
  Ge_VS_Ge.reset("Ge_bidim","Ge bidim;Energy [keV];Energy [keV]",
      4096,0,4096, 4096,0,4096);
  GePrompt_VS_GePrompt.reset("Ge_bidim_prompt","Ge bidim prompt;Energy [keV];Energy [keV]",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GeDelayed.reset("Ge_bidim_delayed","Ge bidim delayed;Energy [keV];Energy [keV]",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GeDelayed_time.reset("Ge_bidim_delayed_time","Ge bidim delayed time",
      250,-50,200, 250,-50,200);
  GeDelayed_VS_GePrompt.reset("Ge_delayed_VS_prompt","Ge delayed VS prompt;Energy [keV];Energy [keV]",
      4096,0,4096, 4096,0,4096);
  Ge_VS_DSSD.reset("Ge VS DSSD","Ge VS DSSD;Energy [keV];Energy [keV]",
      400,0,20000, 14000,0,7000);
  GePrompt_VS_DSSD.reset("Ge Prompt VS DSSD","Ge VS DSSD;Energy [keV];Energy [keV]",
      400,0,20000, 14000,0,7000);
  GeDelayed_VS_DSSD.reset("Ge Delayed VS DSSD","Ge VS DSSD;Energy [keV];Energy [keV]",
      400,0,20000, 14000,0,7000);
  DSSD_TW.reset("DSSD E VS Time","DSSD E VS Time",
      250,-50,200, 400,0,20000);
  Ge_VS_DSSD_Time.reset("Ge VS DSSD Time","Ge VS DSSD Time;DSSD time [ns];Ge time [ns]",
      250,-50,200, 250,-50,200);

      
  BGO_R3A1_1_VS_Ge.reset("R3A1_BGO1_vs_Ge","R3A1_BGO1 VS Ge;Energy [keV];Energy [keV]",
      4096,0,8192, 4096,0,8192);
      
  each_Ge_VS_Time.reset("Ge_time","Timing all Ge", 
      24,0,24, 2*USE_RF,-USE_RF/2,3*USE_RF/2);

  // Set analysis parameters :
  Sorted_Event::setDSSDVeto(-10, 50, 5000);
  RF_Manager::set_offset_ns(40);
}

void AnalyseIsomer::FillClover(Clovers & clovers, Event const & event)
{
  clovers.SetEvent(event);
  clovers.Analyse();
  for (uint loop_i = 0; loop_i<clovers.Clean_Ge.size(); loop_i++)
  {
    auto const & clover_i = clovers.m_Clovers[clovers.Clean_Ge[loop_i]];

    auto const & nrj_i   = clover_i.nrj;
    auto const & time_i  = clover_i.time;
    auto const & label_i = clover_i.label();
    auto const prompt_i  = Ge_prompt_gate.isIn(time_i);
    auto const delayed_i = Ge_delayed_gate.isIn(time_i);

    if (prompt_i) Ge_spectra_prompt.Fill(nrj_i);
    if (delayed_i) Ge_spectra_delayed.Fill(nrj_i);
    
    Ge_Time_VS_Spectra.Fill(nrj_i, time_i);
    each_Ge_VS_Time.Fill(label_i, time_i);

    for (uint loop_j = loop_i+1; loop_j<clovers.Clean_Ge.size(); loop_j++)
    {
      auto const & clover_j = clovers.m_Clovers[clovers.Clean_Ge[loop_j]];

      auto const & nrj_j  = clover_j.nrj;
      auto const & time_j = clover_j.time;
      auto const prompt_j  = Ge_prompt_gate.isIn(time_j);
      auto const delayed_j = Ge_delayed_gate.isIn(time_j);

      Ge_VS_Ge.Fill(nrj_i, nrj_j);
      Ge_VS_Ge.Fill(nrj_j, nrj_i);

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
        if (delayed_j)
        {
          GeDelayed_VS_GeDelayed_time . Fill(time_i, time_j);
          GeDelayed_VS_GeDelayed_time . Fill(time_j, time_i);
          GeDelayed_VS_GeDelayed . Fill(nrj_i, nrj_j);
          GeDelayed_VS_GeDelayed . Fill(nrj_j, nrj_i);
        }
        else if (prompt_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
        }
      }
    }
    for (auto const & bgo : clovers.cristaux_BGO)
    {
      auto const & nrj_bgo   = clovers.cristaux_nrj_BGO [bgo];
      auto const & time_bgo  = clovers.cristaux_time_BGO[bgo];
      auto const prompt_bgo  = Ge_prompt_gate.isIn(time_bgo);
      auto const delayed_bgo = Ge_delayed_gate.isIn(time_bgo);
      if (prompt_bgo && prompt_i) BGO_R3A1_1_VS_Ge.Fill(nrj_i, nrj_bgo);
    }
  }
}

void AnalyseIsomer::FillRaw(Event const & event)
{
  for (size_t i = 0; i<event.size(); i++)
  {
    if (isGe[event.labels[i]]) raw_Ge_Time_VS_Spectra.Fill(event.nrjcals[i],event.time2s[i]);
  }
}

void AnalyseIsomer::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  // if (event_s.dssd_veto) return;
  // bool trigger = false;
  // for (auto const & clover_i : event_s.clover_hits)
  // {
  //   auto const & Time_i = event_s.time_clover[clover_i];
  //   auto const & nrj_i  = event_s.nrj_clover[clover_i];
  //   if (Ge_delayed_gate.isIn(Time_i) && nrj_i>93 && nrj_i<99)
  //   {
  //      trigger = true; break;
  //   }
  // }
  // if (!trigger) return;
  for (auto const & dssd : event_s.DSSD_hits)
  {
    auto const & dssd_nrj = event.nrjcals[dssd];
    auto const & dssd_Time = event_s.time2s[dssd];
    DSSD_TW.Fill(dssd_Time, dssd_nrj);
  }
  for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
  {
    auto const & clover_i = event_s.clover_hits[loop_i];

    auto const & nrj_i  = event_s.nrj_clover[clover_i];
    auto const & Time_i = event_s.time_clover[clover_i];
    auto const prompt_i  = Ge_prompt_gate.isIn(Time_i);
    auto const delayed_i = Ge_delayed_gate.isIn(Time_i);

    // Compton suppression and energy threshold :
    if (event_s.BGO[clover_i] || nrj_i<5) continue;

    Ge_Time_VS_Spectra.Fill(nrj_i, Time_i);
    Ge_spectra.Fill(nrj_i);
    if (delayed_i) Ge_spectra_delayed.Fill(nrj_i);
    else if (prompt_i) Ge_spectra_prompt.Fill(nrj_i);

    for (size_t loop_j = loop_i+1; loop_j<event_s.clover_hits.size(); loop_j++)
    {
      auto const & clover_j = event_s.clover_hits[loop_j];

      auto const & nrj_j  = event_s.nrj_clover[clover_j];
      auto const & Time_j = event_s.time_clover[clover_j];
      auto const prompt_j  = Ge_prompt_gate.isIn(Time_j);
      auto const delayed_j = Ge_delayed_gate.isIn(Time_j);

      // Compton suppression and energy threshold :
      if (event_s.BGO[clover_j] || nrj_j<5) continue;

      // DSSD analysis :
      for (auto const & dssd : event_s.DSSD_hits)
      {
        auto const & dssd_nrj = event.nrjcals[dssd];
        auto const & dssd_Time = event_s.time2s[dssd];
        if (event_s.DSSDRingMult == event_s.DSSDSectorMult)
        {
          Ge_VS_DSSD.Fill(dssd_nrj,nrj_i);
          if (prompt_i) GePrompt_VS_DSSD.Fill(dssd_nrj,nrj_i);
          if (delayed_i)GeDelayed_VS_DSSD.Fill(dssd_nrj,nrj_i);
          Ge_VS_DSSD_Time.Fill(dssd_Time, Time_i);
        }
      }
      // Germanium analysis :
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
        if (delayed_j)
        {
          if (abs(Time_i-Time_j)<50/2
          && nrj_i>900 && nrj_i<908
          // && (Time_i<190 || Time_i>220) && (Time_j<190 || Time_j>220)
        )
          {
            GeDelayed_VS_GeDelayed_time . Fill(Time_i, Time_j);
            GeDelayed_VS_GeDelayed_time . Fill(Time_j, Time_i);
            GeDelayed_VS_GeDelayed . Fill(nrj_i, nrj_j);
            GeDelayed_VS_GeDelayed . Fill(nrj_j, nrj_i);
          }
        }
        else if (prompt_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
        }
      }
    }
  }
}

void AnalyseIsomer::Write()
{

  // print("Writting Radware matrixes...");

  // RWMat RW_prompt_prompt(GePrompt_VS_GePrompt); RW_prompt_prompt.Write();
  // RWMat RW_del_del(GeDelayed_VS_GeDelayed); RW_del_del.Write();

  std::unique_ptr<TFile> outfile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  outfile -> cd();

  print("Writting histograms ...");

  Ge_spectra.Write();
  Ge_spectra_delayed.Write();
  Ge_spectra_prompt.Write();

  print("Writting bidims ...");

  Ge_Time_VS_Spectra.Write();
  raw_Ge_Time_VS_Spectra.Write();

  BGO_R3A1_1_VS_Ge.Write();
  Ge_VS_Ge.Write();
  GePrompt_VS_GePrompt.Write();
  GeDelayed_VS_GeDelayed.Write();
  GeDelayed_VS_GePrompt.Write();

  Ge_VS_DSSD.Write();
  GePrompt_VS_DSSD.Write();
  GeDelayed_VS_DSSD.Write();

  each_Ge_VS_Time.Write();

  DSSD_TW.Write();
  Ge_VS_DSSD_Time.Write();
  GeDelayed_VS_GeDelayed_time.Write();

  outfile->Write();
  outfile->Close();

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
      if (temp == "activated") continue;
      else if (temp == "outDir:")  is >> outDir;
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
        else if (temp == "delayed")
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
        else if (temp == "reject")
        {
          is >> temp;
          if (temp == "Ge")
          {
            Ge_reject_gate.use();
            is >> Ge_reject_gate.start >> Ge_reject_gate.stop;
            print("Rejected Ge gate : ", Ge_reject_gate.start, Ge_reject_gate.stop);
          }
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
