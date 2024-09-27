#include "../lib/libRoot.hpp"

#include "../lib/Classes/Calibration.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"

float const rel_resolution = 2.79/100;
std::vector<float> const peaks = {5155_keV, 5486_keV, 5805_keV};

double peakPos(TH1* h, double raw_peak, double resolution)
{
  print(raw_peak);
  auto const & axis = h->GetXaxis();
  axis->SetRangeUser(raw_peak-resolution, raw_peak+resolution);
  auto mean = h->GetMean();
  axis->SetRangeUser(mean-resolution, mean+resolution);
  mean = h->GetMean();
  axis->SetRangeUser(mean-0.5*resolution, mean+0.5*resolution);
  auto ret = h->GetMean();
  axis->UnZoom();
  return ret;
}

void AlphaOvernightCalib()
{
  Event event;
  Nuball2Tree tree("alpha_overnight.root", event);
  if (!tree) {error("no file"); return;}
  auto nrj_VS_label = new TH2F("nrj_VS_label", "nrj_VS_label;label;ADC", 1000,0,1000, 10000,0,1000000);
  auto nrjcal_VS_label = new TH2F("nrjcal_VS_label", "nrjcal_VS_label;label;Energy [keV]", 1000,0,1000, 10000,0,10000);
  while(tree.readNext()) for (int hit_i = 0; hit_i<event.mult; ++hit_i) nrj_VS_label->Fill(event.labels[hit_i], event.adcs[hit_i]);
  Calibration calib; calib.resize(1000);
  auto projs = allProjectionsY(nrj_VS_label);
  std::vector<TGraph*> graphs;
  for (int x = 0; x<projs.size(); ++x)
  {
    auto proj = projs[x];
    proj->SetTitle(std::to_string(x).c_str());
    if (!proj || proj->Integral()<1) continue;
    auto axis = proj->GetXaxis();
    axis->SetRangeUser(1000, axis->GetXmax());
    auto const & max = proj->GetMaximum();
    auto lastPeak = axis->GetBinLowEdge(proj->FindLastBinAbove(max/4.));
    auto resolution = rel_resolution*lastPeak;
    proj->Draw();
    
    auto const & kpx = peaks.back()/lastPeak;

    std::vector<float> raw_peaks;
    for (auto const & peak : peaks) raw_peaks.push_back(peakPos(proj, peak/kpx, resolution));
    axis->SetRangeUser(raw_peaks.front()-5*resolution, raw_peaks.back()+5*resolution);
    for (auto const & peak : raw_peaks)
    {
      TMarker *marker = new TMarker(peak, proj->GetBinContent(axis->FindBin(peak)), 20);  // (x, y) are the coordinates, 20 is the marker style
      marker->SetMarkerColor(kRed);   // Set the marker color (e.g., red)
      marker->SetMarkerSize(5);       // Set the marker size
      marker->Draw("same");
    }
    gPad->Update();
    gPad->WaitPrimitive();
    gPad->Update();
    // print(raw_peaks, peaks);
    auto graph = new TGraph(raw_peaks.size(), raw_peaks.data(), peaks.data());
    auto fit = new TF1("pol1", "pol1");
    graph->Fit(fit, "Q");
    auto label = nrj_VS_label->GetXaxis()->GetBinLowEdge(x);
    print(x, label);
    calib.set(label, fit->GetParameter(0), fit->GetParameter(1));
  }
  tree.reset();
  calib.write("triple_alpha");
  while(tree.readNext()) for (int hit_i = 0; hit_i<event.mult; ++hit_i) nrjcal_VS_label->Fill(event.labels[hit_i], calib.calibrate(event.adcs[hit_i], event.labels[hit_i]));
  auto outfile = TFile::Open("alpha_overnight_cal_hist.root", "recreate");
  outfile->cd();
  nrj_VS_label->Write();
  nrjcal_VS_label->Write();
  for (auto & graph : graphs) graph->Write();
  outfile->Close();
  print("alpha_overnight_cal_hist.root written");
}