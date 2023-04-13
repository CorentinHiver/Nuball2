#ifndef ANALYSE511_H
#define ANALYSE511_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"


class Analyse511
{

public:
  Analyse511(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void run(Parameters & p, Analyse511 & analyse_511);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();

private:
  // ---- Parameters ---- //
  std::string param_string = "Analyse 511";
  friend class MTObject;
  // ---- Variables ---- //
  std::string m_outDir  = "129/Analyse511/";
  std::string m_outRoot = "Analyse511.root";
  // ---- Histograms ---- //
  // Crystal patterns :
  MTTHist<TH2I> m_histo_crystals_label;
  MTTHist<TH2I> m_histo_511_label_both;
  MTTHist<TH2F> m_histo_511_both;
  MTTHist<TH2I> m_histo_511_label_both_delayed;
  MTTHist<TH2F> m_histo_511_both_delayed;
  MTTHist<TH2I> m_histo_511_label_partners;
  MTTHist<TH2F> m_histo_511_partners;
  MTTHist<TH2I> m_histo_511_label_one_and_other_less;
  MTTHist<TH2F> m_histo_511_one_and_other_less;
  // MTTHist<TH2F> m_histo_opposite_crystal_nrj;
  // ---- Analyse ---- //
  std::vector<Clovers> m_Clovers;
};

bool Analyse511::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void Analyse511::run(Parameters & p, Analyse511 & analyse_511)
{
  std::string rootfile;
  // Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "lTn");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      // event_s.sortEvent(event);
      analyse_511.FillRaw(event);
      // analyse_511.FillSorted(event_s,event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void Analyse511::InitializeManip()
{
  print("Initialize Arrays");
  Clovers::Initialize();
  m_Clovers.resize(MTObject::getThreadsNb());
  print("Initialize histograms");
  m_histo_crystals_label.reset("crystals_patern","all crystals patterns", 144,0,144, 144,0,144);

  m_histo_511_label_both.reset("511_pattern_both","511keV crystal patern both crystals have 511", 144,0,144, 144,0,144);
  m_histo_511_both.reset("two 511","511keV in both crystals", 1024,0,1024, 1024,0,1024);

  m_histo_511_label_both_delayed.reset("511_pattern_both delayed","delayed 511keV crystal patern both crystals have 511", 144,0,144, 144,0,144);
  m_histo_511_both_delayed.reset("two 511 delayed","delayed 511keV in both crystals", 1024,0,1024, 1024,0,1024);

  m_histo_511_partners.reset("511_nrj_at_least_one","511keV partners nrj", 4096,0,4096, 4096,0,4096);
  m_histo_511_label_partners.reset("511_pattern_at_least_one","511keV crystal patern partners", 144,0,144, 144,0,144);

  m_histo_511_label_one_and_other_less.reset("511_pattern_diff","511keV crystal patern one 511 and the other less", 144,0,144, 144,0,144);
  m_histo_511_one_and_other_less.reset("511_nrj","511keV one 511 and the other less", 1024,0,1024, 1024,0,1024);
  // m_histo_opposite_crystal_nrj.reset("opposite_crystal","opposite_crystal", 1024,0,1024, 1024,0,1024);
}

void Analyse511::FillRaw(Event const & event)
{
  auto const & thread_i = MTObject::getThreadIndex();
  auto & clovers = m_Clovers[thread_i];
  clovers.Reset();
  int j = 0;
  for (size_t i = 0; i<event.size(); i++)
  {
    j++;
    clovers.Fill(event,i);
  }
  for (uchar loop_i = 0; loop_i<clovers.cristaux.size(); loop_i++)
  {
    int const cristal_i = clovers.cristaux[loop_i];
    auto const & nrj_i = clovers.cristaux_nrj[cristal_i];
    auto const & time_i = clovers.cristaux_time[cristal_i];

    for (uchar loop_j = loop_i+1; loop_j<clovers.cristaux.size(); loop_j++)
    {
      int const cristal_j = clovers.cristaux[loop_j];
      auto const & nrj_j = clovers.cristaux_nrj[cristal_j];
      auto const & time_j = clovers.cristaux_time[cristal_j];

      if (time_i>50 && time_j>50) continue;
      m_histo_crystals_label.Fill(cristal_i,cristal_j);
      m_histo_crystals_label.Fill(cristal_j,cristal_i);

      if (clovers.has511)
      {
        if (nrj_i>506 && nrj_i<516)
        {
          if (nrj_j>506 && nrj_j<515)
          {
            m_histo_511_label_both.Fill(cristal_i,cristal_j);
            m_histo_511_label_both.Fill(cristal_j,cristal_i);

            m_histo_511_both.Fill(nrj_i, nrj_j);
            m_histo_511_both.Fill(nrj_j, nrj_i);

              m_histo_511_label_both_delayed.Fill(cristal_i,cristal_j);
              m_histo_511_label_both_delayed.Fill(cristal_j,cristal_i);

              m_histo_511_both_delayed.Fill(nrj_i, nrj_j);
              m_histo_511_both_delayed.Fill(nrj_j, nrj_i);
          }
          else if (nrj_j<506)
          {
            m_histo_511_label_one_and_other_less.Fill(cristal_i,cristal_j);
            m_histo_511_label_one_and_other_less.Fill(cristal_j,cristal_i);

            m_histo_511_one_and_other_less.Fill(nrj_i, nrj_j);
            m_histo_511_one_and_other_less.Fill(nrj_j, nrj_i);
          }
        }
        else if (!(nrj_j>506 && nrj_j<516))
        {
          m_histo_511_label_partners.Fill(cristal_i,cristal_j);
          m_histo_511_label_partners.Fill(cristal_j,cristal_i);

          m_histo_511_partners.Fill(nrj_i, nrj_j);
          m_histo_511_partners.Fill(nrj_j, nrj_i);
        }
      }
    }
  }
  // clovers.Analyse();
  // for (auto const & clover_i : clovers.Clean_Ge)
  // {
  //   auto const & c = clovers[clover_i];
  // }
}

void Analyse511::FillSorted(Sorted_Event const & event_s, Event const & event)
{
   for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
   {

   }
}

void Analyse511::Analyse()
{

}

void Analyse511::Write()
{
  print(m_outDir);
  create_folder_if_none(m_outDir);
  std::unique_ptr<TFile> outfile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  outfile->cd();
  m_histo_crystals_label.Write();
  m_histo_511_label_both.Write();
  m_histo_511_label_both_delayed.Write();
  m_histo_511_both_delayed.Write();
  m_histo_511_both.Write();
  m_histo_511_label_one_and_other_less.Write();
  m_histo_511_one_and_other_less.Write();
  m_histo_511_partners.Write();
  m_histo_511_label_partners.Write();
  outfile->Write();
  outfile->Close();
  print("Writting analysis in", m_outDir+m_outRoot);
}

bool Analyse511::setParameters(std::vector<std::string> const & parameters)
{
  if (parameters.size()<1){print("No Parameters for "+param_string+" !!"); return false;}  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> m_outDir;
      else if (temp == "outRoot:")  is >> m_outRoot;
      else
      {
        print("Parameter", temp, "for Analyse511 unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //ANALYSE511_H
