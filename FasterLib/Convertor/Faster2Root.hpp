#ifndef FASTER2ROOT_HPP
#define FASTER2ROOT_HPP

#include <Calibration.hpp>
#include <libCo.hpp>
#include <DetectorsList.hpp>
#include <FilesManager.hpp>
#include <Timeshifts.hpp>
#include <Manip.hpp>
#include <RF_Manager.hpp>
#include <EventBuilder.hpp>
#include <Timer.hpp>
#include <MTCounter.hpp>
#include <MTList.hpp>
#include <Counters.hpp>
#include <Detectors.hpp>
#include <RF_Manager.hpp>

class Faster2Root
{
public:
  Faster2Root
  (
    DetectorsList const & detectorList, 
    std::string   const & timeshifts_file = "", 
    std::string   const & calib_file = "", 
    std::string   const & folder = "", 
    std::string   const & outfolder = "", 
    ushort        const & nb_threads = 2
  );
  void convert(ushort const & nb_threads);
  static void treatFilesMT(MTList<std::string> & files_list, Faster2Root * f2r);
  void treatFile(std::string const & filename);

  void setListDet(std::string const & filename) {m_detList = filename;}
  void setListDet(DetectorsList const & detectorList) {m_detList = detectorList;}
  void setRunPath(std::string const & datapath);
  void setOutDir(std::string const & outfolder);

  bool loadTimeshifts(std::string const & ts_filename) {return m_timeshifts.load(ts_filename);}
  bool loadCalibration(std::string const & calib_filename) {return m_calib.load(calib_filename, m_detList.size());}
  
  bool calculateTimeshifts(std::string const & parameters, int const & nb_files = -1) 
  {
    m_timeshifts.setParameters(parameters);
    return m_timeshifts.calculate(m_runpath, nb_files);
  }

  FilesManager * p_files  () {return &m_files     ;}
  DetectorsList* p_detList() {return &m_detList   ;}
  Timeshifts   * p_ts     () {return &m_timeshifts;}
  Calibration  * p_calib  () {return &m_calib     ;}

  bool const & ok () const {return m_ok;}
  void verbose(bool const & _verbose = true) {m_verbose = _verbose;} 

private:
  bool m_ok = true;
  bool m_verbose = false;

  Timer m_timer;

  DetectorsList m_detList;
  FilesManager  m_files;
  Timeshifts    m_timeshifts;
  Calibration   m_calib;

  std::string m_runpath = "";
  std::string m_outDir = "";

  MTCounter  m_fr_raw_run_size;
  MTCounter  m_fr_treated_run_size;
  MTCounter  m_fr_raw_counter;
  MTCounter  m_fr_treated_counter;
  MTCounter  m_reading_rate;
  MTCounter  m_converting_rate;
  MTCounter  m_writting_rate;
  float m_size_data = 0;
};

void Faster2Root::setRunPath(std::string const & runpath)
{
  if (!folder_exists(runpath, true)) m_ok = false;
  m_runpath = runpath;
}

Faster2Root::Faster2Root
(
  DetectorsList const & detectorList, 
  std::string   const & timeshifts_file, 
  std::string   const & calib_file, 
  std::string   const & datafolder, 
  std::string   const & outfolder, 
  ushort        const & nb_threads
)
{
  m_detList = detectorList;
  m_timeshifts.setDetectorsList(m_detList);
  m_calib.setDetectorsList(&m_detList);
  if (timeshifts_file!="") m_timeshifts.load(timeshifts_file);
  if (calib_file!="") m_calib.load(calib_file, m_detList.size());
  if (outfolder!= "") this -> setOutDir(outfolder);
  if (datafolder != "") 
  {
    this -> setRunPath(datafolder);
    convert(nb_threads);
  }
}

