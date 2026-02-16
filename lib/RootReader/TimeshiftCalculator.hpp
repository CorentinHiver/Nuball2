#pragma once

#include "RootReader.hpp"
#include "../Classes/Timeshifts.hpp"
#include <ROOT/RDataFrame.hxx>

class TimeshiftCalculator : public RootReader
{
public:
  template<class... ARGS>
  TimeshiftCalculator(ARGS &&... args) noexcept : RootReader(std::forward<ARGS>(args)...) {}
  void calculate()
  {
    Label minLabel = std::numeric_limits<Label>::max(); // To determine the detectors in presence
    Label maxLabel = std::numeric_limits<Label>::min(); // To determine the detectors in presence
    Time  mindT    = std::numeric_limits<Label>::max(); // To determine the coincidence time window
    Time  maxdT    = std::numeric_limits<Label>::min(); // To determine the coincidence time window
    auto labels = new TH1F("label", "label", 1_Mi, 0, 1_M);

    RootEvent event;
    event.reading(m_tree, "mlT");
    while(RootReader::readNext()) 
    {
      // The maximum number of hits to read is 1e7. We apply a 10% margin anyway, so 1e7 hits should be large enough to cover the full range of detectors
      if (int(1e7) < RootReader::getCursor()) break; 
      for(int hit_i = 0; hit_i < event.mult; ++hit_i)
      {
        printLoadingPercents(10);
        auto const & label_i = event.labels[hit_i];
        auto const & time_i  = event.times [hit_i];
        minLabel = std::min(minLabel, label_i);
        maxLabel = std::max(maxLabel, label_i);
        mindT    = std::min(mindT   , time_i );
        maxdT    = std::max(maxdT   , time_i );
        labels -> Fill(label_i);
      }
    }
    print();
    mindT*=0.9; // Get a 10% margin
    maxdT*=1.1; // Get a 10% margin
    auto const defaultBin = (maxdT - mindT)/10; // Default binning = 10ps

    print(minLabel, maxLabel, mindT, maxdT);

    if (maxLabel == std::numeric_limits<Label>::min()) Colib::throw_error("No detector !!");
    for (Label label = minLabel; label <= maxLabel; ++label)
    {
      print(label);
      print(labels->GetBinContent(label+1), (Colib::key_found(m_binl, label)) ? ((maxdT - mindT) / m_binl.at(label)) : (defaultBin));
      if (labels->GetBinContent(label+1) <= 1) continue;
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
      printLoadingPercents(1_ki);
      for(int hit_i = 0; hit_i<event.mult; ++hit_i) m_histos.at(event.labels[hit_i])->Fill(event.times[hit_i]);
    }
    print();
    auto outFile = TFile::Open("test.root","recreate");
    outFile -> cd();
    for (auto const & label : m_labels) if (0 < m_histos.at(label)->GetEntries()) m_histos.at(label) -> Write();
    outFile -> Close();
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
  
private:
  Timeshifts m_ts;
  std::unordered_map<Label, TH1*> m_histos;
  std::unordered_map<Label, Time> m_binl; // Bin length in ps
  std::vector<Label> m_labels;
};