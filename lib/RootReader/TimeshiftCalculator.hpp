#pragma once

#include "RootReader.hpp"
#include "../Classes/Timeshifts.hpp"
#include "TH1F.h"
#include "TH2F.h"
// #include <ROOT/RDataFrame.hxx>

class TimeshiftCalculator : public RootReader
{
public:
  template<class... ARGS>
  TimeshiftCalculator(ARGS &&... args) noexcept : RootReader(std::forward<ARGS>(args)...) {}

  void setOutputName(std::string const & name) {m_outName = name;}

  void makeHisto(Label refLabel = 252, bool _calculate = true, std::string outPath = "./", std::string output = "auto.dT")//, std::pair<ADC, ADC> ref_adc_gate = {0, std::numeric_limits<ADC>::max()})
  {
    TH1::AddDirectory(kFALSE);
    // ref_adc_gate=ref_adc_gate;
    Label minLabel = std::numeric_limits<Label>::max(); // To determine the detectors in presence
    Label maxLabel = std::numeric_limits<Label>::min(); // To determine the detectors in presence
    Time  mindT    = std::numeric_limits<Label>::max(); // To determine the coincidence time window
    Time  maxdT    = std::numeric_limits<Label>::min(); // To determine the coincidence time window
    auto labels = new TH1F("label", "label", 1_Mi, 0, 1_M);

    while(RootReader::readNext()) 
    {
      for(int hit_i = 0; hit_i < m_event.mult; ++hit_i)
      {
        printLoadingPercents();
        auto const & label_i = m_event.labels[hit_i];
        auto const & time_i  = m_event.times [hit_i];
        minLabel = std::min(minLabel, label_i);
        maxLabel = std::max(maxLabel, label_i);
        mindT    = std::min(mindT   , time_i );
        maxdT    = std::max(maxdT   , time_i );
        labels -> Fill(label_i);
      }
    }
    
    mindT*=0.9; // Get a 10% margin
    maxdT*=1.1; // Get a 10% margin
    auto const defaultBin = (maxdT - mindT)/10; // Default binning = 10ps

    if (maxLabel == std::numeric_limits<Label>::min()) Colib::throw_error("No detector !!");
    for (Label label = minLabel; label <= maxLabel; ++label)
    {
      if (labels->GetBinContent(label+1) < 1) continue;
      m_labels.push_back(label);
      
      auto const & bins = (Colib::key_found(m_binl, label)) ? ((maxdT - mindT) / m_binl.at(label)) : (defaultBin);
      auto label_str = std::to_string(label);
      std::string name = "dT_" + label_str;
      m_histos.emplace(label, new TH1F(name.c_str(), ("dT " + label_str).c_str(), bins, mindT, maxdT));
    }
    print();
    RootReader::restart();
    while(RootReader::readNext())
    {
      printLoadingPercents();
      bool gated = false;
      for(int hit_i = 0; hit_i<m_event.mult; ++hit_i)
        if ( m_event.labels[hit_i] == refLabel )
        //   && ref_adc_gate.first < m_event.adc[hit_i] 
        //   &&                      m_event.adc[hit_i] < ref_adc_gate.first)
        {
          gated = true;
          break;
        }
      if (gated) for(int hit_i = 0; hit_i<m_event.mult; ++hit_i)
        m_histos.at(m_event.labels[hit_i])->Fill(m_event.times[hit_i]);
    }
    RootReader::m_file->Close();
    print();
    auto outFile = TFile::Open(m_outName.c_str(), "recreate");
    outFile -> cd();
    Timeshifts ts; ts.resize(m_labels.back()+1);
    for (auto const & label : m_labels) 
    {
      auto histo = m_histos.at(label);
      if (!histo || histo->IsZombie()) {error("label", label, ": histo is", histo); continue;}

      auto method = classic;
      if (Colib::found(m_labels_simpleMax, label)) method = simpleMax;
      if (Colib::found(m_labels_raisingEdge, label)) method = raisingEdge;

      if (0 < histo->GetEntries()) 
      {
        if (_calculate) ts.set(label, -calculate_dT(histo, method));
        histo -> Write();
      }
    }
    outFile -> Close();
    ts.write(outPath, output);
  }

