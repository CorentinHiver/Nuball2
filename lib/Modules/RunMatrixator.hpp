#ifndef RUNMATRIXATOR_HPP
#define RUNMATRIXATOR_HPP

#define USE_RF 200

#include "../libCo.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/Alignator.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Timer.hpp"

#include "../Analyse/Clovers.hpp"

#include "../../136/conversion_200ns/EventBuilder_136.hpp"

#include "../Modules/Calibration.hpp"
#include "../Modules/Timeshifts.hpp"

#include "../MTObjects/MTRootReader.hpp"
#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"

/**
 * @brief Matrixator is used to create a bunch of matrixes : each crystal of the setup versus the (add-backed & compton cleaned) clovers
 * 
 */
class RunMatrixator
{
public:
  RunMatrixator() = default;
  void run(std::string const & runpath, std::string const & data = "fast");
  void Initialize();
  void loadData_root(TTree * tree, Event & event);
  void loadData_faster(Hit & hit, FasterReader & reader);
  void fillMatrixes(Clovers const & clovers, Event const & event);
  void Write();
  auto parisRatio(NRJ const & nrj, NRJ const & nrj2) {return (nrj2-nrj)/nrj2;}
  bool parisIsLaBr(NRJ const & nrj, NRJ const & nrj2) 
  {
    auto const & ratio = parisRatio(nrj,nrj2);
    return (ratio>-0.1 && ratio<0.2);
  }
  bool parisIsNaI(NRJ const & nrj, NRJ const & nrj2)  
  {
    auto const & ratio = parisRatio(nrj,nrj2);
    return (ratio>-0.55 && ratio<0.75);
  }

  void setCalibration(Calibration const & calibration) {m_calibration = calibration;}
  void setTimeshifts(Timeshifts const & timeshifts) {m_timeshifts = timeshifts;}

  void dontMatrixate(dType const & type)
  {
    // Initialise if not :
    if (m_dontMatrixateLabel.size()<detectors.size())  m_dontMatrixateLabel.resize(detectors.size(), false);
    if (m_dontMatrixateType.size() < detectors.nbTypes()) for (auto const & type : detectors.types()) m_dontMatrixateType.emplace(type, false);

    // Check if the type exists :
    if (!found(detectors.types(), type)) throw_error("RunMatrixator::dontMatrixate(type) : type unkown");

    // Fill the map :
    m_dontMatrixateType[type] = true;
    auto const & typesArray = detectors.typesArray();
    for (size_t label = 0; label<typesArray.size(); label++) if (typesArray[label] == type) m_dontMatrixateLabel[label] = true;
  }

  void dontMatrixate(Label const & label)
  {
    // Initialise if not :
    if (m_dontMatrixateLabel.size()<detectors.size())  m_dontMatrixateLabel.resize(detectors.size(), false);
    // Check entry :
    if (label>m_dontMatrixateLabel.size()) throw_error("RunMatrixator::dontMatrixate(label) : label out of range");
    m_dontMatrixateLabel[label] = true;
  }

  void keepSingles(bool const & b = true) {m_keep_singles = b;}
  void maxRawMult(uchar const & max_multiplicity = 20) {m_max_multiplicity = max_multiplicity;}

private:
  static void dispatch_root_reader(TTree * tree, Event & event, RunMatrixator & rm);
  static void dispatch_faster_reader(Hit & hit, FasterReader & reader, RunMatrixator & rm);
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matricesPrompt;   // Normal prompt matrices
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matricesPrompt2;  // Used to matrixate nrj2 if needed (Paris, Eden...)
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matricesDelayed;  // Normal delayed matrices
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matricesDelayed2; // Used to matrixate nrj2 if needed (Paris, Eden...)
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singlesPrompt;    // Normal singles spectra
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singlesPrompt2;   // To histogram nrj2 if needed (Paris, Eden...)
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singlesDelayed;   // Normal singles spectra
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singlesDelayed2;  // To histogram nrj2 if needed (Paris, Eden...)
  MTTHist<TH2F> test_paris_vs_mult;
  MTTHist<TH2F> matrix_Clovers_prompt;
  MTTHist<TH2F> matrix_Clovers_delayed;
  MTTHist<TH2F> matrix_Clovers_delayed_vs_prompt;
  Calibration m_calibration;
  Timeshifts m_timeshifts;
  Path m_runpath;

