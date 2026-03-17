#pragma once

#include "FasterReader.hpp"
#include "RootInterface.hpp"

/**
 * @brief Reads a .fast file and writes a TTree in a >root file
 */
class FasterRootInterface : public FasterReader, public RootInterface
{
public:

  FasterRootInterface() noexcept : FasterReader()  {}

  FasterRootInterface(std::string const & filename) noexcept : 
    FasterReader(filename)
  {}

  FasterRootInterface(std::vector<Hit> && data) noexcept : 
    RootInterface(std::move(data))
  {}

  ~FasterRootInterface() {}

  // Interface to the Faster data :
  
  constexpr bool loadNextRootHit() noexcept
  {
    if (m_nb_hits_max < m_hits.size() || m_nb_hits_tot_max < m_nb_hits_tot ) return false;
    auto & hit = m_hits.emplace_back(); // Create a new empty hit at the end of the vector, which will be modified by the following code :
    // cleanQDCs();
    switch(FasterReader::readNextHit())
    {
      case FasterReader::Alias::TRAPEZ_SPECTRO : loadTrapez(hit); break;
      case FasterReader::Alias::RF_DATA        : loadRF    (hit); break;
      case FasterReader::Alias::CRRC4_SPECTRO  : loadCRRC4 (hit); break;
      case FasterReader::Alias::QDC_TDC_X1     : loadQDC<1>(hit); break;
      case FasterReader::Alias::QDC_TDC_X2     : loadQDC<2>(hit); break;
      case FasterReader::Alias::QDC_TDC_X3     : loadQDC<3>(hit); break;
      case FasterReader::Alias::QDC_TDC_X4     : loadQDC<4>(hit); break;
      case FasterReader::Alias::EOF_FASTER     : return false; // End of file
      default: error("Unkown alias", static_cast<int>(m_header.alias()));
    }
    loadLabel(hit); 
    loadTimestamp(hit);
    return true;
  }
  
