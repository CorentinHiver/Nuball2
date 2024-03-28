// 2. Include library
#include <MTObject.hpp>       // This class allows for multithreading additions (needs to be loaded first)
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono threaded code
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want²
#include <MTTHist.hpp>        // Use this to thread safely fill histograms
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information
#include <HitBuffer.hpp>      // A buffer for hits

namespace Clovers_namespace
{
  thread_local static std::array<uchar, 1000> labels;  // Array used to make correspondance between a detector label and its clover label
  thread_local bool initialized = false;
  void initialize()
  {
    if (!initialized)
    {
      std::fill(labels.begin(), labels.end(), -1);
      for (int l = 23; l<167; l++) labels[l] = static_cast<uchar>((l-23)/6);
      initialized = true;
    }
  }
}

// 3. Declare some global variables :
// Data parameters :
double const & g_adc_threshold = 5000;
std::vector<double> g_trigger_blacklist = {70, 97};

// Event building parameters :
double const & g_rf_offset_ns = 25.0;
double const & g_begin_prompt_ns = -10;
double const & g_begin_prompt_Rings_DSSD_ns = -40;
double const & g_end_prompt_ns = 5;
double const & g_begin_prompt_PARIS_ns = -5;
double const & g_end_prompt_PARIS_ns = 3;
double const & g_begin_delayed_ns = 30.0;// The first Ge must hit after this timestamp
double const & g_end_delayed_ns = 180.0;// The first Ge must hit before this timestamp
bool const & g_prompt_trigger = false; // If we require a prompt or not
int g_n_prev_pulses = 4; // Number of pulses before the trigger to take

  // Time windows :
Time const & g_coinc_tw_ns = 50; // time window in ns (+-g_coinc_tw_ns)

// std::vector<Label> g_trigger_blacklist = {70};

// Other parameters :
std::string IDFile = "../index_129.list";
std::string calibFile = "../136_2024.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string output = "-root_dd";
int nb_files_ts = 100;
int nb_files_ts_verify = 10;
int nb_files = -1;
bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool histoed = false;
bool one_run = false;
bool single_clean = false;
std::string one_run_folder = "";
ulonglong max_hits = -1;
bool treat_129 = false;
std::string output_fileinfo_name = "";
int verbose = 2;

bool extend_periods = false; // To take more than one period after a event trigger
uint nb_periods_more = 0; // Number of periods to extend after an event that triggered

class NoDataError
{
public:
  NoDataError() noexcept = default;
  NoDataError(std::string const & message) noexcept {error(message);}
};

char trigger_choice = -1;
std::map<char, std::string> trigger_name = 
{
  {-1, "all"},
  {0,  "P"},
  {1,  "M3G1"},
  {2, "P_M3G1"},
  {3, "PM2G1"},
  {4, "P_M4G1"},
  {5, "M4G1"}
};

std::string trigger_legend = "Legend : P = particle. G = Germanium. M = Module. _ = OR.";

struct RunInfo
{
  RunInfo() noexcept = default;

  static inline void writeHeader(std::ofstream& outfile)
  {
    outfile << "run_name run_number compression_factor trigger_efficiency nb_files Mo_per_sec" << std::endl;
  }

  inline void writeLine(std::ofstream& outfile)
  {
    outfile << run_name << " " << run_number << " " << compression_factor << " " << trigger_efficiency << " " << nb_files << " " << Mo_per_sec;
  }

  std::string run_name;
  int    run_number = 0;
  double compression_factor = 0.0;
  double trigger_efficiency = 0.0;
  int    nb_files = 0;
  double Mo_per_sec = 0.0;
};

struct Histos
{
  MTTHist<TH1F> energy_all_Ge_trigg;
  MTTHist<TH1F> rf_all_raw;
  MTTHist<TH2F> energy_each_raw;
  MTTHist<TH2F> rf_each_raw;

  // Vector_MTTHist<TH2F> paris_bidim;
  // Vector_MTTHist<TH2F> paris_bidim_M_inf_4;

  MTTHist<TH1F> energy_all_Ge;
  MTTHist<TH1F> rf_all;
  MTTHist<TH2F> energy_each;
  MTTHist<TH2F> rf_each;

  MTTHist<TH1F> energy_all_Ge_event;
  MTTHist<TH1F> rf_all_event;
  MTTHist<TH2F> energy_each_event;
  MTTHist<TH2F> rf_each_event;

  MTTHist<TH1F> energy_all_Ge_trig;
  MTTHist<TH1F> rf_all_trig;
  MTTHist<TH2F> energy_each_trig;
  MTTHist<TH2F> rf_each_trig;

  MTTHist<TH1F> first_Ge_spectra;
  MTTHist<TH1F> first_Ge_spectra_Clean;
  MTTHist<TH1F> first_Ge_spectra_Vetoed;
  
  MTTHist<TH1F> all_Ge_after_trigger;
  MTTHist<TH1F> all_Ge_after_trigger_with_prompt;

  MTTHist<TH1F> second_Ge_spectra;
  MTTHist<TH1F> second_Ge_spectra_Clean;
  MTTHist<TH1F> second_Ge_spectra_Vetoed;

  MTTHist<TH2F> BGO_VS_Ge_time;
  MTTHist<TH2F> BGO_VS_Ge_time_prompt;
  MTTHist<TH2F> E_Ge_BGO_dT_Ge_VS;
  MTTHist<TH2F> BGO_VS_Ge_label;
  MTTHist<TH2F> BGO_VS_Ge_label_vetoed;
  MTTHist<TH2F> BGO_VS_Ge_label_clean;


  MTTHist<TH2F> first_Ge_time_VS_nb_delayed;
  MTTHist<TH2F> second_Ge_time_VS_nb_delayed;
  MTTHist<TH2F> Ge2_VS_Ge_time;
  MTTHist<TH2F> Ge3_VS_Ge_time;
  MTTHist<TH2F> Ge3_VS_Ge2_time;

  MTTHist<TH1F> promptGe;
  MTTHist<TH1F> promptGe_clean;
  MTTHist<TH1F> promptGe_vetoed;

