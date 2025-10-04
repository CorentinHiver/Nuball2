#include <MTRootReader.hpp>
#include <Detectors.hpp>
#include <Event.hpp>
#include <RF_Manager.hpp>
#include <FilesManager.hpp>
#include <CloverModule.hpp>
#include <MultiHist.hpp>
#include <RWMat.hxx>
#include <MTList.hpp>
// #include <ParisCluster.hpp>
#include <Paris.hpp>
#include <Timer.hpp>
// Version 2 : reads the root files with the double delayed between 0 and 200 and the previous hits between -n_pulses*200 and 0.

// Version 3 (current) : reads the root files with the double delayed between 0 and n_pulses*200 and the prompt at 0

bool simple_d = false;
bool read_129 = false;
bool read_C2 = false;
bool read_new = false;
bool read_particle = false;
std::string trigger = "dd";


/**
 * @brief A class to simplify the analysis of the Clover data
 * @details Removes the handling of the crystal of maximal energy deposit.
 */
class MyClovers
{
public:
  MyClovers() noexcept
  {
    CloverModule::resetGlobalLabel(); // This allows to correctly label the CloverModules of another instance of MyClovers  
  };
  inline auto const & operator[](int const & i) const noexcept { return m_clovers[i]; }
  static inline uchar subIndex(Label const & label) noexcept {return (label+1)%6;}
  static inline bool isGe(Label const & label) noexcept {return subIndex(label)>1;}
  static inline Label index(Label const & label) noexcept {return (label-23)/6;}
  bool fill(Event const & event, int const & hit_i)
  {
    if (analyzed) Colib::throw_error("MyClovers::fill() called while already analyzed, you need to MyClovers::reset first");
    auto const & label = event.labels[hit_i];
    if (isClover[label])
    {
      auto const & nrj = event.nrjs[hit_i];
      auto const & time = event.times[hit_i];
      auto const & clover_index = MyClovers::index(label); // label = 23 -> index = 0, label = 196 -> index = 23;
      auto const & sub_index = subIndex(label);
      push_back_unique(Hits, clover_index);
      m_clovers[clover_index].addHit(nrj, time, sub_index);
      calorimetryTotal+=nrj;
      if (isGe(label))
      {
        push_back_unique(Ge, clover_index);
        calorimetryGe+=nrj;
      }
      else
      {
        push_back_unique(BGO, clover_index);
        calorimetryBGO+=nrj;
      }
      return true;
    }
    return false;
  }

  void analyze() noexcept
  {
    if (analyzed) return;
    analyzed = true;
    for (auto const & clover_index : Hits)
    {
      auto const & clover = m_clovers[clover_index];
      auto const & Ge_found = clover.nb>0;
      auto const & BGO_found = clover.nbBGO>0;

      if (Ge_found && BGO_found) Rejected.push_back(clover_index);
      else if (Ge_found &&!BGO_found) GeClean.push_back(clover_index);
      else if (!Ge_found &&BGO_found) BGOClean.push_back(clover_index);
    }
  }

  void reset()
  {
    calorimetryTotal = 0.0;
    calorimetryGe = 0.0;
    calorimetryBGO = 0.0;
    analyzed = false;
    for(auto const & clover : Hits) m_clovers[clover].reset();
    Hits.clear();
    Ge.clear();
    BGO.clear();
    GeClean.clear();
    BGOClean.clear();
    Rejected.clear();
  }

  double calorimetryGe = 0.0;
  double calorimetryBGO = 0.0;
  double calorimetryTotal = 0.0;
  
  std::vector<Label> Hits;
  std::vector<Label> Ge;
  std::vector<Label> BGO;
  std::vector<Label> GeClean;
  std::vector<Label> BGOClean;
  std::vector<Label> Rejected;

private:
  std::array<CloverModule, 24> m_clovers;
  bool analyzed = false;
};

std::ostream & operator << (std::ostream & os, MyClovers const & clovers) noexcept
{
  os << clovers.Hits.size() << " clovers hit :" << std::endl;
  for (auto const & clover_i : clovers.Hits)
  {
    os << clovers[clover_i] << std::endl;
  }
  return os;
}

// class MyParisWall
// {
// public:
//   MyParisWall() noexcept = default;

//   void fill(Event const & event, int const & hit_i)
//   {
//     auto const & label = event.labels[hit_i];
    
//   }

// private:

//   std::array<
// };

Label_vec const blacklist = {800, 801};
std::unordered_map<Label, double> const maxE_Ge = {{28, 7500}, {33, 8250}, {46, 9000}, {55, 7500}, {57, 6000}, 
                                                   {68, 7000}, {71, 9500}, {85, 7500}, {91, 8000}, {134, 8500}, 
                                                   {135, 8500}, {136, 9000}, {142, 6000}, {145, 8000}, {146, 9000},
                                                   {147, 9000}, {157, 9000}, {158, 9000}, {159, 9000}};

std::string g_out_filename = "tests.root";
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
  Analysator(int const & number_files, std::string const & data_path)
  // Analysator(int const & number_files, std::string const & data_path = "~/nuball2/N-SI-136-root_dd/")
  {
    print("reading", data_path);
    this->Initialise();
    MTRootReader reader(data_path, number_files);
    print("Launching the reader");
    reader.read([&](Nuball2Tree & tree, Event & event){this->analyze(tree, event);});
    write();
  }

  void Initialise();
  void analyze(Nuball2Tree & tree, Event & event);
  void write();

  static void setMaxHits(int const & nb_max_hits) {if (nb_max_hits>0) g_nb_max_hits = nb_max_hits;}

private:
  /// @brief static 
  static long g_nb_max_hits;

  TRandom* random = new TRandom();
  MTList MTfiles;
  std::vector<int> runs;

  // Histograms :
