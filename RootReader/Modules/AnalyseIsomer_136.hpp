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
#include <DSSD.hpp>

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const &  param);
  void InitializeManip();
  static void run(Parameters & p, AnalyseIsomer & ai);
  void FillRaw(Event const & event);
  void FillSorted(Event const & event, Clovers & clovers, DSSD & dssd);
  // void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();

private:
  std::string param_string = "Isomer";
  // Parameters
  friend class MTObject;
  std::string outDir  = "129/Analyse/Isomer/";
  std::string outRoot = "ai.root";

  double Qvalue = 15000;

  Gate Ge_prompt_gate;
  Gate Ge_delayed_gate;
  Gate Ge_reject_gate;
  Gate LaBr3_prompt_gate;
  Gate LaBr3_delayed_gate; 
  Gate proton_in_DSSD = {3000, 6000};
  Gate proton_in_DSSD_severe = {3000, 5000};
  Gate proton_prompt = {-20, 30};

  // Multiplicity spectra :
  MTTHist<TH2I> Ge_prompt_mult_VS_sectors_mult;
  MTTHist<TH2I> Ge_delayed_mult_VS_sectors_mult;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G1;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G2;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G3;

  // Clovers spectra :
  MTTHist<TH1F> Ge_spectra;
  MTTHist<TH1F> Ge_spectra_prompt;
  MTTHist<TH1F> Ge_spectra_delayed;
  MTTHist<TH1F> Ge_spectra_prompt_proton_gate;
  MTTHist<TH1F> Ge_spectra_delayed_proton_gate;
  MTTHist<TH2F> Ge_VS_Ge;

  MTTHist<TH2F> Ge_Time_VS_Spectra;
  MTTHist<TH2F> raw_Ge_Time_VS_Spectra;

  MTTHist<TH2F> GePrompt_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed_time;
  MTTHist<TH2F> GeDelayed_VS_GePrompt;

  MTTHist<TH2F> GePrompt_VS_GePrompt_proton_gate;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed_proton_gate;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed_time_proton_gate;
  MTTHist<TH2F> GeDelayed_VS_GePrompt_proton_gate;

  MTTHist<TH2F> each_Ge_VS_Time;
  MTTHist<TH2F> each_Ge_spectra;
  MTTHist<TH2F> each_Ge_crystal_spectra;
  MTTHist<TH2F> Ge_VS_size_event;
  MTTHist<TH2F> Ge_VS_Mult;

  // DSSD spectra
  MTTHist<TH1F> DSSD_Spectra;
  MTTHist<TH2F> DSSD_Spectra_VS_angle;
  MTTHist<TH2F> Sector_VS_Rings_mult;
  MTTHist<TH2F> Ge_VS_DSSD;
  MTTHist<TH2F> GePrompt_VS_DSSD;
  MTTHist<TH2F> GeDelayed_VS_DSSD;
  MTTHist<TH2F> DSSD_TW;
  MTTHist<TH2F> Ge_VS_DSSD_Time;

  MTTHist<TH2F> BGO_VS_Ge_511;
  MTTHist<TH2F> BGO_VS_GE_prompt;
  MTTHist<TH2F> BGO_VS_GE_delayed;