  MTTHist<TH2F> time_VS_det;
  MTTHist<TH2F> time_first_prompt;
  MTTHist<TH2F> time_found_prompt;
  MTTHist<TH1F> time_back_event_window;
  MTTHist<TH2F> time_before_to_first_prompt;

  
  TH1F* rf_evolution = nullptr;

  void Initialize()
  {
    auto const & nbDet = detectors.number();

    time_VS_det.reset("time_VS_det", "time VS det", 1000,0,1000, 500,-500,500);
    time_first_prompt.reset("time_first_prompt", "time first prompt", 1000,0,1000, 1000,-1000,500);
    time_found_prompt.reset("time_found_prompt", "time found prompt", 1000,0,1000, 1000,-1000,500);
    time_back_event_window.reset("time_back_event_window", "time back event window", 1000,-1000,500);
    time_before_to_first_prompt.reset("time_before_to_first_prompt", "time before to first prompt", 1000,0,1000, 10000,-10000,500);
    
    energy_all_Ge_trigg.reset("energy_all_Ge_trigg", "Ge spectra raw", 20000,0,10000);
    rf_all_raw.reset("rf_all_raw", "RF Time spectra raw", 2000, -1200, 800);
    energy_each_raw.reset("energy_each_raw", "Energy spectra each raw", nbDet,0,nbDet, 5000,0,15000);
    rf_each_raw.reset("rf_each_raw", "RF timing each raw", nbDet,0,nbDet, 2000, -1200, 800);

    energy_all_Ge.reset("Ge_spectra", "Ge spectra", 20000,0,10000);
    rf_all.reset("RF_Time_spectra", "RF Time spectra", 2000, -1200, 800);
    energy_each.reset("Energy_spectra_each", "Energy spectra each", nbDet,0,nbDet, 5000,0,15000);
    rf_each.reset("RF_timing_each", "RF timing each", nbDet,0,nbDet, 2000, -1200, 800);

    energy_all_Ge_event.reset("Ge_spectra_event", "Ge spectra after event building", 20000,0,10000);
    rf_all_event.reset("Time_spectra_event", "Time spectra after event building", 2000, -1200, 800);
    energy_each_event.reset("Energy_spectra_each_event", "Energy spectra each after event building", nbDet,0,nbDet, 5000,0,15000);
    rf_each_event.reset("RF_timing_each_event", "RF timing each after event building", nbDet,0,nbDet, 2000,-1200,800);

    energy_all_Ge_trig.reset("Ge_spectra_trig", "Ge spectra after trigger", 20000,0,10000);
    rf_all_trig.reset("Time_spectra_trig", "Time spectra after trigger", 2400, -1200, 1200);
    energy_each_trig.reset("Energy_spectra_each_trig", "Energy spectra each after trigger", nbDet,0,nbDet, 5000,0,15000);
    rf_each_trig.reset("RF_timing_each_trig", "RF timing each after trigger", nbDet,0,nbDet, 2000,-1200,800);
    

    // New trigger debug
    BGO_VS_Ge_time.reset("BGO_VS_Ge_time", "BGO VS Ge time;Ge [ns];BGO [ns]", 500,-100,300, 500,-100,300);
    BGO_VS_Ge_time_prompt.reset("BGO_VS_Ge_time_prompt", "BGO VS Ge time;Ge [ns];BGO [ns]", 500,-100,300, 500,-100,300);
    E_Ge_BGO_dT_Ge_VS.reset("E_Ge_BGO_dT_Ge_VS", "Ge energy VS BGO-Ge time;Ge [ns];BGO [ns]", 1000,-300,300, 5000,0,5000);
    BGO_VS_Ge_label.reset("BGO_VS_Ge_label", "BGO VS Ge labels;Ge;BGO", 200,0,200, 200,0,200);
    BGO_VS_Ge_label_vetoed.reset("BGO_VS_Ge_label_vetoed", "BGO VS Ge labels veto;Ge;BGO", 200,0,200, 200,0,200);
    BGO_VS_Ge_label_clean.reset("BGO_VS_Ge_label_clean", "BGO VS Ge labels clean;Ge;BGO", 200,0,200, 200,0,200);
    

    first_Ge_time_VS_nb_delayed.reset("first_Ge_time_VS_nb_delayed", "1st Ge time", 10,0,10, 1000,0,300);
    second_Ge_time_VS_nb_delayed.reset("second_Ge_time_VS_nb_delayed", "2nd Ge time", 10,0,10, 1000,0,300);
    Ge2_VS_Ge_time.reset("Ge2_VS_Ge_time", "Ge2 VS Ge time", 500,-100,300, 500,-100,300);
    Ge3_VS_Ge_time.reset("Ge3_VS_Ge_time", "Ge3 VS Ge time", 500,-100,300, 500,-100,300);
    Ge3_VS_Ge2_time.reset("Ge3_VS_Ge2_time", "Ge3 VS Ge2 time", 500,-100,300, 500,-100,300);

    promptGe.reset("promptGe", "prompt Ge spectra", 10000, 0, 10000);
    promptGe_clean.reset("promptGe_clean", "prompt Ge spectra clean", 10000, 0, 10000);
    promptGe_vetoed.reset("promptGe_vetoed", "prompt Ge spectra delayed", 10000, 0, 10000);

    first_Ge_spectra.reset("first_Ge_spectra", "1st Ge spectra", 10000, 0, 10000);
    first_Ge_spectra_Clean.reset("first_Ge_spectra_Clean", "1st Ge spectra clean", 10000, 0, 10000);
    first_Ge_spectra_Vetoed.reset("first_Ge_spectra_Vetoed", "1st Ge spectra vetoed", 10000, 0, 10000);

    all_Ge_after_trigger.reset("all_Ge_after_trigger", "all_Ge_after_trigger", 20000,0,10000);
    all_Ge_after_trigger_with_prompt.reset("all_Ge_after_trigger_with_prompt", "all_Ge_after_trigger_with_prompt", 20000,0,10000);

    second_Ge_spectra.reset("second_Ge_spectra", "2nd Ge spectra", 10000, 0, 10000);
    second_Ge_spectra_Clean.reset("second_Ge_spectra_Clean", "2nd Ge spectra clean", 10000, 0, 10000);
    second_Ge_spectra_Vetoed.reset("second_Ge_spectra_Vetoed", "2nd Ge spectra vetoed", 10000, 0, 10000);
    
    rf_evolution = new TH1F("rf_evolution", "Evolution RF;# epoch;[ps]", 1000000,0,1000000);
    rf_evolution->SetDirectory(nullptr);
  }

