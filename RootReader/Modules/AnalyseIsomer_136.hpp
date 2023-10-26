#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

#include <libCo.hpp>
#include <Nuball2Tree.hpp>
#include <MTObject.hpp>

#include "../../lib/MTObjects/MTTHist.hpp"

#include "../../lib/Analyse/HistoAnalyse.hpp"


#include "../../lib/Classes/Gate.hpp"
#include "../../lib/Classes/RWMat.hxx"

#include "../Classes/Parameters.hpp"

#include <RF_Manager.hpp>

#include <Clovers.hpp>
#include <DSSD.hpp>
// #include <Paris.hpp>

// #include <MTVariable.hpp>
// MTVariable<int> variable;


class ParisNul
{
public:
  ParisNul()
  {
    indexes.resize(255);
    labr3_hits.resize(255);
    prompt.resize(255);
    delayed.resize(255);
  }

  void Reset()
  {
    PromptMult = 0;
    DelayedMult = 0;
    PromptCalo = 0;
    DelayedCalo = 0;
    indexes.clear();
    labr3_hits.clear();
    prompt.clear();
    delayed.clear();
    rejection = false;
  }

  bool isFront(Label const & label) {return label>700 && label<800;}

  void SetEvent(Event const & event)
  {
    Reset();
    for (int hit_i =0; hit_i<event.mult; hit_i++) this -> Fill(event, hit_i);
  }

  void Fill(Event const & event, int const & hit_i)
  {
    auto const & label = event.labels[hit_i];
    if (!isParis[label]) return;
    auto const & time = event.time2s[hit_i];
    auto const & nrj = event.nrjs[hit_i];
    auto const & nrj2 = event.nrj2s[hit_i];
    auto const & ratio = (nrj2-nrj)/nrj2;
    auto const & isLaBr3 = ratio>-0.1 && ratio < 0.2;
    // auto const & isNaI = ratio>0.55 && ratio < 0.7;

    if (time<-10) return;
    indexes.push_back(hit_i);
    if (time<5)
    {
      prompt.emplace_back(hit_i);
      PromptMult++;
      if (isLaBr3) 
      {
        labr3_hits.push_back(label);
        PromptCalo+=nrj;
      }
    }
    else if (time<50) 
    {
      rejection = true;
    }
    else if (time<145)
    {
      delayed.emplace_back(hit_i);
      DelayedMult++;
      if (isLaBr3) 
      {
        labr3_hits.push_back(label);
        DelayedCalo+=nrj;
      }
    }
  }

  std::vector<int> indexes;
  std::vector<int> labr3_hits;
  std::vector<int> prompt;
  std::vector<int> delayed;
  int PromptMult = 0;
  int DelayedMult = 0;
  float PromptCalo = 0;
  float DelayedCalo = 0;
  bool rejection = false;

};

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const &  param);
  void InitializeManip();
  static void run(Parameters & p, AnalyseIsomer & ai);
  void FillRaw(Event const & event);
  void FillSorted(Event const & event, Clovers & clovers, DSSD & dssd, ParisNul & paris);
  void Write();

  MTTHist<TH2F> time_ref;

  static void choose1(bool const & b = true) {m_choose1 = b;}

private:
  std::string m_param_string = "Isomer";
  // Parameters
  friend class MTObject;
  std::string m_outDir  = "Analyse/Isomer/";
  std::string m_outRoot = "ai.root";

  static bool m_choose1;

  double Qvalue = 4322;
  double Ebeam = 11000;
  double Qdispo = Qvalue+Ebeam;
  bool m_writeRadware = false;
  bool m_trigger1989 = false;

  Gate Ge_prompt_gate;
  Gate Ge_delayed_gate;
  Gate Ge_reject_gate;
  Gate LaBr3_prompt_gate;
  Gate LaBr3_delayed_gate; 
  Gate proton_in_DSSD = {3000, 6000};
  Gate proton_in_DSSD_severe = {3000, 5000};
  Gate proton_prompt = {-20, 30};

  // Multiplicity spectra :
  MTTHist<TH2I> prompt_mult_VS_sectors_mult;
  MTTHist<TH2I> delayed_mult_VS_sectors_mult;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G1;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G2;
  MTTHist<TH2F> Delayed_VS_Prompt_Mult_G3;

  // Clovers spectra :
  MTTHist<TH1F> Ge_spectra;
  MTTHist<TH1F> Ge_spectra_prompt;
  MTTHist<TH1F> Ge_spectra_delayed;
  MTTHist<TH2F> Ge_VS_Ge;

  MTTHist<TH2F> Ge_Time_VS_Spectra;
  MTTHist<TH2F> raw_Ge_Time_VS_Spectra;

  MTTHist<TH2F> GePrompt_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed_time;
  MTTHist<TH2F> GeDelayed_VS_GePrompt;


  MTTHist<TH2F> each_Ge_VS_Time;
  MTTHist<TH2F> each_Ge_spectra;
  MTTHist<TH2F> each_Ge_crystal_spectra;
  MTTHist<TH2F> Ge_VS_size_event;

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
  MTTHist<TH2F> BGO_VS_Ge_prompt;
  MTTHist<TH2F> BGO_prompt_VS_Ge_prompt_mult;
  MTTHist<TH2F> BGO_VS_Ge_delayed;

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
  MTTHist<TH1F> Paris_calo;
  MTTHist<TH1F> Paris_calo_prompt;
  MTTHist<TH1F> Paris_calo_delayed;
  MTTHist<TH1F> total_calo;
  MTTHist<TH1F> total_calo_prompt;
  MTTHist<TH1F> total_calo_delayed;

  MTTHist<TH2F> calo_prompt_total_VS_calo_total;
  MTTHist<TH2F> calo_delayed_total_VS_calo_total;
  MTTHist<TH2F> delayed_calo_total_VS_prompt_calo_total;

  MTTHist<TH2F> Nuball_calo_delayed_VS_prompt;
  MTTHist<TH2F> Clover_Mult_VS_Nuball_calo;
  MTTHist<TH2F> prompt_Clover_Mult_VS_Nuball_calo;
  MTTHist<TH2F> delayed_Clover_Mult_VS_Nuball_calo;
  MTTHist<TH2F> Clover_Mult_VS_Total_calo;
  MTTHist<TH2F> prompt_Clover_Mult_VS_Total_calo;
  MTTHist<TH2F> delayed_Clover_Mult_VS_Total_calo;

  MTTHist<TH2F> DSSD_VS_Nuball_calo;
  MTTHist<TH2F> DSSD_VS_Total_calo;
  MTTHist<TH2F> DSSD_VS_Total_Prompt_calo;
  MTTHist<TH2F> DSSD_VS_Total_Delayed_calo;

  MTTHist<TH2F> calo_VS_promptMult;
  MTTHist<TH2F> calo_VS_delayedMult;
  MTTHist<TH2F> prompt_calo_VS_promptMult;
  MTTHist<TH2F> prompt_calo_VS_delayedMult;
  MTTHist<TH2F> delayed_calo_VS_promptMult;
  MTTHist<TH2F> delayed_calo_VS_delayedMult;

  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo;
  MTTHist<TH2F> Ge_delayed_VS_Total_calo;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo;
  MTTHist<TH2F> Ge_prompt_VS_Total_calo;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo_prompt;
  MTTHist<TH2F> Ge_prompt_VS_Total_calo_prompt;
  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo_prompt;
  MTTHist<TH2F> Ge_delayed_VS_Total_calo_prompt;
  MTTHist<TH2F> Ge_prompt_VS_Nuball_calo_delayed;
  MTTHist<TH2F> Ge_prompt_VS_Total_calo_delayed;
  MTTHist<TH2F> Ge_delayed_VS_Nuball_calo_delayed;
  MTTHist<TH2F> Ge_delayed_VS_Total_calo_delayed;


  MTTHist<TH2F> Delayed_Ge_VS_Missing;
  MTTHist<TH2F> Prompt_Ge_VS_Missing;
  MTTHist<TH2F> Prompt_Calo_VS_Missing;
  MTTHist<TH2F> Missing_VS_Delayed_cal;


  MTTHist<TH2F> ge_spectra_VS_mult;
  MTTHist<TH2F> ge_spectra_VSpromptmult;
  MTTHist<TH2F> ge_spectra_VSdelayedmult;
  MTTHist<TH2F> ge_prompt_spectra_VS_mult;
  MTTHist<TH2F> ge_prompt_spectra_VSpromptmult;
  MTTHist<TH2F> ge_prompt_spectra_VSdelayedmult;
  MTTHist<TH2F> ge_delayed_spectra_VS_mult;
  MTTHist<TH2F> ge_delayed_spectra_VSpromptmult;
  MTTHist<TH2F> ge_delayed_spectra_VSdelayedmult;

  MTTHist<TH1F> Paris_spectra_back;
  MTTHist<TH2F> Paris_time_spectra_back;
  MTTHist<TH2F> Paris_each_spectra_back;
  MTTHist<TH2F> Paris_ratio_VS_time_back;
  MTTHist<TH1F> Paris_spectra_front;
  MTTHist<TH2F> Paris_time_spectra_front;
  MTTHist<TH2F> Paris_each_spectra_front;
  MTTHist<TH2F> Paris_ratio_VS_time_front;

  MTTHist<TH1F> Paris_back_calibrated_VS_delayed_U6;

  Vector_MTTHist<TH1F> Paris_singles_labr;
  Vector_MTTHist<TH1F> Paris_singles_nai;

  Vector_MTTHist<TH2F> DSSD_bidims;
  Vector_MTTHist<TH2F> DSSD_projTot_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj169_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj510_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj642_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj687_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj870_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj880_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj925_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj1014_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj2210_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj3000_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj3682_VS_Rings;
  Vector_MTTHist<TH2F> DSSD_proj3850_VS_Rings;

  // MTTHist<TH2F> mult_VS_Time; In order to see the evolution of Multiplicity over time. To do it, take a moving 50ns time window to group events

};

