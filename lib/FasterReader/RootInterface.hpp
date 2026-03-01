#pragma once 

#include "../Classes/Calibration.hpp"
#include "../Classes/RootEvent.hpp"
#include "../Classes/RF_Manager.hpp"
#include "../Classes/Timer.hpp"

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

class RootInterface
{
public:
  RootInterface() noexcept = default;
  RootInterface(std::vector<Hit> && data) noexcept : 
    m_hits(std::move(data))
  {}

  void setData(std::vector<Hit> && data) noexcept {m_hits = std::move(data);}
  
  auto openRootFile(std::string rootname, std::string option = "RECREATE")
  {
    rootname = Colib::removeExtension(rootname)+".root";
    m_file = TFile::Open(rootname.c_str(), option.c_str());
    if (s_treeInMemory) gROOT->cd();
    return m_file;
  }

  auto initializeTree(std::string treeName, std::string treeTitle)
  {
    m_tree = gFile->Get<TTree>(treeName.c_str());
    if (!m_tree) m_tree = new TTree(treeName.c_str(), treeTitle.c_str());
    return m_tree;
  }

  auto       & getTree ()       {return m_tree;}
  auto const & getTree () const {return m_tree;}

  auto & data() {return m_hits;}

  inline auto fillTree() {return m_tree -> Fill();}

  void writeTree()
  {
    m_file -> cd();
    if (!gFile) Colib::throw_error("In FasterRootInterface::writeTree() : no file !!");
    m_tree -> Write("",TObject::kOverwrite);
    // printsln("Nuball2 written in ", m_file->GetName());
    m_file -> Close();
    clearIO();
  }
  
  static void setTreeInMemory(bool b = true) noexcept {s_treeInMemory = b;}
  void setTimeWindow(Time time_window ) noexcept {m_timeWindow = time_window;}
  void setEventTrigger(EventTrigger trigger) noexcept {m_eventTrigger = trigger;}

  bool timeSorting()
  {
    if (m_hits.size() == 0) 
    {
      error("Time sorting not possible on empty hits buffer");
      return false;
    }
    if (m_timeSorted == true) return true;
    printsln("Time sorting....");
    prepareSortedIndexes();
    m_timeSorted = true;
    Colib::insertionSort(m_hits, m_sortedIDs);
    return true;
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
    timeSorting();
    if (m_sortedIDs.size() == 0) return;
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    eventID.emplace_back(m_sortedIDs[0]); // First hit of first event of buffer
    m_eventIDbuffer.clear();
    m_eventIDbuffer.reserve(m_sortedIDs.size()/10);
    // 2. Loop through the hits buffer
    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i)
    {
      printHitsProgress(loop_i, "Event building :  ");
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
    // print();
    m_eventBuilt = true;
  }

  void buildEventsWithRef(Label refLabel) noexcept
  {
    timeSorting();
    std::vector<size_t> eventID;
    m_eventIDbuffer.clear();
    m_eventIDbuffer.reserve(m_sortedIDs.size()/10);
    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i) 
    {  
      printHitsProgress(loop_i, "Event building :  ");
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
    // print();
    // printsln(m_eventIDbuffer.size(), "events");
    m_eventBuilt = true;
  }

  void buildEventsWithRf(Label rfLabel, int shift = 50_ns)
  {
    timeSorting();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    m_rfTimestamps.clear();
    m_rfTimestamps.reserve(m_sortedIDs.size());
    m_eventIDbuffer.clear();
    m_eventIDbuffer.reserve(m_sortedIDs.size()/10); // Optimization attempt : reserve a size that would perfectly match an event buffer with mean multiplicity of 10

    RF_Manager rf;
    rf.label = rfLabel;
    rf.setOffset(shift);
    bool b = rf.findFirst(m_hits, m_sortedIDs);
    if (!b) Colib::throw_error("FasterRootInterface::buildEventWithRF() :  no RF hit with label "+std::to_string(rfLabel));

    // Handle the first hit:
    eventID.emplace_back(m_sortedIDs[0]); // First hit of first event of buffer
    Timestamp pulseTimestamp = rf.refTime(m_hits[eventID.front()].stamp);

    for (size_t loop_i = 1; loop_i < m_sortedIDs.size(); ++loop_i)
    {
      printHitsProgress(loop_i, "Event building with RF at label " + std::to_string(rfLabel) + " :  ");
      auto const & hit_id     =  m_sortedIDs [loop_i];
      auto const & hit        =  m_hits      [hit_id];

      if (rf.setHit(hit)) continue; // Do not register RF hits, only extract the period

      // Include shift
      auto const & dT     = Time_cast(hit.stamp - pulseTimestamp);
      auto const & dT_max = rf.period - rf.offset();

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
    // print();
    m_eventBuilt = true;
  }

  // --------------------- //
  // Writing data to .root //
  // --------------------- //

  void writeHits(std::string const & rootFilename, std::string options = "ltqe")
  {
    openRootFile(rootFilename, (m_nb_outputs++ == 0) ? "recreate" : "update");
    initializeTree("Nuball2","Nuball2_Hits");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootHit o_hit; 
    if (m_nb_outputs == 1) o_hit.writing(m_tree, options);
    else o_hit.reading(m_tree, options);
    
    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i) for (auto const & hit_id : m_eventIDbuffer[event_i]) 
    {
      o_hit = m_hits[hit_id];
      fillTree();
      printHitsProgress(event_i, "Writting hits :  ");
    }

    writeTree();
  }