  void Write(std::string const & name)
  {
    std::unique_ptr<TFile> outFile (TFile::Open(name.c_str(), "RECREATE"));
    outFile -> cd();

    time_VS_det.Write();
    time_first_prompt.Write();
    time_found_prompt.Write();
    time_back_event_window.Write();
    time_before_to_first_prompt.Write();

    energy_all_Ge_trigg.Write();
    rf_all_raw.Write();
    energy_each_raw.Write();
    rf_each_raw.Write();
    energy_all_Ge.Write();
    energy_each.Write();
    rf_all.Write();
    rf_each.Write();
    energy_all_Ge_event.Write();
    rf_all_event.Write();
    energy_each_event.Write();
    rf_each_event.Write();
    energy_all_Ge_trig.Write();
    energy_each_trig.Write();
    rf_all_trig.Write();
    rf_each_trig.Write();
    // for (auto & histo : paris_bidim) histo.Write();
    // for (auto & histo : paris_bidim_M_inf_4) histo.Write();

    promptGe.Write();
    promptGe_clean.Write();
    promptGe_vetoed.Write();

    BGO_VS_Ge_time.Write();
    BGO_VS_Ge_time_prompt.Write();
    E_Ge_BGO_dT_Ge_VS.Write();
    BGO_VS_Ge_label.Write();
    BGO_VS_Ge_label_vetoed.Write();
    BGO_VS_Ge_label_clean.Write();

    first_Ge_spectra.Write();
    first_Ge_spectra_Clean.Write();
    first_Ge_spectra_Vetoed.Write();

    second_Ge_spectra.Write();
    second_Ge_spectra_Clean.Write();
    second_Ge_spectra_Vetoed.Write();

    first_Ge_time_VS_nb_delayed.Write();
    second_Ge_time_VS_nb_delayed.Write();

    all_Ge_after_trigger.Write();

    Ge2_VS_Ge_time.Write();
    Ge3_VS_Ge_time.Write();
    Ge3_VS_Ge2_time.Write();

    rf_evolution->Write();
    delete rf_evolution;

    outFile -> Write();
    outFile -> Close();
    print(name, "written");
  }
};

inline bool handleRf(RF_Manager & rf, Hit const & hit, Event & event, TTree * tree)
{
  if (rf.setHit(hit))
  {
    Event temp (event);
    event = hit;
    tree -> Fill();
    event = temp;
    return true;
  }
  else return false;
}

double dT_ns(Timestamp const & start, Timestamp const & stop) 
{
  auto const & dT_Time = Time_cast(stop - start);
  return(double_cast(dT_Time)/1000.0);
}

bool isPrompt_ns(double const & rel_time_ns, int const & label) 
{
  if (isParis[label]) return (rel_time_ns > g_begin_prompt_PARIS_ns && rel_time_ns < g_end_prompt_PARIS_ns);
  else                return (rel_time_ns > g_begin_prompt_ns       && rel_time_ns < g_end_prompt_ns      );
}
bool isDelayed_ns(double const & rel_time_ns) {return (rel_time_ns>g_begin_delayed_ns && rel_time_ns<g_end_delayed_ns);}

/// @brief 
/// @return: true when clean, false when vetoed 
bool comptonClean(HitBuffer const & buffer, int & it, RF_Manager & rf, Histos & histos, bool const & histoed, bool const & delayed = true)
{
  auto const init_it = it;
  auto const & hit_Ge = buffer[init_it];
  auto const & clover_Ge = Clovers_namespace::labels[hit_Ge.label];
  auto const & time_Ge = rf.pulse_ToF_ns(hit_Ge);

  // --- a. Look backward to look for potential BGO within the time window before the hit --- //
  // Start with the first hit before the germanium we want to check :
  if (it>0) while(--it>buffer.step())
  {// Loop back until we either go out of the coincident window, 
   // step inside the prompt window or finds the last hit of the last written event
    auto const & hit_it = buffer[it];
    if (!isBGO[hit_it.label]) continue;

    auto const & time_diff = dT_ns(hit_it.stamp, hit_Ge.stamp); // dT_ns(start, stop)
    if(time_diff>g_coinc_tw_ns) break;

    // Checking prompt window :  We don't look inside the prompt peak for Compton suppression
    auto const & rf_time = time_Ge-time_diff;
    // if (delayed && rf_time<g_begin_delayed_ns) break;

    if (histoed && delayed) histos.BGO_VS_Ge_label.Fill(hit_Ge.label, hit_it.label);

    // Looking for BGO of the same clover module as the germanium
    if (clover_Ge==Clovers_namespace::labels[hit_it.label]) 
    {
      if (histoed) 
      {
        if (delayed) 
        {
          histos.BGO_VS_Ge_time.Fill(time_Ge, rf_time);
          histos.BGO_VS_Ge_label_vetoed.Fill(hit_Ge.label, hit_it.label);
        }
        else histos.BGO_VS_Ge_time_prompt.Fill(time_Ge, rf_time);;
      }
      it = init_it;
      return false;
    }
  }

  // --- b. Look forward for potential BGO within the time window after the hit --- //
  for (it = init_it+1; it<int_cast(buffer.size()); it++)
  {
    auto const & hit_it = buffer[it];
    if (!isBGO[hit_it.label]) continue;

    auto const & time_diff = dT_ns(hit_Ge.stamp, hit_it.stamp); // dT_ns(start, stop)
    if(time_diff>g_coinc_tw_ns) break; // No coincident forward BGO

    if (histoed && delayed) histos.BGO_VS_Ge_label.Fill(hit_Ge.label, hit_it.label);

    if (clover_Ge==Clovers_namespace::labels[hit_it.label])
    {// If the hit is a BGO and is in the same Clover as the Germanium, it most probably means it is a scattering from one to another
      if (histoed)
      {
        auto const & rf_time = time_Ge+time_diff;
        if (delayed)
        {
          histos.BGO_VS_Ge_time.Fill(time_Ge, rf_time);
          histos.BGO_VS_Ge_label_vetoed.Fill(hit_Ge.label, hit_it.label);
          histos.E_Ge_BGO_dT_Ge_VS.Fill(time_diff, hit_Ge.nrj);
        }
        else histos.BGO_VS_Ge_time_prompt.Fill(time_Ge, rf_time);
      }
      it = init_it;
      return false;
    }
  }

  // if (histoed) histos.BGO_VS_Ge_label_clean.Fill(hit_Ge.label, hit_it.label);
  it = init_it;
  return true;
}

