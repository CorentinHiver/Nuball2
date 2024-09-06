#ifndef ANALYSE_W_H
#define ANALYSE_W_H
#include "../NearLine.hpp"
#include <time.h>

//Start doing the following :
//search (ctrl+F or equivalent)
//type "Analyse_W"
//then replace by the name of your class
//And modify the template class the way you want
//
//You'll need to make the following steps in NearLine:
// 1 : forward declaration before the NearLine class : class Analyse_W;*
// 2 : declare the class friend like this : friend class Analyse_W;
// 3 : declare the class like this : Analyse_W *m_mt = nullprt;
// 4 : import the module AFTER NearLine class
// 5 : instantiate the module in the NearLine::setConfig member : m_mt = new Analyse_W(this,whatever...);
// 6 : Initialise the module in the NearLine::Initialise() member : m_mt.Initialise();
// 7 : play with it in the NearLine::processFile() member (using Fill or any other user defined function)
// 8 : Calculate or Write anything at the end of runs in the NearLine::WriteData() member : m_mt.Write() | m_mt.Calculate()
// 9 : in the final method to use it, dont forget to DELETE IT after checking it exists


class Analyse_W
{
public:
  Analyse_W(NearLine* nearline) : n(nearline){}

  operator bool() {return m_activated;}
  void operator=(bool activate) {m_activated = activate;}
  // Feel free to custom more operators

  Bool_t Initialise();
  Bool_t SetConfig(std::istringstream & is);
  Bool_t Check();
  void Calculate();
  void Write();

  Float_t time_RF(Time const & time, int const & thread_nb) {return ((n->m_use_RF) ? m_rf[thread_nb]->pulse_ToF(time, n->m_RF_shift)/_ns : 0.);}
  void fillCoincRingSector(Label const & Ring, Label const & Sector, int const & thread_nb);

  //User defined methods :
  void Fill(Event const & event, UShort_t const & thread_nb = 0);
  void FillTiming(Hit const & hit, UShort_t const & thread_nb = 0);
  void FillRaw(Hit const & hit, UShort_t const & thread_nb = 0) {if (isGe[hit.label]) m_Ge_raw[thread_nb] -> Fill(hit.nrjcal);}
  void FillTimeGated(Hit const & hit, UShort_t const & thread_nb) {if (m_time_gated && isGe[hit.label]) m_Ge_prompt[thread_nb] -> Fill(hit.nrjcal);}
  void treat_event(Event const & event, Sorted_Event & arg, UShort_t const & thread_nb);

  // Time gate management :œ
  Bool_t isInTimeGate(Hit const & hit, int const & thread_nb);
  Bool_t   const & isTimeGated() const {return m_time_gated;}
  Long64_t const & getTimeMin() const {return m_time_gate_min;}
  Long64_t const & getTimeGateMax() const {return m_time_gate_max;}

  // User defined setters :
  void setRF(RF_Manager* rf, int const & thread_nb) {if (n->m_use_RF) m_rf[thread_nb] = rf;}

  // User defined getters :
  RF_Manager* RF(int const & thread_nb = 0) {return m_rf[thread_nb];}
  std::string const & outroot() const {return m_outroot;}
  Long64_t const & timeWindow() const {return m_timeWindow;}

private:
  Bool_t m_activated = true;
  NearLine* n = nullptr;

  // User defined members :
  std::vector<RF_Manager*>  m_rf;
  Timing_ref timeref;

  std::vector<Sorted_Event> evt_cntr;

  std::string m_outroot = "analyse.root";
  std::string m_outdir  = "Analyse_W/";
  std::string m_time_ref_det  = ""; // time reference used for time alignement purposes
  UShort_t    m_time_ref_label = 0; // label of the time reference
  Bool_t      m_time_gated = false;
  Long64_t    m_time_gate_min = -20000ll;
  Long64_t    m_time_gate_max = 20000ll;
  Long64_t    m_timeWindow_ns = 350;
  Long64_t    m_timeWindow = 350000;
  #ifdef LICORNE
  Float_t     EDEN_TO_LICORNE_DISTANCE = 268.f; //cm
  Float_t     GAMMA_FLASH = 25.f; //ns
  #endif //LICORNE
  Int_t m_nb_gates = 0;
  // Gates m_gate_counter;

  #ifdef USE_DSSD
  DSSD_Event m_last_DSSD_event;
  Time m_last_DSSD_event_timestamp = 0;
  Time m_current_DSSD_event_timestamp = 0;
  #endif //USE_DSSD

  // Here create any kind of histograms :
  MultiHist<TH1F> m_DeltaT;
  MultiHist<TH2F> m_ToF_spectra;
  // MultiHist<TH1F> DeltaT       ;
  MultiHist<TH1F> m_DeltaT_LaBr3 ;
  MultiHist<TH1F> m_DeltaT_LaBr3_ref ;
  MultiHist<TH1F> m_DeltaT_Paris ;
  MultiHist<TH1F> m_DeltaT_Paris_ref ;
  MultiHist<TH2F> m_label_DeltaT ;
  MultiHist<TH2F> m_label_DeltaT_RF ;
  MultiHist<TH2F> m_label_DeltaT_RF_2 ;
  MultiHist<TH2F> m_label_DeltaT_RF_3 ;
  MultiHist<TH2F> m_DeltaT2D     ;
  MultiHist<TH2F> m_deltaT_energy;
  MultiHist<TH1F> m_EnergyRef    ;

    //Energy Spectra
  MultiHist<TH1F> m_Ge_raw;
  MultiHist<TH1F> m_Ge_prompt;
  MultiHist<TH1F> m_Ge_addback;
  MultiHist<TH1F> m_Clean_Ge;
  MultiHist<TH1F> m_Compton_garbage    ;
  MultiHist<TH1F> m_M;
  MultiHist<TH1F> m_Ge_M1;
  MultiHist<TH1F> m_Ge_M2;
  MultiHist<TH1F> m_Ge_M3;
  MultiHist<TH1F> m_Ge_M4;
  MultiHist<TH1F> m_Ge_M5;
  MultiHist<TH1F> m_Ge_M6;
  MultiHist<TH1F> m_Ge_Msup6;
  MultiHist<TH1F> m_Ge_M3_6;
  MultiHist<TH1F> m_Ge_Single_Spectra ;

  MultiHist<TH1F> m_LaBr3_Spectra;
  MultiHist<TH1F> m_Paris_Spectra;
  MultiHist<TH1F> m_R2_Spectra;
  MultiHist<TH1F> m_R3_Spectra;
  MultiHist<TH1F> m_R2_Spectra_Doppler_correct;
  MultiHist<TH1F> m_R3_Spectra_Doppler_correct;
      // - 4 rings configurations :
  MultiHist<TH1F> m_R1_Spectra;
  MultiHist<TH1F> m_R4_Spectra;
  MultiHist<TH1F> m_Neutrons_Spectra;
      // - paris configuration :
  MultiHist<TH1F> m_Front_Spectra;
  MultiHist<TH1F> m_Back_Spectra;
  MultiHist<TH1F> m_BR1_Spectra;
  MultiHist<TH1F> m_BR2_Spectra;
  MultiHist<TH1F> m_BR3_Spectra;
  MultiHist<TH1F> m_FR1_Spectra;
  MultiHist<TH1F> m_FR2_Spectra;
  MultiHist<TH1F> m_FR3_Spectra;
  MultiHist<TH2F> Ge_VS_time_Paris;
    //Gated energy spectra
  // MultiHist<TH1F> gated_Spectra       ;
  // MultiHist<TH1F> gated_compt_supp    ;
  // MultiHist<TH1F> gated_compt_add_back;
  // MultiHist<TH1F> gated_compt_veto    ;
  // MultiHist<TH2F> gated_deltaT        ;
    //Energy bidim
  MultiHist<TH2F> m_gamma_gamma_single;
  MultiHist<TH2F> m_clover_clover;
  MultiHist<TH2F> m_clover_clover_compt_supp;
  MultiHist<TH2F> m_bidim_clover_clean_M_sup_3;
  MultiHist<TH2F> m_clover_LaBr3;
  MultiHist<TH2F> m_Spectra_all_Paris;
  MultiHist<TH2F> m_clover_Paris;
  MultiHist<TH2F> m_LaBr3_LaBr3;
  MultiHist<TH2F> m_Paris_Paris;
  MultiHist<TH2F> m_Spectra_all_LaBr3;
  MultiHist<TH2F> m_Spectra_all_Ge_Clover;
  MultiHist<TH2F> m_Spectra_all_Clover;

    //Prompt gate:
  MultiHist<TH2F> m_p_Ge_Ge_clean;
  MultiHist<TH2F> m_p_LaBr3_Ge;
  MultiHist<TH2F> m_p_LaBr3_LaBr3;
  MultiHist<TH2F> m_p_Paris_Ge;
  MultiHist<TH2F> m_p_Paris_Ge_C1P1;
  MultiHist<TH2F> m_p_Paris_Paris;

  //dssd :
    // Germanium and particle in coincidence :
  MultiHist<TH1F> m_DSSD_gate_Clover;
  MultiHist<TH2F> m_DSSD_gate_Clover_bidim;
  MultiHist<TH2F> m_prompt_particule_gate_Clover_spectra;
  MultiHist<TH2F> m_particule_gated_Clover_prompt_VS_delayed;
  MultiHist<TH2F> m_clover_VS_DSSD;
  MultiHist<TH2F> m_DSSD_R3_VS_Clover;
  MultiHist<TH2F> m_DSSD_R4_VS_Clover;
  MultiHist<TH2F> m_DSSD_R5_VS_Clover;
  MultiHist<TH1F> m_DSSD_gate_Clover_delayed;
  MultiHist<TH1F> m_DSSD_gate_Clover_prompt;

  // Energy gate :
  MultiHist<TH1F> m_DSSD_E_gate_197Au;
  MultiHist<TH1F> m_DSSD_E_gate_56Fe;
  MultiHist<TH1F> m_DSSD_E_gate_52Cr;
  MultiHist<TH1F> m_DSSD_E_gate_12C;
  MultiHist<TH1F> m_DSSD_E_gate_16O;
  MultiHist<TH1F> m_DSSD_E_gate_236U;
  MultiHist<TH1F> m_DSSD_E_gate_9Be;
  MultiHist<TH1F> m_DSSD_wide_E_gate_16O;

