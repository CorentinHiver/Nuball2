#ifndef ANALYSEISOMER_H
#define ANALYSEISOMER_H

#include "../../lib/MTObjects/MTTHist.hpp"
#include "../../lib/Classes/TimeGate.hpp"

class AnalyseIsomer
{
public:
  AnalyseIsomer(){}

  bool setParameters(std::string const & parameters);
  void Initialize();
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();

private:
  // Parameters
  bool writeDSSD = false;
  std::string outDir  = "129/Analyse/Isomer/";
  std::string outRoot = "ai.root";
  Sorted_Event *m_s_event = nullptr;

  TimeGate Ge_prompt_gate;
  TimeGate Ge_delayed_gate;
  TimeGate LaBr3_prompt_gate;
  TimeGate LaBr3_delayed_gate;

  MTTHist<TH1F> Ge_spectra;
  MTTHist<TH1F> Ge_spectra_prompt;
  MTTHist<TH1F> Ge_spectra_delayed;
  MTTHist<TH2F> GePrompt_VS_GePrompt;
  MTTHist<TH2F> GeDelayed_VS_GeDelayed;
  MTTHist<TH2F> GeDelayed_VS_GePrompt;
  MTTHist<TH2F> Ge_Time_VS_Spectra;
  MTTHist<TH2F> raw_Ge_Time_VS_Spectra;
  Vector_MTTHist<TH2F> each_channel_DSSD_VS_Ge;

};

void AnalyseIsomer::Initialize()
{
  print("Initialize histograms");
  Ge_spectra.reset("Ge spectra","Ge spectra", 14000,0,7000);
  Ge_spectra_prompt.reset("Ge spectra prompt","Ge spectra prompt", 14000,0,7000);
  Ge_spectra_delayed.reset("Ge spectra delayed","Ge spectra delayed", 14000,0,7000);
  Ge_Time_VS_Spectra.reset("Ge spectra VS Time","Ge spectra VS Time", 14000,0,7000, 1000,-100,400);
  raw_Ge_Time_VS_Spectra.reset("Raw Ge spectra VS Time","Raw Ge spectra VS Time", 14000,0,7000, 1000,-100,400);
  GePrompt_VS_GePrompt.reset("Ge spectra bidim prompt","Ge spectra bidim prompt", 14000,0,7000, 3500,0,7000);
  GeDelayed_VS_GeDelayed.reset("Ge spectra bidim delayed","Ge spectra bidim delayed", 14000,0,7000, 3500,0,7000);
  GeDelayed_VS_GePrompt.reset("Ge spectra bidim delayed VS prompt","Ge spectra bidim delayed VS prompt", 14000,0,7000, 3500,0,7000);
  // GeDelayed_VS_GeDelayed_.reset("Ge spectra bidim delayed","Ge spectra bidim delayed", 14000,0,7000, 3500,0,7000);
  // std::string name;
  // if (writeDSSD)
  // each_channel_DSSD_VS_Ge.resize(56,nullptr);
  // for (int i = 0; i<56; i++)
  // {
  //   if ( g_labelToName[800+i] != "")
  //   {
  //     name = g_labelToName[800+i]+"VS Ge";
      // each_channel_DSSD_VS_Ge[i].reset(name.c_str(), name.c_str(), 1000,0,20000, 14000,0,7000);
    // }
  // }
}

void AnalyseIsomer::FillRaw(Event const & event)
{
  for (size_t i = 0; i<event.size(); i++)
  {
    auto const & label = event.labels[i];
    if (isGe[label])
    {
      auto const & nrj = event.nrjs[i];
      auto const & Time = event.Times[i];
      raw_Ge_Time_VS_Spectra.Fill(nrj,Time);
    }
  }
}

