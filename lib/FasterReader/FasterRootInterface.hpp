#pragma once

#include "FasterReader.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/RootEvent.hpp"
#include "../Classes/RF_Manager.hpp"
#include "../Classes/Timer.hpp"

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

/**
 * @brief Reads a .fast file and writes a TTree in a >root file
 */
class FasterRootInterface : public FasterReader
{
public:

  FasterRootInterface() noexcept : FasterReader() {}

  FasterRootInterface(std::string const & filename) noexcept : 
    FasterReader(filename)
  {}

  FasterRootInterface(std::vector<Hit> && data) noexcept : 
    m_hits(std::move(data))
  {}

  ~FasterRootInterface() {print();}

  // Interface to the Faster data :
  
  constexpr bool loadNextRootHit() noexcept
  {
    if (m_cursor_max < m_hits.size() || m_cursor_total_max < m_cursor_total ) return false;
    m_hits.emplace_back(); // Create a new empty hit at the end of the vector, which will be modified by the following code : 
    // cleanQDCs();
    switch(FasterReader::readNextHit())
    {
      case FasterReader::Alias::TRAPEZ_SPECTRO : loadTrapez(); break;
      case FasterReader::Alias::RF_DATA        : loadRF    (); break;
      case FasterReader::Alias::CRRC4_SPECTRO  : loadCRRC4 (); break;
      case FasterReader::Alias::QDC_TDC_X1     : loadQDC<1>(); break;
      case FasterReader::Alias::QDC_TDC_X2     : loadQDC<2>(); break;
      case FasterReader::Alias::QDC_TDC_X3     : loadQDC<3>(); break;
      case FasterReader::Alias::QDC_TDC_X4     : loadQDC<4>(); break;
      case FasterReader::Alias::EOF_FASTER     : return false; // End of file
      default: error("Unkown alias", static_cast<int>(m_header.alias()));
    }
    loadLabel(); 
    loadTimestamp();
    return true;
  }

  // Interface to ROOT :

  auto openRootFile(std::string rootname, std::string option = "RECREATE")
  {
    rootname = Colib::removeExtension(rootname)+".root";
    m_file = TFile::Open(rootname.c_str(), option.c_str());
    if (s_treeInMemory) gROOT->cd();
    return m_file;
  }

