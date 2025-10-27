#include "../../lib/libRoot.hpp"

void Halflife_reader(std::string filename = "../merge_dC1.root")
{
  auto file = TFile::Open(filename.c_str());
  if (!file) Colib::throw_error(filename+ "absent !");
  auto ddt = file->Get<TH3F>("ddt");
  auto ddt_veto = file->Get<TH3F>("ddt_veto");
  auto dd_p = file->Get<TH2F>("dd_p");
  Colib::removeBackground(dd_p, 15);
  if (!ddt || !ddt_veto) Colib::throw_error("Histograms not present");
  auto clean_ddt = Colib::removeVeto(ddt, ddt_veto, 1.10335, "clean_ddt");
  clean_ddt->SetTitle(clean_ddt->GetName());
  auto dt642 = Colib::myProjectionXZ(clean_ddt, 640/2, 644/2, "dt642");
  new TCanvas;
  // ddt->ProjectionX()->Draw();
  clean_ddt->ProjectionX()->Draw();
  Colib::Pad::get_histos()[0]->GetXaxis()->SetRangeUser(0, 400);
  dt642->ProjectionX()->Draw("same");
  dd_p->ProjectionX("dd642", 641, 643)->Draw("same");
  int i = 1;
  for (auto const & histo : Colib::Pad::get_histos()) histo->SetLineColor(++i);
  gPad->BuildLegend();
  Colib::Pad::normalize_histos();
  // gPad->
  // new TCanvas;
  // dt642->Draw();
  // gPad->WaitPrimitive();
  // std::vector<std::pair<int, int>> bins = {{204,208}, {242,246}, {278,282}, {298,302}, {306,310}};
  // std::vector<std::pair<int, int>> bckg = {{210,214}, {250,254}, {288,292}, {288,292}, {316,320}};
  // std::vector<TH2F*> projs;
  // for (int i = 0; i<bins.size(); ++i)
  // {
  //   auto const & p = bins[i];
  //   auto const & b = bckg[i];
  //   auto value = int((p.first+p.second)/2.);
  //   auto value_str = std::to_string(value);

  //   auto gate = myProjectionY(dt642, ("T"+value_str).c_str(), p.first/2, p.second/2, b.first/2, b.second/2); 
  //   gate->SetTitle(("T"+value_str).c_str()); gate->SetName(("T"+value_str).c_str());

  //   projs.emplace_back(Colib::myProjectionXZ(clean_ddt, p.first/2, p.second/2, ("dt"+value_str).c_str()));
  //   auto gate642 = myProjectionY(projs.back(), ("T642_g"+value_str).c_str(), 640/2, 644/2, 650/2, 654/2);

  //   auto c = new TCanvas;
  //   c->Divide(2,2);
  //   c->cd(1);
  //   gate->Draw();
  //   c->cd(2);
  //   gate642->Draw();
  //   print(value, Colib::calculateHalfLife(gate));
  //   print(value, Colib::calculateHalfLife(gate642));
  // }
}