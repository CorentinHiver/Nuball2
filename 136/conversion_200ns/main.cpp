// 1. Parameters
#define QDC2
#define USE_RF 200 //ns
#define KEEP_ALL

// 2. Include library

#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp> // Use this to safely count what you want
#include <MTTHist.hpp>   // Use this to safely fill histograms
#include <Timeshifts.hpp>
#include <Calibration.hpp>
#include <Manip.hpp>
#include <Alignator.hpp>
#include <RF_Extractor.hpp>
#include <RF_Manager.hpp>
#include <Detectors.hpp>

#include "EventBuilder_136.hpp"

// 3. Declare some global object

Folder manip_name = "N-SI-136";
std::string time_ref = "301";
std::string timewindow = "1500";
int nb_files_ts = 20;

DetectorsList detList("../NearLine/ID/index_129.list");
Calibration calibration("../NearLine/Calibration/calib_136.cal", detList);

// 4. Declare the function to run on each file in parallel :
void convert(Hit & hit, FasterReader & reader)
{
  // Extracting the run name
  Filename filename = reader.getFilename();
  std::string run_path = filename.path();
  run_path.pop_back();
  std::string run = rmPathAndExt(run_path);

  // Loading the lookup tables 
  Timeshifts timeshifts("N-SI-136-root/Timeshifts/"+run+".dT");
  if (!detList || !timeshifts || !calibration) return;

  // Initialize the temporary TTree :
  std::unique_ptr<TTree> readTree (new TTree("temp","temp"));
  readTree -> SetDirectory(nullptr); // Force it to be created on RAM rather than on disk - much faster if enough RAM
  readTree -> Branch("label"  , &hit.label );
  readTree -> Branch("time"   , &hit.time  );
  readTree -> Branch("nrjcal" , &hit.nrjcal);
  readTree -> Branch("nrj2"   , &hit.nrj2);
  readTree -> Branch("pileup" , &hit.pileup);

  // Loop over the TTree :
  while(reader.Read())
  {
    hit.time+=timeshifts[hit.label];
    hit.nrjcal = calibration(hit.nrj, hit.label);
    readTree -> Fill();
  }

  // Realign switched hits after timeshifts :
  Alignator gindex(readTree.get());

  // Switch the temporary TTree to reading mode :
  hit.reset();
  readTree -> ResetBranchAddresses();
  readTree -> SetBranchAddress("label"  , &hit.label );
  readTree -> SetBranchAddress("time"   , &hit.time  );
  readTree -> SetBranchAddress("nrjcal" , &hit.nrjcal);
  readTree -> SetBranchAddress("nrj2"   , &hit.nrj2);
  readTree -> SetBranchAddress("pileup" , &hit.pileup);
  readTree -> SetBranchAddress("time"   , &hit.time  );

  // Initialize output TTree
  std::unique_ptr<TTree> outTree(new TTree("Nuball2","Nuball2"));
  Event event(outTree.get(),"ltnNp","w");

  // Initialize event builder based on RF
  RF_Manager rf;
  EventBuilder_136 eventBuilder(&event,&rf);
  eventBuilder.setShift(static_cast<Long64_t>(USE_RF));

  int a = 0;
  int* b = &a;

  b = 2;
  
  // Initialize event analyser : simple modules and DSSD counter
  Counter136 counter;

  // Get the first RF downscale and write it down :
  RF_Extractor first_rf(outTree.get(), rf, hit, gindex);
  eventBuilder.setFirstRF(hit);
  event = hit;
  outTree -> Fill();
  event.clear();

  // Handle the first hit :
  int loop = 0;
  readTree -> GetEntry(gindex[loop]);
  eventBuilder.set_last_hit(hit);

  auto const & nb_data = readTree->GetEntries();
  while (loop<nb_data)
  {
    readTree -> GetEntry(gindex[loop++]);

    // Handle the RF data :
    if (hit.label == RF_Manager::label)
    {
      auto temp = event;
      event = hit;
      outTree -> Fill();
      event = temp;
      rf.setHit(hit);
      continue;
    }

    if (eventBuilder.build(hit))
    {
      counter.count(event); 
    #ifdef M2G1_TRIG
      if (counter.nb_modules>2 && counter.clovers.size()>1)
      {
        outTree->Fill();
      }
    #else
      outTree->Fill();
    #endif
    }
  #ifdef KEEP_ALL
    if (eventBuilder.isSingle())
    {
      auto temp = event;
      event = eventBuilder.singleHit();
      outTree -> Fill();
      event = temp;
      continue;
    }
  #endif
  }

  std::string outName = "coucou";
  std::unique_ptr<TFile> outFile (TFile::Open(outName.c_str(), "RECREATE"));
  outFile -> cd();
  outTree -> Write();
  outFile -> Write();
  outFile -> Close();
}

// 5. Main
int main(int argc, char** argv)
{
  if (!detList || calibration) return -1;
  // MANDATORY : initialize the multithreading !
  MTObject::Initialize(3);


  // Setup the path acordingly to the machine :
    Path datapath = std::string(std::getenv("HOME"));
       if ( datapath == "/home/corentin/") datapath+="faster_data/";
  else if ( datapath == "/home/faster/") datapath="srv/data/nuball2/";
  else {print("Unkown HOME path -",datapath,"- please add yours on top of this line ---|---"); return -1;}

  // Load the list of runs to convert :
  Manip runs(datapath+manip_name+"list_runs.list");

  runs.Print();

  // Loop sequentially through the runs and treat its files in parallel
  std::string run;
  while(runs.getNext(run))
  {
    MTFasterReader readerMT(datapath+manip_name+run);
    readerMT.execute(convert);
  }

  return 1;
}
