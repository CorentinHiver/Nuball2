#ifdef __CLING__
#pragma cling optimize(0)
#endif
void Ge_R2()
{
//=========Macro generated from canvas: Ge_R2/Ge_R2
//=========  (Fri Sep 23 11:31:41 2022) by ROOT version 6.26/04
   TCanvas *Ge_R2 = new TCanvas("Ge_R2", "Ge_R2",0,64,2560,1376);
   Ge_R2->SetHighLightColor(2);
   Ge_R2->Range(0,0,1,1);
   Ge_R2->SetFillColor(0);
   Ge_R2->SetBorderMode(0);
   Ge_R2->SetBorderSize(2);
   Ge_R2->SetFrameBorderMode(0);
  
// ------------>Primitives in pad: Ge_R2_1
   TPad *Ge_R2_1 = new TPad("Ge_R2_1", "Ge_R2_1",0.01,0.76,0.07333333,0.99);
   Ge_R2_1->Draw();
   Ge_R2_1->cd();
   Ge_R2_1->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_1->SetFillColor(0);
   Ge_R2_1->SetBorderMode(0);
   Ge_R2_1->SetBorderSize(2);
   Ge_R2_1->SetFrameBorderMode(0);
   Ge_R2_1->SetFrameBorderMode(0);
   
   Double_t Graph0_fx1[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy1[5] = {
   -0.3239365,
   0.1097412,
   0.7268066,
   -0.260498,
   -0.1833496};
   TGraph *graph = new TGraph(5,Graph0_fx1,Graph0_fy1);
   graph->SetName("Graph0");
   graph->SetTitle("R2A1_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph01 = new TH1F("Graph_Graph01","R2A1_red",100,0,1536.634);
   Graph_Graph01->SetMinimum(-1.5);
   Graph_Graph01->SetMaximum(1.5);
   Graph_Graph01->SetDirectory(0);
   Graph_Graph01->SetStats(0);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#000099");
   Graph_Graph01->SetLineColor(ci);
   Graph_Graph01->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph01->GetXaxis()->SetLabelFont(42);
   Graph_Graph01->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01->GetXaxis()->SetTitleFont(42);
   Graph_Graph01->GetYaxis()->SetLabelFont(42);
   Graph_Graph01->GetYaxis()->SetTitleFont(42);
   Graph_Graph01->GetZaxis()->SetLabelFont(42);
   Graph_Graph01->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph01);
   
   graph->Draw("alp");
   
   TPaveText *pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   TText *pt_LaTex = pt->AddText("R2A1_red");
   pt->Draw();
   Ge_R2_1->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_2
   TPad *Ge_R2_2 = new TPad("Ge_R2_2", "Ge_R2_2",0.09333333,0.76,0.1566667,0.99);
   Ge_R2_2->Draw();
   Ge_R2_2->cd();
   Ge_R2_2->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_2->SetFillColor(0);
   Ge_R2_2->SetBorderMode(0);
   Ge_R2_2->SetBorderSize(2);
   Ge_R2_2->SetFrameBorderMode(0);
   Ge_R2_2->SetFrameBorderMode(0);
   
   Double_t Graph0_fx2[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy2[5] = {
   0.1016388,
   0.4710693,
   -0.7022705,
   -0.3335571,
   0.524292};
   graph = new TGraph(5,Graph0_fx2,Graph0_fy2);
   graph->SetName("Graph0");
   graph->SetTitle("R2A1_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph02 = new TH1F("Graph_Graph02","R2A1_green",100,0,1536.634);
   Graph_Graph02->SetMinimum(-1.5);
   Graph_Graph02->SetMaximum(1.5);
   Graph_Graph02->SetDirectory(0);
   Graph_Graph02->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph02->SetLineColor(ci);
   Graph_Graph02->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph02->GetXaxis()->SetLabelFont(42);
   Graph_Graph02->GetXaxis()->SetTitleOffset(1);
   Graph_Graph02->GetXaxis()->SetTitleFont(42);
   Graph_Graph02->GetYaxis()->SetLabelFont(42);
   Graph_Graph02->GetYaxis()->SetTitleFont(42);
   Graph_Graph02->GetZaxis()->SetLabelFont(42);
   Graph_Graph02->GetZaxis()->SetTitleOffset(1);
   Graph_Graph02->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph02);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A1_green");
   pt->Draw();
   Ge_R2_2->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_3
   TPad *Ge_R2_3 = new TPad("Ge_R2_3", "Ge_R2_3",0.1766667,0.76,0.24,0.99);
   Ge_R2_3->Draw();
   Ge_R2_3->cd();
   Ge_R2_3->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_3->SetFillColor(0);
   Ge_R2_3->SetBorderMode(0);
   Ge_R2_3->SetBorderSize(2);
   Ge_R2_3->SetFrameBorderMode(0);
   Ge_R2_3->SetFrameBorderMode(0);
   
   Double_t Graph0_fx3[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy3[5] = {
   -11.58192,
   3.95636,
   -0.489563,
   3.117981,
   -1.661743};
   graph = new TGraph(5,Graph0_fx3,Graph0_fy3);
   graph->SetName("Graph0");
   graph->SetTitle("R2A1_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph03 = new TH1F("Graph_Graph03","R2A1_black",100,0,1536.634);
   Graph_Graph03->SetMinimum(-1.5);
   Graph_Graph03->SetMaximum(1.5);
   Graph_Graph03->SetDirectory(0);
   Graph_Graph03->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph03->SetLineColor(ci);
   Graph_Graph03->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph03->GetXaxis()->SetLabelFont(42);
   Graph_Graph03->GetXaxis()->SetTitleOffset(1);
   Graph_Graph03->GetXaxis()->SetTitleFont(42);
   Graph_Graph03->GetYaxis()->SetLabelFont(42);
   Graph_Graph03->GetYaxis()->SetTitleFont(42);
   Graph_Graph03->GetZaxis()->SetLabelFont(42);
   Graph_Graph03->GetZaxis()->SetTitleOffset(1);
   Graph_Graph03->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph03);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A1_black");
   pt->Draw();
   Ge_R2_3->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_4
   TPad *Ge_R2_4 = new TPad("Ge_R2_4", "Ge_R2_4",0.26,0.76,0.3233333,0.99);
   Ge_R2_4->Draw();
   Ge_R2_4->cd();
   Ge_R2_4->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_4->SetFillColor(0);
   Ge_R2_4->SetBorderMode(0);
   Ge_R2_4->SetBorderSize(2);
   Ge_R2_4->SetFrameBorderMode(0);
   Ge_R2_4->SetFrameBorderMode(0);
   
   Double_t Graph0_fx4[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy4[5] = {
   0.1298523,
   0.1688843,
   -0.4597168,
   0.0078125,
   0.239502};
   graph = new TGraph(5,Graph0_fx4,Graph0_fy4);
   graph->SetName("Graph0");
   graph->SetTitle("R2A1_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph04 = new TH1F("Graph_Graph04","R2A1_blue",100,0,1536.634);
   Graph_Graph04->SetMinimum(-1.5);
   Graph_Graph04->SetMaximum(1.5);
   Graph_Graph04->SetDirectory(0);
   Graph_Graph04->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph04->SetLineColor(ci);
   Graph_Graph04->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph04->GetXaxis()->SetLabelFont(42);
   Graph_Graph04->GetXaxis()->SetTitleOffset(1);
   Graph_Graph04->GetXaxis()->SetTitleFont(42);
   Graph_Graph04->GetYaxis()->SetLabelFont(42);
   Graph_Graph04->GetYaxis()->SetTitleFont(42);
   Graph_Graph04->GetZaxis()->SetLabelFont(42);
   Graph_Graph04->GetZaxis()->SetTitleOffset(1);
   Graph_Graph04->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph04);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A1_blue");
   pt->Draw();
   Ge_R2_4->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_5
   TPad *Ge_R2_5 = new TPad("Ge_R2_5", "Ge_R2_5",0.3433333,0.76,0.4066667,0.99);
   Ge_R2_5->Draw();
   Ge_R2_5->cd();
   Ge_R2_5->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_5->SetFillColor(0);
   Ge_R2_5->SetBorderMode(0);
   Ge_R2_5->SetBorderSize(2);
   Ge_R2_5->SetFrameBorderMode(0);
   Ge_R2_5->SetFrameBorderMode(0);
   
   Double_t Graph0_fx5[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy5[5] = {
   -0.0423584,
   -0.04278564,
   0.7084351,
   -0.5350342,
   0.0411377};
   graph = new TGraph(5,Graph0_fx5,Graph0_fy5);
   graph->SetName("Graph0");
   graph->SetTitle("R2A2_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph05 = new TH1F("Graph_Graph05","R2A2_red",100,0,1536.634);
   Graph_Graph05->SetMinimum(-1.5);
   Graph_Graph05->SetMaximum(1.5);
   Graph_Graph05->SetDirectory(0);
   Graph_Graph05->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph05->SetLineColor(ci);
   Graph_Graph05->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph05->GetXaxis()->SetLabelFont(42);
   Graph_Graph05->GetXaxis()->SetTitleOffset(1);
   Graph_Graph05->GetXaxis()->SetTitleFont(42);
   Graph_Graph05->GetYaxis()->SetLabelFont(42);
   Graph_Graph05->GetYaxis()->SetTitleFont(42);
   Graph_Graph05->GetZaxis()->SetLabelFont(42);
   Graph_Graph05->GetZaxis()->SetTitleOffset(1);
   Graph_Graph05->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph05);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A2_red");
   pt->Draw();
   Ge_R2_5->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_6
   TPad *Ge_R2_6 = new TPad("Ge_R2_6", "Ge_R2_6",0.4266667,0.76,0.49,0.99);
   Ge_R2_6->Draw();
   Ge_R2_6->cd();
   Ge_R2_6->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_6->SetFillColor(0);
   Ge_R2_6->SetBorderMode(0);
   Ge_R2_6->SetBorderSize(2);
   Ge_R2_6->SetFrameBorderMode(0);
   Ge_R2_6->SetFrameBorderMode(0);
   
   Double_t Graph0_fx6[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy6[5] = {
   -0.1917038,
   -0.04928589,
   0.7106934,
   -0.2001953,
   -0.1629639};
   graph = new TGraph(5,Graph0_fx6,Graph0_fy6);
   graph->SetName("Graph0");
   graph->SetTitle("R2A2_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph06 = new TH1F("Graph_Graph06","R2A2_green",100,0,1536.634);
   Graph_Graph06->SetMinimum(-1.5);
   Graph_Graph06->SetMaximum(1.5);
   Graph_Graph06->SetDirectory(0);
   Graph_Graph06->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph06->SetLineColor(ci);
   Graph_Graph06->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph06->GetXaxis()->SetLabelFont(42);
   Graph_Graph06->GetXaxis()->SetTitleOffset(1);
   Graph_Graph06->GetXaxis()->SetTitleFont(42);
   Graph_Graph06->GetYaxis()->SetLabelFont(42);
   Graph_Graph06->GetYaxis()->SetTitleFont(42);
   Graph_Graph06->GetZaxis()->SetLabelFont(42);
   Graph_Graph06->GetZaxis()->SetTitleOffset(1);
   Graph_Graph06->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph06);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A2_green");
   pt->Draw();
   Ge_R2_6->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_7
   TPad *Ge_R2_7 = new TPad("Ge_R2_7", "Ge_R2_7",0.51,0.76,0.5733333,0.99);
   Ge_R2_7->Draw();
   Ge_R2_7->cd();
   Ge_R2_7->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_7->SetFillColor(0);
   Ge_R2_7->SetBorderMode(0);
   Ge_R2_7->SetBorderSize(2);
   Ge_R2_7->SetFrameBorderMode(0);
   Ge_R2_7->SetFrameBorderMode(0);
   
   Double_t Graph0_fx7[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy7[5] = {
   -0.2758179,
   0.1980286,
   0.7265015,
   -0.5725708,
   0.02490234};
   graph = new TGraph(5,Graph0_fx7,Graph0_fy7);
   graph->SetName("Graph0");
   graph->SetTitle("R2A2_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph07 = new TH1F("Graph_Graph07","R2A2_black",100,0,1536.634);
   Graph_Graph07->SetMinimum(-1.5);
   Graph_Graph07->SetMaximum(1.5);
   Graph_Graph07->SetDirectory(0);
   Graph_Graph07->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph07->SetLineColor(ci);
   Graph_Graph07->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph07->GetXaxis()->SetLabelFont(42);
   Graph_Graph07->GetXaxis()->SetTitleOffset(1);
   Graph_Graph07->GetXaxis()->SetTitleFont(42);
   Graph_Graph07->GetYaxis()->SetLabelFont(42);
   Graph_Graph07->GetYaxis()->SetTitleFont(42);
   Graph_Graph07->GetZaxis()->SetLabelFont(42);
   Graph_Graph07->GetZaxis()->SetTitleOffset(1);
   Graph_Graph07->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph07);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A2_black");
   pt->Draw();
   Ge_R2_7->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_8
   TPad *Ge_R2_8 = new TPad("Ge_R2_8", "Ge_R2_8",0.5933333,0.76,0.6566667,0.99);
   Ge_R2_8->Draw();
   Ge_R2_8->cd();
   Ge_R2_8->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_8->SetFillColor(0);
   Ge_R2_8->SetBorderMode(0);
   Ge_R2_8->SetBorderSize(2);
   Ge_R2_8->SetFrameBorderMode(0);
   Ge_R2_8->SetFrameBorderMode(0);
   
   Double_t Graph0_fx8[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy8[5] = {
   -0.2343216,
   -0.03109741,
   0.6990356,
   -0.1564331,
   -0.1964111};
   graph = new TGraph(5,Graph0_fx8,Graph0_fy8);
   graph->SetName("Graph0");
   graph->SetTitle("R2A2_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph08 = new TH1F("Graph_Graph08","R2A2_blue",100,0,1536.634);
   Graph_Graph08->SetMinimum(-1.5);
   Graph_Graph08->SetMaximum(1.5);
   Graph_Graph08->SetDirectory(0);
   Graph_Graph08->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph08->SetLineColor(ci);
   Graph_Graph08->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph08->GetXaxis()->SetLabelFont(42);
   Graph_Graph08->GetXaxis()->SetTitleOffset(1);
   Graph_Graph08->GetXaxis()->SetTitleFont(42);
   Graph_Graph08->GetYaxis()->SetLabelFont(42);
   Graph_Graph08->GetYaxis()->SetTitleFont(42);
   Graph_Graph08->GetZaxis()->SetLabelFont(42);
   Graph_Graph08->GetZaxis()->SetTitleOffset(1);
   Graph_Graph08->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph08);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A2_blue");
   pt->Draw();
   Ge_R2_8->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_9
   TPad *Ge_R2_9 = new TPad("Ge_R2_9", "Ge_R2_9",0.6766667,0.76,0.74,0.99);
   Ge_R2_9->Draw();
   Ge_R2_9->cd();
   Ge_R2_9->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_9->SetFillColor(0);
   Ge_R2_9->SetBorderMode(0);
   Ge_R2_9->SetBorderSize(2);
   Ge_R2_9->SetFrameBorderMode(0);
   Ge_R2_9->SetFrameBorderMode(0);
   
   Double_t Graph0_fx9[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy9[5] = {
   -0.1888351,
   0.4743958,
   -0.1459351,
   -0.4309082,
   0.2753906};
   graph = new TGraph(5,Graph0_fx9,Graph0_fy9);
   graph->SetName("Graph0");
   graph->SetTitle("R2A3_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph09 = new TH1F("Graph_Graph09","R2A3_red",100,0,1536.634);
   Graph_Graph09->SetMinimum(-1.5);
   Graph_Graph09->SetMaximum(1.5);
   Graph_Graph09->SetDirectory(0);
   Graph_Graph09->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph09->SetLineColor(ci);
   Graph_Graph09->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph09->GetXaxis()->SetLabelFont(42);
   Graph_Graph09->GetXaxis()->SetTitleOffset(1);
   Graph_Graph09->GetXaxis()->SetTitleFont(42);
   Graph_Graph09->GetYaxis()->SetLabelFont(42);
   Graph_Graph09->GetYaxis()->SetTitleFont(42);
   Graph_Graph09->GetZaxis()->SetLabelFont(42);
   Graph_Graph09->GetZaxis()->SetTitleOffset(1);
   Graph_Graph09->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph09);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A3_red");
   pt->Draw();
   Ge_R2_9->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_10
   TPad *Ge_R2_10 = new TPad("Ge_R2_10", "Ge_R2_10",0.76,0.76,0.8233333,0.99);
   Ge_R2_10->Draw();
   Ge_R2_10->cd();
   Ge_R2_10->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_10->SetFillColor(0);
   Ge_R2_10->SetBorderMode(0);
   Ge_R2_10->SetBorderSize(2);
   Ge_R2_10->SetFrameBorderMode(0);
   Ge_R2_10->SetFrameBorderMode(0);
   
   Double_t Graph0_fx10[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy10[5] = {
   -0.5459595,
   -0.02609253,
   0.546875,
   0.9595947,
   -0.8774414};
   graph = new TGraph(5,Graph0_fx10,Graph0_fy10);
   graph->SetName("Graph0");
   graph->SetTitle("R2A3_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph010 = new TH1F("Graph_Graph010","R2A3_green",100,0,1536.634);
   Graph_Graph010->SetMinimum(-1.5);
   Graph_Graph010->SetMaximum(1.5);
   Graph_Graph010->SetDirectory(0);
   Graph_Graph010->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph010->SetLineColor(ci);
   Graph_Graph010->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph010->GetXaxis()->SetLabelFont(42);
   Graph_Graph010->GetXaxis()->SetTitleOffset(1);
   Graph_Graph010->GetXaxis()->SetTitleFont(42);
   Graph_Graph010->GetYaxis()->SetLabelFont(42);
   Graph_Graph010->GetYaxis()->SetTitleFont(42);
   Graph_Graph010->GetZaxis()->SetLabelFont(42);
   Graph_Graph010->GetZaxis()->SetTitleOffset(1);
   Graph_Graph010->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph010);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A3_green");
   pt->Draw();
   Ge_R2_10->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_11
   TPad *Ge_R2_11 = new TPad("Ge_R2_11", "Ge_R2_11",0.8433333,0.76,0.9066667,0.99);
   Ge_R2_11->Draw();
   Ge_R2_11->cd();
   Ge_R2_11->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_11->SetFillColor(0);
   Ge_R2_11->SetBorderMode(0);
   Ge_R2_11->SetBorderSize(2);
   Ge_R2_11->SetFrameBorderMode(0);
   Ge_R2_11->SetFrameBorderMode(0);
   
   Double_t Graph0_fx11[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy11[5] = {
   -0.2514648,
   0.1787415,
   0.6416016,
   -0.5592651,
   0.03735352};
   graph = new TGraph(5,Graph0_fx11,Graph0_fy11);
   graph->SetName("Graph0");
   graph->SetTitle("R2A3_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph011 = new TH1F("Graph_Graph011","R2A3_black",100,0,1536.634);
   Graph_Graph011->SetMinimum(-1.5);
   Graph_Graph011->SetMaximum(1.5);
   Graph_Graph011->SetDirectory(0);
   Graph_Graph011->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph011->SetLineColor(ci);
   Graph_Graph011->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph011->GetXaxis()->SetLabelFont(42);
   Graph_Graph011->GetXaxis()->SetTitleOffset(1);
   Graph_Graph011->GetXaxis()->SetTitleFont(42);
   Graph_Graph011->GetYaxis()->SetLabelFont(42);
   Graph_Graph011->GetYaxis()->SetTitleFont(42);
   Graph_Graph011->GetZaxis()->SetLabelFont(42);
   Graph_Graph011->GetZaxis()->SetTitleOffset(1);
   Graph_Graph011->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph011);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A3_black");
   pt->Draw();
   Ge_R2_11->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_12
   TPad *Ge_R2_12 = new TPad("Ge_R2_12", "Ge_R2_12",0.9266667,0.76,0.99,0.99);
   Ge_R2_12->Draw();
   Ge_R2_12->cd();
   Ge_R2_12->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_12->SetFillColor(0);
   Ge_R2_12->SetBorderMode(0);
   Ge_R2_12->SetBorderSize(2);
   Ge_R2_12->SetFrameBorderMode(0);
   Ge_R2_12->SetFrameBorderMode(0);
   
   Double_t Graph0_fx12[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy12[5] = {
   -0.001197815,
   0.5297241,
   -0.6993408,
   -0.2351685,
   0.4327393};
   graph = new TGraph(5,Graph0_fx12,Graph0_fy12);
   graph->SetName("Graph0");
   graph->SetTitle("R2A3_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph012 = new TH1F("Graph_Graph012","R2A3_blue",100,0,1536.634);
   Graph_Graph012->SetMinimum(-1.5);
   Graph_Graph012->SetMaximum(1.5);
   Graph_Graph012->SetDirectory(0);
   Graph_Graph012->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph012->SetLineColor(ci);
   Graph_Graph012->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph012->GetXaxis()->SetLabelFont(42);
   Graph_Graph012->GetXaxis()->SetTitleOffset(1);
   Graph_Graph012->GetXaxis()->SetTitleFont(42);
   Graph_Graph012->GetYaxis()->SetLabelFont(42);
   Graph_Graph012->GetYaxis()->SetTitleFont(42);
   Graph_Graph012->GetZaxis()->SetLabelFont(42);
   Graph_Graph012->GetZaxis()->SetTitleOffset(1);
   Graph_Graph012->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph012);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A3_blue");
   pt->Draw();
   Ge_R2_12->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_13
   TPad *Ge_R2_13 = new TPad("Ge_R2_13", "Ge_R2_13",0.01,0.51,0.07333333,0.74);
   Ge_R2_13->Draw();
   Ge_R2_13->cd();
   Ge_R2_13->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_13->SetFillColor(0);
   Ge_R2_13->SetBorderMode(0);
   Ge_R2_13->SetBorderSize(2);
   Ge_R2_13->SetFrameBorderMode(0);
   Ge_R2_13->SetFrameBorderMode(0);
   
   Double_t Graph0_fx13[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy13[5] = {
   -0.2425537,
   0.04882812,
   0.7988892,
   -0.5661621,
   -0.03381348};
   graph = new TGraph(5,Graph0_fx13,Graph0_fy13);
   graph->SetName("Graph0");
   graph->SetTitle("R2A4_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph013 = new TH1F("Graph_Graph013","R2A4_red",100,0,1536.634);
   Graph_Graph013->SetMinimum(-1.5);
   Graph_Graph013->SetMaximum(1.5);
   Graph_Graph013->SetDirectory(0);
   Graph_Graph013->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph013->SetLineColor(ci);
   Graph_Graph013->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph013->GetXaxis()->SetLabelFont(42);
   Graph_Graph013->GetXaxis()->SetTitleOffset(1);
   Graph_Graph013->GetXaxis()->SetTitleFont(42);
   Graph_Graph013->GetYaxis()->SetLabelFont(42);
   Graph_Graph013->GetYaxis()->SetTitleFont(42);
   Graph_Graph013->GetZaxis()->SetLabelFont(42);
   Graph_Graph013->GetZaxis()->SetTitleOffset(1);
   Graph_Graph013->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph013);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A4_red");
   pt->Draw();
   Ge_R2_13->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_14
   TPad *Ge_R2_14 = new TPad("Ge_R2_14", "Ge_R2_14",0.09333333,0.51,0.1566667,0.74);
   Ge_R2_14->Draw();
   Ge_R2_14->cd();
   Ge_R2_14->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_14->SetFillColor(0);
   Ge_R2_14->SetBorderMode(0);
   Ge_R2_14->SetBorderSize(2);
   Ge_R2_14->SetFrameBorderMode(0);
   Ge_R2_14->SetFrameBorderMode(0);
   
   Double_t Graph0_fx14[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy14[5] = {
   -0.05706024,
   0.1715698,
   0.3798218,
   -0.593689,
   0.2163086};
   graph = new TGraph(5,Graph0_fx14,Graph0_fy14);
   graph->SetName("Graph0");
   graph->SetTitle("R2A4_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph014 = new TH1F("Graph_Graph014","R2A4_green",100,0,1536.634);
   Graph_Graph014->SetMinimum(-1.5);
   Graph_Graph014->SetMaximum(1.5);
   Graph_Graph014->SetDirectory(0);
   Graph_Graph014->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph014->SetLineColor(ci);
   Graph_Graph014->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph014->GetXaxis()->SetLabelFont(42);
   Graph_Graph014->GetXaxis()->SetTitleOffset(1);
   Graph_Graph014->GetXaxis()->SetTitleFont(42);
   Graph_Graph014->GetYaxis()->SetLabelFont(42);
   Graph_Graph014->GetYaxis()->SetTitleFont(42);
   Graph_Graph014->GetZaxis()->SetLabelFont(42);
   Graph_Graph014->GetZaxis()->SetTitleOffset(1);
   Graph_Graph014->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph014);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A4_green");
   pt->Draw();
   Ge_R2_14->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_15
   TPad *Ge_R2_15 = new TPad("Ge_R2_15", "Ge_R2_15",0.1766667,0.51,0.24,0.74);
   Ge_R2_15->Draw();
   Ge_R2_15->cd();
   Ge_R2_15->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_15->SetFillColor(0);
   Ge_R2_15->SetBorderMode(0);
   Ge_R2_15->SetBorderSize(2);
   Ge_R2_15->SetFrameBorderMode(0);
   Ge_R2_15->SetFrameBorderMode(0);
   
   Double_t Graph0_fx15[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy15[5] = {
   0.001312256,
   0.4757385,
   -0.3629761,
   -0.4028931,
   0.4047852};
   graph = new TGraph(5,Graph0_fx15,Graph0_fy15);
   graph->SetName("Graph0");
   graph->SetTitle("R2A4_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph015 = new TH1F("Graph_Graph015","R2A4_black",100,0,1536.634);
   Graph_Graph015->SetMinimum(-1.5);
   Graph_Graph015->SetMaximum(1.5);
   Graph_Graph015->SetDirectory(0);
   Graph_Graph015->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph015->SetLineColor(ci);
   Graph_Graph015->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph015->GetXaxis()->SetLabelFont(42);
   Graph_Graph015->GetXaxis()->SetTitleOffset(1);
   Graph_Graph015->GetXaxis()->SetTitleFont(42);
   Graph_Graph015->GetYaxis()->SetLabelFont(42);
   Graph_Graph015->GetYaxis()->SetTitleFont(42);
   Graph_Graph015->GetZaxis()->SetLabelFont(42);
   Graph_Graph015->GetZaxis()->SetTitleOffset(1);
   Graph_Graph015->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph015);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A4_black");
   pt->Draw();
   Ge_R2_15->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_16
   TPad *Ge_R2_16 = new TPad("Ge_R2_16", "Ge_R2_16",0.26,0.51,0.3233333,0.74);
   Ge_R2_16->Draw();
   Ge_R2_16->cd();
   Ge_R2_16->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_16->SetFillColor(0);
   Ge_R2_16->SetBorderMode(0);
   Ge_R2_16->SetBorderSize(2);
   Ge_R2_16->SetFrameBorderMode(0);
   Ge_R2_16->SetFrameBorderMode(0);
   
   Double_t Graph0_fx16[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy16[5] = {
   -0.272316,
   0.03851318,
   0.6373901,
   -0.1121216,
   -0.2182617};
   graph = new TGraph(5,Graph0_fx16,Graph0_fy16);
   graph->SetName("Graph0");
   graph->SetTitle("R2A4_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph016 = new TH1F("Graph_Graph016","R2A4_blue",100,0,1536.634);
   Graph_Graph016->SetMinimum(-1.5);
   Graph_Graph016->SetMaximum(1.5);
   Graph_Graph016->SetDirectory(0);
   Graph_Graph016->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph016->SetLineColor(ci);
   Graph_Graph016->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph016->GetXaxis()->SetLabelFont(42);
   Graph_Graph016->GetXaxis()->SetTitleOffset(1);
   Graph_Graph016->GetXaxis()->SetTitleFont(42);
   Graph_Graph016->GetYaxis()->SetLabelFont(42);
   Graph_Graph016->GetYaxis()->SetTitleFont(42);
   Graph_Graph016->GetZaxis()->SetLabelFont(42);
   Graph_Graph016->GetZaxis()->SetTitleOffset(1);
   Graph_Graph016->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph016);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A4_blue");
   pt->Draw();
   Ge_R2_16->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_17
   TPad *Ge_R2_17 = new TPad("Ge_R2_17", "Ge_R2_17",0.3433333,0.51,0.4066667,0.74);
   Ge_R2_17->Draw();
   Ge_R2_17->cd();
   Ge_R2_17->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_17->SetFillColor(0);
   Ge_R2_17->SetBorderMode(0);
   Ge_R2_17->SetBorderSize(2);
   Ge_R2_17->SetFrameBorderMode(0);
   Ge_R2_17->SetFrameBorderMode(0);
   
   Double_t Graph0_fx17[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy17[5] = {
   -0.05932617,
   0.5169983,
   -0.3496704,
   -0.1885376,
   0.298584};
   graph = new TGraph(5,Graph0_fx17,Graph0_fy17);
   graph->SetName("Graph0");
   graph->SetTitle("R2A5_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph017 = new TH1F("Graph_Graph017","R2A5_red",100,0,1536.634);
   Graph_Graph017->SetMinimum(-1.5);
   Graph_Graph017->SetMaximum(1.5);
   Graph_Graph017->SetDirectory(0);
   Graph_Graph017->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph017->SetLineColor(ci);
   Graph_Graph017->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph017->GetXaxis()->SetLabelFont(42);
   Graph_Graph017->GetXaxis()->SetTitleOffset(1);
   Graph_Graph017->GetXaxis()->SetTitleFont(42);
   Graph_Graph017->GetYaxis()->SetLabelFont(42);
   Graph_Graph017->GetYaxis()->SetTitleFont(42);
   Graph_Graph017->GetZaxis()->SetLabelFont(42);
   Graph_Graph017->GetZaxis()->SetTitleOffset(1);
   Graph_Graph017->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph017);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A5_red");
   pt->Draw();
   Ge_R2_17->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_18
   TPad *Ge_R2_18 = new TPad("Ge_R2_18", "Ge_R2_18",0.4266667,0.51,0.49,0.74);
   Ge_R2_18->Draw();
   Ge_R2_18->cd();
   Ge_R2_18->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_18->SetFillColor(0);
   Ge_R2_18->SetBorderMode(0);
   Ge_R2_18->SetBorderSize(2);
   Ge_R2_18->SetFrameBorderMode(0);
   Ge_R2_18->SetFrameBorderMode(0);
   
   Double_t Graph0_fx18[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy18[5] = {
   0.256691,
   -0.3809814,
   -0.5294189,
   -0.1073608,
   0.161499};
   graph = new TGraph(5,Graph0_fx18,Graph0_fy18);
   graph->SetName("Graph0");
   graph->SetTitle("R2A5_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph018 = new TH1F("Graph_Graph018","R2A5_green",100,0,1536.634);
   Graph_Graph018->SetMinimum(-1.5);
   Graph_Graph018->SetMaximum(1.5);
   Graph_Graph018->SetDirectory(0);
   Graph_Graph018->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph018->SetLineColor(ci);
   Graph_Graph018->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph018->GetXaxis()->SetLabelFont(42);
   Graph_Graph018->GetXaxis()->SetTitleOffset(1);
   Graph_Graph018->GetXaxis()->SetTitleFont(42);
   Graph_Graph018->GetYaxis()->SetLabelFont(42);
   Graph_Graph018->GetYaxis()->SetTitleFont(42);
   Graph_Graph018->GetZaxis()->SetLabelFont(42);
   Graph_Graph018->GetZaxis()->SetTitleOffset(1);
   Graph_Graph018->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph018);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A5_green");
   pt->Draw();
   Ge_R2_18->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_19
   TPad *Ge_R2_19 = new TPad("Ge_R2_19", "Ge_R2_19",0.51,0.51,0.5733333,0.74);
   Ge_R2_19->Draw();
   Ge_R2_19->cd();
   Ge_R2_19->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_19->SetFillColor(0);
   Ge_R2_19->SetBorderMode(0);
   Ge_R2_19->SetBorderSize(2);
   Ge_R2_19->SetFrameBorderMode(0);
   Ge_R2_19->SetFrameBorderMode(0);
   
   Double_t Graph0_fx19[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy19[5] = {
   0.00693512,
   0.4770813,
   -0.645813,
   -0.1146851,
   0.3414307};
   graph = new TGraph(5,Graph0_fx19,Graph0_fy19);
   graph->SetName("Graph0");
   graph->SetTitle("R2A5_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph019 = new TH1F("Graph_Graph019","R2A5_black",100,0,1536.634);
   Graph_Graph019->SetMinimum(-1.5);
   Graph_Graph019->SetMaximum(1.5);
   Graph_Graph019->SetDirectory(0);
   Graph_Graph019->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph019->SetLineColor(ci);
   Graph_Graph019->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph019->GetXaxis()->SetLabelFont(42);
   Graph_Graph019->GetXaxis()->SetTitleOffset(1);
   Graph_Graph019->GetXaxis()->SetTitleFont(42);
   Graph_Graph019->GetYaxis()->SetLabelFont(42);
   Graph_Graph019->GetYaxis()->SetTitleFont(42);
   Graph_Graph019->GetZaxis()->SetLabelFont(42);
   Graph_Graph019->GetZaxis()->SetTitleOffset(1);
   Graph_Graph019->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph019);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A5_black");
   pt->Draw();
   Ge_R2_19->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_20
   TPad *Ge_R2_20 = new TPad("Ge_R2_20", "Ge_R2_20",0.5933333,0.51,0.6566667,0.74);
   Ge_R2_20->Draw();
   Ge_R2_20->cd();
   Ge_R2_20->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_20->SetFillColor(0);
   Ge_R2_20->SetBorderMode(0);
   Ge_R2_20->SetBorderSize(2);
   Ge_R2_20->SetFrameBorderMode(0);
   Ge_R2_20->SetFrameBorderMode(0);
   
   Double_t Graph0_fx20[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy20[5] = {
   -1.018929,
   -0.7377319,
   0.4584351,
   0.6159668,
   -0.9998779};
   graph = new TGraph(5,Graph0_fx20,Graph0_fy20);
   graph->SetName("Graph0");
   graph->SetTitle("R2A5_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph020 = new TH1F("Graph_Graph020","R2A5_blue",100,0,1536.634);
   Graph_Graph020->SetMinimum(-1.5);
   Graph_Graph020->SetMaximum(1.5);
   Graph_Graph020->SetDirectory(0);
   Graph_Graph020->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph020->SetLineColor(ci);
   Graph_Graph020->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph020->GetXaxis()->SetLabelFont(42);
   Graph_Graph020->GetXaxis()->SetTitleOffset(1);
   Graph_Graph020->GetXaxis()->SetTitleFont(42);
   Graph_Graph020->GetYaxis()->SetLabelFont(42);
   Graph_Graph020->GetYaxis()->SetTitleFont(42);
   Graph_Graph020->GetZaxis()->SetLabelFont(42);
   Graph_Graph020->GetZaxis()->SetTitleOffset(1);
   Graph_Graph020->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph020);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A5_blue");
   pt->Draw();
   Ge_R2_20->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_21
   TPad *Ge_R2_21 = new TPad("Ge_R2_21", "Ge_R2_21",0.6766667,0.51,0.74,0.74);
   Ge_R2_21->Draw();
   Ge_R2_21->cd();
   Ge_R2_21->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_21->SetFillColor(0);
   Ge_R2_21->SetBorderMode(0);
   Ge_R2_21->SetBorderSize(2);
   Ge_R2_21->SetFrameBorderMode(0);
   Ge_R2_21->SetFrameBorderMode(0);
   
   Double_t Graph0_fx21[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy21[5] = {
   -0.06572723,
   0.4656372,
   -0.164856,
   -0.4021606,
   0.2841797};
   graph = new TGraph(5,Graph0_fx21,Graph0_fy21);
   graph->SetName("Graph0");
   graph->SetTitle("R2A6_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph021 = new TH1F("Graph_Graph021","R2A6_red",100,0,1536.634);
   Graph_Graph021->SetMinimum(-1.5);
   Graph_Graph021->SetMaximum(1.5);
   Graph_Graph021->SetDirectory(0);
   Graph_Graph021->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph021->SetLineColor(ci);
   Graph_Graph021->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph021->GetXaxis()->SetLabelFont(42);
   Graph_Graph021->GetXaxis()->SetTitleOffset(1);
   Graph_Graph021->GetXaxis()->SetTitleFont(42);
   Graph_Graph021->GetYaxis()->SetLabelFont(42);
   Graph_Graph021->GetYaxis()->SetTitleFont(42);
   Graph_Graph021->GetZaxis()->SetLabelFont(42);
   Graph_Graph021->GetZaxis()->SetTitleOffset(1);
   Graph_Graph021->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph021);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A6_red");
   pt->Draw();
   Ge_R2_21->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_22
   TPad *Ge_R2_22 = new TPad("Ge_R2_22", "Ge_R2_22",0.76,0.51,0.8233333,0.74);
   Ge_R2_22->Draw();
   Ge_R2_22->cd();
   Ge_R2_22->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_22->SetFillColor(0);
   Ge_R2_22->SetBorderMode(0);
   Ge_R2_22->SetBorderSize(2);
   Ge_R2_22->SetFrameBorderMode(0);
   Ge_R2_22->SetFrameBorderMode(0);
   
   Double_t Graph0_fx22[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy22[5] = {
   -0.2069168,
   0.5074463,
   -0.01623535,
   -0.5067749,
   0.302002};
   graph = new TGraph(5,Graph0_fx22,Graph0_fy22);
   graph->SetName("Graph0");
   graph->SetTitle("R2A6_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph022 = new TH1F("Graph_Graph022","R2A6_green",100,0,1536.634);
   Graph_Graph022->SetMinimum(-1.5);
   Graph_Graph022->SetMaximum(1.5);
   Graph_Graph022->SetDirectory(0);
   Graph_Graph022->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph022->SetLineColor(ci);
   Graph_Graph022->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph022->GetXaxis()->SetLabelFont(42);
   Graph_Graph022->GetXaxis()->SetTitleOffset(1);
   Graph_Graph022->GetXaxis()->SetTitleFont(42);
   Graph_Graph022->GetYaxis()->SetLabelFont(42);
   Graph_Graph022->GetYaxis()->SetTitleFont(42);
   Graph_Graph022->GetZaxis()->SetLabelFont(42);
   Graph_Graph022->GetZaxis()->SetTitleOffset(1);
   Graph_Graph022->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph022);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A6_green");
   pt->Draw();
   Ge_R2_22->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_23
   TPad *Ge_R2_23 = new TPad("Ge_R2_23", "Ge_R2_23",0.8433333,0.51,0.9066667,0.74);
   Ge_R2_23->Draw();
   Ge_R2_23->cd();
   Ge_R2_23->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_23->SetFillColor(0);
   Ge_R2_23->SetBorderMode(0);
   Ge_R2_23->SetBorderSize(2);
   Ge_R2_23->SetFrameBorderMode(0);
   Ge_R2_23->SetFrameBorderMode(0);
   
   Double_t Graph0_fx23[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy23[5] = {
   -0.2228012,
   0.2528076,
   -0.9606934,
   -0.7429199,
   0.2580566};
   graph = new TGraph(5,Graph0_fx23,Graph0_fy23);
   graph->SetName("Graph0");
   graph->SetTitle("R2A6_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph023 = new TH1F("Graph_Graph023","R2A6_black",100,0,1536.634);
   Graph_Graph023->SetMinimum(-1.5);
   Graph_Graph023->SetMaximum(1.5);
   Graph_Graph023->SetDirectory(0);
   Graph_Graph023->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph023->SetLineColor(ci);
   Graph_Graph023->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph023->GetXaxis()->SetLabelFont(42);
   Graph_Graph023->GetXaxis()->SetTitleOffset(1);
   Graph_Graph023->GetXaxis()->SetTitleFont(42);
   Graph_Graph023->GetYaxis()->SetLabelFont(42);
   Graph_Graph023->GetYaxis()->SetTitleFont(42);
   Graph_Graph023->GetZaxis()->SetLabelFont(42);
   Graph_Graph023->GetZaxis()->SetTitleOffset(1);
   Graph_Graph023->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph023);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A6_black");
   pt->Draw();
   Ge_R2_23->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_24
   TPad *Ge_R2_24 = new TPad("Ge_R2_24", "Ge_R2_24",0.9266667,0.51,0.99,0.74);
   Ge_R2_24->Draw();
   Ge_R2_24->cd();
   Ge_R2_24->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_24->SetFillColor(0);
   Ge_R2_24->SetBorderMode(0);
   Ge_R2_24->SetBorderSize(2);
   Ge_R2_24->SetFrameBorderMode(0);
   Ge_R2_24->SetFrameBorderMode(0);
   
   Double_t Graph0_fx24[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy24[5] = {
   -0.02883911,
   0.3058472,
   -0.08886719,
   -0.5195923,
   0.3210449};
   graph = new TGraph(5,Graph0_fx24,Graph0_fy24);
   graph->SetName("Graph0");
   graph->SetTitle("R2A6_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph024 = new TH1F("Graph_Graph024","R2A6_blue",100,0,1536.634);
   Graph_Graph024->SetMinimum(-1.5);
   Graph_Graph024->SetMaximum(1.5);
   Graph_Graph024->SetDirectory(0);
   Graph_Graph024->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph024->SetLineColor(ci);
   Graph_Graph024->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph024->GetXaxis()->SetLabelFont(42);
   Graph_Graph024->GetXaxis()->SetTitleOffset(1);
   Graph_Graph024->GetXaxis()->SetTitleFont(42);
   Graph_Graph024->GetYaxis()->SetLabelFont(42);
   Graph_Graph024->GetYaxis()->SetTitleFont(42);
   Graph_Graph024->GetZaxis()->SetLabelFont(42);
   Graph_Graph024->GetZaxis()->SetTitleOffset(1);
   Graph_Graph024->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph024);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A6_blue");
   pt->Draw();
   Ge_R2_24->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_25
   TPad *Ge_R2_25 = new TPad("Ge_R2_25", "Ge_R2_25",0.01,0.26,0.07333333,0.49);
   Ge_R2_25->Draw();
   Ge_R2_25->cd();
   Ge_R2_25->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_25->SetFillColor(0);
   Ge_R2_25->SetBorderMode(0);
   Ge_R2_25->SetBorderSize(2);
   Ge_R2_25->SetFrameBorderMode(0);
   Ge_R2_25->SetFrameBorderMode(0);
   
   Double_t Graph0_fx25[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy25[5] = {
   -0.02869415,
   0.3496704,
   -0.3278198,
   -0.1116333,
   0.2369385};
   graph = new TGraph(5,Graph0_fx25,Graph0_fy25);
   graph->SetName("Graph0");
   graph->SetTitle("R2A7_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph025 = new TH1F("Graph_Graph025","R2A7_red",100,0,1536.634);
   Graph_Graph025->SetMinimum(-1.5);
   Graph_Graph025->SetMaximum(1.5);
   Graph_Graph025->SetDirectory(0);
   Graph_Graph025->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph025->SetLineColor(ci);
   Graph_Graph025->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph025->GetXaxis()->SetLabelFont(42);
   Graph_Graph025->GetXaxis()->SetTitleOffset(1);
   Graph_Graph025->GetXaxis()->SetTitleFont(42);
   Graph_Graph025->GetYaxis()->SetLabelFont(42);
   Graph_Graph025->GetYaxis()->SetTitleFont(42);
   Graph_Graph025->GetZaxis()->SetLabelFont(42);
   Graph_Graph025->GetZaxis()->SetTitleOffset(1);
   Graph_Graph025->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph025);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A7_red");
   pt->Draw();
   Ge_R2_25->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_26
   TPad *Ge_R2_26 = new TPad("Ge_R2_26", "Ge_R2_26",0.09333333,0.26,0.1566667,0.49);
   Ge_R2_26->Draw();
   Ge_R2_26->cd();
   Ge_R2_26->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_26->SetFillColor(0);
   Ge_R2_26->SetBorderMode(0);
   Ge_R2_26->SetBorderSize(2);
   Ge_R2_26->SetFrameBorderMode(0);
   Ge_R2_26->SetFrameBorderMode(0);
   
   Double_t Graph0_fx26[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy26[5] = {
   0.04067993,
   0.3424683,
   -0.5302734,
   0.01275635,
   0.2402344};
   graph = new TGraph(5,Graph0_fx26,Graph0_fy26);
   graph->SetName("Graph0");
   graph->SetTitle("R2A7_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph026 = new TH1F("Graph_Graph026","R2A7_green",100,0,1536.634);
   Graph_Graph026->SetMinimum(-1.5);
   Graph_Graph026->SetMaximum(1.5);
   Graph_Graph026->SetDirectory(0);
   Graph_Graph026->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph026->SetLineColor(ci);
   Graph_Graph026->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph026->GetXaxis()->SetLabelFont(42);
   Graph_Graph026->GetXaxis()->SetTitleOffset(1);
   Graph_Graph026->GetXaxis()->SetTitleFont(42);
   Graph_Graph026->GetYaxis()->SetLabelFont(42);
   Graph_Graph026->GetYaxis()->SetTitleFont(42);
   Graph_Graph026->GetZaxis()->SetLabelFont(42);
   Graph_Graph026->GetZaxis()->SetTitleOffset(1);
   Graph_Graph026->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph026);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A7_green");
   pt->Draw();
   Ge_R2_26->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_27
   TPad *Ge_R2_27 = new TPad("Ge_R2_27", "Ge_R2_27",0.1766667,0.26,0.24,0.49);
   Ge_R2_27->Draw();
   Ge_R2_27->cd();
   Ge_R2_27->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_27->SetFillColor(0);
   Ge_R2_27->SetBorderMode(0);
   Ge_R2_27->SetBorderSize(2);
   Ge_R2_27->SetFrameBorderMode(0);
   Ge_R2_27->SetFrameBorderMode(0);
   
   Double_t Graph0_fx27[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy27[5] = {
   0.06887054,
   0.4200134,
   -0.6159668,
   -0.1514282,
   0.3831787};
   graph = new TGraph(5,Graph0_fx27,Graph0_fy27);
   graph->SetName("Graph0");
   graph->SetTitle("R2A7_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph027 = new TH1F("Graph_Graph027","R2A7_black",100,0,1536.634);
   Graph_Graph027->SetMinimum(-1.5);
   Graph_Graph027->SetMaximum(1.5);
   Graph_Graph027->SetDirectory(0);
   Graph_Graph027->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph027->SetLineColor(ci);
   Graph_Graph027->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph027->GetXaxis()->SetLabelFont(42);
   Graph_Graph027->GetXaxis()->SetTitleOffset(1);
   Graph_Graph027->GetXaxis()->SetTitleFont(42);
   Graph_Graph027->GetYaxis()->SetLabelFont(42);
   Graph_Graph027->GetYaxis()->SetTitleFont(42);
   Graph_Graph027->GetZaxis()->SetLabelFont(42);
   Graph_Graph027->GetZaxis()->SetTitleOffset(1);
   Graph_Graph027->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph027);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A7_black");
   pt->Draw();
   Ge_R2_27->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_28
   TPad *Ge_R2_28 = new TPad("Ge_R2_28", "Ge_R2_28",0.26,0.26,0.3233333,0.49);
   Ge_R2_28->Draw();
   Ge_R2_28->cd();
   Ge_R2_28->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_28->SetFillColor(0);
   Ge_R2_28->SetBorderMode(0);
   Ge_R2_28->SetBorderSize(2);
   Ge_R2_28->SetFrameBorderMode(0);
   Ge_R2_28->SetFrameBorderMode(0);
   
   Double_t Graph0_fx28[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy28[5] = {
   0.07536316,
   0.3865356,
   -0.5803223,
   -0.1455688,
   0.3419189};
   graph = new TGraph(5,Graph0_fx28,Graph0_fy28);
   graph->SetName("Graph0");
   graph->SetTitle("R2A7_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph028 = new TH1F("Graph_Graph028","R2A7_blue",100,0,1536.634);
   Graph_Graph028->SetMinimum(-1.5);
   Graph_Graph028->SetMaximum(1.5);
   Graph_Graph028->SetDirectory(0);
   Graph_Graph028->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph028->SetLineColor(ci);
   Graph_Graph028->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph028->GetXaxis()->SetLabelFont(42);
   Graph_Graph028->GetXaxis()->SetTitleOffset(1);
   Graph_Graph028->GetXaxis()->SetTitleFont(42);
   Graph_Graph028->GetYaxis()->SetLabelFont(42);
   Graph_Graph028->GetYaxis()->SetTitleFont(42);
   Graph_Graph028->GetZaxis()->SetLabelFont(42);
   Graph_Graph028->GetZaxis()->SetTitleOffset(1);
   Graph_Graph028->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph028);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A7_blue");
   pt->Draw();
   Ge_R2_28->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_29
   TPad *Ge_R2_29 = new TPad("Ge_R2_29", "Ge_R2_29",0.3433333,0.26,0.4066667,0.49);
   Ge_R2_29->Draw();
   Ge_R2_29->cd();
   Ge_R2_29->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_29->SetFillColor(0);
   Ge_R2_29->SetBorderMode(0);
   Ge_R2_29->SetBorderSize(2);
   Ge_R2_29->SetFrameBorderMode(0);
   Ge_R2_29->SetFrameBorderMode(0);
   
   Double_t Graph0_fx29[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy29[5] = {
   -0.2331085,
   -0.3542175,
   0.7067261,
   0.5568848,
   -0.6256104};
   graph = new TGraph(5,Graph0_fx29,Graph0_fy29);
   graph->SetName("Graph0");
   graph->SetTitle("R2A8_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph029 = new TH1F("Graph_Graph029","R2A8_red",100,0,1536.634);
   Graph_Graph029->SetMinimum(-1.5);
   Graph_Graph029->SetMaximum(1.5);
   Graph_Graph029->SetDirectory(0);
   Graph_Graph029->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph029->SetLineColor(ci);
   Graph_Graph029->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph029->GetXaxis()->SetLabelFont(42);
   Graph_Graph029->GetXaxis()->SetTitleOffset(1);
   Graph_Graph029->GetXaxis()->SetTitleFont(42);
   Graph_Graph029->GetYaxis()->SetLabelFont(42);
   Graph_Graph029->GetYaxis()->SetTitleFont(42);
   Graph_Graph029->GetZaxis()->SetLabelFont(42);
   Graph_Graph029->GetZaxis()->SetTitleOffset(1);
   Graph_Graph029->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph029);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A8_red");
   pt->Draw();
   Ge_R2_29->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_30
   TPad *Ge_R2_30 = new TPad("Ge_R2_30", "Ge_R2_30",0.4266667,0.26,0.49,0.49);
   Ge_R2_30->Draw();
   Ge_R2_30->cd();
   Ge_R2_30->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_30->SetFillColor(0);
   Ge_R2_30->SetBorderMode(0);
   Ge_R2_30->SetBorderSize(2);
   Ge_R2_30->SetFrameBorderMode(0);
   Ge_R2_30->SetFrameBorderMode(0);
   
   Double_t Graph0_fx30[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy30[5] = {
   -0.6006622,
   -0.2042542,
   0.8703613,
   0.9163208,
   -0.9863281};
   graph = new TGraph(5,Graph0_fx30,Graph0_fy30);
   graph->SetName("Graph0");
   graph->SetTitle("R2A8_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph030 = new TH1F("Graph_Graph030","R2A8_green",100,0,1536.634);
   Graph_Graph030->SetMinimum(-1.5);
   Graph_Graph030->SetMaximum(1.5);
   Graph_Graph030->SetDirectory(0);
   Graph_Graph030->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph030->SetLineColor(ci);
   Graph_Graph030->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph030->GetXaxis()->SetLabelFont(42);
   Graph_Graph030->GetXaxis()->SetTitleOffset(1);
   Graph_Graph030->GetXaxis()->SetTitleFont(42);
   Graph_Graph030->GetYaxis()->SetLabelFont(42);
   Graph_Graph030->GetYaxis()->SetTitleFont(42);
   Graph_Graph030->GetZaxis()->SetLabelFont(42);
   Graph_Graph030->GetZaxis()->SetTitleOffset(1);
   Graph_Graph030->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph030);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A8_green");
   pt->Draw();
   Ge_R2_30->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_31
   TPad *Ge_R2_31 = new TPad("Ge_R2_31", "Ge_R2_31",0.51,0.26,0.5733333,0.49);
   Ge_R2_31->Draw();
   Ge_R2_31->cd();
   Ge_R2_31->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_31->SetFillColor(0);
   Ge_R2_31->SetBorderMode(0);
   Ge_R2_31->SetBorderSize(2);
   Ge_R2_31->SetFrameBorderMode(0);
   Ge_R2_31->SetFrameBorderMode(0);
   
   Double_t Graph0_fx31[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy31[5] = {
   -0.32547,
   0.01657104,
   1.115417,
   -0.5009155,
   -0.1685791};
   graph = new TGraph(5,Graph0_fx31,Graph0_fy31);
   graph->SetName("Graph0");
   graph->SetTitle("R2A8_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph031 = new TH1F("Graph_Graph031","R2A8_black",100,0,1536.634);
   Graph_Graph031->SetMinimum(-1.5);
   Graph_Graph031->SetMaximum(1.5);
   Graph_Graph031->SetDirectory(0);
   Graph_Graph031->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph031->SetLineColor(ci);
   Graph_Graph031->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph031->GetXaxis()->SetLabelFont(42);
   Graph_Graph031->GetXaxis()->SetTitleOffset(1);
   Graph_Graph031->GetXaxis()->SetTitleFont(42);
   Graph_Graph031->GetYaxis()->SetLabelFont(42);
   Graph_Graph031->GetYaxis()->SetTitleFont(42);
   Graph_Graph031->GetZaxis()->SetLabelFont(42);
   Graph_Graph031->GetZaxis()->SetTitleOffset(1);
   Graph_Graph031->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph031);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A8_black");
   pt->Draw();
   Ge_R2_31->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_32
   TPad *Ge_R2_32 = new TPad("Ge_R2_32", "Ge_R2_32",0.5933333,0.26,0.6566667,0.49);
   Ge_R2_32->Draw();
   Ge_R2_32->cd();
   Ge_R2_32->Range(0,0,1,1);
   Ge_R2_32->SetFillColor(0);
   Ge_R2_32->SetBorderMode(0);
   Ge_R2_32->SetBorderSize(2);
   Ge_R2_32->SetFrameBorderMode(0);
   Ge_R2_32->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_33
   TPad *Ge_R2_33 = new TPad("Ge_R2_33", "Ge_R2_33",0.6766667,0.26,0.74,0.49);
   Ge_R2_33->Draw();
   Ge_R2_33->cd();
   Ge_R2_33->Range(0,0,1,1);
   Ge_R2_33->SetFillColor(0);
   Ge_R2_33->SetBorderMode(0);
   Ge_R2_33->SetBorderSize(2);
   Ge_R2_33->SetFrameBorderMode(0);
   Ge_R2_33->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_34
   TPad *Ge_R2_34 = new TPad("Ge_R2_34", "Ge_R2_34",0.76,0.26,0.8233333,0.49);
   Ge_R2_34->Draw();
   Ge_R2_34->cd();
   Ge_R2_34->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_34->SetFillColor(0);
   Ge_R2_34->SetBorderMode(0);
   Ge_R2_34->SetBorderSize(2);
   Ge_R2_34->SetFrameBorderMode(0);
   Ge_R2_34->SetFrameBorderMode(0);
   
   Double_t Graph0_fx32[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy32[5] = {
   0.1214066,
   0.2367859,
   -0.4944458,
   -0.1161499,
   0.3261719};
   graph = new TGraph(5,Graph0_fx32,Graph0_fy32);
   graph->SetName("Graph0");
   graph->SetTitle("R2A9_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph032 = new TH1F("Graph_Graph032","R2A9_green",100,0,1536.634);
   Graph_Graph032->SetMinimum(-1.5);
   Graph_Graph032->SetMaximum(1.5);
   Graph_Graph032->SetDirectory(0);
   Graph_Graph032->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph032->SetLineColor(ci);
   Graph_Graph032->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph032->GetXaxis()->SetLabelFont(42);
   Graph_Graph032->GetXaxis()->SetTitleOffset(1);
   Graph_Graph032->GetXaxis()->SetTitleFont(42);
   Graph_Graph032->GetYaxis()->SetLabelFont(42);
   Graph_Graph032->GetYaxis()->SetTitleFont(42);
   Graph_Graph032->GetZaxis()->SetLabelFont(42);
   Graph_Graph032->GetZaxis()->SetTitleOffset(1);
   Graph_Graph032->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph032);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A9_green");
   pt->Draw();
   Ge_R2_34->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_35
   TPad *Ge_R2_35 = new TPad("Ge_R2_35", "Ge_R2_35",0.8433333,0.26,0.9066667,0.49);
   Ge_R2_35->Draw();
   Ge_R2_35->cd();
   Ge_R2_35->Range(0,0,1,1);
   Ge_R2_35->SetFillColor(0);
   Ge_R2_35->SetBorderMode(0);
   Ge_R2_35->SetBorderSize(2);
   Ge_R2_35->SetFrameBorderMode(0);
   Ge_R2_35->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_36
   TPad *Ge_R2_36 = new TPad("Ge_R2_36", "Ge_R2_36",0.9266667,0.26,0.99,0.49);
   Ge_R2_36->Draw();
   Ge_R2_36->cd();
   Ge_R2_36->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_36->SetFillColor(0);
   Ge_R2_36->SetBorderMode(0);
   Ge_R2_36->SetBorderSize(2);
   Ge_R2_36->SetFrameBorderMode(0);
   Ge_R2_36->SetFrameBorderMode(0);
   
   Double_t Graph0_fx33[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy33[5] = {
   0.03894806,
   0.4717407,
   -0.3792114,
   -0.3846436,
   0.4348145};
   graph = new TGraph(5,Graph0_fx33,Graph0_fy33);
   graph->SetName("Graph0");
   graph->SetTitle("R2A9_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph033 = new TH1F("Graph_Graph033","R2A9_blue",100,0,1536.634);
   Graph_Graph033->SetMinimum(-1.5);
   Graph_Graph033->SetMaximum(1.5);
   Graph_Graph033->SetDirectory(0);
   Graph_Graph033->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph033->SetLineColor(ci);
   Graph_Graph033->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph033->GetXaxis()->SetLabelFont(42);
   Graph_Graph033->GetXaxis()->SetTitleOffset(1);
   Graph_Graph033->GetXaxis()->SetTitleFont(42);
   Graph_Graph033->GetYaxis()->SetLabelFont(42);
   Graph_Graph033->GetYaxis()->SetTitleFont(42);
   Graph_Graph033->GetZaxis()->SetLabelFont(42);
   Graph_Graph033->GetZaxis()->SetTitleOffset(1);
   Graph_Graph033->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph033);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A9_blue");
   pt->Draw();
   Ge_R2_36->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_37
   TPad *Ge_R2_37 = new TPad("Ge_R2_37", "Ge_R2_37",0.01,0.01,0.07333333,0.24);
   Ge_R2_37->Draw();
   Ge_R2_37->cd();
   Ge_R2_37->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_37->SetFillColor(0);
   Ge_R2_37->SetBorderMode(0);
   Ge_R2_37->SetBorderSize(2);
   Ge_R2_37->SetFrameBorderMode(0);
   Ge_R2_37->SetFrameBorderMode(0);
   
   Double_t Graph0_fx34[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy34[5] = {
   0.04938507,
   0.2391663,
   0.1326294,
   -0.7472534,
   0.3948975};
   graph = new TGraph(5,Graph0_fx34,Graph0_fy34);
   graph->SetName("Graph0");
   graph->SetTitle("R2A10_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph034 = new TH1F("Graph_Graph034","R2A10_red",100,0,1536.634);
   Graph_Graph034->SetMinimum(-1.5);
   Graph_Graph034->SetMaximum(1.5);
   Graph_Graph034->SetDirectory(0);
   Graph_Graph034->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph034->SetLineColor(ci);
   Graph_Graph034->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph034->GetXaxis()->SetLabelFont(42);
   Graph_Graph034->GetXaxis()->SetTitleOffset(1);
   Graph_Graph034->GetXaxis()->SetTitleFont(42);
   Graph_Graph034->GetYaxis()->SetLabelFont(42);
   Graph_Graph034->GetYaxis()->SetTitleFont(42);
   Graph_Graph034->GetZaxis()->SetLabelFont(42);
   Graph_Graph034->GetZaxis()->SetTitleOffset(1);
   Graph_Graph034->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph034);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A10_red");
   pt->Draw();
   Ge_R2_37->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_38
   TPad *Ge_R2_38 = new TPad("Ge_R2_38", "Ge_R2_38",0.09333333,0.01,0.1566667,0.24);
   Ge_R2_38->Draw();
   Ge_R2_38->cd();
   Ge_R2_38->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_38->SetFillColor(0);
   Ge_R2_38->SetBorderMode(0);
   Ge_R2_38->SetBorderSize(2);
   Ge_R2_38->SetFrameBorderMode(0);
   Ge_R2_38->SetFrameBorderMode(0);
   
   Double_t Graph0_fx35[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy35[5] = {
   0.1790085,
   0.3637085,
   -0.6403198,
   -0.5535889,
   0.5915527};
   graph = new TGraph(5,Graph0_fx35,Graph0_fy35);
   graph->SetName("Graph0");
   graph->SetTitle("R2A10_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph035 = new TH1F("Graph_Graph035","R2A10_green",100,0,1536.634);
   Graph_Graph035->SetMinimum(-1.5);
   Graph_Graph035->SetMaximum(1.5);
   Graph_Graph035->SetDirectory(0);
   Graph_Graph035->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph035->SetLineColor(ci);
   Graph_Graph035->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph035->GetXaxis()->SetLabelFont(42);
   Graph_Graph035->GetXaxis()->SetTitleOffset(1);
   Graph_Graph035->GetXaxis()->SetTitleFont(42);
   Graph_Graph035->GetYaxis()->SetLabelFont(42);
   Graph_Graph035->GetYaxis()->SetTitleFont(42);
   Graph_Graph035->GetZaxis()->SetLabelFont(42);
   Graph_Graph035->GetZaxis()->SetTitleOffset(1);
   Graph_Graph035->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph035);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A10_green");
   pt->Draw();
   Ge_R2_38->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_39
   TPad *Ge_R2_39 = new TPad("Ge_R2_39", "Ge_R2_39",0.1766667,0.01,0.24,0.24);
   Ge_R2_39->Draw();
   Ge_R2_39->cd();
   Ge_R2_39->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_39->SetFillColor(0);
   Ge_R2_39->SetBorderMode(0);
   Ge_R2_39->SetBorderSize(2);
   Ge_R2_39->SetFrameBorderMode(0);
   Ge_R2_39->SetFrameBorderMode(0);
   
   Double_t Graph0_fx36[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy36[5] = {
   -0.0317688,
   0.4594727,
   -0.4602661,
   -0.1518555,
   0.2838135};
   graph = new TGraph(5,Graph0_fx36,Graph0_fy36);
   graph->SetName("Graph0");
   graph->SetTitle("R2A10_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph036 = new TH1F("Graph_Graph036","R2A10_black",100,0,1536.634);
   Graph_Graph036->SetMinimum(-1.5);
   Graph_Graph036->SetMaximum(1.5);
   Graph_Graph036->SetDirectory(0);
   Graph_Graph036->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph036->SetLineColor(ci);
   Graph_Graph036->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph036->GetXaxis()->SetLabelFont(42);
   Graph_Graph036->GetXaxis()->SetTitleOffset(1);
   Graph_Graph036->GetXaxis()->SetTitleFont(42);
   Graph_Graph036->GetYaxis()->SetLabelFont(42);
   Graph_Graph036->GetYaxis()->SetTitleFont(42);
   Graph_Graph036->GetZaxis()->SetLabelFont(42);
   Graph_Graph036->GetZaxis()->SetTitleOffset(1);
   Graph_Graph036->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph036);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A10_black");
   pt->Draw();
   Ge_R2_39->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_40
   TPad *Ge_R2_40 = new TPad("Ge_R2_40", "Ge_R2_40",0.26,0.01,0.3233333,0.24);
   Ge_R2_40->Draw();
   Ge_R2_40->cd();
   Ge_R2_40->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_40->SetFillColor(0);
   Ge_R2_40->SetBorderMode(0);
   Ge_R2_40->SetBorderSize(2);
   Ge_R2_40->SetFrameBorderMode(0);
   Ge_R2_40->SetFrameBorderMode(0);
   
   Double_t Graph0_fx37[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy37[5] = {
   -0.02880859,
   0.1098938,
   0.387207,
   -0.5510864,
   0.1678467};
   graph = new TGraph(5,Graph0_fx37,Graph0_fy37);
   graph->SetName("Graph0");
   graph->SetTitle("R2A10_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph037 = new TH1F("Graph_Graph037","R2A10_blue",100,0,1536.634);
   Graph_Graph037->SetMinimum(-1.5);
   Graph_Graph037->SetMaximum(1.5);
   Graph_Graph037->SetDirectory(0);
   Graph_Graph037->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph037->SetLineColor(ci);
   Graph_Graph037->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph037->GetXaxis()->SetLabelFont(42);
   Graph_Graph037->GetXaxis()->SetTitleOffset(1);
   Graph_Graph037->GetXaxis()->SetTitleFont(42);
   Graph_Graph037->GetYaxis()->SetLabelFont(42);
   Graph_Graph037->GetYaxis()->SetTitleFont(42);
   Graph_Graph037->GetZaxis()->SetLabelFont(42);
   Graph_Graph037->GetZaxis()->SetTitleOffset(1);
   Graph_Graph037->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph037);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A10_blue");
   pt->Draw();
   Ge_R2_40->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_41
   TPad *Ge_R2_41 = new TPad("Ge_R2_41", "Ge_R2_41",0.3433333,0.01,0.4066667,0.24);
   Ge_R2_41->Draw();
   Ge_R2_41->cd();
   Ge_R2_41->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_41->SetFillColor(0);
   Ge_R2_41->SetBorderMode(0);
   Ge_R2_41->SetBorderSize(2);
   Ge_R2_41->SetFrameBorderMode(0);
   Ge_R2_41->SetFrameBorderMode(0);
   
   Double_t Graph0_fx38[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy38[5] = {
   0.0187149,
   0.383728,
   -0.5102539,
   -0.02789307,
   0.2381592};
   graph = new TGraph(5,Graph0_fx38,Graph0_fy38);
   graph->SetName("Graph0");
   graph->SetTitle("R2A11_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph038 = new TH1F("Graph_Graph038","R2A11_red",100,0,1536.634);
   Graph_Graph038->SetMinimum(-1.5);
   Graph_Graph038->SetMaximum(1.5);
   Graph_Graph038->SetDirectory(0);
   Graph_Graph038->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph038->SetLineColor(ci);
   Graph_Graph038->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph038->GetXaxis()->SetLabelFont(42);
   Graph_Graph038->GetXaxis()->SetTitleOffset(1);
   Graph_Graph038->GetXaxis()->SetTitleFont(42);
   Graph_Graph038->GetYaxis()->SetLabelFont(42);
   Graph_Graph038->GetYaxis()->SetTitleFont(42);
   Graph_Graph038->GetZaxis()->SetLabelFont(42);
   Graph_Graph038->GetZaxis()->SetTitleOffset(1);
   Graph_Graph038->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph038);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A11_red");
   pt->Draw();
   Ge_R2_41->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_42
   TPad *Ge_R2_42 = new TPad("Ge_R2_42", "Ge_R2_42",0.4266667,0.01,0.49,0.24);
   Ge_R2_42->Draw();
   Ge_R2_42->cd();
   Ge_R2_42->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_42->SetFillColor(0);
   Ge_R2_42->SetBorderMode(0);
   Ge_R2_42->SetBorderSize(2);
   Ge_R2_42->SetFrameBorderMode(0);
   Ge_R2_42->SetFrameBorderMode(0);
   
   Double_t Graph0_fx39[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy39[5] = {
   0.1163635,
   0.2425842,
   -0.5766602,
   0.05725098,
   0.2399902};
   graph = new TGraph(5,Graph0_fx39,Graph0_fy39);
   graph->SetName("Graph0");
   graph->SetTitle("R2A11_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph039 = new TH1F("Graph_Graph039","R2A11_green",100,0,1536.634);
   Graph_Graph039->SetMinimum(-1.5);
   Graph_Graph039->SetMaximum(1.5);
   Graph_Graph039->SetDirectory(0);
   Graph_Graph039->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph039->SetLineColor(ci);
   Graph_Graph039->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph039->GetXaxis()->SetLabelFont(42);
   Graph_Graph039->GetXaxis()->SetTitleOffset(1);
   Graph_Graph039->GetXaxis()->SetTitleFont(42);
   Graph_Graph039->GetYaxis()->SetLabelFont(42);
   Graph_Graph039->GetYaxis()->SetTitleFont(42);
   Graph_Graph039->GetZaxis()->SetLabelFont(42);
   Graph_Graph039->GetZaxis()->SetTitleOffset(1);
   Graph_Graph039->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph039);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A11_green");
   pt->Draw();
   Ge_R2_42->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_43
   TPad *Ge_R2_43 = new TPad("Ge_R2_43", "Ge_R2_43",0.51,0.01,0.5733333,0.24);
   Ge_R2_43->Draw();
   Ge_R2_43->cd();
   Ge_R2_43->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_43->SetFillColor(0);
   Ge_R2_43->SetBorderMode(0);
   Ge_R2_43->SetBorderSize(2);
   Ge_R2_43->SetFrameBorderMode(0);
   Ge_R2_43->SetFrameBorderMode(0);
   
   Double_t Graph0_fx40[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy40[5] = {
   0.01747894,
   0.4024048,
   -0.5442505,
   -0.00189209,
   0.2391357};
   graph = new TGraph(5,Graph0_fx40,Graph0_fy40);
   graph->SetName("Graph0");
   graph->SetTitle("R2A11_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph040 = new TH1F("Graph_Graph040","R2A11_black",100,0,1536.634);
   Graph_Graph040->SetMinimum(-1.5);
   Graph_Graph040->SetMaximum(1.5);
   Graph_Graph040->SetDirectory(0);
   Graph_Graph040->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph040->SetLineColor(ci);
   Graph_Graph040->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph040->GetXaxis()->SetLabelFont(42);
   Graph_Graph040->GetXaxis()->SetTitleOffset(1);
   Graph_Graph040->GetXaxis()->SetTitleFont(42);
   Graph_Graph040->GetYaxis()->SetLabelFont(42);
   Graph_Graph040->GetYaxis()->SetTitleFont(42);
   Graph_Graph040->GetZaxis()->SetLabelFont(42);
   Graph_Graph040->GetZaxis()->SetTitleOffset(1);
   Graph_Graph040->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph040);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A11_black");
   pt->Draw();
   Ge_R2_43->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_44
   TPad *Ge_R2_44 = new TPad("Ge_R2_44", "Ge_R2_44",0.5933333,0.01,0.6566667,0.24);
   Ge_R2_44->Draw();
   Ge_R2_44->cd();
   Ge_R2_44->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_44->SetFillColor(0);
   Ge_R2_44->SetBorderMode(0);
   Ge_R2_44->SetBorderSize(2);
   Ge_R2_44->SetFrameBorderMode(0);
   Ge_R2_44->SetFrameBorderMode(0);
   
   Double_t Graph0_fx41[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy41[5] = {
   -0.03278351,
   -0.005218506,
   0.5015869,
   -0.5485229,
   0.1032715};
   graph = new TGraph(5,Graph0_fx41,Graph0_fy41);
   graph->SetName("Graph0");
   graph->SetTitle("R2A11_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph041 = new TH1F("Graph_Graph041","R2A11_blue",100,0,1536.634);
   Graph_Graph041->SetMinimum(-1.5);
   Graph_Graph041->SetMaximum(1.5);
   Graph_Graph041->SetDirectory(0);
   Graph_Graph041->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph041->SetLineColor(ci);
   Graph_Graph041->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph041->GetXaxis()->SetLabelFont(42);
   Graph_Graph041->GetXaxis()->SetTitleOffset(1);
   Graph_Graph041->GetXaxis()->SetTitleFont(42);
   Graph_Graph041->GetYaxis()->SetLabelFont(42);
   Graph_Graph041->GetYaxis()->SetTitleFont(42);
   Graph_Graph041->GetZaxis()->SetLabelFont(42);
   Graph_Graph041->GetZaxis()->SetTitleOffset(1);
   Graph_Graph041->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph041);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A11_blue");
   pt->Draw();
   Ge_R2_44->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_45
   TPad *Ge_R2_45 = new TPad("Ge_R2_45", "Ge_R2_45",0.6766667,0.01,0.74,0.24);
   Ge_R2_45->Draw();
   Ge_R2_45->cd();
   Ge_R2_45->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_45->SetFillColor(0);
   Ge_R2_45->SetBorderMode(0);
   Ge_R2_45->SetBorderSize(2);
   Ge_R2_45->SetFrameBorderMode(0);
   Ge_R2_45->SetFrameBorderMode(0);
   
   Double_t Graph0_fx42[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy42[5] = {
   -3.569618,
   -7.657013,
   1.027649,
   -8.201172,
   -11.90735};
   graph = new TGraph(5,Graph0_fx42,Graph0_fy42);
   graph->SetName("Graph0");
   graph->SetTitle("R2A12_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph042 = new TH1F("Graph_Graph042","R2A12_red",100,0,1536.634);
   Graph_Graph042->SetMinimum(-1.5);
   Graph_Graph042->SetMaximum(1.5);
   Graph_Graph042->SetDirectory(0);
   Graph_Graph042->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph042->SetLineColor(ci);
   Graph_Graph042->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph042->GetXaxis()->SetLabelFont(42);
   Graph_Graph042->GetXaxis()->SetTitleOffset(1);
   Graph_Graph042->GetXaxis()->SetTitleFont(42);
   Graph_Graph042->GetYaxis()->SetLabelFont(42);
   Graph_Graph042->GetYaxis()->SetTitleFont(42);
   Graph_Graph042->GetZaxis()->SetLabelFont(42);
   Graph_Graph042->GetZaxis()->SetTitleOffset(1);
   Graph_Graph042->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph042);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A12_red");
   pt->Draw();
   Ge_R2_45->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_46
   TPad *Ge_R2_46 = new TPad("Ge_R2_46", "Ge_R2_46",0.76,0.01,0.8233333,0.24);
   Ge_R2_46->Draw();
   Ge_R2_46->cd();
   Ge_R2_46->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_46->SetFillColor(0);
   Ge_R2_46->SetBorderMode(0);
   Ge_R2_46->SetBorderSize(2);
   Ge_R2_46->SetFrameBorderMode(0);
   Ge_R2_46->SetFrameBorderMode(0);
   
   Double_t Graph0_fx43[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy43[5] = {
   0.1343765,
   0.2722168,
   -0.3087158,
   -0.4963989,
   0.4699707};
   graph = new TGraph(5,Graph0_fx43,Graph0_fy43);
   graph->SetName("Graph0");
   graph->SetTitle("R2A12_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph043 = new TH1F("Graph_Graph043","R2A12_green",100,0,1536.634);
   Graph_Graph043->SetMinimum(-1.5);
   Graph_Graph043->SetMaximum(1.5);
   Graph_Graph043->SetDirectory(0);
   Graph_Graph043->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph043->SetLineColor(ci);
   Graph_Graph043->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph043->GetXaxis()->SetLabelFont(42);
   Graph_Graph043->GetXaxis()->SetTitleOffset(1);
   Graph_Graph043->GetXaxis()->SetTitleFont(42);
   Graph_Graph043->GetYaxis()->SetLabelFont(42);
   Graph_Graph043->GetYaxis()->SetTitleFont(42);
   Graph_Graph043->GetZaxis()->SetLabelFont(42);
   Graph_Graph043->GetZaxis()->SetTitleOffset(1);
   Graph_Graph043->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph043);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A12_green");
   pt->Draw();
   Ge_R2_46->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_47
   TPad *Ge_R2_47 = new TPad("Ge_R2_47", "Ge_R2_47",0.8433333,0.01,0.9066667,0.24);
   Ge_R2_47->Draw();
   Ge_R2_47->cd();
   Ge_R2_47->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_47->SetFillColor(0);
   Ge_R2_47->SetBorderMode(0);
   Ge_R2_47->SetBorderSize(2);
   Ge_R2_47->SetFrameBorderMode(0);
   Ge_R2_47->SetFrameBorderMode(0);
   
   Double_t Graph0_fx44[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy44[5] = {
   -0.2262955,
   0.2797852,
   0.637146,
   -0.9880371,
   0.3074951};
   graph = new TGraph(5,Graph0_fx44,Graph0_fy44);
   graph->SetName("Graph0");
   graph->SetTitle("R2A12_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph044 = new TH1F("Graph_Graph044","R2A12_black",100,0,1536.634);
   Graph_Graph044->SetMinimum(-1.5);
   Graph_Graph044->SetMaximum(1.5);
   Graph_Graph044->SetDirectory(0);
   Graph_Graph044->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph044->SetLineColor(ci);
   Graph_Graph044->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph044->GetXaxis()->SetLabelFont(42);
   Graph_Graph044->GetXaxis()->SetTitleOffset(1);
   Graph_Graph044->GetXaxis()->SetTitleFont(42);
   Graph_Graph044->GetYaxis()->SetLabelFont(42);
   Graph_Graph044->GetYaxis()->SetTitleFont(42);
   Graph_Graph044->GetZaxis()->SetLabelFont(42);
   Graph_Graph044->GetZaxis()->SetTitleOffset(1);
   Graph_Graph044->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph044);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A12_black");
   pt->Draw();
   Ge_R2_47->Modified();
   Ge_R2->cd();
  
// ------------>Primitives in pad: Ge_R2_48
   TPad *Ge_R2_48 = new TPad("Ge_R2_48", "Ge_R2_48",0.9266667,0.01,0.99,0.24);
   Ge_R2_48->Draw();
   Ge_R2_48->cd();
   Ge_R2_48->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R2_48->SetFillColor(0);
   Ge_R2_48->SetBorderMode(0);
   Ge_R2_48->SetBorderSize(2);
   Ge_R2_48->SetFrameBorderMode(0);
   Ge_R2_48->SetFrameBorderMode(0);
   
   Double_t Graph0_fx45[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy45[5] = {
   -0.1833115,
   0.04980469,
   0.5755005,
   -0.4216309,
   -0.00402832};
   graph = new TGraph(5,Graph0_fx45,Graph0_fy45);
   graph->SetName("Graph0");
   graph->SetTitle("R2A12_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph045 = new TH1F("Graph_Graph045","R2A12_blue",100,0,1536.634);
   Graph_Graph045->SetMinimum(-1.5);
   Graph_Graph045->SetMaximum(1.5);
   Graph_Graph045->SetDirectory(0);
   Graph_Graph045->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph045->SetLineColor(ci);
   Graph_Graph045->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph045->GetXaxis()->SetLabelFont(42);
   Graph_Graph045->GetXaxis()->SetTitleOffset(1);
   Graph_Graph045->GetXaxis()->SetTitleFont(42);
   Graph_Graph045->GetYaxis()->SetLabelFont(42);
   Graph_Graph045->GetYaxis()->SetTitleFont(42);
   Graph_Graph045->GetZaxis()->SetLabelFont(42);
   Graph_Graph045->GetZaxis()->SetTitleOffset(1);
   Graph_Graph045->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph045);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R2A12_blue");
   pt->Draw();
   Ge_R2_48->Modified();
   Ge_R2->cd();
   Ge_R2->Modified();
   Ge_R2->cd();
   Ge_R2->SetSelected(Ge_R2);
}