  // Parameters :
  std::unordered_map<dType, bool> m_dontMatrixateType;
  std::vector<Label> m_dontMatrixateLabel;
  bool m_keep_singles = 0;// Keep events with only one hit for the singles spectra
  uchar m_max_multiplicity = 20;
};

void RunMatrixator::run(std::string const & runpath, std::string const & data)
{
  Timer timer;
  m_runpath = runpath;
  // Initialise the matrices :
  this -> Initialize();

  // Fills the matrices :
  if (data == "fast")
  {
    if (!m_calibration) throw_error("NO CALIBRATION");
    if (!m_timeshifts)  throw_error("NO TIMESHIFTS" );

    MTFasterReader mt_reader(m_runpath);
    mt_reader.execute(dispatch_faster_reader, *this);
  }
  else if (data == "root")
  {
    MTRootReader mt_reader(m_runpath);
    mt_reader.execute(dispatch_root_reader, *this);
  }
  else throw_error("Unkown data kind ! Only handles root and fast");
  
  print("Reading done in", timer(), timer.unit());

  // Writes the matrices down :
  this -> Write();
  print(timer(), timer.unit(), "to matrixate", m_runpath);
}

void RunMatrixator::Initialize()
{
  // Hit::setExternalTime(true); // Allows to directly calculate the rf time instead of the relative time
  if (!detectors) {print("Please initialize the Detectors"); throw_error("Detectors not loaded");}
  if (m_keep_singles) Builder::keepSingles();

  test_paris_vs_mult.reset("test_paris_vs_mult", "Paris vs Mult", m_max_multiplicity,0,m_max_multiplicity, 1000,0,10000);

  // --- Clover bidims initialisation : --- //
  matrix_Clovers_prompt.reset("pp","matrix_Clovers_prompt;Clover E [keV];Clover E [keV]", 10000,0,10000, 10000,0,10000);
  matrix_Clovers_delayed.reset("dd","matrix_Clovers_delayed;Clover E [keV];Clover E [keV]", 10000,0,10000, 10000,0,10000);
  matrix_Clovers_delayed_vs_prompt.reset("dp","Clovers delayed VS prompt;Prompt E [keV];Delayed E [keV]", 10000,0,10000, 10000,0,10000);

  // --- All the matrices and single spectra initialisation : --- //
  for (auto const & type : detectors.types())
  {// Loop through all the detectors types (ge, labr...)
    print("Initialize ",type);
    auto const & nb_det = detectors.nbOfType(type);

    // Matrices : 
    if (!m_dontMatrixateType[type])
    {
      matricesPrompt [type].resize(nb_det);
      matricesDelayed[type].resize(nb_det);
    }
    
    // Singles : 
    singlesPrompt [type].resize(nb_det);
    singlesDelayed[type].resize(nb_det);

    // QDC2 :
    if (type == "paris")
    {
      // Matrices : 
      if (!m_dontMatrixateType[type])
      {
        matricesPrompt2 [type].resize(nb_det);
        matricesDelayed2[type].resize(nb_det);
      }

      // Singles : 
      singlesPrompt2 [type].resize(nb_det);
      singlesDelayed2[type].resize(nb_det);
    }

    for(size_t type_i = 0; type_i<nb_det; type_i++)
    {
      auto const & name = detectors.name(type, type_i);
      auto const & label = detectors.label(type, type_i);

      auto const & binning_bidim = detectors.energyBidimBin(type);
      auto const & binning_energy = detectors.energyBin(type);

      // Matrices :
      if (!m_dontMatrixateLabel[label])
      {
        matricesPrompt [type][type_i].reset((name+"_prompt").c_str(),  (name+" Prompt;Clover [keV];" +name+" energy [keV]").c_str(), 10000,0,10000, binning_bidim.bins,binning_bidim.min,binning_bidim.max);
        matricesDelayed[type][type_i].reset((name+"_delayed").c_str(), (name+" Delayed;Clover [keV];"+name+" energy [keV]").c_str(), 10000,0,10000, binning_bidim.bins,binning_bidim.min,binning_bidim.max);
      }
      
      // Singles :
      singlesPrompt [type][type_i].reset((name+"_prompt_singles").c_str(),  (name+" Prompt singles;Clover [keV];" +name+" energy [keV]").c_str(), binning_energy.bins,binning_energy.min,binning_energy.max);
      singlesDelayed[type][type_i].reset((name+"_delayed_singles").c_str(), (name+" Delayed singles;Clover [keV];"+name+" energy [keV]").c_str(), binning_energy.bins,binning_energy.min,binning_energy.max);

      // QDC2 :
      if (type == "paris")
      {
        // Matrices :
        if (!m_dontMatrixateLabel[label])
        {
          matricesPrompt2 [type][type_i].reset((name+"_prompt_dqc2").c_str(),  (name+" Prompt QDC2;Clover [keV];" +name+" QDC2 [keV]").c_str(), 10000,0,10000, binning_bidim.bins,binning_bidim.min,binning_bidim.max);
          matricesDelayed2[type][type_i].reset((name+"_delayed_dqc2").c_str(), (name+" Delayed QDC2;Clover [keV];"+name+" QDC2 [keV]").c_str(), 10000,0,10000, binning_bidim.bins,binning_bidim.min,binning_bidim.max);
        }
        
        // Singles :
        singlesPrompt2 [type][type_i].reset((name+"_prompt_singles_dqc2").c_str(),  (name+" Prompt singles QDC2;Clover [keV];" +name+" QDC2 [keV]").c_str(), binning_energy.bins,binning_energy.min,binning_energy.max);
        singlesDelayed2[type][type_i].reset((name+"_delayed_singles_dqc2").c_str(), (name+" Delayed singles QDC2;Clover [keV];"+name+" QDC2 [keV]").c_str(), binning_energy.bins,binning_energy.min,binning_energy.max);
      }
    }
  }
}