  constexpr bool loadNextHit() noexcept
  {
    if (m_nb_hits_tot_max < m_nb_hits_tot ) return false;
    switch(FasterReader::readNextHit())
    {
      case FasterReader::Alias::TRAPEZ_SPECTRO : loadTrapez(m_hit); break;
      case FasterReader::Alias::RF_DATA        : loadRF    (m_hit); break;
      case FasterReader::Alias::CRRC4_SPECTRO  : loadCRRC4 (m_hit); break;
      case FasterReader::Alias::QDC_TDC_X1     : loadQDC<1>(m_hit); break;
      case FasterReader::Alias::QDC_TDC_X2     : loadQDC<2>(m_hit); break;
      case FasterReader::Alias::QDC_TDC_X3     : loadQDC<3>(m_hit); break;
      case FasterReader::Alias::QDC_TDC_X4     : loadQDC<4>(m_hit); break;
      case FasterReader::Alias::EOF_FASTER     : return false; // End of file
      default: error("Unkown alias", static_cast<int>(m_header.alias()));
    }
    loadLabel(m_hit); 
    loadTimestamp(m_hit);
    return true;
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
    // print();
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
    if (!m_open) return false;
    if (m_nb_hits_max <= m_hits.size()) return false; // Do not treat the file if the maximum number of hits is already reached
    if (m_nb_hits_max < Colib::big<ulonglong>()) m_hits.reserve(m_nb_hits_max);
    while(loadNextRootHit())
    {
      printLoadingHitsProgress();
      ++m_nb_hits_tot;
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
  #if defined (CoMT) && !defined(DEV) 
    printsln("Worker waiting in line !");
    lock_mutex lock(read_mutex);
  #endif // CoMT but no DEV 

    m_filesNb = filenames.size();
    if (m_filesNb <= m_file_id ) return false;

    size_t const toLoad = std::min(maxFiles, m_filesNb - m_file_id);

    for (size_t i = 0; i < toLoad; ++i) loadDatafile(filenames[m_file_id]);

    return 0 < toLoad;
  }

  auto & data() {return m_hits;}
  auto const & sortedIDs() const {return m_sortedIDs;}

  void clearFull() noexcept override
  {
    RootInterface::clearFull();
    FasterReader::fullClear();
    m_nb_hits_tot = {};
  }

  constexpr inline void printLoadingHitsProgress() const noexcept
  {
    if (m_nb_hits_tot % 1_Mi == 0)
    {
      if (m_nb_hits_max < Colib::big<ulonglong>()) 
        printsln("File", Colib::getShortname(m_filename), m_file_id, "/", m_filesNb, Colib::nicer_double(m_nb_hits_tot, 1), 
          Colib::nicer_seconds(m_hits.back().getTimestamp_s()), Colib::nicer_double((100.*m_nb_hits_tot)/m_nb_hits_max, 1), "%");
      else
        printsln("File", Colib::getShortname(m_filename), m_file_id, "/", m_filesNb, Colib::nicer_double(m_nb_hits_tot, 1), Colib::nicer_seconds(m_hits.back().getTimestamp_s()));
    }
  }

  // --------------- //
  // Data Operations //
  // --------------- //

 
  // Options setters:
  void setMaxHits       (ulonglong    max         ) noexcept {m_nb_hits_max       = max       ;}
  void setTotMaxHits    (ulonglong    max         ) noexcept {m_nb_hits_tot_max   = max       ;}
  // void setHitTrigger    (HitTrigger   trigger     ) noexcept {m_trigger          = trigger    ;}

  void setCalibration(Calibration && calib) noexcept
  {
    if (0 < calib.size())
    {
      m_calibrate = true;
      m_calib = std::move(calib);
    }
  }

  constexpr auto const & getMaxHits() const noexcept {return m_nb_hits_max;}

  constexpr auto const & getHit() const noexcept {return m_hit;}
  constexpr auto       & getHit()       noexcept {return m_hit;}

private:

  inline constexpr void loadLabel(Hit & hit) noexcept
  {
    hit.label = FasterReader::m_header.label;
  }

  inline constexpr void loadTimestamp(Hit & hit) noexcept {hit.stamp = FasterReader::m_timestamp;}
  
  inline constexpr void loadTrapez(Hit & hit) noexcept
  {
    hit.adc    = FasterReader::m_trapez_spectro.measure;
    hit.pileup = (FasterReader::m_trapez_spectro.pileup == 1 || FasterReader::m_trapez_spectro.saturated == 1 || FasterReader::m_trapez_spectro.sat_cpz == 1);
  }

  inline constexpr void loadRF(Hit & hit) noexcept
  {
    hit.adc = static_cast<ADC>(FasterReader::m_rf_data.period_ps());
    hit.pileup = FasterReader::m_rf_data.saturated; 
  }

  inline constexpr void loadCRRC4(Hit & hit) noexcept
  {
    hit.adc = FasterReader::m_crrc4_spectro.measure;
    hit.pileup = (FasterReader::m_crrc4_spectro.pileup == 1 || FasterReader::m_crrc4_spectro.saturated == 1);
  }

  template<int n>
  inline constexpr void loadQDC(Hit & hit) noexcept
  {
    auto qdc_data = FasterReader::template getQDC<n>();
    hit.pileup = false;
    if constexpr (n>0)  
    {
      hit.adc  = qdc_data[0].measure; hit.pileup |= qdc_data[0].saturated;
      if constexpr (n>1)  
      {
        hit.qdc2 = qdc_data[1].measure; hit.pileup |= qdc_data[1].saturated;
        if constexpr (n>2)  {hit.qdc3 = qdc_data[2].measure; hit.pileup |= qdc_data[2].saturated;}
      }
    }
  }

  // I/O :
  Hit m_hit;
  size_t m_filesNb = 1;
  ulonglong m_nb_hits_max = Colib::big<ulonglong>();
  ulonglong m_nb_hits_tot_max = Colib::big<ulonglong>();
  size_t m_nb_hits_tot = {}; // The total number of hits loaded since the first file 
#if defined (MULTITHREAD) && !defined(DEV) 
  inline static std::mutex read_mutex;
#endif // CoMT but no DEV 

};

// #endif //FASTERROOTINTERFACEV2_HPP