bool AnalyseIsomer::m_choose1 = false;

bool AnalyseIsomer::launch(Parameters & p)
{
  Timer totalTimer;
  if (!this -> setParameters(p.getParameters(m_param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  Timer runTimer;
  MTObject::parallelise_function(run, p, *this);
  print(runTimer(), runTimer.unit(), "to treat data");
  Timer writeTimer;
  this -> Write();
  print(writeTimer(), writeTimer.unit(), "to write data");
  print(totalTimer(), totalTimer.unit(), "to analyse data");
  return true;
}

void AnalyseIsomer::run(Parameters & p, AnalyseIsomer & ai)
{
  std::string rootfile;
  // Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    Nuball2Tree tree(rootfile);

    if (!tree) continue;

    Event event(tree);
    
    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    RF_Manager rf;
    rf.set_offset_ns(40);

    Clovers clovers;
    ParisNul paris;
    DSSD dssd;

    for (size_t event_i = 0; event_i<events; event_i++)
    {
      // if (event.mult>30) continue;
    // #ifdef DEBUG
      // if (event_i%(int)(1.E+3) == 0) print(event_i/1000000.,"Mevts");
    // #endif //DEBUG
      tree->GetEntry(event_i);
      dssd.Reset();
      clovers.Reset();
      paris.Reset();
      for (int hit_i = 0; hit_i<event.mult; hit_i++)
      {
        dssd.Fill(event, hit_i);
        clovers.Fill(event, hit_i);
        paris.Fill(event, hit_i);
        if (!m_choose1) if (event.labels[hit_i] == 252) ai.time_ref.Fill(event.nrjs[hit_i], event.time2s[hit_i]);
      }
      // if (event.labels[0] == RF_Manager::label)
      // {
      //   rf.last_hit = event.times[0];
      //   rf.period = event.nrjcals[0];
      //   continue;
      // }
      // for (uint loop = 0; loop<event.size(); loop++) event.time2s[loop] = rf.pulse_ToF_ns(event.times[loop]);
      // ai.FillRaw(event);
      clovers.Analyse();
      ai.FillSorted(event, clovers, dssd, paris);
      // event_s.sortEvent(event);
      // ai.FillSorted(event_s,event);
      // file->Close();
    } // End event loop
    // auto const & time = timer();
    print(removePath(rootfile), timer(), timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}


void AnalyseIsomer::InitializeManip()
{
  print("Initialize histograms");

  if(!m_choose1)
  {
    time_ref.reset("time_ref", "Reference LaBR3 time spectra;Energy[keV];Time[ns]", 3000,0,3000, 1001,-100,400);

    prompt_mult_VS_sectors_mult.reset("prompt_mult_VS_sectors_mult","prompt mult VS sectors mult", 
        20,0,20, 20,0,20);
    delayed_mult_VS_sectors_mult.reset("delayed_mult_VS_sectors_mult","delayed mult VS sectors mult", 
        20,0,20, 20,0,20);
    Delayed_VS_Prompt_Mult.reset("Delayed_VS_Prompt_Mult","Delayed VS Prompt multiplicity;Prompt multiplicity;Delayed Multiplicity", 
        10,0,10, 10,0,10);
    Delayed_VS_Prompt_Mult_G1.reset("Delayed_VS_Prompt_Mult_G1","Delayed VS Prompt multiplicity, Clover mult >= 1;Prompt multiplicity;Delayed Multiplicity", 
        10,0,10, 10,0,10);
    Delayed_VS_Prompt_Mult_G2.reset("Delayed_VS_Prompt_Mult_G2","Delayed VS Prompt multiplicity, Clover mult >=2;Prompt multiplicity;Delayed Multiplicity", 
        10,0,10, 10,0,10);
    Delayed_VS_Prompt_Mult_G3.reset("Delayed_VS_Prompt_Mult_G3","Delayed VS Prompt multiplicity, Clover mult >=3;Prompt multiplicity;Delayed Multiplicity", 
        10,0,10, 10,0,10);

    Ge_spectra.reset("Ge_spectra","Ge spectra;Energy [keV]", 30000,0,15000);
    Ge_spectra_prompt.reset("Ge spectra prompt","Ge spectra prompt;Energy [keV]", 30000,0,15000);
    Ge_spectra_delayed.reset("Ge spectra delayed","Ge spectra delayed;Energy [keV]", 30000,0,15000);

    ge_spectra_VS_mult.reset("ge_spectra_VS_mult", "ge_spectra_VS_mult", 20,0,20, 10000,0,10000);
    ge_spectra_VSpromptmult.reset("ge_spectra_VSpromptmult", "ge_spectra_VSpromptmult", 20,0,20, 10000,0,10000);
    ge_spectra_VSdelayedmult.reset("ge_spectra_VSdelayedmult", "ge_spectra_VSdelayedmult", 20,0,20, 10000,0,10000);
    ge_prompt_spectra_VS_mult.reset("ge_prompt_spectra_VS_mult", "ge_prompt_spectra_VS_mult", 20,0,20, 10000,0,10000);
    ge_prompt_spectra_VSpromptmult.reset("ge_prompt_spectra_VSpromptmult", "ge_prompt_spectra_VSpromptmult", 20,0,20, 10000,0,10000);
    ge_prompt_spectra_VSdelayedmult.reset("ge_prompt_spectra_VSdelayedmult", "ge_prompt_spectra_VSdelayedmult", 20,0,20, 10000,0,10000);
    ge_delayed_spectra_VS_mult.reset("ge_delayed_spectra_VS_mult", "ge_delayed_spectra_VS_mult", 20,0,20, 10000,0,10000);
    ge_delayed_spectra_VSpromptmult.reset("ge_delayed_spectra_VSpromptmult", "ge_delayed_spectra_VSpromptmult", 20,0,20, 10000,0,10000);
    ge_delayed_spectra_VSdelayedmult.reset("ge_delayed_spectra_VSdelayedmult", "ge_delayed_spectra_VSdelayedmult", 20,0,20, 10000,0,10000);


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
    GeDelayed_VS_GePrompt.reset("Ge_delayed_VS_prompt","Ge delayed VS prompt;Prompt energy [keV];Delayed energy [keV]",
        4096,0,4096, 4096,0,4096);

    GeDelayed_VS_GeDelayed_time.reset("Ge_bidim_delayed_time","Ge bidim delayed time;time [ns];time [ns]",
        250,-50,200, 250,-50,200);

    DSSD_Spectra.reset("DSSD_spectra","DSSD spectra;Energy [keV];", 500,0,20000);
    DSSD_Spectra_VS_angle.reset("DSSD_Spectra_VS_angle","DSSD spectra VS Angle;Backward Angle [#circ]; Energy [keV];", 
        40,30,70, 500,0,20000);
    Sector_VS_Rings_mult.reset("Sector_VS_Rings_mult", "Sector VS Ring mult;Rings;Sectors", 10,0,10, 10,0,10);
    Ge_VS_DSSD.reset("Ge_VS_DSSD","Ge VS DSSD;Energy DSSD [keV];Energy Ge [keV]",
        400,0,20000, 1000,0,1000);
    GePrompt_VS_DSSD.reset("GePrompt_VS_DSSD","Ge VS DSSD",
        400,0,20000, 10000,0,10000);
    GeDelayed_VS_DSSD.reset("GeDelayed_VS_DSSD","Ge VS DSSD",
        400,0,20000, 10000,0,10000);
    DSSD_TW.reset("DSSD_TW","DSSD E VS Time",
        250,-50,200, 400,0,20000);
    Ge_VS_DSSD_Time.reset("Ge_VS_DSSD_Time","Ge VS DSSD Time;DSSD time [ns];Ge time [ns]",
        250,-50,200, 250,-50,200);

    BGO_VS_Ge_prompt.reset("BGO_VS_Ge_prompt","BGO VS Ge prompt;Ge Energy [keV];BGO Energy [keV]",
        4096,0,4096, 800,0,4096);
    BGO_prompt_VS_Ge_prompt_mult.reset("BGO_prompt_VS_Ge_prompt_mult","BGO VS Ge prompt;Ge Energy [keV];BGO Energy [keV]",
        4096,0,4096, 800,0,4096);
    BGO_VS_Ge_delayed.reset("BGO_VS_Ge_delayed","BGO VS Ge delayed;Ge Energy [keV];BGO Energy [keV]",
        4096,0,4096, 800,0,4096);
    BGO_VS_Ge_511.reset("BGO_vs_Ge_511","BGO in coincidence with 511;BGO energy [keV];",
        48,0,48, 4096,0,100000);

    Nuball_calo.reset("Nuball_calo","Clovers calorimetry;Calorimetry [keV]", 1024,0,20000);
    Nuball_calo_prompt.reset("Nuball_calo_prompt","Clovers prompt calorimetry;Calorimetry [keV]", 1024,0,20000);
    Nuball_calo_delayed.reset("Nuball_calo_delayed","Clovers delayed calorimetry;Calorimetry [keV]", 1024,0,20000);
    Ge_calo.reset("Ge_calo","Ge calorimetry;Calorimetry [keV]", 1024,0,20000);
    Ge_calo_prompt.reset("Ge_calo_prompt","Ge prompt calorimetry;Calorimetry [keV]", 1024,0,20000);
    Ge_calo_delayed.reset("Ge_calo_delayed","Ge delayed calorimetry;Calorimetry [keV]", 1024,0,20000);
    BGO_calo.reset("BGO_calo","BGO calorimetry;Calorimetry [keV]", 1024,0,20000);
    BGO_calo_prompt.reset("BGO_calo_prompt","BGO prompt calorimetry;Calorimetry [keV]", 1024,0,20000);
    BGO_calo_delayed.reset("BGO_calo_delayed","BGO delayed calorimetry;Calorimetry [keV]", 1024,0,20000);
    Paris_calo.reset("Paris_calo","Paris calorimetry;Calorimetry [keV]", 1024,0,20000);
    Paris_calo_prompt.reset("Paris_calo_prompt","Paris prompt calorimetry;Calorimetry [keV]", 1024,0,20000);
    Paris_calo_delayed.reset("Paris_calo_delayed","Paris delayed calorimetry;Calorimetry [keV]", 1024,0,20000);
    total_calo.reset("total_calo","Total calorimetry;Calorimetry [keV]", 1024,0,20000);
    total_calo_prompt.reset("total_calo_prompt","Total prompt calorimetry;Calorimetry [keV]", 1024,0,20000);
    total_calo_delayed.reset("total_calo_delayed","Total delayed calorimetry;Calorimetry [keV]", 1024,0,20000);
    
    calo_prompt_total_VS_calo_total.reset("calo_prompt_total_VS_calo_total","Total prompt calorimetry VS total delayed calorimetry;Total Calorimetry [keV];Prompt Total Calorimetry [keV]", 
        1024,0,20000, 1024,0,20000);
    calo_delayed_total_VS_calo_total.reset("calo_delayed_total_VS_calo_total","Total delayed calorimetry;Calorimetry [keV];Delayed Total Calorimetry [keV]",
        1024,0,20000, 1024,0,20000);
    delayed_calo_total_VS_prompt_calo_total.reset("delayed_calo_total_VS_prompt_calo_total","Total delayed calorimetry;Prompt Total Calorimetry [keV];Delayed Total Calorimetry [keV]",
        1024,0,20000, 1024,0,20000);

    Ge_prompt_VS_Total_calo.reset("Ge_prompt_VS_Total_calo","Ge prompt spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Total_calo.reset("Ge_delayed_VS_Total_calo","Ge delayed spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_prompt_VS_Total_calo_prompt.reset("Ge_prompt_VS_Total_calo_prompt","Ge prompt spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Total_calo_prompt.reset("Ge_delayed_VS_Total_calo_prompt","Ge delayed spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Total_calo_delayed.reset("Ge_delayed_VS_Total_calo_delayed","Ge delayed spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_prompt_VS_Total_calo_delayed.reset("Ge_prompt_VS_Total_calo_delayed","Ge prompt spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);

    Ge_prompt_VS_Nuball_calo.reset("Ge_prompt_VS_Nuball_calo","Ge prompt spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Nuball_calo.reset("Ge_delayed_VS_Nuball_calo","Ge delayed spectra VS Clovers calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_prompt_VS_Nuball_calo_prompt.reset("Ge_prompt_VS_Nuball_calo_prompt","Ge prompt spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Nuball_calo_prompt.reset("Ge_delayed_VS_Nuball_calo_prompt","Ge delayed spectra VS Clovers prompt calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_prompt_VS_Nuball_calo_delayed.reset("Ge_prompt_VS_Nuball_calo_delayed","Ge prompt spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Ge_delayed_VS_Nuball_calo_delayed.reset("Ge_delayed_VS_Nuball_calo_delayed","Ge delayed spectra VS Clovers delayed calorimetry;Calorimetry-Ge Energy [keV];Ge Energy [keV]",
        1024,0,30000, 4096,0,4096);
    Nuball_calo_delayed_VS_prompt.reset("Nuball_calo_delayed_VS_prompt","Clovers delayed calorimetry VS prompt calorimetry;Prompt Calorimetry [keV];Delayed Calorimetry [keV]",
        1024,0,30000, 4096,0,30000);

    DSSD_VS_Nuball_calo.reset("DSSD_VS_Nuball_calo","DSSD spectra VS clovers calorimetry;Calorimetry [keV];Particle Energy [keV]",
        1024,0,20000, 3000,0,30000);
    DSSD_VS_Total_calo.reset("DSSD_VS_Total_calo","DSSD spectra VS total calorimetry;Total Calorimetry [keV];DSSD Energy [keV]",
        1024,0,20000, 3000,0,30000);
    DSSD_VS_Total_Prompt_calo.reset("DSSD_VS_Total_Prompt_calo","DSSD spectra VS Prompt calorimetry;Total Calorimetry [keV];DSSD Energy [keV]",
        1024,0,20000, 3000,0,30000);
    DSSD_VS_Total_Delayed_calo.reset("DSSD_VS_Total_Delayed_calo","DSSD spectra VS Delayed calorimetry;Total Calorimetry [keV];DSSD Energy [keV]",
        1024,0,20000, 3000,0,30000);

    Missing_VS_Delayed_cal.reset("Missing_VS_Delayed_cal","Missing energy VS delayed calorimetry;Delayed Calorimetry [keV];Missing Energy [keV]",
        1024,0,20000, 1024,0,20000);
    Delayed_Ge_VS_Missing.reset("Delayed_Ge_VS_Missing","Delayed_Ge_VS_Missing;Missing Energy [keV];Delayed Calorimetry [keV]",
        300,0,20000, 10000,0,10000);
    Prompt_Ge_VS_Missing.reset("Prompt_Ge_VS_Missing","Prompt_Ge_VS_Missing;Missing Energy [keV];Prompt Calorimetry [keV]",
        300,0,20000, 10000,0,10000);
    Prompt_Calo_VS_Missing.reset("Prompt_Calo_VS_Missing","Missing energy VS delayed calorimetry;Delayed Calorimetry [keV];Missing Energy [keV]",
        500,0,20000, 1024,0,20000);
    Clover_Mult_VS_Nuball_calo.reset("Clover_Mult_VS_Nuball_calo","Multiplicity Clover VS clovers calorimetry;Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);
    prompt_Clover_Mult_VS_Nuball_calo.reset("prompt_Clover_Mult_VS_Nuball_calo","Prompt : Multiplicity Clover VS clovers calorimetry;Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);
    delayed_Clover_Mult_VS_Nuball_calo.reset("delayed_Clover_Mult_VS_Nuball_calo","Delayed : Multiplicity Clover VS clovers calorimetry;Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);
    Clover_Mult_VS_Total_calo.reset("Clover_Mult_VS_Total_calo","Multiplicity Clover VS Total calorimetry;Total Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);
    prompt_Clover_Mult_VS_Total_calo.reset("prompt_Clover_Mult_VS_Total_calo","Prompt : Multiplicity Clover VS Total calorimetry;Total Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);
    delayed_Clover_Mult_VS_Total_calo.reset("delayed_Clover_Mult_VS_Total_calo","Delayed : Multiplicity Clover VS Total calorimetry;Total Calorimetry [keV];Multiplicity",
        1024,0,20000, 20,0,20);

    calo_VS_promptMult.reset("calo_VS_promptMult", "calo_VS_promptMult;Calorimetry [keV];Prompt Multiplicity", 1024,0,20000, 20,0,20);
    calo_VS_delayedMult.reset("calo_VS_delayedMult", "calo_VS_delayedMult;Calorimetry [keV];Delayed Multiplicity", 1024,0,20000, 20,0,20);
    prompt_calo_VS_promptMult.reset("prompt_calo_VS_promptMult", "prompt_calo_VS_promptMult;Calorimetry [keV];Prompt Multiplicity", 1024,0,20000, 20,0,20);
    prompt_calo_VS_delayedMult.reset("prompt_calo_VS_delayedMult", "prompt_calo_VS_delayedMult;Calorimetry [keV];Delayed Multiplicity", 1024,0,20000, 20,0,20);
    delayed_calo_VS_promptMult.reset("delayed_calo_VS_promptMult", "delayed_calo_VS_promptMult;Calorimetry [keV];Prompt Multiplicity", 1024,0,20000, 20,0,20);
    delayed_calo_VS_delayedMult.reset("delayed_calo_VS_delayedMult", "delayed_calo_VS_delayedMult;Calorimetry [keV];Delayed Multiplicity", 1024,0,20000, 20,0,20);
        
    each_Ge_VS_Time.reset("each_Ge_VS_Time","Timing each Ge", 
        24,0,24, 2*USE_RF,-USE_RF/2,3*USE_RF/2);

    each_Ge_spectra.reset("each_Ge_spectra","Spectra each Ge", 
        24,0,24, 20000,0,10000);

    each_Ge_crystal_spectra.reset("each_Ge_crystal_spectra","Spectra each Ge crystal", 
        96,0,96, 30000,0,30000);

    Ge_VS_size_event.reset("Ge_VS_size_event","Ge VS number of detectors", 
        50,0,50, 5000,0,5000);

    Paris_spectra_back.reset("Paris_spectra_back", "Paris spectra back", 750,0,1500);
    Paris_time_spectra_back.reset("Paris_time_spectra_back", "Paris time spectra back", 750,0,1500, 500,-50,200);
    Paris_each_spectra_back.reset("Paris_each_spectra_back", "Paris each spectra back", 750,0,1500, 500,0,500);
    Paris_ratio_VS_time_back.reset("Paris_ratio_VS_time_back", "Paris ratio VS time back", 301,-100,200, 501,-2,2);
    Paris_spectra_front.reset("Paris_spectra_front", "Paris spectra front", 750,0,1500);
    Paris_time_spectra_front.reset("Paris_time_spectra_front", "Paris time spectra front", 750,0,1500, 500,-50,200);
    Paris_each_spectra_front.reset("Paris_each_spectra_front", "Paris each spectra front", 750,0,1500, 500,0,500);
    Paris_ratio_VS_time_front.reset("Paris_ratio_VS_time_front", "Paris ratio VS time front", 301,-100,200, 501,-2,2);
    
    Paris_back_calibrated_VS_delayed_U6.reset("Paris_back_calibrated_VS_delayed_U6", "Paris VS delayed U6 (642, 903, 987)", 1000,0,20000);

    auto const & nb_paris = detectors.nbOfType("paris");
    Paris_singles_labr.resize(nb_paris);
    Paris_singles_nai.resize(nb_paris);
    for (size_t i_paris = 0; i_paris<nb_paris; i_paris++)
    {
      auto const & name = detectors.name("paris", i_paris);
      Paris_singles_labr[i_paris].reset(name+"labr", (name+" LaBr3;E [keV];#").c_str(), 1000,0,10000);
      Paris_singles_nai [i_paris].reset(name+"_nai", (name+" NaI;E [keV];#")  .c_str(), 1000,0,10000);
    }
  }

  auto const & nb_dssd = 32;
  // auto const & nb_dssd = detectors.nbOfType("dssd");
  // DSSD_bidims.resize(nb_dssd);
  DSSD_projTot_VS_Rings.resize(nb_dssd);
  DSSD_proj169_VS_Rings.resize(nb_dssd);
  DSSD_proj510_VS_Rings.resize(nb_dssd);
  DSSD_proj642_VS_Rings.resize(nb_dssd);
  DSSD_proj687_VS_Rings.resize(nb_dssd);
  DSSD_proj870_VS_Rings.resize(nb_dssd);
  DSSD_proj880_VS_Rings.resize(nb_dssd);
  DSSD_proj925_VS_Rings.resize(nb_dssd);
  DSSD_proj1014_VS_Rings.resize(nb_dssd);
  DSSD_proj2210_VS_Rings.resize(nb_dssd);
  DSSD_proj3000_VS_Rings.resize(nb_dssd);
  DSSD_proj3682_VS_Rings.resize(nb_dssd);
  DSSD_proj3850_VS_Rings.resize(nb_dssd);
  for (size_t i_dssd = 0; i_dssd<nb_dssd; i_dssd++)
  {
    auto const & name = detectors.name("dssd", i_dssd);
    // DSSD_bidims[i_dssd].reset(name, (name+";Clovers [keV];DSSD [keV]").c_str(), 15000,0,15000, 750,0,15000);
    DSSD_projTot_VS_Rings[i_dssd].reset(name+"_projTot", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj169_VS_Rings[i_dssd].reset(name+"_proj169", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj510_VS_Rings[i_dssd].reset(name+"_proj510", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj642_VS_Rings[i_dssd].reset(name+"_proj642", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj687_VS_Rings[i_dssd].reset(name+"_proj687", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj870_VS_Rings[i_dssd].reset(name+"_proj870", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj880_VS_Rings[i_dssd].reset(name+"_proj880", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj925_VS_Rings[i_dssd].reset(name+"_proj925", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj1014_VS_Rings[i_dssd].reset(name+"_proj1014", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj2210_VS_Rings[i_dssd].reset(name+"_proj2210", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj3000_VS_Rings[i_dssd].reset(name+"_proj3000", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj3682_VS_Rings[i_dssd].reset(name+"_proj3682", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
    DSSD_proj3850_VS_Rings[i_dssd].reset(name+"_proj3850", (name+";Ring n°;DSSD [keV]").c_str(), 15,0,15, 750,0,15000);
  }

  // Set analysis parameters :
  RF_Manager::set_offset_ns(40);
}

void AnalyseIsomer::FillSorted(Event const & event, Clovers & clovers, DSSD & dssd, ParisNul & paris)
{

  auto const & PromptMult = clovers.PromptMult + paris.PromptMult;
  auto const & DelayedMult = clovers.DelayedMult + paris.DelayedMult;
  auto const & TotalMult = PromptMult+DelayedMult;
  if (m_trigger1989 && (PromptMult<1 || PromptMult>4 || paris.rejection || DelayedMult>5)) return;

  auto const calo_clovers = clovers.totGe+clovers.totBGO;
  auto const & calo_prompt_clovers = clovers.totGe_prompt+clovers.totBGO_prompt;
  auto const & calo_delayed_clovers = clovers.totGe_delayed+clovers.totBGO_delayed;
  // Calorimetry with Paris :
  auto const calo_total_Paris = paris.PromptCalo+paris.DelayedCalo;
  // Total calorimetry :
  auto const calo_total = calo_total_Paris+calo_clovers;
  auto const calo_prompt_total = paris.PromptCalo+calo_prompt_clovers;
  auto const calo_delayed_total = paris.DelayedCalo+calo_delayed_clovers;

  if (!m_choose1)
  {
    for (auto const & crystal : clovers.cristaux) each_Ge_crystal_spectra.Fill(crystal, clovers.cristaux_nrj[crystal]);

    //////////////////////////
    // --- Multiplicity --- //
    //////////////////////////
    Sector_VS_Rings_mult.Fill(dssd.RingMult, dssd.SectorMult);
    Delayed_VS_Prompt_Mult.Fill(PromptMult, DelayedMult);
    prompt_mult_VS_sectors_mult.Fill(dssd.SectorMult, PromptMult);
    delayed_mult_VS_sectors_mult.Fill(dssd.SectorMult, DelayedMult);

    // if (dssd.SectorMult>0) printMT(dssd.oneParticle(), dssd.energy(), proton_in_DSSD.isIn(dssd.energy()), dssd.time(), proton_prompt.isIn(dssd.time()));
    
    /////////////////////////
    // --- Calorimetry --- //
    /////////////////////////


    Nuball_calo.Fill(calo_clovers);
    Nuball_calo_prompt.Fill(calo_prompt_clovers);
    Nuball_calo_delayed.Fill(calo_delayed_clovers);
    
    Nuball_calo_delayed_VS_prompt.Fill(calo_prompt_clovers, calo_delayed_clovers);
    Clover_Mult_VS_Nuball_calo.Fill(calo_clovers, clovers.TotalMult);
    prompt_Clover_Mult_VS_Nuball_calo.Fill(calo_prompt_clovers, clovers.PromptMult);
    delayed_Clover_Mult_VS_Nuball_calo.Fill(calo_delayed_clovers, clovers.DelayedMult);

    Ge_calo.Fill(clovers.totGe);
    Ge_calo_prompt.Fill(clovers.totGe_prompt);
    Ge_calo_delayed.Fill(clovers.totGe_delayed);

    BGO_calo.Fill(clovers.totBGO);
    BGO_calo_prompt.Fill(clovers.totBGO_prompt);
    BGO_calo_delayed.Fill(clovers.totBGO_delayed);

    Paris_calo.Fill(calo_total_Paris);
    Paris_calo_prompt.Fill(paris.PromptCalo);
    Paris_calo_delayed.Fill(paris.DelayedCalo);


    total_calo.Fill(calo_total);
    total_calo_prompt.Fill(calo_prompt_total);
    total_calo_delayed.Fill(calo_delayed_total);  

    Clover_Mult_VS_Total_calo.Fill(calo_total, clovers.TotalMult);
    prompt_Clover_Mult_VS_Total_calo.Fill(calo_prompt_total, clovers.PromptMult);
    delayed_Clover_Mult_VS_Total_calo.Fill(calo_delayed_total, clovers.DelayedMult);
    
    // Calorimetry correlations :
    if (calo_total!=calo_prompt_total) calo_prompt_total_VS_calo_total.Fill(calo_total, calo_prompt_total);
    if (calo_total!=calo_delayed_total) calo_delayed_total_VS_calo_total.Fill(calo_total, calo_delayed_total);
    delayed_calo_total_VS_prompt_calo_total.Fill(calo_prompt_total, calo_delayed_total);

    calo_VS_promptMult.Fill(calo_total, PromptMult);
    calo_VS_delayedMult.Fill(calo_total, DelayedMult);
    prompt_calo_VS_promptMult.Fill(calo_prompt_total, PromptMult);
    prompt_calo_VS_delayedMult.Fill(calo_prompt_total, DelayedMult);
    delayed_calo_VS_promptMult.Fill(calo_delayed_total, PromptMult);
    delayed_calo_VS_delayedMult.Fill(calo_delayed_total, DelayedMult);
  }

  /////////////////////////////////
  // --- Loop over Clovers : --- //
  /////////////////////////////////

  for (uint loop_i = 0; loop_i<clovers.CleanGe.size(); loop_i++)
  {
    auto const & clover_i = clovers.m_clovers[clovers.CleanGe[loop_i]];

    // Extract hit_i data :
    auto const & nrj_i   = clover_i.nrj;
    auto const & time_i  = clover_i.time;
    auto const & label_i = clover_i.label();
    auto const prompt_i  = clover_i.isGePrompt;
    auto const delayed_i = clover_i.isGeDelayed;

    if (!m_choose1)
    {
      // Ge spectra :
      Ge_spectra.Fill(nrj_i);
      each_Ge_spectra.Fill(label_i, nrj_i);

      // Multiplicity :
      ge_spectra_VS_mult      .Fill(TotalMult, nrj_i);
      ge_spectra_VSpromptmult .Fill(PromptMult, nrj_i);
      ge_spectra_VSdelayedmult.Fill(DelayedMult, nrj_i);

      // Time spectra :
      Ge_Time_VS_Spectra.Fill(nrj_i, time_i);
      each_Ge_VS_Time.Fill(label_i, time_i);

      // Others :
      Ge_VS_size_event.Fill(event.mult, nrj_i);

    ////////////////////////
    // --- Prompt Ge: --- //
    ////////////////////////
      if (prompt_i) 
      {
        // Prompt spectra :
        Ge_spectra_prompt.Fill(nrj_i);
        
        // Multiplicity :
        ge_prompt_spectra_VS_mult      .Fill(TotalMult, nrj_i);
        ge_prompt_spectra_VSpromptmult .Fill(PromptMult, nrj_i);
        ge_prompt_spectra_VSdelayedmult.Fill(DelayedMult, nrj_i);

        // Total Calorimetry :
        Ge_prompt_VS_Total_calo.Fill(calo_total, nrj_i);
        Ge_prompt_VS_Total_calo_prompt.Fill(calo_prompt_total, nrj_i);
        Ge_prompt_VS_Total_calo_delayed.Fill(calo_delayed_total, nrj_i);
        
        // Nuball Calorimetry :
        Ge_prompt_VS_Nuball_calo.Fill(calo_clovers, nrj_i);
        Ge_prompt_VS_Nuball_calo_prompt.Fill(calo_prompt_clovers, nrj_i);
        Ge_prompt_VS_Nuball_calo_delayed.Fill(calo_delayed_clovers, nrj_i);
      }

      /////////////////////////
      // --- Delayed Ge: --- //
      /////////////////////////
      if (delayed_i) 
      {
        // Delayed spectra :
        Ge_spectra_delayed.Fill(nrj_i);

        // Multiplicity :
        ge_delayed_spectra_VS_mult      .Fill(TotalMult, nrj_i);
        ge_delayed_spectra_VSpromptmult .Fill(PromptMult, nrj_i);
        ge_delayed_spectra_VSdelayedmult.Fill(DelayedMult, nrj_i);

        // Total Calorimetry :
        Ge_delayed_VS_Total_calo.Fill(calo_total, nrj_i);
        Ge_delayed_VS_Total_calo_prompt.Fill(calo_prompt_total, nrj_i);
        Ge_delayed_VS_Total_calo_delayed.Fill(calo_delayed_total, nrj_i);

        // Nuball Calorimetry :
        Ge_delayed_VS_Nuball_calo.Fill(calo_clovers, nrj_i);
        Ge_delayed_VS_Nuball_calo_prompt.Fill(calo_prompt_clovers, nrj_i);
        Ge_delayed_VS_Nuball_calo_delayed.Fill(calo_delayed_clovers, nrj_i);

        // Gating for the other detectors :
        if (nrj_i>639 && nrj_i<644) 
        {
          for (auto const & index : paris.indexes)
          {
            // auto const & label = event.labels[index];
            auto const & nrj = event.nrjs[index];
            auto const & nrj2 = event.nrj2s[index];
            // auto const & time = event.time2s[index];
            auto const & ratio = (nrj2-nrj)/nrj2;
            if (ratio>-0.2 && ratio<0.2) Paris_back_calibrated_VS_delayed_U6.Fill(nrj);
          }
        }
      }
      
      ////////////////////////
      // --- Ge bidim : --- //
      ////////////////////////
      for (uint loop_j = loop_i+1; loop_j<clovers.CleanGe.size(); loop_j++)
      {
        auto const & clover_j = clovers.m_clovers[clovers.CleanGe[loop_j]];

        // Extract hit_j informations :
        auto const & time_j = clover_j.time;
        auto const & nrj_j  = clover_j.nrj;
        auto const prompt_j  = clover_j.isGePrompt;
        auto const delayed_j = clover_j.isGeDelayed;


        if (nrj_i > 507 && nrj_i<516 && nrj_j > 507 && nrj_j<516) continue;

        Ge_VS_Ge.Fill(nrj_i, nrj_j);
        Ge_VS_Ge.Fill(nrj_j, nrj_i);

        //////////////////////
        // --- Prompt : --- //
        //////////////////////
        if (prompt_i)
        {
          if (prompt_j)
          {
            GePrompt_VS_GePrompt.Fill(nrj_i,nrj_j);
            GePrompt_VS_GePrompt.Fill(nrj_j,nrj_i);
          }
          else if (delayed_j)
          {
            GeDelayed_VS_GePrompt.Fill(nrj_i,nrj_j);
          }
        }

        ///////////////////////
        // --- Delayed : --- //
        ///////////////////////
        else if (delayed_i)
        {        
          if (delayed_j)
          {
            GeDelayed_VS_GeDelayed_time.Fill(time_i, time_j);
            GeDelayed_VS_GeDelayed_time.Fill(time_j, time_i);
            GeDelayed_VS_GeDelayed.Fill(nrj_i, nrj_j);
            GeDelayed_VS_GeDelayed.Fill(nrj_j, nrj_i);
          }
          else if (prompt_j)
          {
            GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
          }
        }
      }

      //////////////////////////
      // --- BGO bidims : --- //
      //////////////////////////
      for (auto const & bgo : clovers.Bgo)
      {
        auto const & clover_bgo = clovers.m_clovers[bgo];
        // auto const & time_bgo  = clovers.cristaux_time_BGO[bgo];
        // auto const & nrj_bgo   = clovers.cristaux_nrj_BGO [bgo];
        auto const & nrj_bgo   = clover_bgo.nrj_BGO;
        auto const & prompt_bgo  = clover_bgo.isBGOPrompt;
        auto const & delayed_bgo = clover_bgo.isBGODelayed;
        if (prompt_bgo && prompt_i)
        {
          BGO_VS_Ge_prompt.Fill(nrj_i, nrj_bgo);
          BGO_prompt_VS_Ge_prompt_mult.Fill(nrj_i, nrj_bgo);
          if (nrj_i > 507 && nrj_i<515) BGO_VS_Ge_511.Fill(bgo, nrj_bgo);
        } 
        else if (delayed_i && delayed_bgo) BGO_VS_Ge_delayed.Fill(nrj_i, nrj_bgo);
      }

    }
    ////////////////////
    // --- DSSD : --- //
    ////////////////////
    auto const & nrj_dssd = dssd.energy();
    if (nrj_dssd>0)
    {
      auto const & time_dssd = dssd.time();
      auto const & angle = dssd.angle()/3.141596*180 + ring_deg_thick;

      for (auto const & sector : dssd) if (sector.nrj>0) 
      {
        // DSSD_bidims[sector.label()].Fill(nrj_i, sector.nrj);

        for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_projTot_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);

        if (nrj_i>167 && nrj_i<171) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj169_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>507 && nrj_i<515) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj510_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>639 && nrj_i<645) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj642_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>684 && nrj_i<691) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj687_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>867 && nrj_i<873) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj870_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>877 && nrj_i<883) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj880_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>922 && nrj_i<928) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj925_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>1011 && nrj_i<1017) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj1014_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>2200 && nrj_i<2230) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj2210_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>2980 && nrj_i<3020) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj3000_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>3675 && nrj_i<3685) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj3682_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
        else if (nrj_i>3845 && nrj_i<3855) 
        {
          for (auto const & ring : dssd.Rings) if (ring.nrj!=0.f) DSSD_proj3850_VS_Rings[sector.label()].Fill(ring.label(), sector.nrj);
        }
      }

      if (!m_choose1)
      {
        DSSD_Spectra.Fill(nrj_dssd);
        DSSD_Spectra_VS_angle.Fill(angle, nrj_dssd);
        Ge_VS_DSSD.Fill(nrj_dssd, nrj_i);
        if (prompt_i) GePrompt_VS_DSSD.Fill(nrj_dssd, nrj_i);
        if (delayed_i) GeDelayed_VS_DSSD.Fill(nrj_dssd, nrj_i);

        DSSD_TW.Fill(time_dssd, nrj_dssd);
        Ge_VS_DSSD_Time.Fill(time_dssd, time_i);
        DSSD_VS_Nuball_calo.Fill(calo_clovers, nrj_dssd);
        DSSD_VS_Total_calo.Fill(calo_total, nrj_dssd);
        DSSD_VS_Total_Prompt_calo.Fill(calo_prompt_total, nrj_dssd);
        DSSD_VS_Total_Delayed_calo.Fill(calo_delayed_total, nrj_dssd);

        //////////////////////////////
        // --- Missing energy : --- //
        //////////////////////////////
        auto const & Missing_E = Qdispo-calo_prompt_total-dssd.energy();
        Missing_VS_Delayed_cal.Fill(calo_delayed_total, Missing_E);
        if (delayed_i) Delayed_Ge_VS_Missing.Fill(Missing_E, nrj_i);
        else if (prompt_i) Prompt_Ge_VS_Missing.Fill(Missing_E, nrj_i);
        Prompt_Calo_VS_Missing.Fill(Missing_E, calo_prompt_total);
      }
    }
  }

  ///////////////////
  // --- PARIS --- //
  ///////////////////
  if (!m_choose1) for (auto const & index : paris.indexes)
  {
    auto const & label = event.labels[index];
    auto const & nrj = event.nrjs[index];
    auto const & nrj2 = event.nrj2s[index];
    auto const & time = event.time2s[index];
    auto const & ratio = (nrj2-nrj)/nrj2;

    if (ratio>-0.1 && ratio<0.2) Paris_singles_labr[index].Fill(nrj);
    if (ratio>0.55 && ratio<0.75) Paris_singles_nai[index].Fill(nrj);

    if (label<500)
    {
      Paris_spectra_back.Fill(nrj);
      Paris_time_spectra_back.Fill(nrj, time);
      Paris_each_spectra_back.Fill(nrj, compressedLabel[label]);
      Paris_ratio_VS_time_back.Fill(time, ratio);
    }
    else
    {
      Paris_spectra_front.Fill(nrj);
      Paris_time_spectra_front.Fill(nrj, time);
      Paris_each_spectra_front.Fill(nrj, compressedLabel[label]);
      Paris_ratio_VS_time_front.Fill(time, ratio);
    }
  }
}

void AnalyseIsomer::FillRaw(Event const & event)
{
  if (!m_choose1) for (size_t i = 0; i<event.size(); i++)
  {
    if (isGe[event.labels[i]]) raw_Ge_Time_VS_Spectra.Fill(event.nrjs[i],event.time2s[i]);
  }
}

void AnalyseIsomer::Write()
{
  if (m_writeRadware)
  {
    print("Writting Radware matrixes...");
    RWMat RW_prompt_prompt(GePrompt_VS_GePrompt); RW_prompt_prompt.Write();
    RWMat RW_del_del(GeDelayed_VS_GeDelayed); RW_del_del.Write();
    RWMat RW_prompt_del(GeDelayed_VS_GePrompt); RW_prompt_del.Write();
  }

  File outfilename(m_outDir+m_outRoot);
  outfilename.makePath(); // Create the output folder if it doesn't exist
  unique_TFile outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  outfile -> cd();

  print("Writting histograms ...");

  time_ref.Write();

  ge_spectra_VS_mult.Write();
  ge_spectra_VSpromptmult.Write();
  ge_spectra_VSdelayedmult.Write();
  ge_prompt_spectra_VS_mult.Write();
  ge_prompt_spectra_VSpromptmult.Write();
  ge_prompt_spectra_VSdelayedmult.Write();
  ge_delayed_spectra_VS_mult.Write();
  ge_delayed_spectra_VSpromptmult.Write();
  ge_delayed_spectra_VSdelayedmult.Write();
  
  delayed_mult_VS_sectors_mult.Write();
  Delayed_VS_Prompt_Mult.Write();
  Delayed_VS_Prompt_Mult_G1.Write();
  Delayed_VS_Prompt_Mult_G2.Write();
  Delayed_VS_Prompt_Mult_G3.Write();

  Ge_spectra.Write();
  Ge_spectra_delayed.Write();
  Ge_spectra_prompt.Write();
  Ge_Time_VS_Spectra.Write();
  raw_Ge_Time_VS_Spectra.Write();
  
  Ge_VS_Ge.Write();
  GePrompt_VS_GePrompt.Write();
  GeDelayed_VS_GeDelayed.Write();
  GeDelayed_VS_GePrompt.Write();


  BGO_VS_Ge_511.Write();
  BGO_VS_Ge_prompt.Write();
  BGO_prompt_VS_Ge_prompt_mult.Write();
  BGO_VS_Ge_delayed.Write();

  Nuball_calo.Write();
  Nuball_calo_prompt.Write();
  Nuball_calo_delayed.Write();
  Ge_calo.Write();
  Ge_calo_prompt.Write();
  Ge_calo_delayed.Write();
  BGO_calo.Write();
  BGO_calo_prompt.Write();
  BGO_calo_delayed.Write();
  Paris_calo.Write();
  Paris_calo_prompt.Write();
  Paris_calo_delayed.Write();
  total_calo.Write();
  total_calo_prompt.Write();
  total_calo_delayed.Write();
  calo_prompt_total_VS_calo_total.Write();
  calo_delayed_total_VS_calo_total.Write();
  delayed_calo_total_VS_prompt_calo_total.Write();

  Nuball_calo_delayed_VS_prompt.Write();
  Clover_Mult_VS_Nuball_calo.Write();
  prompt_Clover_Mult_VS_Nuball_calo.Write();
  delayed_Clover_Mult_VS_Nuball_calo.Write();
  Clover_Mult_VS_Total_calo.Write();
  prompt_Clover_Mult_VS_Total_calo.Write();
  delayed_Clover_Mult_VS_Total_calo.Write();
  
  DSSD_VS_Nuball_calo.Write();
  DSSD_VS_Total_calo.Write();
  DSSD_VS_Total_Prompt_calo.Write();
  DSSD_VS_Total_Delayed_calo.Write();

  calo_VS_promptMult.Write();
  calo_VS_delayedMult.Write();
  prompt_calo_VS_promptMult.Write();
  prompt_calo_VS_delayedMult.Write();
  delayed_calo_VS_promptMult.Write();
  delayed_calo_VS_delayedMult.Write();

  Ge_prompt_VS_Total_calo.Write();
  Ge_delayed_VS_Total_calo.Write();
  Ge_prompt_VS_Total_calo_prompt.Write();
  Ge_delayed_VS_Total_calo_prompt.Write();
  Ge_prompt_VS_Total_calo_delayed.Write();
  Ge_delayed_VS_Total_calo_delayed.Write();

  Ge_prompt_VS_Nuball_calo.Write();
  Ge_delayed_VS_Nuball_calo.Write();
  Ge_prompt_VS_Nuball_calo_prompt.Write();
  Ge_delayed_VS_Nuball_calo_prompt.Write();
  Ge_prompt_VS_Nuball_calo_delayed.Write();
  Ge_delayed_VS_Nuball_calo_delayed.Write();

  Missing_VS_Delayed_cal.Write();
  Delayed_Ge_VS_Missing.Write();
  Prompt_Ge_VS_Missing.Write();
  Prompt_Calo_VS_Missing.Write();


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

  GeDelayed_VS_GeDelayed_time.Write();

  Paris_spectra_back.Write();
  Paris_time_spectra_back.Write();
  Paris_each_spectra_back.Write();
  Paris_ratio_VS_time_back.Write();

  Paris_spectra_front.Write();
  Paris_time_spectra_front.Write();
  Paris_each_spectra_front.Write();
  Paris_ratio_VS_time_front.Write();

  Paris_back_calibrated_VS_delayed_U6.Write();

  for (auto & spectra : Paris_singles_labr) spectra.Write();
  for (auto & spectra : Paris_singles_nai) spectra.Write();

  for (auto & spectra : DSSD_bidims) spectra.Write();
  for (auto & spectra : DSSD_proj169_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj510_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj642_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj687_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj870_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj880_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj925_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj1014_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj2210_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj3000_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj3682_VS_Rings) spectra.Write();
  for (auto & spectra : DSSD_proj3850_VS_Rings) spectra.Write();

  outfile->Write();
  outfile->Close();

  print("Writting analysis in", m_outDir+m_outRoot);
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
      else if (temp == "outDir:")  is >> m_outDir;
      else if (temp == "outRoot:")  is >> m_outRoot;
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
      else if (temp == "writeRadware")
      {
        m_writeRadware = true;
      }
      else if (temp == "trigger1989")
      {
        m_trigger1989 = true;
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

/*
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
//     else if (prompt_i) Ge_spectra.Fill(nrj_i);

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
*/
#endif //ANALYSEISOMER_H
