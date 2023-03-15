#ifndef NEARLINE_H
#define NEARLINE_H

#ifdef N_SI_120
#define LICORNE
#define FATIMA
// #define C1L2_C2_TRIG
#endif //N_SI_120

#ifdef N_SI_122
#define PARIS
#endif //N_SI_122

#ifdef N_SI_131
#define PARIS
#define USE_DSSD
#define FATIMA
#endif //N_SI_131

#ifdef N_SI_129
#define PARIS
#define USE_DSSD
#define FATIMA

#endif //N_SI_129

#if defined(LICORNE) || defined(PARIS)
#define QDC2
#endif //QDC2

// NearLine3 v1
#include "Classes/FilesManager.hpp"
#include "Classes/EventBuilder.hpp"
#include "Classes/CoincBuilder.hpp"
#include "Classes/CoincBuilder2.hpp"
#include "Classes/MTTHist.hpp"
#include "Classes/FasterReader.hpp"
#include "Classes/Calibration.hpp"
#include "Classes/Sorted_Event.hpp"

// Forward declaration of the modules :
class ParisBidim;
class Analyse;
class Analyse_W;

//Forward declaration of the detectors :
// class DSSD;

class NearLine
{
public:

  //Constructors
  NearLine(){}
  ~NearLine(){}

  //General functions
  Bool_t Initialize();
  Bool_t configOk();
  Bool_t launch();
  Bool_t multi_run();
  void   run_thread();
  Bool_t processFile(std::string filename, int thread_number = 0);
  void   faster2root(std::string filename, int thread_nb = 0);
  void   WriteData();

  //Configurations / Options :
  Bool_t loadConfig (std::string config_file_name = "parameters.dat");
  Bool_t setConfig  (std::stringstream & parameters_file);
  Bool_t setConfig  (std::string parameter = "");
  void   setThreads (int nb_threads = 1);

  //Data analysis
  Bool_t calibrate_hit(Hit & hit);
  Bool_t time_shift(Hit & hit);

  //Loading functions
  Bool_t loadLabels(std::string const & _fileID);
  Bool_t loadTimeshifts(std::string _inFile );
  Bool_t loadCalib(std::string const & _fileID);

  //Files managing
  Bool_t addFiles  (std::string const & file) {return p_Files.addFiles(file);};
  Bool_t addFolder (std::string const & folder, int nb_Files) {return p_Files.addFolder(folder, nb_Files);};
  void   flushFiles() {p_Files.flushFiles();};
  FilesManager p_Files; //not recommended to use it !! Just waiting for implement more clever use

  void        m_ca_calculate(std::string _histoFilename, Fits & fits, TFile* outRootFile = nullptr);

private:
  //! Data managing :
  Labels m_labelToName;
  std::map<std::string, UInt_t> m_nameToLabel;
  Int_t m_read_step = 1;
  Int_t m_read_start = 1;
  Int_t m_hits_read = -1;

  //! Utils

  //! Threads managing :
  std::vector<std::thread> m_threads;
  std::mutex m_mutex;
  UShort_t m_nb_threads = 1;
  Bool_t multithread = false;
  Bool_t m_STOP = true;
  UShort_t m_current_thread_nb= 0;

  //! Config managing :
  std::string m_parameters_filename = "parameters.dat";
  std::string m_outdir = "./";
  // Inputs :
    //Timing
  Timeshift m_timeshifts; // List of the time shifts per detector
    //Energy
  Calibration m_calib;// List of the energy calibration per detector
  Int_t m_E_threshold = 0; //keV | energy threshold for calibrated spectra
  Bool_t m_use_threshold = false; //keV | energy threshold for calibrated spectra

    //Energy binning
    MapDetector m_bins_raw   = { {null, 1}, {RF, 1}, {BGO, 1000  }, {LaBr3, 5000   }, {Ge, 20000  }, {EDEN, 10000}, {Paris, 5000  }, {DSSD, 10000 } };
    MapDetector m_min_raw    = { {null, 0}, {RF, 0}, {BGO, 0     }, {LaBr3, 0      }, {Ge, 0      }, {EDEN, 0    }, {Paris, 0     }, {DSSD, 0    } };
    MapDetector m_max_raw    = { {null, 1}, {RF, 1}, {BGO, 300000}, {LaBr3, 1000000}, {Ge, 200000 }, {EDEN, 10000}, {Paris, 1000000}, {DSSD, 100000} };

    MapDetector m_bins_calib = { {null, 1}, {RF, 1}, {BGO, 1000  }, {LaBr3, 1500   }, {Ge, 6000   }, {EDEN, 10000}, {Paris, 5000}, {DSSD, 500  } };
    MapDetector m_min_calib  = { {null, 0}, {RF, 0}, {BGO, 0     }, {LaBr3, 0      }, {Ge, 0      }, {EDEN, 0    }, {Paris, 0   }, {DSSD, 0    } };
    MapDetector m_max_calib  = { {null, 1}, {RF, 1}, {BGO, 3000  }, {LaBr3, 3000   }, {Ge, 3000   }, {EDEN, 1    }, {Paris, 3000}, {DSSD, 20000} };

    MapDetector m_bins_bidim = { {null, 1}, {RF, 1}, {BGO, 250   }, {LaBr3, 1000   }, {Ge, 6000   }, {Paris, 1000}, {DSSD, 1000 } };
    MapDetector m_min_bidim  = { {null, 0}, {RF, 0}, {BGO, 0     }, {LaBr3, 0      }, {Ge, 0      }, {Paris, 0   }, {DSSD, 0    } };
    MapDetector m_max_bidim  = { {null, 1}, {RF, 1}, {BGO, 3000  }, {LaBr3, 3000   }, {Ge, 6000   }, {Paris, 3000}, {DSSD, 20000} };

    //Detectors :

// Data file informations
  Bool_t m_evtbuilt = false; //If the data is known to already be reordered
  Bool_t m_apply_timeshift = false; // To force to apply the loaded timeshift
  Bool_t m_apply_calibration = false; // To force to apply the energy calibration
//Read informations
  Int_t m_maximum_time_read = -1; //if you want to analyse only a fraction of the data //NOT IMPLEMENTED
  Long64_t m_counter = 0;
  // Outputs :
  Bool_t temp_tree_on_disk = false;
    //Analyse (m_a)
  #ifdef LICORNE
  Float_t EDEN_TO_LICORNE_DISTANCE = 0;
  Float_t GAMMA_FLASH = 0;
  #endif //LICORNE

  friend class Analyse;
  Analyse *m_a = nullptr;

  friend class Analyse_W;
  Analyse_W *m_aw = nullptr;

  Int_t    m_RF_shift_ns = 50; //ns
  Long64_t m_RF_shift    = 50000ll; //ns
  Bool_t   m_use_RF = false;
    // Histograms
      // - Raw spectra (m_hr)
  Bool_t      m_hr = false;
  std::string m_hr_outroot = "histo_raw.root";
  void        m_hr_Initialize();
  void        m_hr_Write();
      // - Calibrated spectra (m_hc)
  Bool_t      m_hc            = false;
  Bool_t      m_hc_Ge_Clover  = false;
  Bool_t      m_hc_LaBr3      = false;
  Bool_t      m_hc_DSSD      = false;
  Bool_t      m_hc_Paris      = false;
  std::string m_hc_outroot    = "histo_calib.root";
  void        m_hc_Initialize();
  void        m_hc_Write();
    // Calcul calibration (m_ca)
  Bool_t      m_ca           = false;
  Bool_t      m_ca_verbose   = false;
  Bool_t      m_ca_residus   = false;
  Bool_t      m_ca_outRoot_b = false;
  std::string m_ca_source    = "";
  std::string m_ca_outRoot   = "calibration.root";
  std::string m_ca_outCalib  = "calibration.dat";
  std::string m_ca_outDir    = "Calibration/";
  void        m_ca_Write(std::string _histoFilename);
  void        m_ca_residus_calculate();//FATIMA CONFIGURATION ONLY FOR NOW !
  // std::map<std::string,Source_info> m_ca_sources_info;
    // Calcul Timeshifts (m_ts)
  Bool_t      m_ts            = false;
  Bool_t      m_ts_bidim      = false;
  Bool_t      m_ts_calibrated = false;
  Bool_t      m_ts_verbose    = false;
  std::string m_ts_outroot    = "timeshifts.root";
  std::string m_ts_outdir     = "Timeshifts/";
  Bool_t      m_ts_outroot_b  = false;
  std::string m_ts_outdata    = "deltaT.dat";
  UInt_t      m_ts_min_mult   = 2;
  UInt_t      m_ts_max_mult   = 2;
  Int_t       m_ts_timeWindow = 0; //ps | time window used for the event building
  Int_t       m_ts_timeWindow_ns = 0; //ns | idem
  Timing_ref  m_ts_time_ref; // time reference used for time alignement purposes
  Timeshift   m_ts_array;
  std::map<Detector, Float_t> m_ts_rebin = { {LaBr3,100}, {Ge,1000}, {BGO,500}, {EDEN,500}, {RF,100}, {Paris,100}, {DSSD,1000}, {EDEN,1000}};
  void        m_ts_Initialize();
  void        m_ts_Fill(Event const & ts_buffer, size_t const & refPos, UShort_t const & thread_nb);
  void        m_ts_calculate();

    //Faster2root
  Bool_t   m_fr               = false;
  Bool_t   m_fr_eventbuild    = false;
  Bool_t   m_fr_raw           = false;
  Bool_t   m_fr_keep_all      = false;
  Bool_t   m_fr_throw_singles = false;
  Long64_t m_fr_shift         = 0;
  Trigger  m_fr_trigger;
  MTCounter<Float_t>  m_fr_raw_run_size;
  MTCounter<Float_t>  m_fr_treated_run_size;
  MTCounter<Int_t>    m_fr_raw_counter;
  MTCounter<Int_t>    m_fr_treated_counter;
  void     m_fr_Initialize();
  void     m_fr_Write();
  void     m_fr_trigger_buffer(Event const & event, Counters &arg);
  void     m_fr_sum_counters();


  //! RF check
  bool m_rfc = false;
  int m_rfc_nbFiles = 5;
  Long64_t m_rfc_timeWindow = 1500000; //ps
  Long64_t m_rfc_timeWindow_ns = 1500;
  Label m_rfc_time_ref_label = 252;
  Time m_rfc_RF_shift = 50000ull; //ps
  Time m_rfc_RF_shift_ns = 50;
  std::string m_rfc_time_ref_det = "";
  std::string m_rfc_outroot = "check_rf.root";
  void m_rfc_Initialize();
  void m_rfc_Write();

  //Paris bidim histo :
  friend class ParisBidim;
  ParisBidim *m_pb = nullptr;

  //Detectors :
  // friend class DSSD;
  // DSSD *dssd = nullptr;

  // -------------- //
  //   HISTOGRAMS   //
  // -------------- //
  //Raw and calibrated histograms :
  std::vector<MTTHist<TH1I>> m_hr_histo;
  std::vector<MTTHist<TH1I>> m_hc_histo;
  MTTHist<TH2I>              m_hc_bidim_histo_Ge;
  MTTHist<TH2I>              m_hc_bidim_histo_LaBr3;
  MTTHist<TH2I>              m_hc_bidim_histo_Paris;
  //! Timeshifts calculations :
  MTTHist<TH1F>              m_ts_histo_RF;
  std::vector<MTTHist<TH1F>> m_ts_histo;
  std::vector<MTTHist<TH2F>> m_ts_E_VS_deltaT;
  MTTHist<TH1F> m_ts_DeltaT;
  MTTHist<TH2I> m_ts_label_DeltaT;
  MTTHist<TH1F> m_ts_EnergyRef;
  MTTHist<TH1I> m_ts_Label;
  MTTHist<TH1I> m_ts_mult;
  //! RF check
  MTTHist<TH1F> m_rfc_histo_RF;
  MTTHist<TH1F> m_rfc_histo_RF_test;
  MTTHist<TH1F> m_rfc_period;
  MTTHist<TH2F> m_rfc_RF_label;
  MTTHist<TH2F> m_rfc_RF_evol;

  //!Faster2Root
  MTTHist<TH1F> m_fr_Ge_raw;
  MTTHist<TH1F> m_fr_Ge_prompt;
  MTTHist<TH1F> m_fr_Ge_addback;
  MTTHist<TH1F> m_fr_Ge_compton;
  MTTHist<TH1F> m_fr_Ge_M1;
  MTTHist<TH1F> m_fr_Ge_M2;
  MTTHist<TH1F> m_fr_Ge_M3;
  MTTHist<TH1F> m_fr_Ge_M4;
  MTTHist<TH1F> m_fr_Ge_M5;
  MTTHist<TH1F> m_fr_Ge_M6;
  MTTHist<TH1F> m_fr_Ge_Msup6;
  MTTHist<TH1F> m_fr_M;
  MTTHist<TH2I> m_fr_C_VS_L;
  MTTHist<TH2F> m_fr_ToF_all_det;
  MTTHist<TH1F> m_fr_ToF;
  MTTHist<TH1F> m_fr_dT_pulse;
  MTTHist<TH2I> m_fr_C_VS_dT_pulse;

};

//include the modules :
#include "Modules/ParisBidim.hpp"
#include "Modules/Analyse.hpp"
#include "Modules/Analyse_W.hpp"

//include the detectors :
// #include "Detectors/DSSD.hpp"

Bool_t NearLine::launch()
{
  auto start = std::chrono::high_resolution_clock::now();
  if (p_Files.size() < 10) p_Files.Print();

  // First checks the configuration loaded :
  if (!this -> configOk()) return false;

  // Initialise all the histograms and some other variables :
  if (!this -> Initialize()) return false;

  if (p_Files.isEmpty()) {print("NO .fast FILES IN THAT FOLDER !"); return false;}

  // Data read and histogram filling
  this -> multi_run();

  // Write all the data/histogram
  this -> WriteData();

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration<double, std::milli>(end - start).count() / 1000 << " s" << std::endl;

  if (temp_tree_on_disk) if (folder_exists("analysisTempTrees/")) gSystem -> Exec("rm -r analysisTempTrees/");

  return true;
}

Bool_t NearLine::multi_run()
{
  m_STOP = false;

  if(m_nb_threads == 1)
  {
    multithread = false;
    std::cout << std::endl << "Runing without multithreading ..." << std::endl << std::endl;
    this -> run_thread();
  }
  else
  {
    multithread = true;
    for (int i = 0; i<m_nb_threads; i++)
    {
      // Run in parallel this command :
      //                              vvvvvvvvvvvv
      m_threads.emplace_back([this](){this -> run_thread();});
      //                              ^^^^^^^^^^^^
      //Note : the parameter "this" in the lambda allows all instances of run_thread() to have access to the members of the main NearLine object
    }
    for(size_t i = 0; i < m_threads.size(); i++) m_threads.at(i).join(); //Closes threads
    // print("NUMBER HITS : ", m_counter);
    m_threads.resize(0);
    m_threads.clear();
    std::cout << "Multi-threading is over !" << std::endl;
  }
  return true;
}