#ifndef QUALITY
  MultiHist<TH1F> prompt_Ge;
  MultiHist<TH2F> prompt_Clover_VS_sub_ring;
  MultiHist<TH2F> prompt_Clover_VS_label;
  MultiHist<TH1F> delayed_Ge;
  MultiHist<TH1F> prompt_BGO;
  MultiHist<TH1F> delayed_BGO;
  MultiHist<TH1F> prompt_NaI;
  MultiHist<TH1F> delayed_NaI;
  MultiHist<TH1F> prompt_LaBr;
  MultiHist<TH1F> delayed_LaBr;

  MultiHist<TH1F> paris_pid;
  MultiHist<TH2F> paris_long_vs_short;
  MultiHist<TH2F> paris_long_vs_short_kept;

  MultiHist<TH2F> timing_each_clover_Ge;
  MultiHist<TH2F> timing_each_clover_BGO;
  MultiHist<TH2F> nrj_each_clover_Ge_delayed;
  MultiHist<TH2F> nrj_each_clover_Ge_prompt;
  MultiHist<TH2F> nrj_each_clover_BGO_delayed;
  MultiHist<TH2F> nrj_each_clover_BGO_prompt;

  MultiHist<TH1F> prompt_clean_Ge;
  MultiHist<TH1F> delayed_clean_Ge;

  MultiHist<TH1F> prompt_calo;
  MultiHist<TH1F> delayed_calo;

  MultiHist<TH2F> prompt_delayed_calo;
  MultiHist<TH2F> delayed_Ge_VS_delayed_calo;
  MultiHist<TH2F> delayed_Ge_VS_prompt_calo;

  MultiHist<TH2F> spectra_all;
  MultiHist<TH2F> spectra_all_NaI;
  MultiHist<TH2F> spectra_Ge_VS_run;
  MultiHist<TH2F> spectra_BGO_VS_run;
  MultiHist<TH2F> spectra_LaBr_VS_run;
  MultiHist<TH2F> spectra_NaI_VS_run;
  Vector_MTTHist<TH2F> spectra_NaI_VS_det;


  MultiHist<TH2F> delayed_E_VS_time_Ge_clean;
  MultiHist<TH2F> time_all;
  MultiHist<TH2F> time_NaI;

  MultiHist<TH1F> delayed_Ge_wpp;
  MultiHist<TH1F> delayed_calo_wpp;
  MultiHist<TH1F> prompt_Ge_wpp;
  MultiHist<TH2F> delayed_Ge_VS_delayed_calo_wpp;

  MultiHist<TH1F> delayed_Ge_wppE;
  MultiHist<TH1F> delayed_calo_wppE;
  MultiHist<TH2F> delayed_Ge_VS_delayed_calo_wppE;

  MultiHist<TH2F> pp;
  MultiHist<TH2F> dd;
  MultiHist<TH2F> dd_wp;
  MultiHist<TH2F> dd_wpp;
  MultiHist<TH2F> dd_wppE;
  MultiHist<TH2F> dp;

  MultiHist<TH2F> dd_time_Ge_clean;
  MultiHist<TH2F> dd_time_Ge_clean_wp;
  MultiHist<TH2F> delayed_Ge_clean_VS_prompt_calo;
  MultiHist<TH2F> delayed_Ge_clean_VS_delayed_calo_wop; // With one prompt

  MultiHist<TH1F> number_of_pulses_detected;
  MultiHist<TH1F> preprompt_spectra;

  MultiHist<TH2F> time_vs_run;

  MultiHist<TH2F> dT_VS_sumGe;
  MultiHist<TH2F> delayed_Paris_VS_Germanium;
  MultiHist<TH2F> delayed_Ge_C2_VS_total_Ge;
  MultiHist<TH2F> delayed_Ge_C2_VS_prompt_Ge;
  MultiHist<TH2F> delayed_Ge_C2_VS_prompt_mult;
  MultiHist<TH2F> delayed_Ge_C2_VS_total_Ge_cleaned;
  MultiHist<TH2F> delayed_Ge_C2_VS_total_Ge_rejected;
  MultiHist<TH2F> delayed_Ge_C3_VS_total_Ge;
  MultiHist<TH2F> delayed_Ge_C3_tot3_VS_total_Ge;
  
  // Simple clean Ge delayed trigger :
  MultiHist<TH2F> delayed_Ge_C1_VS_prompt_Ge;
  MultiHist<TH2F> delayed_Ge_C1_VS_delayed_calo;

  MultiHist<TH2F> BGO_with_trigger_Clover_511;
  MultiHist<TH2F> DSSD_VS_Clover_717;
  MultiHist<TH2F> LaBr3_with_trigger_Clover_511;
  MultiHist<TH2F> NaI_with_trigger_Clover_511;
  MultiHist<TH2F> LaBr3_with_trigger_LaBr3_511;
  MultiHist<TH2F> BGO_with_trigger_BGO_511;

  // Different triggers on the delayed Ge spectra :
  MultiHist<TH2F> delayed_Ge_VS_DeM;

  #else //QUALITY

  Map_MTTHist<TH2F> time_vs_run_each_det;
  Map_MTTHist<TH2F> nrj_vs_run_each_det;
  Map_MTTHist<TH2F> time_vs_det_each_run;
  Map_MTTHist<TH2F> nrj_vs_det_each_run;

  #endif//QUALITY
};

long Analysator::g_nb_max_hits = -1;