  //Counters :
  MultiHist<TH1F> m_DSSD_Counter;
  MultiHist<TH2F> m_DSSD_crosstalk_Ring;
  MultiHist<TH2F> m_DSSD_Time_bidim;
  MultiHist<TH2F> m_DSSD_coinc_ring_sector;
  MultiHist<TH2F> m_DSSD_crosstalk_Sector;
  MultiHist<TH1F> m_DSSD_mult;
  MultiHist<TH1F> m_DSSD_mult_Ring;
  MultiHist<TH1F> m_DSSD_mult_Sector;
  MultiHist<TH2F> m_DSSD_mult_Sector_VS_mult_Ring;
  MultiHist<TH2F> m_DSSD_mult_Sector_VS_mult_Ring_E_sup_10_4;

  MultiHist<TH1F> m_DSSD_sum_2_rings;
  MultiHist<TH1F> m_DSSD_sum_2_sectors;

  MultiHist<TH2F> m_DSSD_Spectra_each_channel;
  // MultiHist<TH2F> m_merged_DSSD; //to keep in case
  MultiHist<TH2F> m_DSSD_Spectra_each_channel_1R1S; //when in coinc only 1 ring and 1 sector
  MultiHist<TH2F> m_DSSD_Spectra_each_channel_coinc2Sectors;
  MultiHist<TH2F> m_DSSD_Spectra_each_channel_coinc2Rings;
  MultiHist<TH2F> m_DSSD_R_1_coinc;
  MultiHist<TH2F> m_DSSD_R_0_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_1_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_2_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_3_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_4_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_5_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_6_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_7_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_8_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_9_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_10_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_11_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_12_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_13_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_14_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_15_VS_Sectors_1R1S;
  MultiHist<TH2F> m_DSSD_R_15_VS_R_14;
  MultiHist<TH2F> m_DSSD_R_14_VS_R_13;
  MultiHist<TH2F> m_DSSD_R_13_VS_R_12;
  MultiHist<TH2F> m_DSSD_R_12_VS_R_11;
  MultiHist<TH2F> m_DSSD_R_11_VS_R_10;
  MultiHist<TH2F> m_DSSD_banana_R1_allsectors;
  MultiHist<TH2F> m_DSSD_banana_R1_allsectors_1R1S;
  MultiHist<TH2F> m_DSSD_Sector_spectra_coinc;
  MultiHist<TH1F> m_DSSD_single_Rings;
  MultiHist<TH1F> m_DSSD_single_Sectors;
  MultiHist<TH2F> m_DSSD_single_Sectors_1R1S_each_sector;
  MultiHist<TH1F> m_DSSD_single_R1;
  MultiHist<TH1F> m_DSSD_M2_R1;
  MultiHist<TH1F> m_DSSD_Msup1_R1;
  MultiHist<TH1F> m_DSSD_R1;
  MultiHist<TH2F> m_DSSD_R10_R11;
  MultiHist<TH2F> m_DSSD_R9_R10;
  MultiHist<TH2F> m_DSSD_R8_R9;
  MultiHist<TH2F> m_DSSD_R1_S1_1;

  // ToF spectra
  MultiHist<TH2F> m_DSSD_Tof_VS_Energy;
  MultiHist<TH2F> m_DSSD_Tof_VS_Energy_U6;
  MultiHist<TH2F> m_DSSD_ToF_first_blob;
  MultiHist<TH2F> m_DSSD_ToF_second_blob;
  MultiHist<TH2F> m_DSSD_ToF_third_blob;
  MultiHist<TH2F> m_DSSD_ToF_third_blob_wider;

  MultiHist<TH2F> m_DSSD_Sectors_mult_VS_spectra;
  MultiHist<TH2F> m_DSSD_Rings_mult_VS_spectra;

  MultiHist<TH2F> m_DSSD_Sector_VS_ring_polar;
  MultiHist<TH2F> m_DSSD_Sector_VS_ring_polar_scale;
  MultiHist<TH2F> m_DSSD_Sector_VS_ring_polar_scale_smooth;

  // Two events within 300ns
  MultiHist<TH2F> m_DSSD_Spectra_each_channel_last;
  MultiHist<TH2F> m_DSSD_Spectra_each_channel_current;


  //   //Energy cube
  // std::unique_ptr<TH3I> clean_clover_cube;
  // std::unique_ptr<TH3F> LaBr3_cube;
  // std::unique_ptr<TH3F> clover_clover_LaBr3;
  // std::unique_ptr<TH3F> clover_LaBr3_LaBr3;
    //Timing spectra
  MultiHist<TH1F> m_Ref_VS_RF;
  MultiHist<TH1F> m_RF_time_diff;
  MultiHist<TH1F> m_RF_freq;
  MultiHist<TH2F> timeshift_all;
  // std::unique_ptr<TH1D> evolution_RF;
  MultiHist<TH2F> m_E_VS_time_of_Ref_vs_RF;
    //Neutrons
  MultiHist<TH1F> m_EDEN_ratio;
  MultiHist<TH2F> m_EDEN_Q1_vs_Q2;
  MultiHist<TH1F> m_EDEN_VS_RF;
  MultiHist<TH2F> m_EDEN_VS_RF_2D;
  MultiHist<TH2F> m_EDEN_totE_VS_RF;
    //Other bidim
  MultiHist<TH2S> m_clover_counter_matrice;
  MultiHist<TH2S> m_clover_polar;
  MultiHist<TH2S> m_Paris_xy;
  // MultiHist<TH2S> m_clover_cross_talk;
  MultiHist<TH1I> m_gated_counter;
    //Counter
  MultiHist<TH1I> m_Counters       ;
  MultiHist<TH1I> m_Clover_Counter;
  MultiHist<TH1I> m_Gated_Counter;
  MultiHist<TH2I> m_C_VS_L;
    //Other
  MultiHist<TH1I> m_mult_raw;
  MultiHist<TH1I> m_mult_module;
  MultiHist<TH1I> m_mult_clover;
  MultiHist<TH1I> m_mult_paris;
  MultiHist<TH1I> m_mult_Clean_Ge;
  MultiHist<TH1I> m_mult_BGO;
  MultiHist<TH1F> m_gated_sum_given_pic; //somme(Qn) C {gate}

  TRandom* gRandom;
};

Bool_t Analyse_W::SetConfig(std::istringstream & is)
{
  gRandom = new TRandom(time(0));
  std::string temp;
  while(is >> temp)
  {
         if (temp == "time_ref:")
    {
      is >> m_time_ref_det;
      if (m_time_ref_det == "") {std::cout << "NO TIME REFERENCE FOR ANALYSIS !!" << std::endl; return false; }
      m_time_ref_label = n->m_nameToLabel [m_time_ref_det];
    }
    else if (temp == "gated_counter:")
    {
      is >> m_nb_gates;
      // m_gate_counter.resize(m_nb_gates);
      // for (int i = 0; i<m_nb_gates; i++)
      // {
      //   is >> m_gate_counter[i].first >> m_gate_counter[i].second;
      // }
    }
    else if (temp == "time_gated:" || temp == "time_gate:")
    {
      m_time_gated = true;
      is >> m_time_gate_min >> m_time_gate_max;
      m_time_gate_min *= 1000; //ns -> ps
      m_time_gate_max *= 1000; //ns -> ps
    }
    else if (temp == "timewindow:" || temp == "time_window:")
    {
      is >> m_timeWindow_ns;
      m_timeWindow = m_timeWindow_ns*1000;
    }
    else if (temp == "outRoot:"){ is >> m_outroot; }
    else {std::cout << temp << " not a good parameter for analysis... Please check the parameters file" << std::endl; return false;}
  }
  print(
    "Perform data analysis :",
    (n->m_use_RF) ? "\nwith event building based on RF\n" : "\nwith event building based on coincidences with "+std::to_string(m_timeWindow/_ns)+" ns time gate",
    (m_time_gated) ? "\nwith a selection time gate between "+std::to_string(m_time_gate_min/_ns)+" ns and "+std::to_string(m_time_gate_max/_ns)+" ns" : ""
  );
  return true;
}

Bool_t Analyse_W::Check()
{
  m_outdir = n->m_outdir+"Analyse_W/";
  if (!folder_exists(getPath(m_outdir+outroot()))) {print("ERROR, OUT FOLDER",getPath(m_outdir+outroot()),"DOESN'T EXIST !"); return false;}
  if (m_outroot == "") {std::cout << "ANALYSIS : ERROR : NO OUT FILE NAME, check " << n->m_parameters_filename << std::endl; return false;}
  if (n->m_calib.size() < 1) {std::cout << "ANALYSIS : ERROR : NO CALIBRATION DATA, check " << n->m_parameters_filename << std::endl; return false;}
  if (!n->m_use_RF && m_time_gated) {print("CANT GATE ON PROMPT WITH COINCIDENCE YET, ONLY WITH RF");return false;}
  return true;
}