std::mutex temp_tree_mutex;
std::mutex output_mutex;

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, 
              Calibration const & calibration, 
              Timeshifts const & timeshifts, 
              Path const & outPath, 
              Histos & histos,
              MTCounter & total_read_size,
              RunInfo & run_infos)
{
  // ------------------ //
  // Initialize helpers //
  // ------------------ //
  Clovers_namespace::initialize();
  Timer timer;
  // Checking the lookup tables :
  if (!timeshifts || !calibration || !reader) return;

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  Filename filename = raw_datafile.filename();// "                               run_number_filenumber.fast"
  int filenumber = std::stoi(lastPart(filename.shortName(), '_'));//                        filenumber
  std::string run = removeLastPart(filename.shortName(), '_');    //             run_number
  int run_number = std::stoi(lastPart(run, '_')); //                                            number

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                     // /outPath/run_number.fast/
  Filename outFilename(raw_datafile.shortName()+".root"); //                          run_number_filenumber.root
  File outfile (outFolder, outFilename);                  // /outPath/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  total_read_size+=raw_datafile.size();
  auto dataSize = float_cast(raw_datafile.size("Mo"));


  // ------------------------------ //
  // Initialize the temporary TTree //
  // ------------------------------ //
temp_tree_mutex.lock();
  std::string const & tempTree_name = "temp"+std::to_string(MTObject::getThreadIndex());
  TTree* tempTree (new TTree(tempTree_name.c_str(),tempTree_name.c_str()));
  tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  hit.writting(tempTree, "lsEQ"); // The pileup bit has been removed because of weird errors raised by valgrind drd
  // hit.writting(tempTree, "lsEQp");
temp_tree_mutex.unlock();

  // ------------------------ //
  // Loop over the .fast file //
  // ------------------------ //
  Timer read_timer;
  while(reader.Read())
  {
    // Time alignement :
    hit.stamp+=timeshifts[hit.label];

    // Energy calibration : 
    hit.nrj = calibration(hit.adc,  hit.label); // Normal calibration
    hit.nrj2 = ((hit.qdc2 == 0) ? 0.f : calibration(hit.qdc2, hit.label)); // Calibrate the qdc2 if any
  
    tempTree -> Fill();
  }
  auto rawCounts = tempTree->GetEntries();

  read_timer.Stop();
  
  if (verbose>1) print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

  if (rawCounts==0) {print("NO HITS IN",run); return;}

  // -------------------------------------- //
  // Realign switched hits after timeshifts //
  // -------------------------------------- //
  Alignator gindex(tempTree);

  // ------------------------------------------------------ //
  // Prepare the reading of the TTree and the output Events //
  // ------------------------------------------------------ //
  // Switch the temporary TTree to reading mode :
temp_tree_mutex.lock();
  hit.reset();
  hit.reading(tempTree, "lsEQ"); // The pileup bit has been removed because of weird errors raised by valgrind drd
  // hit.reading(tempTree, "lsEQp");
temp_tree_mutex.unlock();

  // Create output tree and Event 
output_mutex.lock();
  std::string const outTree_name = "Nuball2"+std::to_string(MTObject::getThreadIndex());
  TTree* outTree (new TTree(outTree_name.c_str(),"Nuball2"));
  outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  Event event;
  event.writting(outTree, "lstEQ");// The pileup bit has been removed because of weird errors raised by valgrind drd
output_mutex.unlock();
  // event.writting(outTree, "lstEQp");

  // Initialize RF manager :
  RF_Manager rf;
  if (treat_129) rf.set_period_ns(400);
  else           rf.set_period_ns(200);

  // Handle the first RF downscaled hit :
  RF_Extractor first_rf(tempTree, rf, hit, gindex);
  if (!first_rf) return;

  // debug("first RF found at hit n°", first_rf.cursor());

  // Handle the first hit of the file:
  int loop = 0;

  // In the first file of each run, the first few first hundreds of thousands of hits
  // don't have RF downscale. So we have to skip them in order to maintain good temporal resolution :
  if (filenumber == 1) loop = first_rf.cursor(); 
  tempTree -> GetEntry(gindex[loop++]);

  // --------------------------------- //
  // Loop over the temporary root tree //
  // --------------------------------- //
  
  // Helpers :
  Timer convert_timer;
  auto const & nb_data = tempTree->GetEntries();
  // ulong evts_count = 0;
  ulong trig_count = 0;
  HitBuffer buffer(50000);
  size_t hits_count = 0;
  int count_rf = 0;
  while (loop<nb_data)
  {
    // if (isGe[hit.label] && hit.nrj<5) continue;
    tempTree -> GetEntry(gindex[loop++]);
    if (histoed) histos.rf_each.Fill(hit.label, rf.pulse_ToF_ns(hit.stamp));
    buffer.push_back(hit);
    if (buffer.isFull())
    {
      for (int r_buffer_it = 0; r_buffer_it < int_cast(buffer.size()); ++r_buffer_it)
      {
        auto const & hit_r = buffer[r_buffer_it];
        if (handleRf(rf, hit_r, event, outTree)) 
        {
          if (histoed)
          {
            histos.rf_evolution->SetBinContent(count_rf, rf.period);
            ++count_rf;
          }
          continue;
        }
        
        Time beam_period = rf.period;
        if (treat_129) beam_period/=2;

        // ||||||||||||||||||||||||||||| //
        // ||| FIRST PART : TRIGGER  ||| //
        // ||||||||||||||||||||||||||||| //

        if(isGe[hit_r.label])
        {// Trigger on Germaniums
          auto const & hit_first_Ge = hit_r; // The Germanium hit
          auto const & rel_time_first_Ge_ns = rf.pulse_ToF_ns(hit_first_Ge); // The time of the Germanium relative to the beam pulsation

          // ---------------------------------//
          //1. ---  Treat only delayed Ge --- //
          // ---------------------------------//
          if (isDelayed_ns(rel_time_first_Ge_ns))
          {// If the Germanium is in the delayed window, try to create the event :
            auto const init_it = r_buffer_it; // The buffer's index of the Germanium hit that might open the event
          MTObject::mutex.lock();
            auto first_Ge_Clover_label = Clovers_namespace::labels[hit_first_Ge.label]; // The Clover label of the first Germanium
          MTObject::mutex.unlock();
            auto const & ref_pulse_timestamp = rf.refTime(hit_first_Ge.stamp); // The absolute timestamp of the beam pulse (relative to the delayed Germanium we're trying to trigger on)
            std::vector<uchar> clover_modules = {first_Ge_Clover_label}; // List of the modules that fired in the event (initialise it with the clover that fired)

            // -----------------------------//
            //2. --- BGO clean first Ge --- //
            // -----------------------------//
            if (histoed) histos.first_Ge_spectra.Fill(hit_first_Ge.nrj);
            
            if (!comptonClean(buffer, r_buffer_it, rf, histos, histoed)) 
            {
              if (histoed) histos.first_Ge_spectra_Vetoed.Fill(hit_first_Ge.nrj);
              continue;
            }

            r_buffer_it = init_it;

            if (histoed) histos.first_Ge_spectra_Clean.Fill(hit_first_Ge.nrj);

            // ------------------------------//
            //3. --- Find next detectors --- //
            // ------------------------------//
            

            Timestamp front_coinc_window = hit_first_Ge.stamp + Time_cast(g_coinc_tw_ns*1000.); // The timestamp of the front delayed time window
            Timestamp back_coinc_window  = hit_first_Ge.stamp - Time_cast(g_coinc_tw_ns*1000.); // The timestamp of the back  delayed time window
            
            // If the front window is outside of the end of the delayed time window, shift it to it.
            if (front_coinc_window > ref_pulse_timestamp+beam_period+g_begin_prompt_ns) front_coinc_window = ref_pulse_timestamp+g_end_delayed_ns; 

            // ------------------------------------------------//
            //3.A --- Any other coincident gamma detector  --- //
            // ------------------------------------------------//
            if (single_clean)
            {
              bool single_clean_coinc = false; // Single clean Germanium option : require only another detector in coincidence with a clean Germanium
              // Looking at last hit wether it is in the time window :
              if (r_buffer_it-1 > buffer.step())
              {
                auto const & hit_before = buffer[r_buffer_it-1];
                if (isDSSD[hit_before.label]) continue; // The DSSD does not count as a trigger. Only the gamma detectors do.
                if (!isDelayed_ns(rf.pulse_ToF_ns(hit_before.stamp))) continue;
                if (back_coinc_window < hit_before.stamp) single_clean_coinc = true;
              }

              // Looking at next hit wether it is in the time window :
              if (!single_clean_coinc && r_buffer_it+1 < int_cast(buffer.size()))
              {
                auto const & hit_after = buffer[r_buffer_it+1];
                if (isDSSD[hit_after.label]) continue; // The DSSD does not count as a trigger. Only the gamma detectors do.
                if (!isDelayed_ns(rf.pulse_ToF_ns(hit_after.stamp))) continue;
                if (hit_after.stamp < front_coinc_window) single_clean_coinc = true;
              }

              if (!single_clean_coinc) continue;
            }

            // ----------------------------//
            //3.B --- Another clean Ge --- //
            // ----------------------------//
            else // double clean Ge delayed
            {
              std::vector<int> next_Ge_indexes; // List the indices of the next Germaniums potentially in the event
              while (++r_buffer_it < int_cast(buffer.size()))
              {// Normal double delayed mode :
                auto const & hit_it = buffer[r_buffer_it];

                //1. Checking the time coincidence
                if(hit_it.stamp>front_coinc_window) break;

                auto const & time_diff_ns = dT_ns(hit_first_Ge.stamp, hit_it.stamp);// dT_ns(start, stop)

                //2. Remove hits of the prompt of next pulse
                if (rel_time_first_Ge_ns + time_diff_ns > g_end_delayed_ns) break; 

                if (isGe[hit_it.label])
                {
                  // 3. Add-Back : Checking if the label of the germanium's clover module have already fired
                  auto const & clover_module_it = Clovers_namespace::labels[hit_it.label];
                  if (found(clover_modules, clover_module_it)) continue;
                  clover_modules.push_back(clover_module_it);
                  
                  // 4. Register the hit
                  next_Ge_indexes.push_back(r_buffer_it);
                }
              } 

              r_buffer_it = init_it;

              if (histoed) histos.first_Ge_time_VS_nb_delayed.Fill(next_Ge_indexes.size(), rel_time_first_Ge_ns);

              // Trigger : there needs to be at least another clover that fired
              if (next_Ge_indexes.size() < 1) continue;

              // debug();
              // debug(hit_first_Ge);
              // debug(buffer[next_Ge_indexes.front()]);

              if (histoed)
              {
                auto const & time_Ge2 = rel_time_first_Ge_ns + dT_ns(hit_first_Ge.stamp, buffer[next_Ge_indexes[0]].stamp);
                histos.second_Ge_time_VS_nb_delayed.Fill(next_Ge_indexes.size(), time_Ge2);
                histos.Ge2_VS_Ge_time.Fill(rel_time_first_Ge_ns, time_Ge2);
                
                if (next_Ge_indexes.size() > 2)
                {
                  auto const & time_Ge3 = rel_time_first_Ge_ns + dT_ns(hit_first_Ge.stamp, buffer[next_Ge_indexes[1]].stamp);
                  histos.Ge3_VS_Ge_time.Fill(rel_time_first_Ge_ns, time_Ge3);
                  histos.Ge3_VS_Ge2_time.Fill(time_Ge2, time_Ge3);
                }
              }

              // ------------------------------------------//
              //4. --- BGO clean and Add-back next Ges --- //
              // ------------------------------------------//

              bool trigger_Ge_clean = false;
              // Loop through the other Ges in time window
              for(size_t index_it = 0; index_it<next_Ge_indexes.size(); index_it++)
              {
                auto const & next_Ge_index = next_Ge_indexes[index_it];
                r_buffer_it = next_Ge_index;
                auto const & hit_Ge2 = buffer[r_buffer_it];

                if (histoed && index_it == 0) histos.second_Ge_spectra.Fill(buffer[next_Ge_indexes[0]].nrj);

                // Compton cleaning
                // If the second Germanium is Clean (i.e. no BGO fired in the coincidence time window), then the trigger is complete
                if (comptonClean(buffer, r_buffer_it, rf, histos, histoed))
                {
                  if (histoed && index_it == 0) histos.second_Ge_spectra_Clean.Fill(buffer[next_Ge_index].nrj);
                  trigger_Ge_clean = true;
                  break;
                }

                // If a BGO vetoed this Ge, move to the next one to check wether he's clean or not
                else
                {
                  if (histoed && index_it == 0) histos.second_Ge_spectra_Vetoed.Fill(buffer[next_Ge_index].nrj);
                  continue;
                }
              }

              r_buffer_it = init_it;

              if (!trigger_Ge_clean) continue;

              if (histoed) 
              {
                histos.all_Ge_after_trigger.Fill(hit_first_Ge.nrj);
                for (auto const & _it_ : next_Ge_indexes) histos.all_Ge_after_trigger.Fill(buffer[_it_].nrj);
              }
            }
            
            // ------------------------------------------------------------------------------//
            //5. --- Requiring at least one prompt hit within few RF cycles in the past  --- //
            // ------------------------------------------------------------------------------//
            // (This prompt event is likely to be a prompt decay feeding isomeric states)
            Timestamp const last_prompt_time_window_ps = ref_pulse_timestamp - Time_cast(g_n_prev_pulses*beam_period-rf.offset());
            bool one_prompt_before = false;
            Timestamp last_prompt_ref_pulse_timestamp = 0;
            size_t last_prompt_pulse_index = 0;
            while(--r_buffer_it > buffer.step())
            {
              auto const & hit_it = buffer[r_buffer_it];
              if (isDSSD[hit_it.label]) continue; // The DSSD does not count as a trigger. Only the gamma detectors do.
              if (hit_it.stamp < last_prompt_time_window_ps) break;
              if (isPrompt_ns(rf.pulse_ToF_ns(hit_it.stamp), hit_it.label)) 
              {
                last_prompt_ref_pulse_timestamp = rf.refTime(hit_it.stamp);
                last_prompt_pulse_index = r_buffer_it;
                one_prompt_before = true;
                break;
              }
            }
            
            r_buffer_it = init_it;

            if (!one_prompt_before) continue;

            r_buffer_it = last_prompt_pulse_index;
            if (histoed) histos.time_first_prompt.Fill(buffer[last_prompt_pulse_index].label, dT_ns(ref_pulse_timestamp, buffer[last_prompt_pulse_index].stamp));

          // ||||||||||||||||||||||||||||||||||||||||||||||||||| //
          // ||| SECOND PART : CREATE THE EVENT AND WRITE IT ||| //
          // ||||||||||||||||||||||||||||||||||||||||||||||||||| //

            // Extract some useful informations :
            Timestamp back_event_window = last_prompt_ref_pulse_timestamp + Time_cast(g_begin_prompt_ns*1000); // Beginning of the last prompt pulse (last_prompt_timestamp) - abs(g_begin_prompt_ns)
            // Timestamp back_event_window = ref_pulse_timestamp - g_n_prev_pulses*beam_period - rf.offset(); // Beginning of the

            // ----------------------------------------------//
            //6. --- Locate the first hit in time window --- //
            // ----------------------------------------------//
            if (r_buffer_it > buffer.step()) while (--r_buffer_it > 0)
              if(buffer[r_buffer_it].stamp < back_event_window) break; // Here r_buffer_it points to the hit before the first hit of the event
            
            if (histoed)
            {
              histos.time_before_to_first_prompt.Fill(buffer[r_buffer_it].label, dT_ns(ref_pulse_timestamp, buffer[r_buffer_it].stamp));
              histos.time_found_prompt.Fill(buffer[r_buffer_it+1].label, dT_ns(ref_pulse_timestamp, buffer[r_buffer_it+1].stamp));
              histos.time_back_event_window.Fill(dT_ns(back_event_window, ref_pulse_timestamp));
            }

            // -------------------------//
            //7. --- Fill the Event --- //
            // -------------------------//
            while (++r_buffer_it<int_cast(buffer.size()))
            {
              auto const & hit_i = buffer[r_buffer_it];
              // debug(hit_i, Long64_cast(front_coinc_window-hit_i.stamp));
              if (hit_i.stamp > front_coinc_window) break;
              if (handleRf(rf, hit_i, event, outTree)) continue;
              auto const & time_ns = rf.pulse_ToF_ns(hit_i.stamp);

              // Removes non-prompt hits between previous pulses :
              // auto const & time_j = rf.pulse_ToF_ns(hit_i);
              // if (hit_i.stamp<isPrompt_ns(time_j)) event.push_back(hit_i);

              // Always write all the DSSD hits :
              if (isDSSD[hit_i.label]) 
              {
                event.push_back(hit_i);
              }

              // Handles the delayed hits : 
              else if (isDelayed_ns(time_ns))
              {
                // Removes delayed hits not in coincidence with the opening Germanium :
                if (hit_i.stamp < back_coinc_window) continue;
                if (front_coinc_window < hit_i.stamp) break; // After the end of the coincidence window, the hits are of no interest
                event.push_back(hit_i);
              }
              else if (isPrompt_ns(time_ns, hit_i.label))
              {
                event.push_back(hit_i);
              }
            }
            
            // event.setT0(ref_pulse_timestamp); // The t0 is set to be the closest prompt pulse
            event.setT0(last_prompt_ref_pulse_timestamp);// The t0 is set to be the closest prompt pulse with gamma hits

            // debug(event);
            // pauseDebug();

            // --------------------------//
            //8. --- Write the Event --- //
            // --------------------------//
            if (histoed) for (int hit_i = 0; hit_i<event.mult; hit_i++) 
            {
              // debug(event.labels[hit_i], (event.times[hit_i])/1000.);
              // pauseDebug();
              histos.time_VS_det.Fill(event.labels[hit_i], (event.times[hit_i])/1000.);
              if (isGe[event.labels[hit_i]]) histos.energy_all_Ge_trigg.Fill(event.nrjs[hit_i]);
              // histos.time_VS_det.Fill(event.labels[hit_i], event.times[hit_i]/1000.);
            }

            outTree -> Fill();
            buffer.setStep(r_buffer_it);


            // --------------//
            //9. --- END --- //
            // --------------//
            hits_count+=event.size();
            event.clear();
            r_buffer_it = init_it;
          }
          else if (histoed && isPrompt_ns(rel_time_first_Ge_ns, hit_r.label))
          {
            histos.promptGe.Fill(hit_r.nrj);
            if (comptonClean(buffer, r_buffer_it, rf, histos, histoed, false)) histos.promptGe_clean.Fill(hit_r.nrj);
            else histos.promptGe_vetoed.Fill(hit_r.nrj);
          } 
        }
      }
      buffer.clear();
    }
  }

temp_tree_mutex.lock();
  delete tempTree;
temp_tree_mutex.unlock();

  convert_timer.Stop();
  if (verbose>1) print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s (",dataSize/convert_timer.TimeElapsedSec() ,"Mo/s)");
  Timer write_timer;

  // Create output TTree :
output_mutex.lock();
  TFile* outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> SetDirectory(outFile);
  outTree -> Write("Nuball2", TObject::kOverwrite);
  outFile -> Close();
output_mutex.unlock();

  write_timer.Stop();

  auto outSize = float_cast(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

  auto const & compression_factor = dataSize/outSize;
  auto const & trigger_efficiency = double_cast(hits_count)/double_cast(rawCounts);
  auto const & Mo_per_sec = dataSize/timer.TimeSec();

  if (verbose) 
  {
    print_precision(4);
    print(outfile, " written in ", timer()," (", Mo_per_sec, "Mo/s). Input file ", dataSize, " Mo and output file " 
    , outSize, " Mo : compression factor ", compression_factor, " - ", 100.*trigger_efficiency , " % hits kept");
  }

output_mutex.lock();
  delete outFile;
output_mutex.unlock();

  { // Zone locked by a mutex, meaning this piece of code is thread safe...
    lock_mutex lock(MTObject::mutex);
    run_infos.compression_factor += compression_factor/run_infos.nb_files;
    run_infos.trigger_efficiency += trigger_efficiency/run_infos.nb_files;
    run_infos.Mo_per_sec += Mo_per_sec/run_infos.nb_files;
  }

}

// 5. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  if (argc > 1)
  {
    std::string list_trigger;
    for (auto const & trig : trigger_name) 
    {
      list_trigger.append(std::to_string(trig.first)+std::string(" : ")+trig.second+std::string(". "));
    }
    for(int i = 1; i < argc; i++)
    {
      try
      {
        std::string command = argv[i];
             if (command == "-f" || command == "--files-number")
        {
          nb_files = atoi(argv[++i]);
        }
        else if (                   command == "--run")
        {
          one_run = true;
          one_run_folder = argv[++i];
        }
        else if (command == "-H" || command == "--histograms")
        {
          histoed = true;
        }
        else if (command == "-d" || command == "--single-clean")
        {
          single_clean = true;
          replace(output, "_dd", "_d");
        }
        else if (command == "-m" || command == "--multithread")
        {// Multithreading : number of threads
          nb_threads = atoi(argv[++i]);
        }
        else if (command == "-n" || command == "--number-hits")
        {// Number of hits per file
          FasterReader::setMaxHits(std::atoi(argv[++i]));
        }
        else if (command == "-o" || command == "--overwrite")
        {// Overwright already existing .root files
          overwrite = true;
        }
        else if (command == "-O" || command == "--output")
        {
          output = argv[++i];
        }
        else if (command == "-p" || command == "--number_pulses")
        {
          g_n_prev_pulses = std::atoi(argv[++i]);
        }
        else if (command == "-t" || command == "--trigger")
        {
          trigger_choice = std::stoi(argv[++i]);
        }
        else if (                   command == "--only-timeshift")
        {
          only_timeshifts = true;
        }
        else if (command == "-Th" || command == "--Thorium")
        {
          list_runs = "Thorium.list";
          output+="_Th";
        }
        else if (command == "-U" || command == "--Uranium")
        {
          list_runs = "Uranium.list";
        }
        else if (                   command == "--progress")
        {
          verbose = 0;
          MTFasterReader::showProgressBar();
        }
        else if (command == "-v" || command == "--verbose")
        {
          verbose = std::stoi(argv[++i]);
        }
        else if (                   command == "--129")
        {
          treat_129 = true;
          list_runs = "129.list";
          calibFile = "129.calib";
          manip = "N-SI-129";
        }
        else if (command == "-h" || command == "--help")
        {
          print("List of the commands :");
          print("(-d  || --single-clean)                   : trigger : only requires one clean Ge and one other module");
          print("(-f  || --files-number)   [files-number]  : set the number of files");
          print("(       --run)            [runname]       : set only one folder to convert");
          print("(-h  || --help)                           : display this help");
          print("(-H  || --histograms)                     : Fills and writes raw histograms");
          print("(-m  || --multithread)    [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
          print("(-n  || --number-hits)    [hits_number]   : set the number of hit to read in each file.");
          print("(-o  || --overwrite)                      : overwrites the already written folders. If a folder is incomplete, you need to delete it");
          print("(-p  || --number_pulses)  [nb_pulses]     : Sets the number of pulses to look backward from the trigger. Default : 4 pulses");
          print("(-P  || --particle-trig)                  : trigger : requires one DSSD hit before the clean Ge (TODO)");
          print("(       --run)                            : Treats only the given run (don't forget the .fast at the end)");
          print("(       --only-timeshift)                 : Calculate only timeshifts, force it even if it already has been calculated");
          // print("(-t  || --trigger)        [trigger]       : Default ",list_trigger,"|", trigger_legend);
          print("(-Th || --Thorium)                        : Treats only the thorium runs (run_nb < 75)");
          print("(-U  || --Uranium)                        : Treats only the uranium runs (run_nb >= 75)");
          print("(       --129)                            : Treats the N-SI-129 pulsed runs");
          return 0;
        }
      }
      catch(std::invalid_argument const & error) 
      {
        throw_error("Can't interpret" + std::string(argv[i]) + "as an integer");
      }
    }
  }

  // MANDATORY : initialize the multithreading according to the number required, the number of files and the hardware limitations :
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(nb_files);
  MTObject::Initialise();

  // Setup the path accordingly to the machine - to be changed ...
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp ^^^^^^^^"); return 0;}

  // Input file :
  Path manipPath = datapath+manip;

  // Output file :
  // if (extend_periods) output+="_"+std::to_string(nb_periods_more)+"periods";
  Path outPath (datapath+(manip.name()+output), true);

  print("Reading :", manipPath.string());
  print("Writting in : ", outPath.string());

  output_fileinfo_name = "info/" + outPath.folder().name() + "_" +time_string_inverse() + ".info";

  // Load some modules :
  detectors.load(IDFile);
  Calibration calibration;
  if (!only_timeshifts) calibration.load(calibFile);
  Manip runs(list_runs);
  if (one_run) runs.setFolder(one_run_folder);

  // Checking that all the modules have been loaded correctly :
  if (!runs) return 0;

  // Setup some parameters :
  RF_Manager::set_offset_ns(g_rf_offset_ns);

  // Create the info file :
  std::ofstream output_fileinfo(output_fileinfo_name, std::ios::out);
  RunInfo::writeHeader(output_fileinfo);
  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    MTCounter total_read_size;
    Timer timer;
    auto const & run_path = manipPath+run;
    auto const & run_name = removeExtension(run);
    auto const & run_number = std::stoi(lastPart(run_name, '_'));

    RunInfo run_infos;
    run_infos.nb_files = Path(run_path).nbFiles();
    run_infos.run_name = run_name;
    run_infos.run_number = run_number;
    if (nb_files<0) run_infos.nb_files = nb_files;

    Histos histos;
    if (histoed) histos.Initialize();

    print("----------------");
    print("Treating ", run_name);

    // Creating a lambda that calculates the timeshifts directly from the data :
    auto calculateTimeshifts = [&](Timeshifts & timeshifts){
      if (treat_129)// for N-SI-129 :
      {
        for (int i = 800; i<856; i++) timeshifts.dT_with_RF(i); // Using RF to align the DSSD
        for (int i = 800; i<856; i++) timeshifts.dT_with_raising_edge(i); // Use the raising edge of the dT spectra because of the timewalk of DSSD in this experiment
        timeshifts.periodRF_ns(400);
      }
      else // for N-SI-136 :
      {
        timeshifts.dT_with_biggest_peak_finder(); // Best peak finder for N-SI-136
        timeshifts.periodRF_ns(200);
      }

      timeshifts.setMult(2, 4);// Only events with 2 to 4 hits are included

      timeshifts.setOutDir(outPath.string());
      timeshifts.calculate(run_path, nb_files_ts);
      timeshifts.verify(run_path, nb_files_ts_verify);

      timeshifts.write(run_name);
    };

    // Timeshifts loading : 
    Timeshifts timeshifts;
    File file(outPath.string() + "Timeshifts/" + run_name + ".dT");

    if (histoed) MTObject::setExitFunction([&](){histos.Write("backup.root");});

    if(overwrite && only_timeshifts) {calculateTimeshifts(timeshifts);}
    else
    {
      try
      {
        timeshifts.load(file.string());
      }
      catch(Timeshifts::NotFoundError & error)
      { // If no timeshifts data if available for the run already, calculate it :
        calculateTimeshifts(timeshifts);
      }
      if (only_timeshifts) continue;
    }
    
    // Loop over the files of the run in parallel :
    MTFasterReader readerMT(run_path, nb_files);
    readerMT.readRaw(convert, calibration, timeshifts, outPath, histos, total_read_size, run_infos);

    if (histoed)
    {
      histos.Write(outPath+run_name+"/histo_"+run_name+".root");
    }

    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer.TimeSec());

    Path outDataPath(outPath.string() + "/" + run_name);
    Path outMergedPath (outPath.string()+"merged", true);
    File outMergedFile (outMergedPath, run_name+".root");
    
    run_infos.writeLine(output_fileinfo);
    
    if (outMergedFile.exists())
    {
      if (overwrite) system(("rm " + outMergedFile.string()).c_str());
      else continue;
    }

    auto nb_threads_hadd = int_cast(std::thread::hardware_concurrency()*0.25);

    std::string merge_command = "hadd -v 1 -j " + std::to_string(nb_threads_hadd) + " " + outMergedPath.string() + run_name + ".root " + outDataPath.string() + run_name + "_*.root";
    system(merge_command.c_str());

  }
  output_fileinfo.close();
  print(output_fileinfo_name, "written");
  return 0;
}