#include "../../lib/libRoot.hpp"

void merge_dC1_reader(std::string filename = "merge_dC1_V2.root")
{
  auto file = TFile::Open(filename.c_str());
  if (!file) throw_error(filename+ "absent !");
  auto d_VS_DC = file->Get<TH2F>("d_VS_DC");
  auto d_VS_DC_p = file->Get<TH2F>("d_VS_DC_p");
  auto d_VS_DC_pP = file->Get<TH2F>("d_VS_DC_pP");
  auto d_VS_DC_ExSIP = file->Get<TH2F>("d_VS_DC_ExSIP");

  auto gate2MeV_p = d_VS_DC_p->ProjectionY("gate2MeVp", 150, 250);
  auto gate2MeV_pP = d_VS_DC_pP->ProjectionY("gate2MeVpP", 150, 250);
  auto gate3MeV_2Mev_p = d_VS_DC_p->ProjectionY("gate3MeV-gate2MeVp", 250, 350);
  auto gate3MeV_2Mev_pP = d_VS_DC_pP->ProjectionY("gate3MeVP-gate2MeVp", 250, 350);
  auto gate3MeV_p = d_VS_DC_p->ProjectionY("gate3MeVp", 250, 350);
  auto gate3MeV_pP = d_VS_DC_pP->ProjectionY("gate3MeVpP", 250, 350);
  auto gate3MeV_4Mev_p = d_VS_DC_p->ProjectionY("gate3MeV-gate4MeVp", 250, 350);
  auto gate3MeV_4Mev_pP = d_VS_DC_pP->ProjectionY("gate3MeVP-gate4MeVp", 250, 350);
  auto gate4MeV_p = d_VS_DC_p->ProjectionY("gate4MeVp", 350, 450);
  auto gate4MeV_pP = d_VS_DC_pP->ProjectionY("gate4MeVpP", 350, 450);
  
  gate3MeV_2Mev_p->Add(gate2MeV_p, -1);
  gate3MeV_2Mev_pP->Add(gate2MeV_pP, -1);
  gate3MeV_2Mev_pP->SetLineColor(kGreen);
  gate3MeV_4Mev_p->Add(gate4MeV_p, -1);
  gate3MeV_4Mev_pP->Add(gate4MeV_pP, -1);
  gate3MeV_4Mev_pP->SetLineColor(kGreen);
  gate2MeV_p->Add(gate4MeV_p, -1); gate2MeV_p->SetName("gate2MeV-gate4MeV");
  gate2MeV_pP->Add(gate4MeV_pP, -1); gate2MeV_pP->SetName("gate2MeV-gate4MeV");
  gate2MeV_pP->SetLineColor(kGreen);

  auto c = new TCanvas;
  c->Divide(2,2);
  c->cd(1);
  gate3MeV_2Mev_p->Draw();
  gate3MeV_2Mev_pP->Draw("same");
  CoLib::normalizeHistos();
  c->cd(2);
  gate3MeV_4Mev_p->Draw();
  gate3MeV_4Mev_pP->Draw("same");
  CoLib::normalizeHistos();
  c->cd(3);
  gate2MeV_p->Draw();
  gate2MeV_pP->Draw("same");
  CoLib::normalizeHistos();

  // Autre piste :

  auto d_VS_DC_VS_PC = file->Get<TH3F>("d_VS_DC_VS_PC");

  auto DC_VS_PC_d642 = myProjectionXY(d_VS_DC_VS_PC, 640, 644, "DC_VS_PC_d642");
  auto DC_VS_PC_d642_2 = myProjectionXY(d_VS_DC_VS_PC, 640, 644, "DC_VS_PC_d642_2");

  auto DC_VS_PC_d637 = myProjectionXY(d_VS_DC_VS_PC, 635, 639, "DC_VS_PC_d637");
  auto DC_VS_PC_d653 = myProjectionXY(d_VS_DC_VS_PC, 651, 655, "DC_VS_PC_d653");

  DC_VS_PC_d642->Add(DC_VS_PC_d637, -1);
  DC_VS_PC_d642_2->Add(DC_VS_PC_d653, -1);

  auto c2 = new TCanvas;
  c2->Divide(1,2);
  c2->cd(1);
  DC_VS_PC_d642->ProjectionY("DC_642_P", 1, -1)->Draw();
  DC_VS_PC_d642_2->ProjectionY("DC_642_P_2", 1, -1)->Draw("same");
  gPad->SetLogy();

  // Veto remove
  auto d_VS_DC_VS_PC_particleveto = file->Get<TH3F>("d_VS_DC_VS_PC_particleveto");
  auto clean_d_VS_DC_VS_PC = CoLib::removeVeto(d_VS_DC_VS_PC, d_VS_DC_VS_PC_particleveto, 1.005);

  auto clean_DC_VS_PC_d642 = myProjectionXY(clean_d_VS_DC_VS_PC, 640, 644, "clean_DC_VS_PC_d642");
  auto clean_DC_VS_PC_d642_2 = myProjectionXY(clean_d_VS_DC_VS_PC, 640, 644, "clean_DC_VS_PC_d642_2");

  auto clean_DC_VS_PC_d637 = myProjectionXY(clean_d_VS_DC_VS_PC, 635, 639, "clean_DC_VS_PC_d637");
  auto clean_DC_VS_PC_d653 = myProjectionXY(clean_d_VS_DC_VS_PC, 651, 655, "clean_DC_VS_PC_d653");

  clean_DC_VS_PC_d642->Add(clean_DC_VS_PC_d637, -1);
  clean_DC_VS_PC_d642_2->Add(clean_DC_VS_PC_d653, -1);

  c2->cd(2);
  clean_DC_VS_PC_d642->ProjectionY("clean_DC_642_P", 1, -1)->Draw();
  clean_DC_VS_PC_d642_2->ProjectionY("clean_DC_642_P_2", 1, -1)->Draw("same");
  gPad->SetLogy();

  new TCanvas;

  d_VS_DC_VS_PC->ProjectionZ("test", 2, 30, 20, 30)->Draw()
}