  auto initializeTree(std::string treeName, std::string treeTitle)
  {
    m_tree = gFile->Get<TTree>("Nuball2");
    if (!m_tree) m_tree = new TTree(treeName.c_str(), treeTitle.c_str());
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
  
  // First Interface : direct .fast data

  /// @brief Converts a .fast file into a .root file, without any modifications but optionnal timeshifts
  void convert_raw(std::string outRootFilename, std::string options = "lteqp")
  {
    openRootFile(outRootFilename);
    initializeTree("Nuball2","Nuball2_RawHits");
    RootHit o_hit; o_hit.writing(m_tree, options);
    while(loadNextRootHit())
    {
      fillTree();
      printHitsProgress(m_hits.size());
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
  constexpr inline bool loadDatafile() noexcept
  {
    ++m_file_id;
    if (m_cursor_max <= m_hits.size()) return false; // Do not treat the file if the maximum number of hits is already reached
    if (m_cursor_max < Colib::big<ulonglong>()) m_hits.reserve(m_cursor_max);
    while(loadNextRootHit())
    {
      printLoadingHitsProgress();
      ++m_cursor_total;
    }
    return true;
  }
  
  /// @brief Opens, loads a .fast file in memory and closes it.
  void loadDatafile(std::string const & filename)
  {
    FasterReader::open(filename);
    if (!m_open) return;
    loadDatafile();
    FasterReader::close();
  }

  /// @brief Loads a given number.fast files in memory.
  bool loadDatafiles(std::vector<std::string> const & filenames, size_t maxFiles = std::numeric_limits<size_t>::max())
  {
    m_filesNb = filenames.size();
    if (m_file_id >= m_filesNb) return false;

    size_t remaining = m_filesNb - m_file_id;
    size_t toLoad    = std::min(maxFiles, remaining);

    for (size_t i = 0; i < toLoad; ++i) loadDatafile(filenames[m_file_id]);

    return toLoad > 0;   // or just return true;
  }

  auto & data() {return m_hits;}

  // --------------- //
  // Data Operations //
  // --------------- //

  void timeSorting()
  {
    printsln("Time sorting....");
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
    prepareSortedIndexes();
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
    prepareSortedIndexes();
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
    printsln(m_eventIDbuffer.size(), "events");
    m_eventBuilt = true;
  }

  void buildEventsWithRf(Label rfLabel, int shift = 50_ns)
  {
    prepareSortedIndexes();
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
    ++m_nb_outputs;
    openRootFile(rootFilename);
    initializeTree("Nuball2","Nuball2_Hits");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootHit o_hit; o_hit.writing(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i) for (auto const & hit_id : m_eventIDbuffer[event_i]) 
    {
      o_hit = m_hits[hit_id];
      m_tree -> Fill();
      printHitsProgress(event_i, "Writting hits :  ");
    }
    writeTree();
    printsln("Nuball2 written in", rootFilename);
    clearIO();
  }

  void writeEvents(std::string const & rootFilename, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEvents();
    ++m_nb_outputs;
    auto rootFile = openRootFile(rootFilename, (m_nb_outputs==1) ? "recreate" : "update");
    initializeTree("Nuball2","Nuball2_Events");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; o_event.writing(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      o_event.clear();
      for (auto const & hit_id : m_eventIDbuffer[event_i]) 
      {
        o_event.push_back(m_hits[hit_id]);
        printEventsProgress(event_i, "Writting events : ");
      }
      if (m_eventTrigger(o_event)) 
      {
        if (calibrate) calibrateEvent(o_event);
        m_tree->Fill();
      }
    }
    writeTree();
    printsln("Nuball2 written in", rootFile->GetName());
    clearIO();
  }

  void writeEventsWithRef(std::string const & rootFilename, Label refLabel = 252, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRef(refLabel);
    ++m_nb_outputs;

    auto rootFile = openRootFile(rootFilename, (m_nb_outputs==1) ? "recreate" : "update");
    initializeTree("Nuball2", "Nuball2_EventsRef"+std::to_string(refLabel));

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; o_event.writing(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      o_event.clear();
      for (auto const & hit_id : m_eventIDbuffer[event_i]) 
      {
        auto const & hit = m_hits[hit_id];
        o_event.push_back(hit);
        if(hit.label == refLabel) o_event.setT0(hit);
        printEventsProgress(event_i, "Writting events : ");
      }
      if (m_eventTrigger(o_event)) 
      {
        if (calibrate) calibrateEvent(o_event);
        m_tree->Fill();
      }
    }
    writeTree();
    printsln("Nuball2 written in", rootFile->GetName());
    clearIO();
  }

  void writeEventsWithRF(std::string const & rootFilename, Label rfLabel = 251, std::string options = "ltTqe")
  {
    if (!m_eventBuilt) buildEventsWithRf(rfLabel);
    ++m_nb_outputs;
    auto rootFile = openRootFile(rootFilename, (m_nb_outputs++ == 0) ? "recreate" : "update");
    initializeTree("Nuball2", "Nuball2_EventsRF");

    auto const calibrate = m_calibrate; // Possible optimization
    if (calibrate) options = "ltTEQ";
    RootEvent o_event; o_event.writing(m_tree, options);

    for(size_t event_i = 0; event_i<m_eventIDbuffer.size(); ++event_i)
    {
      printEventsProgress(event_i, "Writting events : ");
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
        m_tree->Fill();
      }
    }
    writeTree();
    printsln("Nuball2 written in", rootFile->GetName());
    clearIO();
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

  void clearFull()
  {
    m_file_id = {};
    m_cursor_total = {};
    m_nb_outputs = {};
    clearIO();
  }

  // ---------- //
  // Parameters //
  // ---------- //

  constexpr inline void printLoadingHitsProgress() const noexcept
  {
    if (m_cursor_total % 1_Mi == 0)
    {
      if (m_cursor_max < Colib::big<ulonglong>()) 
    #ifdef MULTITHREAD
        printslnt("File", m_file_id, "/", m_filesNb, Colib::nicer_double(m_cursor_total, 2));
      else 
        printslnt("File", m_file_id, "/", m_filesNb, Colib::nicer_double(m_cursor_total, 2), Colib::nicer_seconds(m_hits.back().getTimestamp_s()));
    #else //!MULTITHREAD
        printsln("File", m_file_id, "/", m_filesNb, Colib::nicer_double(m_cursor_total, 2));
      else 
        printsln("File", m_file_id, "/", m_filesNb, Colib::nicer_double(m_cursor_total, 2), Colib::nicer_seconds(m_hits.back().getTimestamp_s()));
    #endif //MULTITHREAD
    }
  }

  constexpr void printEventsProgress(size_t cursor, std::string prepend = "", size_t freq = 1_Mi) const noexcept
  {
    if (m_hits.size() != 0 && (cursor % freq == 0 || cursor+1 == m_eventIDbuffer.size()))
      printsln(prepend, Colib::nicer_double(cursor, 2), Colib::nicer_double((100.*cursor)/m_eventIDbuffer.size(), 2), "%");
  }

  constexpr void printHitsProgress(size_t cursor, std::string prepend = "", size_t freq = 1_Mi) const noexcept
  {
    if (m_hits.size() != 0 && (cursor % freq == 0 || cursor+1 == m_hits.size()))
      printsln(prepend, Colib::nicer_double(cursor, 2), Colib::nicer_double((100.*cursor)/m_hits.size(), 2), "%");
  }

  // Options setters:
  // Static (true for all instances)
  static void setTreeInMemory(bool b = true) noexcept {s_treeInMemory = b;}
  // Member ()
  void setMaxHits       (ulonglong    max         ) noexcept {m_cursor_max       = max        ;}
  void setTotMaxHits    (ulonglong    max         ) noexcept {m_cursor_total_max = max        ;}
  void setHitTrigger    (HitTrigger   trigger     ) noexcept {m_trigger          = trigger    ;}
  void setEventTrigger  (EventTrigger trigger     ) noexcept {m_eventTrigger     = trigger    ;}
  void setTimeWindow    (Time         time_window ) noexcept {m_timeWindow       = time_window;}

  void setCalibration(Calibration && calib ) noexcept
  {
    if (0 < m_calib.size())
    {
      m_calibrate = true;
      m_calib = std::move(calib);
    }
  }

  // Options getters :
  constexpr auto const & getMaxHits() const noexcept {return m_cursor_max;}

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

  // inline constexpr void cleanQDCs() noexcept
  // {
  //   m_hits.back().qdc2 = 0;
  //   m_hits.back().qdc3 = 0;
  // }

  inline constexpr void loadLabel() noexcept
  {
    m_hits.back().label = FasterReader::m_header.label;
  }

  inline constexpr void loadTimestamp() noexcept {m_hits.back().stamp = FasterReader::m_timestamp;}
  
  inline constexpr void loadTrapez() noexcept
  {
    m_hits.back().adc    = FasterReader::m_trapez_spectro.measure;
    m_hits.back().pileup = (FasterReader::m_trapez_spectro.pileup == 1 || FasterReader::m_trapez_spectro.saturated == 1 || FasterReader::m_trapez_spectro.sat_cpz == 1);
  }

  inline constexpr void loadRF() noexcept
  {
    m_hits.back().adc = static_cast<ADC>(FasterReader::m_rf_data.period_ps());
    m_hits.back().pileup = FasterReader::m_rf_data.saturated; 
  }

  inline constexpr void loadCRRC4() noexcept
  {
    m_hits.back().adc = FasterReader::m_crrc4_spectro.measure;
    m_hits.back().pileup = (FasterReader::m_crrc4_spectro.pileup == 1 || FasterReader::m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  inline constexpr void loadQDC() noexcept
  {
    auto qdc_data = FasterReader::template getQDC<n>();
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
  
  // I/O :
  size_t m_filesNb = 1;
  size_t m_file_id = {}; // The current .fast file number
  ulonglong m_cursor_max = Colib::big<ulonglong>();
  ulonglong m_cursor_total_max = Colib::big<ulonglong>();
  size_t m_cursor_total = {}; // The total number of hits loaded since the first file 
  size_t m_nb_outputs = {}; // The number of times the .root file has been filled

  // Parameters :
  inline static bool s_treeInMemory = false;
  EventTrigger m_eventTrigger = [](Event const & event) {return !event.isEmpty();};
  HitTrigger   m_trigger      = [](Hit   const & hit  ) {return !hit.pileup     ;};

  // Event building members :
  Time m_timeWindow = 1_us;
  RF_Manager m_rf;
  std::vector<Timestamp> m_rfTimestamps; // One timestamp per event
  
  // States : 
  bool m_timeSorted = false;
  bool m_eventBuilt = false;

  // Data :  
  Calibration m_calib;
  bool m_calibrate = {};
};

// #endif //FASTERROOTINTERFACEV2_HPP