void AnalyseIsomer::FillSorted(Sorted_Event const & event_s, Event const & event)
{
  for (size_t loop_i = 0; loop_i<event_s.clover_hits.size(); loop_i++)
  {
    auto const & clover_i = event_s.clover_hits[loop_i];
    auto const & nrj_i = event_s.nrj_clover[clover_i];
    auto const & Time_i = event_s.time_clover[clover_i];
    auto const delayed_i = Time_i>50 && Time_i<150;
    auto const prompt_i =  Time_i>-10 && Time_i<20;

    if (event_s.BGO[clover_i] || nrj_i<5) continue;

    Ge_Time_VS_Spectra.Fill(nrj_i, Time_i);
    Ge_spectra.Fill(nrj_i);
    if (delayed_i) Ge_spectra_delayed.Fill(nrj_i);
    else if (prompt_i) Ge_spectra_prompt.Fill(nrj_i);

    for (size_t loop_j = loop_i+1; loop_j<event_s.clover_hits.size(); loop_j++)
    {
      auto const & clover_j = event_s.clover_hits[loop_j];
      auto const & nrj_j = event_s.nrj_clover[clover_j];
      auto const & delayed_j = event_s.delayed_Ge[clover_j];
      auto const & prompt_j = event_s.delayed_Ge[clover_j];
      auto const & Time_j = event_s.time_clover[clover_j];

      if (event_s.BGO[clover_j] || nrj_j<5) continue;

      if (prompt_i)
      {
        if (prompt_j)
        {
          GePrompt_VS_GePrompt . Fill(nrj_i,nrj_j);
          GePrompt_VS_GePrompt . Fill(nrj_j,nrj_i);
        }
        else if (delayed_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_i,nrj_j);
        }
      }
      else if (delayed_i)
      {
        if (delayed_j && abs(Time_i-Time_j)<60)
        {
          GeDelayed_VS_GeDelayed . Fill(nrj_i,nrj_j);
          GeDelayed_VS_GeDelayed . Fill(nrj_j,nrj_i);
        }
        else if (prompt_j)
        {
          GeDelayed_VS_GePrompt.Fill(nrj_j,nrj_i);
        }
      }
    }
    // if (writeDSSD)
    // for (auto const & dssd : event_s.DSSD_hits)
    // {
    //   auto const & dssd_nrj = event.nrjs[dssd];
    //   auto const dssd_label = event.labels[dssd] - 800;
    //
    //   each_channel_DSSD_VS_Ge[dssd_label] . Fill(dssd_nrj, nrj_i);
    // }
  }
}

void AnalyseIsomer::Write()
{
  std::unique_ptr<TFile> oufile(TFile::Open((outDir+outRoot).c_str(),"recreate"));
  print("Writting histograms ...");
  Ge_spectra.Write();
  Ge_spectra_delayed.Write();
  Ge_spectra_prompt.Write();
  print("Writting bidims ...");
  Ge_Time_VS_Spectra.Write();
  raw_Ge_Time_VS_Spectra.Write();
  GePrompt_VS_GePrompt.Write();
  GeDelayed_VS_GeDelayed.Write();
  GeDelayed_VS_GePrompt.Write();
  for (auto & hist : each_channel_DSSD_VS_Ge) hist . Write();
  oufile->Write();
  oufile->Close();
  print("Writting analysis in", outDir+outRoot);
}

bool AnalyseIsomer::setParameters(std::string const & parameters)
{
  std::istringstream pa(parameters);
  std::string line;
  while(getline(pa,line))
  {
    std::istringstream is(line);
    std::string temp;
    while(is>>temp)
    {
           if (temp == "gate:")
      {
        is >> temp;
        if (temp == "prompt")
        {
          is >> temp;
          print("'",temp,"'");
          if (temp == " Ge ")
          {
            is >> Ge_prompt_gate.start >> Ge_prompt_gate.stop;
          }
          if (temp == "LaBr3")
          {
            is >> LaBr3_prompt_gate.start >> LaBr3_prompt_gate.stop;
          }
        }
        if (temp == "delayed")
        {
          if (temp == "Ge")
          {
            is >> Ge_delayed_gate.start >> Ge_delayed_gate.stop;
          }
          if (temp == "LaBr3")
          {
            is >> LaBr3_delayed_gate.start >> LaBr3_delayed_gate.stop;
          }
        }
      }
      else if (temp == "outDir:")  is >> outDir;
      else if (temp == "outRoot:")  is >> outRoot;
      else if (temp == "writeDSSD") writeDSSD = true;
      else
      {
        print("Parameter", temp, "for AnalyseIsomer unkown...");
        return false;
      }
    }
  }
  return true;
}

#endif //ANALYSEISOMER_H