// Calorimetry :
  MTTHist<TH1F> Nuball_calo;
  MTTHist<TH1F> Nuball_calo_prompt;
  MTTHist<TH1F> Nuball_calo_delayed;
  MTTHist<TH1F> Ge_calo;
  MTTHist<TH1F> Ge_calo_prompt;
  MTTHist<TH1F> Ge_calo_delayed;
  MTTHist<TH1F> BGO_calo;
  MTTHist<TH1F> BGO_calo_prompt;
  MTTHist<TH1F> BGO_calo_delayed;

  MTTHist<TH2F> Nuball_calo_delayed_VS_prompt;
  MTTHist<TH2F> Clover_Mult_VS_Nuball_calo;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo;
  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo_prompt;
  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo_prompt;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo_delayed;
  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo_delayed;
  MTTHist<TH2F> DSSD_VS_Nuball_calo;
  MTTHist<TH2F> Missing_VS_Delayed_cal;

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
    DSSD dssd;
    dssd.Initialize();

    for (size_t i = 0; i<events; i++)
    {
      // if (event.mult>30) continue;
    // #ifdef DEBUG
      // if (i%(int)(1.E+6) == 0) print(i/1000000.,"Mevts");
    // #endif //DEBUG
      tree->GetEntry(i);
      // if (event.labels[0] == RF_Manager::label)
      // {
      //   rf.last_hit = event.times[0];
      //   rf.period = event.nrjcals[0];
      //   continue;
      // }
      // for (uint loop = 0; loop<event.size(); loop++) event.time2s[loop] = rf.pulse_ToF_ns(event.times[loop]);
      // ai.FillRaw(event);
      dssd.SetEvent(event);
      clovers.SetEvent(event);
      clovers.Analyse();
      ai.FillSorted(event, clovers, dssd);  
      // event_s.sortEvent(event);
      // ai.FillSorted(event_s,event);
    } // End event loop
    // auto const & time = timer();
    print(removePath(rootfile), timer(), timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void AnalyseIsomer::InitializeManip()
{
  print("Initialize histograms");

  Ge_prompt_mult_VS_sectors_mult.reset("Ge_prompt_mult_VS_sectors_mult","Ge prompt mult VS sectors mult", 
      20,0,20, 20,0,20);
  Ge_delayed_mult_VS_sectors_mult.reset("Ge_delayed_mult_VS_sectors_mult","Ge delayed mult VS sectors mult", 
      20,0,20, 20,0,20);
  Delayed_VS_Prompt_Mult.reset("Delayed_VS_Prompt_Mult","Delayed VS Prompt multiplicity;Prompt multiplicity;Delayed Multiplicity", 
      10,0,10, 10,0,10);
  Delayed_VS_Prompt_Mult_G1.reset("Delayed_VS_Prompt_Mult_G1","Delayed VS Prompt multiplicity, Clover mult >= 1;Prompt multiplicity;Delayed Multiplicity", 
      10,0,10, 10,0,10);
  Delayed_VS_Prompt_Mult_G2.reset("Delayed_VS_Prompt_Mult_G2","Delayed VS Prompt multiplicity, Clover mult >=2;Prompt multiplicity;Delayed Multiplicity", 
      10,0,10, 10,0,10);
  Delayed_VS_Prompt_Mult_G3.reset("Delayed_VS_Prompt_Mult_G3","Delayed VS Prompt multiplicity, Clover mult >=3;Prompt multiplicity;Delayed Multiplicity", 
      10,0,10, 10,0,10);

  Ge_spectra.reset("Ge spectra","Ge spectra", 30000,0,15000);
  Ge_spectra_prompt.reset("Ge spectra prompt","Ge spectra prompt;Energy [keV]", 30000,0,15000);
  Ge_spectra_delayed.reset("Ge spectra delayed","Ge spectra delayed;Energy [keV]", 30000,0,15000);

  Ge_spectra_prompt_proton_gate.reset("Ge_spectra_prompt_proton_gate","Ge spectra prompt, proton gate;Energy [keV]", 14000,0,7000);
  Ge_spectra_delayed_proton_gate.reset("Ge_spectra_delayed_proton_gate","Ge spectra delayed, proton gate;Energy [keV]", 14000,0,7000);

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
  GeDelayed_VS_GeDelayed_time.reset("Ge_bidim_delayed_time","Ge bidim delayed time;time [ns];time [ns]",
      250,-50,200, 250,-50,200);
  GeDelayed_VS_GePrompt.reset("Ge_delayed_VS_prompt","Ge delayed VS prompt;Prompt energy [keV];Delayed energy [keV]",
      4096,0,4096, 4096,0,4096);

  // Gate
  GePrompt_VS_GePrompt_proton_gate.reset("GePrompt_VS_GePrompt_proton_gate","Ge prompt VS prompt, DSSD trig;Prompt energy [keV];Prompt energy [keV]",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GeDelayed_proton_gate.reset("GeDelayed_VS_GeDelayed_proton_gate","Ge delayed VS delayed, DSSD trig;Delayed energy [keV];Delayed energy [keV]",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GePrompt_proton_gate.reset("GeDelayed_VS_GePrompt_proton_gate","Ge delayed VS prompt, DSSD trig;Prompt energy [keV];Delayed energy [keV]",
      4096,0,4096, 4096,0,4096);
  GeDelayed_VS_GeDelayed_time_proton_gate.reset("GeDelayed_VS_GeDelayed_time_proton_gate","Ge delayed VS delayed time, DSSD trig;time [ns];time [ns]",
      250,-50,200, 250,-50,200);

  DSSD_Spectra.reset("DSSD_spectra","DSSD spectra;Energy [keV];", 500,0,20000);
  DSSD_Spectra_VS_angle.reset("DSSD_Spectra_VS_angle","DSSD spectra VS Angle;Backward Angle [#circ]; Energy [keV];", 
      40,30,70, 500,0,20000);
  Sector_VS_Rings_mult.reset("Sector_VS_Rings_mult", "Sector VS Ring mult;Rings;Sectors", 10,0,10, 10,0,10);
  Ge_VS_DSSD.reset("Ge_VS_DSSD","Ge VS DSSD;Energy DSSD [keV];Energy Ge [keV]",
      400,0,20000, 14000,0,7000);
  GePrompt_VS_DSSD.reset("GePrompt_VS_DSSD","Ge VS DSSD",
      400,0,20000, 14000,0,7000);
  GeDelayed_VS_DSSD.reset("GeDelayed_VS_DSSD","Ge VS DSSD",
      400,0,20000, 14000,0,7000);
  DSSD_TW.reset("DSSD_TW","DSSD E VS Time",
      250,-50,200, 400,0,20000);
  Ge_VS_DSSD_Time.reset("Ge_VS_DSSD_Time","Ge VS DSSD Time;DSSD time [ns];Ge time [ns]",
      250,-50,200, 250,-50,200);

  BGO_VS_GE_prompt.reset("BGO_VS_GE_prompt","BGO VS Ge prompt;Ge Energy [keV];BGO Energy [keV]",
      4096,0,4096, 800,0,4096);
  BGO_VS_GE_delayed.reset("BGO_VS_GE_delayed","BGO VS Ge delayed;Ge Energy [keV];BGO Energy [keV]",
      4096,0,4096, 800,0,4096);
  BGO_VS_Ge_511.reset("BGO_vs_Ge_511","BGO in coincidence with 511;BGO energy [keV];",
      48,0,48, 4096,0,100000);

  Nuball_calo.reset("Nuball_calo","Clovers calorimetry;Calorimetry [keV]", 4096,0,20000);
  Nuball_calo_prompt.reset("Nuball_calo_prompt","Clovers prompt calorimetry;Calorimetry [keV]", 4096,0,20000);
  Nuball_calo_delayed.reset("Nuball_calo_delayed","Clovers delayed calorimetry;Calorimetry [keV]", 4096,0,20000);
  Ge_calo.reset("Ge_calo","Ge calorimetry;Calorimetry [keV]", 4096,0,20000);
  Ge_calo_prompt.reset("Ge_calo_prompt","Ge prompt calorimetry;Calorimetry [keV]", 4096,0,20000);
  Ge_calo_delayed.reset("Ge_calo_delayed","Ge delayed calorimetry;Calorimetry [keV]", 4096,0,20000);
  BGO_calo.reset("BGO_calo","BGO calorimetry;Calorimetry [keV]", 4096,0,20000);
  BGO_calo_prompt.reset("BGO_calo_prompt","BGO prompt calorimetry;Calorimetry [keV]", 4096,0,20000);
  BGO_calo_delayed.reset("BGO_calo_delayed","BGO delayed calorimetry;Calorimetry [keV]", 4096,0,20000);

  Ge_prompt_VS_Nuball_calo.reset("Ge_prompt_VS_Nuball_calo","Ge prompt spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Ge_delayed_VS_Nuball_calo.reset("Ge_delayed_VS_Nuball_calo","Ge delayed spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Ge_prompt_VS_Nuball_calo_prompt.reset("Ge_prompt_VS_Nuball_calo_prompt","Ge prompt spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Ge_delayed_VS_Nuball_calo_prompt.reset("Ge_delayed_VS_Nuball_calo_prompt","Ge delayed spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Ge_prompt_VS_Nuball_calo_delayed.reset("Ge_prompt_VS_Nuball_calo_delayed","Ge prompt spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Ge_delayed_VS_Nuball_calo_delayed.reset("Ge_delayed_VS_Nuball_calo_delayed","Ge delayed spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
      4096,0,30000, 4096,0,4096);
  Nuball_calo_delayed_VS_prompt.reset("Nuball_calo_delayed_VS_prompt","Clovers delayed calorimetry VS prompt calorimetry;Prompt Calorimetry [keV];Delayed Calorimetry [keV]",
      4096,0,30000, 4096,0,30000);

  DSSD_VS_Nuball_calo.reset("DSSD_VS_Nuball_calo","Ge prompt spectra VS clovers calorimetry;Calorimetry [keV];Particle Energy [keV]",
      2000,0,20000, 3000,0,30000);
  Missing_VS_Delayed_cal.reset("Missing_VS_Delayed_cal","Missing energy VS delayed calorimetry;Delayed Calorimetry [keV];Missing Energy [keV]",
      2000,0,20000, 300,0,15000);
  Clover_Mult_VS_Nuball_calo.reset("Clover_Mult_VS_Nuball_calo","Multiplicity Clover VS clovers calorimetry;Calorimetry [keV];Multiplicity",
      4096,0,20000, 20,0,20);
      
  each_Ge_VS_Time.reset("each_Ge_VS_Time","Timing each Ge", 
      24,0,24, 2*USE_RF,-USE_RF/2,3*USE_RF/2);

  each_Ge_spectra.reset("each_Ge_spectra","Spectra each Ge", 
      24,0,24, 20000,0,10000);

  each_Ge_crystal_spectra.reset("each_Ge_crystal_spectra","Spectra each Ge crystal", 
      96,0,96, 30000,0,30000);

  Ge_VS_size_event.reset("Ge_VS_size_event","Ge VS number of detectors", 
      50,0,50, 5000,0,5000);
  Ge_VS_Mult.reset("Ge_VS_Mult","Ge VS Clean Ge Mult", 
      30,0,30, 5000,0,5000);

  // Set analysis parameters :
  Sorted_Event::setDSSDVeto(-10, 50, 5000);
  RF_Manager::set_offset_ns(40);
}

void AnalyseIsomer::FillSorted(Event const & event, Clovers & clovers, DSSD & dssd)
{
  if (clovers.PromptMult<1 && clovers.DelayedMult<2) return;
  for (auto const & crystal : clovers.cristaux) each_Ge_crystal_spectra.Fill(crystal, clovers.cristaux_nrj[crystal]);

  Delayed_VS_Prompt_Mult.Fill(clovers.PromptMult, clovers.DelayedMult);
  // if (clovers.Mult>0)Delayed_VS_Prompt_Mult_G1.Fill(clovers.PromptMult, clovers.DelayedMult);
  // if (clovers.Mult>1)Delayed_VS_Prompt_Mult_G2.Fill(clovers.PromptMult, clovers.DelayedMult);
  // if (clovers.Mult>2)Delayed_VS_Prompt_Mult_G3.Fill(clovers.PromptMult, clovers.DelayedMult);
  bool DSSD_prompt_proton_gate = (dssd.SectorMult>0 && proton_in_DSSD.isIn(dssd.energy()) && proton_prompt.isIn(dssd.time()));

  // if (dssd.SectorMult>0) printMT(dssd.oneParticle(), dssd.energy(), proton_in_DSSD.isIn(dssd.energy()), dssd.time(), proton_prompt.isIn(dssd.time()));
  
  auto const calo_clovers = clovers.totGe+clovers.totBGO;
  auto const & calo_prompt = clovers.totGe_prompt+clovers.totBGO_prompt;
  auto const & calo_delayed = clovers.totGe_delayed+clovers.totBGO_delayed;

  Ge_prompt_mult_VS_sectors_mult.Fill(dssd.SectorMult, clovers.PromptMult);
  Ge_delayed_mult_VS_sectors_mult.Fill(dssd.SectorMult, clovers.DelayedMult);

// Treat the event as a whole :
  Nuball_calo.Fill(calo_clovers);
  Nuball_calo_prompt.Fill(calo_prompt);
  Nuball_calo_delayed.Fill(calo_delayed);
  
  Nuball_calo_delayed_VS_prompt.Fill(calo_prompt, calo_delayed);
  Clover_Mult_VS_Nuball_calo.Fill(calo_clovers, clovers.Mult);

  Ge_calo.Fill(clovers.totGe);
  Ge_calo_prompt.Fill(clovers.totGe_prompt);
  Ge_calo_delayed.Fill(clovers.totGe_delayed);

  BGO_calo.Fill(clovers.totBGO);
  BGO_calo_prompt.Fill(clovers.totBGO_prompt);
  BGO_calo_delayed.Fill(clovers.totBGO_delayed);

  for (uint loop_i = 0; loop_i<clovers.Clean_Ge.size(); loop_i++)
  {
    auto const & clover_i = clovers.m_Clovers[clovers.Clean_Ge[loop_i]];

    auto const & nrj_i   = clover_i.nrj;
    auto const & time_i  = clover_i.time;
    auto const & label_i = clover_i.label();
    auto const prompt_i  = clovers.isPrompt (clover_i);
    // auto const delayed_i = clovers.isDelayed(clover_i);
    // auto const prompt_i  = time_i>-20 && time_i<10;
    auto const delayed_i = time_i>60 && time_i<145;

    if (prompt_i) 
    {
      Ge_spectra_prompt.Fill(nrj_i);
      if (DSSD_prompt_proton_gate) Ge_spectra_prompt_proton_gate.Fill(nrj_i);
    }
    if (delayed_i) 
    {
      Ge_spectra_delayed.Fill(nrj_i);
      if (DSSD_prompt_proton_gate) Ge_spectra_delayed_proton_gate.Fill(nrj_i);
    }
    
    Ge_Time_VS_Spectra.Fill(nrj_i, time_i);
    each_Ge_VS_Time.Fill(label_i, time_i);
    each_Ge_spectra.Fill(label_i, nrj_i);
    Ge_VS_size_event.Fill(event.mult, nrj_i);
    Ge_VS_Mult.Fill(clovers.CleanGeMult, nrj_i);

    // Ge bidim : 
    for (uint loop_j = loop_i+1; loop_j<clovers.Clean_Ge.size(); loop_j++)
    {
      auto const & clover_j = clovers.m_Clovers[clovers.Clean_Ge[loop_j]];

      auto const & time_j = clover_j.time;
      auto const & nrj_j  = clover_j.nrj;
      // auto const prompt_j  = time_j>-20 && time_j<10;
      auto const delayed_j = time_j>60 && time_j<145;
      auto const prompt_j  = clovers.isPrompt (clover_j);
    // auto const delayed_j = clovers.isDelayed(clover_j);

      if (nrj_i > 507 && nrj_i<515 && nrj_j > 507 && nrj_j<515) continue;

      Ge_VS_Ge.Fill(nrj_i, nrj_j);
      Ge_VS_Ge.Fill(nrj_j, nrj_i);

      if (prompt_i)
      {
        // I do (calo_clovers-nrj_i) so that the plot is not twisted, because calo_clovers>=nrj_i necesseraly
        Ge_prompt_VS_Nuball_calo.Fill(calo_clovers-nrj_i, nrj_i);
        Ge_prompt_VS_Nuball_calo_prompt.Fill(calo_prompt-nrj_i, nrj_i);
        Ge_prompt_VS_Nuball_calo_delayed.Fill(calo_delayed-nrj_i, nrj_i);

        if (prompt_j)
        {
          GePrompt_VS_GePrompt.Fill(nrj_i,nrj_j);
          GePrompt_VS_GePrompt.Fill(nrj_j,nrj_i);
          if (DSSD_prompt_proton_gate)
          {
            GePrompt_VS_GePrompt_proton_gate.Fill(nrj_i,nrj_j);
            GePrompt_VS_GePrompt_proton_gate.Fill(nrj_j,nrj_i);
          }
        }
        else if (delayed_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_i,nrj_j);
          if (DSSD_prompt_proton_gate) GeDelayed_VS_GePrompt_proton_gate.Fill(nrj_i,nrj_j);
        }
      }
      else if (delayed_i)
      {
        Ge_delayed_VS_Nuball_calo.Fill(calo_clovers-nrj_i, nrj_i);
        Ge_delayed_VS_Nuball_calo_prompt.Fill(calo_prompt-nrj_i, nrj_i);
        Ge_delayed_VS_Nuball_calo_delayed.Fill(calo_delayed-nrj_i, nrj_i);
        
        if (delayed_j)
        {
          GeDelayed_VS_GeDelayed_time.Fill(time_i, time_j);
          GeDelayed_VS_GeDelayed_time.Fill(time_j, time_i);
          GeDelayed_VS_GeDelayed.Fill(nrj_i, nrj_j);
          GeDelayed_VS_GeDelayed.Fill(nrj_j, nrj_i);
          
          if (DSSD_prompt_proton_gate)
          {
            GeDelayed_VS_GeDelayed_time_proton_gate.Fill(time_i, time_j);
            GeDelayed_VS_GeDelayed_time_proton_gate.Fill(time_j, time_i);
            GeDelayed_VS_GeDelayed_proton_gate.Fill(nrj_i, nrj_j);
            GeDelayed_VS_GeDelayed_proton_gate.Fill(nrj_j, nrj_i);
          }
        }
        else if (prompt_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
          if (DSSD_prompt_proton_gate) GeDelayed_VS_GePrompt_proton_gate.Fill(nrj_j,nrj_i);
        }
      }
    }

    // BGO bidims :
    for (auto const & bgo : clovers.cristaux_BGO)
    {
      auto const & time_bgo  = clovers.cristaux_time_BGO[bgo];
      auto const & nrj_bgo   = clovers.cristaux_nrj_BGO [bgo];
      auto const prompt_bgo  = Ge_prompt_gate.isIn(time_bgo);
      auto const delayed_bgo = Ge_delayed_gate.isIn(time_bgo);
      if (prompt_bgo && prompt_i)
      {
        BGO_VS_GE_prompt.Fill(nrj_i, nrj_bgo);
        if (nrj_i > 507 && nrj_i<515) BGO_VS_Ge_511.Fill(bgo, nrj_bgo);
      } 
      else if (delayed_i && delayed_bgo) BGO_VS_GE_delayed.Fill(nrj_i, nrj_bgo);
    }

    Sector_VS_Rings_mult.Fill(dssd.RingMult, dssd.SectorMult);
    // dssd bidim :
    // if (dssd.oneParticle())
    if (dssd.energy()>1000 && dssd.energy()<9000)
    {
      auto const & nrj_dssd = dssd.energy();
      auto const & time_dssd = dssd.time();
      auto const angle = dssd.angle()/3.141596*180+(ring_deg_thick*gRandom->Uniform(-0.5,0.5));

      DSSD_Spectra.Fill(nrj_dssd);
      DSSD_Spectra_VS_angle.Fill(angle, nrj_dssd);
      Ge_VS_DSSD.Fill(nrj_dssd, nrj_i);
      if (prompt_i) GePrompt_VS_DSSD.Fill(nrj_dssd, nrj_i);
      if (delayed_i) GeDelayed_VS_DSSD.Fill(nrj_dssd, nrj_i);
      DSSD_TW.Fill(time_dssd, nrj_dssd);
      Ge_VS_DSSD_Time.Fill(time_dssd, time_i);
      DSSD_VS_Nuball_calo.Fill(calo_clovers, nrj_dssd);

      auto const Missing_E = Qvalue-calo_prompt-dssd.energy();
      Missing_VS_Delayed_cal.Fill(calo_delayed, Missing_E);
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

void AnalyseIsomer::Write()
{
  print("Writting Radware matrixes...");
  // RWMat RW_prompt_prompt(GePrompt_VS_GePrompt); RW_prompt_prompt.Write();
  // RWMat RW_del_del(GeDelayed_VS_GeDelayed); RW_del_del.Write();

  // RWMat RW_prompt_prompt_proton_gate(GePrompt_VS_GePrompt_proton_gate); RW_prompt_prompt_proton_gate.Write();
  // RWMat RW_del_del_proton_gate(GeDelayed_VS_GeDelayed_proton_gate); RW_del_del_proton_gate.Write();
  // RWMat RW_del_prompt_proton_gate(GeDelayed_VS_GePrompt_proton_gate); RW_del_prompt_proton_gate.Write();

  std::unique_ptr<TFile> outfile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  outfile -> cd();

  print("Writting histograms ...");

  Ge_prompt_mult_VS_sectors_mult.Write();
  Ge_delayed_mult_VS_sectors_mult.Write();
  Delayed_VS_Prompt_Mult.Write();
  Delayed_VS_Prompt_Mult_G1.Write();
  Delayed_VS_Prompt_Mult_G2.Write();
  Delayed_VS_Prompt_Mult_G3.Write();

  Ge_spectra.Write();
  Ge_spectra_delayed.Write();
  Ge_spectra_prompt.Write();
  Ge_spectra_prompt_proton_gate.Write();
  Ge_spectra_prompt_proton_gate.Write();

  Ge_Time_VS_Spectra.Write();
  raw_Ge_Time_VS_Spectra.Write();
  
  Ge_VS_Ge.Write();
  GePrompt_VS_GePrompt.Write();
  GeDelayed_VS_GeDelayed.Write();
  GeDelayed_VS_GePrompt.Write();

  GePrompt_VS_GePrompt_proton_gate.Write();
  GeDelayed_VS_GeDelayed_proton_gate.Write();
  GeDelayed_VS_GePrompt_proton_gate.Write();
  GeDelayed_VS_GeDelayed_time_proton_gate.Write();

  BGO_VS_Ge_511.Write();
  BGO_VS_GE_prompt.Write();
  BGO_VS_GE_delayed.Write();

  Nuball_calo.Write();
  Nuball_calo_prompt.Write();
  Nuball_calo_delayed.Write();
  Ge_calo.Write();
  Ge_calo_prompt.Write();
  Ge_calo_delayed.Write();
  BGO_calo.Write();
  BGO_calo_prompt.Write();
  BGO_calo_delayed.Write();
  Nuball_calo_delayed_VS_prompt.Write();
  Clover_Mult_VS_Nuball_calo.Write();
  Ge_prompt_VS_Nuball_calo.Write();
  Ge_delayed_VS_Nuball_calo.Write();
  Ge_prompt_VS_Nuball_calo_prompt.Write();
  Ge_delayed_VS_Nuball_calo_prompt.Write();
  Ge_prompt_VS_Nuball_calo_delayed.Write();
  Ge_delayed_VS_Nuball_calo_delayed.Write();
  DSSD_VS_Nuball_calo.Write();
  Missing_VS_Delayed_cal.Write();

  DSSD_Spectra.Write();
  DSSD_Spectra_VS_angle.Write();
  Sector_VS_Rings_mult.Write();
  Ge_VS_DSSD.Write();
  GePrompt_VS_DSSD.Write();
  GeDelayed_VS_DSSD.Write();
  DSSD_TW.Write();
  Ge_VS_DSSD_Time.Write();

  each_Ge_VS_Time.Write();
  each_Ge_spectra.Write();
  each_Ge_crystal_spectra.Write();
  Ge_VS_size_event.Write();
  Ge_VS_Mult.Write();

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

// void AnalyseIsomer::FillSorted(Sorted_Event const & event_s, Event const & event)
// {
//   // if (event_s.dssd_veto) return;
//   // bool trigger = false;
//   // for (auto const & clover_i : event_s.clover_hits)
//   // {
//   //   auto const & Time_i = event_s.time_clover[clover_i];
//   //   auto const & nrj_i  = event_s.nrj_clover[clover_i];
//   //   if (Ge_delayed_gate.isIn(Time_i) && nrj_i>93 && nrj_i<99)
//   //   {
//   //      trigger = true; break;
//   //   }
//   // }
//   // if (!trigger) return;
//   for (auto const & dssd : event_s.DSSD_hits)
//   {
//     auto const & dssd_nrj = event.nrjcals[dssd];
//     auto const & dssd_Time = event_s.time2s[dssd];
//     DSSD_TW.Fill(dssd_Time, dssd_nrj);
//   }
//   for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//   {
//     auto const & clover_i = event_s.clover_hits[loop_i];

//     auto const & nrj_i  = event_s.nrj_clover[clover_i];
//     auto const & Time_i = event_s.time_clover[clover_i];
//     auto const prompt_i  = Ge_prompt_gate.isIn(Time_i);
//     auto const delayed_i = Ge_delayed_gate.isIn(Time_i);

//     // Compton suppression and energy threshold :
//     if (event_s.BGO[clover_i] || nrj_i<5) continue;

//     Ge_Time_VS_Spectra.Fill(nrj_i, Time_i);
//     Ge_spectra.Fill(nrj_i);
//     if (delayed_i) Ge_spectra_delayed.Fill(nrj_i);
//     else if (prompt_i) Ge_spectra_prompt.Fill(nrj_i);

//     for (size_t loop_j = loop_i+1; loop_j<event_s.clover_hits.size(); loop_j++)
//     {
//       auto const & clover_j = event_s.clover_hits[loop_j];

//       auto const & nrj_j  = event_s.nrj_clover[clover_j];
//       auto const & Time_j = event_s.time_clover[clover_j];
//       auto const prompt_j  = Ge_prompt_gate.isIn(Time_j);
//       auto const delayed_j = Ge_delayed_gate.isIn(Time_j);

//       // Compton suppression and energy threshold :
//       if (event_s.BGO[clover_j] || nrj_j<5) continue;

//       // DSSD analysis :
//       for (auto const & dssd : event_s.DSSD_hits)
//       {
//         auto const & dssd_nrj = event.nrjcals[dssd];
//         auto const & dssd_Time = event_s.time2s[dssd];
//         if (event_s.DSSDRingMult == event_s.DSSDSectorMult)
//         {
//           Ge_VS_DSSD.Fill(dssd_nrj,nrj_i);
//           if (prompt_i) GePrompt_VS_DSSD.Fill(dssd_nrj,nrj_i);
//           if (delayed_i)GeDelayed_VS_DSSD.Fill(dssd_nrj,nrj_i);
//           Ge_VS_DSSD_Time.Fill(dssd_Time, Time_i);
//         }
//       }
//       // Germanium analysis :
//       if (prompt_i)
//       {

//         if (prompt_j)
//         {
//           GePrompt_VS_GePrompt . Fill(nrj_i,nrj_j);
//           GePrompt_VS_GePrompt . Fill(nrj_j,nrj_i);
//         }
//         else if (delayed_j)
//         {
//           GeDelayed_VS_GePrompt.Fill(nrj_i,nrj_j);
//         }
//       }
//       else if (delayed_i)
//       {
//         if (delayed_j)
//         {
//           if (abs(Time_i-Time_j)<50/2
//           && nrj_i>900 && nrj_i<908
//           // && (Time_i<190 || Time_i>220) && (Time_j<190 || Time_j>220)
//         )
//           {
//             GeDelayed_VS_GeDelayed_time . Fill(Time_i, Time_j);
//             GeDelayed_VS_GeDelayed_time . Fill(Time_j, Time_i);
//             GeDelayed_VS_GeDelayed . Fill(nrj_i, nrj_j);
//             GeDelayed_VS_GeDelayed . Fill(nrj_j, nrj_i);
//           }
//         }
//         else if (prompt_j)
//         {
//           GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
//         }
//       }
//     }
//   }
// }

#endif //ANALYSEISOMER_H
