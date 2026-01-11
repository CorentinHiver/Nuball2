#ifndef FASTERROOTINTERFACEV2_HPP
#define FASTERROOTINTERFACEV2_HPP

#include "FasterReaderV2.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Timer.hpp"
// #include "../Classes/RF_Manager.hpp"
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

  auto       & getHit ()       {return *m_hit;}
  auto const & getHit () const {return *m_hit;}
  auto       & getTree()       {return m_tree;}
  auto const & getTree() const {return m_tree;}

  // First Interface : direct .fast data

  auto openFile(std::string rootname, bool inMemory = true)
  {
    m_file = TFile::Open(rootname.c_str(), "RECREATE");
    if (inMemory) gROOT->cd();
    return m_file;
  }

  auto initializeTree(std::string treeName, std::string options = "lteq")
  {
    newInternalHit();
    m_tree = new TTree(treeName.c_str(), treeName.c_str());
    m_hit -> writing(m_tree, options);
    return m_tree;
  }

  void newInternalHit()
  {
    if (!m_hit) m_hit = new Hit;
  }

  bool readNextRootHit()
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

  inline auto fillTree () {return m_tree -> Fill();}

  auto writeTree()
  {
    m_file -> cd();
    if (!gFile) Colib::throw_error("In FasterRootInterface::writeTree() : no file !!");
    m_tree -> Write();
    m_file -> Close();
  }

  /**
   * @brief Converts a .fast file into a .root file, without any modifications
   */
  void convert_raw(std::string outRootFilename, bool inMemory = true)
  {
    openFile(outRootFilename, inMemory);
    initializeTree("Nuball2", "lteq");
    while(readNextRootHit())
    {
      applyTimeshifts();
      fillTree();
      printLoadingBar(m_hits.size());
    }
    print();
    getTree()->Print();
    writeTree();
  }

  /**
   * @brief Loads an opened .fast file in memory. Don't forget to close it afterwards
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
   * @brief Loads a .fast file in memory.
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

  void timeSorting()
  {
    print("Time sorting....");
    prepareOIndex();
    m_timeSorted = true;
    std::sort(m_oindex.begin(), m_oindex.end(), [&] (int i, int j) {
        return m_hits[i]->stamp < m_hits[j]->stamp;
    });
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

  void buildEvents(Time time_window)
  {
    print("Event building....");
    prepareOIndex();
    // In the following, ID and index refer to the position of the hit in the buffer 
    // (different from the label of the detector, which is used to identify it)
    // 1. Initialize the event buffer
    std::vector<size_t> eventID;
    eventID.emplace_back(m_oindex[0]); // First hit of first event of buffer
    // 2. Loop through the hits buffer
    for (size_t loop_i = 1; loop_i < m_oindex.size(); ++loop_i)
    {
      auto const & hit_id     =  m_oindex [loop_i         ];
      auto const & hit        =  m_hits   [hit_id         ];
      auto const & first_hit  =  m_hits   [eventID.front()];
      // 3. Add new hits until one is out of time window ("closing the event")
      if (Time_cast(hit->stamp - first_hit->stamp) < time_window)
      {
        eventID.emplace_back(hit_id);
        continue;
      }
      // -- Piece of code only executed when the event is full (closed) -- //
      // 4. Fill the event buffer
      m_eventIDbuffer.emplace_back(std::move(eventID));
      // 5. Prepare next event : 
      eventID.clear();
      eventID.emplace_back(hit_id); // Save the current hit as the first hit of next event
      printLoadingBar(loop_i);
    }
    print();
    m_eventBuilt = true;
  }

  void writeHits(std::string const & rootFilename, std::string const & options = "ltqe", bool inMemory = true)
  {
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_Hits");
    if (inMemory) gROOT->cd();
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

  void writeEvents(std::string const & rootFilename, Time time_window = Time(2e6), std::string const & options = "ltTqe", bool inMemory = true)
  {
    if (!m_eventBuilt) buildEvents(time_window);
    print("Writing events ....");
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_Events");
    if (inMemory) gROOT->cd();
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
      if (m_eventTrigger) m_tree->Fill();
    }
    print();
    writeTree();
    print("Nuball2 written in", rootFilename);
    clearIO();
  }

  void writeEventsWithPulse(std::string const & rootFilename, std::string const & options = "ltqe", bool inMemory = true)
  {
    m_file = TFile::Open(rootFilename.c_str(), "recreate");
    m_tree = new TTree("Nuball2", "Nuball2_PulsedEvents");
    if (inMemory) gROOT->cd();
    Event o_event;
    o_event.writing(m_tree, options);
    // TODO
    clearIO();
  }

  void clearIO()
  {
    m_oindex       .clear();
    m_hits         .clear();
    m_eventIDbuffer.clear();
    m_timeSorted = false;
    m_eventBuilt = false;
  }

  void setMaxHits(ulonglong max) {m_cursor_max = max;}

  auto const & getMaxHits() const {return m_cursor_max;}

  void printLoadingBar(size_t cursor, size_t freq = 1_Mi)
  {
    if (cursor % freq == 0)
    {
      if (m_cursor_max<Colib::big<ulonglong>()) printsln(cursor * 100. / m_cursor_max, "%");
      else printsln(Colib::nicer_double(cursor, 2), Colib::nicer_seconds(m_hits[cursor]->stamp*1e-12), "    ");
    }
  }

  // Options :
  static void setTreeInMemory(bool b = true) {m_treeInMemory = b;}
  void setHitTrigger  (HitTrigger   trigger) {m_trigger      = trigger;}
  void setEventTrigger(EventTrigger trigger) {m_eventTrigger = trigger;}
  void setTimeWindow(Time time_window) {m_timeWindow = time_window;}

private:

  /// @brief Resizes and fill m_oindex with std::iota sequence if m_oindex.size() != m_hits.size()
  void prepareOIndex()
  {
    if (m_oindex.size() != m_hits.size())
    {
      m_oindex.resize(m_hits.size());
      std::iota(std::begin(m_oindex), std::end(m_oindex), 0);
    }
  }

  /// @brief Fills the m_hits with m_hit, a pointer to the reading hit, and creates a new m-hit
  void fillVector()
  {
    m_hits.emplace_back(m_hit);
    m_hit = nullptr;
    newInternalHit();
  }

  void cleanQDCs()
  {
    m_hit -> qdc2 = 0;
    m_hit -> qdc3 = 0;
  }

  void loadLabel()
  {
    m_hit -> label = FasterReaderV2::m_header.label;
  }

  void loadTimestamp() {m_hit->stamp = FasterReaderV2::m_timestamp;}
  
  void loadTrapez()
  {
    m_hit -> adc    = FasterReaderV2::m_trapez_spectro.measure;
    m_hit -> pileup = (FasterReaderV2::m_trapez_spectro.pileup == 1 || FasterReaderV2::m_trapez_spectro.saturated == 1 || FasterReaderV2::m_trapez_spectro.sat_cpz == 1);
  }

  void loadRF()
  {
    m_hit -> adc = static_cast<ADC>(FasterReaderV2::m_rf_data.period*1000);
    m_hit -> pileup = FasterReaderV2::m_rf_data.saturated; 
  }

  void loadCRRC4()
  {
    m_hit -> adc = FasterReaderV2::m_crrc4_spectro.measure;
    m_hit -> pileup = (FasterReaderV2::m_crrc4_spectro.pileup == 1 || FasterReaderV2::m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  void loadQDC()
  {
    auto qdc_data = FasterReaderV2::template getQDC<n>();
    m_hit -> pileup = false;
    if constexpr (n>0)  {m_hit -> adc  = qdc_data[0].measure; m_hit -> pileup |= qdc_data[0].saturated;}
    if constexpr (n>1)  {m_hit -> qdc2 = qdc_data[1].measure; m_hit -> pileup |= qdc_data[1].saturated;}
    if constexpr (n>2)  {m_hit -> qdc3 = qdc_data[2].measure; m_hit -> pileup |= qdc_data[2].saturated;}
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

  Time m_timeWindow = Time(2e6);

  // States : 
  bool m_timeSorted = false;
  bool m_eventBuilt = false;
};

#endif //FASTERROOTINTERFACEV2_HPP


  // Hit* setHit(Hit * hit = nullptr) 
  // {
  //   if (m_hit)
  //   {
  //     if (m_internalHit) delete m_hit;
  //     m_hit = hit;
  //   }
  //   else
  //   { // Setting the internal hit
  //     if (hit) m_hit = hit; // if hit exists then link the internal m_hit to the input hit
  //     else // if hit is nullptr then create a new hit
  //     {
  //       m_hit = new Hit;
  //       m_internalHit = true;
  //     }
  //   }
  //   return m_hit;
  // }
