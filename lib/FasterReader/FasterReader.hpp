#ifndef FASTERREADERV2_HPP
#define FASTERREADERV2_HPP

#include "zlib.h"
#include "../libCo.hpp"
#include "../Classes/Timeshifts.hpp"

struct FasterReaderConfig {
    static constexpr int __bufferSize__ = 4096; // Size of the buffer dta is loaded into. Increase if some hits may be bigger than this (grouped mode i.e.)
    static constexpr int __clockByteSize__ = 6; // Number of bytes coding for the clock. Should be adjusted only in case of hardware change.
    static constexpr int __clockByte_ps__ = 2000; // Length of one byte of timestamp in ps.
};

template <class Config = FasterReaderConfig>
class FasterReader_t
{
public :
  using clock_t = uint64_t;
  using time_t  = double long;

  static const unsigned char FASTER_MAGIC = 0xAA;

  enum class Alias : unsigned char
  {
      EOF_FASTER      =   0,
      GROUP           =  10,
      RF_DATA         =  19,
      RF_COUNTER      =  20,
      GROUP_COUNTER   =  30,
      QDC_COUNTER     =  50,
      CRRC4_SPECTRO   =  61,
      TRAPEZ_SPECTRO  =  62,
      SPECTRO_COUNTER =  70,
      QDC_TDC_X1      = 141,
      QDC_TDC_X2      = 142,
      QDC_TDC_X3      = 143,
      QDC_TDC_X4      = 144,
  };

	FasterReader_t() noexcept = default;
	~FasterReader_t() noexcept {close();}
  FasterReader_t(std::string const & filename) noexcept
  {
    open(filename);
  }

	virtual bool open(std::string const & filename) noexcept
	{
    if (filename.back() == '/') Colib::throw_error("In FasterRootV2::open("+filename+"): not a file !");
    m_cursor = 0;
    m_filename = filename;
    fs::path p(m_filename);
    if(!fs::is_regular_file(p)) 
    {
      error(filename + " is not a file !");
      return false;
    }
		if (!(m_datafile = gzopen(filename.c_str(), "rb")))
		{
			error("Couldn't find "+ filename + " !");
			return false;
		}
		return (m_open = true);
	}

  constexpr inline Alias readNextHit() noexcept
  {
    while(loadData()) if (readData()) return m_header.alias();
    return Alias::EOF_FASTER;
  }

	/// @brief Loads the data in the buffer
	inline bool loadData() noexcept
	{
    if (!m_open) error(m_filename, "not open");
    ++m_cursor;
		int n = gzread(m_datafile, &m_header, sizeof(m_header)) ; // Try to read the next header
		if (gzeof(m_datafile)) return false; // If at the end of the file, the previous gzread failed and gzeof is true. Return false.
    if (n != sizeof(m_header))   error("in FasterReader::readNext("+m_filename+") : header reading " + gzlibError(n));
		if (m_header.buff_size == 0) return true; // No data to read, according to fasterac it's not a big deal (maybe for counters ?)
    n = gzread(m_datafile, &m_buffer, m_header.buff_size); // Loads the data into the buffer
    if (n != m_header.buff_size) error("in FasterReader::readNext("+m_filename+") : data dumping " + gzlibError(n));
    if (m_header.magic != FASTER_MAGIC) error("in FasterReader::readNext("+m_filename+") : magic check failed !!"+std::to_string(m_header.magic)+ "!="+std::to_string(FASTER_MAGIC));
    return true;
	}


  /// @brief EXPERIMENTAL skipHits may not behave as expected
  inline bool skipHits(size_t nbHits) noexcept
  {
    for(size_t hit_i = 0; hit_i<nbHits; ++hit_i)
      if (!loadData()) return false; // If end of file reached return false
    m_cursor += nbHits;
    return true;
  }

  constexpr inline bool readData() noexcept
  {
    checkGroups          (); // If the faster files have been written in group mode (i.e. event building), this class can't read it (yet)
    extractTimestamp     (); // Load the raw timestamp - the high resolution timestamp is added in switch_aliases
    return switch_aliases(); // Read the data based on the type_alias. Returns false if one is not handled.
	}

  constexpr inline void applyTimeshifts() noexcept
  {
    if (static_cast<size_t>(m_header.label) <= m_tshifts.size()) m_timestamp += m_tshifts[m_header.label];
  }

  constexpr inline auto const & getTimestamp() const noexcept {return m_timestamp;}
  
  constexpr inline auto const & getHeader() const noexcept {return m_header;}

