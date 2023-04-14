#ifndef MATRICES_H
#define MATRICES_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../../lib/Analyse/HistoAnalyse.hpp"
#include "../Classes/Parameters.hpp"


class Matrices
{
public:

  Matrices(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void run(Parameters & p, Matrices & matrices);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();
private:

  // ---- Parameters ---- //
  std::string param_string = "Matrices";
  friend class MTObject;

  // ---- Variables ---- //
  std::string outDir  = "129/Matrices/";
  std::string outRoot = "Matrices.root";
  int m_bins_Ge = 2000; Float_t m_min_Ge = 0; Float_t m_max_Ge = 8000;
  int m_bins_BGO = 300; Float_t m_min_BGO = 0; Float_t m_max_BGO = 8000;
  int m_bins_LaBr3 = 600; Float_t m_min_LaBr3 = 0; Float_t m_max_LaBr3 = 8000;

  // ---- Histograms ---- //
  MTTHist<TH2F> m_BGO_VS_Clover;
  MTTHist<TH2F> m_LaBr3_VS_Clover;

  MTTHist<TH2F> m_R3A1_BGO_VS_all_Clover;
  MTTHist<TH2F> m_Clean_R3A1_BGO_VS_all_Clean_Clover;
  MTTHist<TH2F> m_Vetoed_R3A1_BGO_VS_all_Clean_Clover;
  MTTHist<TH2F> m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover;
  MTTHist<TH2F> m_Vetoed_R3A1_BGO_VS_its_Clover;
  MTTHist<TH2F> m_Vetoed_R3A1_BGO_VS_its_Clover_E_total;

  MTTHist<TH2F> m_Paris_B2R1_VS_all_Clean_Clover;
  MTTHist<TH2F> m_Paris_NaI_B2R1_VS_all_Clean_Clover;

  std::vector<MTTHist<TH2F>> m_each_BGO_VS_all_Clover;
  std::vector<MTTHist<TH2F>> m_each_LaBr3_VS_all_Clover;

  MTTHist<TH2F> m_Clean_Ge_bidim;
};

bool Matrices::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Analyse();
  this -> Write();
  return true;
}