void NearLine::run_thread()
{
  if(m_STOP) return;
  m_mutex.lock();

  // Sets the thread number :
  Int_t thread_nb = 0;
  thread_nb = m_current_thread_nb;
  m_current_thread_nb++;
  m_mutex.unlock();

  //Sets the file to treat :
  std::string filename = "";
  while(!m_STOP)
  {
    m_mutex.lock();
    m_STOP = !p_Files.nextFileName(filename, m_read_step);
    m_mutex.unlock();
    if(!m_STOP)
    {
  // Treats the file :
      if (m_fr) faster2root(filename, thread_nb);
      else      processFile(filename, thread_nb);
    }
    else
    {
      break;
    }
  }
  if (multithread) std::cout << "Worker " << thread_nb << " finished" << std::endl;
}

void NearLine::faster2root(std::string filename, int thread_nb)
{
  Hit hit;
  std::string outfile = m_outdir+rmPathAndExt(filename)+".root";

  //Checking the file does'nt already exists :
  if (!folder_exists(getPath(outfile))) gSystem -> Exec(("mkdir "+getPath(outfile)).c_str());
  if ( file_exists(outfile) ) {print(outfile, "already exists !");return;}

  // Initialize :
  FasterReader reader(&hit, filename);
  if (!reader.isReady()) { print("CAN'T READ", filename); return;}

  auto start = std::chrono::high_resolution_clock::now();

  // std::unique_ptr<TFile> tempFile(new TFile(("analysisTempTrees/tempTree"+std::to_string(thread_nb)).c_str(), "recreate"));
  // if (!temp_tree_on_disk) gROOT -> cd();
  std::unique_ptr<TTree> rootTree (new TTree(("tempTree"+std::to_string(thread_nb)).c_str(), ("tempTree"+std::to_string(thread_nb)).c_str()));

  auto file_size = size_file(filename);
  m_fr_raw_run_size[thread_nb]+=file_size;
  rootTree.reset(new TTree("tempTree","tempTree"));
  rootTree -> Branch("label"  , &hit.label );
  rootTree -> Branch("time"   , &hit.time  );
  rootTree -> Branch("nrjcal" , &hit.nrjcal);
  #ifdef QDC2
  rootTree -> Branch("nrj2"   , &hit.nrj2);
  #endif //QDC2
  rootTree -> Branch("pileup" , &hit.pileup);

  int counter = 0;
  // while(reader.Read() && counter<10000)
  while(reader.Read())
  {
    // if(counter%1000 == 0) print(counter);
    counter++;
    time_shift(hit);
    m_calib.calibrate(hit);
    rootTree -> Fill();
    m_fr_raw_counter[thread_nb]++;
    #ifdef QDC2
    hit.nrj2 = 0; // In order to clean the data, as nrj2 never gets cleaned if there is no QDC2 in the next hit
    #endif //QDC2
  }

  print("Read finished, converting");

  ULong64_t nb_data = 0;
  nb_data = rootTree -> GetEntries();
  std::vector<Int_t> gindex(nb_data,0);
  for (size_t nb = 0; nb<gindex.size(); nb++) gindex[nb] = nb;
  if (!m_fr_raw) alignator(rootTree.get(), gindex.data());

  Hit i_hit;
  Event buffer;

  // Set the temporary tree to reading mode :
  rootTree -> ResetBranchAddresses(); // Allows one to use the tree afterwards
  rootTree -> SetBranchAddress("label"  , &i_hit.label );
  rootTree -> SetBranchAddress("time"   , &i_hit.time  );
  rootTree -> SetBranchAddress("nrjcal" , &i_hit.nrjcal);
  #ifdef QDC2
  rootTree -> SetBranchAddress("nrj2"   , &i_hit.nrj2);
  #endif //QDC2
  rootTree -> SetBranchAddress("pileup" , &i_hit.pileup);
  rootTree -> SetBranchStatus("*",true);

  // Sets the output tree :
  std::unique_ptr<TTree> outTree(new TTree("Nuball", "DataTreeEventBuild C1L2 C2"));
  outTree -> Branch("mult",  &buffer.mult);
  outTree -> Branch("label", &buffer.labels , "label[mult]/s" );
  outTree -> Branch("nrj",   &buffer.nrjs   , "nrj[mult]/F"   );
  #ifdef QDC2
  outTree -> Branch("nrj2",  &buffer.nrj2s  , "nrj2[mult]/F"   );
  #endif //QDC2
  outTree -> Branch("time",  &buffer.times  , "time[mult]/l"  );
  outTree -> Branch("pileup",&buffer.pileups, "pileup[mult]/O");

  gROOT -> cd();

  // Remove the hits previous to the first RF measurement :

  ULong64_t loop = 0;
  RF_Manager rf;
  EventBuilder event(&buffer, &rf);
  if (m_use_RF)
  {
    event.setShift(m_RF_shift);
    do{ rootTree -> GetEntry(gindex[loop++]);}
    while(i_hit.label != RF_Manager::label && loop<nb_data);

    if (loop == nb_data) {print("NO RF DATA FOUND !"); return;}
    rf.setHit(i_hit);
    event.setFirstRF(i_hit);
    // Handle the first RF :
    buffer = i_hit;
    outTree -> Fill();
    buffer.clear();
  }

  // Handle the first hit :
  rootTree -> GetEntry(gindex[loop++]);
  event.set_last_hit(i_hit);

  // Read the following hits :
  // ULong64_t evt_start = loop;
  // while (loop<evt_start+1000)
#ifdef DOWNSCALE_M1
  int M1_counter = 0;
#endif //DOWNSCALE_M1
  while (loop<nb_data)
  {
    rootTree -> GetEntry(gindex[loop++]);
    if (m_fr_keep_all || m_fr_raw)
    {// No event building
      outTree -> Fill();
    }
    else if (m_use_RF)
    {// RF based event building
      if (is_RF(i_hit) && event.status() != 1)
      {// To force RF writting in the data if no event is currently being constructed :
        buffer = i_hit;
        outTree -> Fill();
        buffer.clear();
        rf.setHit(i_hit);
        continue;
      }
      if (event.build(i_hit))
      {
        m_fr_treated_counter[thread_nb]+=event.size();
        Counters arg;
        m_fr_trigger_buffer(buffer, arg);

        bool trig = true;
      #if defined (NO_TRIG)
      #elif defined (G1_trig)
        trig = arg.RawGeMult>0; // At least one Ge
      #elif defined (M2G1_TRIG)
        trig = arg.ModulesMult>1 && arg.RawGeMult>0; // At least one Ge cystal and 2 modules
      #elif defined (D1_TRIG)
        trig = arg.DSSDMult>0; // At least one DSSD
      #elif defined (C1L2_C2_TRIG)
        trig = (arg.CleanGeMult>0 && arg.LaBr3Mult>1) || arg.CleanGeMult>1; // At least one clean Ge and 2 LaBr3, or two clean Ge
      #elif defined (CG1L2_CG2_TRIG)
        trig = (arg.CloverGeMult>0 && arg.LaBr3Mult>1) || arg.CloverGeMult>1; // At least one Clover Ge and 2 LaBr3, or two Clover Ge
      #endif //TRIGGER

      #ifdef DOWNSCALE_M1
        if(arg.ModulesMult==1) M1_counter++;
        if (M1_counter>1000)
        {
          trig = true;
          M1_counter = 0;
        }
      #endif //DOWNSCALE_M1

        if ( trig )
        {
          // buffer.resize(); // if the containers in Event class are vector, need to resize to the actual event size before writting down
          outTree -> Fill();
          rf.setHit(i_hit);
        }
        else if (event.hasRF())
        {// If the event didn't pass the trigger we still want to write any potential RF in the event
          for (size_t i = 0; i<buffer.size(); i++)
          {
            if (buffer.labels[i]==251)
            {
              // print(buffer.nrjs[i]);
              buffer = buffer[i];
              outTree -> Fill();
              rf.setHit(i_hit);
              buffer.clear();
              continue;
            }
          }
        }
        if (event.getLastHit().label == 251)
        {
          buffer = event.getLastHit();
          outTree -> Fill();
          rf.setHit(i_hit);
          buffer.clear();
          continue;
        }
        event.reset();
      }
    }
  }

  std::unique_ptr<TFile> outFile (new TFile(outfile.c_str(),"create"));
  print("Writting",outfile);
  outFile -> cd();
  outTree -> Write();
  outFile -> Close();
  m_fr_treated_run_size[thread_nb]+=size_file(outfile);

  auto end = std::chrono::high_resolution_clock::now();
  auto dT = std::chrono::duration<double, std::milli>(end - start).count();
  std::cout << std::setprecision(3) << "Conversion of " << rmPathAndExt(filename) << " done in " << dT/1000. << " s"
  << " - " << nb_data*1000./dT << " counts/s"
  << " (" << (int)(file_size/dT)/1000. << " MB/s)"
  << std::endl;
}