void RunMatrixator::dispatch_faster_reader(Hit & hit, FasterReader & reader, RunMatrixator & rm)
{
  rm.loadData_faster(hit, reader);
}

void RunMatrixator::dispatch_root_reader(TTree * tree, Event & event, RunMatrixator & rm)
{
  rm.loadData_root(tree, event);
}

void RunMatrixator::loadData_faster(Hit & hit, FasterReader & reader)
{
  unique_tree tempTree(new TTree("tempTree", "tempTree"));
  hit.writting(tempTree.get(), "lstEQp");
  
  while(reader.Read())
  {
    auto const & label = hit.label;
    hit.stamp+=m_timeshifts[label];
    hit.nrj = m_calibration.calibrate(hit.adc, label);
    hit.nrj2 = (hit.qdc2) ? m_calibration.calibrate(hit.qdc2, label) : 0;

    tempTree->Fill();
  }

  Alignator aligned_index(tempTree.get());

  hit.reset();
  hit.reading(tempTree.get());

  RF_Manager rf;
  Event event;
  EventBuilder_136 builder(&event, &rf);

  RF_Extractor first_rf(tempTree.get(), rf, hit, aligned_index);
  if (!first_rf) return;
  builder.setFirstRF(hit);

  auto const & nb_hits = tempTree->GetEntries();
  int loop = first_rf.cursor();

  Clovers clovers;

  while (loop<nb_hits)
  {
    tempTree->GetEntry(aligned_index[loop++]);

    if (rf.setEvent(hit)) continue;

    if (builder.build(hit))
    {
      rf.align_to_RF_ns(event);
      clovers.SetEvent(event, 2);
      this -> fillMatrixes(clovers, event);
    }
  } 
}

void RunMatrixator::loadData_root(TTree * tree, Event & event)
{
  Clovers clovers;
  auto const & entries = tree -> GetEntries();
  for (longlong event_i = 0; event_i<entries; event_i++)
  {
    tree -> GetEntry(event_i);
    clovers.SetEvent(event, 2);
    this -> fillMatrixes(clovers, event);
  }
}