void Analyse_W::Fill(Event const & event, UShort_t const & thread_nb)
{
  Sorted_Event & arg = evt_cntr[thread_nb];
  if (event.size() == 1 && is_Clover_Ge(event.labels[0])) {m_Ge_Single_Spectra[thread_nb] -> Fill(event.nrjs[0]);}
  m_mult_raw[thread_nb] -> Fill(event.size());
  treat_event(event, arg, thread_nb);
  if(arg.DSSDSectorMult!=1 && arg.DSSDRingMult!=1) return;
  // #ifdef N_SI_129
  // if (arg.DSSDMult==0) return;
  // #endif //N_SI_129
  m_C_VS_L[thread_nb] -> Fill((arg.CleanGeMult<4) ? arg.CleanGeMult : 4, (arg.LaBr3Mult<4) ? arg.LaBr3Mult : 4);
  m_mult_module[thread_nb] -> Fill(arg.ModulesMult);
  m_mult_clover[thread_nb] -> Fill(arg.CloverMult);
  m_mult_paris[thread_nb] -> Fill(arg.ParisMult);
  m_mult_Clean_Ge[thread_nb] -> Fill(arg.CleanGeMult);
  m_mult_BGO[thread_nb] -> Fill(arg.BGOMult);

  // Raw coincidence spectra :
  for (uchar i = 0; i<event.size(); i++)
  {//Raw Coincidence
    m_Counters[thread_nb] -> Fill(event.labels[i]);
    #ifdef N_SI_129
    if (event.labels[i] == 252)
      for (size_t j = 0; j<event.size(); j++)
      {
        if (event.labels[j] != 252) timeshift_all[thread_nb] -> Fill((Long64_t)(event.times[j] - event.times[i])/1000., event.labels[j]);
      }
    #endif //N_SI_129
  }

  // --------------------------- //
  //      Germanium sorting      //
  // --------------------------- //
  for (size_t loop_i = 0; loop_i<arg.clover_hits.size(); loop_i++)
  {
    size_t clover_i = arg.clover_hits[loop_i];
    auto const & nrj_i = arg.nrj_clover[clover_i];
    m_Spectra_all_Ge_Clover[thread_nb] -> Fill(clover_i, nrj_i);

    // A few information of the current hit :
    Float_t theta = 0.523*(clover_i%12 + gRandom->Uniform(0.1,0.9)); //0.523 = 2*pi/12
    Float_t r = gRandom->Uniform(0,1)+1;

    // ________________________ //

    // One dimensionnal plots : //
    // ________________________ //

    // 1. Only addback :
    m_Spectra_all_Clover[thread_nb] -> Fill(clover_i, nrj_i);
    m_Ge_addback[thread_nb] -> Fill(nrj_i);
    m_Clover_Counter[thread_nb] -> Fill(clover_i);

    // 2. Compton vetoed hits (garbage)
    if (arg.BGO[clover_i])
    {
      m_Compton_garbage[thread_nb] -> Fill(nrj_i);
    }
    // 3. Clean Germanium :
    else
    {
      m_Clean_Ge[thread_nb] -> Fill(nrj_i);
      switch(arg.ModulesMult)
      {
        case 0 : print("chelou"); break;
        case 1 : m_Ge_M1[thread_nb] -> Fill(nrj_i); break;
        case 2 : m_Ge_M2[thread_nb] -> Fill(nrj_i); break;
        case 3 : m_Ge_M3[thread_nb] -> Fill(nrj_i); m_Ge_M3_6[thread_nb] -> Fill(nrj_i); break;
        case 4 : m_Ge_M4[thread_nb] -> Fill(nrj_i); m_Ge_M3_6[thread_nb] -> Fill(nrj_i); break;
        case 5 : m_Ge_M5[thread_nb] -> Fill(nrj_i); m_Ge_M3_6[thread_nb] -> Fill(nrj_i); break;
        case 6 : m_Ge_M6[thread_nb] -> Fill(nrj_i); m_Ge_M3_6[thread_nb] -> Fill(nrj_i); break;
        default : m_Ge_Msup6[thread_nb] -> Fill(nrj_i); break;
      }
      if (clover_i<13)
      {
        m_R3_Spectra[thread_nb] -> Fill(nrj_i);
      }
      else
      {
        m_R2_Spectra[thread_nb] -> Fill(nrj_i);
      }
      // Polar counters :
      m_clover_polar[thread_nb] -> Fill(((clover_i<12) ? -3 : 3)+r*sin(theta), r*cos(theta));

      #ifdef USE_DSSD
      if (arg.DSSDMult>0)
      {
        m_DSSD_gate_Clover[thread_nb] -> Fill(nrj_i);
        //Prompt particle gating :
        // if (arg.DSSDPrompt)
        // {
        //   m_prompt_particule_gate_Clover_spectra[thread_nb] -> Fill(nrj_i, arg.time_clover[clover_i]);
        // }

        if (arg.delayed_Ge[clover_i]) m_DSSD_gate_Clover_delayed[thread_nb] -> Fill(nrj_i);
        else m_DSSD_gate_Clover_prompt[thread_nb] -> Fill(nrj_i);

        // Gate with 236U : 642 keV, 687 keV, 159 keV
        if ( gate(nrj_i,639,645) || gate(nrj_i,684,690) || gate(nrj_i,156,163) )
        {
          for (auto DSSD_hit : arg.DSSD_hits)
          {
            m_DSSD_Tof_VS_Energy_U6[thread_nb] -> Fill(event.nrjs[DSSD_hit], event.times[DSSD_hit]);
            m_DSSD_E_gate_236U[thread_nb] -> Fill(event.nrjs[DSSD_hit]);
          }
        }
        // Gate with Be9: broad peak at 3370
        else if (gate(nrj_i,3340,3400))
        {
          for (auto DSSD_hit : arg.DSSD_hits) m_DSSD_E_gate_9Be[thread_nb] -> Fill(event.nrjs[DSSD_hit]);
        }
      }
      #endif //USE_DSSD
    }//End Clean Germanium

    // ________________________ //

    // Two dimensionnal plots : //
    // ________________________ //

    for (size_t loop_j = loop_i+1; loop_j<arg.clover_hits.size(); loop_j++)
    {
      // 1. Only addback :
      auto const & clover_j = arg.clover_hits[loop_j];
      auto const & nrj_j = arg.nrj_clover[clover_j];
      m_clover_clover[thread_nb] -> Fill(nrj_i,nrj_j);
      m_clover_clover[thread_nb] -> Fill(nrj_j,nrj_i);
      m_clover_counter_matrice[thread_nb] -> Fill(clover_i, clover_j);
      m_clover_counter_matrice[thread_nb] -> Fill(clover_j, clover_i);

      // 2. Clean Germanium :
      if (!arg.BGO[clover_j])
      {
        m_clover_clover_compt_supp[thread_nb] -> Fill(nrj_i, nrj_j);
        m_clover_clover_compt_supp[thread_nb] -> Fill(nrj_j, nrj_i);
        if (arg.ModulesMult>2)
        {
          m_bidim_clover_clean_M_sup_3[thread_nb] -> Fill(nrj_i, nrj_j);
          m_bidim_clover_clean_M_sup_3[thread_nb] -> Fill(nrj_j, nrj_i);
        }
        #ifdef USE_DSSD
        if (arg.DSSDMult > 0)
        {
          // Float_t particle_time =
          m_DSSD_gate_Clover_bidim[thread_nb] -> Fill(nrj_i,nrj_j);
          m_DSSD_gate_Clover_bidim[thread_nb] -> Fill(nrj_j,nrj_i);

          if (arg.prompt_Ge[clover_i] && arg.delayed_Ge[clover_j])
          {
            m_particule_gated_Clover_prompt_VS_delayed[thread_nb] -> Fill(nrj_i,nrj_j);
          }
        }
        #endif //dssd
      }
    }

    #ifdef FATIMA
    for (size_t labr3 = 0; labr3<arg.LaBr3_event.size(); labr3++)
    {
      m_clover_LaBr3[thread_nb] -> Fill(nrj_i, arg.LaBr3_event[labr3].nrjcal);
    }
    #endif //FATIMA

    #ifdef PARIS
    for (auto const & paris : arg.paris_hits)
    {
      m_clover_Paris[thread_nb] -> Fill(nrj_i, event.nrjs[paris]);
    }
    #endif //PARIS

    #ifdef USE_DSSD
    for (auto const & dssd : arg.DSSD_hits)
    {
      m_clover_VS_DSSD[thread_nb] -> Fill(nrj_i, event.nrjs[dssd]);

      switch(event.labels[dssd])
      {
        case 843: m_DSSD_R3_VS_Clover[thread_nb] -> Fill(nrj_i, event.nrjs[dssd]); break;
        case 844: m_DSSD_R4_VS_Clover[thread_nb] -> Fill(nrj_i, event.nrjs[dssd]); break;
        case 845: m_DSSD_R5_VS_Clover[thread_nb] -> Fill(nrj_i, event.nrjs[dssd]); break;
      }

    }
    #endif //PARIS
  }

    // ----------------------- //
    //      LaBr3 sorting      //
    // ----------------------- //

  #ifdef FATIMA
  for (size_t i = 0; i<arg.LaBr3_event.size(); i++)
  {
    auto const & lHit_i = arg.LaBr3_event[i];
    m_Spectra_all_LaBr3[thread_nb] -> Fill(lHit_i.label, lHit_i.nrjcal);
    if (arg.LaBr3_event[i].label<11) m_R1_Spectra[thread_nb] -> Fill(lHit_i.nrjcal);
    else m_R4_Spectra[thread_nb] -> Fill(lHit_i.nrjcal);
    m_LaBr3_Spectra[thread_nb] -> Fill(lHit_i.nrjcal);
    for (size_t j = i+1; j<arg.LaBr3_event.size(); j++)
    {
      auto const & lHit_j = arg.LaBr3_event[j];
      m_LaBr3_LaBr3[thread_nb] -> Fill(lHit_i.nrjcal, lHit_j.nrjcal);
      m_LaBr3_LaBr3[thread_nb] -> Fill(lHit_j.nrjcal, lHit_i.nrjcal);
    }
  }
  #endif //FATIMA

  // ----------------------- //
  //      paris sorting      //
  // ----------------------- //

  //paris events :
  // #ifdef PARIS
  // for (size_t loop_i = 0; loop_i<arg.paris_hits.size(); loop_i++)
  // {
  //   auto const paris_i = arg.paris_hits[loop_i];
  //   auto const nrj_i = event.nrjs[paris_i];
  //   ParisLabel label_i (event.labels[paris_i]);
  //   if (label_i.back)
  //   {
  //     m_Back_Spectra[thread_nb]->Fill(nrj_i);
  //     switch(label_i.ring)
  //     {
  //       case 1:
  //         m_BR1_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //       case 2:
  //         m_BR2_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //       case 3:
  //         m_BR3_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //     }
  //   }
  //   else
  //   {
  //     m_Front_Spectra[thread_nb]->Fill(nrj_i);
  //     switch(label_i.ring)
  //     {
  //       case 1:
  //         m_FR1_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //       case 2:
  //         m_FR2_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //       case 3:
  //         m_FR3_Spectra[thread_nb]->Fill(nrj_i);
  //         break;
  //     }
  //   }
  //   Paris_XY xy(label_i);
  //   m_Paris_xy[thread_nb] -> Fill( ((label_i.back) ? -10 : 10) + xy.x(), xy.y());
  // }
  // #endif //PARIS

    // ---------------------- //
    //      dssd sorting      //
    // ---------------------- //

  #ifdef USE_DSSD

  if (arg.DSSDMult>0)
  {
    m_DSSD_mult_Ring[thread_nb] -> Fill(arg.DSSDMult);
    m_DSSD_mult_Sector[thread_nb] -> Fill(arg.DSSDSectorMult);
    m_DSSD_mult[thread_nb] -> Fill(arg.DSSDRingMult);
    m_DSSD_mult_Sector_VS_mult_Ring[thread_nb] -> Fill(arg.DSSDRingMult, arg.DSSDSectorMult);

    if (arg.DSSDRingMult == 2)
    {
      Float_t sum = 0;
      for (auto const & ring : arg.DSSD_Rings) sum += event.nrjs[ring];
      m_DSSD_sum_2_rings[thread_nb] -> Fill(sum);
    }

    else if (arg.DSSDSectorMult == 2)
    {
      Float_t sum = 0;
      for (auto const & sector : arg.DSSD_Sectors) sum += event.nrjs[sector];
      m_DSSD_sum_2_sectors[thread_nb] -> Fill(sum);
    }

    else if (arg.DSSDSectorMult == 1 && arg.DSSDRingMult == 1)
    {

      Float_t Ring_nrj   = event.nrjs  [arg.DSSD_Rings[0]];
      Label   Ring_label = event.labels[arg.DSSD_Rings[0]]-800;

      Float_t Sector_nrj   = event.nrjs  [arg.DSSD_Sectors[0]];
      Label   Sector_label = event.labels[arg.DSSD_Sectors[0]]-800;

      m_DSSD_Spectra_each_channel_1R1S[thread_nb] -> Fill(Ring_label, Ring_nrj);
      m_DSSD_Spectra_each_channel_1R1S[thread_nb] -> Fill(Sector_label, Sector_nrj);

      switch (Ring_label)
      {
        case 40:
          m_DSSD_R_0_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 41:
          m_DSSD_banana_R1_allsectors_1R1S[thread_nb] -> Fill(Ring_nrj+Sector_nrj, Ring_nrj);
          m_DSSD_R_1_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 42:
          m_DSSD_R_2_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 43:
          m_DSSD_R_3_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 44:
          m_DSSD_R_4_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 45:
          m_DSSD_R_5_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 46:
          m_DSSD_R_6_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 47:
          m_DSSD_R_7_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 48:
          m_DSSD_R_8_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 49:
          m_DSSD_R_9_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 50:
          m_DSSD_R_10_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 51:
          m_DSSD_R_11_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 52:
          m_DSSD_R_12_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 53:
          m_DSSD_R_13_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 54:
          m_DSSD_R_14_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
        case 55:
          m_DSSD_R_15_VS_Sectors_1R1S[thread_nb] -> Fill(Sector_nrj, Ring_nrj);
          break;
      }
    }

    for (size_t loop_i = 0; loop_i < arg.DSSD_hits.size(); loop_i++)
    {
      auto const DSSD_i = arg.DSSD_hits[loop_i];
      auto const time_i = (n->m_use_RF) ? time_RF(event.times[DSSD_i], thread_nb) : event.times[DSSD_i]-event.times[0];
      auto const nrj_i = event.nrjs[DSSD_i];
      auto const label_i = event.labels[DSSD_i]-800;

      m_DSSD_Spectra_each_channel[thread_nb] -> Fill(label_i, nrj_i);
      m_DSSD_Tof_VS_Energy[thread_nb] -> Fill(time_i, nrj_i);

      for (auto const & i_clover : arg.clover_hits)
      {
        if (time_i>20 && time_i<50 && nrj_i<11000 && nrj_i>8000)
        {
          m_DSSD_ToF_first_blob[thread_nb] -> Fill(arg.nrj_clover[i_clover], arg.time_clover[i_clover]);
        }
        if (time_i>35 && time_i<60 && nrj_i<7000 && nrj_i>5000)
        {
          m_DSSD_ToF_second_blob[thread_nb] -> Fill(arg.nrj_clover[i_clover], arg.time_clover[i_clover]);
        }
        if (time_i>40 && time_i<100 && nrj_i<5000 && nrj_i>1000)
        {
          m_DSSD_ToF_third_blob_wider[thread_nb] -> Fill(arg.nrj_clover[i_clover], arg.time_clover[i_clover]);
          if (time_i<70 && nrj_i>3000)
            m_DSSD_ToF_third_blob[thread_nb] -> Fill(arg.nrj_clover[i_clover], arg.time_clover[i_clover]);
        }
      }

      m_DSSD_Counter[thread_nb] -> Fill(label_i);

      // Rings and sectors :
      if (arg.DSSD_is_Ring[loop_i])
      {
        m_DSSD_Rings_mult_VS_spectra[thread_nb] -> Fill(nrj_i, arg.DSSDRingMult);
      }
      else
      {
        m_DSSD_Sectors_mult_VS_spectra[thread_nb] -> Fill(nrj_i, arg.DSSDSectorMult);
      }

      for (size_t loop_j = loop_i+1; loop_j<arg.DSSDMult; loop_j++)
      {
        auto const DSSD_j = arg.DSSD_hits[loop_j];
        auto const time_j = (n->m_use_RF) ? time_RF(event.times[DSSD_j], thread_nb) : event.times[DSSD_i]-event.times[0];
        auto const nrj_j = event.nrjs[DSSD_j];
        auto const label_j = event.labels[DSSD_j]-800;

        m_DSSD_Time_bidim[thread_nb] -> Fill(time_i, time_j);
        m_DSSD_Time_bidim[thread_nb] -> Fill(time_j, time_i);

        if (arg.DSSD_is_Ring[loop_i])
        { // Ring coincidence
          if (arg.DSSD_is_Ring[loop_j])
          {
            m_DSSD_crosstalk_Ring[thread_nb] -> Fill(label_i, label_j);
            m_DSSD_crosstalk_Ring[thread_nb] -> Fill(label_j, label_i);
          }
          else
          {
            fillCoincRingSector(label_i,label_j, thread_nb);
          }
        }

        else if (!arg.DSSD_is_Ring[loop_i] )
        { // Sectors coincidence
          if (!arg.DSSD_is_Ring[loop_j])
          {
            m_DSSD_crosstalk_Sector[thread_nb] -> Fill(label_i, label_j);
            m_DSSD_crosstalk_Sector[thread_nb] -> Fill(label_j, label_i);
          }
          else
          {
            fillCoincRingSector(label_j,label_i, thread_nb);
          }
        }

        if (label_i == 51)
        {
          if (label_j == 50)
          {
            m_DSSD_R10_R11[thread_nb] -> Fill(nrj_i, nrj_j);
          }
          else if (label_j == 1)
          {
            m_DSSD_R1_S1_1[thread_nb] -> Fill(nrj_i, nrj_j);
          }
        }
        if (label_i == 49)
        {
          if (label_j == 50)
          {
            m_DSSD_R9_R10[thread_nb] -> Fill(nrj_i, nrj_j);
          }
          else if (label_j == 48)
          {
            m_DSSD_R8_R9[thread_nb] -> Fill(nrj_j, nrj_i);
          }
        }
      }
    }
  }
  #endif //dssd
}