void Faster2Root::convert(ushort const & nb_threads = 2)
{
  print("_________");
  if (m_runpath=="") {print("Nothing to convert"); return;}
  m_timer.Restart();
  if (!m_ok) {print("Faster2Root module not correctly initialized !"); return;}

  m_files.addFolder(m_runpath);

  std::string run_name = m_files.getListFolders()[0];
  run_name.pop_back();
  run_name = rmPathAndExt(run_name);
  
  print("Conversion of ", run_name);

  m_size_data = m_files.diskSize();

  if (nb_threads>1 && !MTObject::ON) MTObject::Initialize(nb_threads);

  if (MTObject::ON)
  {
    MTList<std::string> filesMT(m_files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, filesMT, this);
  }
  else 
  {
    std::string filename;
    while(m_files.nextFileName(filename))
    {
      this -> treatFile(filename);
    }
    print("Worker", MTObject::getThreadIndex(), "finished");
  }

  m_timer.Stop();
  std::ofstream outfile("log.log",std::ios::app);
  auto const rate = m_fr_raw_run_size / m_timer.TimeElapsedSec();
  std::cout << std::setprecision(3);

  print(run_name, "treated at", (int)(rate*1.E-6), "Mo/sec |",
        "Reading :"    , MTObject::getThreadsNb() * (m_reading_rate    / (float)(m_files.size())),"Mo/s |",
        "Converting : ", MTObject::getThreadsNb() * (m_converting_rate / (float)(m_files.size())),"Mo/s |",
        "Writting : "  , MTObject::getThreadsNb() * (m_writting_rate / (float)(m_files.size()))  ,"Mo/s"
      );
  outfile << run_name << ": CompressionFactor: " << m_fr_raw_run_size/m_fr_treated_run_size << " RawCounter: " << m_fr_raw_counter
  << " TreatedCounter: " << m_fr_treated_counter << std::endl;
  outfile.close();

  print("________");
  print();
}

void Faster2Root::treatFilesMT(MTList<std::string> & files_list, Faster2Root * f2r)
{
  std::string filename;
  while(files_list.getNext(filename))
  {
    f2r->treatFile(filename);
  }
}