  static Time calculate_dT(TH1* histo, int method = classic)
  {
    if (!histo) Colib::throw_error("TimeshitCalculator::calculate(histo): no histo !!");

    auto dTmaxY = histo->GetMaximum();
    auto dTmaxX = histo->GetMaximumBin();

    switch(method)
    {
      case classic:
      {
        auto leftPeak = histo->FindFirstBinAbove(dTmaxY*0.7);
        auto rightPeak = histo->FindLastBinAbove(dTmaxY*0.7);
    
        histo->GetXaxis()->SetRange(leftPeak * 0.9, rightPeak * 1.1);
        
        // dTmaxX = histo->GetMaximumBin();
        leftPeak  = histo->FindFirstBinAbove(dTmaxY*0.5);
        rightPeak = histo->FindLastBinAbove (dTmaxY*0.5);
        auto proto_sigma = rightPeak-leftPeak;
    
        histo->GetXaxis()->SetRange(dTmaxX-proto_sigma, dTmaxX+proto_sigma);
    
        return histo -> GetMean();
      }
      case simpleMax:
        return histo->GetXaxis()->GetBinLowEdge(dTmaxX);

      case raisingEdge:
        return histo->GetXaxis()->GetBinLowEdge(histo->FindFirstBinAbove(dTmaxY*0.8));

      default: return 0;
    }
  }

  /// @brief Fast calculation for trees that have enough statistics. Do not perform energy gate.
  static void calculate(TTree * tree, std::string output = "auto.dT", std::string outPath = "./")
  {
    int nbLabels = 1000;
    auto bidim = new TH2F("dT", "dT", nbLabels, 0, nbLabels, 1250, -1250000, 1250000);
    tree->Draw("time:label>>dT(1000,0,1000, 250, -1250000, 1250000)","","colz");
    Timeshifts ts; ts.get().resize(nbLabels);
    for (int label = 1; label<nbLabels; ++label)
    {
      auto name = "dT_"+std::to_string(label);
      auto histo = bidim->ProjectionY(name.c_str(), label+1, label+1);
      if (!histo || histo->IsZombie()) {print(label, "skipped"); continue;}
      if (0 < histo->GetEntries()) ts.set(label, -calculate_dT(histo));
    }
    ts.write(outPath, output);
  }

  static void calculate(std::string dataFilename, std::string output = "auto.dT", std::string outPath = "./")
  {
    auto file = TFile::Open(dataFilename.c_str(), "READ");
    if (!file || file->IsZombie()) {error("in TimeshiftCalculator::calculate(): file", dataFilename, "is nullptr or a zombie"); return;}
    auto tree = file->Get<TTree>("Nuball2");
    if (!tree || tree->IsZombie()) {error("in TimeshiftCalculator::calculate(): in, ", dataFilename, ", tree Nuball2 is nullptr or a zombie"); return;}
    calculate(tree, output, outPath);
  }
  
  void setBins(std::unordered_map<Label, Time> const & bins) {m_binl = bins;}

  template <Label labels, class Generator>
  constexpr void setBins(Generator g)
  {
    for (Label label = 0; label<labels; ++label)
    {
      m_binl.emplace(label, g(label));
    }
  }

  void setSimpleMax  (Label label) {m_labels_simpleMax.push_back(label);}
  void setRaisingEdge(Label label) {m_labels_raisingEdge.push_back(label);}

private:
  Timeshifts m_ts;
  std::string m_outName = "test.root";
  std::unordered_map<Label, TH1*> m_histos;
  std::unordered_map<Label, Time> m_binl; // Bin length in ps
  std::vector<Label> m_labels;

  // Parameters:
  enum Methods{classic, simpleMax, raisingEdge};
  std::vector<Label> m_labels_simpleMax;
  std::vector<Label> m_labels_raisingEdge;
};