void Analyse_W::fillCoincRingSector(Label const & Ring, Label const & Sector, int const & thread_nb)
{
  m_DSSD_coinc_ring_sector[thread_nb] -> Fill(Ring, Sector);

  Float_t r = 55-Ring;
  Float_t r_smooth = r + gRandom->Uniform(-1,1)/2;
  r+=gRandom->Uniform(-1,1)*0.4;

  int label_sector = (Sector<16) ? (Sector) : (Sector-4);
  Float_t theta = 0.19634975*label_sector; // 0,19634975 = 2*pi/32
  Float_t theta_smooth = theta+0.19634975*gRandom->Uniform(0,1);
  theta += 0.19634975*gRandom->Uniform(0.05,0.95);
  m_DSSD_Sector_VS_ring_polar[thread_nb] -> Fill(r*TMath::Cos(theta), r*TMath::Sin(theta));
  r = 1.5+r*(4.1-1.5)/15;
  r_smooth=1.5+r_smooth*(4.1-1.5)/15;
  m_DSSD_Sector_VS_ring_polar_scale[thread_nb] -> Fill(r*TMath::Cos(theta), r*TMath::Sin(theta));
  m_DSSD_Sector_VS_ring_polar_scale_smooth[thread_nb] -> Fill(r_smooth*TMath::Cos(theta_smooth), r_smooth*TMath::Sin(theta_smooth));
}

