#include "../lib/libRoot.hpp"

void placeText(TH1* histo, std::string text_str, double x, double y, double size = 0.04, double rotation = 0, TCanvas * c = nullptr)
{
  if (c) c->cd();
  if (size <= 0) size = 0.04;
  auto text = new TLatex;
  text->SetTextAngle(rotation);
  text->SetTextSize(size);
  text->DrawLatex(x, y, text_str.c_str());
}

auto placeTextPeak(TH1* histo, std::string text_str, double peak, double size = 0.04, double rotation = 0, TCanvas * c = nullptr)
{
  auto const & y = histo->GetBinContent(histo->GetXaxis()->FindBin(peak));
  auto const & yshift = histo->GetMaximum()*0.02;
  placeText(histo, text_str, peak, y+yshift, size, rotation, c);
};

auto placeLine(double x1, double y1, double x2, double y2, double width = 2, double color = kBlack, TCanvas * c = nullptr)
{
  auto *line = new TLine(x1, y1, x2, y2);
  line->SetLineColor(color);
  line->SetLineWidth(width);
  line->Draw();
};

void EPJA_Kisomer()
{
  auto file = TFile::Open("data/merge_C2.root","read");
  file->cd();

  auto dd = file->Get<TH2F>("dd");
  auto dd_prompt_veto = file->Get<TH2F>("dd_prompt_veto");
  auto dd_veto_clean = CoLib::removeVeto(dd, dd_prompt_veto, 500, 520);
  CoLib::removeBackground(dd_veto_clean, 15);

  auto dd_p = file->Get<TH2F>("dd_p");
  auto dd_p_prompt_veto = file->Get<TH2F>("dd_p_prompt_veto");
  auto dd_p_veto_clean = CoLib::removeVeto(dd_p, dd_p_prompt_veto, 500, 520);
  CoLib::removeBackground(dd_p_veto_clean, 15);

  auto dp_p = file->Get<TH2F>("dp_p");

  //////////////
  // GATE 903 //
  //////////////

  auto canvas903 = new TCanvas("canvas903", "canvas903", 1000, 600); canvas903->cd();

  auto d903 = dd_veto_clean->ProjectionX("d903", 903, 904);
  CoLib::Multiply(d903, 0.5);
  auto const & d903_xaxis = d903->GetXaxis();
  d903_xaxis->SetTitle("E_{#gamma delayed} [keV]");

  d903_xaxis->SetRangeUser(0,1000);

  d903->Draw();  
  
  double angle_rotation = 0;

  auto text_peak104 = new TLatex;
  double x104 = 105;
  double y104 = d903->GetBinContent(d903_xaxis->FindBin(x104));
  text_peak104->SetTextAngle(angle_rotation);
  text_peak104->SetTextSize(0.040);
  text_peak104->DrawLatex(x104, y104, "104 keV 4^{+}_{K=0^{+}}#rightarrow2^{+}_{K=0^{+}}");

  auto text_peak511 = new TLatex;
  double x511 = 510;
  double y511 = d903->GetBinContent(d903_xaxis->FindBin(x511));
  text_peak511->SetTextAngle(angle_rotation);
  text_peak511->SetTextSize(0.040);
  text_peak511->DrawLatex(x511, y511, "511 keV #beta^{+} decay");

  d903->SetTitle("gate delayed 903");

  TLegend *legend_d903 = new TLegend(0.55, 0.8, 0.9, 0.9);
  legend_d903->SetBorderSize(0);
  legend_d903->SetFillStyle(0);
  legend_d903->SetTextSize(0.05);
  legend_d903->AddEntry(d903, d903->GetTitle(), "l");
  
  legend_d903->Draw();

  CoLib::Pad::remove_stats();

  ///////////////////////
  // GATE 903 particle //
  ///////////////////////

  auto canvas903_p = new TCanvas("canvas903_p", "canvas903_p", 1000, 600); canvas903_p->cd();

  auto d903_p = dd_p_veto_clean->ProjectionX("d903_p", 903, 904);
  CoLib::Multiply(d903_p, 0.5);
  auto const & d903_p_xaxis = d903_p->GetXaxis();
  d903_p_xaxis->SetTitle("E_{#gamma delayed} [keV]");

  d903_p_xaxis->SetRangeUser(0,1000);

  d903_p->Draw();  
  
  angle_rotation = 0;

  auto text_peak104_p = new TLatex;
  double x104_d903_p = 105;
  double y104_d903_p = d903_p->GetBinContent(d903_p_xaxis->FindBin(x104_d903_p));
  text_peak104_p->SetTextAngle(angle_rotation);
  text_peak104_p->SetTextSize(0.040);
  text_peak104_p->DrawLatex(x104_d903_p, y104_d903_p, "104 keV 4^{+}_{K=0^{+}}#rightarrow2^{+}_{K=0^{+}}");

  auto text_peak511_d903_p = new TLatex;
  double x511_903_p = 510;
  double y511_903_p = d903_p->GetBinContent(d903_p_xaxis->FindBin(x511_903_p));
  text_peak511_d903_p->SetTextAngle(angle_rotation);
  text_peak511_d903_p->SetTextSize(0.040);
  text_peak511_d903_p->DrawLatex(x511, y511, "511 keV #beta^{+} decay");

  d903_p->SetTitle("gate delayed 903");

  TLegend *legend_d903_p = new TLegend(0.55, 0.8, 0.9, 0.9);
  legend_d903_p->SetBorderSize(0);
  legend_d903_p->SetFillStyle(0);
  legend_d903_p->SetTextSize(0.05);
  legend_d903_p->AddEntry(d903_p, d903_p->GetTitle(), "l");
  
  legend_d903_p->Draw();

  CoLib::Pad::remove_stats();

  /////////////////////
  // GATE 642 proton //
  /////////////////////

  auto canvas642 = new TCanvas("canvas642", "canvas642", 1000, 600); canvas642->cd();

  auto d642 = dd_p_veto_clean->ProjectionX("d642", 641, 644);
  CoLib::Multiply(d642, 0.5);
  auto const & d642_xaxis = d642->GetXaxis();
  d642_xaxis->SetTitle("E_{#gamma delayed} [keV]");
  d642_xaxis->SetRangeUser(0,500);
  d642->GetYaxis()->SetRangeUser(d642->GetMinimum(), 800);

  d642->Draw();

  double XrayX = 100;
  double XrayY = 250;
  placeText(d642, "X-rays", XrayX, XrayY);
  placeLine(96, 120, XrayX+20, XrayY);
  placeLine(99, 200, XrayX+20, XrayY);
  placeLine(111, 60, XrayX+20, XrayY);

  placeText(d642, "104/105", 125, 100);
  placeLine(105, 40, 125, 100);
  placeTextPeak(d642, "205", 205);
  placeText(d642, "222", 215, 50);
  placeTextPeak(d642, "244", 244);
  placeTextPeak(d642, "279", 279);
  placeText(d642, "301", 285, 205);
  placeTextPeak(d642, "308", 308);
  placeTextPeak(d642, "350", 350);
   
  d642->SetTitle("gate delayed 642");

  CoLib::Pad::remove_background();
  CoLib::Pad::remove_stats();

  double resolution = 2.5;
  auto nndc642 = new TH1F("NNDC","NNDC",500,0,500);
  CoLib::simulatePeak(nndc642, 103, resolution, 106);
  CoLib::simulatePeak(nndc642, 204, resolution, 2160);
  CoLib::simulatePeak(nndc642, 243, resolution, 26);
  CoLib::simulatePeak(nndc642, 300, resolution, 306);
  CoLib::simulatePeak(nndc642, 307, resolution, 81);
  CoLib::shiftX(nndc642, 1);
  nndc642->SetLineColor(kRed);
  nndc642->Draw("same");

  TLegend *legend1 = new TLegend(0.55, 0.8, 0.9, 0.9);
  legend1->SetBorderSize(0);
  legend1->SetFillStyle(0);
  legend1->SetTextSize(0.05);

  legend1->AddEntry(d642, d642->GetTitle(), "l");
  legend1->AddEntry(nndc642, nndc642->GetTitle(), "l");
  
  legend1->Draw();

  /////////////////////////////////
  // DELAYED 642 PROMPT SPECTRUM //
  /////////////////////////////////

  auto canvasPrompt642 = new TCanvas("canvasPrompt642", "canvasPrompt642", 1000, 600); canvasPrompt642->cd();

  auto d642p = dp_p->ProjectionX("gate_delayed_642", 641, 644);
  auto dp_p_projX = dp_p->ProjectionX();

  d642p -> Draw();
  dp_p_projX -> Draw("same");
  d642p->GetXaxis()->SetRangeUser(212, 215);
  CoLib::Pad::normalize_histos_min();
  d642p->GetXaxis()->UnZoom();
  CoLib::Pad::subtract_histos();
  delete dp_p_projX;
  CoLib::Pad::remove_background();
  gPad->Update();
  d642p->GetXaxis()->SetRangeUser(0, 1000);
  d642p->GetYaxis()->SetRangeUser(0, 500);
  d642p->GetXaxis()->SetTitle("E_{#gamma prompt} [keV]");

  XrayX = 140;
  XrayY = 250;
  placeText(d642p, "X-rays", XrayX, XrayY);
  placeLine(96, 300, XrayX, XrayY);
  placeLine(99, 350, XrayX, XrayY);
  placeLine(111, 180, XrayX, XrayY);
  placeLine(115, 80, XrayX, XrayY);
  
  placeTextPeak(d642p, "136", 135);
  placeText(d642p, "239", 215, 40);
  placeText(d642p, "294", 255, 60);
  placeText(d642p, "308", 290, 100);
  placeTextPeak(d642p, "367", 367);
  placeTextPeak(d642p, "(511)", 510, 0, 90);
  placeText(d642p, "522", 521, 75);
  placeTextPeak(d642p, "721", 721);

  d642p->SetTitle("gate delayed 642");

  TLegend *legend_d642p = new TLegend(0.55, 0.8, 0.9, 0.9);
  legend_d642p->SetBorderSize(0);
  legend_d642p->SetFillStyle(0);
  legend_d642p->SetTextSize(0.05);
  legend_d642p->AddEntry(d642p, d642p->GetTitle(), "l");

  legend_d642p->Draw();

  CoLib::Pad::remove_stats();

  ///////////
  // WRITE //
  ///////////

  TString outname = "EPJA_figures.root";
  auto outfile = TFile::Open(outname, "recreate");
  outfile->cd();
    canvas903->Write();
    canvas903_p->Write();
    canvas642->Write();
    canvasPrompt642->Write();
  outfile->Close();
  file->Close();
  print(outname, "written");

}

int main()
{
  EPJA_Kisomer();
  return 1;
}

// g++ -O2 -o epja EPJA_Kisomer.C ` root-config --cflags` `root-config --glibs` -lSpectrum