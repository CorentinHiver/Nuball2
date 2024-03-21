#include <MTRootReader.hpp>
#include <Detectors.hpp>
#include <Event.hpp>
#include <RF_Manager.hpp>
#include <FilesManager.hpp>
#include <Clovers.hpp>
#include <MTTHist.hpp>
#include <RWMat.hxx>
#include <MTList.hpp>

bool simple_d = false;

Label_vec const blacklist = {800, 801};
std::unordered_map<Label, double> const maxE_Ge = {{28, 7500}, {33, 8250}, {46, 9000}, {55, 7500}, {57, 6000}, 
                                                   {68, 7000}, {71, 9500}, {85, 7500}, {91, 8000}, {134, 8500}, 
                                                   {135, 8500}, {136, 9000}, {142, 6000}, {145, 8000}, {146, 9000},
                                                   {147, 9000}, {157, 9000}, {158, 9000}, {159, 9000}};

std::string g_outfilename = "tests.root";
int nb_threads = -1;

inline float smear(float const & nrj, Label const & label, TRandom* random)
{
  if (nrj>0)
  {
    if (isBGO[label]) return nrj;
    else return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
  }
  else return 0;
}

inline float smear(float const & nrj, Label const & label)
{
  if (nrj>0)
  {
    if (isBGO[label]) return nrj;
    else return randomCo::gaussian(nrj, nrj*((400.0/sqrt(nrj))/100.0)/2.35);
  }
  else return 0;
}

class Analysator
{
public:
  Analysator(int const & number_files, std::string const & datapath)
  // Analysator(int const & number_files, std::string const & datapath = "~/nuball2/N-SI-136-root_dd/")
  {
    this->Initialise();
    MTRootReader reader(datapath, number_files);
    print("Launching the reader");
    reader.read(dispatch_threads, *this);
    write();
  }

  void Initialise();
  void analyse(Nuball2Tree & tree, Event & event);
  void write();

  static void setMaxHits(int const & nb_max_hits) {g_nb_max_hits = nb_max_hits;}

private:
  /// @brief static 
  static void dispatch_threads(Nuball2Tree & tree, Event & event, Analysator & analysator) {analysator.analyse(tree, event);}
  static long g_nb_max_hits;

  TRandom* random = new TRandom();
  MTList MTfiles;
  std::vector<int> runs;

  // Histograms :
#ifndef QUALITY
  MTTHist<TH1F> prompt_Ge;
  MTTHist<TH1F> delayed_Ge;
  MTTHist<TH1F> prompt_BGO;
  MTTHist<TH1F> delayed_BGO;
  MTTHist<TH1F> prompt_NaI;
  MTTHist<TH1F> delayed_NaI;
  MTTHist<TH1F> prompt_LaBr;
  MTTHist<TH1F> delayed_LaBr;

  MTTHist<TH1F> prompt_clean_Ge;
  MTTHist<TH1F> delayed_clean_Ge;

  MTTHist<TH1F> prompt_Ge_wp;
  MTTHist<TH1F> delayed_Ge_wp;
  MTTHist<TH1F> prompt_BGO_wp;
  MTTHist<TH1F> delayed_BGO_wp;
  MTTHist<TH1F> prompt_NaI_wp;
  MTTHist<TH1F> delayed_NaI_wp;
  MTTHist<TH1F> prompt_LaBr_wp;
  MTTHist<TH1F> delayed_LaBr_wp;

  MTTHist<TH1F> prompt_Ge_Clover_wp;
  
  MTTHist<TH1F> prompt_calo;
  MTTHist<TH1F> prompt_calo_A;
  MTTHist<TH1F> prompt_calo_B;
  MTTHist<TH1F> prompt_calo_C;
  MTTHist<TH1F> prompt_calo_D;
  MTTHist<TH1F> prompt_calo_E;
  MTTHist<TH1F> closest_prompt_calo_histo;
  MTTHist<TH1F> delayed_calo;

  MTTHist<TH2F> prompt_delayed_calo;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo;
  MTTHist<TH2F> delayed_Ge_VS_prompt_calo;

  MTTHist<TH2F> spectra_all;
  MTTHist<TH2F> spectra_Ge_VS_run;
  MTTHist<TH2F> spectra_BGO_VS_run;
  MTTHist<TH2F> spectra_LaBr_VS_run;
  MTTHist<TH2F> spectra_NaI_VS_run;


  MTTHist<TH2F> delayed_E_VS_time_Ge_clean;
  MTTHist<TH2F> delayed_E_VS_time_Ge_clean_wp;
  MTTHist<TH2F> E_VS_time_BGO_wp;
  MTTHist<TH2F> E_VS_time_LaBr_wp;
  MTTHist<TH2F> E_VS_time_NaI_wp;
  MTTHist<TH2F> time_all;
  MTTHist<TH2F> time_NaI;
  MTTHist<TH2F> time_all_knowing_pulse_A;
  MTTHist<TH2F> time_all_knowing_pulse_B;
  MTTHist<TH2F> time_all_knowing_pulse_C;
  MTTHist<TH2F> time_all_knowing_pulse_D;
  MTTHist<TH2F> time_all_knowing_pulse_E;
  MTTHist<TH2F> time_all_knowing_only_pulse_A;
  MTTHist<TH2F> time_all_knowing_only_pulse_B;
  MTTHist<TH2F> time_all_knowing_only_pulse_C;
  MTTHist<TH2F> time_all_knowing_only_pulse_D;
  MTTHist<TH2F> time_all_knowing_only_pulse_E;
  MTTHist<TH2F> time_all_pulse_A;
  MTTHist<TH2F> time_all_pulse_B;
  MTTHist<TH2F> time_all_pulse_C;
  MTTHist<TH2F> time_all_pulse_D;


  // MTTHist<TH1F> delayed_Ge_wp;
  MTTHist<TH1F> delayed_calo_wp;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wp;

  MTTHist<TH1F> delayed_Ge_wpp;
  MTTHist<TH1F> delayed_calo_wpp;
  MTTHist<TH1F> prompt_Ge_wpp;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wpp;

  MTTHist<TH1F> delayed_Ge_wppE;
  MTTHist<TH1F> delayed_calo_wppE;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wppE;

  MTTHist<TH2F> pp;
  MTTHist<TH2F> dd;
  MTTHist<TH2F> dd_wp;
  MTTHist<TH2F> dd_wpp;
  MTTHist<TH2F> dd_wppE;
  MTTHist<TH2F> dp;

  MTTHist<TH1F> delayed_clean_Ge_last_pulse_A;
  MTTHist<TH1F> delayed_clean_Ge_last_pulse_B;
  MTTHist<TH1F> delayed_clean_Ge_last_pulse_C;
  MTTHist<TH1F> delayed_clean_Ge_last_pulse_D;
  MTTHist<TH1F> delayed_clean_Ge_last_pulse_E;

  MTTHist<TH2F> dd_time_Ge_clean;
  MTTHist<TH2F> dd_time_Ge_clean_wp;
  MTTHist<TH2F> delayed_Ge_clean_VS_prompt_calo;
  MTTHist<TH2F> delayed_Ge_clean_VS_delayed_calo_wop; // With one prompt

