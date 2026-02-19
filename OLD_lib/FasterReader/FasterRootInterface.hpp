#pragma once

#include "FasterReaderV2.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/RF_Manager.hpp"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

/**
 * @brief Reads a .fast file and writes a TTree in a >root file
 */
class FasterRootInterface : public FasterReaderV2
{
public:

  FasterRootInterface() : FasterReaderV2() {}

  FasterRootInterface(std::string const & filename) : 
    FasterReaderV2(filename)
  {}

  FasterRootInterface(std::vector<Hit> && data) : 
    m_hits(std::move(data))
  {}

  // Interface to the Faster data :
  
  constexpr bool readNextRootHit()
  {
    m_hits.emplace_back();
    if (m_hits.size() == m_cursor_max || m_cursor_total == m_cursor_max) return false;
    cleanQDCs();
    auto readOutput = FasterReaderV2::readNextHit();
    switch(readOutput)
    {
      case FasterReaderV2::Alias::TRAPEZ_SPECTRO : loadTrapez(); break;
      case FasterReaderV2::Alias::RF_DATA        : loadRF    (); break;
      case FasterReaderV2::Alias::CRRC4_SPECTRO  : loadCRRC4 (); break;
      case FasterReaderV2::Alias::QDC_TDC_X1     : loadQDC<1>(); break;
      case FasterReaderV2::Alias::QDC_TDC_X2     : loadQDC<2>(); break;
      case FasterReaderV2::Alias::QDC_TDC_X3     : loadQDC<3>(); break;
      case FasterReaderV2::Alias::QDC_TDC_X4     : loadQDC<4>(); break;
      case FasterReaderV2::Alias::EOF_FASTER     : return false; // End of file
      default: ;
    }
    loadLabel(); 
    FasterReaderV2::applyTimeshifts();
    loadTimestamp();
    return true;
  }
  
  // First Interface : direct .fast data

  auto openRootFile(std::string rootname, std::string option = "RECREATE")
  {
    m_file = TFile::Open(rootname.c_str(), option.c_str());
    if (m_treeInMemory) gROOT->cd();
    return m_file;
  }

  auto initializeTree(std::string treeName, std::string treeTitle)
  {
    m_tree = gFile->Get<TTree>("Nuball2");
    if (!m_tree) m_tree = new TTree("Nuball2", "Nuball2_Events");
    m_tree = new TTree(treeName.c_str(), treeTitle.c_str());
    return m_tree;
  }

  auto       & getTree ()       {return m_tree;}
  auto const & getTree () const {return m_tree;}

  inline auto  fillTree()       {return m_tree -> Fill();}

  void writeTree()
  {
    m_file -> cd();
    if (!gFile) Colib::throw_error("In FasterRootInterface::writeTree() : no file !!");
    m_tree -> Write("",TObject::kOverwrite);
    m_file -> Close();
  }

  /// @brief Converts a .fast file into a .root file, without any modifications but optionnal timeshifts
  void convert_raw(std::string outRootFilename, std::string options = "lteqp")
  {
    openRootFile(outRootFilename);
    initializeTree("Nuball2","Nuball2_RawHits");
    RootHit o_hit; o_hit.writing(m_tree, options);
    while(readNextRootHit())
    {
      fillTree();
      printLoadingBar(m_hits.size());
    }
    print();
    getTree()->Print();
    writeTree();
  }

  // ------------------------------------------------ //
  // Second interface : loading .fast data in memory  //
  // and perform data alignement, event building ...  //
  // And THEN write the hits or events in .root files //
  // ------------------------------------------------ //

  // ------------ //
  // Data loading //
  // ------------ //