Bool_t NearLine::processFile(std::string filename, int thread_nb)
{
  Hit hit;
  // ________________________________ //
  // ________________________________ //
  //                                  //
  //      READ THE FASTER FILE        //
  // ________________________________ //
  // ________________________________ //

  // ================================= //
  //   °°°   INITIALIZE Reader   °°°   //
  FasterReader reader(&hit, filename);
  if (!reader.isReady()) return false;
  // ================================= //

  auto start = std::chrono::high_resolution_clock::now();

  // ===================================== //
  //   °°°   INITIALIZE Temp TTree   °°°   //
  std::unique_ptr<TFile> tempFile;
  if (temp_tree_on_disk) tempFile.reset(new TFile(("analysisTempTrees/tempTree"+std::to_string(thread_nb)).c_str(), "recreate"));
  std::unique_ptr<TTree> rootTree;

  auto file_size = size_file(filename);
  if (m_a||m_aw)
  {
    rootTree.reset (new TTree(("tempTree"+std::to_string(thread_nb)).c_str(), ("tempTree"+std::to_string(thread_nb)).c_str()));
    rootTree -> Branch("label"  , &hit.label );
    rootTree -> Branch("time"   , &hit.time  );
    rootTree -> Branch("pileup" , &hit.pileup);
    rootTree -> Branch("nrjcal" , &hit.nrjcal);
  }
  // ===================================== //

  // ============================ //
  //   °°°   PROCESS DATA   °°°   //
  int counter = 0; int rejected = 0;
  Event ts_buffer;
  CoincBuilder2 ts_coinc(&ts_buffer, m_ts_timeWindow);
  Buffer rfc_buffer(0);
  CoincBuilder rfc_coinc(&rfc_buffer, m_rfc_timeWindow);
  // ULong64_t time_init;
  RF_Manager rf;
  Long64_t m_rf_period_test = 399998000;
  size_t counter_rf = 0;
  // while(reader.Read() && counter<5000000)
  // while(reader.Read() && counter<100)
  while(reader.Read())
  {
    counter++;
    #ifdef PARIS
    if (!m_pb && isParis[hit.label] && hit.gate_ratio() > 0.35) continue;
    #endif //PARIS

    // RAW SPECTRA //
    if (m_hr && hit.label<m_hr_histo.size() && m_hr_histo[hit.label].exists())  m_hr_histo[hit.label][thread_nb] -> Fill(hit.nrj);

    // Align the timestamp :
    if (m_apply_timeshift) time_shift (hit);

    //Calibrate the hit :
    if (m_apply_calibration || m_hc)
    {
      m_calib.calibrate(hit);
      // Reject hit if the calibrated energy is too low.
      if (m_use_threshold && hit.nrjcal < m_E_threshold) {rejected ++; continue;}
    }
    else if (m_ts) hit.nrjcal = hit.nrj;

    if(m_pb)
    {
      if (m_use_RF && is_RF(hit.label)) {rf.last_hit = hit.time; rf.period = hit.nrj;}
      m_pb->Fill(hit,rf,thread_nb);
    }

    // TIMESHIFT //
    if (m_ts)
    {
      if (m_use_RF)
      {
        if(is_RF(hit.label)) {rf.last_hit = hit.time; rf.period = hit.nrj;}
        else if (hit.label == m_ts_time_ref.label) m_ts_histo_RF[thread_nb] -> Fill(rf.pulse_ToF(hit.time));
      }
      if (isDSSD[hit.label] && m_use_RF)
      {
        m_ts_histo[hit.label][thread_nb]->Fill(rf.pulse_ToF(hit.time,m_RF_shift));
      }
      else if (ts_coinc.build(hit))
      {
        for (size_t i = 0; i < ts_buffer.size(); i++)
        {
          if ( ts_buffer.labels[i] == m_ts_time_ref.label ) // if the ref detector triggered
          {
            m_ts_Fill(ts_buffer, i, thread_nb);
            break;
          }
        }
      }
    }//END TIMESHIFT

    // CALIBRATED SPECTRA //
    if (m_hc && m_hc_histo[hit.label].exists())
    {
      m_hc_histo[hit.label][thread_nb] -> Fill(hit.nrjcal);
      if (isGe[hit.label]) m_hc_bidim_histo_Ge[thread_nb] -> Fill (hit.label, hit.nrjcal);
      else if (isLaBr3[hit.label]) m_hc_bidim_histo_LaBr3[thread_nb] -> Fill (hit.label, hit.nrjcal);
      else if (isParis[hit.label]) m_hc_bidim_histo_Paris[thread_nb] -> Fill (hit.label, hit.nrjcal);
    }

    // RF CHECK //
    if (m_rfc)
    {
      if (is_RF(hit.label))
      {
        if (counter_rf>0)
        {
          m_rf_period_test = hit.time-rf.last_hit;
          // print(hit.time,m_rfc_RF_shift*1000,rf.last_hit,hit.time+m_rfc_RF_shift*1000-rf.last_hit, "%", m_rf_period_test);
          Long64_t deltaT_RF = rf.pulse_ToF(hit,m_rfc_RF_shift);
          // Long64_t deltaT_RF = (((Long64_t)(hit.time+m_rfc_RF_shift-rf.last_hit)%rf.period)-m_rfc_RF_shift);
          m_rfc_histo_RF_test[thread_nb] -> Fill(deltaT_RF/_ns);
          // print(deltaT_RF);
          m_rfc_period[thread_nb] -> Fill(m_rf_period_test);
          // print(m_rf_period_test, deltaT_RF);
        }
        rf.last_hit = hit.time;
        rf.period = hit.nrj;
        counter_rf++;
      }
      m_rfc_RF_label[thread_nb] -> Fill(rf.pulse_ToF(hit.time, m_rfc_RF_shift)/_ns, hit.label);
      if (hit.label == m_rfc_time_ref_label)
      {
        m_rfc_histo_RF[thread_nb] -> Fill(rf.pulse_ToF(hit.time, m_rfc_RF_shift)/_ns);
      }
    }

    // EARLY ANALYSIS //
    if (m_a||m_aw) rootTree -> Fill();
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto dT = std::chrono::duration<double, std::milli>(end - start).count();
  std::cout << rmPathAndExt(filename) << std::setprecision(3) << " : " << (Float_t) counter << " hits ";
  if (rejected>1) std::cout << "(" << procent(rejected, counter) << " rejected) ";
  std::cout
  << "read in " << dT/1000 << " s - "
  << (counter/(dT/1000)) << " counts/s ("
  << (int)(file_size/dT)/1000. << " MB/s)"
  << std::endl;

  // ________________________________ //
  // ________________________________ //
  //                                  //
  //  LOOP OVER THE LOADED ROOT FILE  //
  // ________________________________ //
  // ________________________________ //

  if (m_a)
  {
    // Performances :
    auto start_ = std::chrono::high_resolution_clock::now();

    // Initialization :
    ULong64_t nb_data = rootTree -> GetEntries();
    std::vector<Int_t> gindex(nb_data);
    for (size_t nb = 0; nb<gindex.size(); nb++) gindex[nb] = nb;

      // Realignement of the time shifted data :
    alignator(rootTree.get(), gindex.data()); // test_alignator(rootTree.get(), gindex.data(), 1);


    std::cout << "Data reordered, starting analysis" << std::endl;

    // RF managing :
    RF_Manager rf;
    m_a -> setRF(&rf, thread_nb);

    // Event building :
    Event coinc_buffer;
    CoincBuilder2  coinc(&coinc_buffer, m_a->timeWindow());
    Event event_buffer;
    EventBuilder  event(&event_buffer, &rf);
    // EventBuilder  event(&event_buffer);
    Hit i_hit;
    //Configure the tree to be read :
    if (!m_evtbuilt) rootTree -> ResetBranchAddresses();
    rootTree -> SetBranchAddress("label"  , &i_hit.label );
    rootTree -> SetBranchAddress("time"   , &i_hit.time  );
    rootTree -> SetBranchAddress("nrjcal" , &i_hit.nrjcal);
    rootTree -> SetBranchStatus("*",true);

    ULong64_t loop = 0;
    if (m_use_RF)
    {
      do //Remove the hits previous to the first RF measurement
      {
        m_counter++;
        rootTree -> GetEntry(gindex[loop]);
        loop++;
      } while(i_hit.label != RF_Manager::label && loop<nb_data);
      if (loop == nb_data) {print("NO RF DATA FOUND !"); return -1;}
      // print("Start at event n°", loop, "(",gindex[loop],")");
      event.setFirstRF(i_hit);
    }
    rootTree -> GetEntry(gindex[loop++]);
    coinc.set_last_hit(i_hit);
    event.set_last_hit(i_hit);

    // Long64_t prompt_min = -20000ll;
    // Long64_t prompt_max = 40000ll;
    // print(loop/(nb_data/100.), "%");
    // ULong64_t start_read = loop;
    // while (loop<start_read+1000)
    // Long64_t t = i_hit.time;
    while (loop<nb_data)
    {
      // Extracts the hit
      rootTree -> GetEntry(gindex[loop++]);
      // print(loop);
      // print ("status : ", (int)coinc.status());
      // print(i_hit);
      // print("delta T = ", (i_hit.time-t)/_ns);
      // t=i_hit.time;
      // if (loop%(nb_data/10) == 0) print("Treated", 100*loop/nb_data,"%");
      m_a -> FillRaw(i_hit, thread_nb);
      if (m_use_RF)
      {// RF based event building
        if (m_a)
        {
          m_a->FillTiming(i_hit, thread_nb);
          if (event.build(i_hit))
          {
            m_a->Fill(event_buffer, thread_nb);
          }
          else if (event.isSingle())
          {//Manage single hits
            m_a->Fill(event.singleHit(), thread_nb);
          }
          rf.setHit(i_hit);
        }
      }
      else
      {// Coincidence based event building
        if (m_a)
        {
          if (coinc.build(i_hit))
          {
            m_a->Fill(coinc_buffer, thread_nb);
          }
          else if (coinc.isSingle())
          {// Manage single hits
            m_a->Fill(coinc.getSingleEvent(), thread_nb);
          }
        }
      }
    }// End hits loop

    auto end_ = std::chrono::high_resolution_clock::now();
    auto dT_ = std::chrono::duration<double, std::milli>(end_ - start_).count();
    std::cout << "Analysis of reordered " << rmPathAndExt(filename) << " done in " << dT_/1000. << " s" << std::endl;
  }

  else if (m_aw)
  {
    // Performances :
    auto start_ = std::chrono::high_resolution_clock::now();

    // Initialization :
    ULong64_t nb_data = rootTree -> GetEntries();
    std::vector<Int_t> gindex(nb_data);
    for (size_t nb = 0; nb<gindex.size(); nb++) gindex[nb] = nb;

      // Realignement of the time shifted data :
    alignator(rootTree.get(), gindex.data());

    std::cout << "Data reordered, starting analysis" << std::endl;

    // Event building :
    Event coinc_buffer;
    CoincBuilder2  coinc(&coinc_buffer, m_aw->timeWindow());
    Hit i_hit;
    //Configure the tree to be read :
    if (!m_evtbuilt) rootTree -> ResetBranchAddresses();
    rootTree -> SetBranchAddress("label"  , &i_hit.label );
    rootTree -> SetBranchAddress("time"   , &i_hit.time  );
    rootTree -> SetBranchAddress("nrjcal" , &i_hit.nrjcal);
    rootTree -> SetBranchStatus("*",true);

    ULong64_t loop = 0;
    rootTree -> GetEntry(gindex[loop++]);
    coinc.set_last_hit(i_hit);

    while (loop<nb_data)
    {
      // Extracts the hit
      rootTree -> GetEntry(gindex[loop++]);
      m_aw -> FillRaw(i_hit, thread_nb);
      if (coinc.build(i_hit))
      {
        m_aw->Fill(coinc_buffer, thread_nb);
      }
      else if (coinc.isSingle())
      {// Manage single hits
        m_aw->Fill(coinc.getSingleEvent(), thread_nb);
      }
    }// End hits loop

    auto end_ = std::chrono::high_resolution_clock::now();
    auto dT_ = std::chrono::duration<double, std::milli>(end_ - start_).count();
    std::cout << "Analysis of reordered " << rmPathAndExt(filename) << " done in " << dT_/1000. << " s" << std::endl;
  }

  return true;
}

void NearLine::m_hr_Initialize()
{
  if (m_hr)
  {
    m_hr_histo.resize(m_labelToName.size());
    UShort_t l=0;
    Detector type = null;
    for (auto name : m_labelToName)
    {
      type = type_det(l);
      if (type) m_hr_histo[l].reset((name+"_raw").c_str(), name.c_str(), m_bins_raw[type],m_min_raw[type], m_max_raw[type]  );
      else m_hr_histo[l].reset(nullptr);
      l++;
    }
  }
}

void NearLine::m_fr_Initialize()
{
  m_fr_raw_run_size.init();
  m_fr_treated_run_size.init();
  m_fr_raw_counter.init();
  m_fr_treated_counter.init();
  // m_use_threshold = false;
  // m_fr_Ge_raw.reset("Raw spectra","Raw spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_prompt.reset("Prompt spectra","Prompt spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_addback.reset("Addback spectra","Addback spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_compton.reset("Compton spectra","Compton spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M1.reset("M1 spectra","M1 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M2.reset("M2 spectra","M2 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M3.reset("M3 spectra","M3 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M4.reset("M4 spectra","M4 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M5.reset("M5 spectra","M5 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_M6.reset("M6 spectra","M6 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_Ge_Msup6.reset("M>6 spectra","M>6 spectra",m_bins_calib[Ge],m_min_calib[Ge],m_max_calib[Ge]) ;
  // m_fr_M.reset("Multiplicity","Multiplicity M", 51, 0, 50) ;
  // #if defined FATIMA
  // m_fr_C_VS_L.reset("C VS L","Clean Germanium Multiplicity VS LaBr3 Multiplicity", 5,0,5, 5,0,5);
  // #elif defined PARIS
  // m_fr_C_VS_L.reset("C VS L","Clean Germanium Multiplicity VS Paris Multiplicity", 5,0,5, 5,0,5);
  // #endif //PARIS
  // m_fr_ToF_all_det.reset("ToF all detectors","ToF all detectors", 5000,-200,400, m_labelToName.size()+1,0,m_labelToName.size()) ;
  // m_fr_ToF.reset("ToF", "ToF", 2500, -100, 400);
  // m_fr_dT_pulse.reset("time VS pulse ref", "time VS pulse ref", 2500, -100, 400);
  // m_fr_C_VS_dT_pulse.reset("mult VS pulse ref", "mult VS pulse ref", 600,-200,400, 21,0,20);
}

void NearLine::m_hc_Initialize()
{
  if (m_hc)
  {
    m_hc_histo.resize(m_labelToName.size());
    UShort_t l=0;
    Detector type = null;
    for (auto name : m_labelToName)
    {
      type = type_det(l);
      if( (type == Ge && m_hc_Ge_Clover) || (type == Paris && m_hc_Paris) || (type == LaBr3 && m_hc_LaBr3) || (type == DSSD && m_hc_DSSD))
        m_hc_histo[l].reset((name+"_calib").c_str(), name.c_str(), m_bins_calib[type],m_min_calib[type],m_max_calib[type]);
      l++;
    }

    // m_hc_bidim_histo_Ge -> GetYaxis() -> Set(m_bins_bidim[Ge],m_min_bidim[Ge],m_max_bidim[Ge]);
    // m_hc_bidim_histo_Ge -> GetXaxis() -> Set(Ge_Labels.size()-1,Ge_Labels.data());
    // m_hc_bidim_histo_Ge -> GetXaxis() -> SetCanExtend();
    // m_hc_bidim_histo_Ge -> GetXaxis() -> SetAlphanumeric();
    // for (size_t i = 0; i<Ge_Labels.size(); i++)
    // {
    //
    // }
    m_hc_bidim_histo_Ge.reset("All Clover Germanium calibrated spectra", "All Ge Clover Spectra calibrated spectra", Ge_Labels.size()-1,Ge_Labels.data(), m_bins_bidim[Ge],m_min_bidim[Ge],m_max_bidim[Ge]);
    #ifdef FATIMA
    m_hc_bidim_histo_LaBr3.reset("All FATIMA LaBr3 calibrated spectra", "All FATIMA LaBr3 calibrated spectra", LaBr3_Labels.size()-1,LaBr3_Labels.data(), m_bins_bidim[LaBr3],m_min_bidim[LaBr3],m_max_bidim[LaBr3]);
    #endif //FATIMA
    #ifdef PARIS
    m_hc_bidim_histo_Paris.reset("All PARIS calibrated spectra", "All PARIS calibrated spectra", Paris_Labels.size()-1,Paris_Labels.data(), m_bins_bidim[Paris],m_min_bidim[Paris],m_max_bidim[Paris]);
    #endif //PARIS
  }
}

void NearLine::m_ts_Initialize()
{
  if (m_ts)
  {
    m_ts_histo.resize(m_labelToName.size()); for (size_t i = 0; i<m_ts_histo.size(); i++) m_ts_histo[i].reset(nullptr);
    m_ts_E_VS_deltaT.resize(m_labelToName.size()); for (size_t i = 0; i<m_ts_E_VS_deltaT.size(); i++) m_ts_E_VS_deltaT[i].reset(nullptr);
    Detector type_ref = type_det(m_ts_time_ref.label);
    m_ts_DeltaT       . reset ("deltaT all detectors","Timeshifts all detectors together", m_ts_timeWindow/100, -1.1*m_ts_timeWindow/2, 1.1*m_ts_timeWindow/2);
    m_ts_label_DeltaT . reset ("deltaT each detector","Timeshifts each detector", m_ts_timeWindow/100,-m_ts_timeWindow/2,m_ts_timeWindow/2, m_labelToName.size()+1,0lu,m_labelToName.size());
    m_ts_EnergyRef    . reset ("Spectra ref detector","Energy spectra of reference detector", m_bins_raw[type_ref], m_min_raw[type_ref], m_max_raw[type_ref]);

    m_ts_Label        . reset ("ts_label","label", 300,0,300);
    m_ts_mult         . reset ("ts_mult","multiplicity", 50,0,50);
    Detector type = null;
    for (UShort_t l = 0; l<m_labelToName.size(); l++)
    {
      std::string const & name = m_labelToName[l];
      type = Type_det[l];
      if (type == RF)
      {
        m_ts_histo_RF.reset( (name+"_calculated").c_str(), (name+"_calculated").c_str(), m_ts_timeWindow/m_ts_rebin[RF], -m_ts_timeWindow/2, m_ts_timeWindow/2);
      }
      if (type != null)
      {
        m_ts_histo[l].reset(name.c_str(), name.c_str(), m_ts_timeWindow/m_ts_rebin[type], -m_ts_timeWindow/2, m_ts_timeWindow/2);

        if ( m_ts_bidim )
        {
          if (m_ts_calibrated)  m_ts_E_VS_deltaT[l].reset((name+"_E_deltaT").c_str(), (name+"_E_deltaT").c_str(),
          m_ts_timeWindow/m_ts_rebin[type], -m_ts_timeWindow/2, m_ts_timeWindow/2, 200, m_min_bidim[type], m_max_bidim[type]) ;

          else                  m_ts_E_VS_deltaT[l].reset((name+"_E_deltaT").c_str(), (name+"_E_deltaT").c_str(),
          m_ts_timeWindow/m_ts_rebin[type], -m_ts_timeWindow/2, m_ts_timeWindow/2, 200, m_min_raw[type]  , m_max_raw[type]  ) ;
        }
      }
    }
  }
}

void NearLine::m_rfc_Initialize()
{
  m_rfc_histo_RF.reset( ("ToF VS "+m_rfc_time_ref_det).c_str(), ("RF VS "+m_rfc_time_ref_det).c_str(), 5000, -50, 450);
  m_rfc_histo_RF_test.reset( ("test RF VS "+m_rfc_time_ref_det).c_str(), ("test RF VS "+m_rfc_time_ref_det).c_str(), 1000, -5, 5);
  m_rfc_period.reset( ("Period "+m_rfc_time_ref_det).c_str(), ("Period "+m_rfc_time_ref_det).c_str(), 10000, 399997, 400000);
  m_rfc_RF_label.reset("ToF each detector","ToF each detector",1000,-100,500, 801,0,800);
  if (p_Files.getListFolders().size() == 1 && p_Files.size() > m_rfc_nbFiles)
  {
    ListFiles files;
    files.resize(m_rfc_nbFiles);
    p_Files.Print();
    for (int i = 1; i<m_rfc_nbFiles; i++)
    {
      print(i, files[i]);
      files[i] = p_Files[p_Files.size()-i];
    }
    p_Files.setListFiles(files);
  }
  p_Files.Print();
}

Bool_t NearLine::Initialize()
{
  if (!folder_exists(m_outdir))
  {
    print("Creating output directory ",m_outdir);
    if (gSystem -> Exec((std::string("mkdir ")+m_outdir).c_str())) return false;
  }
  if (temp_tree_on_disk) if (!folder_exists("analysisTempTrees/")) gSystem -> Exec("mkdir analysisTempTrees/");
  TThread::Initialize();
  srand(time(0));

  //   ARRAYS   //
  setArrays(m_labelToName.size());
  set_type_det(m_labelToName);
  // m_calib.setCalibrationTables();

  // Detectors :
  // #ifdef USE_DSSD
  // dssd = new DSSD();
  // dssd-> set(this);
  // #endif //USE_DSSD

    //   HISTOGRAMS INITIALIZE   //

    // Raw Histo //
    if (m_hr) m_hr_Initialize();

    // Calibrated spectra //
    if (m_hc) m_hc_Initialize();

    // TIMESHIFT //
    if (m_ts) m_ts_Initialize();

    // Analysis //
    // if (m_a) m_a_Initialize();
    if (m_a) m_a->Initialize();

    if (m_aw) m_aw->Initialize();

    if (m_fr) m_fr_Initialize();

    if (m_pb) m_pb->Initialize();

    if (m_rfc) m_rfc_Initialize();

    return true;
}

Bool_t NearLine::configOk()
{
  // ******************************** //
  //   ---   Checking threads   ---   //
  m_nb_threads = checkThreads(m_nb_threads, p_Files.size());
  MTObject::nb_threads = m_nb_threads;
  // ******************************** //

  // *************************************** //
  //    ---    CHECKING PARAMETERS    ---    //
  if (m_labelToName.size()==0) {std::cout << "ERROR : NO ID FOUND, check " << m_parameters_filename << std::endl; return false;}
  if (m_hr)
  {
    if (!folder_exists(getPath(m_outdir+m_hr_outroot))) {print("ERROR, OUT FOLDER DOESN'T EXISTS ! ",getPath(m_outdir+m_hr_outroot)); return false;}
    if (m_hr_outroot=="")  {std::cout << "ERROR : NO OUT FILE NAME, check " << m_parameters_filename << std::endl; return false;}
  }
  if (m_hc)
  {
    if (!folder_exists(getPath(m_outdir+m_hc_outroot))) {print("ERROR, OUT FOLDER DOESN'T EXISTS ! ", getPath(m_outdir+m_hc_outroot)); return false;}
    if (m_hc_outroot=="") {std::cout << "ERROR : NO OUT FILE NAME, check " << m_parameters_filename << std::endl; return false;}
    if (!m_hc_LaBr3 && !m_hc_Ge_Clover && !m_hc_Paris && !m_hc_DSSD) {std::cout << "ERROR : NO SPECTRA OUT, check " << m_parameters_filename << std::endl; return false;}
    if (m_calib.size() == 0) {std::cout << "ERROR : NO CALIBRATION DATA, check " << m_parameters_filename << std::endl; return false;}
  }
  if (m_ca)
  {
    if (!folder_exists(getPath(m_outdir+m_ca_outDir+m_ca_outRoot)) && m_ca_outRoot_b ) {print("ERROR, OUT FOLDER DOESN'T EXISTS ! ", getPath(m_outdir+m_ca_outDir+m_ca_outRoot)); return false;}
    if (!m_hr) {std::cout << "INTERNAL ERROR, call dev ;)" << std::endl; return false;}
    if (!(m_ca_source=="152Eu" || m_ca_source=="232Th" || m_ca_source=="60Co" || isTripleAlpha(m_ca_source) || m_ca_source=="")) { std::cout << "Fatal error : unkown source..." << std::endl; return false; }
    if (m_ca_outRoot=="") {std::cout << "No root output" << std::endl; return false;}
    if (m_ca_residus && (m_a || m_aw || m_ts || m_fr)) {print("Residus calculations must stand alone"); return false;}
  }
  if ((m_a && m_ca) || (m_a && m_ts)) {std::cout << "CANNOT CALIBRATE and ANALYSE !!" << std::endl; return false;}

  if ((m_aw && m_ca) || (m_aw && m_ts)) {std::cout << "CANNOT CALIBRATE and ANALYSE !!" << std::endl; return false;}

  if (m_ts)
  {
    if (!folder_exists(m_outdir+m_ts_outdir)) {print("ERROR, OUT FOLDER DOESN'T EXISTS ! ", m_outdir+m_ts_outdir);return false;}
  }

  if (m_a && !m_a->Check()) return false;

  if (m_aw && !m_aw->Check()) return false;

  if (m_fr)
  {
    if (!m_fr_raw)
    {
      if (!m_calib.isFilled()) {std::cout << "CALIBRATION NOT LOADED !!"; return false;}
      if (m_timeshifts.size()<10 && m_fr_eventbuild) {std::cout << "NO TIMESHIFT DATA" << std::endl; return false;}
    }
    if (m_fr_throw_singles && !m_fr_eventbuild) {std::cout << "CAN'T THROW SINGLES WITHOUT EVENTBUILDING" << std::endl; return false;}
  }

  if (m_rfc)
  {
    if (!folder_exists(getPath(m_outdir+m_rfc_outroot))) {print("ERROR, OUT FOLDER DOESN'T EXISTS !"); return false;}
    if (m_timeshifts.size()==0) {std::cout << "CANT APPLY TIMESHIFT, TIMESHIFT DATA SEEMS EMPTY" << std::endl; return false;}
  }

  if (!(m_a || m_aw || m_ca || m_hc || m_hr || m_ts || m_fr || m_rfc || m_pb)) {std::cout << "ERROR : NO OUTPUT ASKED FOR !!! check " << m_parameters_filename << std::endl; return false;}

  if (m_apply_timeshift && m_timeshifts.size()<10) {std::cout << "CANT APPLY TIMESHIFT, TIMESHIFT DATA SEEMS EMPTY (<10 entries)" << std::endl; return false;}

  std::cout << std::endl << "Parameters seems okay, let's roll !" << std::endl << std::endl;

  //TO MAKE SURE EVERYTHING IS IN PLACE :
  if (m_a || m_aw || m_fr || m_rfc) m_apply_timeshift = true;
  if (m_a || m_aw || m_hc || m_ts_calibrated) m_apply_calibration = true;

  return true;
}

void NearLine::m_hr_Write()
{
  TFile* outFile (TFile::Open((m_outdir+m_hr_outroot).c_str(),"recreate"));
  outFile->cd();
  int l = 0;
  for (auto name : m_labelToName)
  {
    m_hr_histo[l].Write();
    l++;
  }
  std::cout << "Raw histograms written to " << m_outdir+m_hr_outroot << std::endl;
  outFile -> Write();
  outFile -> Close();
  delete outFile;
}

void NearLine::m_hc_Write()
{
  TH1F* sum_Ge_Clover = new TH1F("sum_Ge_Clover","sum_Ge_Clover", m_bins_calib[Ge], m_min_calib[Ge], m_max_calib[Ge]);
  TH1F* sum_LaBr3 = new TH1F("sum_LaBr3","sum_LaBr3", m_bins_calib[LaBr3], m_min_calib[LaBr3], m_max_calib[LaBr3]);
  TH1F* sum_Paris = new TH1F("Sum_Paris","Sum_Paris", m_bins_calib[Paris], m_min_calib[Paris], m_max_calib[Paris]);

  TFile* outFile = TFile::Open((m_outdir+m_hc_outroot).c_str(),"recreate");
  outFile->cd();
  int l = 0; Detector type = null;

  m_hc_bidim_histo_Ge.Write();
  m_hc_bidim_histo_LaBr3.Write();
  m_hc_bidim_histo_Paris.Write();

  for (auto name : m_labelToName)
  {
    type = type_det(l);
    m_hc_histo[l].Merge();
    if ( type && m_hc_histo[l].exists())
    {
      m_hc_histo[l].Write();
           if (m_hc_Ge_Clover && type==Ge) sum_Ge_Clover->Add(m_hc_histo[l].Merged());
      else if (m_hc_LaBr3 && type==LaBr3)  sum_LaBr3->Add(m_hc_histo[l].Merged());
      else if (m_hc_Paris && type==Paris)  sum_Paris->Add(m_hc_histo[l].Merged());
    }
    l++;
  }
  if(m_hc_Ge_Clover) sum_Ge_Clover->Write();
  if(m_hc_LaBr3) sum_LaBr3->Write();
  if(m_hc_Paris) sum_Paris->Write();
  std::cout << "Calibrated histograms written to " << m_outdir+m_hc_outroot << std::endl;
  outFile -> Write();
  outFile -> Close();
  delete outFile;
}

void NearLine::WriteData()
{
  //   ---   HISTOGRAMS WRITING   ---   //
  // Raw Histo //
  if (m_hr)
  {
    m_hr_Write();
  }
  // Calibrated Spectra //
  if (m_hc)
  {
    m_hc_Write();
  }
  // Timeshifts //
  if (m_ts)
  {
    m_ts_calculate();
  }
  // Analysis Spectra //
  if (m_a)
  {
    m_a->Write();
  }

  if (m_aw)
  {
    m_aw -> Calculate();
    m_aw -> Write();
  }
  // Calibration file or residues calculations //
  if (m_ca)
  {
    m_ca_Write(m_hr_outroot);
    if (m_ca_residus) m_ca_residus_calculate();
  }
  // if (m_fr) m_fr_Write();
  if(m_fr) m_fr_sum_counters();
  if (m_pb)
  {
    m_pb->Write();
    delete m_pb;
  }
  if (m_rfc)
  {
    m_rfc_Write();
  }
}

void NearLine::m_fr_sum_counters()
{
  m_fr_raw_run_size.Merge();
  m_fr_treated_run_size.Merge();
  m_fr_raw_counter.Merge();
  m_fr_treated_counter.Merge();

  std::string run_name = p_Files.getListFolders()[0];
  run_name.pop_back();
  run_name = rmPathAndExt(run_name);
  std::ofstream outfile("log.log",std::ios::app);
  print(run_name);
  outfile << run_name << ": CompressionFactor: " << m_fr_raw_run_size/m_fr_treated_run_size << " RawCounter: " << m_fr_raw_counter
  << " TreatedCounter: " << m_fr_treated_counter << std::endl;
  outfile.close();
}

void NearLine::m_fr_trigger_buffer(Event const & event, Counters & arg)
{
  arg.BGO.fill(false);
  arg.Ge.fill(false);
  for (unsigned char i = 0; i<event.mult; i++)
  {
    const Label& label = event.labels[i];
    if(isGe[label])
    {
      arg.RawGeMult++;
      arg.Ge [labelToClover_fast[label]] = true;
    }
    else if (isBGO[label])
    {
      arg.BGO[labelToClover_fast[label]] = true;
    }
    else if (isLaBr3[label])
    {
      #ifdef FATIMA
      arg.LaBr3Mult++;
      arg.ModulesMult++;
      #endif //FATIMA
    }
    else if (isParis[label])
    {
      #ifdef PARIS
      arg.ParisMult++;
      arg.ModulesMult++;
      #endif //PARIS
    }
    else if (isDSSD[label])
    {
      #ifdef USE_DSSD
      arg.DSSDMult++;
      #endif //USE_DSSD
    }
  }

  //Compton-rejection :
  for (unsigned char i = 0; i<24; i++)
  {
    if(arg.BGO[i] || arg.Ge[i]) arg.ModulesMult++;
    if(arg.Ge[i])
    {
      if(!arg.BGO[i])
      {
        arg.CleanGeMult++;
      }
    }
  }
}

void NearLine::m_fr_Write()
{
  print("Writing to faster2root_histo.root");
  TFile* file (TFile::Open("faster2root_histo.root","recreate"));
  file -> cd();
  m_fr_Ge_raw.Write();
  m_fr_Ge_prompt.Write();
  m_fr_Ge_addback.Write();
  m_fr_Ge_compton.Write();
  m_fr_Ge_M1.Write();
  m_fr_Ge_M2.Write();
  m_fr_Ge_M3.Write();
  m_fr_Ge_M4.Write();
  m_fr_Ge_M5.Write();
  m_fr_Ge_M6.Write();
  m_fr_Ge_Msup6.Write();
  m_fr_M.Write();
  m_fr_C_VS_L.Write();
  m_fr_ToF_all_det.Write();
  m_fr_ToF.Write();
  m_fr_dT_pulse.Write();
  m_fr_C_VS_dT_pulse.Write();
  file -> Write();
  file -> Close();
  delete file;
}

void NearLine::m_rfc_Write()
{
  TFile* outFile = TFile::Open((m_outdir+m_rfc_outroot).c_str(),"recreate");
  m_rfc_histo_RF.Write();
  m_rfc_histo_RF_test.Write();
  m_rfc_period.Write();
  m_rfc_RF_label.Write();
  outFile->Write();
  outFile->Close();
  delete outFile;
  print("RF checking root file written to",m_outdir+m_rfc_outroot);
}

void NearLine::m_ts_Fill(Event const & ts_buffer, size_t const & refPos, UShort_t const & thread_nb)
{
  Long64_t deltaT = 0;
  m_ts_mult.Get(thread_nb) -> Fill(ts_buffer.size());
  if (ts_buffer.size() >= m_ts_max_mult &&  ts_buffer.size() <= m_ts_min_mult) return;
  m_ts_EnergyRef.Get(thread_nb) -> Fill(ts_buffer.nrjs[refPos]);
  for (size_t i = 0; i<ts_buffer.size(); i++)
  {
    if (ts_buffer.labels[i] == m_ts_time_ref.label) continue; // To reject the time spectra of the time reference
    deltaT = ts_buffer.times[refPos]-ts_buffer.times[i];
    m_ts_Label[thread_nb] -> Fill(ts_buffer.labels[i]);// label number
    m_ts_DeltaT[thread_nb] -> Fill(deltaT); // stored in ps
    m_ts_label_DeltaT[thread_nb] -> Fill(deltaT, ts_buffer.labels[i]); // stored in ps
    if (m_ts_histo[ts_buffer.labels[i]].exists()) m_ts_histo[ts_buffer.labels[i]][thread_nb]->Fill(deltaT); //stored in ps
    if (m_ts_bidim)
      if (m_ts_E_VS_deltaT[ts_buffer.labels[i]].exists())
      {// Energy VS deltaT bidim :
        m_ts_E_VS_deltaT[ts_buffer.labels[i]][thread_nb]->Fill(deltaT, ts_buffer.nrjs[i]); //stored in ps, ADC
      }
  }
}

void NearLine::m_ts_calculate()
{
  Double_t pospic, amppic, dump_sigma;
  Float_t cte, Mean, sigma;
  std::unique_ptr<TF1> gaus_pol0, fittedPic;
  std::vector<Float_t> array_labels(m_labelToName.size());
  for (size_t i = 0; i<array_labels.size(); i++) array_labels[i] = i;
  std::vector<Float_t> array_resolutions(m_labelToName.size(),0);
  Detector type = null;
  std::ofstream outDeltaTfile(m_outdir+m_ts_outdir+m_ts_outdata, std::ios::out);
  // Calculate the RF timeshift first :
  m_ts_histo_RF.Merge();
  Long64_t deltaT_RF = ( m_ts_histo_RF->GetMaximumBin() - (m_ts_histo_RF -> GetNbinsX()/2) ) * m_ts_rebin[RF];
  for (size_t it = 0; it<m_ts_histo.size(); it++)
  {
    type = type_det(it);
    m_ts_histo[it].Merge();
    if (!m_ts_histo[it].exists()) continue; // Eliminate empty histograms
    if (m_ts_verbose) print(m_labelToName[it]);
    if (type == RF)
    {
      if (m_ts_verbose) print("RF :",deltaT_RF,"with",m_ts_histo_RF->GetMaximum(),"bins in peak");
      outDeltaTfile << it << "\t" << deltaT_RF << std::endl;
      continue;
    }
    #ifdef LICORNE
    if (type == EDEN)
    {
      outDeltaTfile << it << "\t" << 0;
      continue;
    }
    #endif //LICORNE
    #ifdef USE_DSSD
    if (type == DSSD)
    {
      if (m_use_RF)
      {
        amppic = m_ts_histo[it] -> GetMaximum();
        Float_t zero = ( m_ts_histo[it] -> FindFirstBinAbove(amppic/2) - (m_ts_histo[it] -> GetNbinsX()/2) ) * m_ts_rebin[DSSD] ;
        // print("amppic :", amppic, "zero :", zero, "deltaT_RF", deltaT_RF, "RF-zero = ", zero - deltaT_RF);
        outDeltaTfile << it << "\t" << (Float_t)(deltaT_RF-zero) << std::endl;
        if (m_ts_verbose) print("Edge :", zero, "with", (int) m_ts_histo[it] -> GetMaximum(), "counts in peak");
      }
      else
      {
        pospic = m_ts_histo[it] -> GetMean();
        m_ts_histo[it] -> GetXaxis() -> SetRange(pospic+100*m_ts_rebin[DSSD], pospic+100*m_ts_rebin[DSSD]);
        outDeltaTfile << it << "\t" << (Float_t)(m_ts_histo[it] -> GetMean()) << std::endl;
        if (m_ts_verbose) print( "mean : ", m_ts_histo[it] -> GetMean(), "with", (int) m_ts_histo[it] -> GetMaximum(), "counts in peak");
      }
      continue;
    }
    #endif
    amppic = m_ts_histo[it] -> GetMaximum();
    pospic = (Float_t) (m_ts_histo[it] -> GetMaximumBin() - (m_ts_histo[it] -> GetNbinsX()/2))*m_ts_rebin[type];
    dump_sigma = (Float_t) (m_ts_histo[it] -> FindLastBinAbove(amppic/2) - m_ts_histo[it] -> FindFirstBinAbove(amppic/2))*m_ts_rebin[type]/2;
    if (dump_sigma<2.) dump_sigma = 2.;
    if (m_ts_verbose) print("Dump parameters \t|mean : ", std::setprecision(3), pospic/1000, " ns sigma : ", dump_sigma, " ps -- amppic : ", amppic, "coups");

    gaus_pol0.reset(new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*dump_sigma,pospic+20*dump_sigma));
    gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
    gaus_pol0 -> SetRange(pospic-dump_sigma*20,pospic+dump_sigma*20);
    m_ts_histo[it] -> Fit(gaus_pol0.get(),"R+q");

    fittedPic.reset(m_ts_histo[it] -> GetFunction("gaus+pol0"));
    if (!fittedPic) continue; // Eliminate non existing fits
    cte = fittedPic -> GetParameter(0);
    Mean = fittedPic -> GetParameter(1);
    sigma = fittedPic -> GetParameter(2);
    if (m_ts_verbose) print("Fit results \t\t|mean : ", std::setprecision(3), Mean/1000, "ns sigma : ", sigma, " ps | Cte : ", cte);
    array_resolutions[it] = sigma*2.35;
    if (it == m_ts_time_ref.label) outDeltaTfile << it << "\t" << 0 << std::endl;
    else                           outDeltaTfile << it << "\t" << (Int_t) Mean << std::endl;
  }
  outDeltaTfile.close();
  if (m_ts_outroot_b)
  {
    TFile* outFile = TFile::Open((m_outdir+m_ts_outdir+m_ts_outroot).c_str(),"recreate");
    if (outFile == nullptr) {print("Cannot open file ", m_outdir+m_ts_outdir+m_ts_outroot, " !!!\nAbort !");return;}
    outFile -> cd();

    m_ts_EnergyRef.Write();
    m_ts_DeltaT.Write();
    m_ts_label_DeltaT.Write();
    m_ts_Label.Write();
    m_ts_mult.Write();

    // Save the deltaT histo for each detector :
    for (size_t i = 0; i < m_ts_histo.size(); i++) m_ts_histo[i].Write();
    m_ts_histo_RF.Write();

    //Save the deltaT VS Energy bidim for each detector
    for (size_t i = 0; i < m_ts_histo.size(); i++)
    {
      m_ts_E_VS_deltaT[i].Write();
    }

    outFile -> Write();
    outFile -> Close();
    delete outFile;
    std::cout << "Timeshifts root file saved to " << m_outdir+m_ts_outdir+m_ts_outroot << std::endl;
  }

  // TO RECODE ...
  std::ofstream f(m_outdir+m_ts_outdir+"resolutions_temporelles",std::ios::out);
  for (size_t i = 0; i<array_resolutions.size(); i++)
  {
    if (array_resolutions.size()<1E-6f)
    {
      array_resolutions.erase(array_resolutions.begin()+i);
      array_labels.erase(array_labels.begin()+i);
    }
    else { f << array_labels[i] << " " << array_resolutions[i] << std::endl; }
  }
  f.close();
  // ... UNTIL HERE

  std::cout << "Timeshifts data written to " << m_outdir+m_ts_outdir+m_ts_outdata << std::endl;
}

