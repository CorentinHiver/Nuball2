#include "../lib/libRoot.hpp"

#include "../lib/Analyse/WarsawDSSD.hpp"

#include "../lib/Classes/Calibration.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/CoProgressBar.hpp"
#include "../lib/Classes/Timer.hpp"

void AlphaOvernightAnalyse()
{
  Timer timer;
  Calibration calib("triple_alpha.calib");
  Event event;
  Nuball2Tree tree("alpha_overnight.root", event);
  // tree.setMaxHits(1.e6);

  WarsawDSSD dssd;

  if (!tree) {error("no file"); return;}
  auto adc_VS_label = new TH2F("adc_VS_label", "adc_VS_label;Label;ADC", 1000,0,1000, 10000,0,1000000);
  auto nrj_VS_label = new TH2F("nrj_VS_label", "nrj_VS_label;Label;Energy [keV]", 1000,0,1000, 10000,0,10000);
  auto nrj_VS_label_ok = new TH2F("nrj_VS_label_ok", "nrj_VS_label_ok;Label;Energy [keV]", 1000,0,1000, 10000,0,10000);
  auto nrj_VS_SM = new TH2F("nrj_VS_SM", "nrj_VS_SM;Label;Energy [keV]", 10,0,10, 10000,0,10000);
  auto nrj_VS_RM = new TH2F("nrj_VS_RM", "nrj_VS_RM;Label;Energy [keV]", 10,0,10, 10000,0,10000);

  auto R2_VS_R1 = new TH2F("R2_VS_R1", "R2_VS_R1;Energy R1 [keV];Energy R2 [keV]", 1000,0,10000, 1000,0,10000);
  auto R_VS_S_ok = new TH2F("R_VS_S_ok", "R_VS_S_ok;Energy Sector [keV];Energy Ring [keV]", 1000,0,10000, 1000,0,10000);
  auto R_VS_S_test = new TH2F("R_VS_S_test", "R_VS_S_test;Energy Sector [keV];Energy Ring [keV]", 1000,0,10000, 1000,0,10000);

  auto cell_nrj = new TH1F("cell_nrj", "cell_nrj;Cell Energy [keV]", 10000,0,10000);
  auto cell_nrj_VS_mult = new TH2F("cell_nrj_VS_mult", "cell_nrj_VS_mult;Cell multiplicity;Cell Energy [keV]", 5,0,5, 10000,0,10000);
  auto cell_Ering_VS_Esector_clean = new TH2F("cell_Ering_VS_Esector_clean", "cell_Ering_VS_Esector_clean;Energy sector [keV];Cell Energy [keV]", 1000,0,10000, 1000,0,10000);
  auto cell_nrj_VS_mult_random = new TH2F("cell_nrj_VS_mult_random", "cell_nrj_VS_mult_random;Cell multiplicity;Cell Energy [keV]", 5,0,5, 10000,0,10000);
  auto cell_sector_nrj = new TH1F("cell_sector_nrj", "cell_sector_nrj;Cell Energy [keV]", 10000,0,10000);
  auto cell_ring_nrj = new TH1F("cell_ring_nrj", "cell_sector_nrj;Cell Energy [keV]", 10000,0,10000);

  CoProgressBar bar(tree.cursor_ptr(), tree.getMaxHits());
  while(tree.readNext()) 
  {
    // CALCULER COUNT RATE
    bar.showEvery(1.e5);
    dssd.clear();
    for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    {
      adc_VS_label->Fill(event.labels[hit_i], event.adcs[hit_i]);
      calib.calibrate(event, hit_i);
      nrj_VS_label->Fill(event.labels[hit_i], event.nrjs[hit_i]);
      dssd.fill(event, hit_i);
    }
    dssd.analyze();

    for (int hit_i = 0; hit_i<event.mult; ++hit_i)
    {
      nrj_VS_RM->Fill(dssd.sectors().mult, event.nrjs[hit_i]);
      nrj_VS_SM->Fill(dssd.rings().mult, event.nrjs[hit_i]);

      if (dssd.ok)
      {
        nrj_VS_label_ok->Fill(event.labels[hit_i], event.nrjs[hit_i]);
      }
    }

    if (dssd.rings().mult == 2) R2_VS_R1->Fill(dssd.rings()[0].nrj, dssd.rings()[1].nrj);
    for (auto const & sector : dssd.sectors()) for (auto const & ring : dssd.rings()) R_VS_S_ok->Fill(sector->nrj, ring->nrj);
    for (size_t sector_i = 0; sector_i<dssd.sectors().size(); ++sector_i) for (size_t ring_i = 0; ring_i<dssd.rings().size(); ++ring_i)
      R_VS_S_test->Fill(dssd.sectors()[sector_i].nrj, dssd.rings()[ring_i].nrj);

    dssd.buildCells(false, false);

    for (auto const & cell : dssd.cells()) 
    {
      cell_nrj->Fill(cell.nrj);
      cell_nrj_VS_mult->Fill(dssd.cells().size(), cell.nrj);
      cell_sector_nrj->Fill(cell.Esector);
      cell_ring_nrj->Fill(cell.Ering);
      if (cell.random) cell_nrj_VS_mult_random->Fill(dssd.cells().size(), cell.nrj);
      else cell_Ering_VS_Esector_clean->Fill(cell.Esector, cell.Ering);
    }
  }

  auto outfile = TFile::Open("alpha_analyse.root", "recreate");
  outfile->cd();
  adc_VS_label->Write();
  nrj_VS_label->Write();
  nrj_VS_label_ok->Write();
  nrj_VS_SM->Write();
  nrj_VS_RM->Write();
  R2_VS_R1->Write();
  R_VS_S_ok->Write();
  R_VS_S_test->Write();
  cell_nrj->Write();
  cell_nrj_VS_mult->Write();
  cell_Ering_VS_Esector_clean->Write();
  cell_nrj_VS_mult_random->Write();
  cell_sector_nrj->Write();
  cell_ring_nrj->Write();
  outfile->Close();
  print("alpha_analyse.root written");
  print(timer());
}