  constexpr inline clock_t getClock() const noexcept
  {
    clock_t clock = 0;
    memcpy(&clock, m_header.clock, Config::__clockByteSize__); 
    return clock * Config::__clockByte_ps__/256.;
  }
  
  constexpr inline time_t getClock_ps() const noexcept
  {
    return static_cast<time_t>(getClock());
  }
  
  constexpr inline time_t getClock_ns() const noexcept
  {
    return static_cast<time_t>(getClock()) * 1e-03;
  }

  constexpr inline time_t getClock_ms() const noexcept
  {
    return static_cast<time_t>(getClock()) * 1e-06;
  }

  constexpr inline time_t getClock_s() const noexcept
  {
    return static_cast<time_t>(getClock()) * 1e-12;
  }

  void Print() const noexcept
  {
    std::cout << std::setw(10) << m_cursor       <<    " ";
    std::cout << std::setw( 3) << m_header.label <<    " ";
    std::cout << std::setw(15) << getClock()     << " ps ";
    std::cout << std::setw(15) << m_timestamp    << " ps ";
    switch(m_header.alias())
    {
      case Alias::TRAPEZ_SPECTRO : std::cout << m_trapez_spectro            ; break;
      case Alias::RF_DATA        : std::cout << m_rf_data                   ; break;
      case Alias::CRRC4_SPECTRO  : std::cout << m_crrc4_spectro             ; break;
      case Alias::QDC_TDC_X1     : std::cout << m_qdc_data.template get<1>(); break;
      case Alias::QDC_TDC_X2     : std::cout << m_qdc_data.template get<2>(); break;
      case Alias::QDC_TDC_X3     : std::cout << m_qdc_data.template get<3>(); break;
      case Alias::QDC_TDC_X4     : std::cout << m_qdc_data.template get<4>(); break;
      default: std::cerr << "Unkown data type";
    } 
    std::cout << std::endl;
  }

  auto const & getCursor() const noexcept {return m_cursor;}
  bool setCursor(size_t cursor) noexcept{return skipHits(m_cursor-cursor);} // Returns false if end of file reached

	inline void close() noexcept
	{
    if (m_open) gzclose(m_datafile);
    m_cursor = 0;
    m_open = false;
	}

  void loadTimeshifts (std::string const & tsFile) {m_tshifts.load(tsFile)       ;}
  void loadTimeshifts (Timeshifts  const & tsData) {m_tshifts = tsData           ;}
  void loadTimeshifts (Timeshifts       && tsData) {m_tshifts = std::move(tsData);}

  //////////////////////
  // -- Structures -- //
  //////////////////////

  struct DataStructure{}; // Not used, for extra readability only

  struct TrapezSpectro : public DataStructure
  {//  from Trapezoidal_Spectro_Caras or Trapezoidal_Spectro_Mosahr
    signed   measure   : 23;
    signed   tdc       :  6;
    unsigned pileup    :  1;
    unsigned saturated :  1;
    unsigned sat_cpz   :  1;

    inline constexpr auto tdc_ps() const noexcept {return tdc * tdc_clock_LSB_6bits;}
    
    friend std::ostream & operator << (std::ostream & out, TrapezSpectro const & data)
    {
      out << std::setw(6) << data.tdc_ps() << " ps ";
      out << std::setw(10) << data.measure << " adc";
      if (data.pileup )    out << " pileup";
      if (data.saturated ) out << " saturated";
      if (data.sat_cpz )   out << " sat_cpz";
      return out;
    }
  };

  struct RFData : public DataStructure
  {
    unsigned period    : 31;
    unsigned saturated :  1;
      signed trig_dt   : 32;
      signed pll_dt    : 32;

    static constexpr inline double period_to_ns = 1.0 / pow (2.0, 19);
    inline constexpr double period_ns() const noexcept {return period * period_to_ns;}
    inline constexpr double period_ps() const noexcept {return period_ns() * 1000.;}

    friend std::ostream & operator << (std::ostream & out, RFData const & data)
    {
      out << std::setw(10) << data.period_ns()*1000. << " ns period";
      if (data.saturated ) out << " saturated";
      return out;
    }
  };

  struct CRRC4Spectro : public DataStructure
  { //  from CRRC4_Spectro_Caras or CRRC4_Spectro_Mosahr
    unsigned pad1      : 10;
    signed   delta_t   : 16; // position of max relative to trigger (ns)
    unsigned pad2      :  6;
    signed   measure   : 22;
    unsigned pad3      :  8;
    unsigned saturated :  1;
    unsigned pileup    :  1;