void NearLine::m_ca_Write(std::string _histoFilename)
{
  std::ofstream outfile  (m_outdir+m_ca_outDir+m_ca_outCalib);
  std::ofstream outfile2 (m_outdir+m_ca_outDir+"peaks.dat");
  std::ofstream outfile3 (m_outdir+m_ca_outDir+"fit_info.dat");
  Fits fits;
  TFile* outRootFile = nullptr;
  if (m_ca_outRoot!="") outRootFile = TFile::Open((m_outdir+m_ca_outDir+m_ca_outRoot).c_str(),"recreate");
  m_ca_calculate(_histoFilename, fits, outRootFile);
  for (size_t label = 0; label<fits.size(); label++)
  {
    if (label==215) {outfile << "251 0 1" << std::endl; continue;} // No calibration for the RF
    pic_fit_result & fit = fits[label];
    if ((!isTripleAlpha(m_ca_source) || !fit.exists()) && isDSSD[label]){outfile << std::to_string(label)+" 0 1"<<std::endl;continue;}
    if (!fit.exists()) continue;

    if (fit.too_few_counts())
    {
      outfile2 << label << "Too few counts : " << fit.nb_hits << " events" << std::endl;
      continue;
    }

    outfile << label << " " << fit.parameter0 << " " << fit.parameter1 << " ";
    if (fit.parameter2!=0.f) outfile << fit.parameter2;
    outfile << std::endl;

    for (size_t i = 0; i<fit.peaks.size(); i++ ) outfile2 << label << " " << fit.cmeasures[i]*fit.scalefactor ;

    outfile3 << label << " chi2: " << fit.chi2 << std::endl;
  }

  if (outRootFile !=  nullptr)
  {
    std::cout << "Root verification histograms written to " << m_outdir+m_ca_outDir+m_ca_outRoot << std::endl;
    outRootFile -> cd();
    outRootFile -> Write();
    outRootFile -> Close();
    delete outRootFile;
  }

  outfile.close();
  outfile2.close();
  outfile3.close();
  std::cout << "Calibration file written to " << m_outdir+m_ca_outDir+m_ca_outCalib << std::endl;
}

