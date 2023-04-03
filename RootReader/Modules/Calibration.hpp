#ifndef CALIBRATION_H
#define CALIBRATION_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"

std::string param_string = "Calibration";

class Calibration
{
public:

  Calibration(){};
  launch(Parameters & p);
  setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void run(Parameters & p, Calibration & calibration);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();
private:

  // Parameters
  friend class MTObject;
  std::string outDir  = "129/Calibration/";
  std::string outRoot = "Calibration.root";
};

bool Calibration::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void Calibration::run(Parameters & p, Calibration & calibration)
{
  std::string rootfile;
  Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
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

void Calibration::InitializeManip()
{
  print("Initialize histograms");
}

void Calibration::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void Calibration::FillSorted(Sorted_Event const & event_s, Event const & event)
{
//    for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
//    {
//    }
}

void Calibration::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outDir+outRoot);
 }

bool Calibration::setParameters(std::vector<std::string> const & parameters)
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
    else
    {
      print("Parameter", temp, "for Calibration unkown...");
      return false;
    }
  }
}
  return true;

#endif //CALIBRATION_H