  MTTHist<TH1F> number_of_pulses_detected;
  MTTHist<TH1F> preprompt_spectra;

  MTTHist<TH2F> time_vs_run;

  MTTHist<TH2F> dT_VS_sumGe;
  MTTHist<TH2F> delayed_Ge_C2_VS_total_Ge;
  MTTHist<TH2F> delayed_Ge_C2_VS_prompt_Ge;
  MTTHist<TH2F> delayed_Ge_C2_VS_prompt_mult;
  MTTHist<TH2F> delayed_Ge_C2_VS_total_Ge_cleaned;
  MTTHist<TH2F> delayed_Ge_C2_VS_total_Ge_rejected;
  MTTHist<TH2F> delayed_Ge_C3_VS_total_Ge;
  MTTHist<TH2F> delayed_Ge_C3_tot3_VS_total_Ge;
  
  // simple clean Ge delayed trigger :
  MTTHist<TH2F> delayed_Ge_C1_VS_prompt_Ge;
  MTTHist<TH2F> delayed_Ge_C1_VS_delayed_calo;

  #else //QUALITY

  Map_MTTHist<TH2F> time_vs_run_each_det;
  Map_MTTHist<TH2F> time_vs_det_each_run;

  #endif//QUALITY
};

long Analysator::g_nb_max_hits = -1;