void Analysator::Initialise()
{
  randomCo::setSeed(time(0));
  detectors.load("index_129.list");
  random->SetSeed(time(0));
  // Clovers::InitialiseArrays();
  // Clovers::timePs(true);
  int run_min = 70;
  int run_max = 130;
  if (read_129)
  {
    run_min = 20;
    run_max = 110;
  }
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
  
  prompt_Clover_VS_sub_ring.reset("prompt_Clover_VS_sub_ring" , "prompt Clover VS cristal;E[keV];ring_n°+sub_cristal" , 5000,0,10000, 10,-1,9);
  prompt_Clover_VS_label.reset("prompt_Clover_VS_label" , "prompt Clover VS cristal label;E[keV];cristal label" , 5000,0,10000, 10,-1,9);

  paris_pid.reset("paris_pid" , "paris pid;E[keV]" , 1000,-2,2);
  paris_long_vs_short.reset("paris_long_vs_short", "paris long vs short", 1000,-200,20000, 1000,-200,20000);
  paris_long_vs_short_kept.reset("paris_long_vs_short_kept", "paris long vs short kept", 1000,-200,20000, 1000,-200,20000);

  timing_each_clover_Ge.reset("timing_each_clover_Ge", "timing each clover Ge", 27,-2,25, 400,-100000,300000);
  timing_each_clover_BGO.reset("timing_each_clover_BGO", "timing each clover BGO", 27,-2,25, 400,-100000,300000);
  nrj_each_clover_Ge_delayed.reset("nrj_each_clover_Ge_delayed", "nrj each clover Ge delayed", 27,-2,25, 5000,0,10000);
  nrj_each_clover_Ge_prompt.reset("nrj_each_clover_Ge_prompt", "nrj each clover Ge prompt", 27,-2,25, 5000,0,10000);
  nrj_each_clover_BGO_delayed.reset("nrj_each_clover_BGO_delayed", "nrj each clover BGO delayed", 27,-2,25, 1000,0,20000);
  nrj_each_clover_BGO_prompt.reset("nrj_each_clover_BGO_prompt", "nrj each clover BGO prompt", 27,-2,25, 1000,0,20000);

  number_of_pulses_detected.reset("number_of_pulses_detected","number of pulses detected", 5,0,5);

  // Calorimetry
  prompt_calo.reset("prompt_calo" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  delayed_calo.reset("delayed_calo", "delayed calorimetry;E[keV]", 5000,0,10000);
  prompt_delayed_calo.reset("prompt_delayed_calo", "Delayed VS closest prompt calorimetry;Prompt Calorimetry[keV];Delayed Calorimetry[keV]", 
      1000,0,20000, 1000,0,20000);
  delayed_Ge_VS_delayed_calo.reset("delayed_Ge_VS_delayed_calo", "Delayed Ge VS delayed calorimetry;Delayed Calorimetry[keV];E[keV]", 
      500,0,10000, 5000,0,10000);
  delayed_Ge_VS_prompt_calo.reset("delayed_Ge_VS_prompt_calo", "Delayed Ge VS prompt calorimetry;Prompt Calorimetry[keV];E[keV]", 
      20000,0,10000, 500,0,20000);

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
  // pp.reset("pp", "pp;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  pp.reset("pp", "pp;E[keV];E[keV]", 4096,0,2048, 4096,0,2048);
  dd.reset("dd", "dd;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);
  dp.reset("dp", "delayed VS prompt;E[keV];E[keV]", 4096,0,4096, 4096,0,4096);

  spectra_all.reset("spectra_all", "Spectra;label;Energy [keV]", 1000,0,1000, 10000,0,10000);
  spectra_all_NaI.reset("spectra_all_NaI", "Spectra;label;Energy [keV]", 1000,0,1000, 10000,0,10000);

  spectra_Ge_VS_run.reset("spectra_Ge_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_BGO_VS_run.reset("spectra_BGO_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_LaBr_VS_run.reset("spectra_LaBr_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);
  spectra_NaI_VS_run.reset("spectra_NaI_VS_run", "Spectra;run number;Energy [keV]", 100,50,150, 2000,0,2000);

  // spectra_NaI_VS_det.resize(1000);
  // for (int label = 0; label<1000; ++label) if (isParis[label])
  // {
  //   std::string spectra = "spectra_NaI_VS_det"+std::to_string(label);
  //   spectra_NaI_VS_det[label].reset(spectra.c_str(), (spectra+";Clover [keV];NaI [keV]").c_str(), 2000,0,2000, 500,0,2000);
  // }

  // Timing
  time_all.reset("time_all", "Time", 1000,0,1000, 2000,-1000,1000);
  time_NaI.reset("time_NaI", "Time NaI", 1000,0,1000, 2000,-1000,1000);

  preprompt_spectra.reset("preprompt_spectra", "preprompt spectra", 10000,0,10000);
  time_vs_run.reset("time_vs_run","time vs run", nb_runs,run_min,run_max, 750, -1000, 500);

  delayed_E_VS_time_Ge_clean.reset("delayed_E_VS_time_Ge_clean", "Energy vs time delayed;time [ns];Energy [keV]", 500,-1000,1000, 5000,0,10000);

  dd_time_Ge_clean.reset("dd_time_Ge_clean", "dd time Ge clean;time [ns];time [ns]", 500,0,500,500,0,500);
  dd_time_Ge_clean_wp.reset("dd_time_Ge_clean_wp", "dd time Ge clean wp;time [ns];time [ns]", 500,0,500,500,0,500);
  delayed_Ge_clean_VS_prompt_calo.reset("delayed_Ge_clean_VS_prompt_calo", "delayed Ge clean VS prompt calo", 500,0,15000,10000,0,10000);
  delayed_Ge_clean_VS_delayed_calo_wop.reset("delayed_Ge_clean_VS_delayed_calo_wop", "delayed Ge clean VS delayed calo with only one prompt before", 500,0,15000,10000,0,10000);

  // When only 2-3 germaniums, plot germaniums VS the sum of them
  delayed_Ge_C2_VS_total_Ge.reset("delayed_Ge_C2_VS_total_Ge", "Clean Ge VS sum clean Ge C2", 2000,0,4000, 5000,0,5000);
  delayed_Ge_C2_VS_prompt_Ge.reset("delayed_Ge_C2_VS_prompt_Ge", "Clean delayed Ge C2 VS prompt Ge", 2000,0,4000, 5000,0,5000);
  delayed_Ge_C2_VS_prompt_mult.reset("delayed_Ge_C2_VS_prompt_mult", "Clean Ge C2 VS prompt Ge mult", 15,0,15, 2000,0,4000);
  delayed_Ge_C2_VS_total_Ge_cleaned.reset("delayed_Ge_C2_VS_total_Ge_cleaned", "Clean Ge VS sum clean Ge C2 cleaned", 2000,0,4000, 5000,0,5000 );
  delayed_Ge_C2_VS_total_Ge_rejected.reset("delayed_Ge_C2_VS_total_Ge_rejected", "Clean Ge VS sum clean Ge C2 rejected", 2000,0,4000, 5000,0,5000);
  delayed_Ge_C3_VS_total_Ge.reset("delayed_Ge_C3_VS_total_Ge", "Clean Ge VS sum clean Ge C3 - only 2 by 2", 2000,0,4000, 5000,0,5000);
  delayed_Ge_C3_tot3_VS_total_Ge.reset("delayed_Ge_C3_tot3_VS_total_Ge", "Clean Ge VS sum clean Ge C3", 2000,0,4000, 5000,0,5000);

  // For TSC method :
  dT_VS_sumGe.reset("dT_VS_sumGe", "dT VS ESum Ge", 10000,0,10000, 100,-200,200);

  // Paris_VS_Germaniums :
  delayed_Paris_VS_Germanium.reset("delayed_Paris_VS_Germanium", "delayed Paris VS Germanium", 2000,0,10000, 10000,0,10000);
  
  if (simple_d)
  {
    delayed_Ge_C1_VS_delayed_calo.reset("delayed_Ge_C1_VS_delayed_calo", "Clean Ge C1 VS delayed calo", 500,0,5000, 5000,0,5000);
    delayed_Ge_C1_VS_prompt_Ge.reset("delayed_Ge_C1_VS_prompt_Ge", "Clean Ge C1 VS prompt Ge mult", 2000,0,4000, 5000,0,5000);
  }

  BGO_with_trigger_Clover_511.reset("BGO_with_trigger_Clover_511","BGO with trigger Clover 511", 1000,0,1000, 1000,0,5000);
  BGO_with_trigger_BGO_511.reset("BGO_with_trigger_BGO_511","BGO with trigger BGO 511", 1000,0,1000, 1000,0,5000);
  LaBr3_with_trigger_Clover_511.reset("LaBr3_with_trigger_Clover_511","LaBr3 with trigger Clover 511", 1000,0,1000, 2500,0,5000);
  NaI_with_trigger_Clover_511.reset("NaI_with_trigger_Clover_511","NaI with trigger Clover 511", 1000,0,1000, 2500,0,5000);
  LaBr3_with_trigger_LaBr3_511.reset("LaBr3_with_trigger_LaBr3_511","LaBr3 with trigger LaBr3 511", 1000,0,1000, 2500,0,5000);
  DSSD_VS_Clover_717.reset("DSSD_VS_Clover_717","DSSD VS Clover 717", 1000,0,1000, 1000,0,30000);

  // Different triggers on the delayed Ge spectra :
  delayed_Ge_VS_DeM.reset("delayed_Ge_VS_DeM", "delayed Ge VS delayed module multiplicity;Module multiplicity;E[keV]", 20,0,20, 20000,0,10000);

  // Run quality :
#else // if QUALITY
  print("_______________");
  print("Running quality check");

  for (auto const & label : detectors.labels())
  {
    std::string name = "time_vs_run_"+detectors[label];
    time_vs_run_each_det.emplace(label, MultiHist<TH2F>(name.c_str(), "time vs run each;run number;time [ns]", nb_runs,run_min,run_max, 2000,-1000,1000));
    name = "nrj_vs_run_"+detectors[label];
    nrj_vs_run_each_det.emplace(label, MultiHist<TH2F>(name.c_str(), "E vs run each;run number;Energy [keV]", nb_runs,run_min,run_max, 2000, 0,2000));
  }
  for (int run_i = run_min; run_i<run_max+1; run_i++)
  {
    auto name = ("time_vs_det_run_"+std::to_string(run_i));
    auto title = concatenate("time vs det run", run_i, ";run number;time [ns]");
    time_vs_det_each_run.emplace(run_i, MultiHist<TH2F>(name.c_str(), title.c_str(), 900,0,900, 2000,-1000,1000));
    name = ("nrj_vs_det_run_"+std::to_string(run_i));
    title = concatenate("nrj vs det run", run_i, ";run number;Energy [keV]");
    nrj_vs_det_each_run.emplace(run_i, MultiHist<TH2F>(name.c_str(), title.c_str(), 900,0,900, 1000, 0,1000));
  }
#endif //QUALITY
}

// bool isDelayed(Time_ns const & time_ns) {return ((int_cast(time_ns)%200)>60 && (int_cast(time_ns)%200)<170);}

void Analysator::analyze(Nuball2Tree & tree, Event & event)
{
  File file(tree.filename());
  Timer timer;
  int run_number = std::stoi(getList(file.shortName(), '_')[1]);
MTObject::mutex.lock();
  runs.push_back(run_number);
MTObject::mutex.unlock();

  // Bools isNaI;
  // Bools isLaBr;
  // Bools rejected;

  // // std::vector<MyClovers> other_clover_hits;
  // MyClovers clovers_prompt;
  // MyClovers clovers_delayed;

  // Paris paris_prompt;
  // Paris paris_delayed;

  RF_Manager rf;

  auto nb_evts = tree->GetEntries();
  if (g_nb_max_hits>-1) nb_evts = g_nb_max_hits;
  for (int evt_i = 0; evt_i<nb_evts; ++evt_i)
  { // Iterate over the events of the file
    if(evt_i>0 && evt_i%int_cast(1.e+7) == 0) printC(Colib::nicer_double(evt_i, 2), "evts"); 

    tree->GetEntry(evt_i);

    if (rf.setEvent(event)) continue;
  
  #ifndef QUALITY
    // isNaI   .resize(event.mult, false);
    // isLaBr  .resize(event.mult, false);
    // rejected.resize(event.mult, false);

    // clovers_delayed.reset();
    // clovers_prompt.reset();

    int prompt_mult = 0;

    // std::vector<bool> is_prompt(event.mult, false);
    // std::vector<bool> is_delayed(event.mult, false);

    // Data pre-treatment :
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {// Iterate over the hits of the event
      auto const & label = event.labels[hit_i];
      auto const & time  = event.times [hit_i];
      auto const & nrj2  = event.nrj2s [hit_i];
      auto const & nrj   = event.nrjs  [hit_i];
      auto const & time_ns = time/1000.0;

      for (int hit_j = hit_i+1; hit_j<event.mult; hit_j++) 
      {
        auto const & label_j = event.labels[hit_j];
        auto const & time_j  = event.times [hit_j];
        auto const & nrj2_j  = event.nrj2s [hit_j];
        auto const & nrj_j   = event.nrjs  [hit_j];
        auto const & time_ns_j = time/1000.0;
        if (-10 < time_ns && time_ns < 10)
        {
          if (-10 < time_ns_j && time_ns_j < 10)
          {
            pp.Fill(nrj, nrj_j);
            pp.Fill(nrj_j, nrj);
          }
          else if (60 < time_ns_j && time_ns_j < 170)
          {
            dp.Fill(nrj, nrj_j);
          }
        }
        else if (60 < time_ns && time_ns < 170) 
        {
          if (60 < time_ns_j && time_ns_j < 170)
          {
            dd.Fill(nrj, nrj_j);
            dd.Fill(nrj_j, nrj);
          }
          else if (-10 < time_ns_j && time_ns_j < 10)
          { // Hits are ordered, should never happen that the hit_i
            dp.Fill(nrj_j, nrj);
          }
        }
      }

    //   if (isDSSD[label]) {rejected[hit_i] = true; continue;} // There is nothing to do with the dssd now
    //   if (found(blacklist, label)) {rejected[hit_i] = true; continue;}
      
    //   // Throw events with too low energy
    //   if (nrj<5)
    //   {
    //     rejected[hit_i] = true;
    //     continue;
    //   }

    //   time_all.Fill(label, time_ns);
    //   time_vs_run.Fill(run_number, time_ns);

    //   //////////////////
    //   // GERMANIUMS : //
    //   //////////////////
    //   if (isGe[label])
    //   {
    //     if (find_key(maxE_Ge, label) && nrj>maxE_Ge.at(label))
    //     {
    //       rejected[hit_i] = true;
    //       continue;
    //     }
    //     if (time_ns<10)
    //     {
    //       is_prompt[hit_i] = true;
    //       ++prompt_mult;
    //       clovers_prompt.fill(event, hit_i);
    //     }      
    //     else if (60 < time_ns && time_ns < 170) 
    //     {
    //       is_delayed[hit_i] = true;
    //       clovers_delayed.fill(event, hit_i);
    //     }
    //     else 
    //     {
    //       rejected[hit_i] = true;
    //     }
    //   }

    //   ///////////
    //   // BGO : //
    //   ///////////
    //   else if (isBGO[label]) 
    //   {
    //     if (time_ns<10)
    //     {
    //       is_prompt[hit_i] = true;
    //       ++prompt_mult;
    //       clovers_prompt.fill(event, hit_i);
    //     }      
    //     else if (60 < time_ns && time_ns < 190) 
    //     {
    //       is_delayed[hit_i] = true;
    //       clovers_delayed.fill(event, hit_i);
    //     }
    //     else 
    //     {
    //       rejected[hit_i] = true;
    //     }
    //   }
      
    //   /////////////
    //   // PARIS : //
    //   /////////////
    //   else if (isParis[label]) 
    //   {
    //     paris_long_vs_short.Fill(nrj, nrj2);
    //     paris_long_vs_short_kept.Fill(nrj, nrj2);
    //     auto const & state = ParisPhoswitch::test_gate_simple(nrj, nrj2);
    //     switch (state)
    //     {
    //       case  0 : // Is LaBr3
    //         isLaBr[hit_i] = true;
    //         if (-5 < time_ns && time_ns < 5) 
    //         {
    //           is_prompt[hit_i] = true;
    //           prompt_LaBr .Fill(nrj);
    //         }
    //         else if (60 < time_ns && time_ns < 190) 
    //         {
    //           is_delayed[hit_i] = true;
    //           delayed_LaBr.Fill(nrj);
    //         }
    //         else 
    //         {
    //           rejected[hit_i] = true;
    //         }
    //         break;
    //       case  1 : // Is NaI
    //         isNaI[hit_i] = true;
    //         time_NaI.Fill(label, time_ns);
    //         if (time_ns<10) 
    //         {
    //           is_prompt[hit_i] = true;
    //           prompt_NaI .Fill(nrj);
    //         }
    //         else if (60 < time_ns && time_ns < 190) 
    //         {
    //           is_delayed[hit_i] = true;
    //           delayed_NaI.Fill(nrj);
    //         }
    //         else 
    //         {
    //           rejected[hit_i] = true;
    //         }
    //         break;
    //       case 2 : // Is Internal add-back :
    //       default : // All the rest : 
    //         rejected[hit_i] = true;
    //         break;
    //     }
    //   }

    //   else // Not a known detector hit :
    //   {
    //     rejected[hit_i] = true;
    //   }
    // }

    // ////////////////////
    // // ANALYSE CLOVER //
    // ////////////////////
    
    // clovers_delayed.analyze();
    // clovers_prompt.analyze();

    // // print(clovers_delayed);
    // // print(clovers_prompt);
    // // Colib::pause();

    // for (auto const & clover_i : clovers_prompt.BGO)
    // {
    //   auto const & clover = clovers_prompt[clover_i];
    //   prompt_BGO.Fill(clover.nrj_BGO);
    //   timing_each_clover_BGO.Fill(clover_i, clover.time_BGO);
    //   nrj_each_clover_BGO_prompt.Fill(clover_i, clover.nrj_BGO);
    // }

    // for (auto const & clover_i : clovers_delayed.BGO)
    // {
    //   auto const & clover = clovers_delayed[clover_i];
    //   delayed_BGO.Fill(clover.nrj_BGO);
    //   timing_each_clover_BGO.Fill(clover_i, clover.time_BGO);
    //   nrj_each_clover_BGO_delayed.Fill(clover_i, clover.nrj_BGO);
    // }

    // for (auto const & clover_i : clovers_prompt.Ge)
    // {
    //   auto const & clover = clovers_prompt[clover_i];
    //   timing_each_clover_Ge.Fill(clover_i, clover.time);
    //   nrj_each_clover_Ge_prompt.Fill(clover_i, clover.nrj);
    // }

    // for (auto const & clover_i : clovers_delayed.Ge)
    // {
    //   auto const & clover = clovers_delayed[clover_i];
    //   timing_each_clover_Ge.Fill(clover_i, clover.time);
    //   nrj_each_clover_Ge_delayed.Fill(clover_i, clover.nrj);
    // }

    // // Some aliases for the next parts :
    // auto const & delayed_indexes = clovers_delayed.GeClean; // Simple alias
    // auto const & prompt_indexes = clovers_prompt.GeClean; // Simple alias


    // //////////////////////
    // // Clovers prompt : //
    // //////////////////////

    // for (size_t clover_it_i = 0; clover_it_i<prompt_indexes.size(); ++clover_it_i)
    // {
    //   auto const & clover_i = clovers_prompt[prompt_indexes[clover_it_i]];
    //   auto const & label_i = prompt_indexes[clover_it_i];
    //   auto const & nrj_i = clover_i.nrj;
    //   auto const & time_i = clover_i.time;
    //   prompt_Ge.Fill(nrj_i);
    //   int sub_ring = 4*(clover_i.label()/12) + clover_i.maxE_Ge_cristal-2;
    //   Label label_cristal = clover_i.label()*4 + clover_i.maxE_Ge_cristal-2;
    //   prompt_Clover_VS_sub_ring.Fill(nrj_i, sub_ring);
    //   prompt_Clover_VS_label.Fill(nrj_i, label_cristal);
    //   // for(int hit_i = 0; hit_i<event.mult; ++hit_i) if(isNaI[hit_i]) spectra_NaI_VS_det[event.labels[hit_i]].Fill(nrj_i, event.nrjs[hit_i]);
    //   if (505<nrj_i && nrj_i<515) for(int hit_i = 0; hit_i<event.mult; ++hit_i)
    //   {
    //     auto const & label = event.labels[hit_i];
    //     auto const & nrj = event.nrjs[hit_i];
    //          if (isBGO[label]) BGO_with_trigger_Clover_511.Fill(label, nrj);
    //     else if (isLaBr[hit_i]) LaBr3_with_trigger_Clover_511.Fill(label, nrj);
    //     else if (isNaI[hit_i]) NaI_with_trigger_Clover_511.Fill(label, nrj);
    //   }
    //   if (715<nrj_i && nrj_i<720) for(int hit_i = 0; hit_i<event.mult; ++hit_i)
    //   {
    //     if (isDSSD[event.labels[hit_i]]) DSSD_VS_Clover_717.Fill(event.labels[hit_i], event.nrjs[hit_i]);
    //   }
    //   for (size_t clover_it_j = clover_it_i+1; clover_it_j<prompt_indexes.size(); ++clover_it_j)
    //   {
    //     auto const & nrj_j = clovers_prompt[prompt_indexes[clover_it_j]].nrj;
    //     pp.Fill(nrj_i, nrj_j);
    //     pp.Fill(nrj_j, nrj_i);
    //   }
    // }

    // ///////////////////////
    // // Clovers delayed : //
    // ///////////////////////

    // if ((delayed_indexes.size() == 2))
    // {
    //   auto const & time_0 = clovers_delayed[delayed_indexes[0]].time;
    //   auto const & time_1 = clovers_delayed[delayed_indexes[1]].time;
    //   auto const & nrj_0 = clovers_delayed[delayed_indexes[0]].nrj;
    //   auto const & nrj_1 = clovers_delayed[delayed_indexes[1]].nrj;
    //   auto const & dT = time_1 - time_0;
    //   if (dT>50) continue;
    //   auto const & calo = nrj_0+nrj_1;
    //   delayed_Ge_C2_VS_total_Ge.Fill(calo, nrj_0);
    //   delayed_Ge_C2_VS_total_Ge.Fill(calo, nrj_1);
    //   if(prompt_mult > 1 && prompt_mult < 5)
    //   {
    //     delayed_Ge_C2_VS_total_Ge_cleaned.Fill(calo, nrj_0);
    //     delayed_Ge_C2_VS_total_Ge_cleaned.Fill(calo, nrj_1);
    //     // dd_wp.Fill(nrj_0, nrj_1);
    //     // dd_wp.Fill(nrj_1, nrj_0);
    //   }
    //   else
    //   {
    //     delayed_Ge_C2_VS_total_Ge_rejected.Fill(calo, nrj_0);
    //     delayed_Ge_C2_VS_total_Ge_rejected.Fill(calo, nrj_1);
    //   }

    //   // Prompt VS delayed :
    //   for (auto const & index : prompt_indexes)
    //   {
    //     auto const & clover = clovers_prompt[index];
    //     delayed_Ge_C2_VS_prompt_Ge.Fill(clover.nrj, nrj_1);
    //     delayed_Ge_C2_VS_prompt_Ge.Fill(clover.nrj, nrj_0);
    //   }
    //   delayed_Ge_C2_VS_prompt_mult.Fill(prompt_mult, nrj_0);
    //   delayed_Ge_C2_VS_prompt_mult.Fill(prompt_mult, nrj_1);
    // }

    // if ((delayed_indexes.size() == 3)) 
    // {
    //   auto const & time_0 = clovers_delayed[delayed_indexes[0]].time;
    //   auto const & time_1 = clovers_delayed[delayed_indexes[1]].time;
    //   auto const & time_2 = clovers_delayed[delayed_indexes[2]].time;
    //   auto const & nrj_0 = clovers_delayed[delayed_indexes[0]].nrj;
    //   auto const & nrj_1 = clovers_delayed[delayed_indexes[1]].nrj;
    //   auto const & nrj_2 = clovers_delayed[delayed_indexes[2]].nrj;
    //   auto const & dT01 = time_1-time_0;
    //   auto const & dT02 = time_2-time_0;
    //   auto const & dT12 = time_2-time_1;
    //   if (dT01<50)
    //   {
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_1, nrj_0);
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_1, nrj_1);
    //   }
    //   if (dT02<50)
    //   {
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_2, nrj_0);
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_0+nrj_2, nrj_2);
    //   }
    //   if (dT12<50)
    //   {
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_1+nrj_2, nrj_1);
    //     delayed_Ge_C3_VS_total_Ge.Fill(nrj_1+nrj_2, nrj_2);
    //   }
    //   if (dT01<50 && dT02 < 50)
    //   {
    //     delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_0);
    //     delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_1);
    //     delayed_Ge_C3_tot3_VS_total_Ge.Fill(nrj_0+nrj_1+nrj_2, nrj_2);
    //   }
    // }

    // // All delayed hits :
    // for (size_t clover_it_i = 0; clover_it_i<delayed_indexes.size(); ++clover_it_i)
    // {
    //   auto const & clover_i = clovers_delayed[delayed_indexes[clover_it_i]];
    //   auto const & label_i = delayed_indexes[clover_it_i];
    //   auto const & nrj_i = clover_i.nrj;
    //   auto const & time_i = clover_i.time;
    //   delayed_Ge.Fill(nrj_i);
    //   if (505<nrj_i && nrj_i<515) for(int hit_i = 0; hit_i<event.mult; ++hit_i)
    //   {
    //          if (isBGO[event.labels[hit_i]]) BGO_with_trigger_Clover_511.Fill(event.labels[hit_i], event.nrjs[hit_i]);
    //     else if (isLaBr[hit_i]) LaBr3_with_trigger_Clover_511.Fill(event.labels[hit_i], event.nrjs[hit_i]);
    //     else if (isNaI[hit_i]) NaI_with_trigger_Clover_511.Fill(event.labels[hit_i], event.nrjs[hit_i]);
    //   }
        
    //   delayed_E_VS_time_Ge_clean.Fill(time_i, nrj_i);
      
    //   // delayed_Ge_VS_DeM.Fill();

    //   // Creating the gamma-gamma matrices :
    //   for (size_t clover_it_j = clover_it_i+1; clover_it_j<delayed_indexes.size(); ++clover_it_j)
    //   {
    //     auto const & clover_j = clovers_delayed[delayed_indexes[clover_it_j]];
    //     auto const & nrj_j = clover_j.nrj;
        
    //     dd.Fill(nrj_i, nrj_j);
    //     dd.Fill(nrj_j, nrj_i);
    //   }

    //   // Create the prompt-delayed matrix
    //   for (size_t clover_it_j = 0; clover_it_j<prompt_indexes.size(); ++clover_it_j)
    //   {
    //     auto const & clover_j = clovers_prompt[prompt_indexes[clover_it_j]];
    //     auto const & nrj_prompt = clover_j.nrj;
        
    //     dp.Fill(nrj_prompt, nrj_i);
    //   }
    }
  
  #else // if QUALITY

    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label = event.labels[hit_i];
      auto const & nrj   = event.nrjs  [hit_i];
      auto const & time  = event.times [hit_i];
      auto const & time_ns = time/1000.;

      time_vs_run_each_det[label].Fill(run_number, time_ns);
      nrj_vs_run_each_det[label].Fill(run_number, nrj);
      time_vs_det_each_run[run_number].Fill(label, time_ns);
      nrj_vs_det_each_run[run_number].Fill(label, nrj);
    }

  #endif // QUALITY
  }

  printC("File ", file.string(), " read in ", timer(3), " (", file.size("Mo")/timer.Time("s"), " Mo/s)");
}

