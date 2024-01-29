// 2. Include library
#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp>      // Use this to thread safely count what you want²
#include <MTTHist.hpp>        // Use this to thread safely fill histograms
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Detectors.hpp>      // Eases the manipulation of detector's labels
#include <Manip.hpp>          // Eases the manipulation of files and folder of an experiments
#include <RF_Manager.hpp>     // Eases manipulation of RF information
#include <Clovers.hpp>        // Analysis module for clovers

// #include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
// Data parameters :
double const & g_adc_threshold = 5000;
std::vector<double> g_trigger_blacklist = {70, 97};

// Event building parameters :
double const & g_rf_offset_ns = 25.0;
double const & g_begin_prompt_ns = -10;
double const & g_end_prompt_ns = 5;
double const & g_begin_delayed_ns = 30.0;// The first Ge must hit after this
double const & g_end_delayed_ns = 180.0;// The first Ge must hit before this
bool const & g_prompt_trigger = false; // If we require a prompt or not
int const & g_n_prev_pulses = 4; // Number of pulses before the trigger to take

  // Time windows :
Time const & g_Ge_coinc_tw_ns = 50; // time window in ns
Time const & g_BGO_coinc_tw_ns = 100; // time window in ns

// std::vector<Label> g_trigger_blacklist = {70};

// Other parameters :
std::string IDFile = "../index_129.list";
std::string calibFile = "../136_2024.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string output = "-root_dd";
int nb_files_ts = 60;
int nb_files = -1;
bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool histoed = false;
bool one_run = false;
std::string one_run_folder = "";
ulonglong max_hits = -1;
bool treat_129 = false;

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

struct Histos
{
  MTTHist<TH1F> energy_all_Ge_raw;
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

  
  TH1F* rf_evolution = nullptr;