    inline constexpr int deltaT_ps() const noexcept {return delta_t * 8000;}
    
    friend std::ostream & operator << (std::ostream & out, CRRC4Spectro const & data) noexcept
    {
      out << std::setw(10) << data.measure << " adc";
       if (data.saturated)  printSaturated(out);
      return out;
    }
  };

  struct QDCSpectro 
  { 
    signed   measure   : 31;
    unsigned saturated :  1;
  };

  template<int n>
  struct QDCSpectro_xn : public DataStructure
  { 
    QDCSpectro qdc[n];
    int32_t tdc = 0;

    static constexpr inline double adc_mv = 0.036468506 * 2.0;
    
    inline constexpr auto qdc_mv (int qdc_i) const noexcept { return qdc_i * adc_mv; }
    inline constexpr auto tdc_ps ()          const noexcept { return tdc * tdc_clock_LSB_8bits; }
    
    inline constexpr QDCSpectro operator[] (int i) const noexcept { return qdc[i]; }

    friend std::ostream & operator << (std::ostream & out, QDCSpectro_xn<n> const & data)
    {
      out << std::setw(6) << data.tdc_ps() << " ps ";
      bool saturated = false;
      for (int i = 0; i < n; ++i) 
      {
        out << std::setw(10) << data[i].measure << " qdc" << i+1;
        saturated |= data[i].saturated;
      }
      if (saturated) out << " saturated";
      return out;
    }
  };

  struct QDCSpectro_all
  {
    std::tuple<
        QDCSpectro_xn<1>, 
        QDCSpectro_xn<2>, 
        QDCSpectro_xn<3>, 
        QDCSpectro_xn<4>
    > data;

    template <int n>
    inline auto& get() noexcept
    {
      static_assert(n >= 1 && n <= 4, "Invalid QDC index");
      return std::get<n-1>(data); // std::get uses 0-based indexing
    }

    template <int n>
    inline constexpr auto const & get() const noexcept
    {
      static_assert(n >= 1 && n <= 4, "Invalid QDC index");
      return std::get<n-1>(data);// std::get uses 0-based indexing
    }
  };
  
  TrapezSpectro    m_trapez_spectro;
  RFData           m_rf_data       ;
  CRRC4Spectro     m_crrc4_spectro ;
  QDCSpectro_all   m_qdc_data      ;
  
protected :

  constexpr inline void checkGroups() const noexcept
  {
    if (m_header.alias() == Alias::GROUP) error("Group mode is not handled in FasterReader (sry) !!!");
  }

  constexpr inline void extractTimestamp() noexcept
  {
    m_timestamp = 0;
    memcpy(&m_timestamp, m_header.clock, Config::__clockByteSize__); 
    m_timestamp *= Config::__clockByte_ps__;
    applyTimeshifts();
  }

  constexpr inline bool switch_aliases() noexcept
  {
    switch(m_header.alias())
    {
      case Alias::TRAPEZ_SPECTRO : return loadBufferTrapez();
      case Alias::RF_DATA        : return loadBufferRF    ();
      case Alias::CRRC4_SPECTRO  : return loadBufferCRRC4 ();
      case Alias::QDC_TDC_X1     : return loadBufferQDC<1>();
      case Alias::QDC_TDC_X2     : return loadBufferQDC<2>();
      case Alias::QDC_TDC_X3     : return loadBufferQDC<3>();
      case Alias::QDC_TDC_X4     : return loadBufferQDC<4>();
      default: return false;
    }
  }

  inline static constexpr std::string gzlibError(int err) noexcept// Should make an object to handle gzlib errors ?
  {
    if (err < 0) return "failed, gzlib error, verify file integrity with \"gunzip -t filename\", and check thread safety";
    else         return "incomplete read, got "+std::to_string(err)+" instead of "+std::to_string(sizeof(m_header));
  }

  inline bool loadBufferTrapez() noexcept
  {
    loadBuffer(m_trapez_spectro);
    m_timestamp += m_trapez_spectro.tdc * tdc_clock_LSB_6bits;
    return true;
  }

  template<int n> 
  constexpr inline auto & getQDC() noexcept {return m_qdc_data.template get<n>();}

  template<int n> constexpr inline bool loadBufferQDC() noexcept
  {
    auto & qdc_data = getQDC<n>();
    loadBuffer(qdc_data);
    m_timestamp += qdc_data.tdc * tdc_clock_LSB_8bits;
    return true;
  }