void NearLine::m_ca_calculate(std::string _histoFilename, Fits & fits, TFile* outRootFile)
{
  std::unique_ptr<TFile> file (new TFile((m_outdir+_histoFilename).c_str(),"read"));
  if (file.get() == nullptr || file-> IsZombie()) {std::cout << "Cannot open histo file" << std::endl; return;}
  TList *list = file->GetListOfKeys();
  fits.resize(m_labelToName.size());
  TIter next(list) ;
  TKey* key = nullptr;
  TObject *obj=nullptr;
  Int_t label = 0;
  TH1F* histo;
  TH1F* copyHisto;

  while ( (key = (TKey*)next()) )
  {
    // Parameterize the pics to fit :
    Int_t nb_pics = 0;
    Float_t E_right_pic = 0.f;

    //Get the spectra and its name :
    obj = key->ReadObj();
    if ( (strcmp(obj->IsA()->GetName(),"TProfile")!=0) && (!obj->InheritsFrom("TH1")))
    {printf("Object %s is not 1D histogram : will not be treated\n",obj->GetName()) ;continue;}
    std::string name = obj->GetName();
    std::string detector = name;
    if (lastPart(name,'_') == "calib" || lastPart(name,'_') == "raw") detector = removeLastPart(name,'_');
    else continue;
    label = m_nameToLabel.at(detector);
    pic_fit_result & fit = fits[label];
    fit.label = label;
    if (name == "") continue;
    if (m_ca_verbose) std::cout << std::endl << name << std::endl;

    //Extract type of detector :
    Detector type = type_det(label);
    if (!type) continue; //remove all non handled detector
    histo = (TH1F*)file->Get(name.c_str());
    if (histo == nullptr || histo -> IsZombie()) {if (m_ca_verbose) print("Histo doesn't exists"); continue;}

    // Initialize algorithm parameters :
    Float_t integral_ratio_threshold = 0.f;
    Int_t ADC_threshold = 0;
    Int_t window_1 = 0, window_2 = 0, window_3 = 0;//Window value in keV
    #ifdef USE_DSSD
    bool triplealpha  = isTripleAlpha(m_ca_source);
    if (triplealpha && type!=DSSD) continue;
    #endif //USE_DSSD
    if (type == Ge)
    {// For Clovers
      window_1 = 10, window_2 = 8, window_3 = 4;
      if (m_ca_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.011;
        ADC_threshold = 100;
      }
      else if (m_ca_source == "232Th")
      {
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 910, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0049;
        ADC_threshold = 100;
      }
      else if (m_ca_source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        fit.peaks.resize(nb_pics);
        fit.peaks = {1172, 1333};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.06;
        ADC_threshold = 100;
      }
    }
    else if (type == LaBr3)
    {// For LaBr3
      window_1 = 70, window_2 = 50, window_3 = 20;
      if (m_ca_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.012;
        ADC_threshold = 500;
      }
      else if (m_ca_source == "232Th")
      {
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 911, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0025;
        ADC_threshold = 500;
      }
      else if (m_ca_source == "60Co")
      {// NOT FUNCTIONNAL YET !!!
        nb_pics = 2;
        fit.peaks.resize(nb_pics);
        fit.peaks = {1172, 1333};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.1;
        ADC_threshold = 100;
      }
    }
    else if (type == Paris)
    {// For Paris
      window_1 = 70, window_2 = 50, window_3 = 25;
      if (m_ca_source == "152Eu")
      {
        nb_pics = 5;
        fit.peaks.resize(nb_pics);
        fit.peaks = {121.7830, 344.2760, 778.9030, 964.1310, 1408.0110};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.012;
        ADC_threshold = 500;
      }
      else if (m_ca_source == "232Th")
      {//never tested yet - but should be similar to fatima
        nb_pics = 4;
        fit.peaks.resize(nb_pics);
        fit.peaks = {238, 583, 911, 2614};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.0025;
        ADC_threshold = 500;
      }
    }
    else if (type == DSSD)
    {
      print("coucou");
      window_1 = 150, window_2 = 100, window_3 = 50;
      #ifdef USE_DSSD
      if (triplealpha)
      {
        nb_pics = 3;
        fit.peaks.resize(nb_pics);
        fit.peaks = {5150, 5480, 5800};
        E_right_pic = fit.peaks.back();
        integral_ratio_threshold = 0.05;
        ADC_threshold = 500;
      }
      else
      {

      }
      #endif //USE_DSSD
    }
    else {if (m_ca_verbose) std::cout << name << " not a handled detector" << std::endl; continue;}
    fit.resize(nb_pics);//resize the intern vectors of the detector's pic_fit_result

    int vmaxchan = 0;// position of the right pic in bins (e.g. 1408 keV in Eu)
    double scalefactor = 0.; //to adapt to the binnin of the histogram (ADC/bin)
    double kpc = 0.; //keV per bin (=channel)

    // OPERATE ON THE SPECTRUM
    int nbins = histo->GetXaxis()->GetNbins();
    scalefactor=histo->GetXaxis()->GetXmax()/nbins;
    fit.scalefactor = scalefactor;
    Int_t ADC_threshold_scaled = ADC_threshold/scalefactor;
    if (m_hc) ADC_threshold_scaled = 0;
    double sum=histo->Integral(ADC_threshold_scaled, nbins-1);
    fit.nb_hits = sum;
    #if defined USE_DSSD
    if (sum<1000 && triplealpha)
    {
      if (m_ca_verbose)
      {
        fit.too_few_counts(true);
        std::cout << "Too few counts to calibrate : " << sum << std::endl;
      }
      continue;
    }
    #else
    if (sum < 50000)
    {
      if (m_ca_verbose)
      {
        fit.too_few_counts(true);
        std::cout << "Too few counts to calibrate : " << sum << std::endl;
      }
      continue;
    }
    #endif
    if (m_ca_verbose)
    {
      std::cout << "Integral = " << sum << " counts; Number of bins = " <<  nbins << std::endl;
      std::cout << "scale factor = " << scalefactor << std::endl;
    }
    vmaxchan=nbins;
    double val=0;
    for (int j=nbins-2; j > ADC_threshold_scaled; j--)
    {
      val+=histo->GetBinContent(j);
      if ((val/sum) > integral_ratio_threshold) {vmaxchan=j; break;}
    }
    #ifdef PARIS
    if (type == Paris)vmaxchan = paris_vmaxchan[m_labelToName[label]]/scalefactor;
    #endif //PARIS
    if (vmaxchan==ADC_threshold_scaled) { if (m_ca_verbose) std::cout << "Could not fit this spectrum " << std::endl; continue;}
    // We have found the first peak in the right
    if (m_ca_verbose) std::cout << "right pic found at channel " << vmaxchan*scalefactor << " ADC" << std::endl;
    kpc=E_right_pic/vmaxchan;
    fit.keVperADC = kpc;
    if (m_ca_verbose) std::cout << "kev per channel = " << kpc << std::endl;
    for (int j=nb_pics-1; j>-1; j--) //Starting with the higher energy pic
    // for (int j=0; j<nb_pics; j++)
    {
      // 1st iteration :
      double p=fit.peaks[j];
      double cguess_high = (p+window_1)/kpc; //in bins
      double cguess_low  = (p-window_1)/kpc; // in bins
      if (cguess_low<ADC_threshold_scaled) cguess_low = ADC_threshold_scaled; //cannot read the spectra below the ADC threshold !
      if (m_ca_verbose) std::cout << "Pic : " << fit.peaks[j] << std::endl;
      if (m_ca_verbose) std::cout << "["<< cguess_low*scalefactor << " , " << cguess_high*scalefactor << "]" << std::endl;
      histo->GetXaxis()->SetRange(cguess_low, cguess_high);
      double cmc = 0.5 + histo->GetMaximumBin();
      if (m_ca_verbose) std::cout << "First mean = " << cmc*scalefactor << std::endl;

      // 2nd iteration :
      cguess_high = 0.5 + cmc + window_2/kpc;
      cguess_low  = 0.5 + cmc - window_2/kpc;
      histo->GetXaxis()->SetRange(cguess_low, cguess_high);
      double cm  = histo->GetMean(); //in ADC (scalefactor*bins)
      cmc = cm/scalefactor;
      if (m_ca_verbose) std::cout << "["<< cguess_low*scalefactor << " , " << cguess_high*scalefactor << "]" << std::endl;
      if (m_ca_verbose) std::cout << "Second mean = " << cm << std::endl;

      // 3rd iteration :
      cguess_high = cmc + window_3/kpc;
      cguess_low  = cmc - window_3/kpc;
      histo->GetXaxis()->SetRange(cguess_low, cguess_high);
      cm  = histo->GetMean(); //in ADC
      cmc = cm/scalefactor; //in bins
      if (m_ca_verbose) std::cout << "["<< cguess_low*scalefactor << " , " << cguess_high*scalefactor << "]" << std::endl;
      if (m_ca_verbose) std::cout << "Third mean = " << cm << std::endl;
      fit.cmeasures[j]=cmc; //The measured channel number
      if (outRootFile !=  nullptr)
      {
        // !!! ATTENTION : THE FIT IS TO BE IMPROVED  !!!
        // print(name + " pic n°" + j + " : " + fit.peaks[j] );
        outRootFile -> cd();
        copyHisto = (TH1F*)histo->Clone();
        double constante = copyHisto -> GetMaximum();
        double mean = cmc;
        double sigma = (copyHisto -> FindLastBinAbove(constante/2) - copyHisto -> FindFirstBinAbove(constante/2))/2.35;
        TF1*  gaus(new TF1("gaus","gaus"));
        gaus -> SetRange(cguess_low*scalefactor, cguess_high*scalefactor);
        gaus -> SetParameter(0, constante);
        gaus -> SetParameter(1, mean);
        gaus -> SetParameter(2, sigma);
        copyHisto -> Fit(gaus,"RQ");
        // TF1* gaus_pol0(new TF1("gaus(0)+pol1(3)","gaus(0)+pol1(3)"));
        // gaus_pol0 -> SetRange(cguess_low*scalefactor, cguess_high*scalefactor);
        // gaus_pol0 -> SetParameter(0, gaus_pol0 -> GetParameter(0));
        // gaus_pol0 -> SetParameter(1, gaus_pol0 -> GetParameter(1));
        // gaus_pol0 -> SetParameter(2, gaus_pol0 -> GetParameter(2));
        // gaus_pol0 -> SetParameter(3, copyHisto -> GetBinContent(0));
        // copyHisto -> Fit(gaus_pol0,"RIQE");
        fit.mean [j] = gaus -> GetParameter(1);
        fit.sigma[j] = gaus -> GetParameter(2);
        // fit.mean [j] = gaus_pol0 -> GetParameter(1);
        // fit.sigma[j] = gaus_pol0 -> GetParameter(2);
        std::cout << std::setprecision(6);
        // print(label, "kpc : ", kpc, " scale : ", scalefactor, " pic : ", p, " -> ", cguess_low*kpc, " : ", cguess_high*kpc);
        // print(label, " pic : ", p, " -> ", cm, " ", copyHisto -> GetBinContent(0));
        if (j == nb_pics) kpc = cmc;
        // delete copyHisto;
      }
    }
    std::vector<Double_t> x(nb_pics);
    std::vector<Double_t> y(nb_pics);
    std::vector<Double_t> ex(nb_pics);
    std::vector<Double_t> ey(nb_pics);
    for (int j=0; j < nb_pics; j++)
    {
      if (m_ca_verbose) std::cout << "Energy = " << fit.peaks[j] << " Channel = " << fit.cmeasures[j]*scalefactor << std::endl;
      x[j]=fit.cmeasures[j]*scalefactor;
      y[j]=fit.peaks[j];
      ex[j]=0;
      ey[j]=0;
    }

    if (outRootFile !=  nullptr) outRootFile -> cd();
    TGraphErrors* gr (new TGraphErrors(nb_pics,x.data(),y.data(),ex.data(),ey.data()));
    gr -> SetName((name+"_gr").c_str());
    TF1* linear(new TF1("lin","pol1")); //Range and number of fit parameters
    gr->Fit(linear,"q");
    TF1* binom (new TF1("pol", "pol2"));

    if(isGe[label] || isDSSD[label])
    {//First order fit
      fit.parameter0 = linear -> GetParameter(0);
      fit.parameter1 = linear -> GetParameter(1);
      fit.chi2       = linear -> GetChisquare( );
      fit.exists(true);
    }
    else if (isLaBr3[label] || isParis[label])
    {// Second order fit
      binom -> SetParameters(0, linear -> GetParameter(0));
      binom -> SetParameters(1, linear -> GetParameter(1));
      gr -> Fit(binom,"q");
      fit.parameter0 = binom -> GetParameter(0);
      fit.parameter1 = binom -> GetParameter(1);
      fit.parameter2 = binom -> GetParameter(2);
      fit.chi2       = binom -> GetChisquare( );
      fit.exists(true);
    }
    else {
      fit.exists(false);
    }

    if (outRootFile !=  nullptr)
    {
      outRootFile -> cd();
      auto c1 = new TCanvas;
      c1 -> cd();
      gr -> Draw("A*");
      c1 -> Write();
      delete c1;
      gROOT -> cd();
    }
  }
}