  /// @brief Loads the already open .fast file in memory. Don't forget to close it afterwards.
  void loadDatafile()
  {
    if (m_cursor_max < Colib::big<ulonglong>()) m_hits.reserve(m_cursor_max);
    if (m_cursor_max <= m_hits.size()) return; // Do not treat the file if the maximum number of hits is already reached
    // newInternalHit(); // Create first hit. The others are created in fillVector();
    while(readNextRootHit())
    {
      // FasterReaderV2::applyTimeshifts();
      // fillVector();
      ++m_cursor_total;
      printLoadingHit();
    }
  }
  
  /// @brief Opens, loads a .fast file in memory and closes it.
  void loadDatafile(std::string const & filename)
  {
    FasterReaderV2::open(filename);
    if (!m_open) return;
    loadDatafile();
    FasterReaderV2::close();
  }

  /// @brief Loads a given number.fast files in memory.
  bool loadDatafiles(std::vector<std::string> const& filenames, size_t maxFiles = std::numeric_limits<size_t>::max())
  {
    print(m_file_id, filenames.size());
    if (m_file_id >= filenames.size()) return false;

    size_t remaining = filenames.size() - m_file_id;
    size_t toLoad    = std::min(maxFiles, remaining);

    for (size_t i = 0; i < toLoad; ++i) loadDatafile(filenames[m_file_id + i]);

    m_file_id += toLoad;

    print();

    return toLoad > 0;   // or just return true;
  }

  auto & data() {return m_hits;}

  // --------------- //
  // Data Operations //
  // --------------- //

  void timeSorting()
  {
    print("Time sorting....");
    prepareSortedIndexes();
    m_timeSorted = true;
    Colib::insertionSort(m_hits, m_sortedIDs);
  }

  void checkTimeSorting()
  {
    if(!m_timeSorted) print("Time not sorted");
    auto oldTS = m_hits[m_sortedIDs[0]].stamp;
    for (auto const & index : m_sortedIDs)
    {
      if (m_hits[index].stamp < oldTS) print("oulala, weird time sorting...");
      oldTS = m_hits[index].stamp;
    }
  }

