#ifndef RUNMATRIXATOR_HPP
#define RUNMATRIXATOR_HPP

#define USE_RF 200

#include "../libCo.hpp"

#include "../Analyse/Clovers.hpp"


#include "../Classes/Detectors.hpp"
#include "../Classes/Alignator.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Timer.hpp"

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

private:
  static void dispatch_root_reader(TTree * tree, Event & event, RunMatrixator & rm);
  static void dispatch_faster_reader(Hit & hit, FasterReader & reader, RunMatrixator & rm);
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matrices;
  std::unordered_map<dType, Vector_MTTHist<TH2F>> matrices2; // Used to matrixate nrj2 if needed (Paris, Eden...)
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singles;
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singlesNoRejection;
  std::unordered_map<dType, Vector_MTTHist<TH1F>> singles2; // To histogram nrj2 if needed (Paris, Eden...)
  Calibration m_calibration;
  Timeshifts m_timeshifts;
  Path m_runpath;
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
  
  // Writes the matrices down :
  this -> Write();
  print(timer(), timer.unit(), "to matrixate", m_runpath);
}

void RunMatrixator::Initialize()
{
  if (!detectors) {print("Please initialize the Detectors"); throw_error("Detectors not loaded");}
  for (auto const & type : detectors.typesArray())
  {// Loop through all the detectors types (ge, labr...)
    auto & type_matrices = matrices[type]; // The normal matrices VS clovers
    auto & type_matrices2 = matrices2[type]; // The QDC2 matrices VS clovers
    auto & type_singles = singles[type]; // The normal singles spectra
    auto & type_singles2 = singles2[type]; // The QDC2 singles spectra
    // auto & type_singles_test = singlesNoRejection[type]; // Test

    // Matrices : 
    type_matrices.resize(detectors.nbOfType(type));
    int index_in_type = 0;
    for(auto & matrice : type_matrices)
    {
      auto const & name = detectors.name(type, index_in_type);
      auto const & binning = detectors.energyBidimBin(type);
      matrice.reset(name.c_str(), (name+";Clover [keV];"+name+" energy [keV]").c_str(), 10000,0,10000, binning.bins,binning.min,binning.max);
      index_in_type++;
    }

    // Matrices of QDC2 :
    if (type == "paris")
    {
      type_matrices2.resize(detectors.nbOfType(type));
      index_in_type = 0;
      for(auto & matrice2 : type_matrices2)
      {
        auto const & name = detectors.name(type, index_in_type);
        auto const & binning = detectors.energyBidimBin(type);
        matrice2.reset((name+"_nrj2").c_str(), (name+" QDC2;Clover [keV];"+name+" QDC2 [keV]").c_str(), 10000,0,10000, binning.bins,binning.min,binning.max);
        index_in_type++;
      }
    }

    // Singles spectra :
    type_singles.resize(detectors.nbOfType(type));
    index_in_type = 0;
    for(auto & singles : type_singles)
    {
      auto const & name = detectors.name(type, index_in_type);
      auto const & binning = detectors.energyBin(type);
      singles.reset((name+"_singles").c_str(), (name+";Clover [keV];"+name+" energy [keV]").c_str(), binning.bins,binning.min,binning.max);
      index_in_type++;
    }

    // type_singles_test.resize(detectors.nbOfType(type));
    // index_in_type = 0;
    // for(auto & single : type_singles_test)
    // {
    //   auto const & name = detectors.name(type, index_in_type);
    //   auto const & binning = detectors.energyBin(type);
    //   single.reset((name+"_singles_test").c_str(), (name+" TEST;Clover [keV];"+name+" energy [keV]").c_str(), binning.bins,binning.min,binning.max);
    //   index_in_type++;
    // }

    // Singles spectra of QDC2 :
    if (type == "paris")
    {
      type_singles2.resize(detectors.nbOfType(type));
      index_in_type = 0;
      for(auto & single : type_singles2)
      {
        auto const & name = detectors.name(type, index_in_type);
        auto const & binning = detectors.energyBin(type);
        single.reset((name+"_singles_qdc2").c_str(), (name+" QDC2;Clover [keV];"+type+" QDC2 [keV]").c_str(), binning.bins,binning.min,binning.max);
        index_in_type++;
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
  hit.writting(tempTree.get(), "lsEQp");

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

  Event event;
  RF_Manager rf;
  EventBuilder_136 builder(&event, &rf);

  RF_Extractor first_rf(tempTree.get(), rf, hit, aligned_index);
  if (!first_rf) return;
  builder.setFirstRF(hit);

  auto const & nb_hits = tempTree->GetEntries();
  int loop = 0;

  Clovers clovers;

  while (loop<nb_hits)
  {
    tempTree->GetEntry(aligned_index[loop++]);

    if (rf.setEvent(hit)) continue;

    if (builder.build(hit))
    {
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
  bool paris_is_labr = false;
  bool paris_is_nai = false;

  // Looping through all the crystals :
  for(int hit_j = 0; hit_j<event.mult; hit_j++)
  {
    auto const & label = event.labels[hit_j];
    if (!detectors.exists(label)) continue;
    auto const & type = detectors.type(label);
    auto const & index_j = detectors.index(label);

    auto & matrice = matrices[type][index_j];
    auto & single = singles[type][index_j];
    // auto & singleTest = singlesNoRejection[type][index_j];

    auto const & nrj = event.nrjs[hit_j];
    auto const & nrj2 = event.nrj2s[hit_j];

    // Reject hit if any other crystal of the clover fired (hard compton cleaning) :
    if (isClover[label]) 
    {
      // if(clovers.PromptClovers[Clovers::label_to_clover(label)].nbCrystals()>1) continue;
      single.Fill(nrj);
    }
    // Correctly handles Paris :
    else if (isParis[label])
    {
      paris_is_labr = parisIsLaBr(nrj, nrj2);
      paris_is_nai = parisIsNaI(nrj, nrj2);
      if (paris_is_labr) single.Fill(nrj);
      else if(paris_is_nai) singles2[type][index_j].Fill(nrj2);
      else continue;
    }
    else single.Fill(nrj);

    // Looping through the raw clovers (no add-back, no compton clean)
    for (auto const & clover_index : clovers.promptClovers)
    {
      auto const & clover = clovers.PromptClovers[clover_index];
      auto const & nrj_clover = clover.nrj;
      if (clover.nb_BGO>0) continue; // BGO clean

      if (isClover[label]) 
      {
        // Reject the hit if is located in the same clover as the clover (reject diagonal Ge)
        if (clover_index == Clovers::label_to_clover(label)) continue; 
        matrice.Fill(nrj_clover, nrj);
      }
      else if (isParis[label])
      {
        if (paris_is_labr) matrice.Fill(nrj_clover, nrj); // Fill the LaBr3 part
        else if (paris_is_nai) matrices2[type][index_j].Fill(nrj_clover, nrj2);// Fill the NaI part
      }
      else matrice.Fill(nrj_clover, nrj);
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
  for (auto & type_matrice : matrices) for (auto & matrice : type_matrice.second) matrice.Write();
  for (auto & type_matrice : matrices2) for (auto & matrice : type_matrice.second) matrice.Write();
  for (auto & type_single : singles) for (auto & single : type_single.second) single.Write();
  for (auto & type_single : singlesNoRejection) for (auto & single : type_single.second) single.Write();
  for (auto & type_single : singles2) for (auto & single : type_single.second) single.Write();
  file -> Write();
  file -> Close();
  print(outRoot, "written");
}

#endif //RUNMATRIXATOR_HPP