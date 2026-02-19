#ifdef __CLING__
#pragma cling optimize(0)
#endif
void Ge_R3()
{
//=========Macro generated from canvas: Ge_R3/Ge_R3
//=========  (Fri Sep 23 11:33:01 2022) by ROOT version 6.26/04
   TCanvas *Ge_R3 = new TCanvas("Ge_R3", "Ge_R3",0,64,2560,1376);
   Ge_R3->SetHighLightColor(2);
   Ge_R3->Range(0,0,1,1);
   Ge_R3->SetFillColor(0);
   Ge_R3->SetBorderMode(0);
   Ge_R3->SetBorderSize(2);
   Ge_R3->SetFrameBorderMode(0);
  
// ------------>Primitives in pad: Ge_R3_1
   TPad *Ge_R3_1 = new TPad("Ge_R3_1", "Ge_R3_1",0.01,0.76,0.07333333,0.99);
   Ge_R3_1->Draw();
   Ge_R3_1->cd();
   Ge_R3_1->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_1->SetFillColor(0);
   Ge_R3_1->SetBorderMode(0);
   Ge_R3_1->SetBorderSize(2);
   Ge_R3_1->SetFrameBorderMode(0);
   Ge_R3_1->SetFrameBorderMode(0);
   
   Double_t Graph0_fx46[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy46[5] = {
   -0.2677383,
   -0.132019,
   0.553772,
   0.2165527,
   -0.4329834};
   TGraph *graph = new TGraph(5,Graph0_fx46,Graph0_fy46);
   graph->SetName("Graph0");
   graph->SetTitle("R3A1_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph046 = new TH1F("Graph_Graph046","R3A1_red",100,0,1536.634);
   Graph_Graph046->SetMinimum(-1.5);
   Graph_Graph046->SetMaximum(1.5);
   Graph_Graph046->SetDirectory(0);
   Graph_Graph046->SetStats(0);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#000099");
   Graph_Graph046->SetLineColor(ci);
   Graph_Graph046->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph046->GetXaxis()->SetLabelFont(42);
   Graph_Graph046->GetXaxis()->SetTitleOffset(1);
   Graph_Graph046->GetXaxis()->SetTitleFont(42);
   Graph_Graph046->GetYaxis()->SetLabelFont(42);
   Graph_Graph046->GetYaxis()->SetTitleFont(42);
   Graph_Graph046->GetZaxis()->SetLabelFont(42);
   Graph_Graph046->GetZaxis()->SetTitleOffset(1);
   Graph_Graph046->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph046);
   
   graph->Draw("alp");
   
   TPaveText *pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   TText *pt_LaTex = pt->AddText("R3A1_red");
   pt->Draw();
   Ge_R3_1->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_2
   TPad *Ge_R3_2 = new TPad("Ge_R3_2", "Ge_R3_2",0.09333333,0.76,0.1566667,0.99);
   Ge_R3_2->Draw();
   Ge_R3_2->cd();
   Ge_R3_2->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_2->SetFillColor(0);
   Ge_R3_2->SetBorderMode(0);
   Ge_R3_2->SetBorderSize(2);
   Ge_R3_2->SetFrameBorderMode(0);
   Ge_R3_2->SetFrameBorderMode(0);
   
   Double_t Graph0_fx47[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy47[5] = {
   -0.3296509,
   0.0027771,
   0.2617188,
   0.7429199,
   -0.5683594};
   graph = new TGraph(5,Graph0_fx47,Graph0_fy47);
   graph->SetName("Graph0");
   graph->SetTitle("R3A1_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph047 = new TH1F("Graph_Graph047","R3A1_green",100,0,1536.634);
   Graph_Graph047->SetMinimum(-1.5);
   Graph_Graph047->SetMaximum(1.5);
   Graph_Graph047->SetDirectory(0);
   Graph_Graph047->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph047->SetLineColor(ci);
   Graph_Graph047->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph047->GetXaxis()->SetLabelFont(42);
   Graph_Graph047->GetXaxis()->SetTitleOffset(1);
   Graph_Graph047->GetXaxis()->SetTitleFont(42);
   Graph_Graph047->GetYaxis()->SetLabelFont(42);
   Graph_Graph047->GetYaxis()->SetTitleFont(42);
   Graph_Graph047->GetZaxis()->SetLabelFont(42);
   Graph_Graph047->GetZaxis()->SetTitleOffset(1);
   Graph_Graph047->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph047);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A1_green");
   pt->Draw();
   Ge_R3_2->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_3
   TPad *Ge_R3_3 = new TPad("Ge_R3_3", "Ge_R3_3",0.1766667,0.76,0.24,0.99);
   Ge_R3_3->Draw();
   Ge_R3_3->cd();
   Ge_R3_3->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_3->SetFillColor(0);
   Ge_R3_3->SetBorderMode(0);
   Ge_R3_3->SetBorderSize(2);
   Ge_R3_3->SetFrameBorderMode(0);
   Ge_R3_3->SetFrameBorderMode(0);
   
   Double_t Graph0_fx48[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy48[5] = {
   -0.1856613,
   0.00289917,
   0.482666,
   -0.1543579,
   -0.1239014};
   graph = new TGraph(5,Graph0_fx48,Graph0_fy48);
   graph->SetName("Graph0");
   graph->SetTitle("R3A1_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph048 = new TH1F("Graph_Graph048","R3A1_black",100,0,1536.634);
   Graph_Graph048->SetMinimum(-1.5);
   Graph_Graph048->SetMaximum(1.5);
   Graph_Graph048->SetDirectory(0);
   Graph_Graph048->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph048->SetLineColor(ci);
   Graph_Graph048->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph048->GetXaxis()->SetLabelFont(42);
   Graph_Graph048->GetXaxis()->SetTitleOffset(1);
   Graph_Graph048->GetXaxis()->SetTitleFont(42);
   Graph_Graph048->GetYaxis()->SetLabelFont(42);
   Graph_Graph048->GetYaxis()->SetTitleFont(42);
   Graph_Graph048->GetZaxis()->SetLabelFont(42);
   Graph_Graph048->GetZaxis()->SetTitleOffset(1);
   Graph_Graph048->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph048);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A1_black");
   pt->Draw();
   Ge_R3_3->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_4
   TPad *Ge_R3_4 = new TPad("Ge_R3_4", "Ge_R3_4",0.26,0.76,0.3233333,0.99);
   Ge_R3_4->Draw();
   Ge_R3_4->cd();
   Ge_R3_4->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_4->SetFillColor(0);
   Ge_R3_4->SetBorderMode(0);
   Ge_R3_4->SetBorderSize(2);
   Ge_R3_4->SetFrameBorderMode(0);
   Ge_R3_4->SetFrameBorderMode(0);
   
   Double_t Graph0_fx49[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy49[5] = {
   0.1416321,
   -0.00201416,
   -0.2202148,
   0.1237183,
   0.04296875};
   graph = new TGraph(5,Graph0_fx49,Graph0_fy49);
   graph->SetName("Graph0");
   graph->SetTitle("R3A1_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph049 = new TH1F("Graph_Graph049","R3A1_blue",100,0,1536.634);
   Graph_Graph049->SetMinimum(-1.5);
   Graph_Graph049->SetMaximum(1.5);
   Graph_Graph049->SetDirectory(0);
   Graph_Graph049->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph049->SetLineColor(ci);
   Graph_Graph049->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph049->GetXaxis()->SetLabelFont(42);
   Graph_Graph049->GetXaxis()->SetTitleOffset(1);
   Graph_Graph049->GetXaxis()->SetTitleFont(42);
   Graph_Graph049->GetYaxis()->SetLabelFont(42);
   Graph_Graph049->GetYaxis()->SetTitleFont(42);
   Graph_Graph049->GetZaxis()->SetLabelFont(42);
   Graph_Graph049->GetZaxis()->SetTitleOffset(1);
   Graph_Graph049->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph049);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A1_blue");
   pt->Draw();
   Ge_R3_4->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_5
   TPad *Ge_R3_5 = new TPad("Ge_R3_5", "Ge_R3_5",0.3433333,0.76,0.4066667,0.99);
   Ge_R3_5->Draw();
   Ge_R3_5->cd();
   Ge_R3_5->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_5->SetFillColor(0);
   Ge_R3_5->SetBorderMode(0);
   Ge_R3_5->SetBorderSize(2);
   Ge_R3_5->SetFrameBorderMode(0);
   Ge_R3_5->SetFrameBorderMode(0);
   
   Double_t Graph0_fx50[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy50[5] = {
   -0.3382187,
   -0.01132202,
   0.8409424,
   -0.08312988,
   -0.3323975};
   graph = new TGraph(5,Graph0_fx50,Graph0_fy50);
   graph->SetName("Graph0");
   graph->SetTitle("R3A2_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph050 = new TH1F("Graph_Graph050","R3A2_red",100,0,1536.634);
   Graph_Graph050->SetMinimum(-1.5);
   Graph_Graph050->SetMaximum(1.5);
   Graph_Graph050->SetDirectory(0);
   Graph_Graph050->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph050->SetLineColor(ci);
   Graph_Graph050->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph050->GetXaxis()->SetLabelFont(42);
   Graph_Graph050->GetXaxis()->SetTitleOffset(1);
   Graph_Graph050->GetXaxis()->SetTitleFont(42);
   Graph_Graph050->GetYaxis()->SetLabelFont(42);
   Graph_Graph050->GetYaxis()->SetTitleFont(42);
   Graph_Graph050->GetZaxis()->SetLabelFont(42);
   Graph_Graph050->GetZaxis()->SetTitleOffset(1);
   Graph_Graph050->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph050);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A2_red");
   pt->Draw();
   Ge_R3_5->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_6
   TPad *Ge_R3_6 = new TPad("Ge_R3_6", "Ge_R3_6",0.4266667,0.76,0.49,0.99);
   Ge_R3_6->Draw();
   Ge_R3_6->cd();
   Ge_R3_6->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_6->SetFillColor(0);
   Ge_R3_6->SetBorderMode(0);
   Ge_R3_6->SetBorderSize(2);
   Ge_R3_6->SetFrameBorderMode(0);
   Ge_R3_6->SetFrameBorderMode(0);
   
   Double_t Graph0_fx51[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy51[5] = {
   -0.4268646,
   0.3292236,
   0.6703491,
   -0.3599243,
   -0.1323242};
   graph = new TGraph(5,Graph0_fx51,Graph0_fy51);
   graph->SetName("Graph0");
   graph->SetTitle("R3A2_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph051 = new TH1F("Graph_Graph051","R3A2_green",100,0,1536.634);
   Graph_Graph051->SetMinimum(-1.5);
   Graph_Graph051->SetMaximum(1.5);
   Graph_Graph051->SetDirectory(0);
   Graph_Graph051->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph051->SetLineColor(ci);
   Graph_Graph051->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph051->GetXaxis()->SetLabelFont(42);
   Graph_Graph051->GetXaxis()->SetTitleOffset(1);
   Graph_Graph051->GetXaxis()->SetTitleFont(42);
   Graph_Graph051->GetYaxis()->SetLabelFont(42);
   Graph_Graph051->GetYaxis()->SetTitleFont(42);
   Graph_Graph051->GetZaxis()->SetLabelFont(42);
   Graph_Graph051->GetZaxis()->SetTitleOffset(1);
   Graph_Graph051->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph051);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A2_green");
   pt->Draw();
   Ge_R3_6->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_7
   TPad *Ge_R3_7 = new TPad("Ge_R3_7", "Ge_R3_7",0.51,0.76,0.5733333,0.99);
   Ge_R3_7->Draw();
   Ge_R3_7->cd();
   Ge_R3_7->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_7->SetFillColor(0);
   Ge_R3_7->SetBorderMode(0);
   Ge_R3_7->SetBorderSize(2);
   Ge_R3_7->SetFrameBorderMode(0);
   Ge_R3_7->SetFrameBorderMode(0);
   
   Double_t Graph0_fx52[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy52[5] = {
   0.01394653,
   0.3092651,
   -0.3548584,
   -0.05828857,
   0.2155762};
   graph = new TGraph(5,Graph0_fx52,Graph0_fy52);
   graph->SetName("Graph0");
   graph->SetTitle("R3A2_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph052 = new TH1F("Graph_Graph052","R3A2_black",100,0,1536.634);
   Graph_Graph052->SetMinimum(-1.5);
   Graph_Graph052->SetMaximum(1.5);
   Graph_Graph052->SetDirectory(0);
   Graph_Graph052->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph052->SetLineColor(ci);
   Graph_Graph052->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph052->GetXaxis()->SetLabelFont(42);
   Graph_Graph052->GetXaxis()->SetTitleOffset(1);
   Graph_Graph052->GetXaxis()->SetTitleFont(42);
   Graph_Graph052->GetYaxis()->SetLabelFont(42);
   Graph_Graph052->GetYaxis()->SetTitleFont(42);
   Graph_Graph052->GetZaxis()->SetLabelFont(42);
   Graph_Graph052->GetZaxis()->SetTitleOffset(1);
   Graph_Graph052->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph052);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A2_black");
   pt->Draw();
   Ge_R3_7->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_8
   TPad *Ge_R3_8 = new TPad("Ge_R3_8", "Ge_R3_8",0.5933333,0.76,0.6566667,0.99);
   Ge_R3_8->Draw();
   Ge_R3_8->cd();
   Ge_R3_8->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_8->SetFillColor(0);
   Ge_R3_8->SetBorderMode(0);
   Ge_R3_8->SetBorderSize(2);
   Ge_R3_8->SetFrameBorderMode(0);
   Ge_R3_8->SetFrameBorderMode(0);
   
   Double_t Graph0_fx53[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy53[5] = {
   0.01681519,
   0.1422119,
   -0.08532715,
   -0.0078125,
   0.07507324};
   graph = new TGraph(5,Graph0_fx53,Graph0_fy53);
   graph->SetName("Graph0");
   graph->SetTitle("R3A2_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph053 = new TH1F("Graph_Graph053","R3A2_blue",100,0,1536.634);
   Graph_Graph053->SetMinimum(-1.5);
   Graph_Graph053->SetMaximum(1.5);
   Graph_Graph053->SetDirectory(0);
   Graph_Graph053->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph053->SetLineColor(ci);
   Graph_Graph053->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph053->GetXaxis()->SetLabelFont(42);
   Graph_Graph053->GetXaxis()->SetTitleOffset(1);
   Graph_Graph053->GetXaxis()->SetTitleFont(42);
   Graph_Graph053->GetYaxis()->SetLabelFont(42);
   Graph_Graph053->GetYaxis()->SetTitleFont(42);
   Graph_Graph053->GetZaxis()->SetLabelFont(42);
   Graph_Graph053->GetZaxis()->SetTitleOffset(1);
   Graph_Graph053->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph053);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A2_blue");
   pt->Draw();
   Ge_R3_8->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_9
   TPad *Ge_R3_9 = new TPad("Ge_R3_9", "Ge_R3_9",0.6766667,0.76,0.74,0.99);
   Ge_R3_9->Draw();
   Ge_R3_9->cd();
   Ge_R3_9->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_9->SetFillColor(0);
   Ge_R3_9->SetBorderMode(0);
   Ge_R3_9->SetBorderSize(2);
   Ge_R3_9->SetFrameBorderMode(0);
   Ge_R3_9->SetFrameBorderMode(0);
   
   Double_t Graph0_fx54[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy54[5] = {
   0.06106567,
   0.3677368,
   -0.3748169,
   -0.3081665,
   0.352417};
   graph = new TGraph(5,Graph0_fx54,Graph0_fy54);
   graph->SetName("Graph0");
   graph->SetTitle("R3A3_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph054 = new TH1F("Graph_Graph054","R3A3_red",100,0,1536.634);
   Graph_Graph054->SetMinimum(-1.5);
   Graph_Graph054->SetMaximum(1.5);
   Graph_Graph054->SetDirectory(0);
   Graph_Graph054->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph054->SetLineColor(ci);
   Graph_Graph054->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph054->GetXaxis()->SetLabelFont(42);
   Graph_Graph054->GetXaxis()->SetTitleOffset(1);
   Graph_Graph054->GetXaxis()->SetTitleFont(42);
   Graph_Graph054->GetYaxis()->SetLabelFont(42);
   Graph_Graph054->GetYaxis()->SetTitleFont(42);
   Graph_Graph054->GetZaxis()->SetLabelFont(42);
   Graph_Graph054->GetZaxis()->SetTitleOffset(1);
   Graph_Graph054->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph054);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A3_red");
   pt->Draw();
   Ge_R3_9->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_10
   TPad *Ge_R3_10 = new TPad("Ge_R3_10", "Ge_R3_10",0.76,0.76,0.8233333,0.99);
   Ge_R3_10->Draw();
   Ge_R3_10->cd();
   Ge_R3_10->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_10->SetFillColor(0);
   Ge_R3_10->SetBorderMode(0);
   Ge_R3_10->SetBorderSize(2);
   Ge_R3_10->SetFrameBorderMode(0);
   Ge_R3_10->SetFrameBorderMode(0);
   
   Double_t Graph0_fx55[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy55[5] = {
   -0.2419739,
   0.09249878,
   0.8734131,
   -0.6484985,
   -0.01538086};
   graph = new TGraph(5,Graph0_fx55,Graph0_fy55);
   graph->SetName("Graph0");
   graph->SetTitle("R3A3_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph055 = new TH1F("Graph_Graph055","R3A3_green",100,0,1536.634);
   Graph_Graph055->SetMinimum(-1.5);
   Graph_Graph055->SetMaximum(1.5);
   Graph_Graph055->SetDirectory(0);
   Graph_Graph055->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph055->SetLineColor(ci);
   Graph_Graph055->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph055->GetXaxis()->SetLabelFont(42);
   Graph_Graph055->GetXaxis()->SetTitleOffset(1);
   Graph_Graph055->GetXaxis()->SetTitleFont(42);
   Graph_Graph055->GetYaxis()->SetLabelFont(42);
   Graph_Graph055->GetYaxis()->SetTitleFont(42);
   Graph_Graph055->GetZaxis()->SetLabelFont(42);
   Graph_Graph055->GetZaxis()->SetTitleOffset(1);
   Graph_Graph055->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph055);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A3_green");
   pt->Draw();
   Ge_R3_10->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_11
   TPad *Ge_R3_11 = new TPad("Ge_R3_11", "Ge_R3_11",0.8433333,0.76,0.9066667,0.99);
   Ge_R3_11->Draw();
   Ge_R3_11->cd();
   Ge_R3_11->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_11->SetFillColor(0);
   Ge_R3_11->SetBorderMode(0);
   Ge_R3_11->SetBorderSize(2);
   Ge_R3_11->SetFrameBorderMode(0);
   Ge_R3_11->SetFrameBorderMode(0);
   
   Double_t Graph0_fx56[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy56[5] = {
   -1.616592,
   -1.152954,
   -1.099609,
   -0.9538574,
   0.3232422};
   graph = new TGraph(5,Graph0_fx56,Graph0_fy56);
   graph->SetName("Graph0");
   graph->SetTitle("R3A3_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph056 = new TH1F("Graph_Graph056","R3A3_black",100,0,1536.634);
   Graph_Graph056->SetMinimum(-1.5);
   Graph_Graph056->SetMaximum(1.5);
   Graph_Graph056->SetDirectory(0);
   Graph_Graph056->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph056->SetLineColor(ci);
   Graph_Graph056->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph056->GetXaxis()->SetLabelFont(42);
   Graph_Graph056->GetXaxis()->SetTitleOffset(1);
   Graph_Graph056->GetXaxis()->SetTitleFont(42);
   Graph_Graph056->GetYaxis()->SetLabelFont(42);
   Graph_Graph056->GetYaxis()->SetTitleFont(42);
   Graph_Graph056->GetZaxis()->SetLabelFont(42);
   Graph_Graph056->GetZaxis()->SetTitleOffset(1);
   Graph_Graph056->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph056);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A3_black");
   pt->Draw();
   Ge_R3_11->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_12
   TPad *Ge_R3_12 = new TPad("Ge_R3_12", "Ge_R3_12",0.9266667,0.76,0.99,0.99);
   Ge_R3_12->Draw();
   Ge_R3_12->cd();
   Ge_R3_12->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_12->SetFillColor(0);
   Ge_R3_12->SetBorderMode(0);
   Ge_R3_12->SetBorderSize(2);
   Ge_R3_12->SetFrameBorderMode(0);
   Ge_R3_12->SetFrameBorderMode(0);
   
   Double_t Graph0_fx57[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy57[5] = {
   0.180191,
   0.3240051,
   -0.7731934,
   -0.3352661,
   0.5493164};
   graph = new TGraph(5,Graph0_fx57,Graph0_fy57);
   graph->SetName("Graph0");
   graph->SetTitle("R3A3_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph057 = new TH1F("Graph_Graph057","R3A3_blue",100,0,1536.634);
   Graph_Graph057->SetMinimum(-1.5);
   Graph_Graph057->SetMaximum(1.5);
   Graph_Graph057->SetDirectory(0);
   Graph_Graph057->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph057->SetLineColor(ci);
   Graph_Graph057->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph057->GetXaxis()->SetLabelFont(42);
   Graph_Graph057->GetXaxis()->SetTitleOffset(1);
   Graph_Graph057->GetXaxis()->SetTitleFont(42);
   Graph_Graph057->GetYaxis()->SetLabelFont(42);
   Graph_Graph057->GetYaxis()->SetTitleFont(42);
   Graph_Graph057->GetZaxis()->SetLabelFont(42);
   Graph_Graph057->GetZaxis()->SetTitleOffset(1);
   Graph_Graph057->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph057);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A3_blue");
   pt->Draw();
   Ge_R3_12->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_13
   TPad *Ge_R3_13 = new TPad("Ge_R3_13", "Ge_R3_13",0.01,0.51,0.07333333,0.74);
   Ge_R3_13->Draw();
   Ge_R3_13->cd();
   Ge_R3_13->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_13->SetFillColor(0);
   Ge_R3_13->SetBorderMode(0);
   Ge_R3_13->SetBorderSize(2);
   Ge_R3_13->SetFrameBorderMode(0);
   Ge_R3_13->SetFrameBorderMode(0);
   
   Double_t Graph0_fx58[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy58[5] = {
   -0.2694855,
   -0.2888794,
   0.4701538,
   -0.1017456,
   -0.4248047};
   graph = new TGraph(5,Graph0_fx58,Graph0_fy58);
   graph->SetName("Graph0");
   graph->SetTitle("R3A4_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph058 = new TH1F("Graph_Graph058","R3A4_red",100,0,1536.634);
   Graph_Graph058->SetMinimum(-1.5);
   Graph_Graph058->SetMaximum(1.5);
   Graph_Graph058->SetDirectory(0);
   Graph_Graph058->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph058->SetLineColor(ci);
   Graph_Graph058->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph058->GetXaxis()->SetLabelFont(42);
   Graph_Graph058->GetXaxis()->SetTitleOffset(1);
   Graph_Graph058->GetXaxis()->SetTitleFont(42);
   Graph_Graph058->GetYaxis()->SetLabelFont(42);
   Graph_Graph058->GetYaxis()->SetTitleFont(42);
   Graph_Graph058->GetZaxis()->SetLabelFont(42);
   Graph_Graph058->GetZaxis()->SetTitleOffset(1);
   Graph_Graph058->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph058);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A4_red");
   pt->Draw();
   Ge_R3_13->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_14
   TPad *Ge_R3_14 = new TPad("Ge_R3_14", "Ge_R3_14",0.09333333,0.51,0.1566667,0.74);
   Ge_R3_14->Draw();
   Ge_R3_14->cd();
   Ge_R3_14->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_14->SetFillColor(0);
   Ge_R3_14->SetBorderMode(0);
   Ge_R3_14->SetBorderSize(2);
   Ge_R3_14->SetFrameBorderMode(0);
   Ge_R3_14->SetFrameBorderMode(0);
   
   Double_t Graph0_fx59[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy59[5] = {
   -0.2814102,
   0.06188965,
   0.7738037,
   -0.3511963,
   -0.1796875};
   graph = new TGraph(5,Graph0_fx59,Graph0_fy59);
   graph->SetName("Graph0");
   graph->SetTitle("R3A4_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph059 = new TH1F("Graph_Graph059","R3A4_green",100,0,1536.634);
   Graph_Graph059->SetMinimum(-1.5);
   Graph_Graph059->SetMaximum(1.5);
   Graph_Graph059->SetDirectory(0);
   Graph_Graph059->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph059->SetLineColor(ci);
   Graph_Graph059->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph059->GetXaxis()->SetLabelFont(42);
   Graph_Graph059->GetXaxis()->SetTitleOffset(1);
   Graph_Graph059->GetXaxis()->SetTitleFont(42);
   Graph_Graph059->GetYaxis()->SetLabelFont(42);
   Graph_Graph059->GetYaxis()->SetTitleFont(42);
   Graph_Graph059->GetZaxis()->SetLabelFont(42);
   Graph_Graph059->GetZaxis()->SetTitleOffset(1);
   Graph_Graph059->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph059);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A4_green");
   pt->Draw();
   Ge_R3_14->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_15
   TPad *Ge_R3_15 = new TPad("Ge_R3_15", "Ge_R3_15",0.1766667,0.51,0.24,0.74);
   Ge_R3_15->Draw();
   Ge_R3_15->cd();
   Ge_R3_15->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_15->SetFillColor(0);
   Ge_R3_15->SetBorderMode(0);
   Ge_R3_15->SetBorderSize(2);
   Ge_R3_15->SetFrameBorderMode(0);
   Ge_R3_15->SetFrameBorderMode(0);
   
   Double_t Graph0_fx60[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy60[5] = {
   -0.2728043,
   0.2304688,
   0.6102295,
   -0.5876465,
   0.08618164};
   graph = new TGraph(5,Graph0_fx60,Graph0_fy60);
   graph->SetName("Graph0");
   graph->SetTitle("R3A4_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph060 = new TH1F("Graph_Graph060","R3A4_black",100,0,1536.634);
   Graph_Graph060->SetMinimum(-1.5);
   Graph_Graph060->SetMaximum(1.5);
   Graph_Graph060->SetDirectory(0);
   Graph_Graph060->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph060->SetLineColor(ci);
   Graph_Graph060->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph060->GetXaxis()->SetLabelFont(42);
   Graph_Graph060->GetXaxis()->SetTitleOffset(1);
   Graph_Graph060->GetXaxis()->SetTitleFont(42);
   Graph_Graph060->GetYaxis()->SetLabelFont(42);
   Graph_Graph060->GetYaxis()->SetTitleFont(42);
   Graph_Graph060->GetZaxis()->SetLabelFont(42);
   Graph_Graph060->GetZaxis()->SetTitleOffset(1);
   Graph_Graph060->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph060);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A4_black");
   pt->Draw();
   Ge_R3_15->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_16
   TPad *Ge_R3_16 = new TPad("Ge_R3_16", "Ge_R3_16",0.26,0.51,0.3233333,0.74);
   Ge_R3_16->Draw();
   Ge_R3_16->cd();
   Ge_R3_16->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_16->SetFillColor(0);
   Ge_R3_16->SetBorderMode(0);
   Ge_R3_16->SetBorderSize(2);
   Ge_R3_16->SetFrameBorderMode(0);
   Ge_R3_16->SetFrameBorderMode(0);
   
   Double_t Graph0_fx61[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy61[5] = {
   0.03177643,
   0.3554688,
   -0.5498047,
   -0.1347046,
   0.2932129};
   graph = new TGraph(5,Graph0_fx61,Graph0_fy61);
   graph->SetName("Graph0");
   graph->SetTitle("R3A4_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph061 = new TH1F("Graph_Graph061","R3A4_blue",100,0,1536.634);
   Graph_Graph061->SetMinimum(-1.5);
   Graph_Graph061->SetMaximum(1.5);
   Graph_Graph061->SetDirectory(0);
   Graph_Graph061->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph061->SetLineColor(ci);
   Graph_Graph061->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph061->GetXaxis()->SetLabelFont(42);
   Graph_Graph061->GetXaxis()->SetTitleOffset(1);
   Graph_Graph061->GetXaxis()->SetTitleFont(42);
   Graph_Graph061->GetYaxis()->SetLabelFont(42);
   Graph_Graph061->GetYaxis()->SetTitleFont(42);
   Graph_Graph061->GetZaxis()->SetLabelFont(42);
   Graph_Graph061->GetZaxis()->SetTitleOffset(1);
   Graph_Graph061->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph061);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A4_blue");
   pt->Draw();
   Ge_R3_16->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_17
   TPad *Ge_R3_17 = new TPad("Ge_R3_17", "Ge_R3_17",0.3433333,0.51,0.4066667,0.74);
   Ge_R3_17->Draw();
   Ge_R3_17->cd();
   Ge_R3_17->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_17->SetFillColor(0);
   Ge_R3_17->SetBorderMode(0);
   Ge_R3_17->SetBorderSize(2);
   Ge_R3_17->SetFrameBorderMode(0);
   Ge_R3_17->SetFrameBorderMode(0);
   
   Double_t Graph0_fx62[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy62[5] = {
   -0.1797562,
   -0.2946167,
   0.6508789,
   0.2376709,
   -0.4014893};
   graph = new TGraph(5,Graph0_fx62,Graph0_fy62);
   graph->SetName("Graph0");
   graph->SetTitle("R3A5_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph062 = new TH1F("Graph_Graph062","R3A5_red",100,0,1536.634);
   Graph_Graph062->SetMinimum(-1.5);
   Graph_Graph062->SetMaximum(1.5);
   Graph_Graph062->SetDirectory(0);
   Graph_Graph062->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph062->SetLineColor(ci);
   Graph_Graph062->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph062->GetXaxis()->SetLabelFont(42);
   Graph_Graph062->GetXaxis()->SetTitleOffset(1);
   Graph_Graph062->GetXaxis()->SetTitleFont(42);
   Graph_Graph062->GetYaxis()->SetLabelFont(42);
   Graph_Graph062->GetYaxis()->SetTitleFont(42);
   Graph_Graph062->GetZaxis()->SetLabelFont(42);
   Graph_Graph062->GetZaxis()->SetTitleOffset(1);
   Graph_Graph062->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph062);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A5_red");
   pt->Draw();
   Ge_R3_17->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_18
   TPad *Ge_R3_18 = new TPad("Ge_R3_18", "Ge_R3_18",0.4266667,0.51,0.49,0.74);
   Ge_R3_18->Draw();
   Ge_R3_18->cd();
   Ge_R3_18->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_18->SetFillColor(0);
   Ge_R3_18->SetBorderMode(0);
   Ge_R3_18->SetBorderSize(2);
   Ge_R3_18->SetFrameBorderMode(0);
   Ge_R3_18->SetFrameBorderMode(0);
   
   Double_t Graph0_fx63[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy63[5] = {
   -0.2039185,
   -0.08157349,
   0.824585,
   -0.4018555,
   -0.1334229};
   graph = new TGraph(5,Graph0_fx63,Graph0_fy63);
   graph->SetName("Graph0");
   graph->SetTitle("R3A5_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph063 = new TH1F("Graph_Graph063","R3A5_green",100,0,1536.634);
   Graph_Graph063->SetMinimum(-1.5);
   Graph_Graph063->SetMaximum(1.5);
   Graph_Graph063->SetDirectory(0);
   Graph_Graph063->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph063->SetLineColor(ci);
   Graph_Graph063->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph063->GetXaxis()->SetLabelFont(42);
   Graph_Graph063->GetXaxis()->SetTitleOffset(1);
   Graph_Graph063->GetXaxis()->SetTitleFont(42);
   Graph_Graph063->GetYaxis()->SetLabelFont(42);
   Graph_Graph063->GetYaxis()->SetTitleFont(42);
   Graph_Graph063->GetZaxis()->SetLabelFont(42);
   Graph_Graph063->GetZaxis()->SetTitleOffset(1);
   Graph_Graph063->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph063);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A5_green");
   pt->Draw();
   Ge_R3_18->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_19
   TPad *Ge_R3_19 = new TPad("Ge_R3_19", "Ge_R3_19",0.51,0.51,0.5733333,0.74);
   Ge_R3_19->Draw();
   Ge_R3_19->cd();
   Ge_R3_19->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_19->SetFillColor(0);
   Ge_R3_19->SetBorderMode(0);
   Ge_R3_19->SetBorderSize(2);
   Ge_R3_19->SetFrameBorderMode(0);
   Ge_R3_19->SetFrameBorderMode(0);
   
   Double_t Graph0_fx64[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy64[5] = {
   -0.2587967,
   -0.02557373,
   0.8445435,
   -0.2232056,
   -0.230957};
   graph = new TGraph(5,Graph0_fx64,Graph0_fy64);
   graph->SetName("Graph0");
   graph->SetTitle("R3A5_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph064 = new TH1F("Graph_Graph064","R3A5_black",100,0,1536.634);
   Graph_Graph064->SetMinimum(-1.5);
   Graph_Graph064->SetMaximum(1.5);
   Graph_Graph064->SetDirectory(0);
   Graph_Graph064->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph064->SetLineColor(ci);
   Graph_Graph064->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph064->GetXaxis()->SetLabelFont(42);
   Graph_Graph064->GetXaxis()->SetTitleOffset(1);
   Graph_Graph064->GetXaxis()->SetTitleFont(42);
   Graph_Graph064->GetYaxis()->SetLabelFont(42);
   Graph_Graph064->GetYaxis()->SetTitleFont(42);
   Graph_Graph064->GetZaxis()->SetLabelFont(42);
   Graph_Graph064->GetZaxis()->SetTitleOffset(1);
   Graph_Graph064->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph064);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A5_black");
   pt->Draw();
   Ge_R3_19->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_20
   TPad *Ge_R3_20 = new TPad("Ge_R3_20", "Ge_R3_20",0.5933333,0.51,0.6566667,0.74);
   Ge_R3_20->Draw();
   Ge_R3_20->cd();
   Ge_R3_20->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_20->SetFillColor(0);
   Ge_R3_20->SetBorderMode(0);
   Ge_R3_20->SetBorderSize(2);
   Ge_R3_20->SetFrameBorderMode(0);
   Ge_R3_20->SetFrameBorderMode(0);
   
   Double_t Graph0_fx65[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy65[5] = {
   -0.5156708,
   0.05764771,
   0.9474487,
   -0.5770264,
   -0.2910156};
   graph = new TGraph(5,Graph0_fx65,Graph0_fy65);
   graph->SetName("Graph0");
   graph->SetTitle("R3A5_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph065 = new TH1F("Graph_Graph065","R3A5_blue",100,0,1536.634);
   Graph_Graph065->SetMinimum(-1.5);
   Graph_Graph065->SetMaximum(1.5);
   Graph_Graph065->SetDirectory(0);
   Graph_Graph065->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph065->SetLineColor(ci);
   Graph_Graph065->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph065->GetXaxis()->SetLabelFont(42);
   Graph_Graph065->GetXaxis()->SetTitleOffset(1);
   Graph_Graph065->GetXaxis()->SetTitleFont(42);
   Graph_Graph065->GetYaxis()->SetLabelFont(42);
   Graph_Graph065->GetYaxis()->SetTitleFont(42);
   Graph_Graph065->GetZaxis()->SetLabelFont(42);
   Graph_Graph065->GetZaxis()->SetTitleOffset(1);
   Graph_Graph065->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph065);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A5_blue");
   pt->Draw();
   Ge_R3_20->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_21
   TPad *Ge_R3_21 = new TPad("Ge_R3_21", "Ge_R3_21",0.6766667,0.51,0.74,0.74);
   Ge_R3_21->Draw();
   Ge_R3_21->cd();
   Ge_R3_21->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_21->SetFillColor(0);
   Ge_R3_21->SetBorderMode(0);
   Ge_R3_21->SetBorderSize(2);
   Ge_R3_21->SetFrameBorderMode(0);
   Ge_R3_21->SetFrameBorderMode(0);
   
   Double_t Graph0_fx66[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy66[5] = {
   0.0504303,
   -0.04605103,
   0.06311035,
   0.1879883,
   -0.1003418};
   graph = new TGraph(5,Graph0_fx66,Graph0_fy66);
   graph->SetName("Graph0");
   graph->SetTitle("R3A6_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph066 = new TH1F("Graph_Graph066","R3A6_red",100,0,1536.634);
   Graph_Graph066->SetMinimum(-1.5);
   Graph_Graph066->SetMaximum(1.5);
   Graph_Graph066->SetDirectory(0);
   Graph_Graph066->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph066->SetLineColor(ci);
   Graph_Graph066->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph066->GetXaxis()->SetLabelFont(42);
   Graph_Graph066->GetXaxis()->SetTitleOffset(1);
   Graph_Graph066->GetXaxis()->SetTitleFont(42);
   Graph_Graph066->GetYaxis()->SetLabelFont(42);
   Graph_Graph066->GetYaxis()->SetTitleFont(42);
   Graph_Graph066->GetZaxis()->SetLabelFont(42);
   Graph_Graph066->GetZaxis()->SetTitleOffset(1);
   Graph_Graph066->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph066);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A6_red");
   pt->Draw();
   Ge_R3_21->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_22
   TPad *Ge_R3_22 = new TPad("Ge_R3_22", "Ge_R3_22",0.76,0.51,0.8233333,0.74);
   Ge_R3_22->Draw();
   Ge_R3_22->cd();
   Ge_R3_22->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_22->SetFillColor(0);
   Ge_R3_22->SetBorderMode(0);
   Ge_R3_22->SetBorderSize(2);
   Ge_R3_22->SetFrameBorderMode(0);
   Ge_R3_22->SetFrameBorderMode(0);
   
   Double_t Graph0_fx67[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy67[5] = {
   0.2111282,
   -0.2886353,
   -0.003417969,
   0.151123,
   -0.04174805};
   graph = new TGraph(5,Graph0_fx67,Graph0_fy67);
   graph->SetName("Graph0");
   graph->SetTitle("R3A6_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph067 = new TH1F("Graph_Graph067","R3A6_green",100,0,1536.634);
   Graph_Graph067->SetMinimum(-1.5);
   Graph_Graph067->SetMaximum(1.5);
   Graph_Graph067->SetDirectory(0);
   Graph_Graph067->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph067->SetLineColor(ci);
   Graph_Graph067->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph067->GetXaxis()->SetLabelFont(42);
   Graph_Graph067->GetXaxis()->SetTitleOffset(1);
   Graph_Graph067->GetXaxis()->SetTitleFont(42);
   Graph_Graph067->GetYaxis()->SetLabelFont(42);
   Graph_Graph067->GetYaxis()->SetTitleFont(42);
   Graph_Graph067->GetZaxis()->SetLabelFont(42);
   Graph_Graph067->GetZaxis()->SetTitleOffset(1);
   Graph_Graph067->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph067);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A6_green");
   pt->Draw();
   Ge_R3_22->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_23
   TPad *Ge_R3_23 = new TPad("Ge_R3_23", "Ge_R3_23",0.8433333,0.51,0.9066667,0.74);
   Ge_R3_23->Draw();
   Ge_R3_23->cd();
   Ge_R3_23->Range(0,0,1,1);
   Ge_R3_23->SetFillColor(0);
   Ge_R3_23->SetBorderMode(0);
   Ge_R3_23->SetBorderSize(2);
   Ge_R3_23->SetFrameBorderMode(0);
   Ge_R3_23->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_24
   TPad *Ge_R3_24 = new TPad("Ge_R3_24", "Ge_R3_24",0.9266667,0.51,0.99,0.74);
   Ge_R3_24->Draw();
   Ge_R3_24->cd();
   Ge_R3_24->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_24->SetFillColor(0);
   Ge_R3_24->SetBorderMode(0);
   Ge_R3_24->SetBorderSize(2);
   Ge_R3_24->SetFrameBorderMode(0);
   Ge_R3_24->SetFrameBorderMode(0);
   
   Double_t Graph0_fx68[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy68[5] = {
   0.01499176,
   0.5608521,
   -0.6207886,
   -0.3161011,
   0.4466553};
   graph = new TGraph(5,Graph0_fx68,Graph0_fy68);
   graph->SetName("Graph0");
   graph->SetTitle("R3A6_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph068 = new TH1F("Graph_Graph068","R3A6_blue",100,0,1536.634);
   Graph_Graph068->SetMinimum(-1.5);
   Graph_Graph068->SetMaximum(1.5);
   Graph_Graph068->SetDirectory(0);
   Graph_Graph068->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph068->SetLineColor(ci);
   Graph_Graph068->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph068->GetXaxis()->SetLabelFont(42);
   Graph_Graph068->GetXaxis()->SetTitleOffset(1);
   Graph_Graph068->GetXaxis()->SetTitleFont(42);
   Graph_Graph068->GetYaxis()->SetLabelFont(42);
   Graph_Graph068->GetYaxis()->SetTitleFont(42);
   Graph_Graph068->GetZaxis()->SetLabelFont(42);
   Graph_Graph068->GetZaxis()->SetTitleOffset(1);
   Graph_Graph068->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph068);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A6_blue");
   pt->Draw();
   Ge_R3_24->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_25
   TPad *Ge_R3_25 = new TPad("Ge_R3_25", "Ge_R3_25",0.01,0.26,0.07333333,0.49);
   Ge_R3_25->Draw();
   Ge_R3_25->cd();
   Ge_R3_25->Range(0,0,1,1);
   Ge_R3_25->SetFillColor(0);
   Ge_R3_25->SetBorderMode(0);
   Ge_R3_25->SetBorderSize(2);
   Ge_R3_25->SetFrameBorderMode(0);
   Ge_R3_25->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_26
   TPad *Ge_R3_26 = new TPad("Ge_R3_26", "Ge_R3_26",0.09333333,0.26,0.1566667,0.49);
   Ge_R3_26->Draw();
   Ge_R3_26->cd();
   Ge_R3_26->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_26->SetFillColor(0);
   Ge_R3_26->SetBorderMode(0);
   Ge_R3_26->SetBorderSize(2);
   Ge_R3_26->SetFrameBorderMode(0);
   Ge_R3_26->SetFrameBorderMode(0);
   
   Double_t Graph0_fx69[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy69[5] = {
   0.04715729,
   0.5335693,
   -0.6982422,
   -0.1143188,
   0.3625488};
   graph = new TGraph(5,Graph0_fx69,Graph0_fy69);
   graph->SetName("Graph0");
   graph->SetTitle("R3A7_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph069 = new TH1F("Graph_Graph069","R3A7_green",100,0,1536.634);
   Graph_Graph069->SetMinimum(-1.5);
   Graph_Graph069->SetMaximum(1.5);
   Graph_Graph069->SetDirectory(0);
   Graph_Graph069->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph069->SetLineColor(ci);
   Graph_Graph069->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph069->GetXaxis()->SetLabelFont(42);
   Graph_Graph069->GetXaxis()->SetTitleOffset(1);
   Graph_Graph069->GetXaxis()->SetTitleFont(42);
   Graph_Graph069->GetYaxis()->SetLabelFont(42);
   Graph_Graph069->GetYaxis()->SetTitleFont(42);
   Graph_Graph069->GetZaxis()->SetLabelFont(42);
   Graph_Graph069->GetZaxis()->SetTitleOffset(1);
   Graph_Graph069->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph069);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A7_green");
   pt->Draw();
   Ge_R3_26->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_27
   TPad *Ge_R3_27 = new TPad("Ge_R3_27", "Ge_R3_27",0.1766667,0.26,0.24,0.49);
   Ge_R3_27->Draw();
   Ge_R3_27->cd();
   Ge_R3_27->Range(0,0,1,1);
   Ge_R3_27->SetFillColor(0);
   Ge_R3_27->SetBorderMode(0);
   Ge_R3_27->SetBorderSize(2);
   Ge_R3_27->SetFrameBorderMode(0);
   Ge_R3_27->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_28
   TPad *Ge_R3_28 = new TPad("Ge_R3_28", "Ge_R3_28",0.26,0.26,0.3233333,0.49);
   Ge_R3_28->Draw();
   Ge_R3_28->cd();
   Ge_R3_28->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_28->SetFillColor(0);
   Ge_R3_28->SetBorderMode(0);
   Ge_R3_28->SetBorderSize(2);
   Ge_R3_28->SetFrameBorderMode(0);
   Ge_R3_28->SetFrameBorderMode(0);
   
   Double_t Graph0_fx70[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy70[5] = {
   -0.2601624,
   0.4682617,
   0.154541,
   -0.4850464,
   0.2039795};
   graph = new TGraph(5,Graph0_fx70,Graph0_fy70);
   graph->SetName("Graph0");
   graph->SetTitle("R3A7_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph070 = new TH1F("Graph_Graph070","R3A7_blue",100,0,1536.634);
   Graph_Graph070->SetMinimum(-1.5);
   Graph_Graph070->SetMaximum(1.5);
   Graph_Graph070->SetDirectory(0);
   Graph_Graph070->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph070->SetLineColor(ci);
   Graph_Graph070->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph070->GetXaxis()->SetLabelFont(42);
   Graph_Graph070->GetXaxis()->SetTitleOffset(1);
   Graph_Graph070->GetXaxis()->SetTitleFont(42);
   Graph_Graph070->GetYaxis()->SetLabelFont(42);
   Graph_Graph070->GetYaxis()->SetTitleFont(42);
   Graph_Graph070->GetZaxis()->SetLabelFont(42);
   Graph_Graph070->GetZaxis()->SetTitleOffset(1);
   Graph_Graph070->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph070);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A7_blue");
   pt->Draw();
   Ge_R3_28->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_29
   TPad *Ge_R3_29 = new TPad("Ge_R3_29", "Ge_R3_29",0.3433333,0.26,0.4066667,0.49);
   Ge_R3_29->Draw();
   Ge_R3_29->cd();
   Ge_R3_29->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_29->SetFillColor(0);
   Ge_R3_29->SetBorderMode(0);
   Ge_R3_29->SetBorderSize(2);
   Ge_R3_29->SetFrameBorderMode(0);
   Ge_R3_29->SetFrameBorderMode(0);
   
   Double_t Graph0_fx71[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy71[5] = {
   -0.4323578,
   -10.25482,
   1.398926,
   14.13739,
   -5.196045};
   graph = new TGraph(5,Graph0_fx71,Graph0_fy71);
   graph->SetName("Graph0");
   graph->SetTitle("R3A8_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph071 = new TH1F("Graph_Graph071","R3A8_red",100,0,1536.634);
   Graph_Graph071->SetMinimum(-1.5);
   Graph_Graph071->SetMaximum(1.5);
   Graph_Graph071->SetDirectory(0);
   Graph_Graph071->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph071->SetLineColor(ci);
   Graph_Graph071->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph071->GetXaxis()->SetLabelFont(42);
   Graph_Graph071->GetXaxis()->SetTitleOffset(1);
   Graph_Graph071->GetXaxis()->SetTitleFont(42);
   Graph_Graph071->GetYaxis()->SetLabelFont(42);
   Graph_Graph071->GetYaxis()->SetTitleFont(42);
   Graph_Graph071->GetZaxis()->SetLabelFont(42);
   Graph_Graph071->GetZaxis()->SetTitleOffset(1);
   Graph_Graph071->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph071);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3768949,0.94,0.6231051,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A8_red");
   pt->Draw();
   Ge_R3_29->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_30
   TPad *Ge_R3_30 = new TPad("Ge_R3_30", "Ge_R3_30",0.4266667,0.26,0.49,0.49);
   Ge_R3_30->Draw();
   Ge_R3_30->cd();
   Ge_R3_30->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_30->SetFillColor(0);
   Ge_R3_30->SetBorderMode(0);
   Ge_R3_30->SetBorderSize(2);
   Ge_R3_30->SetFrameBorderMode(0);
   Ge_R3_30->SetFrameBorderMode(0);
   
   Double_t Graph0_fx72[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy72[5] = {
   -0.1742935,
   0.2406006,
   -0.1589966,
   0.07055664,
   -0.03808594};
   graph = new TGraph(5,Graph0_fx72,Graph0_fy72);
   graph->SetName("Graph0");
   graph->SetTitle("R3A8_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph072 = new TH1F("Graph_Graph072","R3A8_green",100,0,1536.634);
   Graph_Graph072->SetMinimum(-1.5);
   Graph_Graph072->SetMaximum(1.5);
   Graph_Graph072->SetDirectory(0);
   Graph_Graph072->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph072->SetLineColor(ci);
   Graph_Graph072->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph072->GetXaxis()->SetLabelFont(42);
   Graph_Graph072->GetXaxis()->SetTitleOffset(1);
   Graph_Graph072->GetXaxis()->SetTitleFont(42);
   Graph_Graph072->GetYaxis()->SetLabelFont(42);
   Graph_Graph072->GetYaxis()->SetTitleFont(42);
   Graph_Graph072->GetZaxis()->SetLabelFont(42);
   Graph_Graph072->GetZaxis()->SetTitleOffset(1);
   Graph_Graph072->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph072);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A8_green");
   pt->Draw();
   Ge_R3_30->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_31
   TPad *Ge_R3_31 = new TPad("Ge_R3_31", "Ge_R3_31",0.51,0.26,0.5733333,0.49);
   Ge_R3_31->Draw();
   Ge_R3_31->cd();
   Ge_R3_31->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_31->SetFillColor(0);
   Ge_R3_31->SetBorderMode(0);
   Ge_R3_31->SetBorderSize(2);
   Ge_R3_31->SetFrameBorderMode(0);
   Ge_R3_31->SetFrameBorderMode(0);
   
   Double_t Graph0_fx73[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy73[5] = {
   0.08301544,
   0.4920349,
   -0.7619019,
   -0.4439697,
   0.4818115};
   graph = new TGraph(5,Graph0_fx73,Graph0_fy73);
   graph->SetName("Graph0");
   graph->SetTitle("R3A8_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph073 = new TH1F("Graph_Graph073","R3A8_black",100,0,1536.634);
   Graph_Graph073->SetMinimum(-1.5);
   Graph_Graph073->SetMaximum(1.5);
   Graph_Graph073->SetDirectory(0);
   Graph_Graph073->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph073->SetLineColor(ci);
   Graph_Graph073->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph073->GetXaxis()->SetLabelFont(42);
   Graph_Graph073->GetXaxis()->SetTitleOffset(1);
   Graph_Graph073->GetXaxis()->SetTitleFont(42);
   Graph_Graph073->GetYaxis()->SetLabelFont(42);
   Graph_Graph073->GetYaxis()->SetTitleFont(42);
   Graph_Graph073->GetZaxis()->SetLabelFont(42);
   Graph_Graph073->GetZaxis()->SetTitleOffset(1);
   Graph_Graph073->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph073);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A8_black");
   pt->Draw();
   Ge_R3_31->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_32
   TPad *Ge_R3_32 = new TPad("Ge_R3_32", "Ge_R3_32",0.5933333,0.26,0.6566667,0.49);
   Ge_R3_32->Draw();
   Ge_R3_32->cd();
   Ge_R3_32->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_32->SetFillColor(0);
   Ge_R3_32->SetBorderMode(0);
   Ge_R3_32->SetBorderSize(2);
   Ge_R3_32->SetFrameBorderMode(0);
   Ge_R3_32->SetFrameBorderMode(0);
   
   Double_t Graph0_fx74[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy74[5] = {
   0.1170807,
   0.1368713,
   -0.3241577,
   6.103516e-05,
   0.1621094};
   graph = new TGraph(5,Graph0_fx74,Graph0_fy74);
   graph->SetName("Graph0");
   graph->SetTitle("R3A8_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph074 = new TH1F("Graph_Graph074","R3A8_blue",100,0,1536.634);
   Graph_Graph074->SetMinimum(-1.5);
   Graph_Graph074->SetMaximum(1.5);
   Graph_Graph074->SetDirectory(0);
   Graph_Graph074->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph074->SetLineColor(ci);
   Graph_Graph074->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph074->GetXaxis()->SetLabelFont(42);
   Graph_Graph074->GetXaxis()->SetTitleOffset(1);
   Graph_Graph074->GetXaxis()->SetTitleFont(42);
   Graph_Graph074->GetYaxis()->SetLabelFont(42);
   Graph_Graph074->GetYaxis()->SetTitleFont(42);
   Graph_Graph074->GetZaxis()->SetLabelFont(42);
   Graph_Graph074->GetZaxis()->SetTitleOffset(1);
   Graph_Graph074->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph074);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A8_blue");
   pt->Draw();
   Ge_R3_32->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_33
   TPad *Ge_R3_33 = new TPad("Ge_R3_33", "Ge_R3_33",0.6766667,0.26,0.74,0.49);
   Ge_R3_33->Draw();
   Ge_R3_33->cd();
   Ge_R3_33->Range(0,0,1,1);
   Ge_R3_33->SetFillColor(0);
   Ge_R3_33->SetBorderMode(0);
   Ge_R3_33->SetBorderSize(2);
   Ge_R3_33->SetFrameBorderMode(0);
   Ge_R3_33->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_34
   TPad *Ge_R3_34 = new TPad("Ge_R3_34", "Ge_R3_34",0.76,0.26,0.8233333,0.49);
   Ge_R3_34->Draw();
   Ge_R3_34->cd();
   Ge_R3_34->Range(0,0,1,1);
   Ge_R3_34->SetFillColor(0);
   Ge_R3_34->SetBorderMode(0);
   Ge_R3_34->SetBorderSize(2);
   Ge_R3_34->SetFrameBorderMode(0);
   Ge_R3_34->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_35
   TPad *Ge_R3_35 = new TPad("Ge_R3_35", "Ge_R3_35",0.8433333,0.26,0.9066667,0.49);
   Ge_R3_35->Draw();
   Ge_R3_35->cd();
   Ge_R3_35->Range(0,0,1,1);
   Ge_R3_35->SetFillColor(0);
   Ge_R3_35->SetBorderMode(0);
   Ge_R3_35->SetBorderSize(2);
   Ge_R3_35->SetFrameBorderMode(0);
   Ge_R3_35->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_36
   TPad *Ge_R3_36 = new TPad("Ge_R3_36", "Ge_R3_36",0.9266667,0.26,0.99,0.49);
   Ge_R3_36->Draw();
   Ge_R3_36->cd();
   Ge_R3_36->Range(0,0,1,1);
   Ge_R3_36->SetFillColor(0);
   Ge_R3_36->SetBorderMode(0);
   Ge_R3_36->SetBorderSize(2);
   Ge_R3_36->SetFrameBorderMode(0);
   Ge_R3_36->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_37
   TPad *Ge_R3_37 = new TPad("Ge_R3_37", "Ge_R3_37",0.01,0.01,0.07333333,0.24);
   Ge_R3_37->Draw();
   Ge_R3_37->cd();
   Ge_R3_37->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_37->SetFillColor(0);
   Ge_R3_37->SetBorderMode(0);
   Ge_R3_37->SetBorderSize(2);
   Ge_R3_37->SetFrameBorderMode(0);
   Ge_R3_37->SetFrameBorderMode(0);
   
   Double_t Graph0_fx75[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy75[5] = {
   -0.04761505,
   0.3135986,
   -0.03887939,
   -0.5244751,
   0.3140869};
   graph = new TGraph(5,Graph0_fx75,Graph0_fy75);
   graph->SetName("Graph0");
   graph->SetTitle("R3A10_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph075 = new TH1F("Graph_Graph075","R3A10_red",100,0,1536.634);
   Graph_Graph075->SetMinimum(-1.5);
   Graph_Graph075->SetMaximum(1.5);
   Graph_Graph075->SetDirectory(0);
   Graph_Graph075->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph075->SetLineColor(ci);
   Graph_Graph075->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph075->GetXaxis()->SetLabelFont(42);
   Graph_Graph075->GetXaxis()->SetTitleOffset(1);
   Graph_Graph075->GetXaxis()->SetTitleFont(42);
   Graph_Graph075->GetYaxis()->SetLabelFont(42);
   Graph_Graph075->GetYaxis()->SetTitleFont(42);
   Graph_Graph075->GetZaxis()->SetLabelFont(42);
   Graph_Graph075->GetZaxis()->SetTitleOffset(1);
   Graph_Graph075->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph075);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A10_red");
   pt->Draw();
   Ge_R3_37->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_38
   TPad *Ge_R3_38 = new TPad("Ge_R3_38", "Ge_R3_38",0.09333333,0.01,0.1566667,0.24);
   Ge_R3_38->Draw();
   Ge_R3_38->cd();
   Ge_R3_38->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_38->SetFillColor(0);
   Ge_R3_38->SetBorderMode(0);
   Ge_R3_38->SetBorderSize(2);
   Ge_R3_38->SetFrameBorderMode(0);
   Ge_R3_38->SetFrameBorderMode(0);
   
   Double_t Graph0_fx76[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy76[5] = {
   -0.1209564,
   0.2190857,
   -0.1715088,
   0.0949707,
   -0.004638672};
   graph = new TGraph(5,Graph0_fx76,Graph0_fy76);
   graph->SetName("Graph0");
   graph->SetTitle("R3A10_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph076 = new TH1F("Graph_Graph076","R3A10_green",100,0,1536.634);
   Graph_Graph076->SetMinimum(-1.5);
   Graph_Graph076->SetMaximum(1.5);
   Graph_Graph076->SetDirectory(0);
   Graph_Graph076->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph076->SetLineColor(ci);
   Graph_Graph076->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph076->GetXaxis()->SetLabelFont(42);
   Graph_Graph076->GetXaxis()->SetTitleOffset(1);
   Graph_Graph076->GetXaxis()->SetTitleFont(42);
   Graph_Graph076->GetYaxis()->SetLabelFont(42);
   Graph_Graph076->GetYaxis()->SetTitleFont(42);
   Graph_Graph076->GetZaxis()->SetLabelFont(42);
   Graph_Graph076->GetZaxis()->SetTitleOffset(1);
   Graph_Graph076->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph076);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A10_green");
   pt->Draw();
   Ge_R3_38->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_39
   TPad *Ge_R3_39 = new TPad("Ge_R3_39", "Ge_R3_39",0.1766667,0.01,0.24,0.24);
   Ge_R3_39->Draw();
   Ge_R3_39->cd();
   Ge_R3_39->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_39->SetFillColor(0);
   Ge_R3_39->SetBorderMode(0);
   Ge_R3_39->SetBorderSize(2);
   Ge_R3_39->SetFrameBorderMode(0);
   Ge_R3_39->SetFrameBorderMode(0);
   
   Double_t Graph0_fx77[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy77[5] = {
   -0.5357971,
   0.05667114,
   0.8134155,
   0.2106934,
   -0.5690918};
   graph = new TGraph(5,Graph0_fx77,Graph0_fy77);
   graph->SetName("Graph0");
   graph->SetTitle("R3A10_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph077 = new TH1F("Graph_Graph077","R3A10_black",100,0,1536.634);
   Graph_Graph077->SetMinimum(-1.5);
   Graph_Graph077->SetMaximum(1.5);
   Graph_Graph077->SetDirectory(0);
   Graph_Graph077->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph077->SetLineColor(ci);
   Graph_Graph077->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph077->GetXaxis()->SetLabelFont(42);
   Graph_Graph077->GetXaxis()->SetTitleOffset(1);
   Graph_Graph077->GetXaxis()->SetTitleFont(42);
   Graph_Graph077->GetYaxis()->SetLabelFont(42);
   Graph_Graph077->GetYaxis()->SetTitleFont(42);
   Graph_Graph077->GetZaxis()->SetLabelFont(42);
   Graph_Graph077->GetZaxis()->SetTitleOffset(1);
   Graph_Graph077->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph077);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A10_black");
   pt->Draw();
   Ge_R3_39->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_40
   TPad *Ge_R3_40 = new TPad("Ge_R3_40", "Ge_R3_40",0.26,0.01,0.3233333,0.24);
   Ge_R3_40->Draw();
   Ge_R3_40->cd();
   Ge_R3_40->Range(0,0,1,1);
   Ge_R3_40->SetFillColor(0);
   Ge_R3_40->SetBorderMode(0);
   Ge_R3_40->SetBorderSize(2);
   Ge_R3_40->SetFrameBorderMode(0);
   Ge_R3_40->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_41
   TPad *Ge_R3_41 = new TPad("Ge_R3_41", "Ge_R3_41",0.3433333,0.01,0.4066667,0.24);
   Ge_R3_41->Draw();
   Ge_R3_41->cd();
   Ge_R3_41->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_41->SetFillColor(0);
   Ge_R3_41->SetBorderMode(0);
   Ge_R3_41->SetBorderSize(2);
   Ge_R3_41->SetFrameBorderMode(0);
   Ge_R3_41->SetFrameBorderMode(0);
   
   Double_t Graph0_fx78[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy78[5] = {
   0.1142883,
   0.1851807,
   -0.486145,
   0.0670166,
   0.1931152};
   graph = new TGraph(5,Graph0_fx78,Graph0_fy78);
   graph->SetName("Graph0");
   graph->SetTitle("R3A11_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph078 = new TH1F("Graph_Graph078","R3A11_red",100,0,1536.634);
   Graph_Graph078->SetMinimum(-1.5);
   Graph_Graph078->SetMaximum(1.5);
   Graph_Graph078->SetDirectory(0);
   Graph_Graph078->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph078->SetLineColor(ci);
   Graph_Graph078->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph078->GetXaxis()->SetLabelFont(42);
   Graph_Graph078->GetXaxis()->SetTitleOffset(1);
   Graph_Graph078->GetXaxis()->SetTitleFont(42);
   Graph_Graph078->GetYaxis()->SetLabelFont(42);
   Graph_Graph078->GetYaxis()->SetTitleFont(42);
   Graph_Graph078->GetZaxis()->SetLabelFont(42);
   Graph_Graph078->GetZaxis()->SetTitleOffset(1);
   Graph_Graph078->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph078);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A11_red");
   pt->Draw();
   Ge_R3_41->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_42
   TPad *Ge_R3_42 = new TPad("Ge_R3_42", "Ge_R3_42",0.4266667,0.01,0.49,0.24);
   Ge_R3_42->Draw();
   Ge_R3_42->cd();
   Ge_R3_42->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_42->SetFillColor(0);
   Ge_R3_42->SetBorderMode(0);
   Ge_R3_42->SetBorderSize(2);
   Ge_R3_42->SetFrameBorderMode(0);
   Ge_R3_42->SetFrameBorderMode(0);
   
   Double_t Graph0_fx79[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy79[5] = {
   -0.2388763,
   0.4614868,
   -0.07562256,
   -0.1173096,
   0.08996582};
   graph = new TGraph(5,Graph0_fx79,Graph0_fy79);
   graph->SetName("Graph0");
   graph->SetTitle("R3A11_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph079 = new TH1F("Graph_Graph079","R3A11_green",100,0,1536.634);
   Graph_Graph079->SetMinimum(-1.5);
   Graph_Graph079->SetMaximum(1.5);
   Graph_Graph079->SetDirectory(0);
   Graph_Graph079->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph079->SetLineColor(ci);
   Graph_Graph079->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph079->GetXaxis()->SetLabelFont(42);
   Graph_Graph079->GetXaxis()->SetTitleOffset(1);
   Graph_Graph079->GetXaxis()->SetTitleFont(42);
   Graph_Graph079->GetYaxis()->SetLabelFont(42);
   Graph_Graph079->GetYaxis()->SetTitleFont(42);
   Graph_Graph079->GetZaxis()->SetLabelFont(42);
   Graph_Graph079->GetZaxis()->SetTitleOffset(1);
   Graph_Graph079->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph079);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A11_green");
   pt->Draw();
   Ge_R3_42->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_43
   TPad *Ge_R3_43 = new TPad("Ge_R3_43", "Ge_R3_43",0.51,0.01,0.5733333,0.24);
   Ge_R3_43->Draw();
   Ge_R3_43->cd();
   Ge_R3_43->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_43->SetFillColor(0);
   Ge_R3_43->SetBorderMode(0);
   Ge_R3_43->SetBorderSize(2);
   Ge_R3_43->SetFrameBorderMode(0);
   Ge_R3_43->SetFrameBorderMode(0);
   
   Double_t Graph0_fx80[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy80[5] = {
   -0.2425766,
   0.1890564,
   0.7522583,
   -0.8786011,
   0.1933594};
   graph = new TGraph(5,Graph0_fx80,Graph0_fy80);
   graph->SetName("Graph0");
   graph->SetTitle("R3A11_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph080 = new TH1F("Graph_Graph080","R3A11_black",100,0,1536.634);
   Graph_Graph080->SetMinimum(-1.5);
   Graph_Graph080->SetMaximum(1.5);
   Graph_Graph080->SetDirectory(0);
   Graph_Graph080->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph080->SetLineColor(ci);
   Graph_Graph080->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph080->GetXaxis()->SetLabelFont(42);
   Graph_Graph080->GetXaxis()->SetTitleOffset(1);
   Graph_Graph080->GetXaxis()->SetTitleFont(42);
   Graph_Graph080->GetYaxis()->SetLabelFont(42);
   Graph_Graph080->GetYaxis()->SetTitleFont(42);
   Graph_Graph080->GetZaxis()->SetLabelFont(42);
   Graph_Graph080->GetZaxis()->SetTitleOffset(1);
   Graph_Graph080->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph080);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A11_black");
   pt->Draw();
   Ge_R3_43->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_44
   TPad *Ge_R3_44 = new TPad("Ge_R3_44", "Ge_R3_44",0.5933333,0.01,0.6566667,0.24);
   Ge_R3_44->Draw();
   Ge_R3_44->cd();
   Ge_R3_44->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_44->SetFillColor(0);
   Ge_R3_44->SetBorderMode(0);
   Ge_R3_44->SetBorderSize(2);
   Ge_R3_44->SetFrameBorderMode(0);
   Ge_R3_44->SetFrameBorderMode(0);
   
   Double_t Graph0_fx81[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy81[5] = {
   -0.1436157,
   -0.1726379,
   0.508667,
   -0.0201416,
   -0.2060547};
   graph = new TGraph(5,Graph0_fx81,Graph0_fy81);
   graph->SetName("Graph0");
   graph->SetTitle("R3A11_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph081 = new TH1F("Graph_Graph081","R3A11_blue",100,0,1536.634);
   Graph_Graph081->SetMinimum(-1.5);
   Graph_Graph081->SetMaximum(1.5);
   Graph_Graph081->SetDirectory(0);
   Graph_Graph081->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph081->SetLineColor(ci);
   Graph_Graph081->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph081->GetXaxis()->SetLabelFont(42);
   Graph_Graph081->GetXaxis()->SetTitleOffset(1);
   Graph_Graph081->GetXaxis()->SetTitleFont(42);
   Graph_Graph081->GetYaxis()->SetLabelFont(42);
   Graph_Graph081->GetYaxis()->SetTitleFont(42);
   Graph_Graph081->GetZaxis()->SetLabelFont(42);
   Graph_Graph081->GetZaxis()->SetTitleOffset(1);
   Graph_Graph081->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph081);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A11_blue");
   pt->Draw();
   Ge_R3_44->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_45
   TPad *Ge_R3_45 = new TPad("Ge_R3_45", "Ge_R3_45",0.6766667,0.01,0.74,0.24);
   Ge_R3_45->Draw();
   Ge_R3_45->cd();
   Ge_R3_45->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_45->SetFillColor(0);
   Ge_R3_45->SetBorderMode(0);
   Ge_R3_45->SetBorderSize(2);
   Ge_R3_45->SetFrameBorderMode(0);
   Ge_R3_45->SetFrameBorderMode(0);
   
   Double_t Graph0_fx82[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy82[5] = {
   -0.02270508,
   0.2892151,
   -0.1992188,
   -0.05596924,
   0.1361084};
   graph = new TGraph(5,Graph0_fx82,Graph0_fy82);
   graph->SetName("Graph0");
   graph->SetTitle("R3A12_red");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph082 = new TH1F("Graph_Graph082","R3A12_red",100,0,1536.634);
   Graph_Graph082->SetMinimum(-1.5);
   Graph_Graph082->SetMaximum(1.5);
   Graph_Graph082->SetDirectory(0);
   Graph_Graph082->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph082->SetLineColor(ci);
   Graph_Graph082->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph082->GetXaxis()->SetLabelFont(42);
   Graph_Graph082->GetXaxis()->SetTitleOffset(1);
   Graph_Graph082->GetXaxis()->SetTitleFont(42);
   Graph_Graph082->GetYaxis()->SetLabelFont(42);
   Graph_Graph082->GetYaxis()->SetTitleFont(42);
   Graph_Graph082->GetZaxis()->SetLabelFont(42);
   Graph_Graph082->GetZaxis()->SetTitleOffset(1);
   Graph_Graph082->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph082);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3655844,0.94,0.6344156,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A12_red");
   pt->Draw();
   Ge_R3_45->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_46
   TPad *Ge_R3_46 = new TPad("Ge_R3_46", "Ge_R3_46",0.76,0.01,0.8233333,0.24);
   Ge_R3_46->Draw();
   Ge_R3_46->cd();
   Ge_R3_46->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_46->SetFillColor(0);
   Ge_R3_46->SetBorderMode(0);
   Ge_R3_46->SetBorderSize(2);
   Ge_R3_46->SetFrameBorderMode(0);
   Ge_R3_46->SetFrameBorderMode(0);
   
   Double_t Graph0_fx83[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy83[5] = {
   -0.09398651,
   -0.06454468,
   0.7807007,
   -0.4448853,
   -0.0703125};
   graph = new TGraph(5,Graph0_fx83,Graph0_fy83);
   graph->SetName("Graph0");
   graph->SetTitle("R3A12_green");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph083 = new TH1F("Graph_Graph083","R3A12_green",100,0,1536.634);
   Graph_Graph083->SetMinimum(-1.5);
   Graph_Graph083->SetMaximum(1.5);
   Graph_Graph083->SetDirectory(0);
   Graph_Graph083->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph083->SetLineColor(ci);
   Graph_Graph083->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph083->GetXaxis()->SetLabelFont(42);
   Graph_Graph083->GetXaxis()->SetTitleOffset(1);
   Graph_Graph083->GetXaxis()->SetTitleFont(42);
   Graph_Graph083->GetYaxis()->SetLabelFont(42);
   Graph_Graph083->GetYaxis()->SetTitleFont(42);
   Graph_Graph083->GetZaxis()->SetLabelFont(42);
   Graph_Graph083->GetZaxis()->SetTitleOffset(1);
   Graph_Graph083->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph083);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A12_green");
   pt->Draw();
   Ge_R3_46->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_47
   TPad *Ge_R3_47 = new TPad("Ge_R3_47", "Ge_R3_47",0.8433333,0.01,0.9066667,0.24);
   Ge_R3_47->Draw();
   Ge_R3_47->cd();
   Ge_R3_47->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_47->SetFillColor(0);
   Ge_R3_47->SetBorderMode(0);
   Ge_R3_47->SetBorderSize(2);
   Ge_R3_47->SetFrameBorderMode(0);
   Ge_R3_47->SetFrameBorderMode(0);
   
   Double_t Graph0_fx84[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy84[5] = {
   -0.05007935,
   -0.02038574,
   0.6031494,
   -0.4547119,
   0.03210449};
   graph = new TGraph(5,Graph0_fx84,Graph0_fy84);
   graph->SetName("Graph0");
   graph->SetTitle("R3A12_black");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph084 = new TH1F("Graph_Graph084","R3A12_black",100,0,1536.634);
   Graph_Graph084->SetMinimum(-1.5);
   Graph_Graph084->SetMaximum(1.5);
   Graph_Graph084->SetDirectory(0);
   Graph_Graph084->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph084->SetLineColor(ci);
   Graph_Graph084->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph084->GetXaxis()->SetLabelFont(42);
   Graph_Graph084->GetXaxis()->SetTitleOffset(1);
   Graph_Graph084->GetXaxis()->SetTitleFont(42);
   Graph_Graph084->GetYaxis()->SetLabelFont(42);
   Graph_Graph084->GetYaxis()->SetTitleFont(42);
   Graph_Graph084->GetZaxis()->SetLabelFont(42);
   Graph_Graph084->GetZaxis()->SetTitleOffset(1);
   Graph_Graph084->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph084);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3429634,0.94,0.6570366,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A12_black");
   pt->Draw();
   Ge_R3_47->Modified();
   Ge_R3->cd();
  
// ------------>Primitives in pad: Ge_R3_48
   TPad *Ge_R3_48 = new TPad("Ge_R3_48", "Ge_R3_48",0.9266667,0.01,0.99,0.24);
   Ge_R3_48->Draw();
   Ge_R3_48->cd();
   Ge_R3_48->Range(-192.0792,-1.875,1728.713,1.875);
   Ge_R3_48->SetFillColor(0);
   Ge_R3_48->SetBorderMode(0);
   Ge_R3_48->SetBorderSize(2);
   Ge_R3_48->SetFrameBorderMode(0);
   Ge_R3_48->SetFrameBorderMode(0);
   
   Double_t Graph0_fx85[5] = {
   121.783,
   344.276,
   778.903,
   964.131,
   1408.011};
   Double_t Graph0_fy85[5] = {
   -0.05162048,
   0.1957397,
   0.1270142,
   -0.3399658,
   0.1369629};
   graph = new TGraph(5,Graph0_fx85,Graph0_fy85);
   graph->SetName("Graph0");
   graph->SetTitle("R3A12_blue");
   graph->SetFillStyle(1000);
   
   TH1F *Graph_Graph085 = new TH1F("Graph_Graph085","R3A12_blue",100,0,1536.634);
   Graph_Graph085->SetMinimum(-1.5);
   Graph_Graph085->SetMaximum(1.5);
   Graph_Graph085->SetDirectory(0);
   Graph_Graph085->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph085->SetLineColor(ci);
   Graph_Graph085->GetXaxis()->SetTitle("Energy [keV]");
   Graph_Graph085->GetXaxis()->SetLabelFont(42);
   Graph_Graph085->GetXaxis()->SetTitleOffset(1);
   Graph_Graph085->GetXaxis()->SetTitleFont(42);
   Graph_Graph085->GetYaxis()->SetLabelFont(42);
   Graph_Graph085->GetYaxis()->SetTitleFont(42);
   Graph_Graph085->GetZaxis()->SetLabelFont(42);
   Graph_Graph085->GetZaxis()->SetTitleOffset(1);
   Graph_Graph085->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph085);
   
   graph->Draw("alp");
   
   pt = new TPaveText(0.3542739,0.94,0.6457261,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   pt_LaTex = pt->AddText("R3A12_blue");
   pt->Draw();
   Ge_R3_48->Modified();
   Ge_R3->cd();
   Ge_R3->Modified();
   Ge_R3->cd();
   Ge_R3->SetSelected(Ge_R3);
}
