#ifndef CALIBRATOR_HPP
#define CALIBRATOR_HPP
#include <MTObjects/MultiHist.hpp>
#include "../Classes/Parameters.hpp"

std::string param_string = "Calibrator";

class Calibrator
{
public:

  Calibrator(){for (Label label = 0; label<detectors.size(); label++) print(label, detectors.exists(label));};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitialiseManip();
  static void run(Parameters & p, Calibrator & calibration);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();

private:

  // Parameters
  friend class MTObject;
  std::string outDir  = "136/Calibrator/";
  std::string outRoot = "Calibrator.root";
};

bool Calibrator::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitialiseManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void Calibrator::run(Parameters & p, Calibrator & calibration)
{
  std::string rootfile;
  Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    unique_TFile file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    unique_TTree tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "ln");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      calibration.FillSorted(event_s, event);
      calibration.FillRaw(event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void Calibrator::InitialiseManip()
{
  print("Initialise histograms");
}

void Calibrator::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void Calibrator::FillSorted(Sorted_Event const & event_s, Event const & event)
{
//    for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//    {
//    }
}

void Calibrator::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outDir+outRoot);
 }

bool Calibrator::setParameters(std::vector<std::string> const & parameters)
{
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> outDir;
      else if (temp == "outRoot:")  is >> outRoot;
      else {print("Parameter ", temp, "unkown for prompt gate...");return false;}
    }
  }
  return true;
}

#endif //CALIBRATOR_HPP