void RunMatrixator::fillMatrixes(Clovers const & clovers, Event const & event)
{
  if (event.mult > m_max_multiplicity) return;

  // For Ge and BGO : 
  bool prompt = false;
  bool delayed = false;

  // For paris : 
  bool paris_is_labr = false;
  bool paris_is_nai = false;

  // --- Looping through all the crystals : --- //
  for(int hit_i = 0; hit_i<event.mult; hit_i++)
  {
    auto const & label = event.labels[hit_i];
    if (!detectors.exists(label) || event.pileups[hit_i]) continue;
    auto const & time_i = event.time2s[hit_i];
    auto const & nrj = event.nrjs[hit_i];
    auto const & nrj2 = event.nrj2s[hit_i];

    auto const & type = detectors.type(label);
    auto const & index_i = detectors.index(label);

    auto & matricePrompt = matricesPrompt[type][index_i];
    auto & matriceDelayed = matricesDelayed[type][index_i];
    auto & singlePrompt = singlesPrompt[type][index_i];
    auto & singleDelayed = singlesDelayed[type][index_i];

    prompt = (time_i>-10) && (time_i<7);
    delayed = (time_i>60) && (time_i<160);

    if (!(prompt || delayed)) continue;

    // Correct handling of Paris :
    if (isParis[label])
    {
      paris_is_labr = parisIsLaBr(nrj, nrj2);
      paris_is_nai = parisIsNaI(nrj, nrj2);
      if (paris_is_labr) 
      {
        if (prompt) 
        {
          test_paris_vs_mult.Fill(event.mult, nrj);
          singlePrompt.Fill(nrj);
        }
        else if (delayed) singleDelayed.Fill(nrj);
      }
      else if(paris_is_nai) 
      {
        if (prompt) singlesPrompt2[type][index_i].Fill(nrj2);
        else if (delayed) singlesDelayed2[type][index_i].Fill(nrj2);
      }
      else continue; // Reject intermediate structures 
    }

    // All the other kind of detectors :
    else
    {
           if (prompt ) singlePrompt .Fill(nrj);
      else if (delayed) singleDelayed.Fill(nrj);
    }

    if(m_dontMatrixateLabel[label]) continue;

    // Looping through the prompt clovers (add-backed, no compton clean)
    for (auto const & clover_index : clovers.promptClovers)
    {
      // Only doing prompt-prompt matrices :
      if (!prompt) continue; 

      auto const & clover = clovers.PromptClovers[clover_index];
      auto const & nrj_clover = clover.nrj;
      if (clover.nb_BGO>0) continue; // Compton cleaning done here 

      // Reject the hit if is located in the same clover as the ref clover (= reject diagonal Ge)
      if (isClover[label]) {if (clover_index == Clovers::label_to_clover(label) || !prompt) continue; }
      else if (isParis[label])
      {
        if (paris_is_labr) matricePrompt.Fill(nrj_clover, nrj); // Fill the LaBr3 part
        else if (paris_is_nai) matricesPrompt2[type][index_i].Fill(nrj_clover, nrj2);// Fill the NaI part
      }
      matricePrompt.Fill(nrj_clover, nrj);
    }

    // Looping through the delayed clovers (add-backed, no compton clean)
    for (auto const & clover_index : clovers.delayedClovers)
    {
      // Only doing delayed-delayed matrices :
      if (!delayed) continue;

      auto const & clover = clovers.DelayedClovers[clover_index];
      auto const & nrj_clover = clover.nrj;
      if (clover.nb_BGO>0) continue; // Compton cleaning done here 

      // Reject the hit if is located in the same clover as the ref clover (= reject diagonal Ge) :
      if (isClover[label]) {if (clover_index == Clovers::label_to_clover(label)) continue; }
      else if (isParis[label])
      {
        if (paris_is_labr) matriceDelayed.Fill(nrj_clover, nrj); // Fill the LaBr3 part
        else if (paris_is_nai) matricesDelayed2[type][index_i].Fill(nrj_clover, nrj2);// Fill the NaI part
        continue;
      }
      matriceDelayed.Fill(nrj_clover, nrj);
    }
  }

  // --- Creating clover prompt matrices : --- //
  for (size_t prompt_i = 0; prompt_i<clovers.promptClovers.size(); prompt_i++) 
  {
    // Extract the prompt clover :
    auto const & clover_prompt_i = clovers.promptClovers[prompt_i];
    auto const & clover_prompt = clovers.PromptClovers[clover_prompt_i];
    if (clover_prompt.nb_BGO>0) continue;

    // Delayed VS prompt :
    for (auto const & clover_index_delayed : clovers.delayedClovers)
    {
      auto const & clover_delayed = clovers.DelayedClovers[clover_index_delayed];
      if (clover_delayed.nb_BGO>0) continue;
      matrix_Clovers_delayed_vs_prompt.Fill(clover_prompt.nrj, clover_delayed.nrj);
    }

    // Prompt-prompt :
    for (size_t prompt_j = prompt_i+1; prompt_j<clovers.promptClovers.size(); prompt_j++)
    {
      auto const & clover_index_prompt_j = clovers.promptClovers[prompt_j];
      auto const & clover_prompt_j = clovers.PromptClovers[clover_index_prompt_j];
      matrix_Clovers_prompt.Fill(clover_prompt.nrj, clover_prompt_j.nrj);
    }
  }

  // --- Creating clover prompt matrices : --- //
  for (size_t delayed_i = 0; delayed_i<clovers.delayedClovers.size(); delayed_i++) 
  {
    // Extract the delayed clover :
    auto const & clover_index_delayed = clovers.delayedClovers[delayed_i];
    auto const & clover_delayed = clovers.DelayedClovers[clover_index_delayed];

    // Delayed-delayed :
    for (size_t delayed_j = delayed_i+1; delayed_j<clovers.delayedClovers.size(); delayed_j++)
    {
      auto const & clover_index_delayed_j = clovers.delayedClovers[delayed_j];
      auto const & clover_delayed_j = clovers.DelayedClovers[clover_index_delayed_j];
      matrix_Clovers_delayed.Fill(clover_delayed.nrj, clover_delayed_j.nrj);
    }
  }
}