void Analysator::write()
{
#ifdef QUALITY
  g_out_filename = "run_quality.root";
#endif //QUALITY

  g_out_filename=removeExtension(g_out_filename)+"_"+trigger+"."+extension(g_out_filename);
  auto outfile(TFile::Open(g_out_filename.c_str(), "RECREATE"));
  outfile->cd();

#ifndef QUALITY
  BGO_with_trigger_Clover_511.Write();
  DSSD_VS_Clover_717.Write();
  LaBr3_with_trigger_Clover_511.Write();
  NaI_with_trigger_Clover_511.Write();
  LaBr3_with_trigger_LaBr3_511.Write();
  BGO_with_trigger_BGO_511.Write();

  // RWMat RW_dd(dd); RW_dd.Write();
  pp.Write();
  dd.Write();
  dp.Write();

  prompt_Ge.Write();
  delayed_Ge.Write();
  prompt_BGO.Write();
  delayed_BGO.Write();
  prompt_NaI.Write();
  delayed_NaI.Write();
  prompt_LaBr.Write();
  delayed_LaBr.Write();

  prompt_Clover_VS_sub_ring.Write();
  prompt_Clover_VS_label.Write();

  paris_pid.Write();
  paris_long_vs_short.Write();
  paris_long_vs_short_kept.Write();

  delayed_clean_Ge.Write();

  timing_each_clover_Ge.Write();
  timing_each_clover_BGO.Write();
  nrj_each_clover_Ge_delayed.Write();
  nrj_each_clover_Ge_prompt.Write();
  nrj_each_clover_BGO_delayed.Write();
  nrj_each_clover_BGO_prompt.Write();

  prompt_calo.Write();
  delayed_calo.Write();

  prompt_delayed_calo.Write();
  delayed_Ge_VS_prompt_calo.Write();

  delayed_Ge_wpp.Write();
  delayed_calo_wpp.Write();
  prompt_Ge_wpp.Write();

  delayed_Ge_wppE.Write();
  delayed_calo_wppE.Write();

  delayed_Ge_VS_delayed_calo.Write();
  delayed_Ge_VS_delayed_calo_wpp.Write();
  delayed_Ge_VS_delayed_calo_wppE.Write();

  spectra_all.Write();
  spectra_all_NaI.Write();
  spectra_Ge_VS_run.Write();
  spectra_BGO_VS_run.Write();
  spectra_LaBr_VS_run.Write();
  spectra_NaI_VS_run.Write();

  time_all.Write();
  time_NaI.Write();

  dd_time_Ge_clean.Write();
  dd_time_Ge_clean_wp.Write();
  delayed_Ge_clean_VS_prompt_calo.Write();
  delayed_Ge_clean_VS_delayed_calo_wop.Write();

  number_of_pulses_detected.Write();
  preprompt_spectra.Write();

  time_vs_run.Write();

  delayed_E_VS_time_Ge_clean.Write();

  dT_VS_sumGe.Write();
  delayed_Paris_VS_Germanium.Write();
  delayed_Ge_C2_VS_total_Ge.Write();
  delayed_Ge_C2_VS_prompt_Ge.Write();
  delayed_Ge_C2_VS_prompt_mult.Write();
  delayed_Ge_C2_VS_total_Ge_cleaned.Write();
  delayed_Ge_C2_VS_total_Ge_rejected.Write();
  delayed_Ge_C3_VS_total_Ge.Write();
  delayed_Ge_C3_tot3_VS_total_Ge.Write();

  delayed_Ge_C1_VS_prompt_Ge.Write();
  delayed_Ge_C1_VS_delayed_calo.Write();


#else //QUALITY
  std::sort(runs.begin(), runs.end());
  for (auto const label : detectors.labels())
  {
    time_vs_run_each_det.at(label).Write();
    nrj_vs_run_each_det.at(label).Write();
  }
  for (auto & run : runs) time_vs_det_each_run.at(run).Write();
  for (auto & run : runs) nrj_vs_det_each_run.at(run).Write();
#endif //QUALITY

  outfile->Write();
  outfile->Close();
  print(g_out_filename, "written");
}