void Analysator::Initialise()
{
  randomCo::setSeed(time(0));
  detectors.load("index_129.list");
  random->SetSeed(time(0));
  Clovers::InitializeArrays();
  Clovers::timePs(true);
  int run_min = 70;
  int run_max = 130;
  int nb_runs = run_max-run_min;

#ifndef QUALITY

  prompt_Ge.reset("prompt_Ge" , "prompt Ge;E[keV]" , 20000,0,10000);
  delayed_Ge.reset("delayed_Ge", "delayed Ge;E[keV]", 20000,0,10000);
  prompt_BGO.reset("prompt_BGO" , "prompt BGO;E[keV]" , 2000,0,10000);
  delayed_BGO.reset("delayed_BGO", "delayed BGO;E[keV]", 2000,0,10000);
  prompt_NaI.reset("prompt_NaI" , "prompt NaI;E[keV]" , 5000,0,10000);
  delayed_NaI.reset("delayed_NaI" , "delayed NaI;E[keV]" , 5000,0,10000);
  prompt_LaBr.reset("prompt_LaBr" , "prompt LaBr;E[keV]" , 5000,0,10000);
  delayed_LaBr.reset("delayed_LaBr" , "delayed LaBr;E[keV]" , 5000,0,10000);

  prompt_Ge_wp.reset("prompt_Ge_wp" , "prompt Ge with at least one prompt gamma;E[keV]" , 20000,0,10000);
  delayed_Ge_wp.reset("delayed_Ge_wp", "delayed Ge with at least one prompt gamma;E[keV]", 20000,0,10000);
  prompt_BGO_wp.reset("prompt_BGO_wp" , "prompt BGO with at least one prompt gamma;E[keV]" , 2000,0,10000);
  delayed_BGO_wp.reset("delayed_BGO_wp", "delayed BGO with at least one prompt gamma;E[keV]", 2000,0,10000);
  prompt_NaI_wp.reset("prompt_NaI_wp" , "prompt NaI with at least one prompt gamma;E[keV]" , 5000,0,10000);
  delayed_NaI_wp.reset("delayed_NaI_wp" , "delayed NaI with at least one prompt gamma;E[keV]" , 5000,0,10000);
  prompt_LaBr_wp.reset("prompt_LaBr_wp" , "prompt LaBr with at least one prompt gamma;E[keV]" , 5000,0,10000);
  delayed_LaBr_wp.reset("delayed_LaBr_wp" , "delayed LaBr with at least one prompt gamma;E[keV]" , 5000,0,10000);

  number_of_pulses_detected.reset("number_of_pulses_detected","number of pulses detected", 5,0,5);

  prompt_Ge_Clover_wp.reset("prompt_Ge_Clover_wp" , "prompt Ge Clover with at least one prompt gamma;E[keV]" , 20000,0,10000);

  // Calorimetry
  prompt_calo.reset("prompt_calo" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  prompt_calo_A.reset("prompt_calo_A" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  prompt_calo_B.reset("prompt_calo_B" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  prompt_calo_C.reset("prompt_calo_C" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  prompt_calo_D.reset("prompt_calo_D" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  prompt_calo_E.reset("prompt_calo_E" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  closest_prompt_calo_histo.reset("closest_prompt_calo_histo" , "closest prompt calorimetry;E[keV]" , 5000,0,10000);
  delayed_calo.reset("delayed_calo", "delayed calorimetry;E[keV]", 5000,0,10000);
  prompt_delayed_calo.reset("prompt_delayed_calo", "Delayed VS clostest prompt calorimetry;Prompt Calorimetry[keV];Delayed Calorimetry[keV]", 
      1000,0,20000, 1000,0,20000);
  delayed_Ge_VS_delayed_calo.reset("delayed_Ge_VS_delayed_calo", "Delayed Ge VS delayed calorimetry;Delayed Calorimetry[keV];E[keV]", 
      500,0,10000, 5000,0,10000);
  delayed_Ge_VS_prompt_calo.reset("delayed_Ge_VS_prompt_calo", "Delayed Ge VS prompt calorimetry;Prompt Calorimetry[keV];E[keV]", 
      20000,0,10000, 500,0,20000);

  // Prompt condition
  delayed_Ge_wp.reset("delayed_Ge_wp", "delayed Ge with prompt condition;E[keV]", 20000,0,10000);
  delayed_calo_wp.reset("delayed_calo_wp", "delayed calo with prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wp.reset("delayed_Ge_VS_delayed_calo_wp", "delayed calo with prompt condition;E[keV]", 
      500,0,20000, 10000,0,10000);

  // Prompt particle condition
  prompt_Ge_wpp.reset("prompt_Ge_wpp", "prompt Ge with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_wpp.reset("delayed_Ge_wpp", "delayed Ge with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_calo_wpp.reset("delayed_calo_wpp", "delayed calo with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wpp.reset("delayed_Ge_VS_delayed_calo_wpp", "delayed calo with particle prompt condition;E[keV]", 
      500,0,20000, 10000,0,10000);

  // Prompt particle energy condition
  delayed_Ge_wppE.reset("delayed_Ge_wppE", "delayed Ge with particle prompt with E<5MeV;E[keV]", 20000,0,10000);
  delayed_calo_wppE.reset("delayed_calo_wppE", "delayed calo with particle prompt with E<5MeV;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wppE.reset("delayed_Ge_VS_delayed_calo_wppE", "delayed calo with particle prompt with E<5MeV;E[keV]", 
      500,0,20000, 10000,0,10000);


  // Final bidims
  pp.reset("pp", "pp;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dd.reset("dd", "dd;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dd_wp.reset("dd_wp", "dd;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dd_wpp.reset("dd_wp", "dd;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dd_wppE.reset("dd_wp", "dd;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dp.reset("dp", "delaed VS prompt;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);

  spectra_all.reset("spectra_all", "Spectra;label;Energy [keV]", 1000,0,1000, 10000,0,10000);

  spectra_Ge_VS_run.reset("spectra_Ge_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_BGO_VS_run.reset("spectra_BGO_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_LaBr_VS_run.reset("spectra_LaBr_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_NaI_VS_run.reset("spectra_NaI_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);

  // Timing
  time_all.reset("time_all", "Time", 1000,0,1000, 2000,-1000,1000);
  time_NaI.reset("time_NaI", "Time NaI", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_pulse_A.reset("time_all_knowing_pulse_A", "Time knownig the pulse 0 has hits;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_pulse_B.reset("time_all_knowing_pulse_B", "Time knownig the pulse -1 has hits;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_pulse_C.reset("time_all_knowing_pulse_C", "Time knownig the pulse -2 has hits;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_pulse_D.reset("time_all_knowing_pulse_D", "Time knownig the pulse -3 has hits;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_pulse_E.reset("time_all_knowing_pulse_E", "Time knownig the pulse -3 has hits;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_only_pulse_A.reset("time_all_knowing_only_pulse_A", "Time knownig the pulse 0 is the only one;label,time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_only_pulse_B.reset("time_all_knowing_only_pulse_B", "Time knownig the pulse -1 is the only one;label,time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_only_pulse_C.reset("time_all_knowing_only_pulse_C", "Time knownig the pulse -2 is the only one;label,time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_only_pulse_D.reset("time_all_knowing_only_pulse_D", "Time knownig the pulse -3 is the only one;label,time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_knowing_only_pulse_E.reset("time_all_knowing_only_pulse_E", "Time knownig the pulse -4 is the only one;label,time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_pulse_A.reset("time_all_pulse_A", "Time of the pulse 0;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_pulse_B.reset("time_all_pulse_B", "Time of the pulse -1;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_pulse_C.reset("time_all_pulse_C", "Time of the pulse -2;label;time [ns]", 1000,0,1000, 2000,-1000,1000);
  time_all_pulse_D.reset("time_all_pulse_D", "Time of the pulse -3;label;time [ns]", 1000,0,1000, 2000,-1000,1000);

  delayed_E_VS_time_Ge_clean.reset("delayed_E_VS_time_Ge_clean", "Energy vs time delayed;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);
  delayed_E_VS_time_Ge_clean_wp.reset("delayed_E_VS_time_Ge_clean_wp", "Energy Ge vs time delayed requiring one prompt;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);

  E_VS_time_BGO_wp.reset("E_VS_time_BGO_wp", "Energy BGO vs time requiring one prompt;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);
  E_VS_time_LaBr_wp.reset("E_VS_time_LaBr_wp", "Energy LaBr vs time requiring one prompt;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);
  E_VS_time_NaI_wp.reset("E_VS_time_NaI_wp", "Energy Nai vs time requiring one prompt;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);

  delayed_clean_Ge_last_pulse_A.reset("delayed_clean_Ge_last_pulse_A", "clean Ge last pulse A;Energy [keV]", 10000,0,10000);
  delayed_clean_Ge_last_pulse_B.reset("delayed_clean_Ge_last_pulse_B", "clean Ge last pulse B;Energy [keV]", 10000,0,10000);
  delayed_clean_Ge_last_pulse_C.reset("delayed_clean_Ge_last_pulse_C", "clean Ge last pulse C;Energy [keV]", 10000,0,10000);
  delayed_clean_Ge_last_pulse_D.reset("delayed_clean_Ge_last_pulse_D", "clean Ge last pulse D;Energy [keV]", 10000,0,10000);
  delayed_clean_Ge_last_pulse_E.reset("delayed_clean_Ge_last_pulse_E", "clean Ge last pulse E;Energy [keV]", 10000,0,10000);

  dd_time_Ge_clean.reset("dd_time_Ge_clean", "dd time Ge clean;time [ns];time [ns]", 500,0,500,500,0,500);
  dd_time_Ge_clean_wp.reset("dd_time_Ge_clean_wp", "dd time Ge clean wp;time [ns];time [ns]", 500,0,500,500,0,500);
  delayed_Ge_clean_VS_prompt_calo.reset("delayed_Ge_clean_VS_prompt_calo", "delayed Ge clean VS prompt calo", 500,0,15000,10000,0,10000);
  delayed_Ge_clean_VS_delayed_calo_wop.reset("delayed_Ge_clean_VS_delayed_calo_wop", "delayed Ge clean VS delayed calo with only one prompt before", 500,0,15000,10000,0,10000);

  // When only 2-3 germaniums, plot germaniums VS the sum of them
  delayed_Ge_C2_VS_total_Ge.reset("delayed_Ge_C2_VS_total_Ge", "Clean Ge VS sum clean Ge C2", 5000,0,5000, 2000,0,4000);
  delayed_Ge_C2_VS_prompt_Ge.reset("delayed_Ge_C2_VS_prompt_Ge", "Clean Ge C2 VS prompt Ge", 5000,0,5000, 2000,0,4000);
  delayed_Ge_C2_VS_prompt_mult.reset("delayed_Ge_C2_VS_prompt_mult", "Clean Ge C2 VS prompt Ge mult", 15,0,15, 2000,0,4000);
  delayed_Ge_C2_VS_total_Ge_cleaned.reset("delayed_Ge_C2_VS_total_Ge_cleaned", "Clean Ge VS sum clean Ge C2 cleaned", 5000,0,5000, 2000,0,4000);
  delayed_Ge_C2_VS_total_Ge_rejected.reset("delayed_Ge_C2_VS_total_Ge_rejected", "Clean Ge VS sum clean Ge C2 rejected", 5000,0,5000, 2000,0,4000);
  delayed_Ge_C3_VS_total_Ge.reset("delayed_Ge_C3_VS_total_Ge", "Clean Ge VS sum clean Ge C3 - only 2 by 2", 5000,0,5000, 2000,0,4000);
  delayed_Ge_C3_tot3_VS_total_Ge.reset("delayed_Ge_C3_tot3_VS_total_Ge", "Clean Ge VS sum clean Ge C3", 5000,0,5000, 2000,0,4000);

  // For TSC method :
  dT_VS_sumGe.reset("dT_VS_Esum_Ge", "dT VS Esum Ge", 10000,0,10000, 100,-200,200);

  preprompt_spectra.reset("preprompt_spectra", "preprompt spectra", 10000,0,10000);

  time_vs_run.reset("time_vs_run","time vs run", nb_runs,run_min,run_max, 750, -1000, 500);
  
  if (simple_d)
  {
    delayed_Ge_C1_VS_delayed_calo.reset("delayed_Ge_C1_VS_delayed_calo", "Clean Ge C1 VS delayed calo", 500,0,5000, 5000,0,5000);
    delayed_Ge_C1_VS_prompt_Ge.reset("delayed_Ge_C1_VS_prompt_Ge", "Clean Ge C1 VS prompt Ge mult", 5000,0,5000, 2000,0,4000);
  }

  // Run quality :
#else // if QUALITY
  print("_______________");
  print("Running quality check");

  for (auto const & label : detectors.labels())
  {
    std::string name = "time_vs_run_"+detectors[label];
    time_vs_run_each_det.emplace(label, MTTHist<TH2F>(name.c_str(), "time vs run each;run number;time [ns]", nb_runs,run_min,run_max, 1500,-1000,500));
  }
  for (int run_i = run_min; run_i<run_max+1; run_i++)
  {
    auto name = ("time_vs_det_run_"+std::to_string(run_i));
    auto title = concatenate("time vs det run", run_i, ";run number;time [ns]");
    time_vs_det_each_run.emplace(run_i, MTTHist<TH2F>(name.c_str(), title.c_str(), 900,0,900, 750,-1000,500));
  }
#endif //QUALITY
}

bool NaI_pid(float nrj, float nrj2)
{
  return ((nrj2-nrj)/double_cast(nrj2) > 0.15);
}

bool isDelayed(Time_ns const & time_ns) {return (time_ns>20 && time_ns<180);}

void Analysator::analyse(Nuball2Tree & tree, Event & event)
{
  File file(tree.filename());
  int run_number = std::stoi(getList(file.shortName(), '_')[1]);
  MTObject::mutex.lock();
    runs.push_back(run_number);
  MTObject::mutex.unlock();

  Bools isNaI;
  Bools isLaBr;
  Bools rejected;

  RF_Manager rf;

  auto nb_evts = tree->GetEntries();
  if (g_nb_max_hits>-1) nb_evts = g_nb_max_hits;
  for (int evt_i = 0; evt_i<nb_evts; ++evt_i)
  { // Iterate over the events of the file
    if(evt_i>0 && evt_i%int_cast(1.e+7) == 0) print(evt_i/1.e+6, "Mevts"); 

    tree->GetEntry(evt_i);

    if (rf.setEvent(event)) continue;
    #ifndef QUALITY

    isNaI   .resize(event.mult, false);
    isLaBr  .resize(event.mult, false);
    rejected.resize(event.mult, false);

    double prompt_calorimetry = 0;
    double prompt_calorimetry_Ge = 0;
    double prompt_calorimetry_LaBr = 0;
    double prompt_calorimetry_BGO = 0;
    double prompt_calorimetry_NaI = 0;

    double delayed_calorimetry = 0;
    double delayed_calorimetry_Ge = 0;
    double delayed_calorimetry_LaBr = 0;
    double delayed_calorimetry_BGO = 0;
    double delayed_calorimetry_NaI = 0;

    int multiplicity_prompt = 0;
    int multiplicity_delayed = 0;


    Clovers clovers_delayed;

    int nb_prompts = 6;

    bool only_prompt_with_gamma = false;
    std::vector<int> nb_gamma_in_prompts(nb_prompts, 0);
    std::vector<double> calo_prompts(nb_prompts, 0.0);
    std::vector<Clovers> prompt_clovers(nb_prompts);
    Clovers prompt_clovers_d;
    int nb_gamma_in_prompts_d = 0;
    std::vector<bool> particle_associated_with_prompt(nb_prompts, false); // Deal with DSSD later

    std::vector<bool> is_prompt(event.mult, false);
    std::vector<bool> is_delayed(event.mult, false);

    // Fine tune data and pre-treatment :
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {// Iterate over the hits of the event
      auto const & label = event.labels[hit_i];
      auto       & time  = event.times [hit_i];
      auto const & nrj2  = event.nrj2s [hit_i];
      auto       & nrj   = event.nrjs[hit_i];

      if (isDSSD[label]) continue; // There is nothing to do with paris now
      if (found(blacklist, label)) {rejected[hit_i] = true; continue;}
      // Throw events with too low energy
      if (nrj<5)
      {
        rejected[hit_i] = true;
        continue;
      }


      // if ((label == 506 || label == 508) && (run_number==100 || run_number==101)) time+=3500; // BETTER TIME ALIGN !!
      float time_ns = time/1000.;
      time_all.Fill(label, time_ns);
      if (time_ns>180) {rejected[hit_i] = true; continue;}
      is_delayed[hit_i] = isDelayed(time_ns);

      time_vs_run.Fill(run_number, time_ns);
      //////////////
      // Prompt : //
      //////////////

      // Range les gammas dans leurs prompts respectifs:
      int index_prompt = -1;
      int min_T = -15;
      int max_T = 5;
      int pulse_freq = 200;
      if (isParis[label]) {min_T = -5; max_T = 4;}
      if (simple_d && !isDSSD[label] && -10<time_ns && time_ns<10)
      {
        ++nb_gamma_in_prompts_d;
        is_prompt[hit_i] = true;
      }
      if (!simple_d && time_ns<max_T && !isDSSD[label])
      {                                                                 // Let's take an example : time = -798 and max_T = 5;
        auto const & shifted_time = abs(time_ns)+max_T;                 // shifted_time = 803
                     index_prompt = (int_cast(shifted_time)/pulse_freq);// index_prompt = 4
        auto const & dT = shifted_time-index_prompt*pulse_freq;         // dT = 3 -> It is prompt !
        if (dT>0 && dT<abs(min_T)+max_T) 
        {
          is_prompt[hit_i] = true;
          nb_gamma_in_prompts[index_prompt]++;
          switch(index_prompt)
          {
            case 0: time_all_pulse_A.Fill(label, time_ns);break;
            case 1: time_all_pulse_B.Fill(label, time_ns);break;
            case 2: time_all_pulse_C.Fill(label, time_ns);break;
            case 3: time_all_pulse_D.Fill(label, time_ns);break;
            default: break;
          }
        }
      }

      //////////////////
      // GERMANIUMS : //
      //////////////////
      if (isGe[label])
      {
        if (find_key(maxE_Ge, label) && nrj>maxE_Ge.at(label))
        {
          rejected[hit_i] = true;
          continue;
        }
        if (is_prompt[hit_i]) 
        {
          prompt_clovers[index_prompt].Fill(event, hit_i);
          if (simple_d) 
          {
            prompt_clovers_d.Fill(event, hit_i);
            prompt_clovers[0].Fill(event, hit_i);
          }
          prompt_Ge.Fill(nrj);
          
        }
        else if (is_delayed[hit_i]) 
        {
          delayed_Ge.Fill(nrj);
          clovers_delayed.Fill(event, hit_i);
        }
      }
      
      /////////////
      // PARIS : //
      /////////////
      else if (isParis[label]) 
      {
        if (NaI_pid(nrj, nrj2))
        {
          isNaI[hit_i] = true;
          nrj=nrj2*1.1; // CALIBRATE BETTER !!!

          time_all.Fill(label, time_ns, -1);
          time_NaI.Fill(label, time_ns);
               if (is_prompt [hit_i]) {prompt_NaI .Fill(nrj);}
          else if (is_delayed[hit_i]) {delayed_NaI.Fill(nrj);}
        }
        else
        {
          isLaBr[hit_i] = true;
               if (is_prompt [hit_i]) {prompt_LaBr .Fill(nrj);}
          else if (is_delayed[hit_i]) {delayed_LaBr.Fill(nrj);}
        }
      }

      ///////////
      // BGO : //
      ///////////
      else if (isBGO[label]) 
      {
        nrj*=1.11; // CALIBRATE BETTER !!!
             if (is_prompt [hit_i]) {prompt_BGO .Fill(nrj);}
        else if (is_delayed[hit_i]) {delayed_BGO.Fill(nrj);}
      }
      
      ///////////////////
      // Calorimetry : //
      ///////////////////
      auto const & smeared_energy = smear(nrj, label);
      if (is_prompt[hit_i])
      {
        prompt_calorimetry += smeared_energy;

        calo_prompts[index_prompt] += smeared_energy;

             if (isGe[label])  {prompt_calorimetry_Ge  += smeared_energy;}
        else if (isBGO[label]) {prompt_calorimetry_BGO +=nrj;}
        else if (isNaI[hit_i]) {prompt_calorimetry_NaI  += smeared_energy;}
        else if (isLaBr[hit_i]){prompt_calorimetry_LaBr += smeared_energy;}

        ++multiplicity_prompt;
      }
      else if (is_delayed[hit_i])
      {
        delayed_calorimetry += smeared_energy;
            if (isGe[label])  {delayed_calorimetry_Ge += smeared_energy;}
        else if (isBGO[label]) {delayed_calorimetry_BGO +=nrj;}
        else if (isParis[label])
        {
          if (isNaI[hit_i])    {delayed_calorimetry_NaI  += smeared_energy;}
          else                 {delayed_calorimetry_LaBr += smeared_energy;}
        }
        ++multiplicity_delayed;
      }
      spectra_all.Fill(label, nrj);
    }
    
    ///////////////////
    // Calorimetry : //
    ///////////////////

    int closest_prompt = 0;
    double all_prompt_calo = 0.0;

    if (multiplicity_prompt>0)
    {
      prompt_calo.Fill(prompt_calorimetry);

      // Find the closest prompt :
      for (int prompt_i = nb_prompts; prompt_i>-1; --prompt_i) 
      {
        all_prompt_calo += calo_prompts[prompt_i];
        if (nb_gamma_in_prompts[prompt_i]>0) closest_prompt = prompt_i;
      }

      closest_prompt_calo_histo.Fill(calo_prompts[closest_prompt]);

      if (nb_gamma_in_prompts[0]>0) prompt_calo_A.Fill(calo_prompts[0]);
      if (nb_gamma_in_prompts[1]>0) prompt_calo_B.Fill(calo_prompts[1]);
      if (nb_gamma_in_prompts[2]>0) prompt_calo_C.Fill(calo_prompts[2]);
      if (nb_gamma_in_prompts[3]>0) prompt_calo_D.Fill(calo_prompts[3]);
      if (nb_gamma_in_prompts[4]>0) prompt_calo_E.Fill(calo_prompts[4]);
    
      prompt_delayed_calo.Fill(calo_prompts[closest_prompt], delayed_calorimetry);
    }

    delayed_calo.Fill(delayed_calorimetry);

    ///////////
    // OTHER //
    ///////////

    int nb_prompts_with_gammas = 0;
    for (auto const & nb_hits : nb_gamma_in_prompts) if (nb_hits>0) ++nb_prompts_with_gammas;
    number_of_pulses_detected.Fill(nb_prompts_with_gammas);

    std::vector<bool> only_prompt; // Set true if there is only this prompt that fired
    for (auto const & nb_hits : nb_gamma_in_prompts) only_prompt.push_back((nb_hits>0 && nb_prompts_with_gammas == 1));


    /////////////////////
    // Treated Event : //
    /////////////////////
    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      auto const & nrj   = event.nrjs  [hit_i];
      auto const & time  = event.times [hit_i];
      auto const & time_ns = time/1000.;

      if(rejected[hit_i]) continue;

      if (multiplicity_prompt>0)
      {
             if (isGe[label])
        {
          if (is_delayed[hit_i]) delayed_Ge_wp.Fill(nrj);
        }

        else if (isBGO[label])
        {
          if(is_delayed[hit_i]) delayed_BGO_wp .Fill(nrj);
          E_VS_time_BGO_wp.Fill(time_ns, nrj);
        }
        else if (isLaBr[hit_i])
        {
          if(is_delayed[hit_i]) delayed_LaBr_wp.Fill(nrj);
          E_VS_time_LaBr_wp.Fill(time_ns, nrj);
        }
        else if (isNaI[hit_i])
        {
          if(is_delayed[hit_i]) delayed_NaI_wp .Fill(nrj);
          E_VS_time_NaI_wp.Fill(time_ns, nrj);
        }

        if (nb_gamma_in_prompts[0]>0) time_all_knowing_pulse_A.Fill(label, time_ns);
        if (nb_gamma_in_prompts[1]>0) time_all_knowing_pulse_B.Fill(label, time_ns);
        if (nb_gamma_in_prompts[2]>0) time_all_knowing_pulse_C.Fill(label, time_ns);
        if (nb_gamma_in_prompts[3]>0) time_all_knowing_pulse_D.Fill(label, time_ns);
        if (nb_gamma_in_prompts[4]>0) time_all_knowing_pulse_E.Fill(label, time_ns);

        if (only_prompt[0]) time_all_knowing_only_pulse_A.Fill(label, time_ns);
        if (only_prompt[1]) time_all_knowing_only_pulse_B.Fill(label, time_ns);
        if (only_prompt[2]) time_all_knowing_only_pulse_C.Fill(label, time_ns);
        if (only_prompt[3]) time_all_knowing_only_pulse_D.Fill(label, time_ns);
        if (only_prompt[4]) time_all_knowing_only_pulse_E.Fill(label, time_ns);
      }
    } 

    clovers_delayed.Analyse();

    ///////////////////////
    // Clovers delayed : //
    ///////////////////////
    auto const & clean_indexes = clovers_delayed.CleanGe; // Simple alias

    // Trigger : only one prompt in maximum 3*rf_period: 
    bool one_close_prompt = false;
    if (nb_prompts_with_gammas == 1) for (int prompt_i = 0; prompt_i<nb_prompts; ++prompt_i){
      if (only_prompt[prompt_i] && prompt_i<3)
      {
        one_close_prompt = true;
        break;
    }}

    if (simple_d)
    {
      prompt_clovers_d.Analyse();
      if (clean_indexes.size() == 1)
      {
        auto const & time = clovers_delayed[clean_indexes[0]].time;
        auto const & nrj = clovers_delayed[clean_indexes[0]].nrj;
        delayed_Ge_C1_VS_delayed_calo.Fill(delayed_calorimetry, nrj);
        for (auto const & clean_index : prompt_clovers_d.CleanGe)
        {
          delayed_Ge_C1_VS_prompt_Ge.Fill(prompt_clovers[closest_prompt][clean_index].nrj, nrj);
        }
      }
    }

    bool C2_PM24 = false; // 2 clean delayed and 1<prompt mult<5

    // Calculate delayed calorimetry with 2 or 3 clean germaniums :
    if ((clean_indexes.size() == 2) && (simple_d || one_close_prompt))
    {
      auto const & time_0 = clovers_delayed[clean_indexes[0]].time;
      auto const & time_1 = clovers_delayed[clean_indexes[1]].time;
      auto const & nrj_0 = clovers_delayed[clean_indexes[0]].nrj;
      auto const & nrj_1 = clovers_delayed[clean_indexes[1]].nrj;
      auto const & dT = time_1 - time_0;
      auto const & prompt_mult = (simple_d) ? : nb_gamma_in_prompts[closest_prompt];
      if (dT>50) continue;
      auto const & calo = nrj_0+nrj_1;
      delayed_Ge_C2_VS_total_Ge.Fill(calo, nrj_0);
      delayed_Ge_C2_VS_total_Ge.Fill(calo, nrj_1);
      if(prompt_mult > 1 && prompt_mult<5 
      && calo_prompts[closest_prompt]>515 && calo_prompts[closest_prompt]<2000)
      {
        delayed_Ge_C2_VS_total_Ge_cleaned.Fill(calo, nrj_0);
        delayed_Ge_C2_VS_total_Ge_cleaned.Fill(calo, nrj_1);
        dd_wp.Fill(nrj_0, nrj_1);
        dd_wp.Fill(nrj_1, nrj_0);
      }
      else
      {
        delayed_Ge_C2_VS_total_Ge_rejected.Fill(calo, nrj_0);
        delayed_Ge_C2_VS_total_Ge_rejected.Fill(calo, nrj_1);
      }

      // Prompt VS delayed :
      for (auto const & clean_index : prompt_clovers[closest_prompt].CleanGe)
      {
        auto const & clover = prompt_clovers[closest_prompt][clean_index];
        delayed_Ge_C2_VS_prompt_Ge.Fill(clover.nrj, nrj_1);
        delayed_Ge_C2_VS_prompt_Ge.Fill(clover.nrj, nrj_0);
      }
      delayed_Ge_C2_VS_prompt_mult.Fill(prompt_mult, nrj_0);
      delayed_Ge_C2_VS_prompt_mult.Fill(prompt_mult, nrj_1);
    }

    if ((clean_indexes.size() == 3) && (simple_d || one_close_prompt)) 
    {
      auto const & time_0 = clovers_delayed[clean_indexes[0]].time;
      auto const & time_1 = clovers_delayed[clean_indexes[1]].time;
      auto const & time_2 = clovers_delayed[clean_indexes[2]].time;
      auto const & nrj_0 = clovers_delayed[clean_indexes[0]].nrj;
      auto const & nrj_1 = clovers_delayed[clean_indexes[1]].nrj;
      auto const & nrj_2 = clovers_delayed[clean_indexes[2]].nrj;
      auto const & dT01 = time_1-time_0;
      auto const & dT02 = time_2-time_0;
      auto const & dT12 = time_2-time_1;
      if (dT01<50)
      {
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_1, nrj_0);
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_1, nrj_1);
      }
      if (dT02<50)
      {
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_2, nrj_0);
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_2, nrj_2);
      }
      if (dT12<50)
      {
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_1+nrj_2, nrj_1);
        delayed_Ge_C3_VS_total_Ge.Fill(nrj_1+nrj_2, nrj_2);
      }
      if (dT01<50 && dT02 < 50)
      {
        delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_0);
        delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_1);
        delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_2);
      }

    }

    // Others :
    /*for (size_t clover_it_i = 0; clover_it_i<clean_indexes.size(); ++clover_it_i)
    {
      continue;
      auto const & clover_i = clovers_delayed[clean_indexes[clover_it_i]];
      auto const & nrj_i = clover_i.nrj;
      auto const & time_i = clover_i.time;

      delayed_E_VS_time_Ge_clean.Fill(time_i, nrj_i);
      delayed_E_VS_time_Ge_clean_wp.Fill(time_i, nrj_i);

      if (nb_prompts_with_gammas == 1)
      {
        if (nb_gamma_in_prompts[0]>0) delayed_clean_Ge_last_pulse_A.Fill(nrj_i);
        if (nb_gamma_in_prompts[1]>0) delayed_clean_Ge_last_pulse_B.Fill(nrj_i);
        if (nb_gamma_in_prompts[2]>0) delayed_clean_Ge_last_pulse_C.Fill(nrj_i);
        if (nb_gamma_in_prompts[3]>0) delayed_clean_Ge_last_pulse_D.Fill(nrj_i);
        if (nb_gamma_in_prompts[4]>0) delayed_clean_Ge_last_pulse_E.Fill(nrj_i);
      }

      // Taking the pre-prompt delayed events :
      auto const & prompt_ref_time = -closest_prompt*200;
      if (time_i>prompt_ref_time && time_i>prompt_ref_time+150) preprompt_spectra.Fill(nrj_i);

      // Now, the point is to clean a maximum the data based on all the informations we have on the event,
      // in order to have to cleanest gamma-gamma matrices possible.

      // We are only interested in the events that has at least one prompt event :
      if (multiplicity_prompt==0) continue;

      // This removes the events that have the latest prompt at more than 2*pulse_rf : 
      if (closest_prompt>2) continue;

      // This removes the events that have a prompt event preceeding the latest prompt :
      if (nb_gamma_in_prompts[closest_prompt+1]>0) continue;

      // Creating the gamma-gamma matrices :
      for (size_t clover_it_j = clover_it_i+1; clover_it_j<clean_indexes.size(); ++clover_it_j)
      {
        auto const & clover_j = clovers_delayed[clean_indexes[clover_it_j]];
        auto const & nrj_j = clover_j.nrj;
        auto const & time_j = clover_j.time;
        
        // dd.Fill(nrj_i, nrj_j);
        // dd.Fill(nrj_j, nrj_i);

        // dd_time_Ge_clean.Fill(time_j, time_i);
        // dd_time_Ge_clean.Fill(time_i, time_j);

        // dd_wp.Fill(nrj_i, nrj_j);
        // dd_wp.Fill(nrj_j, nrj_i);

        dd_time_Ge_clean_wp.Fill(time_i, time_j);
        dd_time_Ge_clean_wp.Fill(time_j, time_i);
      }

      // Some other interesting plots :
      if (nb_prompts_with_gammas == 1)
      {
        delayed_Ge_clean_VS_prompt_calo.Fill(calo_prompts[0], nrj_i);
        if (one_close_prompt) delayed_Ge_clean_VS_delayed_calo_wop.Fill(delayed_calorimetry, nrj_i);
      }
    }*/
  
  #else // if QUALITY

    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      // auto const & nrj   = event.nrjs  [hit_i];
      auto const & time  = event.times [hit_i];
      auto const & time_ns = time/1000.;

      time_vs_run_each_det[label].Fill(run_number, time_ns);
      time_vs_det_each_run[run_number].Fill(label, time_ns);
    }

  #endif // QUALITY
  }

}

void Analysator::write()
{
#ifdef QUALITY
  g_outfilename = "run_quality.root";
#endif //QUALITY
  auto outfile(TFile::Open(g_outfilename.c_str(), "RECREATE"));
  outfile->cd();

#ifndef QUALITY
  RWMat RW_dd_wp(dd_wp); RW_dd_wp.Write();
  dd.Write();
  dp.Write();
  dd_wp.Write();
  dd_wpp.Write();
  dd_wppE.Write();

  prompt_Ge.Write();
  delayed_Ge.Write();
  prompt_BGO.Write();
  delayed_BGO.Write();
  prompt_NaI.Write();
  delayed_NaI.Write();
  prompt_LaBr.Write();
  delayed_LaBr.Write();
  delayed_clean_Ge.Write();

  delayed_Ge_wp.Write();
  delayed_BGO_wp.Write();
  delayed_NaI_wp.Write();
  delayed_LaBr_wp.Write();

  prompt_Ge_Clover_wp.Write();

  prompt_calo.Write();
  prompt_calo_A.Write();
  prompt_calo_B.Write();
  prompt_calo_C.Write();
  prompt_calo_D.Write();
  prompt_calo_E.Write();
  closest_prompt_calo_histo.Write();
  delayed_calo.Write();

  prompt_delayed_calo.Write();
  delayed_Ge_VS_prompt_calo.Write();

  delayed_Ge_wp.Write();
  delayed_calo_wp.Write();

  delayed_Ge_wpp.Write();
  delayed_calo_wpp.Write();
  prompt_Ge_wpp.Write();

  delayed_Ge_wppE.Write();
  delayed_calo_wppE.Write();

  delayed_Ge_VS_delayed_calo.Write();
  delayed_Ge_VS_delayed_calo_wp.Write();
  delayed_Ge_VS_delayed_calo_wpp.Write();
  delayed_Ge_VS_delayed_calo_wppE.Write();

  spectra_all.Write();
  spectra_Ge_VS_run.Write();
  spectra_BGO_VS_run.Write();
  spectra_LaBr_VS_run.Write();
  spectra_NaI_VS_run.Write();

  time_all.Write();
  time_NaI.Write();
  time_all_knowing_pulse_A.Write();
  time_all_knowing_pulse_B.Write();
  time_all_knowing_pulse_C.Write();
  time_all_knowing_pulse_D.Write();
  time_all_knowing_pulse_E.Write();
  time_all_knowing_only_pulse_A.Write();
  time_all_knowing_only_pulse_B.Write();
  time_all_knowing_only_pulse_C.Write();
  time_all_knowing_only_pulse_D.Write();
  time_all_knowing_only_pulse_E.Write();
  time_all_pulse_A.Write();
  time_all_pulse_B.Write();
  time_all_pulse_C.Write();
  time_all_pulse_D.Write();

  delayed_clean_Ge_last_pulse_A.Write();
  delayed_clean_Ge_last_pulse_B.Write();
  delayed_clean_Ge_last_pulse_C.Write();
  delayed_clean_Ge_last_pulse_D.Write();
  delayed_clean_Ge_last_pulse_E.Write();

  dd_time_Ge_clean.Write();
  dd_time_Ge_clean_wp.Write();
  delayed_Ge_clean_VS_prompt_calo.Write();
  delayed_Ge_clean_VS_delayed_calo_wop.Write();

  number_of_pulses_detected.Write();
  preprompt_spectra.Write();

  time_vs_run.Write();

  delayed_E_VS_time_Ge_clean.Write();
  delayed_E_VS_time_Ge_clean_wp.Write();

  dT_VS_sumGe.Write();
  delayed_Ge_C2_VS_total_Ge.Write();
  delayed_Ge_C2_VS_prompt_Ge.Write();
  delayed_Ge_C2_VS_prompt_mult.Write();
  delayed_Ge_C2_VS_total_Ge_cleaned.Write();
  delayed_Ge_C2_VS_total_Ge_rejected.Write();
  delayed_Ge_C3_VS_total_Ge.Write();
  delayed_Ge_C3_tot3_VS_total_Ge.Write();

  delayed_Ge_C1_VS_prompt_Ge.Write();
  delayed_Ge_C1_VS_delayed_calo.Write();

  E_VS_time_BGO_wp.Write();
  E_VS_time_LaBr_wp.Write();
  E_VS_time_NaI_wp.Write();

#else //QUALITY
  std::sort(runs.begin(), runs.end());
  for (auto const label : detectors.labels())
  {
    time_vs_run_each_det.at(label).Write();
  }
  for (auto & run : runs) time_vs_det_each_run.at(run).Write();
#endif //QUALITY

  outfile->Write();
  outfile->Close();
  print(g_outfilename, "written");
}

void reader(int number_files = -1)
{
#if defined(QUALITY) || defined(DEBUG)
  Analysator::setMaxHits(1.e+7);
#endif //QUALITY or DEBUG
  
  if (found(Path::pwd().string(), "faster")) 
  {
    MTObject::Initialize((nb_threads<0) ? 10 : nb_threads);
    if (simple_d) Analysator analysator(number_files, "~/nuball2/N-SI-136-root_d/merged/");
    else          Analysator analysator(number_files, "~/nuball2/N-SI-136-root_dd/merged/");
  }
  else 
  {
    MTObject::Initialize((nb_threads<0) ? 2 : nb_threads);
    if (simple_d) Analysator analysator(number_files, "~/faster_data/N-SI-136-root_d/");
    else          Analysator analysator(number_files, "~/faster_data/N-SI-136-root_dd/");
  }
}

int main(int argc, char** argv)
{
  int nb_files = -1;

  for (int i = 1; i<argc; ++i)
  {
    std::string param(argv[i]);
         if (param == "-f") nb_files = std::stoi(argv[++i]);
    else if (param == "-n") Analysator::setMaxHits(std::stoi(argv[++i]));
    else if (param == "-d") {simple_d = true; print("Reading N-SI-136-root_d");}
    else if (param == "-m") nb_threads = std::stoi(argv[++i]);
    else
    {
      print("Usage of reader :");
      print("-f : number of files");
      print("-n : number of hits per file");
      print("-d : single clean Ge trigger");
      print("-m : number of threads");
    }
  }
  reader(nb_files);
  return 1;
}




//////////////////////////////////////
//            DEPRECATED            //
//////////////////////////////////////






    // RF_Manager rf;

    // Clovers clovers;

    // auto const & nb_events = tree->GetEntries();
    // for (int event_i = 0; event_i<nb_events; event_i++)
    // {
    //   if(event_i%int_cast(10.e+6) == 0) print(event_i/1.e+6, "Mevts");

    //   tree->GetEntry(event_i);
    //   // clovers.Reset();

    //   std::vector<CloverModule> clovers_prompt(24);
    //   std::vector<uchar> clovers_prompt_fired;
    //   CloverModule::resetGlobalLabel();
    //   std::vector<CloverModule> clovers_delayed(24);
    //   std::vector<uchar> clovers_delayed_fired;

    //   float totalE_prompt = 0;
    //   float totalE_prompt_200ns = 0;
    //   float totalE_delayed = 0;
    //   float energyDSSD = 0;

    //   // Some triggers :
    //   bool has_prompt = false;
    //   bool has_prompt_particle = false;
    //   bool has_prompt_particle_E5 = false; // Particle prompt with energy<5MeV

    //   if (event.size()==1) continue;

    //   Time_ns last_prompt_time = -1000;
    //   for (int hit_i = 0; hit_i<event.mult; hit_i++)
    //   {
    //     // clovers.Fill(event, hit_i);
    //     if (rf.setEvent(event)) continue;
    //     auto const & label = event.labels[hit_i];
    //     auto const & nrj2  = event.nrj2s [hit_i];
    //     auto const & time  = event.times [hit_i];
    //     auto       & nrj   = event.nrjs  [hit_i];
        
    //     float const & time_ns = time/1000.f;

    //     if (time_ns<-20) continue;

    //     if (nrj<5) continue;

    //     // To correct for wrong BGO calibration :
    //     if (isBGO[label]) nrj *= 1.15;

    //     // To correct the NaI calibration :
    //     if(isParis[label] && NaI_pid(nrj, nrj2)) nrj = nrj2*1.1;
        
    //     if (isPrompt(time_ns)) 
    //     {
    //       has_prompt = true;
    //       if (isDSSD[label]) 
    //       {
    //         has_prompt_particle = true;
    //         if (isSector[label])
    //         {
    //           energyDSSD = nrj;
    //           if (nrj<5000) has_prompt_particle_E5 = true;
    //         }
    //       }
    //     }

    //     if (Clovers::isClover[label])
    //     {
    //       auto const & clover_label = Clovers::labels[label];
    //       if (isPrompt(time_ns))
    //       {
    //         push_back_unique(clovers_prompt_fired, clover_label);
    //         if (isGe[label])  clovers_prompt[clover_label].addGe (time, nrj);
    //         if (isBGO[label]) clovers_prompt[clover_label].addBGO(time, nrj);
    //       }
    //       else if (isDelayed(time_ns))
    //       {
    //         push_back_unique(clovers_delayed_fired, clover_label);
    //         if (isGe[label])  clovers_delayed[clover_label].addGe (time, nrj);
    //         if (isBGO[label]) clovers_delayed[clover_label].addBGO(time, nrj);
    //       }
    //     }

    //     // Calculate calorimetry :
    //     if (isParis[label] || isBGO[label] || isGe[label]) 
    //     {
    //       auto const & smeared_energy = smear(nrj, label, random);
    //       if(isPrompt(time_ns)) 
    //       {
    //         totalE_prompt+=smeared_energy;
    //       }
    //       else if(isDelayed(time_ns)) totalE_delayed+=smeared_energy;
    //       // print(nrj,smear(nrj, label, random), totalE_prompt, totalE_delayed);
    //       // pauseCo();
    //     }

    //     if (isGe[label])
    //     {
    //       if (time_ns<20) prompt_Ge.Fill(nrj);
    //       else
    //       {
    //         delayed_Ge.Fill(nrj);
    //         if (has_prompt) delayed_Ge_wp.Fill(nrj);
    //         if (has_prompt_particle) delayed_Ge_wpp.Fill(nrj);
    //         if (has_prompt_particle_E5) delayed_Ge_wppE.Fill(nrj);
    //       }
    //     }
    //     else if (isParis[label])
    //     {
    //       if (isPrompt(time_ns)) prompt_NaI.Fill(nrj);
    //       if (isPrompt(time_ns)) prompt_LaBr.Fill(nrj);
    //     }
    //     else if (isBGO[label])
    //     {
    //       if(isPrompt(time_ns)) prompt_BGO.Fill(nrj);
    //       else if(isDelayed(time_ns)) delayed_BGO.Fill(nrj);
    //     }
    //   }
      
    //   if (totalE_prompt>5) 
    //   {
    //     prompt_calo.Fill(totalE_prompt);
    //     closest_prompt_calo_histo.Fill(totalE_prompt);
    //   }

    //   if (totalE_delayed>5)
    //   {
    //     delayed_calo.Fill(totalE_delayed);
    //     if(has_prompt) delayed_calo_wp.Fill(totalE_delayed);
    //     if(has_prompt_particle) delayed_calo_wpp.Fill(totalE_delayed);
    //     if(has_prompt_particle_E5) delayed_calo_wppE.Fill(totalE_delayed);
    //     for (size_t hit_i = 0; hit_i<event.size(); hit_i++) 
    //     {
    //       if (isGe[event.labels[hit_i]])
    //       {
    //         if (event.times[hit_i]>20000)
    //         {
    //           delayed_Ge_VS_delayed_calo.Fill(totalE_delayed, event.nrjs[hit_i]);
    //           if(has_prompt) delayed_Ge_VS_delayed_calo_wp.Fill(totalE_delayed, event.nrjs[hit_i]);
    //           if(has_prompt_particle) delayed_Ge_VS_delayed_calo_wpp.Fill(totalE_delayed, event.nrjs[hit_i]);
    //           if(has_prompt_particle_E5) delayed_Ge_VS_delayed_calo_wppE.Fill(totalE_delayed, event.nrjs[hit_i]);
    //         }
    //         else // for prompt
    //         {
    //           delayed_Ge_VS_prompt_calo.Fill(totalE_prompt, event.nrjs[hit_i]);
    //         }
    //       } 
    //     }
    //   }

    //   if (totalE_prompt>5 && totalE_delayed>5) 
    //   {
    //     prompt_delayed_calo.Fill(totalE_prompt, totalE_delayed);
    //     for (size_t hit_i = 0; hit_i<event.size(); hit_i++) if (isGe[event.labels[hit_i]])
    //     {
    //        if (isGe[event.labels[hit_i]]) 
    //       {
    //         for (size_t hit_j = hit_i+1; hit_j<event.size(); hit_j++) if(isGe[event.labels[hit_j]])
    //         {
    //           dd_wp.Fill(event.nrjs[hit_i], event.nrjs[hit_j]);
    //           dd_wp.Fill(event.nrjs[hit_j], event.nrjs[hit_i]);

    //           if(has_prompt_particle)
    //           {
    //             dd_wpp.Fill(event.nrjs[hit_i], event.nrjs[hit_j]);
    //             dd_wpp.Fill(event.nrjs[hit_j], event.nrjs[hit_i]);
    //           }

    //           if(has_prompt_particle_E5)
    //           {
    //             dd_wppE.Fill(event.nrjs[hit_i], event.nrjs[hit_j]);
    //             dd_wppE.Fill(event.nrjs[hit_j], event.nrjs[hit_i]);
    //           }
    //         }
    //       }
    //     }
    //   }
      
    //   if (totalE_delayed > 1000 && totalE_delayed < 3500)
    //   {
    //     for (size_t hit_i = 0; hit_i<event.size(); hit_i++) if (isGe[event.labels[hit_i]]) for (size_t hit_j = hit_i+1; hit_j<event.size(); hit_j++) if(isGe[event.labels[hit_j]])
    //     {
    //       dd.Fill(event.nrjs[hit_i], event.nrjs[hit_j]);
    //       dd.Fill(event.nrjs[hit_j], event.nrjs[hit_i]);
    //     }
    //   }
    // }
    