  void buildEvents()
  {
    print("Event building....");
    prepareSortedIndexes();
    if (m_sortedIDs.size() == 0) return;
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    eventID.emplace_back(m_sortedIDs[0]); // First hit of first event of buffer
    // 2. Loop through the hits buffer
    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i)
    {
      printLoadingBar(loop_i);
      auto const & hit_id     =  m_sortedIDs [loop_i         ];
      auto const & hit        =  m_hits      [hit_id         ];
      auto const & first_hit  =  m_hits      [eventID.front()];
      // 3. Add new hits until one is out of time window ("closing the event")
      if (Time_cast(hit.stamp - first_hit.stamp) < m_timeWindow)
      {
        eventID.emplace_back(hit_id);
        continue;
      }
      // -- Piece of code only executed when the event is full (closed) -- //
      // 4. Fill the event buffer
      m_eventIDbuffer.emplace_back(std::move(eventID));
      // 5. Prepare next event : 
      eventID.emplace_back(hit_id); // Save the current hit as the first hit of next event
    }
    print();
    m_eventBuilt = true;
  }

  void buildEventsWithRef(Label refLabel) noexcept
  {
    print("Event building....");
    prepareSortedIndexes();
    std::vector<size_t> eventID;
    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i) 
    {  
      printLoadingBar(loop_i);
      auto const & hit_ref = m_hits[m_sortedIDs[loop_i]];
      if (hit_ref.label != refLabel) continue;
      auto const & stamp_ref = hit_ref.stamp;
      auto stamp_beg = stamp_ref - m_timeWindow;
      auto stamp_end = stamp_ref + m_timeWindow;

      // Backward scan to find start.
      auto cursor = loop_i;
      while (stamp_beg <= m_hits[m_sortedIDs[cursor-1]].stamp) 
      {
        if (cursor-1 == 0) break;
        else --cursor;
      }
      
      // Forward fill with full window.
      int nbRefs = 0;
      for (; cursor < m_sortedIDs.size(); ++cursor) 
      {
        auto const & hit = m_hits[m_sortedIDs[cursor]];
        if (stamp_end < hit.stamp) break;
        eventID.push_back(m_sortedIDs[cursor]);
        if (hit.label == refLabel) ++nbRefs;
      }

      if (nbRefs == 1) m_eventIDbuffer.emplace_back(std::move(eventID));
      else eventID.clear();
    }
    print();
    print(m_eventIDbuffer.size(), "events");
    m_eventBuilt = true;
  }

  void buildEventsWithRf(Label rfLabel, int shift = 50_ns)
  {
    print("Event building with RF at label", rfLabel, "....");
    prepareSortedIndexes();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    m_rfTimestamps.clear();
    m_rfTimestamps.reserve(m_sortedIDs.size());
    RF_Manager rf; 
    rf.label = rfLabel;
    rf.setOffset(shift);
    bool b = rf.findFirst(m_hits, m_sortedIDs);
    if (!b) Colib::throw_error("FasterRootInterface::buildEventWithRF() :  no RF hit with label"+std::to_string(rfLabel));

    // Handle the first hit:
    eventID.emplace_back(m_sortedIDs[0]); // First hit of first event of buffer
    Timestamp pulseTimestamp = rf.refTime(m_hits[eventID.front()].stamp);

    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i)
    {
      printLoadingBar(loop_i);
      auto const & hit_id     =  m_sortedIDs [loop_i];
      auto const & hit        =  m_hits      [hit_id];

      if (rf.setHit(hit)) continue; // Do not register RF hits, only extract the period

      // Include shift
      auto const & dT     = Time_cast(hit.stamp - pulseTimestamp);
      auto const & dT_max = Time_cast(rf.period) - rf.offset();

      m_rfTimestamps.push_back(pulseTimestamp);

      Colib::printAndPause(dT);
      
      // 3. Add new hits until one is out of time window ("closing the event")
      if (dT < dT_max) 
      {
        eventID.emplace_back(hit_id);
      }
      else
      {
        // 4. Event closed, fill the event buffer
        // Colib::printAndPause(eventID);
        m_eventIDbuffer.emplace_back(std::move(eventID));
        // 5. Prepare next event : 
        eventID.emplace_back(hit_id); // Save the current hit as the first hit of next event
        m_rfTimestamps.push_back(pulseTimestamp);
        pulseTimestamp = rf.refTime(hit.stamp);
      }
    }
    print();
    m_eventBuilt = true;
  }

  // --------------------- //
  // Writing data to .root //
  // --------------------- //

  void writeHits(std::string const & rootFilename, std::string const & options = "ltqe")
  {
    ++m_nb_outputs;
    openRootFile(rootFilename);
    initializeTree("Nuball2","Nuball2_Hits");
    RootHit o_hit; o_hit.writing(m_tree, options);
    ulonglong cursor = 0;
    for (auto const & hit : m_hits)
    {
      o_hit = hit;
      m_tree -> Fill();
      printLoadingBar(++cursor);
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    print();
    clearIO();
  }

  void writeEvents(std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    if (!m_eventBuilt) buildEvents();
    ++m_nb_outputs;
    print("Writing events ....");
    auto const filename = Colib::removeExtension(rootFilename)+".root";
    openRootFile(filename, (m_nb_outputs==1) ? "recreate" : "update");
    initializeTree("Nuball2","Nuball2_Hits");
    Event o_event; o_event.writing(m_tree, options);
    ulonglong cursor = 0;
    for(auto const & event_id : m_eventIDbuffer)
    {
      o_event.clear();
      for (auto const & hit_id : event_id) 
      {
        o_event.push_back(m_hits[hit_id]);
        printLoadingBar(++cursor);
      }
      if (m_eventTrigger(o_event)) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    print();
    clearIO();
  }

  void writeEventsWithRef(Label refLabel, std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRef(refLabel);
    ++m_nb_outputs;
    print("Writing events ....");
    auto const filename = Colib::removeExtension(rootFilename)+"_ref_"+std::to_string(refLabel)+".root";
    m_file = TFile::Open(filename.c_str(), (m_nb_outputs==1) ? "recreate" : "update");
    m_tree = m_file->Get<TTree>("Nuball2");
    if (!m_tree) m_tree = new TTree("Nuball2", "Nuball2_Events");
    if (m_treeInMemory) gROOT->cd();

    Event o_event;
    o_event.writing(m_tree, options);
    ulonglong cursor = 0;
    for (auto const & event_ids : m_eventIDbuffer)
    {
      o_event.clear();
      for (auto const & hit_id : event_ids) 
      {
        auto const & hit = m_hits[hit_id];
        o_event.push_back(hit);
        if(hit.label == refLabel) o_event.setT0(hit);
        printLoadingBar(++cursor);
      }
      if (m_eventTrigger(o_event)) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", filename);
    print();
    clearIO();
  }

  void writeEventsWithRF(Label rfLabel, std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRf(rfLabel);
    ++m_nb_outputs;
    auto const filename = Colib::removeExtension(rootFilename)+"_rf.root";
    m_file = TFile::Open(filename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_PulsedEvents");
    if (m_treeInMemory) gROOT->cd();

    Event o_event;
    o_event.writing(m_tree, options);
    ulonglong cursor = 0;
    for (auto const & event_ids : m_eventIDbuffer)
    {
      o_event.clear();
      o_event.setT0(m_rfTimestamps[cursor]);
      for (auto const & hit_id : event_ids) 
      {
        auto hit = m_hits[hit_id];
        o_event.push_back(hit);
        printLoadingBar(++cursor);
      }
      if (m_eventTrigger(o_event)) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", filename);
    clearIO();
  }
  
  // -------- //
  // Cleaning //
  // -------- //

  void clearIO()
  {
    m_sortedIDs    .clear();
    m_hits         .clear();
    m_eventIDbuffer.clear();
    m_timeSorted = false;
    m_eventBuilt = false;
  }

  void clearFull()
  {
    m_file_id = 0;
    m_cursor_total = 0;
    m_nb_outputs = 0;
    clearIO();
  }

  // ---------- //
  // Parameters //
  // ---------- //

  constexpr void printLoadingBar(size_t cursor, size_t freq = 1_Mi) const noexcept
  {
    if (m_hits.size() != 0 && cursor % freq == 0)
      printsln(Colib::nicer_double(cursor, 2), Colib::nicer_seconds(m_hits[cursor].stamp*1e-12), Colib::nicer_double((100.*cursor)/m_hits.size(), 2), "    ");
  }

  constexpr void printLoadingHit(size_t freq = 1_Mi) const noexcept
  {
    if (m_cursor_total % freq == 0)
    {
      if (m_cursor_max < Colib::big<ulonglong>()) 
    #ifdef MULTITHREAD
        printslnt(Colib::nicer_double(m_cursor_total, 2), "      ");
      else 
        printslnt(Colib::nicer_double(m_cursor_total, 2), Colib::nicer_seconds(m_hits.back().getTimestamp_s()), "     ");
    #else //!MULTITHREAD
        printsln(Colib::nicer_double(m_cursor_total, 2), "      ");
      else 
        printsln(Colib::nicer_double(m_cursor_total, 2), Colib::nicer_seconds(m_hits.back().getTimestamp_s()), "     ");
    #endif //MULTITHREAD
    }
  }

  // Options setters:
  static void setTreeInMemory(bool b = true) {m_treeInMemory = b;}
  void setMaxHits       (ulonglong    max         ) {m_cursor_max       = max        ;}
  void setHitTrigger    (HitTrigger   trigger     ) {m_trigger          = trigger    ;}
  void setEventTrigger  (EventTrigger trigger     ) {m_eventTrigger     = trigger    ;}
  void setTimeWindow    (Time         time_window ) {m_timeWindow       = time_window;}

  // Options getters :
  auto const & getMaxHits() const {return m_cursor_max;}

private:

  /// @brief Resizes and fill m_sortedIDs with a std::iota sequence, only if its size does not match the hits buffer vector (m_sortedIDs.size() != m_hits.size())
  inline constexpr void prepareSortedIndexes() noexcept
  {
    if (m_sortedIDs.size() != m_hits.size())
    {
      m_sortedIDs.resize(m_hits.size());
      std::iota(std::begin(m_sortedIDs), std::end(m_sortedIDs), 0);
    }
  }

  // inline constexpr void fillVector() noexcept
  // {
  //   m_hits.emplace_back(Hit);
  //   newInternalHit();
  // }

  inline constexpr void cleanQDCs() noexcept
  {
    m_hits.back().qdc2 = 0;
    m_hits.back().qdc3 = 0;
  }

  inline constexpr void loadLabel() noexcept
  {
    m_hits.back().label = FasterReaderV2::m_header.label;
  }

  inline constexpr void loadTimestamp() noexcept {m_hits.back().stamp = FasterReaderV2::m_timestamp;}
  
  inline constexpr void loadTrapez() noexcept
  {
    m_hits.back().adc    = FasterReaderV2::m_trapez_spectro.measure;
    m_hits.back().pileup = (FasterReaderV2::m_trapez_spectro.pileup == 1 || FasterReaderV2::m_trapez_spectro.saturated == 1 || FasterReaderV2::m_trapez_spectro.sat_cpz == 1);
  }

  inline constexpr void loadRF() noexcept
  {
    m_hits.back().adc = static_cast<ADC>(FasterReaderV2::m_rf_data.period_ps());
    m_hits.back().pileup = FasterReaderV2::m_rf_data.saturated; 
  }

  inline constexpr void loadCRRC4() noexcept
  {
    m_hits.back().adc = FasterReaderV2::m_crrc4_spectro.measure;
    m_hits.back().pileup = (FasterReaderV2::m_crrc4_spectro.pileup == 1 || FasterReaderV2::m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  inline constexpr void loadQDC() noexcept
  {
    auto qdc_data = FasterReaderV2::template getQDC<n>();
    m_hits.back().pileup = false;
    if constexpr (n>0)  
    {
      m_hits.back().adc  = qdc_data[0].measure; m_hits.back().pileup |= qdc_data[0].saturated;
      if constexpr (n>1)  
      {
        m_hits.back().qdc2 = qdc_data[1].measure; m_hits.back().pileup |= qdc_data[1].saturated;
        if constexpr (n>2)  {m_hits.back().qdc3 = qdc_data[2].measure; m_hits.back().pileup |= qdc_data[2].saturated;}
      }
    }
      
  }

  std::vector<Hit> m_hits;
  std::vector<size_t> m_sortedIDs;
  std::vector<std::vector<size_t>> m_eventIDbuffer;
  
  TFile * m_file = nullptr;
  TTree * m_tree = nullptr;
  
  // I/O
  size_t m_file_id = 0; // The current .fast file number
  ulonglong m_cursor_max = Colib::big<ulonglong>();
  size_t m_cursor_total = 0; // The total number of hits loaded since the first file 
  size_t m_nb_outputs = 0; // The number of times the .root file has been filled

  // Parameters :
  inline static bool m_treeInMemory = false;
  EventTrigger m_eventTrigger = [](Event const & event) {return !event.isEmpty();};
  HitTrigger   m_trigger      = [](Hit   const & hit  ) {return !hit.pileup     ;};

  // Event building members :
  Time m_timeWindow = 1_us;
  RF_Manager m_rf;
  std::vector<Timestamp> m_rfTimestamps; // One timestamp per event
  // std::vector<Time> m_rfTimes;

  // States : 
  bool m_timeSorted = false;
  bool m_eventBuilt = false;
};

// #endif //FASTERROOTINTERFACEV2_HPP