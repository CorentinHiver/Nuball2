// 1. Parameters
  // RF : 
#define USE_RF 200 //ns
  // Detectors :
#define USE_DSSD
#define USE_PARIS
  // Triggers :
#define TRIGGER
  // Event building :
#define PREPROMPT
// #define UNSAFE

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
#include <Clovers.hpp>

#include "EventBuilder_136.hpp" // Event builder for this experiment

// 3. Declare some global variables :
std::string IDFile = "index_129.list";
std::string calibFile = "136_final.calib";
Folder manip = "N-SI-136";
std::string list_runs = "list_runs.list";
std::string output = "-root_";
int nb_files_ts = 60;
int nb_files = -1;
int rf_shift = 40;
bool only_timeshifts = false; // No conversion : only calculate the timeshifts
bool overwrite = false; // Overwrite already existing converted root files. Works also with -t options (only_timeshifts)
bool histoed = false;
bool one_run = false;
bool check_preprompt = false;
std::string one_run_folder = "";
ulonglong max_hits = -1;
bool treat_129 = false;

bool extend_periods = false; // To take more than one period after a event trigger
uint nb_periods_more = 0; // Number of periods to extend after an event that triggered

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

bool trigger(Counter136 const & counter)
{
  switch (trigger_choice)
  {
    case 0: return counter.nb_dssd>0;
    case 1: return counter.nb_modules>2 && counter.nb_Ge>0;
    case 2: return (counter.nb_dssd>0 || (counter.nb_modules>2 && counter.nb_Ge>0));
    case 3: return (counter.nb_dssd>0 && counter.nb_modules>1 && counter.nb_Ge>0);
    case 4: return (counter.nb_dssd>0 || (counter.nb_modules>3 && counter.nb_Ge>0));
    case 5: return (counter.nb_modules>3 && counter.nb_Ge>0);
    default: return true;
  }
  // return ((counter.nb_modules>2 && counter.nb_clovers>0) || counter.nb_dssd>0);
}

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

  void Initialize()
  {
    auto const & nbDet = detectors.number();

    energy_all_Ge_raw.reset("energy_all_Ge_raw", "Ge spectra raw", 20000,0,10000);
    rf_all_raw.reset("rf_all_raw", "RF Time spectra raw", 2000, -1200, 800);
    energy_each_raw.reset("energy_each_raw", "Energy spectra each raw", nbDet,0,nbDet, 5000,0,15000);
    rf_each_raw.reset("rf_each_raw", "RF timing each raw", nbDet,0,nbDet, 2000, -1200, 800);

    // auto const & nb_paris = detectors.nbOfType("paris");
    // paris_bidim.resize(nb_paris);
    // paris_bidim_M_inf_4.resize(nb_paris);
    // for (size_t paris_i = 0; paris_i<nb_paris; paris_i++)
    // {
    //   auto const & name = detectors.name("paris", paris_i);
    //   paris_bidim[paris_i].reset(name.c_str(), name.c_str(), 100,-2,2, 15000,0,15000);
    //   paris_bidim_M_inf_4[paris_i].reset((name+"_M<4").c_str(), (name+"_M<4").c_str(), 100,-1,2, 15000,0,15000);
    // }

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
  }
};

bool handleRf(RF_Manager & rf, Hit const & hit, Event & event, TTree * tree)
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

double dT_ns(Time const & stop, Time const & start) {return(double_cast(stop-start)/1000.0);}

bool isPrompt(double const & time_rf) {return (time_rf>-20 && time_rf<5);}
bool isDelayed(double const & time_rf) {return (time_rf>60 && time_rf<180);}

class HitBuffer
{
public:
  HitBuffer(size_t size = 100)
  {
    m_buffer.resize(size);
  }

  void push_back(Hit const & hit) {m_buffer[m_size++] = hit;}
  void clear() {m_size = 0;}

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
  auto max_size() const {return m_buffer.size();} 

  auto begin() {return m_buffer.begin();}
  auto end() {return m_buffer.begin()+m_size;}

  auto begin() const {return m_buffer.begin();}
  auto end() const {return m_buffer.begin()+m_size;}

  bool isFull() const {return m_size+1 >= m_buffer.size();}