void reader(int number_files = -1)
{
  MTObject::Initialise((nb_threads<0) ? 10 : nb_threads);
  std::string path = "~/faster_data/";
  if (found(Path::pwd().string(), "faster")) path = "~/nuball2/";
  std::string run_name = "N-SI-136";
  if (read_129) run_name = "N-SI-129";
  if (simple_d) trigger = "d";
  else if (read_C2) trigger = "C2";
  else if (read_new) trigger = "PrM1DeC1";
  else if (read_particle) trigger = "P";
  Analysator analysator(number_files, path+run_name+"-root_"+trigger+"/merged/");
}

int main(int argc, char** argv)
{
  int nb_files = -1;

  for (int i = 1; i<argc; ++i)
  {
    std::string param(argv[i]);
         if (param == "-f") nb_files = std::stoi(argv[++i]);
    else if (param == "-n") Analysator::setMaxHits(std::stoi(argv[++i]));
    else if (param == "-p") 
    {
      if(read_C2 || read_new || simple_d) Colib::throw_error("Can't have more than one trigger"); 
      read_particle = true;
    }
    else if (param == "-d") 
    {
      if(read_C2 || read_new || read_particle) Colib::throw_error("Can't have more than one trigger"); 
      simple_d = true;
    }
    else if (param == "-m") nb_threads = std::stoi(argv[++i]);
    else if (param == "--129") read_129 = true;
    else if (param == "--C2") 
    {
      if(simple_d || read_new || read_particle) Colib::throw_error("Can't have more than one trigger"); 
      read_C2 = true;
    }
    else if (param == "--new") 
    {
      if(simple_d || read_C2) Colib::throw_error("Can't have more than one trigger"); 
      read_new = true;
    }
    else
    {
      print("Usage of reader :");
      print("-f : number of files");
      print("-n : number of hits per file");
      print("-d : single clean Ge trigger");
      print("-p : particle trigger");
      print("--C2 : 2 clean Ge in the 200ns time window");
      print("--new: the M1 prompt C1 delayed trigger");
      print("-m : number of threads");
      print("--129 : read N-SI-129 data");
      exit(42);
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
    //   if(event_i%int_cast(10.e+6) == 0) print(event_i/1.e+6, "MEvts");

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
    //       auto const & clover_index = Clovers::labels[label];
    //       if (isPrompt(time_ns))
    //       {
    //         push_back_unique(clovers_prompt_fired, clover_index);
    //         if (isGe[label])  clovers_prompt[clover_index].addGe (time, nrj);
    //         if (isBGO[label]) clovers_prompt[clover_index].addBGO(time, nrj);
    //       }
    //       else if (isDelayed(time_ns))
    //       {
    //         push_back_unique(clovers_delayed_fired, clover_index);
    //         if (isGe[label])  clovers_delayed[clover_index].addGe (time, nrj);
    //         if (isBGO[label]) clovers_delayed[clover_index].addBGO(time, nrj);
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
    //       // Colib::pause();
    //     }

    //     if (isGe[label])
    //     {
    //       if (time_ns<20) prompt_Ge.Fill(nrj);
    //       if (time_ns<20) prompt_Clover_VS_sub_ring.Fill(nrj);
    //       if (time_ns<20) prompt_Clover_VS_label.Fill(nrj);
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
    