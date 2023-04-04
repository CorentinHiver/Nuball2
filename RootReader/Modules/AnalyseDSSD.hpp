#ifndef ANALYSEDSSD_H
#define ANALYSEDSSD_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"


class AnalyseDSSD
{
public:

  AnalyseDSSD(){};
  bool launch(Parameters & p);
  bool setParameters(std::vector<std::string> const & param);
  void InitializeManip();
  static void run(Parameters & p, AnalyseDSSD & analysedssd);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();
private:

  // Parameters
  std::string param_string = "DSSD";
  friend class MTObject;
  std::string outDir  = "129/AnalyseDSSD/";
  std::string outRoot = "AnalyseDSSD.root";

  // ---- Parameters ---- //

  int m_bins_spectra = 0;
  Float_t m_min_spectra = 0.;
  Float_t m_max_spectra = 0.;

  // ---- Histograms ---- //
  MTTHist<TH1F> m_Ge_spectra_gate_on_particle_20MeV;
  MTTHist<TH1F> m_Ge_spectra_gate_on_particle_sup_11MeV_R15; // Rings intérieurs
  MTTHist<TH1F> m_Ring_14_spectra;

  MTTHist<TH2F> m_mult_R_VS_S;

  MTTHist<TH2F> m_each_Sector_spectra;
  MTTHist<TH2F> m_each_Ring_spectra;

  // nRnS : same number of rings and sectors
  MTTHist<TH2F> m_each_Sector_spectra_nRnS;
  MTTHist<TH2F> m_each_Ring_spectra_nRnS;

  // 1 ring only
  MTTHist<TH2F> m_polar_1R;
  MTTHist<TH1F> m_check_S_1R;
  MTTHist<TH1F> m_linear_1R;

  // 1 sector only
  MTTHist<TH2F> m_polar_1S;

  //1 ring 1 sector
  MTTHist<TH2F> m_polar_1R1S;
  MTTHist<TH2F> m_R_VS_S;
  MTTHist<TH2F> m_R_VS_S_time;
  //2 rings 2 sectors
  MTTHist<TH2F> m_R1_VS_R2;
  MTTHist<TH2F> m_S1_VS_S2;
  MTTHist<TH2F> m_R1_VS_S1;
  MTTHist<TH2F> m_R1_VS_S2;
  MTTHist<TH2F> m_R2_VS_S1;
  MTTHist<TH2F> m_R2_VS_S2;

  MTTHist<TH2F> m_R1_VS_R2_time;
  MTTHist<TH2F> m_S1_VS_S2_time;
  MTTHist<TH2F> m_R1_VS_S1_time;
  MTTHist<TH2F> m_R1_VS_S2_time;
  MTTHist<TH2F> m_R2_VS_S1_time;
  MTTHist<TH2F> m_R2_VS_S2_time;

  Vector_MTTHist<TH2F> m_each_DSSD_VS_Time;
};

bool AnalyseDSSD::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

void AnalyseDSSD::run(Parameters & p, AnalyseDSSD & analysedssd)
{
  std::string rootfile;
  Sorted_Event event_s;
  event_s.addGeGateTrig(639,645);
  event_s.addGeGateTrig(647,653);
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
      event_s.sortEvent(event);
      analysedssd.FillRaw(event);
      analysedssd.FillSorted(event_s,event);
    } // End event loop
    auto const & time = timer();
    print(removePath(rootfile), time, timer.unit(), ":", filesize/timer.TimeSec(), "Mo/sec");
  } // End files loop
}