void NearLine::m_ca_residus_calculate()
{ //FATIMA CONFIGURATION ONLY FOR NOW !
  m_hr = false;
  m_hc = true;
  m_hc_outroot = "Calibrated_histo.root";
  if (!m_calib.readFile(m_outdir+m_ca_outDir+m_ca_outCalib, m_labelToName.size())) return;//Sets the calibration file
  m_hc_Ge_Clover = true;
  #ifdef FATIMA
  m_hc_LaBr3 = true;
  #endif //FATIMA
  #ifdef PARIS
  m_hc_Paris = true;
  #endif //PARIS
  m_hc_Initialize();
  p_Files.setCursor(0);
  multi_run();
  m_hc_Write();
  Fits fits;
  TFile* outRootFile (TFile::Open((m_outdir+m_ca_outDir+"residues.root").c_str(),"recreate"));
  m_ca_calculate(m_hc_outroot, fits, outRootFile);
  gROOT -> cd();
  std::ofstream outfile  (m_outdir+m_ca_outDir+"residues.dat");
  std::ofstream outfile2 (m_outdir+m_ca_outDir+"residues2.dat");
  std::vector<TCanvas*> canvas(28, nullptr);

  for (int i = 0; i<24; i++)
  {
    canvas[i] = new TCanvas(("Clover R"+std::to_string((i<12) ? 3 : 2)+"A"+std::to_string(i%12+1)).c_str(), ("Clover_"+std::to_string(i)).c_str());
    canvas[i] -> Divide(2,2);
  }

  canvas[24] = new TCanvas("LaBr3_R1", "LaBr3_R1");
  canvas[24] -> Divide(5,2);
  canvas[25] = new TCanvas("Ge_R2", "Ge_R2");
  canvas[25] -> Divide(12,4);
  canvas[26] = new TCanvas("Ge_R3", "Ge_R3");
  canvas[26] -> Divide(12,4);
  canvas[27] = new TCanvas("LaBr3_R4", "LaBr3_R4");
  canvas[27] -> Divide(5,2);
  for (auto fit : fits)
  {
    std::vector<Float_t> residus(fit.peaks.size());
    if (m_labelToName[fit.label] == "") continue;
    outfile << fit.label;
    outfile2 << fit.label;
    for (size_t i = 0; i<fit.peaks.size(); i++)
    {
      // residus[i] = fit.peaks[i] - fit.mean[i];
      residus[i] = fit.peaks[i] - fit.cmeasures[i]*fit.scalefactor;
      outfile << " " << fit.peaks[i] - fit.cmeasures[i]*fit.scalefactor;
      outfile2 << " " << fit.peaks[i] - fit.mean[i];
    }
    if (fit.label > 199 && fit.label < 210) canvas[24] -> cd(fit.label-199);
    else if (fit.label > 96 && fit.label < 167) canvas[25] -> cd(fit.label - 95 - 2*( (fit.label - 95)/6 + 1 ) + 1 );
    else if (fit.label > 23 && fit.label < 95) canvas[26] -> cd( fit.label - 23 - 2*( (fit.label - 23)/6 + 1 ) + 1 );
    else if (fit.label > 210 && fit.label < 220) canvas[27] -> cd(fit.label-209);
    else continue;
    TGraph* graph = new TGraph(fit.peaks.size(), fit.peaks.data(), residus.data());
    graph -> SetTitle(m_labelToName[fit.label].c_str());
    graph -> GetXaxis() -> SetTitle("Energy [keV]");
    if (is_Clover_Ge(fit.label)) graph -> GetYaxis() -> SetRangeUser(-1.5,1.5);
    graph -> Draw();
    // TF1* EMT_max = new TF1("EMT_max","0.5",0,10000);
    // TF1* EMT_min = new TF1("EMT_in","-0.5",0,10000);
    // EMT_max -> Draw();
    // EMT_min -> Draw();

    // To group all the same clovers channels :
    if (is_Clover_Ge(fit.label))
    {
      canvas[labelToClover(fit.label)] -> cd(fit.label%6);
      TGraph* g = new TGraph(fit.peaks.size(), fit.peaks.data(), residus.data());
      g -> SetTitle(m_labelToName[fit.label].c_str());
      g -> GetXaxis() -> SetTitle("Energy [keV]");
      g -> GetYaxis() -> SetTitle("Residus [keV]");
      g -> GetYaxis() -> SetRangeUser(-1.5,1.5);
      outRootFile -> cd();
      g -> Draw();
      gROOT -> cd();
    }
    outfile << std::endl;
    outfile2 << std::endl;
  }
  outRootFile -> cd();
  for (auto c : canvas) c -> Write();
  outRootFile -> Write();
  outRootFile -> Close();
  delete outRootFile;
  outfile.close();
  outfile2.close();
  m_hc = false;
}

Bool_t NearLine::loadLabels(std::string const & _fileID)
{
  m_labelToName.clear();
  m_labelToName = arrayID(_fileID);
  for (size_t i = 0; i<m_labelToName.size(); i++)
  {
    if (m_labelToName[i]!="") m_nameToLabel[m_labelToName[i]] = i;
  }
  if (m_labelToName.size()>1) return true; else return false;
}

Bool_t NearLine::loadCalib(std::string const & calib_filename)
{
  m_calib.readFile(calib_filename, m_labelToName.size());
  if (m_calib.size() > 1) return true;
  else return false;
}

Bool_t NearLine::loadTimeshifts(std::string _inFile)
{
  m_timeshifts = arrayTimeShift(_inFile);
  if (m_timeshifts.size() > 1)
  return true;
  else return false;
}

inline Bool_t NearLine::calibrate_hit(Hit & hit)
{
  return (time_shift(hit) && m_calib.calibrate(hit));
}

inline Bool_t NearLine::time_shift(Hit & hit)
{
  if (hit.label>m_timeshifts.size()) return false;
  hit.time += m_timeshifts[hit.label];
  return true;
}

