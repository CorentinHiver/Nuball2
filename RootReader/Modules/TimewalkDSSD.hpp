#ifndef TimewalkDSSD_H
#define TimewalkDSSD_H

#include <libRoot.hpp>
#include "../../lib/MTObjects/MultiHist.hpp"
#include "../Classes/Parameters.hpp"

class TimewalkDSSD
{

public:

  TimewalkDSSD(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitialiseManip();
  static void run(Parameters & p, TimewalkDSSD & TimewalkDSSD);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Analyse();
  void Write();

  // Parameters readers :
  std::pair<Float_t, Float_t> const & getGeGate() {return m_E_gate;}
  std::pair<Float_t, Float_t> const & getGeBackgroundGate() {return m_Bkcgd_gate;}

private:

  // ---- Parameters ---- //
  std::string param_string = "TimewalkDSSD";

  // ---- Variables ---- //
  std::string m_outDir  = "129/TimewalkDSSD/";
  std::string m_outRoot = "TimewalkDSSD.root";
  int m_njr_bins = 100; Float_t m_nrj_min = 0; Float_t m_nrj_max = 20000;
  int m_time_bins = 500; Float_t m_time_min = -100; Float_t m_time_max = 400;
  bool m_writedata = false;
  bool m_correct = false;
  bool m_gated = false;
  std::pair<Float_t, Float_t> m_E_gate;
  std::pair<Float_t, Float_t> m_Bkcgd_gate;


  // ---- Histograms ---- //
  Vector_MTTHist<TH2F> timewalk_DSSD_sectors_nRnS;
  MultiHist<TH2F> timewalk_DSSD_all_sectors;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_nRnS_raw;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_nRnS_raw_delayed;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_nRnS_background;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_nRnS;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_1R1S;
  MultiHist<TH2F> timewalk_DSSD_all_sectors_2R2S;
  Vector_MTTHist<TH2F> timewalk_DSSD_rings_nRnS;

  MultiHist<TH2F> timewalk_DSSD_all_sectors_1S2R;

  MultiHist<TH2F> DSSD_VS_Clover;
  MultiHist<TH1F> DSSD_spectra;

  Timewalks m_Timewalks_DSSD;
};

bool TimewalkDSSD::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitialiseManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  if (m_writedata) this -> Analyse();
  this -> Write();
  return true;
}

void TimewalkDSSD::run(Parameters & p, TimewalkDSSD & TimewalkDSSD)
{
  std::string rootfile;
  Sorted_Event event_s;
  event_s.addGeGateTrig(TimewalkDSSD.getGeGate());
  event_s.addGeGateTrig(TimewalkDSSD.getGeBackgroundGate());
  while(p.getNextFile(rootfile))
  {
    Timer timer;

    std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
    if (file->IsZombie()) {print(rootfile, "is a Zombie !");continue;}
    std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));

    Event event(tree.get(), "lTn");

    auto const & events = tree->GetEntries();
    p.totalCounter+=events;

    auto const & filesize = size_file(rootfile, "Mo");
    p.totalFilesSize+=filesize;

    for (int i = 0; i<events; i++)
    {
      tree->GetEntry(i);
      event_s.sortEvent(event);
      TimewalkDSSD.FillSorted(event_s,event);
      TimewalkDSSD.FillRaw(event);
    } // End event loop
    auto const & time = timer;
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/s");
  } // End files loop
}