void Matrices::run(Parameters & p, Matrices & matrices)
{
  std::string rootfile;
  Sorted_Event event_s;
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));
    Event event(tree.get(), "lTnN");

    size_t events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    for (size_t i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      matrices.FillSorted(event_s,event);
      matrices.FillRaw(event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void Matrices::InitializeManip()
{
  print("Initialize histograms");
  m_R3A1_BGO_VS_all_Clover.reset("R3A1_BGO1_VS_Clover", "R3A1 BGO1 VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  m_Clean_R3A1_BGO_VS_all_Clean_Clover.reset("Clean_R3A1_BGO1_VS_Clean_Clover", "Clean R3A1 BGO1 VS Clean Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  m_Vetoed_R3A1_BGO_VS_all_Clean_Clover.reset("Vetoed_R3A1_BGO1_VS_Clean_Clover", "Vetoed R3A1 BGO1 VS Clean Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover.reset("Vetoed_R3A1_BGO1_VS_other_Vetoed_Clover", "Vetoed R3A1 BGO1 VS other Vetoed Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  m_Vetoed_R3A1_BGO_VS_its_Clover.reset("Vetoed_R3A1_BGO1_VS_its_Clover", "Vetoed R3A1 BGO1 VS its Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  m_Vetoed_R3A1_BGO_VS_its_Clover_E_total.reset("Vetoed_R3A1_BGO1_VS_E_module", "Vetoed R3A1 BGO1 VS sum BGO && its Clover",
      m_bins_BGO,m_min_BGO,m_max_BGO, m_bins_BGO,m_min_BGO,m_max_BGO);
  // m_Clean_Ge_bidim.reset("Clean Ge bidim", "Clean Ge bidim",
  //     m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_Ge,m_min_Ge,m_max_Ge);
  // m_BGO_VS_Clover.reset("BGO VS Clover", "BGO VS Clover",
  //     m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  // m_LaBr3_VS_Clover.reset("Paris VS Clover", "Paris VS Clover",
  //     m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_Paris_NaI_B2R1_VS_all_Clean_Clover.reset("B2R1_NaI_VS_Clover", "B2R1 NaI VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  m_Paris_B2R1_VS_all_Clean_Clover.reset("B2R1_VS_Clover", "B2R1 VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_each_BGO_VS_all_Clover.resize(48);
  m_each_LaBr3_VS_all_Clover.resize(74);

  for (size_t label = 0; label<g_labelToName.size(); label++)
  {
    std::string name = g_labelToName[label]+" VS CLover";
    if (isBGO[label])
    {
      m_each_BGO_VS_all_Clover[labelToBGOcrystal[label]].reset(name.c_str(), name.c_str(),
          m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
    }
    else if (isParis[label])
    {
      m_each_LaBr3_VS_all_Clover[labelToPariscrystal[label]].reset(name.c_str(), name.c_str(),
          m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
    }
  }
}

void Matrices::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void Matrices::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  if (event_s.ModulesMult>3) return ;
  for (size_t loop_i = 0; loop_i<event.size(); loop_i++)
  {
    auto const & Time = event.Times[loop_i];
    if (Time<-10 && Time>20) continue;


    auto const & label = event.labels[loop_i];
    if (isBGO[label] || isParis[label])
    {
      for (size_t loop_Ge = 0; loop_Ge<event_s.clover_hits.size(); loop_Ge++)
      {
        auto const & cloverGe = event_s.clover_hits[loop_Ge];
        auto const & TimeGe = event_s.time_clover[cloverGe];
        if(TimeGe<-10 || TimeGe>20) continue;
        auto const & nrjGe = event_s.nrj_clover[cloverGe];
        bool const vetoed_Ge = static_cast<bool> (event_s.BGO[cloverGe]);

        // Fill BGO bidim :
        if (isBGO[label])
        {
          auto const & nrjBGO = event.nrjs[loop_i];
          auto const & cloverBGO = labelToClover_fast[label];
          bool const vetoed_BGO = static_cast<bool> (event_s.Ge[cloverBGO]);
          if (label == 23)
          {
            m_R3A1_BGO_VS_all_Clover.Fill(nrjGe,nrjBGO);

            if (cloverBGO==cloverGe)
            {
              m_Vetoed_R3A1_BGO_VS_its_Clover.Fill(nrjGe,nrjBGO);
              m_Vetoed_R3A1_BGO_VS_its_Clover_E_total.Fill(nrjGe+nrjBGO, nrjBGO);
            }

            else if (vetoed_Ge && vetoed_BGO) m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover.Fill(nrjGe,nrjBGO);

            if (vetoed_BGO && !vetoed_Ge) m_Vetoed_R3A1_BGO_VS_all_Clean_Clover.Fill(nrjGe,nrjBGO);

            if (!vetoed_BGO && !vetoed_Ge) m_Clean_R3A1_BGO_VS_all_Clean_Clover.Fill(nrjGe,nrjBGO);
          }

          // auto const & crystalBGO = labelToBGOcrystal[label];
          // m_each_BGO_VS_all_Clover[crystalBGO].Fill(nrjGe,nrjBGO);

        }

        // Fill Paris LaBr3 bidim :
        else
        {
          auto const & nrjParis = event.nrjs[loop_i];
          auto const & nrj2Paris = event.nrj2s[loop_i];
          auto const & crystalParis = labelToPariscrystal[label];
          if (crystalParis == 10)
          {
            if ((nrj2Paris-nrjParis)/nrjParis < 0.5) m_Paris_B2R1_VS_all_Clean_Clover.Fill(nrjGe, nrjParis);
            else m_Paris_NaI_B2R1_VS_all_Clean_Clover.Fill(nrjGe, nrj2Paris);
          }
        }
      }
    }
  }
}

void Matrices::Analyse()
{
  // for ( size_t i = 0; i<m_each_BGO_VS_all_Clover.size(); i++ ) { NormalizeX(m_each_BGO_VS_all_Clover[i]); }
}

void Matrices::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  m_R3A1_BGO_VS_all_Clover.Write();
  m_Clean_R3A1_BGO_VS_all_Clean_Clover.Write();
  m_Vetoed_R3A1_BGO_VS_all_Clean_Clover.Write();
  m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover.Write();
  m_Vetoed_R3A1_BGO_VS_its_Clover.Write();
  m_Vetoed_R3A1_BGO_VS_its_Clover_E_total.Write();
  m_Paris_B2R1_VS_all_Clean_Clover.Write();
  m_Paris_NaI_B2R1_VS_all_Clean_Clover.Write();
  for (auto & histo : m_each_BGO_VS_all_Clover) histo.Write();
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outDir+outRoot);
 }

bool Matrices::setParameters(std::vector<std::string> const & parameters)
{
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> outDir;
      else if (temp == "outRoot:")  is >> outRoot;
      else
      {
        print("Parameter", temp, "for Matrices unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //MATRICES_H