void RunMatrixator::Write()
{
  std::string outRoot = m_runpath.folder().string();

  // Mandatory if reading faster files : 
  if (outRoot.back() == '/') outRoot.pop_back();
  outRoot = removeExtension(outRoot);

  outRoot+="_matrixated.root";

  unique_TFile file(TFile::Open(outRoot.c_str(), "RECREATE"));
  try
  {
    if (!file) throw_error("Error");
  }
  catch(std::runtime_error const & error)
  {
    print("Try another output file");
    std::cin >> outRoot;
    file.reset(TFile::Open(outRoot.c_str(), "RECREATE"));
  }
  file -> cd();

  test_paris_vs_mult.Write();
  matrix_Clovers_prompt.Write();
  matrix_Clovers_delayed.Write();
  matrix_Clovers_delayed_vs_prompt.Write();

  // Writting the matrices :
  for (auto & type_matrice : matricesPrompt)   for ( auto & matrice : type_matrice.second ) matrice.Write();
  for (auto & type_matrice : matricesPrompt2)  for ( auto & matrice : type_matrice.second ) matrice.Write();
  for (auto & type_matrice : matricesDelayed)  for ( auto & matrice : type_matrice.second ) matrice.Write();
  for (auto & type_matrice : matricesDelayed2) for ( auto & matrice : type_matrice.second ) matrice.Write();

  // Writting the singles :
  for (auto & type_single : singlesPrompt)   for ( auto & single : type_single.second ) single.Write();
  for (auto & type_single : singlesPrompt2)  for ( auto & single : type_single.second ) single.Write();
  for (auto & type_single : singlesDelayed)  for ( auto & single : type_single.second ) single.Write();
  for (auto & type_single : singlesDelayed2) for ( auto & single : type_single.second ) single.Write();

  file -> Write();
  file -> Close();
  print(outRoot, "written");
}

#endif //RUNMATRIXATOR_HPP




/*

  void dontMatrixate(std::string const & type) 
  {
    // Initialise if not :
    
  }

*/