void Analyse_W::FillTiming(Hit const & hit, UShort_t const & thread_nb)
{
  Float_t ToF = time_RF(hit.time, thread_nb); //ns
  m_ToF_spectra[thread_nb] -> Fill(ToF, hit.label);

  #ifdef LICORNE
  if (is_EDEN(hit.label))
  {
    m_EDEN_ratio     [thread_nb] -> Fill(hit.nrjcal);
    m_EDEN_VS_RF_2D  [thread_nb] -> Fill(ToF, hit.nrjcal);
    m_EDEN_totE_VS_RF[thread_nb] -> Fill(ToF, hit.nrj);

    //Neutrons enery :
    Float_t EDEN_T0 = GAMMA_FLASH-EDEN_TO_LICORNE_DISTANCE/29.9792458;
    Float_t ToF_neutrons = ToF-EDEN_T0;
    m_EDEN_VS_RF[thread_nb] -> Fill(ToF_neutrons);
    if (ToF_neutrons>EDEN_T0+1)
    {
      Float_t v = EDEN_TO_LICORNE_DISTANCE/(ToF_neutrons);
      m_Neutrons_Spectra[thread_nb] -> Fill(0.5*939.5654*v*v);
    }
  }
  #endif //LICORNE

  #ifdef FATIMA
  if (isLaBr3[hit.label])
  {//COMPARE RF WITH LaBr3
    if (hit.label == m_time_ref_label) m_Ref_VS_RF[thread_nb] -> Fill(ToF);
    if(hit.nrjcal > n->m_E_threshold) m_E_VS_time_of_Ref_vs_RF[thread_nb]->Fill(ToF,hit.nrjcal);
  }
  #endif //FATIMA

  #ifdef PARIS
  if (isParis[hit.label])
  {
    if (hit.label == m_time_ref_label) m_Ref_VS_RF[thread_nb] -> Fill(ToF);
  }
  #endif //PARIS
}

void Analyse_W::treat_event(Event const & event, Sorted_Event & arg, UShort_t const & thread_nb)
{
  arg.reset();
  uchar clover_label = 0;
  for (uchar i = 0; i<event.size(); i++)
  {
    if (isGe[event.labels[i]])
    {//AddBack
      if (event.nrjs[i]<5) continue;
      clover_label = labelToClover_fast[event.labels[i]];
      arg.nrj_clover[clover_label] += event.nrjs[i];
      if (n->m_use_RF) arg.time_clover[clover_label] = m_rf[thread_nb]->pulse_ToF(event.times[i], n->m_RF_shift)/_ns;
      else arg.time_clover[clover_label] = (event.times[i]-event.times[0])/_ns;
      if (arg.time_clover[clover_label]>-10 && arg.time_clover[clover_label]<70) arg.prompt_Ge[clover_label] = true;
      else if (arg.time_clover[clover_label]>70) arg.delayed_Ge[clover_label] = true;
      arg.Ge[clover_label]++;
      // arg.crystals_labels.push_back(clover_label*4+labelToCloverCrystal_fast[event.labels[i]]);
      // arg.nrj_crystals[arg.crystals_labels.back()] = event.nrjs[i];
      std::vector<uchar>::iterator finder = std::find(std::begin(arg.clover_hits), std::end(arg.clover_hits), clover_label);
      if (finder == std::end(arg.clover_hits)) arg.clover_hits.push_back(clover_label);
      arg.RawGeMult++;
    }
    else if (isBGO[event.labels[i]])
    {
      clover_label = labelToClover_fast[event.labels[i]];
      if (n->m_use_RF) arg.time_clover[clover_label] = m_rf[thread_nb]->pulse_ToF(event.times[i], n->m_RF_shift)/_ns;
      else arg.time_clover[clover_label] = (event.times[i]-event.times[0])/_ns;
      arg.BGO[clover_label]++;
    }
    else if (isLaBr3[event.labels[i]])
    {
      LaBr3_Hit labr3_hit;
      labr3_hit.nrjcal = event.nrjs[i];
      labr3_hit.label = event.labels[i]-199;
      labr3_hit.time = event.times[i];
      arg.LaBr3_event.push_back(labr3_hit);
      arg.ModulesMult++;
      arg.LaBr3Mult++;
    }
    else if (isParis[event.labels[i]])
    {
      arg.paris_hits.push_back(i);
      arg.ModulesMult++;
      arg.ParisMult++;
    }
    else if (isDSSD[event.labels[i]] && event.nrjs[i]>500)
    {
      arg.DSSD_hits.push_back(i);
      // auto const time = time_RF(event.times[i], thread_nb);
      // if (time>-10 && time<70) arg.DSSDPrompt = true;
      bool isRing = isDSSD_Ring[event.labels[i]];
      arg.DSSD_is_Ring.push_back(isRing);
      if (isRing)
      {
        arg.DSSD_Rings.push_back(i);
        arg.DSSDRingMult++;
      }
      else
      {
        arg.DSSD_Sectors.push_back(i);
        arg.DSSDSectorMult++;
      }
      arg.DSSDMult++;
    }
  }
  for (size_t i = 0; i<24l; i++)
  {
    if (arg.Ge[i])
    {
      arg.CloverMult++;
      arg.ModulesMult++;
      if (arg.BGO[i]) arg.ComptonVetoMult++;
      else arg.CleanGeMult++;
    }
    else if (arg.BGO[i])
    {
      arg.CloverMult++;
      arg.ModulesMult++;
      arg.BGOMult++;
    }
  }
}