  void writeEvents(std::string const & rootFilename, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEvents();
    openRootFile(rootFilename, (m_nb_outputs++ == 0) ? "recreate" : "update");
    initializeTree("Nuball2","Nuball2_Events");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; 
    if (m_nb_outputs == 1) o_event.writing(m_tree, options);
    else o_event.reading(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      o_event.clear();
      for (auto const & hit_id : m_eventIDbuffer[event_i]) 
      {
        o_event.push_back(m_hits[hit_id]);
        printEventsProgress(event_i);
      }
      if (m_eventTrigger(o_event)) 
      {
        if (calibrate) calibrateEvent(o_event);
        fillTree();
      }
    }

    writeTree();
  }

  void writeEventsWithRef(std::string const & rootFilename, Label refLabel = 252, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRef(refLabel);
    openRootFile(rootFilename, (m_nb_outputs++ == 0) ? "recreate" : "update");
    initializeTree("Nuball2", "Nuball2_EventsRef"+std::to_string(refLabel));

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; 
    if (m_nb_outputs == 1) o_event.writing(m_tree, options);
    else o_event.reading(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      o_event.clear();
      for (auto const & hit_id : m_eventIDbuffer[event_i]) 
      {
        auto const & hit = m_hits[hit_id];
        o_event.push_back(hit);
        if(hit.label == refLabel) o_event.setT0(hit);
        printEventsProgress(event_i);
      }
      if (m_eventTrigger(o_event)) 
      {
        if (calibrate) calibrateEvent(o_event);
        fillTree();
      }
    }
    writeTree();
  }

  void writeEventsWithRF(std::string const & rootFilename, Label rfLabel = 251, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRf(rfLabel);
    openRootFile(rootFilename, (m_nb_outputs++ == 0) ? "recreate" : "update");
    initializeTree("Nuball2", "Nuball2_EventsRF");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; 
    if (m_nb_outputs == 1) o_event.writing(m_tree, options);
    else o_event.reading(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      printEventsProgress(event_i);
      auto const & evt_ids = m_eventIDbuffer[event_i];
      o_event.clear();

      // Handle first hit to aligns the event wih the RF reference timestamp
      o_event.push_back(m_hits[evt_ids[0]]);
      o_event.setT0(m_rfTimestamps[event_i]);

      // Handle the other hits: 
      for (size_t hit_i = 1; hit_i<evt_ids.size(); ++hit_i) o_event.push_back(m_hits[evt_ids[hit_i]]);
      
      if (m_eventTrigger(o_event)) 
      {
        if (calibrate) calibrateEvent(o_event);
        fillTree();
      }
    }

    writeTree();
  }

  inline void calibrateEvent(Event & event)
  {
    for (int hit_i = 0; hit_i<event.mult; ++hit_i) m_calib.calibrate(event, hit_i);
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

  virtual void clearFull()
  {
    m_nb_outputs = {};
    clearIO();
  }

  // ---------- //
  // Parameters //
  // ---------- //
  constexpr void printEventsProgress(size_t cursor, size_t freq = 1_Mi) const noexcept
  {
    if (m_hits.size() != 0 && (cursor % freq == 0 || cursor+1 == m_eventIDbuffer.size()))
      printsln("Writting events : ", Colib::nicer_double(cursor, 2), Colib::nicer_double((100.*cursor)/m_eventIDbuffer.size(), 1), "%");
  }

  constexpr void printHitsProgress(size_t cursor, std::string prepend = "", size_t freq = 1_Mi) const noexcept
  {
    if (m_hits.size() != 0 && (cursor % freq == 0 || cursor+1 == m_hits.size()))
      printsln(prepend, Colib::nicer_double(cursor, 2), Colib::nicer_double((100.*cursor)/m_hits.size(), 1), "%");
  }


protected:

  /// @brief Resizes and fill m_sortedIDs with a std::iota sequence, only if its size does not match the hits buffer vector (m_sortedIDs.size() != m_hits.size())
  inline constexpr void prepareSortedIndexes() noexcept
  {
    if (m_sortedIDs.size() != m_hits.size())
    {
      m_sortedIDs.resize(m_hits.size());
      std::iota(std::begin(m_sortedIDs), std::end(m_sortedIDs), 0);
    }
  }

  std::vector<Hit> m_hits;
  std::vector<size_t> m_sortedIDs;
  std::vector<std::vector<size_t>> m_eventIDbuffer;
  
  inline static bool s_treeInMemory = false;

  TFile * m_file = nullptr;
  TTree * m_tree = nullptr;
  
  EventTrigger m_eventTrigger = [](Event const & event) {return !event.isEmpty();};
  
  // Parameters :
  HitTrigger   m_trigger      = [](Hit   const & hit  ) {return !hit.pileup     ;};

  // Event building members :
  Time m_timeWindow = 1_us;
  RF_Manager m_rf;
  std::vector<Timestamp> m_rfTimestamps; // One timestamp per event
  
  // States : 
  bool m_timeSorted = false;
  bool m_eventBuilt = false;

  // IO
  size_t m_nb_outputs = {}; // The number of times the .root file has been filled

  // Data :  
  Calibration m_calib;
  bool m_calibrate = {};
};
