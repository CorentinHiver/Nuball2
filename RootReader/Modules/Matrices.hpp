#ifndef MATRICES_H
#define MATRICES_H
#include <libRoot.hpp>
#include "../../lib/MTObjects/MultiHist.hpp"
#include "../../lib/Analyse/HistoAnalyse.hpp"
#include "../Classes/Parameters.hpp"
#include "../../lib/Analyse/EventAnalyse.hpp"

class Matrices
{
public:

  Matrices(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitialiseManip();
  static void run(Parameters & p, Matrices & matrices);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void FillAnalysed(EventAnalyse & event_a);
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
  int m_bins_LaBr3 = 600; Float_t m_min_LaBr3 = 0; Float_t m_max_LaBr3 = 6000;

  // ---- Histograms ---- //
  MultiHist<TH2F> m_BGO_VS_Clover;
  MultiHist<TH2F> m_LaBr3_VS_Clover;

  MultiHist<TH2F> m_R3A1_BGO_VS_all_Clover;
  MultiHist<TH2F> m_Clean_R3A1_BGO_VS_all_Clean_Clover;
  MultiHist<TH2F> m_Vetoed_R3A1_BGO_VS_all_Clean_Clover;
  MultiHist<TH2F> m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover;
  MultiHist<TH2F> m_Vetoed_R3A1_BGO_VS_its_Clover;
  MultiHist<TH2F> m_Vetoed_R3A1_BGO_VS_its_Clover_E_total;

  MultiHist<TH2F> m_Paris_NaI_BR2D1_VS_all_Clean_Clover;
  MultiHist<TH2F> m_Paris_BR2D1_VS_all_Clean_Clover;
  MultiHist<TH2F> m_LaBr3_BR2D1_VS_all_Clean_Clover;
  MultiHist<TH2F> m_NaI_BR2D1_VS_all_Clean_Clover;
  MultiHist<TH2F> m_Paris_BR2D1_VS_all_LaBr3_Paris;
  MultiHist<TH2F> m_Paris_BR2D1_VS_front_LaBr3_Paris;

  MultiHist<TH2F> m_BR2D1_ratio_VS_time;
  MultiHist<TH2F> m_BR2D1_ratiov1_VS_time;
  MultiHist<TH2F> m_BR2D1_ratiov2_VS_time;
  MultiHist<TH2F> m_BR2D1_ratiov3_VS_time;
  MultiHist<TH2F> m_BR2D1_ratiov4_VS_time;

  MultiHist<TH2F> m_BR2D1_LaBr3_E_VS_time;
  MultiHist<TH2F> m_BR2D1_NaI_E_VS_time;

  MultiHist<TH2F> m_BR2D1_paris_E_VS_ratio;
  MultiHist<TH2F> m_BR2D1_paris_E2_VS_ratio;
  MultiHist<TH2F> m_BR2D1_paris_E_VS_ratio_m35_m25;
  MultiHist<TH2F> m_BR2D1_paris_E_VS_ratio_m20_m5;
  MultiHist<TH2F> m_BR2D1_paris_E_VS_ratio_m4_3;
  MultiHist<TH2F> m_BR2D1_paris_E_VS_ratio_6_20;

  std::vector<MultiHist<TH2F>> m_each_BGO_VS_all_Clover;
  std::vector<MultiHist<TH2F>> m_each_LaBr3_VS_all_Clover;

  // std::vector<Clovers> m_Clovers;

  MultiHist<TH2F> m_Clean_Ge_bidim;
};

bool Matrices::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitialiseManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Analyse();
  this -> Write();
  return true;
}