Bool_t Analyse_W::Initialise()
{
  evt_cntr.resize(MTObject::nb_threads);
  m_rf.resize(MTObject::nb_threads, nullptr);
  #ifdef FATIMA
  // m_DeltaT_LaBr3.reset("deltaT LaBr3","deltaT LaBr3", m_timeWindow_ns+2,-1,_ns) ;
  // m_DeltaT_LaBr3_ref.reset("deltaT LaBr3 ref","deltaT LaBr3 ref", m_timeWindow_ns*10,-1,m_timeWindow_ns) ;
  #endif //FATIMA
  // m_DeltaT.reset( "deltaT total","deltaT total",m_timeWindow_ns+2,-1,m_timeWindow_ns);
  // m_label_DeltaT.reset("DeltaT all detectors","DeltaT all detectors", m_timeWindow_ns+1,0,m_timeWindow_ns, n->m_labelToName.size()+1,0,n->m_labelToName.size()) ;
  m_ToF_spectra.reset("ToF all detectors","ToF all detectors", 4000,-100,400, n->m_labelToName.size()+1,0,n->m_labelToName.size()) ;
  // m_label_DeltaT_RF_2.reset("ToF all detectors 2","ToF all detectors 2", 10000,-1000,1000, n->m_labelToName.size()+1,0,n->m_labelToName.size()) ;
  // m_label_DeltaT_RF_3.reset("ToF all detectors 3","ToF all detectors 3", 10000,-1000,1000, n->m_labelToName.size()+1,0,n->m_labelToName.size()) ;
  // m_deltaT_energy.reset("deltaT energy total","deltaT energy total", m_timeWindow_ns+2,-1,m_timeWindow_ns, n->m_bins_bidim[Ge]/2,n->m_min_bidim[Ge],n->m_max_bidim[Ge] );

  // Energy spectra :
  m_Ge_raw.reset("Raw spectra","Raw spectra",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_prompt.reset("Prompt Clover spectra","Prompt Clover spectra",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_addback.reset("Clover Add-Back Spectra","Clover Add-Back Spectra",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Clean_Ge.reset("Compton Clean","Compton Clean",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Compton_garbage.reset("Compton veto(garbage)","Compton veto",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_Single_Spectra.reset("Single Germanium","Single Germanium",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);
  m_Ge_M1.reset("Multiplicity 1","Multiplicity 1",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M2.reset("Multiplicity 2","Multiplicity 2",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M3.reset("Multiplicity 3","Multiplicity 3",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M4.reset("Multiplicity 4","Multiplicity 4",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M5.reset("Multiplicity 5","Multiplicity 5",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M6.reset("Multiplicity 6","Multiplicity 6",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_Msup6.reset("Multiplicity > 6","Multiplicity > 6",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_Ge_M3_6.reset("Multiplicity between 3 and 7","Multiplicity between 3 and 7",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;

  // Rings :
  m_R2_Spectra.reset("Sum R2", "Sum R2", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  m_R3_Spectra.reset("Sum R3", "Sum R3", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  // m_R2_Spectra_Doppler_correct.reset("R2 Doppler shifted", "R2  Doppler shifted", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  // m_R3_Spectra_Doppler_correct.reset("R3 Doppler shifted", "R3  Doppler shifted", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;

  #ifdef FATIMA
  m_LaBr3_Spectra.reset("Sum LaBr3", "Sum LaBr3", n->m_bins_calib[LaBr3],n->m_min_calib[LaBr3],n->m_max_calib[LaBr3]) ;
  #endif //FATIMA

  #if defined FATIMA || defined PHASEI
  m_R1_Spectra.reset("Sum R1", "Sum R1", n->m_bins_calib[LaBr3],n->m_min_calib[LaBr3],n->m_max_calib[LaBr3]) ;
  m_R4_Spectra.reset("Sum R4", "Sum R4", n->m_bins_calib[LaBr3],n->m_min_calib[LaBr3],n->m_max_calib[LaBr3]) ;
  #endif //FATIMA || PHASEI

  #if defined PARIS
  m_Back_Spectra.reset("Sum Back paris", "Sum Back PARIS", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_Front_Spectra.reset("Sum Front paris", "Sum Front PARIS", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_BR1_Spectra.reset("Sum Back R1 paris", "Sum PARIS Back R1", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_BR2_Spectra.reset("Sum Back R2 paris", "Sum PARIS Back R2", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_BR3_Spectra.reset("Sum Back R3 paris", "Sum PARIS Back R3", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_FR1_Spectra.reset("Sum Front R1 paris", "Sum PARIS Front R1", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_FR2_Spectra.reset("Sum Front R2 paris", "Sum PARIS Front R2", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  m_FR3_Spectra.reset("Sum Front R3 paris", "Sum PARIS Front R3", n->m_bins_calib[paris],n->m_min_calib[paris],n->m_max_calib[paris]) ;
  Ge_VS_time_Paris.reset("Ge time VS paris", "Ge time VS paris", 500,-100,400, n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]) ;
  #endif //PARIS

  // Energy bidim :
  m_gamma_gamma_single.reset("Gamma-gamma raw singles","#gamma-#gamma raw singles",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge]);

  m_clover_clover.reset("Gamma-gamma Clover","#gamma-#gamma Clover",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge]);

  m_clover_clover_compt_supp.reset("Clean Gamma-gamma Clover","Clean #gamma-#gamma Clover",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge]) ;

  m_bidim_clover_clean_M_sup_3.reset("Clean Gamma-gamma Clover Mult > 3","Clean #gamma-#gamma Clover Multiplicity > 3",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge]) ;


  #ifdef FATIMA
  m_clover_LaBr3.reset("Gamma-gammma Clean Clover VS LaBr3", "#gamma-#gamma Clean Clover VS LaBr3",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[LaBr3],n->m_min_bidim[LaBr3],n->m_max_bidim[LaBr3]) ;

  m_LaBr3_LaBr3.reset("Gamma-gamma LaBr3", "#gamma-#gamma LaBr3",
    n->m_bins_bidim[LaBr3],n->m_min_bidim[LaBr3],n->m_max_bidim[LaBr3], n->m_bins_bidim[LaBr3],n->m_min_bidim[LaBr3],n->m_max_bidim[LaBr3]) ;

  m_Spectra_all_LaBr3.reset("All LaBr3 spectra", "All LaBr3 spectra", 20,1,20,  n->m_bins_bidim[LaBr3],n->m_min_bidim[LaBr3],n->m_max_bidim[LaBr3]);
  #endif //FATIMA

  #ifdef PARIS
  m_clover_Paris.reset("Gamma-gammma Clean Clover VS paris", "#gamma-#gamma Clean Clover VS paris",
  n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[paris],n->m_min_bidim[paris],n->m_max_bidim[paris]) ;
  m_p_Paris_Ge_C1P1.reset("Ge VS paris M>1", "Ge VS paris M>1",
    n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[paris],n->m_min_bidim[paris],n->m_max_bidim[paris]) ;
  m_Spectra_all_Paris.reset("All paris spectra", "All paris spectra",
    92,1,92,  n->m_bins_bidim[paris],n->m_min_bidim[paris],n->m_max_bidim[paris]);
  #endif //PARIS

  m_Spectra_all_Ge_Clover.reset("All Clovers Ge spectra", "All Clovers Germanium spectra",
    Ge_Labels.size()-1, Ge_Labels.data(),  n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);

  m_Spectra_all_Clover.reset("All Clovers spectra", "All Clovers spectra",
    24,1,24, n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);

  //Timing
  // m_RF_freq.reset("Period RF","Period RF", 1000,399,401) ;
  // evolution_RF.reset( new TH1D("Evolution RF", "evolution_RF", 20000, -1000,500000) ;
  m_Ref_VS_RF.reset("Reference Detector VS RF","Reference Detector VS RF", 5200,-60,460) ;
  timeshift_all.reset("All Detectors VS ref det","All Detectors VS ref det", 1001,-500,500, 1000,0,1000) ;
  m_E_VS_time_of_Ref_vs_RF.reset("E gamma VS timing reference Detector","E #gamma VS RF ToF", 5200,-60,460,2000,0,4000) ;
  // m_RF_time_diff.reset("RF time diff","RF time difference", 20001,-1000,1000) ;

  //Neutrons
  #ifdef LICORNE
  m_Neutrons_Spectra.reset("Neutrons spectra", "Neutrons spectra", 1000,0,5000) ;
  m_EDEN_ratio.reset("EDEN Q1/Q2 ratio","EDEN Q1/Q2 ratio", 1000,-1,2);
  m_EDEN_Q1_vs_Q2.reset( "EDEN Q1 vs Q2 ","EDEN Q1 vs Q2", 500,-10000,100000, 500,-10000,100000);
  m_EDEN_VS_RF.reset("EDEN ToF","EDEN Time of Flight", 800,-100,400);
  m_EDEN_VS_RF_2D.reset("EDEN Q1/Q2 vs ToF","EDEN Q1/Q2 vs Time of Flight", 500,-100,400, 1000,-1,2);
  m_EDEN_totE_VS_RF.reset("EDEN Q2 vs ToF","EDEN Q2 vs Time of Flight", 500,-100,400, 1000,-1000,500000);
  #endif //LICORNE

  //multiplicity
  m_mult_raw.reset("Raw multiplicity", "Raw multiplicity", 50,0,50);
  m_mult_module.reset("Module multiplicity", "Module multiplicity", 50,0,50);
  m_mult_clover.reset("Clover multiplicity", "Clover multiplicity", 50,0,50);
  m_mult_paris.reset("paris multiplicity", "paris multiplicity", 50,0,50);
  m_mult_Clean_Ge.reset("Clean Ge multiplicity", "Clean Ge multiplicity", 50,0,50);
  m_mult_BGO.reset("BGO multiplicity", "BGO multiplicity", 50,0,50);
  #if defined FATIMA
  m_C_VS_L.reset ("C VS L","Clean Germanium Multiplicity VS LaBr3 Multiplicity", 5,0,5, 5,0,5);
  #elif defined PARIS
  m_C_VS_L.reset ("C VS P","Clean Germanium Multiplicity VS paris LaBr3 Multiplicity", 5,0,5, 5,0,5);
  #endif

  #ifdef USE_DSSD
  m_DSSD_gate_Clover.reset("Particle gated Ge Spectra","Particle gated Ge Spectra",n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);
  m_DSSD_gate_Clover_bidim.reset("Particle gated Ge Spectra bidim","Particle gated Ge Spectra bidim",
        n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge], n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge]);
  m_prompt_particule_gate_Clover_spectra.reset("Particle in prompt, Ge Spectra","Particle in prompt, Ge Spectra", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,500);
  m_clover_VS_DSSD.reset("dssd VS Clover","dssd VS Clover", n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge],n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R3_VS_Clover.reset("dssd R3 VS Clover","dssd R3 VS Clover", n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge],n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R4_VS_Clover.reset("dssd R4 VS Clover","dssd R4 VS Clover", n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge],n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R5_VS_Clover.reset("dssd R5 VS Clover","dssd R5 VS Clover", n->m_bins_bidim[Ge],n->m_min_bidim[Ge],n->m_max_bidim[Ge],n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_particule_gated_Clover_prompt_VS_delayed.reset("Particle gated, Clover Spectra prompt VS delayed","Particle gated, Clover Spectra prompt VS delayed", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,500);
  m_DSSD_gate_Clover_delayed.reset("Particle gated, delayed Ge Spectra","Particle gated, delayed Ge Spectra", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);
  m_DSSD_gate_Clover_prompt.reset("Particle gated, prompt Ge Spectra","Particle gated, prompt Ge Spectra", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge]);
  m_DSSD_E_gate_197Au.reset("Particle spectra gate on 197Au", "Particle spectra gate on 197Au",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_56Fe.reset("Particle spectra gate on 56Fe", "Particle spectra gate on 56Fe",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_52Cr.reset("Particle spectra gate on 52Cr", "Particle spectra gate on 52Cr",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_12C.reset("Particle spectra gate on 12C", "Particle spectra gate on 12C",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_16O.reset("Particle spectra gate on 16O", "Particle spectra gate on 16O",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_236U.reset("Particle spectra gate on 236U", "Particle spectra gate on 236U",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_E_gate_9Be.reset("Particle spectra gate on 9Be", "Particle spectra gate on 9Be",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_wide_E_gate_16O.reset("Particle spectra (wide) gate on 16O", "Particle spectra (wide) gate on 16O",n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);

  m_DSSD_mult.reset("dssd mult", "dssd multiplicity", 15, 0, 15);
  m_DSSD_mult_Ring.reset("dssd mult rings", "dssd multiplicity rings", 6, 0, 5);
  m_DSSD_mult_Sector.reset("dssd mult sectors", "dssd multiplicity sectors", 6, 0, 5);
  m_DSSD_mult_Sector_VS_mult_Ring.reset("dssd mult sectors VS mult rings", "dssd multiplicity sectors VS mult rings", 5,0,5, 5,0,5);
  m_DSSD_mult_Sector_VS_mult_Ring_E_sup_10_4.reset("dssd mult sectors VS mult rings pic alpha", "dssd multiplicity sectors VS mult rings pic alpha", 5,0,5, 5,0,5);

  m_DSSD_Counter.reset("dssd Counters", "dssd Counters", 56,0,57);

  m_DSSD_sum_2_rings.reset("dssd sum 2 Rings", "dssd sum 2 Rings", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);
  m_DSSD_sum_2_sectors.reset("dssd sum 2 Sectors", "dssd sum 2 Sectors", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd]);

  m_DSSD_Time_bidim.reset("dssd Time bidim", "Time bidim", 201,-50,350, 201,-50,350);
  m_DSSD_crosstalk_Sector.reset("dssd Cross talk Sector", "Cross talk Sector", 41,0,40, 41,0,40);
  m_DSSD_crosstalk_Ring.reset("dssd Cross talk Ring", "Cross talk Ring", 17,39,56, 17,39,56);
  m_DSSD_Rings_mult_VS_spectra.reset("Ring Mult VS Spectra", "Ring Mult VS Spectra", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd], 5,0,5);
  m_DSSD_Sectors_mult_VS_spectra.reset("Sector Mult VS Spectra", "Sector Mult VS Spectra", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd], 5,0,5);

  m_DSSD_coinc_ring_sector.reset("dssd coinc sector ring", "dssd coinc sector ring", 17,39,56, 41,0,40);
  m_DSSD_Sector_VS_ring_polar.reset("Polar coordinates", "Polar coordinates", 800,-20,20, 800,-20,20);
  m_DSSD_Sector_VS_ring_polar_scale.reset("Polar coordinates scaled", "Polar coordinates scaled", 800,-7,7, 800,-7,7);
  m_DSSD_Sector_VS_ring_polar_scale_smooth.reset("Polar coordinates scaled smooth", "Polar coordinates scaled smooth", 800,-7,7, 800,-7,7);

  m_DSSD_R_1_coinc.reset("R1 && sector", "Sector VS R1",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_0_VS_Sectors_1R1S.reset("coinc R0 && sector", "R0 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_1_VS_Sectors_1R1S.reset("coinc R1 && sector", "R1 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_2_VS_Sectors_1R1S.reset("coinc R2 && sector", "R2 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_3_VS_Sectors_1R1S.reset("coinc R3 && sector", "R3 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_4_VS_Sectors_1R1S.reset("coinc R4 && sector", "R4 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_5_VS_Sectors_1R1S.reset("coinc R5 && sector", "R5 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_6_VS_Sectors_1R1S.reset("coinc R6 && sector", "R6 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_7_VS_Sectors_1R1S.reset("coinc R7 && sector", "R7 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_8_VS_Sectors_1R1S.reset("coinc R8 && sector", "R8 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_9_VS_Sectors_1R1S.reset("coinc R9 && sector", "R9 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_10_VS_Sectors_1R1S.reset("coinc R10 && sector", "R10 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_11_VS_Sectors_1R1S.reset("coinc R11 && sector", "R11 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_12_VS_Sectors_1R1S.reset("coinc R12 && sector", "R12 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_13_VS_Sectors_1R1S.reset("coinc R13 && sector", "R13 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_14_VS_Sectors_1R1S.reset("coinc R14 && sector", "R14 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R_15_VS_Sectors_1R1S.reset("coinc R15 && sector", "R15 VS Sectors",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_banana_R1_allsectors.reset("banana R1 VS all sectors", "E R1 VS E R1 + E any sector ",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]*2, n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_banana_R1_allsectors_1R1S.reset("banana R1 VS all sectors coinc 1R&1S", "E R1 VS E R1 + E any sector if only 1 sector and 1 ring in coincidence",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]*2, n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);

  m_DSSD_Spectra_each_channel.reset("dssd spectra each channel ", "dssd spectra each channel", 56,0,56,
        n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  // m_merged_DSSD.reset("Merged dssd spectra", "Merged dssd spectra", 57,0,56, 2000,0,50000);
  m_DSSD_Spectra_each_channel_1R1S.reset("dssd spectra each channel 1 ring et 1 sector coinc","dssd spectra each channel 1 ring et 1 sector in coincidence",
        57,0,56, n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_Spectra_each_channel_coinc2Sectors.reset("dssd spectra each channel 2 sectors coinc", "dssd spectra each channel 2 sectors in coincidence",
        57,0,56, n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_Spectra_each_channel_coinc2Rings.reset("dssd spectra each channel 2 rings coinc", "dssd spectra each channel 2 rings in coincidence",
        57,0,56, n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);


  m_DSSD_R1.reset("dssd R1", "dssd R1", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_single_R1.reset("dssd single R1", "dssd single R1", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_M2_R1.reset("dssd MultR=2 R1", "dssd multiplicity rings = 2 R1", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_Msup1_R1.reset("dssd MultR>1 R1", "dssd multiplicity rings > 1 R1", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_single_Rings.reset("dssd single Rings", "dssd single Rings", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_single_Sectors.reset("dssd single Sectors", "dssd single Sectors", n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);
  m_DSSD_single_Sectors_1R1S_each_sector.reset("dssd single Sectors 1R1S coinc R1", "dssd single Sectors 1R1S coinc R1",
        16,0,16, n->m_bins_calib[dssd], n->m_min_calib[dssd], n->m_max_calib[dssd]);

  m_DSSD_R10_R11.reset("dssd R10 VS R11", "dssd R10 VS R11",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R9_R10.reset("dssd R9 VS R10", "dssd R9 VS R10",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R8_R9.reset("dssd R8 VS R9", "dssd R8 VS R9",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  m_DSSD_R1_S1_1.reset("dssd R1 VS S1_1", "dssd R1 VS S1_1",
        n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);

  //ToF
  m_DSSD_Tof_VS_Energy.reset("dssd ToF", "dssd ToF", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd], 601,-100,400);
  m_DSSD_Tof_VS_Energy_U6.reset("dssd ToF U6", "dssd ToF U6", n->m_bins_calib[dssd],n->m_min_calib[dssd],n->m_max_calib[dssd], 601,-100,400);
  m_DSSD_ToF_first_blob.reset("dssd first blob", "dssd first blob", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,400);
  m_DSSD_ToF_second_blob.reset("dssd second blob", "dssd second blob", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,400);
  m_DSSD_ToF_third_blob.reset("dssd third blob", "dssd third blob", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,400);
  m_DSSD_ToF_third_blob_wider.reset("dssd third blob wider", "dssd third blob wider", n->m_bins_calib[Ge],n->m_min_calib[Ge],n->m_max_calib[Ge], 601,-100,400);

  #endif //dssd

  // Counters:
  m_clover_counter_matrice.reset( "clovers counter matrice","clovers counter matrice", 24,0,23, 24,0,23);
  m_clover_polar.reset( "Clovers polar","Left R3 Right R2", 500,-6,6, 500,-3,3);
  m_Paris_xy.reset("Display of paris Counters", "paris Counters : left back, right front", 41,-20,20, 21,-10,10);
  // m_clover_cross_talk.reset( "Clovers polar cross talk","Clovers polar cross talk", 500,-7,7, 500,-2,2);
  m_Counters.reset ("Counter","Counter", n->m_labelToName.size(),0,n->m_labelToName.size());
  m_Clover_Counter.reset ("Counter clovers","Counter clovers", 23,0,23);
  // gated_deltaT.reset ( "deltaT_double_coinc", "deltaT_double_coinc", m_timeWindow_ns*2,0,m_timeWindow_ns, n->m_bins_bidim[LaBr3],n->m_min_bidim[LaBr3],n->m_max_bidim[LaBr3]));
  return true;
}

void Analyse_W::Write()
{
  //Writting spectra :
  print("Merging and writing histograms...");
  TFile* file;
  print(m_outroot);
  if(m_outroot == "")
  {
    print("RECOVERY MODE !! Writting to recovery_analyse.root");
    file = TFile::Open("recovery_analyse.root","recreate");
  }
  else file = TFile::Open((m_outdir+m_outroot).c_str(),"recreate");
  file -> cd();

  m_mult_raw.Write();
  m_mult_module.Write();
  m_mult_clover.Write();
  m_mult_paris.Write();
  m_mult_Clean_Ge.Write();
  m_mult_BGO.Write();

  //Timeshifts
  m_DeltaT.Write();
  m_ToF_spectra.Write();
  m_label_DeltaT_RF_2.Write();
  m_label_DeltaT_RF_3.Write();
  m_deltaT_energy.Write();
  m_label_DeltaT.Write();
  m_label_DeltaT_RF.Write();
  // gated_deltaT.Write();
  m_DeltaT_LaBr3.Write();
  m_DeltaT_LaBr3_ref.Write();

  //Spectra
  m_Spectra_all_LaBr3.Write();
  m_Spectra_all_Paris.Write();
  m_Spectra_all_Ge_Clover.Write();
  m_Spectra_all_Clover.Write();

  m_Ge_raw.Write();
  m_Ge_prompt.Write();
  m_Ge_addback.Write();
  m_Clean_Ge.Write();
  m_Compton_garbage.Write();
  m_LaBr3_Spectra.Write();
  m_Back_Spectra.Write();
  m_BR1_Spectra.Write();
  m_BR2_Spectra.Write();
  m_BR3_Spectra.Write();
  m_R1_Spectra.Write();
  m_R2_Spectra.Write();
  m_R3_Spectra.Write();
  m_R3_Spectra_Doppler_correct.Write();
  m_R4_Spectra.Write();
  m_Front_Spectra.Write();
  m_FR1_Spectra.Write();
  m_FR2_Spectra.Write();
  m_FR3_Spectra.Write();
  Ge_VS_time_Paris.Write();
  m_Neutrons_Spectra.Write();

  m_Ge_Single_Spectra.Write();
  m_Ge_M1.Write();
  m_Ge_M2.Write();
  m_Ge_M3.Write();
  m_Ge_M4.Write();
  m_Ge_M5.Write();
  m_Ge_M6.Write();
  m_Ge_Msup6.Write();
  m_Ge_M3_6.Write();
  //Gamma-gamma spectra
  m_gamma_gamma_single.Write();
  m_clover_clover.Write();
  m_clover_clover_compt_supp.Write();
  m_bidim_clover_clean_M_sup_3.Write();
  m_clover_LaBr3.Write();
  m_clover_Paris.Write();
  m_LaBr3_LaBr3.Write();
  m_Paris_Paris.Write();
  m_p_Paris_Ge_C1P1.Write();

  //dssd
  m_DSSD_gate_Clover.Write();
  //
  // std::ofstream outfileinfo("U_over_Be.info", std::ios_base::app);
  // int nbins = m_DSSD_gate_Clover->GetXaxis()->GetNbins();
  // Float_t scalefactor = m_DSSD_gate_Clover->GetXaxis()->GetXmax()/nbins;
  // Float_t background_642 = m_DSSD_gate_Clover -> GetBinContent(639/scalefactor)*6/scalefactor;
  // Float_t peak_642 = m_DSSD_gate_Clover -> Integral(639/scalefactor,645/scalefactor);
  // Float_t background_717 = m_DSSD_gate_Clover -> GetBinContent(714/scalefactor)*6/scalefactor;
  // Float_t peak_717 = m_DSSD_gate_Clover -> Integral(714/scalefactor,720/scalefactor);
  //
  // outfileinfo << removeExtension(n->p_Files.getListFolders()[0]) << (peak_642-background_642)/(peak_717-background_717)
  //   << " with " << m_DSSD_gate_Clover -> Integral() << " hits\n";
  // outfileinfo.close();


  m_DSSD_gate_Clover_bidim.Write();
  m_prompt_particule_gate_Clover_spectra.Write();
  m_particule_gated_Clover_prompt_VS_delayed.Write();
  m_clover_VS_DSSD.Write();
  m_DSSD_R3_VS_Clover.Write();
  m_DSSD_R4_VS_Clover.Write();
  m_DSSD_R5_VS_Clover.Write();
  m_DSSD_gate_Clover_delayed.Write();
  m_DSSD_gate_Clover_prompt.Write();
  m_DSSD_E_gate_197Au.Write();
  m_DSSD_E_gate_56Fe.Write();
  m_DSSD_E_gate_52Cr.Write();
  m_DSSD_E_gate_12C.Write();
  m_DSSD_E_gate_16O.Write();
  m_DSSD_E_gate_236U.Write();
  m_DSSD_E_gate_9Be.Write();
  m_DSSD_wide_E_gate_16O.Write();

  m_DSSD_mult.Write();
  m_DSSD_mult_Ring.Write();
  m_DSSD_mult_Sector.Write();
  m_DSSD_mult_Sector_VS_mult_Ring.Write();
  m_DSSD_mult_Sector_VS_mult_Ring_E_sup_10_4.Write();

  m_DSSD_sum_2_rings.Write();
  m_DSSD_sum_2_sectors.Write();

  m_DSSD_R_0_VS_Sectors_1R1S.Write();
  m_DSSD_R_1_VS_Sectors_1R1S.Write();
  m_DSSD_R_2_VS_Sectors_1R1S.Write();
  m_DSSD_R_3_VS_Sectors_1R1S.Write();
  m_DSSD_R_4_VS_Sectors_1R1S.Write();
  m_DSSD_R_5_VS_Sectors_1R1S.Write();
  m_DSSD_R_6_VS_Sectors_1R1S.Write();
  m_DSSD_R_7_VS_Sectors_1R1S.Write();
  m_DSSD_R_8_VS_Sectors_1R1S.Write();
  m_DSSD_R_9_VS_Sectors_1R1S.Write();
  m_DSSD_R_10_VS_Sectors_1R1S.Write();
  m_DSSD_R_11_VS_Sectors_1R1S.Write();
  m_DSSD_R_12_VS_Sectors_1R1S.Write();
  m_DSSD_R_13_VS_Sectors_1R1S.Write();
  m_DSSD_R_14_VS_Sectors_1R1S.Write();
  m_DSSD_R_15_VS_Sectors_1R1S.Write();
  m_DSSD_banana_R1_allsectors.Write();
  m_DSSD_banana_R1_allsectors_1R1S.Write();
  m_DSSD_Spectra_each_channel.Write();
  // m_merged_DSSD.Write();
  m_DSSD_Spectra_each_channel_1R1S.Write();
  m_DSSD_Spectra_each_channel_coinc2Sectors.Write();
  m_DSSD_Spectra_each_channel_coinc2Rings.Write();
  m_DSSD_Counter.Write();
  m_DSSD_crosstalk_Sector.Write();
  m_DSSD_crosstalk_Ring.Write();
  m_DSSD_Time_bidim.Write();
  m_DSSD_Rings_mult_VS_spectra.Write();
  m_DSSD_Sectors_mult_VS_spectra.Write();
  m_DSSD_coinc_ring_sector.Write();
  m_DSSD_Sector_VS_ring_polar.Write();
  m_DSSD_Sector_VS_ring_polar_scale.Write();
  m_DSSD_Sector_VS_ring_polar_scale_smooth.Write();
  m_DSSD_R1.Write();
  m_DSSD_single_R1.Write();
  m_DSSD_M2_R1.Write();
  m_DSSD_Msup1_R1.Write();
  m_DSSD_single_Rings.Write();
  m_DSSD_single_Sectors.Write();
  m_DSSD_single_Sectors_1R1S_each_sector.Write();
  m_DSSD_R10_R11.Write();
  m_DSSD_R9_R10.Write();
  m_DSSD_R8_R9.Write();
  m_DSSD_R1_S1_1.Write();
  m_DSSD_Tof_VS_Energy.Write();
  m_DSSD_Tof_VS_Energy_U6.Write();
  m_DSSD_ToF_first_blob.Write();
  m_DSSD_ToF_second_blob.Write();
  m_DSSD_ToF_third_blob.Write();
  m_DSSD_ToF_third_blob_wider.Write();

  //Counters
  m_Counters.Write();
  m_Clover_Counter.Write();
  m_C_VS_L.Merge();
  m_C_VS_L -> GetXaxis() -> SetTitle("LaBr3 multiplicity");
  m_C_VS_L -> GetYaxis() -> SetTitle("Clean Clover multiplicity");
  m_C_VS_L.Write();
  m_clover_counter_matrice.Write();
  m_clover_polar.Write();
  m_Paris_xy.Write();
  // m_clover_cross_talk.Write();
  //Other
  // m_gated_sum_give_pic.Write();
  // m_display_counts.Write();
  //Timing
  m_RF_freq.Write();
  m_Ref_VS_RF.Write();
  timeshift_all.Write();
  m_E_VS_time_of_Ref_vs_RF.Write();
  m_RF_time_diff.Write();
  // evolution_RF.Write();
  //Neutrons
  #ifdef LICORNE
  m_EDEN_ratio.Write();
  m_EDEN_Q1_vs_Q2.Write();
  m_EDEN_VS_RF.Write();
  m_EDEN_VS_RF_2D.Write();
  m_EDEN_totE_VS_RF.Write();
  #endif //LICORNE
  file -> Write();
  file -> Close();
  std::cout  << std::endl << "Data written to file " << m_outdir+m_outroot << std::endl;
  delete file;
}

void Analyse_W::Calculate()
{

}

Bool_t Analyse_W::isInTimeGate(Hit const & hit, int const & thread_nb)
{
  if (!m_time_gated) return true;
  Long64_t ToF = 0.;
  if (n->m_use_RF) ToF = m_rf[thread_nb]->pulse_ToF(hit.time,n->m_RF_shift);
  return ( ToF>m_time_gate_min &&  ToF<m_time_gate_max);
}

#endif //ANALYSE_W_H





// // Attempt to normalize
// TH1D *deltaT_energy_first_line = deltaT_energy -> ProjectionY("first_line", 1, 2);
// Double_t value = 0., N = 0., N2 = 0.;
// for (int x = 0; x< deltaT_energy->GetNbinsX(); x++)
// {
//   for (int y = 0; y<deltaT_energy->GetNbinsY(); y++)
//   {
//     N = deltaT_energy -> GetBinContent(x,y);
//     if (N < 1E-20) continue;
//     N2 = deltaT_energy_first_line -> GetBinContent(y);
//     if (N2 < 1E-20) continue;
//     value = N/N2;
//     deltaT_energy -> SetBinContent(x,y,value);
//   }
// }
// delete deltaT_energy_first_line;






//Gated spectra
// m_gated_Spectra.Write();
// m_gated_compt_supp.Write();
// m_gated_compt_add_back.Write();
// m_gated_compt_veto.Write();
//Gated Counters





  // m_DSSD_S1_1_VS_Sectors_1R1S.reset("coinc S1_1 && ring", "S1_1 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_2_VS_Sectors_1R1S.reset("coinc S1_2 && ring", "S1_2 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_3_VS_Sectors_1R1S.reset("coinc S1_3 && ring", "S1_3 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_4_VS_Sectors_1R1S.reset("coinc S1_4 && ring", "S1_4 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_5_VS_Sectors_1R1S.reset("coinc S1_5 && ring", "S1_5 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_6_VS_Sectors_1R1S.reset("coinc S1_6 && ring", "S1_6 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_7_VS_Sectors_1R1S.reset("coinc S1_7 && ring", "S1_7 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_8_VS_Sectors_1R1S.reset("coinc S1_8 && ring", "S1_8 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_9_VS_Sectors_1R1S.reset("coinc S1_9 && ring", "S1_9 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_10_VS_Sectors_1R1S.reset("coinc S1_10 && ring", "S1_10 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_11_VS_Sectors_1R1S.reset("coinc S1_11 && ring", "S1_11 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_12_VS_Sectors_1R1S.reset("coinc S1_12 && ring", "S1_12 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_13_VS_Sectors_1R1S.reset("coinc S1_13 && ring", "S1_13 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_14_VS_Sectors_1R1S.reset("coinc S1_14 && ring", "S1_14 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);
  // m_DSSD_S1_15_VS_Sectors_1R1S.reset("coinc S1_15 && ring", "S1_15 VS Rings",
  //       n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd], n->m_bins_bidim[dssd],n->m_min_bidim[dssd],n->m_max_bidim[dssd]);






// Gate with 197Au : 191 keV, 279 keV, 547 keV
// if ( gate(nrj_i,277,281) ||  gate(nrj_i,189,193) ||  gate(nrj_i,545,550))
// {
//   for (auto const & DSSD_hit : arg.DSSD_event) m_DSSD_E_gate_197Au[thread_nb] -> Fill(DSSD_hit.nrj);
//   break;
// }
// // Gate with 56Fe : 846 keV, 1303 keV, 1238 keV, 1810 keV, 2095 keV, 2113 keV, 2523 keV,
// // 2598 keV, doublet 2569/2536 keV
// if ( gate(nrj_i,845,850) || gate(nrj_i,1034,1041) || gate(nrj_i,1234,1241)
//   || gate(nrj_i,1805,1815) || gate(nrj_i,2090,2098) || gate(nrj_i,2106,2117)
//   || gate(nrj_i,2269,2277) || gate(nrj_i,2515,2526) || gate(nrj_i,2592,2604)
//   || gate(nrj_i,2752,27650) || gate(nrj_i,2979,2987) || gate(nrj_i,3295,3305))
// {
//   for (auto DSSD_hit : arg.DSSD_event) m_DSSD_E_gate_56Fe[thread_nb] -> Fill(DSSD_hit.nrj);
//   break;
// }
// // Gate with 52Cr : 1434 keV, 935 keV
// if ( gate(nrj_i,1432,1438) || gate(nrj_i,933,940))
// {
//   for (auto DSSD_hit : arg.DSSD_event) m_DSSD_E_gate_52Cr[thread_nb] -> Fill(DSSD_hit.nrj);
//   break;
// }
// // Gate with 12C :
// if (gate(nrj_i,4380,4500))
// {
//   for (auto DSSD_hit : arg.DSSD_event) m_DSSD_E_gate_12C[thread_nb] -> Fill(DSSD_hit.nrj);
//   break;
// }
// // Gate with 16O : 6128 keV, 6915 keV, 7115 keV
// if ( gate(nrj_i,6850,6980) || gate(nrj_i,6110,6140) || gate(nrj_i,7050,7200) )
// {
//   for (auto DSSD_hit : arg.DSSD_event) m_DSSD_E_gate_16O[thread_nb] -> Fill(DSSD_hit.nrj);
//   break;
// }