Bool_t NearLine::setConfig (std::stringstream & parameters_file)
{
  while (parameters_file.good())
  {
    std::string parameter;
    std::string temp1, temp2;
    getline(parameters_file, parameter);
    std::istringstream is(parameter);
    is.clear();
    temp1 = "NULL"; temp2 = "NULL";
    is>>temp1;
    if (temp1[0]=='#' || temp1[1]=='#') continue;
    // ************************ //
    //        LOAD INPUTS       //
    // ************************ //
    if(temp1 == "ID:")
    {
      is >> temp2;
      if (temp2 == "NULL") {std::cout << "NO PARAMETER for ID" << std::endl; return false;}
      if(!loadLabels(temp2)) {std::cout << "ERROR : Labels not loaded " << std::endl; return false;}
      else {std::cout << temp2 << " labels file loaded successfully" << std::endl;}
    }

    else if(temp1 == "DATADIR:")
    {
      Int_t nb_files = -1; std::string folder = "";
      is >> folder;
      if (folder == "NULL") {std::cout << "NO PARAMETER for DATADIR" << std::endl; return false;}
      while (is>>temp2)
      {
        if (temp2 == "nb:")  is >> nb_files;
        else if (temp2 == "step:") is >> m_read_step;
        else if (temp2 == "start:") is >> m_read_start;
        else if (temp2 == "root_eventbuild") m_evtbuilt = true; //deprecated
      }
      if (!p_Files.addFolder(folder, nb_files)) {std::cout << "ERROR : files from directory " << folder << " not loaded " << std::endl; return false;}
      if (nb_files == '*') nb_files = -1;
    }

    else if(temp1 == "DATAFILE:")
    {
      is >> temp2;
      if (temp2 == "NULL") {std::cout << "NO PARAMETER for DATAFILE" << std::endl; return false;}
      if (!p_Files.addFiles(temp2)) {std::cout << "ERROR : file(s) " << temp2 << " not loaded " << std::endl; return false;}
    }

    else if (temp1 == "TIMESHIFT_DATA:")
    {
      is >> temp2;
      if (temp2 == "NULL") {std::cout << "NO PARAMETER for TIMESHIFTS" << std::endl; return false;}
      else
      {
        if (!loadTimeshifts(temp2)) {std::cout << "ERROR : file(s) " << temp2 << " not loaded " << std::endl; return false;}
        std::cout << temp2 << " time shift file successfully loaded" << std::endl;
      }
    }

    else if (temp1 == "CALIBRATION:")
    {
      is >> temp2;
      if (temp2 == "NULL") {std::cout << "NO PARAMETER for CALIBRATION" << std::endl; return false;}
      if (!loadCalib(temp2)) {std::cout << "ERROR : calibration file " << temp2 << " not loaded " << std::endl; return false;}
      std::cout << temp2 << " calibration file successfully loaded" << std::endl;
    }

    else if (temp1 == "THRESHOLD:")
    {
      is >> m_E_threshold;
      m_use_threshold = true;
      std::cout << "Threshold set to " << m_E_threshold << " keV" << std::endl;
    }

    else if (temp1 == "BINNING:")
    {
      is >> temp2;
      if (temp2 == "calibrated:")
      {
        while(is >> temp2)
        {
          if      (temp2 == "Ge:"   ) is >> m_bins_calib[Ge]    >> m_min_calib[Ge]    >> m_max_calib[Ge]   ;
          else if (temp2 == "BGO:"  ) is >> m_bins_calib[BGO]   >> m_min_calib[BGO]   >> m_max_calib[BGO]  ;
          else if (temp2 == "LaBr3:") is >> m_bins_calib[LaBr3] >> m_min_calib[LaBr3] >> m_max_calib[LaBr3];
          else if (temp2 == "Paris:") is >> m_bins_calib[Paris] >> m_min_calib[Paris] >> m_max_calib[Paris];
          else if (temp2 == "DSSD:")  is >> m_bins_calib[DSSD]  >> m_min_calib[DSSD]  >> m_max_calib[DSSD];
          else std::cout << std::endl << "BINNING: calibrated: ATTENTION, parameter " << temp2 << " not recognized !" << std::endl << std::endl;
        }
      }
      else if (temp2 == "bidim:")
      {
        while(is >> temp2)
        {
          if      (temp2 == "Ge:"   ) is >> m_bins_bidim[Ge]    >> m_min_bidim[Ge]    >> m_max_bidim[Ge]   ;
          else if (temp2 == "BGO:"  ) is >> m_bins_bidim[BGO]   >> m_min_bidim[BGO]   >> m_max_bidim[BGO]  ;
          else if (temp2 == "LaBr3:") is >> m_bins_bidim[LaBr3] >> m_min_bidim[LaBr3] >> m_max_bidim[LaBr3];
          else if (temp2 == "Paris:") is >> m_bins_bidim[Paris] >> m_min_bidim[Paris] >> m_max_bidim[Paris];
          else if (temp2 == "DSSD:")  is >> m_bins_bidim[DSSD] >> m_min_bidim[DSSD] >> m_max_bidim[DSSD];
          else std::cout << std::endl << "BINNING: bidim: ATTENTION, parameter " << temp2 << " not recognized !" << std::endl << std::endl;
        }
      }
      else if (temp2 == "uncalibrated:")
      {
        while(is >> temp2)
        {
          if      (temp2 == "Ge:"   ) is >> m_bins_raw[Ge]    >> m_min_raw[Ge]    >> m_max_raw[Ge]   ;
          else if (temp2 == "BGO:"  ) is >> m_bins_raw[BGO]   >> m_min_raw[BGO]   >> m_max_raw[BGO]  ;
          else if (temp2 == "LaBr3:") is >> m_bins_raw[LaBr3] >> m_min_raw[LaBr3] >> m_max_raw[LaBr3];
          else if (temp2 == "Paris:") is >> m_bins_raw[Paris] >> m_min_raw[Paris] >> m_max_raw[Paris];
          else if (temp2 == "DSSD:")  is >> m_bins_raw[DSSD] >>  m_min_raw[DSSD] >>  m_max_raw[DSSD];
          else std::cout << std::endl << "BINNING: uncalibrated: ATTENTION, parameter " << temp2 << " not recognized !" << std::endl << std::endl;
        }
      }
    }

    else if (temp1 == "NB_THREADS:")
    {
      is >> m_nb_threads;
      print(m_nb_threads, " threads");
    }

    else if (temp1 == "OUTDIR:")
    {
      is >> m_outdir;
      if (m_outdir.back()!='/') m_outdir.push_back('/');
    }

    else if (temp1 == "TEMP_TREE_ON_DISK")
    {
      print("TEMPORARY TREE ON DISK");
      temp_tree_on_disk = true;
    }

    else if (temp1 == "FORCE_TIMESHIFT")
    {
      print("FORCE TIMESHIFTS");
      m_apply_timeshift = true;
    }

    else if (temp1 == "FORCE_CALIBRATION")
    {
      print("FORCE CALIBRATION");
      m_apply_calibration = true;
    }

    else if (temp1 == "USE_RF:")
    {
      is >>m_RF_shift_ns;
      m_use_RF = true;
      m_RF_shift = m_RF_shift_ns*1000ull;
    }

    // **************** //
    //     CONSTANTS    //
    // **************** //
    else if (temp1 == "EDEN_TO_LICORNE_DISTANCE=")
    {
      #ifdef LICORNE
      is>>EDEN_TO_LICORNE_DISTANCE;
      #endif //LICORNE
    }

    else if (temp1 == "GAMMA_FLASH=")
    {
      #ifdef LICORNE
      is>>GAMMA_FLASH;
      #endif //LICORNE
    }

    // ************************ //
    //     CONFIGURE OUTPUTS    //
    // ************************ //
    else if (temp1 == "HISTOGRAM:")
    {
      is >> temp2;
      if (temp2 == "calibrated:")
      {
        m_hc = true;
        m_apply_calibration = true;
        if (m_calib.size() == 0) {std::cout << "No calibration data..."; return false;}
        std::string name = "", filename = "";
        while(is >> name)
        {
          if (name == "") {std::cout << "What calibrated histogram do you want ?" << std::endl
          <<  "Check your parameters.dat files ..." << std::endl;return false;}
          else if (name == "Ge_Clover") {m_hc_Ge_Clover = true;}
          else if (name == "LaBr3") {m_hc_LaBr3 = true;}
          else if (name == "DSSD") {m_hc_DSSD = true;}
          else if (name == "Paris") {m_hc_Paris = true;}
          else if (name == "outRoot:") is >> m_hc_outroot;
          else {std::cout << name << " calibrated spectra not taken care of. Sorry ! " << std::endl; return false;}
        }
      }
      else
      {
        m_hr = true;
        m_hr_outroot = temp2;
        if (m_hr_outroot == "NULL") {std::cout << "NO PARAMETER for histogram" << std::endl; return false;}
      }
    }

    else if (temp1 == "PARIS_BIDIM:")
    {
      m_pb = new ParisBidim(this);
      m_apply_calibration = true;
      if (!m_pb -> SetConfig(is)) return false;
    }

    else if (temp1 == "RF_CHECK:")
    {
      m_rfc = true;
      m_apply_timeshift = true;
      m_rfc_timeWindow_ns = 1500;
      m_rfc_timeWindow = m_rfc_timeWindow_ns*1000;
      while (is >> temp2)
      {
        if (temp2 == "timewindow:")
        {
          is >> m_rfc_timeWindow_ns;
          if (m_rfc_timeWindow_ns == 0) {std::cout << "NO TIME WINDOW FOR RF CHECK !!" << std::endl; return false; }
          m_rfc_timeWindow = m_rfc_timeWindow_ns*1000;
        }
        else if (temp2 == "time_ref:")
        {
          is >> m_rfc_time_ref_det;
          if (m_rfc_time_ref_det == "") {std::cout << "NO TIME REFERENCE FOR RF CHECK !!" << std::endl; return false; }
          m_rfc_time_ref_label = m_nameToLabel [m_rfc_time_ref_det];
          print(m_rfc_time_ref_label);
        }
        else if (temp2 == "RF_shift:")
        {
          is>>m_rfc_RF_shift_ns;
          m_rfc_RF_shift = m_rfc_RF_shift_ns*1000;
        }
        else if (temp2 == "nbMax:") {is >> m_rfc_nbFiles;}
        else if (temp2 == "outRoot:"){ is >> m_rfc_outroot; }
        else { print("ERROR IN PARAMETERS");return false;}
      }
      std::cout << "Perform RF Checkings with time window of " << m_rfc_timeWindow_ns
                << "ns and ref detector " << m_rfc_time_ref_det << "(" << m_rfc_time_ref_label << ")" << std::endl;
    }

    else if (temp1 == "ANALYSE:")
    {
      m_a = new Analyse(this);
      if (!m_a -> SetConfig(is)) return false;
      m_apply_calibration = true;
      m_apply_timeshift = true;
    }

    else if (temp1 == "ANALYSE_W:")
    {
      m_aw = new Analyse_W(this);
      if (!m_aw -> SetConfig(is)) return false;
      m_apply_calibration = true;
      m_apply_timeshift = true;
    }

    else if (temp1 == "TIMESHIFT:")
    {
      m_ts = true;
      while(is >> temp2)
      {
        if (temp2 == "-")
        {
          getline(parameters_file, parameter);
          is.clear();
          is.str(parameter);
        }
        else if (temp2 == "timewindow:")
        {
          is >> m_ts_timeWindow_ns;
          if (m_ts_timeWindow_ns == 0) {std::cout << "NO TIME WINDOW !!" << std::endl; return false; }
          m_ts_timeWindow = m_ts_timeWindow_ns*_ns;
          std::cout << "Extracting timeshifts with a time window of " << m_ts_timeWindow_ns << " ns" << std::endl;
        }
        else if (temp2 == "time_reference:")
        {
          is >> m_ts_time_ref.name;
          if (m_ts_time_ref.name == "") {std::cout << "NO TIME REFERENCE !!" << std::endl; return false; }
          m_ts_time_ref.label = m_nameToLabel [m_ts_time_ref.name];
          std::cout << "Reference detector set to be " << m_ts_time_ref.name << " (n°" << m_ts_time_ref.label << ")" << std::endl;
        }
        else if (temp2 == "outRoot:") {is >> m_ts_outroot; m_ts_outroot_b = true;}
        else if (temp2 == "outData:") {is >> m_ts_outdata;}
        else if (temp2 == "mult:")    {is >> m_ts_min_mult >> m_ts_max_mult;} //by default
        else if (temp2 == "bidim")        {m_ts_bidim      = true;}
        else if (temp2 == "calibrated")   {m_ts_calibrated = true;}
        else if (temp2 == "uncalibrated") {m_ts_calibrated = false;} //by default
        else if (temp2 == "verbose") {m_ts_verbose = true;}
        else {std::cout << std::endl << "ATTENTION, parameter " << temp2 << " not recognized !" << std::endl << std::endl; return false;}
      }
    }

    else if (temp1 == "CALIBRATE:")
    {
      m_ca = true;
      m_hr = true;
      if (!folder_exists(m_outdir, true)) return false;
      m_hr_outroot = "temp_histo.root";
      while (is >> temp2)
      {
        if (temp2 == "-")
        {
          getline(parameters_file, parameter);
          is.clear();
          is.str(parameter);
        }
        else if (temp2 == "outRoot:")
        {
          m_ca_outRoot_b = true;
          is >> m_ca_outRoot;
        }
        else if (temp2 == "source:")    { is >> m_ca_source;  }
        else if (temp2 == "outCalib:")  { is >> m_ca_outCalib;}
        else if (temp2 == "residues")   { m_ca_residus = true;}
        else if (temp2 == "verbose")    { m_ca_verbose = true;}
        else {std::cout << "Unknown parameter " << temp2 << std::endl; return false;}
      }
    }

    else if (temp1 == "FASTER2ROOT:")
    {
      m_fr = true;
      while (is >> temp2)
      {
        if (temp2 == "-")
        {
          getline(parameters_file, parameter);
          is.clear();
          is.str(parameter);
        }
        else if (temp2 == "trigger:")
        {
          int nb_conditions; is >> nb_conditions;
          std::string condition;
          for (int i = 0; i<nb_conditions; i++)
          {
            is >> condition;
            m_fr_trigger.addCondition(condition);
          }
        }
        else if (temp2 == "keep_all") {m_fr_keep_all = true;}
        else if (temp2 == "raw") {m_fr_raw = true;}
        // else if (temp2 == "throwsingles") { m_fr_throw_singles = true; }
        // else if (temp2 == "eventbuild"  ) { m_fr_eventbuild    = true; }
        else if (temp2 == "outDir:") { is >> m_outdir; }
        else {std::cout << temp2 << " not a good parameter for analysis... Please check the parameters file" << std::endl; return false;}
      }
      if (m_outdir.back() != '/') m_outdir.push_back('/');
    }

    else if(temp1 != "NULL")
    {
      std::cout << "WARNING: Unknown parameter " << temp1 << " will be ignored !" << std::endl;
      return false;
    }
  }
  return true;
}

Bool_t NearLine::setConfig (std::string parameter)
{
  std::stringstream stream;
  stream.str(parameter);
  return setConfig(stream);
}

Bool_t NearLine::loadConfig (std::string _name)
{
  m_parameters_filename = _name;
  std::ifstream f(_name);
  if (f && f.is_open())
  {
    std::stringstream parameters;
    parameters << f.rdbuf();
    f.close();
    return setConfig(parameters);
  }
  else {return false;}
}

void NearLine::setThreads(int nb_threads)
{
  m_nb_threads = nb_threads;
}

// ARCHIVES :