void TimewalkDSSD::InitialiseManip()
{
  print("Initialise histograms");
  if (m_correct)
  {
    m_Timewalks_DSSD.resize(56);
    m_Timewalks_DSSD.loadFile("129/Analyse/DSSD/gate642_prompt/timewalk_data/fit.fit");
  }
  DSSD_VS_Clover.reset("DSSD_VS_Clover", "DSSD VS Clover", 2000,0,1000, 500,0,15000);
  DSSD_spectra.reset("DSSD", "DSSD", 500,0,15000);
  std::string name;
  timewalk_DSSD_all_sectors.reset("All sectors", "All sectors", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_nRnS.reset("All sectors nRnS", "All sectors nRnS", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_nRnS_background.reset("Background All sectors nRnS", "Background All sectors nRnS", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_nRnS_raw.reset("Raw All sectors nRnS", "All sectors nRnS before bckgnd subs", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_nRnS_raw_delayed.reset("Delayed Raw All sectors nRnS", "Delayed All sectors nRnS before bckgnd subs", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_1R1S.reset("All sectors 1R1S", "All sectors 1R1S", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_all_sectors_2R2S.reset("All sectors 2R2S", "All sectors 2R2S", m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  timewalk_DSSD_sectors_nRnS.resize(36);
  timewalk_DSSD_rings_nRnS.resize(16);
  for (int i = 0; i<36; i++)
  {
    name = std::to_string(i+800)+"_"+g_labelToName[i+800]+"_nRnS";
    timewalk_DSSD_sectors_nRnS[i].reset(name.c_str(), name.c_str(), m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  }
  for (int i = 0; i<16; i++)
  {
    name = std::to_string(i+840)+"_"+g_labelToName[i+840]+"_nRnS";
    timewalk_DSSD_rings_nRnS[i].reset(name.c_str(), name.c_str(), m_time_bins,m_time_min,m_time_max, m_njr_bins,m_nrj_min,m_nrj_max);
  }
}

void TimewalkDSSD::FillRaw(Event const & event)
{
  // for (size_t i = 0; i<event.size(); i++)
  // {
  //
  // }
}

void TimewalkDSSD::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  Float_t Time = 0.;
  auto const & thread_nb =  MTObject::getThreadIndex();

  auto const & gateGe = event_s.gateGe();
  if (m_gated && gateGe==0) return;
  auto const & triggedClover = event_s.getGatedClover();
  Float_t weight = (gateGe==1) ? 1 : -1; // Here

  bool nRnS = false, R1S1 = false, R2S2 = false;
  if (event_s.DSSDRingMult ==  event_s.DSSDSectorMult)
  {
    nRnS = true;
    if (event_s.DSSDRingMult == 1 && event_s.DSSDSectorMult == 1) R1S1 = true;
    else if (event_s.DSSDRingMult == 2 && event_s.DSSDSectorMult == 2) R2S2 = true;
  }

  bool prompt = false, delayed = false;
  if (event_s.time_clover[triggedClover]>-10 && event_s.time_clover[triggedClover]<20) prompt = true;
  else if (event_s.time_clover[triggedClover]<160) delayed = true;

  for (size_t loop_i = 0; loop_i<event_s.DSSD_Rings.size(); loop_i++)
  {
    auto const & dssd = event_s.DSSD_Rings[loop_i];

    auto const & label = event.labels[dssd];
    auto const & nrj   = event.nrjs  [dssd];
                 Time  = event.time2s [dssd];

    DSSD_spectra.Fill(nrj, weight);


    if (m_correct) Time -= m_Timewalks_DSSD.get(label-800, nrj);

    if (prompt)
    {
      if(nRnS) timewalk_DSSD_rings_nRnS[label-840][thread_nb] -> Fill(Time, nrj, weight);
    }
    else if (delayed);

    for (auto const clover : event_s.clover_hits) if (!event_s.BGO[clover])
      DSSD_VS_Clover.Fill(event_s.nrj_clover[clover], nrj, weight);
  }

  for (size_t loop_i = 0; loop_i<event_s.DSSD_Sectors.size(); loop_i++)
  {
    auto const & dssd = event_s.DSSD_Sectors[loop_i];

    auto const & label = event.labels[dssd];
    auto const & nrj   = event.nrjs  [dssd];
                 Time  = event.time2s [dssd];
    if (m_correct) Time -= m_Timewalks_DSSD.get(label-800, nrj);

    DSSD_spectra.Fill(nrj, weight);

    if (prompt)
    {
     timewalk_DSSD_all_sectors[thread_nb] -> Fill(Time, nrj, weight);
     if(nRnS)
     {
       // timewalk_DSSD_sectors_nRnS[label-800].Fill(Time, nrj, weight);
       timewalk_DSSD_sectors_nRnS[label-800][thread_nb] -> Fill(Time, nrj, weight);
       timewalk_DSSD_all_sectors_nRnS.Fill(Time, nrj, weight);
       if (gateGe == 1) timewalk_DSSD_all_sectors_nRnS_raw.Fill(Time, nrj);
       else if (gateGe == 2) timewalk_DSSD_all_sectors_nRnS_background.Fill(Time, nrj);
       //      if (R1S1) timewalk_DSSD_all_sectors_1R1S.Fill(Time, nrj, weight);
       // else if (R2S2) timewalk_DSSD_all_sectors_2R2S.Fill(Time, nrj, weight);
       if (R1S1) timewalk_DSSD_all_sectors_1R1S[thread_nb] -> Fill(Time, nrj, weight);
       else if (R2S2) timewalk_DSSD_all_sectors_2R2S[thread_nb] -> Fill(Time, nrj, weight);
     }
    }
    for (auto const clover : event_s.clover_hits) if (!event_s.BGO[clover])
      DSSD_VS_Clover.Fill(event_s.nrj_clover[clover], nrj, weight);
  }
}

void TimewalkDSSD::Analyse()
{
  std::string pathWrite = m_outDir+"timewalk_data/";
  create_folder_if_none(pathWrite);
  for (size_t i = 0; i<timewalk_DSSD_sectors_nRnS.size(); i++  )
  {
    std::string filename = pathWrite+"timewalk_"+std::to_string(i+800)+".tw";
    std::ofstream timewalk(filename,std::ios::out);
    auto & histo = timewalk_DSSD_sectors_nRnS[i];
    histo.Merge();
    if (histo->Integral()<1) continue;
    for (int nrj = 0; nrj<m_njr_bins-1; nrj++)
    {
      int nrjcal_min = (Float_t)nrj*m_nrj_max/m_njr_bins;
      int nrjcal_max = (Float_t)(nrj+1)*m_nrj_max/m_njr_bins;
      std::string name_proj = g_labelToName[i+800]+"_p_"+std::to_string(nrjcal_min)+";"+std::to_string(nrjcal_max);
      auto proj = histo -> ProjectionX(name_proj.c_str(),nrj,nrj+1);
      if (proj->Integral()>1)
      {
        auto const & max = proj->GetXaxis()->GetBinCenter(proj->GetMaximumBin());
        timewalk << (nrjcal_min+nrjcal_max)/2 << " " << max << "\n";
      }
    }
    print("data written to ", filename);
    timewalk.close();
  }
  for (size_t i = 0; i<timewalk_DSSD_rings_nRnS.size(); i++  )
  {
    std::string filename = pathWrite+"timewalk_"+std::to_string(i+840)+".tw";
    std::ofstream timewalk(filename,std::ios::out);
    auto & histo = timewalk_DSSD_rings_nRnS[i];
    histo.Merge();
    if (histo->Integral()<1) continue;
    for (int nrj = 0; nrj<m_njr_bins-1; nrj++)
    {
      int nrjcal_min = (Float_t)nrj*m_nrj_max/m_njr_bins;
      int nrjcal_max = (Float_t)(nrj+1)*m_nrj_max/m_njr_bins;
      std::string name_proj = g_labelToName[i+840]+"_p_"+std::to_string(nrjcal_min)+";"+std::to_string(nrjcal_max);
      auto proj = histo -> ProjectionX(name_proj.c_str(),nrj,nrj+1);
      if (proj->Integral()>1)
      {
        auto const & max = proj->GetXaxis()->GetBinCenter(proj->GetMaximumBin());
        timewalk << (nrjcal_min+nrjcal_max)/2 << " " << max << "\n";
      }
    }
    print("data written to ", filename);
    timewalk.close();
  }
}

void TimewalkDSSD::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((m_outDir+m_outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  // for (auto & histo : timewalk_DSSD_sectors) histo.Write();
  // for (auto & histo : timewalk_DSSD_rings  ) histo.Write();
  DSSD_VS_Clover.Write();
  DSSD_spectra.Write();
  timewalk_DSSD_all_sectors.Write();
  timewalk_DSSD_all_sectors_nRnS_raw.Write();
  timewalk_DSSD_all_sectors_nRnS_background.Write();
  timewalk_DSSD_all_sectors_nRnS.Write();
  timewalk_DSSD_all_sectors_1R1S.Write();
  timewalk_DSSD_all_sectors_2R2S.Write();
  for (auto & histo : timewalk_DSSD_sectors_nRnS) histo.Write();
  for (auto & histo : timewalk_DSSD_rings_nRnS  ) histo.Write();
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", m_outDir+m_outRoot);
 }

bool TimewalkDSSD::setParameters(std::vector<std::string> const & parameters)
{
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")
      {
        is >> m_outDir;
        if (m_outDir.back() != '/') m_outDir.push_back('/');
      }
      else if (temp == "outRoot:")  is >> m_outRoot;
      else if (temp == "writedata")  m_writedata = true;
      else if (temp == "correct")
      {
        m_outRoot = removeExtension(m_outRoot)+"_corrected.root";
        m_correct = true;
      }
      else if (temp == "gate:")
      {
        m_gated = true;
        Float_t Emin = 0., EMax = 0., EminBckgd = 0., EmaxBckgd = 0.;
        is >> Emin >> EMax >> EminBckgd >> EmaxBckgd;
        m_E_gate = std::make_pair(Emin, EMax);
        m_Bkcgd_gate = std::make_pair(EminBckgd, EmaxBckgd);
        m_outDir.pop_back();
        m_outDir = m_outDir+"_gate"+std::to_string((int)((Emin+EMax)/2.))+"/";
      }
      else
      {
        print("Parameter", temp, "for TimewalkDSSD unkown...");
        return false;
      }
    }
  }
  if (m_writedata && m_correct) {print("Can't correct AND write data !!"); return false;}
  return true;
}

#endif //TimewalkDSSD_H