  void Initialize()
  {
    auto const & nbDet = detectors.number();

    energy_all_Ge_raw.reset("energy_all_Ge_raw", "Ge spectra raw", 20000,0,10000);
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

double dT_ns(Time const & start, Time const & stop) {return(double_cast(stop-start)/1000.0);}

bool isPrompt(double const & rf_time) {return (rf_time>-g_begin_prompt_ns && rf_time<g_end_prompt_ns);}
bool isDelayed(double const & rf_time) {return (rf_time>g_begin_delayed_ns && rf_time<g_end_delayed_ns);}

/**
 * @brief Hits container
 * @details HitBuffer
 */
class HitBuffer
{
public:
  HitBuffer(size_t size = 100) : m_max_size(size) {m_buffer.resize(size);}

  void push_back(Hit const & hit) {m_buffer[m_size++] = hit;}
  void clear() {m_size = 0; m_buffer.clear(); m_buffer.resize(m_max_size); m_step = 0; m_nb_clear++;}

  auto const & operator[] (size_t const & i) const {return m_buffer[i];}
  auto operator[] (size_t const & i) {return m_buffer[i];}
  
  auto at(size_t const & i) 
  {
    if(i>m_size)
    {
      print("Exceeding size of HitBuffer");
      return m_buffer[m_size];
    }
    else return m_buffer[i];
  }

  auto const & size() const {return m_size;}
  auto max_size() const {return m_max_size;} 

  auto begin()       {return m_buffer.begin();}
  auto begin() const {return m_buffer.begin();}
  auto end  ()       {return m_buffer.end  ();}
  auto end  () const {return m_buffer.end  ();}

  bool isFull() const {return m_size+1 >= m_max_size;}

  auto const & nbClear() const {return m_nb_clear;}
  auto       & nbClear()       {return m_nb_clear;}

  void setStep(size_t const & step) {m_step = step;}
  auto const & step() const {return m_step;}
  auto       & step()       {return m_step;}

  /**
   * @brief Shifts the buffer of hits by a certain amount
   * 
   * @param n:
   * if n == 0 nothing happens
   * if n>0 shifts the buffer to the right by n indexes, expends the size if needed
   * if n<0 moves the n last cells to the beginning, size stays the same
   * @details
   * Example : 
   * HitBuffer buffer = {hit1, hit2, hit3, hit4};
   * buffer.shift( 2); // buffer = {empty1, empty2, hit1, hit2, hit3, hit4}
   * buffer.shift(-2); // buffer = {hit3, hit4, hit1, hit2}
   * Carefull : for n>0 , need to copy twice the data, so it can be very long
   */
  void shift(int n)
  {
    if (n == 0) return;
    else if (n>0)
    {
      std::vector<Hit> temp;
      temp.reserve(m_size+n);
      for (int i = 0; i<n; i++) temp[i] = Hit(); // Fill the nth first hits with empty hits
      for (size_t i = 0; i<m_size; i++)
      {// Shifts the other hits
        temp[i+n] = m_buffer[i];
      }
      m_buffer = temp;// Copy back to the first container
      m_size += n;// Update size
    }
    else if (n>0)
    {
      for (int i = 0; i<n; i++)
      {
        m_buffer[i] = m_buffer[m_size-n+i];
      }
      m_size = n;
    }
  }

private:
  size_t m_max_size = 0;
  size_t m_size = 0;
  size_t m_step = 0;
  size_t m_nb_clear = 0;
  std::vector<Hit> m_buffer;
};

/// @brief 
/// @return: true when clean, false when vetoed 
bool comptonClean(HitBuffer const & buffer, size_t & it, RF_Manager & rf, Histos & histos, bool const & histoed, bool const & delayed = true)
{
  auto const init_it = it;
  auto const & hit_Ge = buffer[init_it];
  auto const & clover_Ge = Clovers::labels[hit_Ge.label];
  auto const & time_Ge = rf.pulse_ToF_ns(hit_Ge);

  // --- a. Look backward to look for potential BGO within the time window before the hit --- //
  // Start with the first hit before the germanium we want to check :
  if (it>0) while(--it>buffer.step())
  {// Loop back until we either go out of the coincident window, 
   // step inside the prompt window or finds the last hit of the last written event
    auto const & hit_it = buffer[it];
    if (!isBGO[hit_it.label]) continue;

    auto const & time_diff = dT_ns(hit_it.stamp, hit_Ge.stamp); // dT_ns(start, stop)
    if(time_diff>g_BGO_coinc_tw_ns/2) break;

    // Checking prompt window :  We don't look inside the prompt peak for Compton suppression
    auto const & rf_time = time_Ge-time_diff;
    // if (delayed && rf_time<g_begin_delayed_ns) break;

    if (histoed && delayed) histos.BGO_VS_Ge_label.Fill(hit_Ge.label, hit_it.label);

    // Looking for BGO of the same clover module as the germanium
    if (clover_Ge==Clovers::labels[hit_it.label]) 
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

  // --- b. Look forward to look for potential BGO within the time window after the hit --- //
  for (it = init_it+1; it<buffer.size(); it++)
  {
    auto const & hit_it = buffer[it];
    if (!isBGO[hit_it.label]) continue;

    auto const & time_diff = dT_ns(hit_Ge.stamp, hit_it.stamp); // dT_ns(start, stop)
    if(time_diff>g_BGO_coinc_tw_ns/2) break; // No coincident forward BGO

    // if (delayed && rf_time > (rf.period_ns()-20)) break; // We don't look inside the prompt peak for Compton suppression

    if (histoed && delayed) histos.BGO_VS_Ge_label.Fill(hit_Ge.label, hit_it.label);

    if (clover_Ge==Clovers::labels[hit_it.label])
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

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, 
              Calibration const & calibration, 
              Timeshifts const & timeshifts, 
              Path const & outPath, 
              Histos & histos,
              MTCounter & total_read_size)
{
  // ------------------ //
  // Initialize helpers //
  // ------------------ //
  Timer timer;
  // Checking the lookup tables :
  if (!timeshifts || !calibration || !reader) return;

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  Filename filename = raw_datafile.filename();// "                               run_number_filenumber.fast"
  int filenumber = std::stoi(lastPart(filename.shortName(), '_'));//                        filenumber
  std::string run = removeLastPart(filename.shortName(), '_'); 

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
  Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
  File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  total_read_size+=raw_datafile.size();
  auto dataSize = float_cast(raw_datafile.size("Mo"));

  // ------------------------------ //
  // Initialize the temporary TTree //
  // ------------------------------ //
  std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
  tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  hit.writting(tempTree.get(), "lsEQp");

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
  
  print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

  if (rawCounts==0) {print("NO HITS IN",run); return;}

  // -------------------------------------- //
  // Realign switched hits after timeshifts //
  // -------------------------------------- //
  Alignator gindex(tempTree.get());

  // ------------------------------------------------------ //
  // Prepare the reading of the TTree and the output Events //
  // ------------------------------------------------------ //
  // Switch the temporary TTree to reading mode :
  hit.reset();
  hit.reading(tempTree.get(), "lsEQp");

  // Create output tree and Event 
  unique_tree outTree (new TTree("Nuball2","Nuball2"));
  outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  Event event;
  event.writting(outTree.get(), "lstEQp");

  // Initialize RF manager :
  RF_Manager rf;
  rf.set_period_ns(200);

  // Handle the first RF downscaled hit :
  RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
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
  Clovers::InitializeArrays();
  
  // Helpers :
  Timer convert_timer;
  auto const & nb_data = tempTree->GetEntries();
  // ulong evts_count = 0;
  ulong trig_count = 0;
  HitBuffer buffer(5000);
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
      for (size_t r_buffer_it = 0; r_buffer_it<buffer.size(); r_buffer_it ++)
      {
        auto const & hit_r = buffer[r_buffer_it];
        if (handleRf(rf, hit_r, event, outTree.get())) 
        {
          if (histoed)
          {
            histos.rf_evolution->SetBinContent(count_rf, rf.period);
            count_rf++;
          }
          continue;
        }
        // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||| //
        // ||| FIRST PART : TRIGGER ON TWO DELAYED CLEAN CLOVERS ||| //
        // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||| //

        if(isGe[hit_r.label])
        {// Trigger on Germaniums
          auto const & hit_first_Ge = hit_r;
          auto const & rel_time_first_Ge = rf.pulse_ToF_ns(hit_first_Ge);

          // ---------------------------------//
          //1. ---  Treat only delayed Ge --- //
          // ---------------------------------//
          if (isDelayed(rel_time_first_Ge))
          {// If the Germanium is in the delayed window, try to create the event :
            auto const init_it = r_buffer_it; // To easily come back to the first Ge
            std::vector<uchar> clover_modules = {Clovers::labels[hit_first_Ge.label]}; // List of the modules that fired in the event

            // -----------------------------//
            //2. --- BGO clean first Ge --- //
            // -----------------------------//
            if (histoed) histos.first_Ge_spectra.Fill(hit_first_Ge.nrj);
            
            if (!comptonClean(buffer, r_buffer_it, rf, histos, histoed)) 
            {
              if (histoed) histos.first_Ge_spectra_Vetoed.Fill(hit_first_Ge.nrj);
              continue;
            }

            if (histoed) histos.first_Ge_spectra_Clean.Fill(hit_first_Ge.nrj);

            // ------------------------//
            //3. --- Find next Ges --- //
            // ------------------------//
            Timestamp front_window = hit_first_Ge.stamp + g_Ge_coinc_tw_ns*1000;
            std::vector<int> next_Ge_indexes;
            while (++r_buffer_it < buffer.size())
            {
              auto const & hit_it = buffer[r_buffer_it];

              //1. Checking the time coincidence
              if(hit_it.stamp>front_window) break;

              //2. Remove hits of the prompt of next pulse
              auto const & time_diff_ns = dT_ns(hit_first_Ge.stamp, hit_it.stamp);// dT_ns(start, stop)
              if (rel_time_first_Ge + time_diff_ns > g_end_delayed_ns) break; 

              if (isGe[hit_it.label])
              {
                // 3. Add-Back : Checking if the label of the germanium's clover module have already fired
                auto const & clover_module_it = Clovers::labels[hit_it.label];
                if (found(clover_modules, clover_module_it)) continue;
                clover_modules.push_back(clover_module_it);
                
                // 4. Register the hit
                next_Ge_indexes.push_back(r_buffer_it);
              }
            } 

            r_buffer_it = init_it;

            if (histoed) histos.first_Ge_time_VS_nb_delayed.Fill(next_Ge_indexes.size(), rel_time_first_Ge);

            // Trigger : there needs to be at least another clover that fired
            if (next_Ge_indexes.size() < 1) continue;

            // debug();
            // debug(hit_first_Ge);
            // debug(buffer[next_Ge_indexes.front()]);

            if (histoed)
            {
              auto const & time_Ge2 = rel_time_first_Ge + dT_ns(hit_first_Ge.stamp, buffer[next_Ge_indexes[0]].stamp);
              histos.second_Ge_time_VS_nb_delayed.Fill(next_Ge_indexes.size(), time_Ge2);
              histos.Ge2_VS_Ge_time.Fill(rel_time_first_Ge, time_Ge2);
              
              if (next_Ge_indexes.size() > 2)
              {
                auto const & time_Ge3 = rel_time_first_Ge + dT_ns(hit_first_Ge.stamp, buffer[next_Ge_indexes[1]].stamp);
                histos.Ge3_VS_Ge_time.Fill(rel_time_first_Ge, time_Ge3);
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
              auto const & index_next_clover = Clovers::labels[hit_Ge2.label];

              if (histoed && index_it == 0) histos.second_Ge_spectra.Fill(buffer[next_Ge_indexes[0]].nrj);

              // 4.b Compton cleaning
              // If the Germanium is Clean, then the trigger is complete
              if (comptonClean(buffer, r_buffer_it, rf, histos, histoed))
              {
                if (histoed && index_it == 0) histos.second_Ge_spectra_Clean.Fill(buffer[next_Ge_index].nrj);
                trigger_Ge_clean = true;
                break;
              }

              // If a BGO vetoed this Ge, move to next one to check wether he's rejected
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

            // ------------------------------------------------------------------------------//
            //5. --- Requiring at least one prompt hit within few RF cycles in the past  --- //
            // ------------------------------------------------------------------------------//
            // (This prompt event is likely to be prompt decay feeding isomeric states)
            bool one_prompt_before = false;
            for (;r_buffer_it>buffer.step();r_buffer_it--)
            {
              auto const & hit_it = buffer[r_buffer_it];
              auto const & pulse_time_ns = rf.pulse_ToF_ns(hit_it.stamp);
              if (isPrompt(pulse_time_ns)) 
              {
                // all_Ge_after_trigger_with_prompt.Fill();
                one_prompt_before = true;
                break;
              }
            }
            

            r_buffer_it = init_it;

            // if (!one_prompt_before) continue;

            
        // ||||||||||||||||||||||||||||||||||||||||||||||||||| //
        // ||| SECOND PART : CREATE THE EVENT AND WRITE IT ||| //
        // ||||||||||||||||||||||||||||||||||||||||||||||||||| //

            // Extract some usefull informations :
            Timestamp ref_time = rf.refTime(hit_first_Ge.stamp);
            Timestamp back_window = ref_time - g_n_prev_pulses*rf.period - rf.offset();
            
            // ----------------------------------------------//
            //6. --- Locate the first hit in time window --- //
            // ----------------------------------------------//
            int pos = 0;
            if (r_buffer_it>0) while (--r_buffer_it>buffer.step())
            {
              if(buffer[r_buffer_it].stamp<back_window) break;
              pos++;
            }

            // -------------------------//
            //7. --- Fill the Event --- //
            // -------------------------//
            int count = 0;
            while (++r_buffer_it<buffer.size())
            {
              auto const & hit_it = buffer[r_buffer_it];
              // debug(hit_it, Long64_cast(front_window-hit_it.stamp));
              if (hit_it.stamp>front_window) break;
              if (handleRf(rf, hit_it, event, outTree.get())) continue;
              // Add this line if too much data is written :
              // auto const & time_j = rf.pulse_ToF_ns(hit_it);
              // if (isPrompt(time_j))  
              // {
                if (hit_it.stamp == 0) print(hit_it);
              event.push_back(hit_it);
              // }
              count++;
            }
            // The t0 is set to be the closest prompt peak
            // !!! to be changed afterwards to be the closest prompt clean event !!!
            event.setT0(ref_time);

            // debug(event);
            // pauseDebug();

            // --------------------------//
            //8. --- Write the Event --- //
            // --------------------------//
            outTree -> Fill();
            buffer.setStep(r_buffer_it);

            if (histoed) for (size_t i = 0; i<event.size(); i++) if (isGe[event.labels[i]])
              histos.energy_all_Ge_raw.Fill(event.nrjs[i]);

            // --------------//
            //9. --- END --- //
            // --------------//
            hits_count+=event.size();
            event.clear();
            r_buffer_it = init_it;
          }
          else if (isPrompt(rel_time_first_Ge) && histoed)
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
  convert_timer.Stop();
  print("Conversion finished here done in", convert_timer.TimeElapsedSec() , "s (",dataSize/convert_timer.TimeElapsedSec() ,"Mo/s)");
  Timer write_timer;

  // Initialize output TTree :
  unique_TFile outFile (TFile::Open(outfile.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  write_timer.Stop();

  auto outSize  = float_cast(size_file_conversion(float_cast(outFile->GetSize()), "o", "Mo"));

  print_precision(4);
  print(outfile, "written in", timer(), timer.unit(),"(",dataSize/timer.TimeSec(),"Mo/s). Input file", dataSize, 
        "Mo and output file", outSize, "Mo : compression factor ", dataSize/outSize,"-", (100.*double_cast(hits_count))/double_cast(rawCounts),"% hits kept");
}

// Attempt to bypass the MTFasterReader to see if it is slowing down the process :
// void paralellisation(MTList & list,
//               Calibration const & calibration, 
//               Timeshifts const & timeshifts, 
//               Path const & outPath, 
//               Histos & histos,
//               MTCounter & total_read_size)
// {
//   std::string filename;
//   while(list.getNext(filename))
//   {
//     Hit hit;
//     FasterReader reader(&hit, filename);
//     convert(hit, reader, calibration, timeshifts, outPath, histos, total_read_size); 
//   }
// }

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
        else if (command == "--run")
        {
          one_run = true;
          one_run_folder = argv[++i];
        }
        else if (command == "-H" || command == "--histograms")
        {
          histoed = true;
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
          print("(-f  || --files-number)   [files-number]  : set the number of files");
          print("(       --run)            [runname]       : set only one folder to convert");
          print("(-h  || --help)                           : display this help");
          print("(-H  || --histograms)                     : Fills and writes raw histograms");
          print("(-m  || --multithread)    [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
          print("(-n  || --number-hits)    [hits_number]   : set the number of hit to read in each file.");
          print("(-o  || --overwrite)                      : overwrites the already written folders. If a folder is incomplete, you need to delete it");
          print("(       --only-timeshift)                 : Calculate only timeshifts, force it even if it already has been calculated");
          // print("(-t  || --trigger)        [trigger]       : Default ",list_trigger,"|", trigger_legend);
          print("(-Th || --Thorium)                        : Treats only the thorium runs (run_nb < 75)");
          print("(-U  || --Uranium)                        : Treats only the uranium runs (run_nb >= 75)");
          // print("(       --129)                            : Treats the N-SI-129 pulsed runs");
          return 0;
        }
      }
      catch(std::invalid_argument const & error) 
      {
        throw_error("Can't interpret" + std::string(argv[i]) + "as an integer");
      }
    }
  }

  // MANDATORY : initialize the multithreading !
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(nb_files);
  MTObject::Initialize();

  // Setup the path accordingly to the machine :
  Path datapath = Path::home();
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line in the main.cpp ^^^^^^^^"); return -1;}

  // Input file :
  Path manipPath = datapath+manip;

  // Output file :
  // if (extend_periods) output+="_"+std::to_string(nb_periods_more)+"periods";
  Path outPath (datapath+(manip.name()+output), true);

  print("Reading :", manipPath.string());
  print("Writting in : ", outPath.string());

  // Load some modules :
  detectors.load(IDFile);
  Calibration calibration(calibFile);
  Manip runs(list_runs);
  if (one_run) runs.setFolder(one_run_folder);

  // Checking that all the modules have been loaded correctly :
  if (!calibration || !runs) return -1;

  // Setup some parameters :
  RF_Manager::set_offset_ns(g_rf_offset_ns);
  
  // Loop sequentially through the runs and treat their files in parallel :
  std::string run;
  while(runs.getNext(run))
  {
    MTCounter total_read_size;
    Timer timer;
    auto const & run_path = manipPath+run;
    auto const & run_name = removeExtension(run);

    Histos histos;
    if (histoed && !only_timeshifts) histos.Initialize();

    print("----------------");
    print("Treating ", run_name);

    // Timeshifts loading : 
    Timeshifts timeshifts(outPath.string(), run_name);
    if (treat_129) 
    {
      timeshifts.dT_with_raising_edge("dssd");
      timeshifts.dT_with_RF("dssd");
    }

    // If no timeshifts data for the run already available, calculate it :
    if (!timeshifts || (only_timeshifts && overwrite)) 
    { 
      timeshifts.setMult(2,4);
      timeshifts.setOutDir(outPath.string());

      timeshifts.calculate(run_path, nb_files_ts);
      timeshifts.verify(run_path, 10);

      timeshifts.write(run_name);
    }

    if (!only_timeshifts)
    {
      // Loop over the files in parallel :
      MTFasterReader readerMT(run_path, nb_files);
      readerMT.readRaw(convert, calibration, timeshifts, outPath, histos, total_read_size);

      if (histoed)
      {
        auto const name = outPath+run_name+"/histo_"+run_name+".root";
        std::unique_ptr<TFile> outFile (TFile::Open(name.c_str(), "RECREATE"));
        outFile -> cd();
        histos.energy_all_Ge_raw.Write();
        histos.rf_all_raw.Write();
        histos.energy_each_raw.Write();
        histos.rf_each_raw.Write();
        histos.energy_all_Ge.Write();
        histos.energy_each.Write();
        histos.rf_all.Write();
        histos.rf_each.Write();
        histos.energy_all_Ge_event.Write();
        histos.rf_all_event.Write();
        histos.energy_each_event.Write();
        histos.rf_each_event.Write();
        histos.energy_all_Ge_trig.Write();
        histos.energy_each_trig.Write();
        histos.rf_all_trig.Write();
        histos.rf_each_trig.Write();
        // for (auto & histo : histos.paris_bidim) histo.Write();
        // for (auto & histo : histos.paris_bidim_M_inf_4) histo.Write();

        histos.promptGe.Write();
        histos.promptGe_clean.Write();
        histos.promptGe_vetoed.Write();

        histos.BGO_VS_Ge_time.Write();
        histos.BGO_VS_Ge_time_prompt.Write();
        histos.E_Ge_BGO_dT_Ge_VS.Write();
        histos.BGO_VS_Ge_label.Write();
        histos.BGO_VS_Ge_label_vetoed.Write();
        histos.BGO_VS_Ge_label_clean.Write();

        histos.first_Ge_spectra.Write();
        histos.first_Ge_spectra_Clean.Write();
        histos.first_Ge_spectra_Vetoed.Write();

        histos.second_Ge_spectra.Write();
        histos.second_Ge_spectra_Clean.Write();
        histos.second_Ge_spectra_Vetoed.Write();

        histos.first_Ge_time_VS_nb_delayed.Write();
        histos.second_Ge_time_VS_nb_delayed.Write();

        histos.all_Ge_after_trigger.Write();

        histos.Ge2_VS_Ge_time.Write();
        histos.Ge3_VS_Ge_time.Write();
        histos.Ge3_VS_Ge2_time.Write();


        histos.rf_evolution->Write();
        delete histos.rf_evolution;

        outFile -> Write();
        outFile -> Close();
        print(name, "written");
      }
    }
    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer.TimeSec());

    Path outMerged (outpath.string()+"merged", true);

    std::string merge_command = "hadd -v 1 -j " + std::to_string(nb_threads) + outMerged.string() + run_name + ".root "+run_name+"_*.root";
    MTObject::parallelise_function(system, merge_command.c_str());
  }
  return 1;
}
