#ifndef CALIBRATIONREADER_H
#define CALIBRATIONREADER_H
#include <libRoot.hpp>
#include <MTTHist.hpp>
#include <Clovers.hpp>
#include <DSSD.hpp>
#include <Paris.hpp>
#include "../Classes/Parameters.hpp"


class CalibrationReader
{
public:

  CalibrationReader(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void treatFile(Parameters & p, CalibrationReader & calibrationreader);
  // void FillRaw(Event const & event);
  void FillSorted(Clovers & clovers, DSSD & dssd, Paris & paris, Event const & event);
  // void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();
private:

  // ---- Parameters ---- //
  std::string param_string = "CalibrationReader";

  // ---- Variables ---- //
  std::string m_outDir  = "136/CalibrationReader/";
  std::string m_outRoot = "CalibrationReader.root";

  // ---- Histograms ---- //
};

bool CalibrationReader::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(treatFile, p, *this);
  this -> Write();
  return true;
}

void CalibrationReader::treatFile(Parameters & p, CalibrationReader & calibrationReader)
{
  std::string rootfile;
  // Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !"); continue;}
    TTree* tree = file->Get<TTree>("Nuball2");
    if (!tree) {print("Nuball2 trees not found in", rootfile ); continue;}
    Event event(tree);
    // unique_tree tree (file->Get<TTree>("Nuball2"));
    // if (!tree.get()) {print("Nuball2 trees not found in",rootfile ); continue;}
    // Event event(tree.get(), "ltqe");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    Clovers clovers;
    DSSD dssd;
    Paris paris;

    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);

      clovers.Reset();
      dssd.Reset();
      paris.Reset();
      for (int hit_i = 0; hit_i<event.mult; hit_i++)
      {
        clovers.Fill(event, hit_i);
        dssd.Fill(event, hit_i);
        paris.Fill(event, hit_i);
      }

      calibrationReader.FillSorted(clovers, dssd, paris, event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/s");
    file -> Close();
  } // End files loop
}

void CalibrationReader::InitializeManip()
{
  // print("Initialize histograms");
}

// void CalibrationReader::FillRaw(Event const & event)
// {
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
// }

void CalibrationReader::FillSorted(Clovers & clovers, DSSD & dssd, Paris & paris, Event const & event)
{
  // prin
}

// void CalibrationReader::FillSorted(Sorted_Event const & event_s, Event const & event)
// {
// //    for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
// //    {
// //    }
// }

void CalibrationReader::Analyse()
{
  
}

void CalibrationReader::Write()
{
  Path (m_outDir, 1);
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  outfile->cd();
  print("Writting histograms ...");
  // Write here the histograms :

  outfile->Write();
  outfile->Close();
  print("Writting analysis in", m_outDir+m_outRoot);
}

bool CalibrationReader::setParameters(std::vector<std::string> const & parameters)
{
  if (parameters.size()<1){print("No Parameters for "+param_string+" !!"); return false;}  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "activated") continue;
      else if (temp == "outDir:")  is >> m_outDir;
      else if (temp == "outRoot:")  is >> m_outRoot;
      else
      {
        print("Parameter", temp, "for", param_string, "unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //CALIBRATIONREADER_H
