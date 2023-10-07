#ifndef RUNMATRIXATOR_HPP
#define RUNMATRIXATOR_HPP

#include "../libCo.hpp"
#include "../Analyse/Clovers.hpp"

#include "../Classes/Detectors.hpp"

#include "../MTObjects/MTRootReader.hpp"
#include "../MTObjects/MTTHist.hpp"

/**
 * @brief Matrixator is used to create a bunch of matrixes : each crystal of the setup versus the (add-backed & compton cleaned) clovers
 * 
 */
class RunMatrixator
{
public:
  RunMatrixator(std::string const & runpath);
  void Initialize();
  void matrixate_file_with_clovers(TTree * tree, Event & event);
  void Write();

private:
  static void dispatch_thread(TTree * tree, Event & event, RunMatrixator & rm);
  std::unordered_map<dType, Vector_MTTHist<std::vector<TH1F>>> matrices;
  std::string const & m_runpath;
};

RunMatrixator::RunMatrixator(std::string const & runpath) : m_runpath(runpath)
{
  // Initialise the matrices :
  this -> Initialize();

  // Fills the matrices :
  MTRootReader mt_reader(m_runpath);
  mt_reader.execute(dispatch_thread, *this);

  // Writes the matrices down :
  this -> Write();
}

void RunMatrixator::Initialize()
{
  if (!detectors) {print("Please initialize the Detectors"); throw_error("Detectors not loaded");}
  for (auto const & type : detectors.typesArray())
  {// Loop through all the detector types (ge, labr...)
    auto & type_matrices = matrices[type];
    type_matrices.resize(detectors.nbOfType(type));
    int index_in_type = 0;
    for(auto & matrice : type_matrices)
    {
      auto const & name = detectors.name(type, index_in_type);
      auto const & binning_it = Detectors::energy_bidim_bins.find(type);
      if (binning_it ==  Detectors::energy_bidim_bins.end()) throw_error("NO BINNING FOUND FOR "+type+" !!");
      auto const & binning = binning_it->second;
      print(name, binning);
      matrice.reset(name.c_str(), name.c_str(),
          10000,0,10000, binning.bins,binning.min,binning.max);
      index_in_type++;
    }
  }
}

void RunMatrixator::dispatch_thread(TTree * tree, Event & event, RunMatrixator & rm)
{
  rm.matrixate_file_with_clovers(tree, event);
}

void RunMatrixator::matrixate_file_with_clovers(TTree * tree, Event & event)
{
  Clovers clovers;
  auto const & entries = tree -> GetEntries();
  for (longlong event_i = 0; event_i<entries; event_i++)
  {
    tree -> GetEntry(event_i);
    clovers.SetEvent(event);
    // Looping through the clean clovers (add-back + compton clean)
    for (size_t hit_i = 0; hit_i<clovers.CleanGe.size(); hit_i++)
    {
      auto const & clover_index_i = clovers.CleanGe[hit_i]; // The index of the clover (between 0 and 23)
      auto const & clover = clovers[clover_index_i]; // The clover module itself
      auto const & nrj_clover = clover.nrj;

      // Looping through all the crystals :
      for(int hit_j = 0; hit_j<event.mult; hit_j++)
      {
        auto const & label = event.labels[hit_j];
        auto const & type = detectors.type(label);
        auto const & index_j = detectors.index(label);

        auto  & matrice = matrices[type][index_j];

        auto const & nrj = event.nrjs[hit_j];
        auto const & nrj2 = event.nrj2s[hit_j];

        if (isGe[label]) 
        {
          auto const & clover_index_j = Clovers::label_to_clover(label);
          if (clover_index_i != clover_index_j) continue; // Reject the hit if is locate in the same clover as the clover (reject diagonal Ge)
          if (clover.nb>1) continue; // Reject if there are more than one germanium that fired (reject add-back events in the clover holding the crystal)
        }
        matrice->Fill(nrj_clover, nrj);
      }
    }
  }
}

void RunMatrixator::Write()
{
  Path runPath(m_runpath);
  std::string const & outRoot = runPath.folder().string()+"_matrixated.root";
  unique_TFile file(TFile::Open(outRoot.c_str(), "RECREATE"));
  file -> cd();
    for (auto & type_matrice : matrices) for (auto & matrice : type_matrice.second) matrice->Write();
  file -> Write();
  file -> Close();
}

#endif //RUNMATRIXATOR_HPP