  constexpr bool loadBufferRF() noexcept
  { // Maybe to correct, I'm not using pll_dt ...
    loadBuffer(m_rf_data);
    m_timestamp += m_rf_data.trig_dt * tdc_clock_LSB_8bits; 
    return true;
  }

  constexpr bool loadBufferCRRC4() noexcept
  {
    loadBuffer(m_crrc4_spectro);
    // There is no tdc in m_crrc4_spectro
    return true;
  }

  static constexpr std::ostream & printSaturated(std::ostream & out) noexcept { out << Colib::Color::RED << " saturated" << Colib::Color::RESET; return out;}

  static constexpr long double tdc_clock_LSB_8bits = Config::__clockByte_ps__ / 256; //  sample length + 8 bits tdc -> lsb = 2000 / 2^8
  static constexpr long double tdc_clock_LSB_6bits = Config::__clockByte_ps__ / 64 ; //  sample length + 6 bits tdc -> lsb = 2000 / 2^6

  struct Header {
    unsigned char  type_alias;
    unsigned char  magic     ;
    unsigned char  clock [Config::__clockByteSize__];
    unsigned short label     ;
    unsigned short buff_size ;
    inline constexpr Alias alias() const noexcept {return static_cast<Alias>(type_alias);}
  };

  unsigned long long m_timestamp;

	std::string m_filename;
	gzFile m_datafile;
  bool m_open = false;
  size_t m_cursor = 0;

	Header m_header;
  char m_buffer[Config::__bufferSize__];

private:

  template<class T>
  constexpr bool loadBuffer(T & structure) noexcept {return static_cast<bool>(memcpy(&structure, m_buffer, sizeof(T)));}
  
  Timeshifts  m_tshifts;
};

using FasterReader = FasterReader_t<>;

#endif //FASTERREADERV2_HPP



  // { // Content of one QDC
  //   signed   measure   : 31;
  //   unsigned saturated :  1;
  // };

  // template<int n>
  // struct QDCSpectro_xn : public DataStructure
  // { // Creates a QDC_xn with n the number of gates
  //   QDCSpectro qdc[n];
  //   int32_t tdc = 0;

  //   static constexpr double adc_mv = 0.036468506 * 2.0;//   CARAS VOLTAGE  LSB : 2*2390 / 2^17 mV SAMPLING  : 2.0 ns
  //   inline constexpr auto         qdc_mv     (int qdc_i)     const noexcept {return qdc_i * adc_mv;}
  //   inline constexpr auto         tdc_ps     ()              const noexcept {return tdc * tdc_clock_LSB_8bits;}
  //   inline constexpr auto const & operator[] (int const & i) const noexcept {return qdc[i];}

  //   friend std::ostream & operator << (std::ostream & out, QDCSpectro_xn<n> const & data)
  //   {
  //     out << std::setw(6) << data.tdc_ps() << " ps ";
  //     bool saturated = false;
  //     for (int i = 0; i<n; ++i) 
  //     {
  //       out << std::setw(10) << data[i].measure << " qdc" << i+1;
  //       saturated |= data[i].saturated;
  //     }
  //     if (saturated) out << " saturated";
  //     return out;
  //   }
  // };

  // struct QDCSpectro_all
  // {// Creates 4 QDC_xn with n from 1 to 4.
  //   QDCSpectro_xn<1> qdc_spectro_x1;
  //   QDCSpectro_xn<2> qdc_spectro_x2;
  //   QDCSpectro_xn<3> qdc_spectro_x3;
  //   QDCSpectro_xn<4> qdc_spectro_x4;

  //   template <int n>
  //   inline QDCSpectro_xn<n> & get() noexcept
  //   {
  //          if constexpr (n == 1) return qdc_spectro_x1;
  //     else if constexpr (n == 2) return qdc_spectro_x2;
  //     else if constexpr (n == 3) return qdc_spectro_x3;
  //     else if constexpr (n == 4) return qdc_spectro_x4;
  //     else static_assert(n >= 1 && n <= 4, "Invalid QDC index"); // To check
  //   }

  //   template <int n>
  //   inline constexpr QDCSpectro_xn<n> const & get() const noexcept
  //   {
  //          if constexpr (n == 1) return qdc_spectro_x1;
  //     else if constexpr (n == 2) return qdc_spectro_x2;
  //     else if constexpr (n == 3) return qdc_spectro_x3;
  //     else if constexpr (n == 4) return qdc_spectro_x4;
  //     else static_assert(n >= 1 && n <= 4, "Invalid QDC index"); // To check
  //   }
  // };