  void setStep(size_t const & step) {m_step = step;}
  auto const & step() const {return m_step;}
  auto const & step() {return m_step;}

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
      for (int i = 0; i<m_size; i++)
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
  size_t m_size = 0;
  size_t m_step = 0;
  std::vector<Hit> m_buffer;
};

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader, 
              Calibration const & calibration, 
              Timeshifts const & timeshifts, 
              Path const & outPath, 
              Histos & histos,
              MTCounter & total_read_size)
{
  Timer timer;
  // Checking the lookup tables :
  if (!timeshifts || !calibration || !reader) return;

  // Extracting the run name :
  File raw_datafile = reader.getFilename();   // "/path/to/manip/run_number.fast/run_number_filenumber.fast"
  Filename filename = raw_datafile.filename();// "                               run_number_filenumber.fast"
  int filenumber = std::stoi(lastPart(filename.shortName(), '_'));
  std::string run_path = raw_datafile.path(); // "/path/to/manip/run_number.fast/"
  std::string temp = run_path;                // "/path/to/manip/run_number.fast/"
  temp.pop_back();                            // "/path/to/manip/run_number.fast"
  std::string run = rmPathAndExt(temp);       //                "run_number"
  // int run_number = std::stoi(lastPart(run,'_'));//                   number

  // Setting the name of the output file :
  Path outFolder (outPath+run, true);                     // /path/to/manip-root/run_number.fast/
  Filename outFilename(raw_datafile.shortName()+".root"); //                                     run_number_filenumber.root
  File outfile (outFolder, outFilename);                  // /path/to/manip-root/run_number.fast/run_number_filenumber.root

  // Important : if the output file already exists, then do not overwrite it !
  if ( !overwrite && file_exists(outfile) ) {print(outfile, "already exists !"); return;}

  total_read_size+=raw_datafile.size();
  auto dataSize = float_cast(raw_datafile.size("Mo"));

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> tempTree (new TTree("temp","temp"));
  tempTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  hit.writting(tempTree.get(), "lsEQp");

  // Loop over the TTree 
  Timer read_timer;
  ulong rawCounts = 0;
  while(reader.Read())
  {
    // Time calibration :
    hit.stamp+=timeshifts[hit.label];

    // Energy calibration : 
    hit.nrj = calibration(hit.adc,  hit.label); // Normal calibration
    hit.nrj2 = ((hit.qdc2 == 0) ? 0.f : calibration(hit.qdc2, hit.label)); // Calibrate the qdc2 if any
  
    if (isGe[hit.label] && hit.nrj>10000) continue;

    tempTree -> Fill();
    rawCounts++;
  }

  read_timer.Stop();
  
  print("Read of",raw_datafile.shortName(),"finished here,", rawCounts,"counts in", read_timer.TimeElapsedSec(),"s (", dataSize/read_timer.TimeElapsedSec(), "Mo/s)");

  if (rawCounts==0) {print("NO HITS IN",run); return;}

  // Realign switched hits after timeshifts :
  Alignator gindex(tempTree.get());

  // Switch the temporary TTree to reading mode :
  hit.reset();
  hit.reading(tempTree.get(), "lsEQp");

  unique_tree outTree (new TTree("Nuball2","Nuball2"));
  outTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  Event event;
  event.writting(outTree.get(), "lstEQp");

  // Initialize event builder based on RF :
  RF_Manager rf;
  auto const & back_time_window = 4*rf.period+rf.offset();
  auto const & front_time_window = 4*rf.period-rf.offset();
  EventBuilder_136 eventBuilder(&event, &rf);
  eventBuilder.setNbPeriodsMore(nb_periods_more);

  // Handle the first RF downscale :
  RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);
  if (!first_rf) return;
  eventBuilder.setFirstRF(hit);

  debug("first RF found at hit n°", first_rf.cursor());

  // Initialize event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Handle the first hit :
  int loop = 0;
  // The first few first hundreds of thousands of hits, in the first file of each run,
  // don't have RF downscale. So we have to skip them in order to maintain good temporal resolution :
  if (filenumber == 1) loop = first_rf.cursor(); 
  tempTree -> GetEntry(gindex[loop++]);
  eventBuilder.set_first_hit(hit);

  //Loop over the data :
  Timer convert_timer;
  auto const & nb_data = tempTree->GetEntries();
  ulong hits_count = 0;
  ulong evts_count = 0;
  ulong trig_count = 0;
  HitBuffer buffer(500);
  size_t w_buffer_it = 0;
  while (loop<nb_data)
  {
    tempTree -> GetEntry(gindex[loop++]);
    
    buffer.push_back(hit);
    if (buffer.isFull()) 
    {
      size_t last_event_index = 0; // The index of the last hit of the last event
      for (size_t r_buffer_it = 0; r_buffer_it<w_buffer_it; r_buffer_it ++)
      {
        auto const & hit_r = buffer[r_buffer_it];
        if (handleRf(rf, hit_r, event, outTree.get())) continue;
        else // Treat the other data :
        {
          if(isGe[hit_r.label])
          {
            auto const & hit_first_Ge = hit_r;
            auto const & time_first_Ge = rf.pulse_ToF_ns(hit_first_Ge);

            // ---------------------------------//
            //1. ---  Treat only delayed Ge --- //
            // ---------------------------------//

            if (time_first_Ge>60 && time_first_Ge<180)
            {
              auto const & buffer_index_first_Ge = r_buffer_it; // Store the position of the Ge
              auto const & index_first_clover = Clovers::labels[hit_first_Ge.label]; // Between 0 and 23

              // -----------------------------//
              //2. --- BGO clean first Ge --- //
              // -----------------------------//
              
              int coinc_timewindow = 60;
              bool rejected_first_Ge = false;
              // --- a. Look backward to look for potential BGO within 60ns time window --- //
              // Start already with the first hit before the germanium we want to check :
              --r_buffer_it;
              for(; r_buffer_it>buffer.step(); r_buffer_it--)
              {
                auto const & hit_j = buffer[r_buffer_it];
                auto const & dT_j = dT_ns(hit_first_Ge.stamp, hit_j.stamp); // In ns
                if(dT_j>coinc_timewindow/2) break;
                auto const & time_rf = rf.pulse_ToF(hit_j);
                if (isPrompt(time_rf)) break; // We don't look inside the prompt peak for Compton suppression
                if (isBGO[hit_j.label] && index_first_clover==Clovers::labels[hit_j.label]) 
                {
                  rejected_first_Ge = true;
                  break;
                }
              } 

              if (rejected_first_Ge) continue;

              // --- b. Look forward to look for potential BGO within the time window --- //
              // Start with the first hit after the germanium we want to check :
              r_buffer_it = buffer_index_first_Ge+1;
              for (; r_buffer_it<buffer.size(); r_buffer_it++)
              {
                auto const & hit_j = buffer[r_buffer_it];
                auto const & dT_j = double_cast(hit_j.stamp-hit_first_Ge.stamp)/1000.0; // In ns
                if(dT_j>coinc_timewindow/2) break; // No coincident forward BGO
                auto const & time_rf = rf.pulse_ToF(hit_j);
                if (isPrompt(time_rf)) break;
                if (isBGO[hit_j.label] && index_first_clover==Clovers::labels[hit_j.label])
                {
                  rejected_first_Ge = true;
                  break;
                }
              }

              if (rejected_first_Ge) continue;

              // ------------------------//
              //3. --- Find next Ges --- //
              // ------------------------//
              
              std::vector<int> buffer_index_second_Ges;
              while (++r_buffer_it < buffer.size())
              {
                auto const & hit_j = buffer[r_buffer_it];
                auto const & dT_j = double_cast(hit_j.stamp-hit_first_Ge.stamp)/1000.0; // In ns
                if(dT_j>30) break; // No other coincident hits

                // JE LE GARDE ???? PEUT-ÊTRE QUE CA PERMET DE CHOPPER DES VRAIS DELAYED
                auto const & time_j = rf.pulse_ToF_ns(hit_j);
                if (time_j>180) break; // Not delayed anymore 

                if (isGe[hit_j.label] && index_first_clover!=Clovers::labels[hit_j.label])
                {
                  buffer_index_second_Ges.push_back(r_buffer_it);
                }
              } 

              if (buffer_index_second_Ges.size() == 0) continue;
              
              // -------------------------------//
              //4. --- BGO clean second Ges --- //
              // -------------------------------//

              bool trigger_2Ge_clean = false;
              // Loop through the other Ges in time window
              for(auto const & buffer_index_second_Ge : buffer_index_second_Ges)
              {
                r_buffer_it = buffer_index_second_Ge;
                auto const & hit_Ge2 = buffer[r_buffer_it];
                auto const & index_second_clover = Clovers::labels[hit_Ge2.label];

                bool rejected_second_Ge = false;
                // 4.a Look backward to look for potential BGO within 60ns time window
                do
                {
                  auto const & hit_j = buffer[--r_buffer_it];
                  auto const & dT_j = dT_ns(hit_first_Ge.stamp, hit_j.stamp); // In ns
                  if(dT_j>30) break; // No backward coincident BGO
                  if (isBGO[hit_j.label] && index_second_clover==Clovers::labels[hit_j.label]) rejected_second_Ge = true;
                } while(r_buffer_it>last_event_index && !rejected_second_Ge);

                if (rejected_second_Ge) // TODO Also check size here
                {
                  r_buffer_it = buffer_index_second_Ge+1;
                  continue;
                }
                else r_buffer_it = buffer_index_second_Ge+1;

                // 4.b Look forward to look for potential BGO within 60ns time window
                do
                {
                  auto const & hit_j = buffer[r_buffer_it];
                  auto const & dT_j = dT_ns(hit_j.stamp, hit_first_Ge.stamp); // In ns
                  if(dT_j>30) break; // No coincident forward BGO
                  if (isBGO[hit_j.label] && index_second_clover==Clovers::labels[hit_j.label]) rejected_second_Ge = true;
                } while(++r_buffer_it<buffer.size() && !rejected_second_Ge);

                // If a BGO vetoed this potential Ge, move to next one
                if (rejected_second_Ge)
                {
                  r_buffer_it = buffer_index_second_Ge+1;
                  continue;
                }
                // If the Germanium is Clean, then 
                else
                {
                  trigger_2Ge_clean = true;
                  r_buffer_it = buffer_index_first_Ge-1;
                  break;
                }
              }

              if (!trigger_2Ge_clean) continue;

              // --------------------------------------------------//
              //5. --- Look for at least one prompt hit before --- //
              // --------------------------------------------------//

              bool one_prompt_before = false;
              for (;r_buffer_it>last_event_index;r_buffer_it--)
              {
                auto const & hit_j = buffer[r_buffer_it];
                if (dT_ns(hit_first_Ge.stamp, hit_j.stamp) < back_time_window) 
                {
                  one_prompt_before = true;
                  break;
                }
              }

              if (!one_prompt_before) continue;
              
              // ------------------------------------------------//
              //6. --- Look for the first hit in time window --- //
              // ------------------------------------------------//
              while(--r_buffer_it>last_event_index)
              {
                auto const & hit_j = buffer[r_buffer_it];
                if (dT_ns(hit_first_Ge.stamp, hit_j.stamp)>back_time_window) break;
              }

              // -------------------------//
              //6. --- Fill the Event --- //
              // -------------------------//

              while (++r_buffer_it<buffer.size())
              {
                auto const & hit_j = buffer[r_buffer_it];
                if (dT_ns(hit_first_Ge.stamp, hit_j.stamp)>front_time_window) break;
                // Add this line if too much data is written :
                // auto const & time_j = rf.pulse_ToF_ns(hit_j);
                // if (isPrompt(time_j))  
                // {
                event.push_back(hit);
                // }
              }

              // --------------------------//
              //7. --- Write the Event --- //
              // --------------------------//

              outTree -> Fill();
            }
          }
        }
      }
      w_buffer_it = 0;
    }
    buffer.clear();

    // if (histoed)
    // {
    //   auto const tof_hit = rf.pulse_ToF_ns(hit.stamp);
    //   histos.rf_all.Fill(tof_hit);
    //   histos.rf_each.Fill(compressedLabel[hit.label], tof_hit);
      
    //   if (isGe[hit.label]) histos.energy_all_Ge.Fill(hit.nrj);
    //   histos.energy_each.Fill(compressedLabel[hit.label], hit.nrj);

    //   // if (isParis[hit.label])
    //   // {
    //   //   auto const & ratio = (hit.nrj2-hit.nrj)/hit.nrj2;
    //   //   auto const & index = detectors.index(hit.label);
    //   //   histos.paris_bidim[index].Fill(ratio, hit.nrj);
    //   //   if (event.mult<5) histos.paris_bidim_M_inf_4[index].Fill(ratio, hit.nrj);
    //   // }
    // }

    // print_precision(13);
    // Event building :
    // if (eventBuilder.build(hit))
    // {
    //   evts_count++;
    //   counter.count(event); 
    //   if ((evts_count%(int)(1.E+6)) == 0) debug(evts_count/(int)(1.E+6), "Mevts");
    //   if (histoed)
    //   {
    //     auto const pulse_ref = rf.pulse_ToF_ns(event.stamp);
    //     for (size_t trig_loop = 0; trig_loop<event.size(); trig_loop++)
    //     {
    //       auto const & label  = event.labels[trig_loop];
    //       auto const & time   = event.times [trig_loop];
    //       auto const tof_trig = pulse_ref+time/1000ll;
    //       auto const & nrj    = calibration(event.nrjs[trig_loop], label);

    //       histos.rf_all_event.Fill(tof_trig);
    //       histos.rf_each_event.Fill(compressedLabel[label], tof_trig);
      
    //       if (isGe[label]) histos.energy_all_Ge_event.Fill(nrj+gRandom->Uniform(0,1));
    //       histos.energy_each_event.Fill(compressedLabel[label], nrj);
    //     }
    //   }
    // #ifdef TRIGGER
    //   if (trigger(counter))
    //   {
        
    // #ifdef PREPROMPT
    //     eventBuilder.tryAddPreprompt_simple();
    // #endif //PREPROMPT

    //     if (nb_periods_more>0) eventBuilder.tryAddNextHit_simple(tempTree.get(), outTree.get(), hit, loop, gindex);

    //     hits_count+=event.size();
    //     trig_count++;
    //     outTree->Fill();

    //     if (histoed)
    //     {
    //       auto const pulse_ref = rf.pulse_ToF_ns(event.stamp);
    //       for (uint trig_loop = 0; trig_loop<event.size(); trig_loop++)
    //       {
    //         auto const & label  = event.labels[trig_loop];
    //         auto const & nrj    = event.nrjs  [trig_loop];
    //         auto const & time   = event.times [trig_loop];
    //         auto const tof_trig = pulse_ref+time/1000ll;

    //         histos.rf_all_trig.Fill(tof_trig);
    //         histos.rf_each_trig.Fill(compressedLabel[label], tof_trig);

    //         if (isGe[label]) histos.energy_all_Ge_trig.Fill(nrj);
    //         histos.energy_each_trig.Fill(compressedLabel[label], nrj);
    //       }
    //     }
    //   }
    // #else
    //   outTree->Fill();
    // #endif
    // }
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
            if (command == "-e" || command == "--extend-period")
        {// To get more than 1 period after pulse if trigger activated 
          extend_periods = true;
          nb_periods_more = atoi(argv[++i]);
        }
        else if (command == "-f" || command == "--files-number")
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
          check_preprompt = true;
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
          check_preprompt = true;
        }
        else if (command == "-h" || command == "--help")
        {
          print("List of the commands :");
          print("(-f  || --files-number)   [files-number]  : set the number of files");
          print("(-e  || --extend-period)  [nb_periods]    : set the number of periods to extend the time window after trigger");
          print("(       --run)            [runname]       : set only one folder to convert");
          print("(-h  || --help)                           : display this help");
          print("(-H  || --histograms)                     : Fills and writes raw histograms");
          print("(-m  || --multithread)    [thread_number] : set the number of threads to use. Maximum allowed : 3/4 of the total number of threads");
          print("(-n  || --number-hits)    [hits_number]   : set the number of hit to read in each file.");
          print("(-o  || --overwrite)                      : overwrites the already written folders. If a folder is incomplete, you need to delete it");
          print("(       --only-timeshift)                 : Calculate only timeshifts, force it even if it already has been calculated");
          print("(-t  || --trigger)        [trigger]       : Default ",list_trigger,"|", trigger_legend);
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
  output+=trigger_name.at(trigger_choice);
  if (extend_periods) output+="_"+std::to_string(nb_periods_more)+"periods";
  Path outPath (datapath+(manip.name()+output), true);

  print("Reading :", manipPath.string());
  print("Writting in : ", outPath.string());

  // Load some modules :
  detectors.load("index_129.list");
  Calibration calibration(calibFile);
  Manip runs(list_runs);
  if (one_run) runs.setFolder(one_run_folder);

  // Checking that all the modules have been loaded correctly :
  if (!calibration || !runs) return -1;

  // Setup some parameters :
  RF_Manager::set_offset_ns(rf_shift);
  
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
    Timeshifts timeshifts(outPath, run_name);
    if (treat_129) 
    {
      timeshifts.dT_with_raising_edge("dssd");
      timeshifts.dT_with_RF("dssd");
    }

    // If no timeshifts data for the run already available, calculate it :
    if (!timeshifts || (only_timeshifts && overwrite)) 
    { 
      timeshifts.setMult(2,4);
      timeshifts.setOutDir(outPath);
      timeshifts.checkForPreprompt(check_preprompt);

      timeshifts.calculate(run_path, nb_files_ts);
      timeshifts.verify(run_path, 10);

      timeshifts.write(run_name);
    }

    if (!only_timeshifts)
    {
      // Loop over the files in parallel :
      MTFasterReader readerMT(run_path, nb_files);
      readerMT.readRaw(convert, calibration, timeshifts, outPath, histos, total_read_size);

      // Attempt to bypass the MTFasterReader to see if it is slowing down the process :
  // FilesManager files
  // if (!files) {print("NO DATA FILE FOUND"); throw std::runtime_error("DATA");}
  // MTFile MTfiles = files.getListFiles();
  // MTObject::parallelise_function(MTfiles, paralellisation, calibration, timeshifts, outPath, histos, total_read_size);

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


        outFile -> Write();
        outFile -> Close();
        print(name, "written");
      }
    }
    print(run_name, "converted at a rate of", size_file_conversion(total_read_size, "o", "Mo")/timer.TimeSec());
  }
  return 1;
}