// if (m_fr)
// {// Faster 2 root
//   if (isGe[i_hit.label]) m_fr_Ge_raw[thread_nb] -> Fill(i_hit.nrjcal);
//   if (i_hit.label > 251) continue; //reject EDEN

// ******************** //
// -------------------- //
//      CODE JON        //
// -------------------- //
// ******************** //

  /*
  if (fr_event.build(i_hit, rf))
  {
    Counters arg;
    // for (unsigned char i = 0; i<fr_buffer.mult; i++)
    // m_fr_dT_pulse[thread_nb] -> Fill(fr_buffer.times[i]/_ns);
    if (m_fr_trigger_buffer(fr_buffer, arg))
    {
      // if ((ToF_single = (Long64_t)(fr_event.singleHit().time - fr_event.setRFTime - fr_event.getShift())) > (Long64_t)(-400000ull+fr_event.getShift()))
      // m_fr_dT_pulse[thread_nb] -> Fill(ToF_single/1000.);
      // if ((ToF_single = (Long64_t)(fr_event.olderHit().time - fr_event.setRFTime - fr_event.getShift())) > (Long64_t)(-500000ull+fr_event.getShift()))
      // print((Long64_t)(fr_event.olderHit().time - fr_event.setRFTime - fr_event.getShift())/1000);
      // m_fr_dT_pulse[thread_nb] -> Fill(ToF_single/1000.);
      for (unsigned char i = 0; i<fr_buffer.mult; i++)
      {
        m_counter++;
        m_fr_dT_pulse[thread_nb] -> Fill((Long64_t)(fr_buffer.times[i] - fr_event.setRFTime - m_fr_shift)/1000.);
        m_fr_C_VS_L[thread_nb] -> Fill( ( (arg.LaBr3Mult<4) ? arg.LaBr3Mult : 4 ), ( (arg.CleanGeMult<4) ? arg.CleanGeMult : 4 ) );
        m_fr_ToF[thread_nb] -> Fill(rf.pulse_ToF(fr_buffer.times[i], m_fr_shift)/1000.);
        m_fr_ToF_all_det[thread_nb] -> Fill(rf.pulse_ToF(fr_buffer.times[i], m_fr_shift)/1000., fr_buffer.labels[i]);
      }
      // while(fr_event.build_more(i_hit))
      // {
      //   rootTree -> GetEntry(gindex[loop++]);
      //   m_fr_dT_pulse
      // }
      // fr_event.set_last_hit(i_hit);
      // outTree -> Fill();
    }
  }
  // if (!rf.isPrompt(i_hit,prompt_min,prompt_max)) continue;
  //
  //Spectra :
//   if (!rf.isPrompt(i_hit,prompt_min,prompt_max)) continue;
//   if (isGe[i_hit.label])
//   {
//     m_fr_Ge_prompt[thread_nb] -> Fill(i_hit.nrjcal);
//   }
//   if (fr_h_event.build(i_hit,rf))
//   {
//     m_fr_dT_pulse[thread_nb] -> Fill(rf.pulse_ToF(i_hit,fr_h_event.getShift())/1000.);
//     Counters arg;
//     m_fr_trigger_buffer(fr_h_buffer, arg);
//     m_fr_M[thread_nb] -> Fill(arg.ModulesMult);
//
//     for (size_t i = 0; i<arg.addback.size(); i++)
//     {
//       m_fr_Ge_addback[thread_nb] -> Fill(arg.addback[i].nrjcal);
//     }
//
//     for (size_t i = 0; i<arg.comptonClean.size(); i++)
//     {
//       const NRJ & E = arg.comptonClean[i].nrjcal;
//       m_fr_Ge_compton[thread_nb] -> Fill(E);
//       switch (arg.ModulesMult)
//       {
//         case 0 :
//           break;
//         case 1 :
//           m_fr_Ge_M1[thread_nb] -> Fill(E);
//           break;
//         case 2 :
//           m_fr_Ge_M2[thread_nb] -> Fill(E);
//           break;
//         case 3 :
//           m_fr_Ge_M3[thread_nb] -> Fill(E);
//           break;
//         case 4 :
//           m_fr_Ge_M4[thread_nb] -> Fill(E);
//           break;
//         case 5 :
//           m_fr_Ge_M5[thread_nb] -> Fill(E);
//           break;
//         case 6 :
//           m_fr_Ge_M6[thread_nb] -> Fill(E);
//           break;
//         default :
//           m_fr_Ge_Msup6[thread_nb] -> Fill(E);
//           break;
//       }
//     }
//   }
//   else if (fr_h_event.isSingle())
//   {
//     m_fr_ToF[thread_nb] -> Fill(rf.pulse_ToF(i_hit,fr_h_event.getShift())/1000.);
//     const Hit& single = fr_h_event.singleHit();
//     if (isGe[single.label])
//     {
//       m_fr_Ge_addback[thread_nb]->Fill(single.nrjcal);
//       m_fr_Ge_compton[thread_nb]->Fill(single.nrjcal);
//       m_fr_Ge_M1[thread_nb] -> Fill(single.nrjcal);
//       m_fr_M[thread_nb] -> Fill(1);
//     }
//   }
//   rf.setHit(i_hit); //important to set rf AFTER event building (some time discontinuity issues otherwise... )
// }

*/

/*
Double_t tdif=0; //timestamp difference to the beam pulse

int TriggerCount=0; //Number of triggers
int TriggerL2=0;
int TriggerL2C1=0;
int TriggerC0=0;
int TriggerC1=0;
int TriggerC2=0;
int TriggerC3=0;
int TriggerM3=0;
int TriggerM4=0;
int TriggerM5=0;
int TriggerM6=0;
// int TriggerCount2=0; //Number of event writes
Double_t DeltaTS=0; //This is the variable for the NEW time stamps
int RawMult=0; //Count the raw multiplicity in an event
bool PromptTrigger=false; //Flag to say if the prompt trigger passed
bool PromptTriggerCalculated=false;
bool EndEvent=false; //Flag to say the event has ended
bool EndOfPrompt=false; //Flag to say the prompt part of the event has ended
// double TotalEnergy=0;
// double BGOEnergy=0;
// double GeEnergy=0;
double LaEnergy=0;
int LaMult=0; // LaBr3 Mult
int TotalMult=0;
int PromptMult=0;
int PromptGeMult=0;
// int DelayedMult=0;
// int DelayedGeMult=0;
//Used for Compton suppression and addback in clovers
int* spat=new int[35]; //BGO hit pattern in the event
int* gpat=new int[35]; //Ge clover mult pattern in the event
// double* Esum=new double[35]; //Ge module energies
// double* Tsum=new double[35]; //Variable to make an "average" clover timestamp
// double* penergy=new double[35]; //Array of prompt Ge module energies
// double* ptime=new double[35]; //Array of prompt Ge module energies
// double* pid=new double[35]; //Array of prompt Ge module energies

// PROCESS ONE FILE AT A TIME AND CORRECT TIME STAMPS FOR THE WHOLE FILE
int PutPeakAt=340; //Prompt peak at 40 ns
// Read through
ULong64_t beampulse=0; //We want to keep the beam pulse from the previous file
UInt_t beamperiod=400000; //We want to keep the beam period from the previous file
ULong64_t StartTimeStamp=0;
// ULong64_t PreviousTimeStamp=0;
// int nfiles=0;
// int RunNumber;
// int FileNumber;
std::string OutputSpectraFile;
// TFile *outputspectrafile;
ULong64_t tab_tm;
// UChar_t tab_label;
// Double_t tab_nrj;
// UInt_t tab_nrj2;
// Bool_t tab_pileup;

//DEFINE OUTPUT TREE
// New tree in which to write the sorted data
UInt_t size;
// Double_t energies[256];
Double_t timestamps[256];
UInt_t ids[256]; //ids array
// Bool_t pups[256]; //pileups array
int EVENT_TIME_WINDOW= 700;
// bool pileup = false;
// ******************** //
// -------------------- //
//     FIN CODE JON     //
// -------------------- //
// ******************** //
//while (Read())
{
        // ******************** //
        // -------------------- //
        //      CODE JON        //
        // -------------------- //
        // ******************** //

        Double_t nrj=i_hit.nrj; // THE ENERGY
      	Int_t detid=i_hit.label; // THE DETECTOR ID
      	ULong64_t timestamp=i_hit.time+(PutPeakAt*1000);//Apply the offset in ps. 132 to put peak chanel 200.
      	if (detid==251) {beampulse=tab_tm; beamperiod=rf.period;}//No offset applied to beam pulse
      	Long64_t TSBeamDifference=timestamp-beampulse;
      	Double_t tdifps=((TSBeamDifference) % beamperiod);
      	tdif=tdifps/1000.0; //convert to ns

      	Double_t cnrj=nrj;
        bool laprompt=false;

//StartTimeStamp is the first timestamp of the prompt gammas minus its tdif from beampulse to get event "zero".
	if (RawMult==0) {StartTimeStamp=(timestamp-tdifps);} //Get the "zero" value for this potential event
        DeltaTS=(timestamp-StartTimeStamp)/1000.0;

    if (DeltaTS < EVENT_TIME_WINDOW)
    {
      timestamps[RawMult]=DeltaTS;
      ids[RawMult]=detid;
      RawMult++;
    }
	//Different time windows for different detector types.
	//Germaniums decide end of prompt for everyone else.
        if (DeltaTS >= EVENT_TIME_WINDOW) {EndOfPrompt=true;} //Force the end of prompt part, no matter what
        if (EndOfPrompt && RawMult>1)
        {
          for (int loop_hit = 0; loop_hit<RawMult; loop_hit++)
          {
            m_a_label_DeltaT_RF_2[thread_nb] -> Fill(timestamps[loop_hit], ids[loop_hit]);
            // print(loop_hit, " : ", timestamps[loop_hit],ids[loop_hit]);
          }
        }

	//Delayed gammas now. Go calculate the prompt mult
        if ((EndOfPrompt) && !PromptTriggerCalculated)
	//At least 2 Ge before we even bother with trigger logic
        	{
		//End of Prompt gammas. Did event pass prompt trigger condition ok?
		//Clear the suppression array. Suppression and addback in the ge modules
		int CMult=0;
		int MMult=0;
		if (PromptGeMult >=2) //Don't bother to do the more advanced test if minimum test is not met
			{
			for (int k=1; k <= 34; k++) {spat[k]=0; gpat[k]=0;}
			for (int j=0; j < PromptMult; j++)
				{
				// int mnum=module[ids[j]];
				// if ((isge[ids[j]]) && (energies[j] < 10000)) {gpat[mnum]++; GeEnergy+=energies[j];}
				// if ((isbgo[ids[j]]) && (energies[j] < 10000)) {spat[mnum]++; BGOEnergy+=energies[j];}
                		}
			for(int k = 1; k <= 34; k++) //loop through the Ge modules
				{
				if (spat[k] || gpat[k]) {MMult++;} //Either Ge or BGO or both
				// Caclulate the clean Ge multiplicity here. Compton Suppression pattern in spat[]
				if ((spat[k]==0) && (gpat[k] >= 1)) {CMult++;}
				} //end for k loop
			TotalMult=MMult+LaMult;//LaBr3 mult + module mult. Reduced Multiplicity
                	// TotalEnergy=BGOEnergy+GeEnergy+LaEnergy;
// !!!!!!!!!!!!!!!!! PROMPT MULTIPLICITIES ARE NOW KNOWN HERE. WE CAN START TO TRIGGER !!!!!!!!!!
			}
		// *THE* PROMPT TRIGGER CONDITION.
		if (((CMult >= 2) || (LaMult >=2)) && (TotalMult >=3)) //Clean Ge module multiplicity of 2 or more
			{
			PromptTrigger=true;
			TriggerCount++;
			if (LaMult >= 2) {TriggerL2++;}
			if ((LaMult >= 2) && (CMult >=1)) {TriggerL2C1++;}
			if (CMult == 0) {TriggerC0++;}
			if (CMult == 1) {TriggerC1++;}
			if (CMult == 2) {TriggerC2++;}
			if (CMult >= 3) {TriggerC3++;}
			if (TotalMult >= 3) {TriggerM3++;}
			if (TotalMult >= 4) {TriggerM4++;}
			if (TotalMult >= 5) {TriggerM5++;}
			if (TotalMult >= 6) {TriggerM6++;}
			}
		else {EndEvent=true; PromptTrigger=false;} //Look for next event. Forced end.
		PromptTriggerCalculated=true;
  } //We're done finding the prompt Multiplicity. We do this part only ONCE.

// !!!!!!!!!!!!!!!!!!!!!!!!!!! THE LENGTH OF THE DELAYED PART DEFINED HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!
        if (DeltaTS >= EVENT_TIME_WINDOW) {EndEvent=true;} //Force event to end no matter what.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//Either the prompt trigger failed, or the next timestamp is out of the time window range
	if (EndEvent)
		{
		if (PromptTrigger) //THEN WRITE OUT THE EVENT
			{
			// WRITE THE EVENT OUT HERE TO THE NEW TREE
			size=RawMult;
      // print(timestamps[RawMult]);
			// if (size < 256) {outputtree->Fill();}
			// else {cout << "WARNING, very long event!";}
    }
		//Clear everything
		RawMult=0;
		PromptMult=0;
    PromptGeMult=0;
		// DelayedMult=0;
		// DelayedGeMult=0;
		PromptTrigger=false;
		EndEvent=false;
		EndOfPrompt=false;
		PromptTriggerCalculated=false;
		// TotalEnergy=0;
		// BGOEnergy=0;
		// GeEnergy=0;
		LaEnergy=0;
		LaMult=0;
		TotalMult=0;
		// TRY AND START A NEW EVENT WITH THIS ENERGY. New StartTimeStamp
		StartTimeStamp=(timestamp-tdifps); //Get the "zero" value for this event
                DeltaTS=(timestamp-StartTimeStamp)/1000.0;//Caclulate the "new" DeltaTS for this hit

                //Use the new hit that ended the old event to try and start a new event
		if (DeltaTS < 700)
			{
			// energies[RawMult]=cnrj;
			timestamps[RawMult]=DeltaTS;
			ids[RawMult]=detid;
			// pups[RawMult]=pileup;
			RawMult++;
                        if (is_Clover_Ge(detid)) {PromptGeMult++; PromptMult++;}
			if (is_LaBr3(detid) && laprompt) {LaMult++; LaEnergy+=cnrj; PromptMult++;}
                        // if (isbgo[detid]) {PromptMult++;}
			}
		}
} //End of test for good detector

        // ******************** //
        // -------------------- //
        //     FIN CODE JON     //
        // -------------------- //
        // ******************** //
}
*/

#endif //NEARLINE_H