void Faster2Root::treatFile(std::string const & filename)
{
  // The out .root file name should have the same name as the .fast file
  std::string outfile = m_outDir+rmPathAndExt(filename)+".root";
  if ( file_exists(outfile) ) {print(outfile, "already exists !");return;}
  
  Hit hit;
 
  Timer time_read;
  FasterReader reader(&hit, filename);
  if (!reader.isReady()) { print("CAN'T READ", filename); return;}

  auto file_size = size_file(filename);
  m_fr_raw_run_size+=file_size;

  auto rootTreeName = "tempTree"+std::to_string(MTObject::getThreadIndex());
  std::unique_ptr<TTree> rootTree (new TTree(rootTreeName.c_str(), rootTreeName.c_str()));
  rootTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  rootTree -> Branch("label"  , &hit.label );
  rootTree -> Branch("time"   , &hit.time  );
  rootTree -> Branch("nrjcal" , &hit.nrjcal);
#ifdef QDC2
  rootTree -> Branch("nrj2"   , &hit.nrj2);
#endif //QDC2
  rootTree -> Branch("pileup" , &hit.pileup);

  // Convert the raw faster data to raw root tree
  int counter = 0;
  // while(reader.Read() && counter<20000)
  while(reader.Read())
  {
    counter++;
    hit.time+=m_timeshifts[hit.label];
    m_calib.calibrate(hit);
    rootTree -> Fill();
  }

  m_fr_raw_counter+=counter;

  time_read.Stop();

  ULong64_t nb_data = rootTree -> GetEntries();
  // Align in time after the timeshift (can shuffle some hits)
  std::vector<int> gindex(nb_data);
  std::iota(std::begin(gindex), std::end(gindex), 0); // Fill with 0,1,2,...,nb_data
  alignator(rootTree.get(), gindex.data());

  Event buffer;
  Timer time_conv;

  // Sets the output tree :
  std::unique_ptr<TTree> outTree (new TTree("Nuball", "DataTreeEventBuild C1L2 C2"));
  outTree -> SetDirectory(nullptr);
  outTree -> Branch("mult",   &buffer.mult);
  outTree -> Branch("label",  &buffer.labels , "label[mult]/s" );
  outTree -> Branch("nrj",    &buffer.nrjs   , "nrj[mult]/F"   );
#ifdef QDC2
  outTree -> Branch("nrj2",   &buffer.nrj2s  , "nrj2[mult]/F"  );
#endif //QDC2
  outTree -> Branch("time",   &buffer.times  , "time[mult]/l"  );
  outTree -> Branch("pileup", &buffer.pileups, "pileup[mult]/O");
#ifdef TIMREF_DSSD
  outTree -> Branch("Time",   &buffer.time2s , "Time[mult]/F"  );
  ULong64_t dssd_timeRef = 0;
#endif //TIMREF_DSSD

  // Reset the branch address and set the temporary tree in reading mode
  rootTree -> ResetBranchAddresses();
  hit.reset();
  rootTree -> SetBranchAddress("label"  , &hit.label );
  rootTree -> SetBranchAddress("time"   , &hit.time  );
  rootTree -> SetBranchAddress("nrjcal" , &hit.nrjcal);
#ifdef QDC2
  rootTree -> SetBranchAddress("nrj2"   , &hit.nrj2);
#endif //QDC2
  rootTree -> SetBranchAddress("pileup" , &hit.pileup);
  rootTree -> SetBranchAddress("time"   , &hit.time  );

  std::size_t loop = 0;

#ifdef DEBUG
  print("reading ready");
#endif //DEBUG


#ifdef USE_RF

  // Remove the hits previous to the first RF measurement :
  RF_Manager rf;
  EventBuilder event(&buffer, &rf);
  event.setShift(static_cast<Long64_t>(USE_RF));
  do{ rootTree -> GetEntry(gindex[loop++]);}
  while(hit.label != RF_Manager::label && loop<nb_data);
  if (loop == nb_data) {print("NO RF DATA FOUND !"); return;}

  // Extract the first RF data and write it down:
  rf.setHit(hit);
  event.setFirstRF(hit);
  buffer = hit;
  outTree -> Fill();
  buffer.clear();

#else // NO RF
  CoincBuilder2 event(&buffer);

#endif //USE_RF

  // Handle the first hit :
  rootTree -> GetEntry(gindex[loop++]);
  event.set_last_hit(hit);

  // ULong64_t evt_start = loop;
  // while (loop<evt_start+1000)
  Counters Counter;
  loop = 0;
  while (loop<nb_data)
  {
    rootTree -> GetEntry(gindex[loop++]);

  #ifdef USE_RF
    if (isRF[hit.label])
    {// To force RF writting in the data
      auto temp = buffer;
      buffer = hit;
      outTree -> Fill();
      buffer = temp;
      rf.setHit(hit);
      continue;
    }
  #endif //USE_RF

    if (event.build(hit))
    {
      Counter.count_event(buffer);

    #ifdef TIMREF_DSSD
      if (Counter.DSSDMult > 0)
      {
        for (size_t i = 0; i<buffer.size(); i++) if (isDSSD[buffer.labels[i]]) {dssd_timeRef = buffer.times[i]; break;}
        for (size_t i = 0; i<buffer.size(); i++) buffer.time2s[i] = static_cast<float>(static_cast<Long64_t>(buffer.times[i]-dssd_timeRef))/1000.;
        outTree -> Fill();
      }

    #else //NOT TIMREF_DSSD
      if(Counter.DSSDMult > 0 || (Counter.RawGe > 0 && Counter.Modules > 1))
      {
        outTree -> Fill();
      }

    #endif //TIMREF_DSSD
    }
  }

  time_conv.Stop();

  Timer time_write;

  std::unique_ptr<TFile> outFile (new TFile(outfile.c_str(),"create"));
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();

  time_write.Stop();

  m_fr_treated_run_size+=size_file(outFile->GetName());
  m_fr_treated_counter += outTree -> GetEntries();

  m_reading_rate+=1.E-3*file_size/time_read.TimeElapsed();
  m_converting_rate+=1.E-3*file_size/time_conv.TimeElapsed();
  m_writting_rate+=1.E-3*outFile->GetSize()/time_conv.TimeElapsed();

  auto const totalTimeSec = time_read.TimeElapsedSec()+time_conv.TimeElapsedSec();
  std::cout << std::setprecision(3);
  MTObject::shared_mutex.lock();
  if (m_verbose) print
  (
    rmPathAndExt(filename), "->",
    "Total :",      1.E-6*file_size, "Mo    in", totalTimeSec, "s  (", 1.E-6*file_size/totalTimeSec, "Mo/s ) |",
    "Reading :"   , 1.E-6*nb_data,   "Mevts in", time_read.TimeElapsedSec(),  "s |",
    "Converting :", 1.E-6*nb_data,   "Mevts to", 1.E-6*outTree->GetEntries(), "Mevts ( factor", nb_data/outTree->GetEntries(), ")"
  );
  else print(rmPathAndExt(filename), ":",100*(m_fr_raw_run_size/m_size_data),"%");
  MTObject::shared_mutex.unlock();
}

void Faster2Root::setOutDir(std::string const & outfolder)
{
  m_outDir = outfolder;
  print(outfolder);
  create_folder_if_none(m_outDir);
  if (!folder_exists(m_outDir)) {print("Can't create the output", m_outDir, "folder!");return;}
}

#endif //FASTER2ROOT_HPP