void AnalyseDSSD::InitializeManip()
{
  print("Initialize histograms");
  m_Ring_14_spectra.reset("Ring 15","Ring 15",10000,0,100000);
  m_mult_R_VS_S.reset("Mult Rings VS Sectors","Multiplicity Rings VS Sectors", 10,0,10, 15,0,15);
  m_each_Sector_spectra.reset("Each sector spectra","Each sector spectra",
      36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra.reset("Each ring spectra","Each ring spectra",
      16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Sector_spectra_nRnS.reset("Each sector spectra nRnS","Each sector spectra same number of sectors and rings",
      36,0,36, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_each_Ring_spectra_nRnS.reset("Each ring spectra nRnS","Each ring spectra same number of sectors and rings",
      16,0,16, m_bins_spectra,m_min_spectra,m_max_spectra);

  m_check_S_1R.reset("1R check sectors","1R check sectors", 33,0,33);
  m_linear_1R.reset("1R linear","1R linear", 15,0,15);
  m_polar_1R.reset("1R polar","1R polar", 600,-15,15, 600,-15,15);
  m_polar_1S.reset("1S polar","1S polar", 800,-20,20, 800,-20,20);
  m_polar_1R1S.reset("1R1S polar","1R1S polar", 800,-20,20, 800,-20,20);
  m_R_VS_S.reset("1R1S Ring VS Sector","1R1S Ring VS Sector", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_R_VS_S_time.reset("1R1S Ring VS Sector Time","1R1S Ring VS Sector Time", 500,-100,400, 500,-100,400);

  m_R1_VS_R2.reset("2R2S Ring 1 VS Rings 2","2R2S Ring 1 VS Rings 2", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_S1_VS_S2.reset("2R2S Sector 1 VS Sector 2","2R2S Sector 1 VS Sector 2", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_R1_VS_S1.reset("2R2S Ring 1 VS Sector 1","2R2S Ring 1 VS Sector 1", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_R1_VS_S2.reset("2R2S Ring 1 VS Sector 2","2R2S Ring 1 VS Sector 2", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_R2_VS_S1.reset("2R2S Ring 2 VS Sector 1","2R2S Ring 2 VS Sector 1", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);
  m_R2_VS_S2.reset("2R2S Ring 2 VS Sector 2","2R2S Ring 2 VS Sector 2", m_bins_spectra,m_min_spectra,m_max_spectra, m_bins_spectra,m_min_spectra,m_max_spectra);

  m_R1_VS_R2_time.reset("2R2S Timing Ring 1 VS Rings 2","2R2S Timing Ring 1 VS Rings 2", 500,-100,400, 500,-100,400);
  m_S1_VS_S2_time.reset("2R2S Timing Sector 1 VS Sector 2","2R2S Timing Sector 1 VS Sector 2", 500,-100,400, 500,-100,400);
  m_R1_VS_S1_time.reset("2R2S Timing Ring 1 VS Sector 1","2R2S Timing Ring 1 VS Sector 1", 500,-100,400, 500,-100,400);
  m_R1_VS_S2_time.reset("2R2S Timing Ring 1 VS Sector 2","2R2S Timing Ring 1 VS Sector 2", 500,-100,400, 500,-100,400);
  m_R2_VS_S1_time.reset("2R2S Timing Ring 2 VS Sector 1","2R2S Timing Ring 2 VS Sector 1", 500,-100,400, 500,-100,400);
  m_R2_VS_S2_time.reset("2R2S Timing Ring 2 VS Sector 2","2R2S Timing Ring 2 VS Sector 2", 500,-100,400, 500,-100,400);

  m_Ge_spectra_gate_on_particle_20MeV.reset("Ge spectra gated on 20MeV peak","Ge spectra gated on 20MeV peak",20000,0,10000);
  m_Ge_spectra_gate_on_particle_sup_11MeV_R15.reset("Ge gated on Ep > 11 MeV ring interieur","Ge spectra gated on E particule > 11 MeV ring interieur",20000,0,10000);
}

void AnalyseDSSD::FillRaw(Event const & event){}

void AnalyseDSSD::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  auto const & gateGe = event_s.gateGe();
  if (gateGe==0) return;
  Float_t weight = (gateGe==1) ? 1 : -1;
  // print(weight);
  bool nRnS = false;
  m_mult_R_VS_S.Fill(event_s.DSSDSectorMult, event_s.DSSDRingMult);
  if (event_s.DSSDRingMult == 1 && event_s.DSSDSectorMult == 0 )
  {
    auto const randSector = static_cast<int>(gRandom->Uniform(0,1)*33);
    auto const label_ring = event.labels[event_s.DSSD_Rings[0]]-800;
    // print("Sector : ", randSector);
    auto pos = calculate_DSSD_pos(randSector, label_ring);
    m_check_S_1R.Fill(randSector);
    m_linear_1R.Fill(label_ring);
    print(label_ring);
    m_polar_1R.Fill(pos.first, pos.second);
  }
  if (event_s.DSSDRingMult == 0 && event_s.DSSDSectorMult == 1 )
  {
    auto const randRing = static_cast<int>(gRandom->Uniform(0,1)*15.49);
    auto const label_sector = event.labels[event_s.DSSD_Sectors[0]]-800;
    auto pos = calculate_DSSD_pos(label_sector, randRing);
    m_polar_1S.Fill(pos.first, pos.second);
  }
  if (event_s.DSSDSectorMult == event_s.DSSDRingMult) nRnS = true;

  if (event_s.DSSDSectorMult == 1 && event_s.DSSDRingMult == 1)
  {
    // R2S2 = true;
    auto const & ring = event_s.DSSD_Rings[0];
    auto const & sector = event_s.DSSD_Sectors[0];

    auto const label_ring = event.labels[ring]-800;
    auto const & nrj_ring = event.nrjs[ring];
    auto const & Time_ring = event.Times[ring];

    auto const label_sector = event.labels[sector]-800;
    auto const & nrj_sector = event.nrjs[sector];
    auto const & Time_sector = event.Times[sector];

    m_R_VS_S.Fill(nrj_ring, nrj_sector, weight);
    m_R_VS_S_time.Fill(Time_ring, Time_sector, weight);

    auto pos = calculate_DSSD_pos(label_ring, label_sector);
    m_polar_1R1S.Fill(pos.first, pos.second);
  }
  // bool R2S2 = false;
  else if (event_s.DSSDSectorMult == 2 && event_s.DSSDRingMult == 2)
  {
    // R2S2 = true;
    auto const & nrj_ring_1 = event.nrjs[event_s.DSSD_Rings[0]];
    auto const & nrj_ring_2 = event.nrjs[event_s.DSSD_Rings[1]];
    auto const & nrj_sector_1 = event.nrjs[event_s.DSSD_Sectors[0]];
    auto const & nrj_sector_2 = event.nrjs[event_s.DSSD_Sectors[1]];

    m_R1_VS_R2.Fill(nrj_ring_2, nrj_ring_1, weight);
    m_S1_VS_S2.Fill(nrj_sector_2, nrj_sector_1, weight);
    m_R1_VS_S1.Fill(nrj_sector_1, nrj_ring_1, weight);
    m_R1_VS_S2.Fill(nrj_sector_1, nrj_ring_2, weight);
    m_R2_VS_S1.Fill(nrj_sector_2, nrj_ring_1, weight);
    m_R2_VS_S2.Fill(nrj_sector_2, nrj_ring_2, weight);

    auto const & time_ring_1 = event.Times[event_s.DSSD_Rings[0]];
    auto const & time_ring_2 = event.Times[event_s.DSSD_Rings[1]];
    auto const & time_sector_1 = event.Times[event_s.DSSD_Sectors[0]];
    auto const & time_sector_2 = event.Times[event_s.DSSD_Sectors[1]];

    m_R1_VS_R2_time.Fill(time_ring_2, time_ring_1, weight);
    m_S1_VS_S2_time.Fill(time_sector_2, time_sector_1, weight);
    m_R1_VS_S1_time.Fill(time_sector_1, time_ring_1, weight);
    m_R1_VS_S2_time.Fill(time_sector_2, time_ring_1, weight);
    m_R2_VS_S1_time.Fill(time_sector_1, time_ring_2, weight);
    m_R2_VS_S2_time.Fill(time_sector_2, time_ring_2, weight);
  }

  for (size_t loop_i = 0; loop_i<event_s.DSSD_hits.size(); loop_i++)
  {
    auto const & dssd_i = event_s.DSSD_hits[loop_i];
    auto const & ring_i = event_s.DSSD_is_Ring[loop_i];

    auto const & raw_label_i = event.labels[dssd_i];
    auto const & nrj_i = event.nrjs[dssd_i];


    bool peak20MeV = false, fission_fragment = false;
    if (nrj_i > 15000 && nrj_i < 23000) peak20MeV = true;
    if (ring_i)
    {
      auto const label_i = raw_label_i-840;
      m_each_Ring_spectra.Fill(label_i, nrj_i, weight);
      if (nRnS)
      {
        m_each_Ring_spectra_nRnS.Fill(label_i, nrj_i, weight);
      }
      if (label_i==14)
      {
        m_Ring_14_spectra.Fill(nrj_i, weight);
        if (nrj_i > 11000) fission_fragment = true;
      }
    }
    else
    {
      auto const label_i = raw_label_i-800;
      m_each_Sector_spectra.Fill(label_i, nrj_i, weight);
      if (nRnS)
      {
        m_each_Sector_spectra_nRnS.Fill(label_i, nrj_i, weight);
      }
    }


    for (auto const & clover : event_s.clover_hits)
    {
      if (event_s.BGO[clover]) continue;
      if (peak20MeV) m_Ge_spectra_gate_on_particle_20MeV.Fill(event_s.nrj_clover[clover]);
      if (fission_fragment) m_Ge_spectra_gate_on_particle_sup_11MeV_R15.Fill(event_s.nrj_clover[clover]);

    }
  }
}

void AnalyseDSSD::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");

  m_Ge_spectra_gate_on_particle_20MeV.Write();
  m_Ge_spectra_gate_on_particle_sup_11MeV_R15.Write();

  m_Ring_14_spectra.Write();
  m_mult_R_VS_S.Write();

  m_check_S_1R.Write();
  m_linear_1R.Write();
  m_polar_1R.Write();
  m_polar_1S.Write();
  m_polar_1R1S.Write();

  m_each_Sector_spectra.Write();
  m_each_Ring_spectra.Write();
  m_each_Sector_spectra_nRnS.Write();
  m_each_Ring_spectra_nRnS.Write();

  m_R_VS_S.Write();
  m_R_VS_S_time.Write();

  m_R1_VS_R2.Write();
  m_S1_VS_S2.Write();
  m_R1_VS_S1.Write();
  m_R1_VS_S2.Write();
  m_R2_VS_S1.Write();
  m_R2_VS_S2.Write();

  m_R1_VS_R2_time.Write();
  m_S1_VS_S2_time.Write();
  m_R1_VS_S1_time.Write();
  m_R1_VS_S2_time.Write();
  m_R2_VS_S1_time.Write();
  m_R2_VS_S2_time.Write();

  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outDir+outRoot);
 }

bool AnalyseDSSD::setParameters(std::vector<std::string> const & parameters)
{
  for (auto const & param : parameters)
  {
    std::istringstream is(param);
    std::string temp;
    while(is>>temp)
    {
      if (temp == "outDir:")  is >> outDir;
      else if (temp == "outRoot:")  is >> outRoot;
      else if (temp == "binning:") is >> m_bins_spectra >> m_min_spectra >> m_max_spectra;
      else
      {
        print("Parameter", temp, "for AnalyseDSSD unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //ANALYSEDSSD_H