void Matrices::run(Parameters & p, Matrices & matrices)
{
  std::string rootfile;
  // Sorted_Event event_s;
  EventAnalyse event_a;
  MTObject::shared_mutex.lock();
  print("Thread", MTObject::getThreadIndex(), " ready to go !");
  MTObject::shared_mutex.unlock();
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
      event_a.Set(event);
      // clovers.Set(event);
      // event_s.sortEvent(event);
      // matrices.FillSorted(event_s,event);
      matrices.FillAnalysed(event_a);
      // matrices.FillRaw(event);
    } // End event loop
    auto const & time = timer;
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void Matrices::InitialiseManip()
{
  // m_Clovers.resize(MTObject::getThreadsNb());
  print("Initialise histograms");
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

  m_Paris_NaI_BR2D1_VS_all_Clean_Clover.reset("BR2D1_NaI_VS_Clover", "BR2D1 NaI VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  m_Paris_BR2D1_VS_all_Clean_Clover.reset("BR2D1_VS_Clover", "BR2D1 VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  m_LaBr3_BR2D1_VS_all_Clean_Clover.reset("LaBr3_BR2D1_VS_Clover", "BR2D1 VS Clover",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  m_NaI_BR2D1_VS_all_Clean_Clover.reset("NaI_BR2D1_VS_Clover", "BR2D1 VS NaI",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  m_Paris_BR2D1_VS_all_LaBr3_Paris.reset("BR2D1_VS_Paris_LaBr3", "BR2D1 VS Paris LaBr3",
      m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_Paris_BR2D1_VS_front_LaBr3_Paris.reset("BR2D1_VS_front_Paris_LaBr3", "BR2D1 VS Front Paris LaBr3",
      m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_BR2D1_paris_E_VS_ratio.reset("BR2D1_E_vs_ratio", "BR2D1 E vs ratio",
      501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_BR2D1_paris_E2_VS_ratio.reset("BR2D1_E2_vs_ratio", "BR2D1 E2 vs ratio",
      501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  // m_BR2D1_paris_E_VS_ratio_m35_m25.reset("BR2D1_E_vs_ratio_m35_m25", "BR2D1 E vs ratio time{-35,-25}",
  //     501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  // m_BR2D1_paris_E_VS_ratio_m20_m5.reset("BR2D1_E_vs_ratio_m25_m5", "BR2D1 E vs ratio time{-25,-5}",
  //     501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  // m_BR2D1_paris_E_VS_ratio_m4_3.reset("BR2D1_E_vs_ratio_m4_3", "BR2D1 E vs ratio time{-4,3}",
  //     501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  // m_BR2D1_paris_E_VS_ratio_6_20.reset("BR2D1_E_vs_ratio_6_20", "BR2D1 E vs ratio time{6,20}",
  //     501,-1,1, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_BR2D1_LaBr3_E_VS_time.reset("BR2D1_LaBR3_E_vs_time", "BR2D1 LaBR3 E vs time",
      500, -50, 150, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  m_BR2D1_NaI_E_VS_time.reset("BR2D1_NaI_E_vs_time", "BR2D1 NaI E vs time",
      500, -50, 150, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);

  // m_BR2D1_ratio_VS_time.reset("BR2D1_ratio_vs_time", "BR2D1 ratio vs time;[ns];(long-short)/long",
  //     500, -100, 400, 501,-1,1);
  // m_BR2D1_ratiov1_VS_time.reset("BR2D1_ratiov2_vs_time", "BR2D1 ratio V2 vs time;[ns];(long-short)/short",
  //     500, -100, 400, 501,-5,5);
  // m_BR2D1_ratiov2_VS_time.reset("BR2D1_ratiov2_vs_time", "BR2D1 ratio V2 vs time;[ns];(long-short)/short",
  //     500, -100, 400, 501,-5,5);
  // m_BR2D1_ratiov3_VS_time.reset("BR2D1_ratiov3_vs_time", "BR2D1 ratio V2 vs time;[ns];(long-short)/short",
  //     500, -100, 400, 501,-5,5);
  // m_BR2D1_ratiov4_VS_time.reset("BR2D1_ratiov4_vs_time", "BR2D1 ratio V2 vs time;[ns];(long-short)/short",
  //     500, -100, 400, 501,-5,5);

  m_each_BGO_VS_all_Clover.resize(48);
  m_each_LaBr3_VS_all_Clover.resize(74);
  std::string name;

  // for (size_t label = 0; label<g_labelToName.size(); label++)
  // {
  //   name = g_labelToName[label]+" VS CLover";
  //   if (isBGO[label])
  //   {
  //     m_each_BGO_VS_all_Clover[labelToBGOcrystal[label]].reset(name.c_str(), name.c_str(),
  //         m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_BGO,m_min_BGO,m_max_BGO);
  //   }
  //   else if (isParis[label])
  //   {
  //     m_each_LaBr3_VS_all_Clover[labelToPariscrystal[label]].reset(name.c_str(), name.c_str(),
  //         m_bins_Ge,m_min_Ge,m_max_Ge, m_bins_LaBr3,m_min_LaBr3,m_max_LaBr3);
  //   }
  // }
}

void Matrices::FillAnalysed(EventAnalyse & event_a)
{
  auto & paris_back = event_a.getParis().back();
  auto & paris_front = event_a.getParis().front();
  auto & clovers = event_a.getClovers();

  for (auto const & index : paris_back.Hits) if (index == 8)
  {
    auto const & nrj_paris = paris_back[8].nrj;
    // auto const & nrj2_paris = paris_back[8].nrj2;
    auto const & ratio = paris_back[8].ratio;
    // auto const & time = paris_back[8].time;
    // auto const ratio_v1 = nrj_paris/nrj2_paris;
    // auto const ratio_v2 = nrj2_paris/nrj_paris;
    // auto const ratio_v3 = 1-nrj2_paris/nrj_paris;
    // auto const ratio_v4 = 1-nrj_paris/nrj2_paris;

    m_BR2D1_paris_E_VS_ratio.Fill(ratio, nrj_paris);
    m_BR2D1_paris_E2_VS_ratio.Fill(ratio, paris_back[8].nrj2);


    if (paris_back[8].labr3) m_BR2D1_LaBr3_E_VS_time.Fill(paris_back[8].time, paris_back[8].labr3.nrj);
    else if (paris_back[8].nai)   m_BR2D1_NaI_E_VS_time  .Fill(paris_back[8].time, paris_back[8].nai.nrj  );

    m_BR2D1_ratio_VS_time.Fill(paris_back[8].time, ratio);
    // m_BR2D1_ratiov1_VS_time.Fill(paris_back[8].time, ratio_v2);
    // m_BR2D1_ratiov2_VS_time.Fill(paris_back[8].time, ratio_v2);
    // m_BR2D1_ratiov3_VS_time.Fill(paris_back[8].time, ratio_v3);
    // m_BR2D1_ratiov4_VS_time.Fill(paris_back[8].time, ratio_v4);


    //      if (time>-35 && time<-25) m_BR2D1_paris_E_VS_ratio_m35_m25.Fill(ratio, nrj_paris);
    // else if (time>-20 && time< -5) m_BR2D1_paris_E_VS_ratio_m20_m5 .Fill(ratio, nrj_paris);
    // else if (time>-25 && time< -5) m_BR2D1_paris_E_VS_ratio_m20_m5 .Fill(ratio, nrj_paris);
    // else if (time> -4 && time<  3) m_BR2D1_paris_E_VS_ratio_m4_3   .Fill(ratio, nrj_paris);
    // else if (time>  6 && time< 20) m_BR2D1_paris_E_VS_ratio_6_20   .Fill(ratio, nrj_paris);

    for (auto const & index_clover : clovers.Clean_Ge)
    {
      auto const & nrj_clover = clovers[index_clover].nrj;
      m_Paris_BR2D1_VS_all_Clean_Clover.Fill(nrj_clover, nrj_paris);
      if (paris_back[8].labr3)
      {
        m_LaBr3_BR2D1_VS_all_Clean_Clover.Fill(nrj_clover, paris_back[8].labr3.nrj);
      }

      else if (paris_back[8].nai)
      {
        m_NaI_BR2D1_VS_all_Clean_Clover.Fill(nrj_clover, paris_back[8].nai.nrj);
      }
    }
    for (auto const & index : paris_front.Hits)
    {
      if (paris_back[index].labr3)
      m_Paris_BR2D1_VS_front_LaBr3_Paris.Fill(paris_back[index].labr3.nrj, nrj_paris);
    }
  }// End paris_i loop
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
  // if (event_s.ModulesMult>3) return ;
  // for (size_t loop_i = 0; loop_i<event.clover_hits.size(); loop_i++)
  // {
  //   auto const & Time = event.Times[loop_i];
  //   if (Time<-10 && Time>20) continue;
  //
  //   auto const & label_i = event.labels[loop_i];
  //
  //   for (size_t loop_Ge = 0; loop_Ge<event_s.clover_hits.size(); loop_Ge++)
  //   {
  //     auto const & cloverGe = event_s.clover_hits[loop_Ge];
  //     auto const & TimeGe = event_s.time_clover[cloverGe];
  //     if(TimeGe<-10 || TimeGe>20) continue;
  //     auto const & nrjGe = event_s.nrj_clover[cloverGe];
  //     bool const vetoed_Ge = static_cast<bool> (event_s.BGO[cloverGe]);
  //
  //     // Fill BGO bidim :
  //     if (isBGO[label_i])
  //     {
  //       if (label_i == 23)
  //       {
  //         auto const & nrjBGO = event.nrjs[loop_i];
  //         auto const & cloverBGO = labelToClover_fast[label_i];
  //         bool const vetoed_BGO = static_cast<bool> (event_s.Ge[cloverBGO]);
  //         m_R3A1_BGO_VS_all_Clover.Fill(nrjGe,nrjBGO);
  //
  //         if (cloverBGO==cloverGe)
  //         {
  //           m_Vetoed_R3A1_BGO_VS_its_Clover.Fill(nrjGe,nrjBGO);
  //           m_Vetoed_R3A1_BGO_VS_its_Clover_E_total.Fill(nrjGe+nrjBGO, nrjBGO);
  //         }
  //
  //         else if (vetoed_Ge && vetoed_BGO) m_Vetoed_R3A1_BGO_VS_all_other_Vetoed_Clover.Fill(nrjGe,nrjBGO);
  //
  //         if (vetoed_BGO && !vetoed_Ge) m_Vetoed_R3A1_BGO_VS_all_Clean_Clover.Fill(nrjGe,nrjBGO);
  //
  //         if (!vetoed_BGO && !vetoed_Ge) m_Clean_R3A1_BGO_VS_all_Clean_Clover.Fill(nrjGe,nrjBGO);
  //       }
  //
  //       // auto const & crystalBGO = labelToBGOcrystal[label];
  //       // m_each_BGO_VS_all_Clover[crystalBGO].Fill(nrjGe,nrjBGO);
  //
  //     }
  //
  //     // Fill Paris LaBr3 bidim :
  //     else
  //     {
  //       auto const & crystalParis = labelToPariscrystal[label];
  //       if (crystalParis == 10)
  //       {
  //         auto const & nrjParis = event.nrjs[loop_i];
  //         auto const & nrj2Paris = event.nrj2s[loop_i];
  //         if ((nrj2Paris-nrjParis)/nrjParis < 0.5) m_Paris_B2R1_VS_all_Clean_Clover.Fill(nrjGe, nrjParis);
  //         else m_Paris_NaI_B2R1_VS_all_Clean_Clover.Fill(nrjGe, nrj2Paris);
  //       }
  //     }
  //   }// End clover j loop
  // }// End clover i loop
  //
  // if (isParis[label_i])
  // {
  //   auto const & nrjParis_i = event.nrjs[loop_i];
  //   auto const & nrj2Paris_i = event.nrj2s[loop_i];
  //   auto const ratio_i = (nrj2Paris_i-nrjParis_i)/nrjParis_i;
  //   for (size_t loop_j = loop_i+1; loop_j<event.size(); loop_j++)
  //   {
  //     auto const & label_j = event.labels[loop_i];
  //     if (isParis[label_j])
  //     auto const & nrjParis_j = event.nrjs[loop_j];
  //     auto const & nrj2Paris_j = event.nrj2s[loop_j];
  //     auto const ratio_j = (nrj2Paris_j-nrjParis_j)/nrjParis_j;
  //     auto const & crystalParis = labelToPariscrystal[label_j];
  //     if (ratio_j > 0.5 || ratio_j<-0.5) continue;
  //     if (crystalParis == 10) m_Paris_B2R1_VS_all_LaBr3_Paris.Fill(nrjParis_j, nrjParis_i);// j first because i is BR2D1
  //   }
  // }
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
  m_Paris_NaI_BR2D1_VS_all_Clean_Clover.Write();
  m_Paris_BR2D1_VS_all_Clean_Clover.Write();
  m_LaBr3_BR2D1_VS_all_Clean_Clover.Write();
  m_NaI_BR2D1_VS_all_Clean_Clover.Write();
  m_Paris_BR2D1_VS_all_LaBr3_Paris.Write();
  m_Paris_BR2D1_VS_front_LaBr3_Paris.Write();
  m_BR2D1_LaBr3_E_VS_time.Write();
  m_BR2D1_NaI_E_VS_time.Write();
  m_BR2D1_ratio_VS_time.Write();
  m_BR2D1_ratiov1_VS_time.Write();
  m_BR2D1_ratiov2_VS_time.Write();
  m_BR2D1_ratiov3_VS_time.Write();
  m_BR2D1_ratiov4_VS_time.Write();
  m_BR2D1_paris_E_VS_ratio.Write();
  m_BR2D1_paris_E2_VS_ratio.Write();
  m_BR2D1_paris_E_VS_ratio_m35_m25.Write();
  m_BR2D1_paris_E_VS_ratio_m20_m5.Write();
  m_BR2D1_paris_E_VS_ratio_m4_3.Write();
  m_BR2D1_paris_E_VS_ratio_6_20.Write();
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
        print("Parameter", temp, "for", param_string, "unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //MATRICES_H
