#ifndef FASTERROOTINTERFACEV2_HPP
#define FASTERROOTINTERFACEV2_HPP

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
  ~FasterRootInterface() {delete m_hit;}

  FasterRootInterface(std::string const & filename) : 
    FasterReaderV2(filename)
  {}

  FasterRootInterface(std::vector<Hit*> && hits) : 
    m_hits(std::move(hits))
  {}

  auto       & getHit ()       {return *m_hit;}
  auto const & getHit () const {return *m_hit;}

  // Interface to the Faster data :
  
  constexpr bool readNextRootHit()
  {
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
    loadTimestamp();
    return m_hits.size() < m_cursor_max;
  }
  
  constexpr inline void newInternalHit() noexcept
  {
    if (!m_hit) m_hit = new Hit;
  }

  // First Interface : direct .fast data

  auto openRootFile(std::string rootname)
  {
    m_file = TFile::Open(rootname.c_str(), "RECREATE");
    if (m_treeInMemory) gROOT->cd();
    return m_file;
  }

  auto initializeTree(std::string treeName, std::string options = "lteq")
  {
    newInternalHit();
    m_tree = new TTree(treeName.c_str(), treeName.c_str());
    m_hit -> writing(m_tree, options);
    return m_tree;
  }

  auto       & getTree ()       {return m_tree;}
  auto const & getTree () const {return m_tree;}
  inline auto  fillTree()       {return m_tree -> Fill();}

  auto writeTree()
  {
    m_file -> cd();
    if (!gFile) Colib::throw_error("In FasterRootInterface::writeTree() : no file !!");
    m_tree -> Write();
    m_file -> Close();
  }

  /**
   * @brief Converts a .fast file into a .root file, without any modifications but optionnal timeshifts
   */
  void convert_raw(std::string outRootFilename)
  {
    openRootFile(outRootFilename);
    initializeTree("Nuball2", "lteq");
    while(readNextRootHit())
    {
      FasterReaderV2::applyTimeshifts();
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
  // And THEN write the hits or event in .root files  //
  // ------------------------------------------------ //

  // ------------ //
  // Data loading //
  // ------------ //

  /**
   * @brief Loads the already open .fast file in memory. Don't forget to close it afterwards.
   */
  void loadDatafile()
  {
    if (m_cursor_max < Colib::big<ulonglong>()) m_hits.reserve(m_cursor_max);
    if (m_cursor_max <= m_hits.size()) return; // Do not treat the file if the maximum number of hits is already reached
    newInternalHit(); // Create first hit. The others are created in fillVector();
    while(readNextRootHit())
    {
      FasterReaderV2::applyTimeshifts();
      fillVector();
      printLoadingBar(m_hits.size()-1);
    }
  }
  
  /**
   * @brief Opens, loads a .fast file in memory and closes it.
   */
  void loadDatafile(std::string const & filename)
  {
    FasterReaderV2::open(filename);
    loadDatafile();
    FasterReaderV2::close();
  }

  /**
   * @brief Loads .fast files in memory
   */
  virtual void loadDatafiles(std::vector<std::string> filenames)
  {
    for (auto const & filename : filenames) (loadDatafile(filename));
    print();
  }

  // --------------- //
  // Data Operations //
  // --------------- //

  void timeSorting()
  {
    print("Time sorting....");
    prepareOutIndex();
    m_timeSorted = true;
    // std::sort(m_oindex.begin(), m_oindex.end(), [&] (int i, int j) {
    //     return m_hits[i]->stamp < m_hits[j]->stamp;
    // });
    Colib::insertionSortPtr(m_hits, m_oindex);
  }

  void checkTimeSorting()
  {
    if(!m_timeSorted) print("Time not sorted");
    auto oldTS = m_hits[m_oindex[0]]->stamp;
    for (auto const & index : m_oindex)
    {
      if (m_hits[index]->stamp < oldTS) print("oulala");
      oldTS = m_hits[index]->stamp;
    }
  }

  void buildEvents()
  {
    print("Event building....");
    prepareOutIndex();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    eventID.emplace_back(m_oindex[0]); // First hit of first event of buffer
    // 2. Loop through the hits buffer
    for (size_t loop_i = 1; loop_i < m_oindex.size(); ++loop_i)
    {
      print(loop_i);
      printLoadingBar(loop_i);
      auto const & hit_id     =  m_oindex [loop_i         ];
      auto const & hit        =  m_hits   [hit_id         ];
      auto const & first_hit  =  m_hits   [eventID.front()];
      // 3. Add new hits until one is out of time window ("closing the event")
      if (Time_cast(hit->stamp - first_hit->stamp) < m_timeWindow)
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

  void buildEventsWithRef(Label refLabel)
  {
    print("Event building....");
    prepareOutIndex();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    // 2. Loop through the hits buffer
    for (size_t loop_i = 1; loop_i < m_oindex.size(); ++loop_i)
    {
      printLoadingBar(loop_i);
      auto const & hit = m_hits[m_oindex[loop_i]];
      if (hit->label == refLabel)
      {
        auto const & hit_ref = hit; // Simple aliasing for readability
        // 3. Find the first hit of the event :
        int firstID = loop_i-1;
        while(0 < firstID)
        {
          auto dT = std::abs(Time_cast(m_hits[m_oindex[firstID]]->stamp - hit_ref->stamp));
          if (dT < m_timeWindow) --firstID;
          else break;
        }
        // 4. Fill the event :
        for (size_t loop_j = firstID; loop_j < m_oindex.size(); ++loop_j)
        {
          auto const & index_j = m_oindex[loop_j];
          auto const & hit_j   = m_hits[index_j];
          auto dT = Time_cast(hit_j->stamp - hit_ref->stamp);
          if (m_timeWindow < dT) break;
          eventID.emplace_back(index_j);
        }
        // -- Piece of code only executed when the event is full (closed) -- //
        // 4. Fill the event buffer
        m_eventIDbuffer.emplace_back(std::move(eventID));
      }
    }
    print();
    print(m_eventIDbuffer.size(), "events");
    m_eventBuilt = true;
  }

  void buildPulsedEvents()
  {
    print("Event building....");
    prepareOutIndex();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    eventID.emplace_back(m_oindex[0]); // First hit of first event of buffer
  }

  // --------------------- //
  // Writing data to .root //
  // --------------------- //

  void writeHits(std::string const & rootFilename, std::string const & options = "ltqe")
  {
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_Hits");
    if (m_treeInMemory) gROOT->cd();
    Hit o_hit;
    o_hit.writing(m_tree, options);
    ulonglong cursor = 0;
    for (auto const & hit : m_hits)
    {
      o_hit = *hit;
      m_tree -> Fill();
      printLoadingBar(++cursor);
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    clearIO();
  }

  void writeEvents(std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    if (!m_eventBuilt) buildEvents();
    print("Writing events ....");
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_Events");
    if (m_treeInMemory) gROOT->cd();
    Event o_event;
    o_event.writing(m_tree, options);
    ulonglong cursor = 0;
    for(auto const & event_id : m_eventIDbuffer)
    {
      o_event.clear();
      for (auto const & hit_id : event_id) 
      {
        o_event.push_back(*(m_hits[hit_id]));
        delete m_hits[hit_id];
        printLoadingBar(++cursor);
      }
      if (m_eventTrigger(std::forward<Event>(o_event))) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    clearIO();
  }

  void writeEventsWithRef(Label refLabel, std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    buildEventsWithRef(refLabel);
    print("Writing events ....");
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_Events");
    if (m_treeInMemory) gROOT->cd();
    Event o_event;
    o_event.writing(m_tree, options);
    ulonglong cursor = 0;
    for(auto const & event_id : m_eventIDbuffer)
    {
      o_event.clear();
      auto refTime = 0;
      for (auto const & hit_id : event_id) 
      {
        auto const & hit = m_hits[hit_id];
        o_event.push_back(*hit);
        if (hit->label == refLabel) refTime = o_event.times[o_event.mult-1];
        printLoadingBar(++cursor);
      }
      print(refTime/1_ns);
      for (int hit_i = 0; hit_i < o_event.mult; ++hit_i) o_event.times[hit_i] -= refTime;
      if (m_eventTrigger(std::forward<Event>(o_event))) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    clearIO();
  }

  void writePulsedEvents(std::string const & rootFilename, std::string const & options = "ltTqe")
  {
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_PulsedEvents");
    if (m_treeInMemory) gROOT->cd();
    Event o_event;
    o_event.writing(m_tree, options);
    // TODO
    clearIO();
  }
  
  // -------- //
  // Cleaning //
  // -------- //

  void clearIO()
  {
    m_oindex       .clear();
    m_hits         .clear();
    m_eventIDbuffer.clear();
    m_timeSorted = false;
    m_eventBuilt = false;
  }

  // ---------- //
  // Parameters //
  // ---------- //

  void printLoadingBar(size_t cursor, size_t freq = 1_Mi)
  {
    if (cursor % freq == 0)
    {
      if (m_cursor_max<Colib::big<ulonglong>()) printsln(cursor * 100. / m_cursor_max, "%");
      else printsln(Colib::nicer_double(cursor, 2), Colib::nicer_seconds(m_hits[cursor]->stamp*1e-12), "    ");
    }
  }

  // Options setters:
  void setMaxHits (ulonglong max) {m_cursor_max = max;}
  static void setTreeInMemory(bool b = true) {m_treeInMemory = b;}
  void setHitTrigger  (HitTrigger   trigger) {m_trigger      = trigger;}
  void setEventTrigger(EventTrigger trigger) {m_eventTrigger = trigger;}
  void setTimeWindow(Time time_window) {m_timeWindow = time_window;}

  // Options getters :
  auto const & getMaxHits() const {return m_cursor_max;}

private:

  /// @brief Resizes and fill m_oindex with std::iota sequence if m_oindex.size() != m_hits.size()
  inline constexpr void prepareOutIndex() noexcept
  {
    if (m_oindex.size() != m_hits.size())
    {
      m_oindex.resize(m_hits.size());
      std::iota(std::begin(m_oindex), std::end(m_oindex), 0);
    }
  }

  /// @brief Fills the m_hits with m_hit, a pointer to the reading hit, and creates a new m-hit
  inline constexpr void fillVector() noexcept
  {
    m_hits.emplace_back(m_hit);
    m_hit = nullptr;
    newInternalHit();
  }

  inline constexpr void cleanQDCs() noexcept
  {
    m_hit -> qdc2 = 0;
    m_hit -> qdc3 = 0;
  }

  inline constexpr void loadLabel() noexcept
  {
    m_hit -> label = FasterReaderV2::m_header.label;
  }

  inline constexpr void loadTimestamp() noexcept {m_hit->stamp = FasterReaderV2::m_timestamp;}
  
  inline constexpr void loadTrapez() noexcept
  {
    m_hit -> adc    = FasterReaderV2::m_trapez_spectro.measure;
    m_hit -> pileup = (FasterReaderV2::m_trapez_spectro.pileup == 1 || FasterReaderV2::m_trapez_spectro.saturated == 1 || FasterReaderV2::m_trapez_spectro.sat_cpz == 1);
  }

  inline constexpr void loadRF() noexcept
  {
    m_hit -> adc = static_cast<ADC>(FasterReaderV2::m_rf_data.period*1000);
    m_hit -> pileup = FasterReaderV2::m_rf_data.saturated; 
  }

  inline constexpr void loadCRRC4() noexcept
  {
    m_hit -> adc = FasterReaderV2::m_crrc4_spectro.measure;
    m_hit -> pileup = (FasterReaderV2::m_crrc4_spectro.pileup == 1 || FasterReaderV2::m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  inline constexpr void loadQDC() noexcept
  {
    auto qdc_data = FasterReaderV2::template getQDC<n>();
    m_hit -> pileup = false;
    if constexpr (n>0)  
    {
      m_hit -> adc  = qdc_data[0].measure; m_hit -> pileup |= qdc_data[0].saturated;
      if constexpr (n>1)  
      {
        m_hit -> qdc2 = qdc_data[1].measure; m_hit -> pileup |= qdc_data[1].saturated;
        if constexpr (n>2)  {m_hit -> qdc3 = qdc_data[2].measure; m_hit -> pileup |= qdc_data[2].saturated;}
      }
    }
      
  }

  Hit   * m_hit  = nullptr;
  TFile * m_file = nullptr;
  TTree * m_tree = nullptr;
  ulonglong m_cursor_max = Colib::big<ulonglong>();

  // I/O
  std::vector<size_t> m_oindex;
  std::vector<Hit*> m_hits;
  std::vector<std::vector<size_t>> m_eventIDbuffer;

  // Parameters :
  inline static bool m_treeInMemory = false;
  EventTrigger m_eventTrigger = [](Event const & event) {return !event.isEmpty();};
  HitTrigger   m_trigger      = [](Hit   const & hit  ) {return !hit.pileup     ;};

  // Event building members :
  Time m_timeWindow = 1_us;
  RF_Manager m_rf;

  // States : 
  bool m_timeSorted = false;
  bool m_eventBuilt = false;
};

#endif //FASTERROOTINTERFACEV2_HPP