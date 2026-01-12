#ifndef FASTERREADERV2_HPP
#define FASTERREADERV2_HPP

#include "zlib.h"
#include "../libCo.hpp"
#include "../Classes/Timeshifts.hpp"

struct FasterReaderConfig {
    static constexpr int __bufferSize__ = 4096;
    static constexpr int __clockByteSize__ = 6;
    static constexpr int __tick_ps__ = 2000;
};

template <class Config = FasterReaderConfig>
class FasterReaderV2_t
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

	FasterReaderV2_t() noexcept = default;
	~FasterReaderV2_t() {close();}
  FasterReaderV2_t(std::string const & filename) noexcept
  {
    open(filename);
  }

	virtual bool open(std::string const & filename) noexcept
	{
    m_cursor = 0;
    m_filename = filename;
		m_datafile = gzopen(filename.c_str(), "rb");
		if (!m_datafile)
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

	inline bool loadData() noexcept
	{
    ++m_cursor;
		int n = gzread(m_datafile, &m_header, sizeof(m_header)) ; // Try to read the next header
		if (gzeof(m_datafile)) return false; // If at the end of the file, the previous gzread failed and gzeof is true. Return false.
    if (n != sizeof(m_header))   error("in FasterReaderV2::readNext() : header reading " + gzlibError(n));
		if (m_header.buff_size == 0) return true; // No data to read, according to fasterac it's not a big deal (maybe for counters ?)
    n = gzread(m_datafile, &m_buffer, m_header.buff_size);
    if (n != m_header.buff_size) error("in FasterReaderV2::readNext() : data dumping " + gzlibError(n));
    if (m_header.magic != FASTER_MAGIC) error("in FasterReaderV2::readNext() : magic check failed !!"+std::to_string(m_header.magic)+ "!="+std::to_string(FASTER_MAGIC));
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
    if (static_cast<size_t>(m_header.label+1) < m_timeshifts.size()) m_timestamp += m_timeshifts[m_header.label];
  }

  constexpr inline auto const & getTimestamp() const noexcept {return m_timestamp;}
  
  constexpr inline auto const & getHeader() const noexcept {return m_header;}

  constexpr inline clock_t getClock() const noexcept
  {
    clock_t clock = 0;
    memcpy(&clock, m_header.clock, Config::__clockByteSize__); 
    return clock * Config::__tick_ps__;
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
      default: std::cerr << "unkown data type";
    } 
    std::cout << std::endl;
  }

  auto const & getCursor() const noexcept {return m_cursor;}

	inline void close() noexcept
	{
    if (m_open) gzclose(m_datafile);
    m_cursor = 0;
    m_open = false;
	}

  void loadTimeshifts(std::string tsFile) {m_timeshifts.load(tsFile);}

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

    friend std::ostream & operator << (std::ostream & out, RFData const & data)
    {
      out << std::setw(10) << data.period << " period";
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
    
    friend std::ostream & operator << (std::ostream & out, CRRC4Spectro const & data)
    {
      out << std::setw(10) << data.measure << " adc";
       if (data.saturated)  printSaturated(out);
      return out;
    }
    
  };
  
  struct QDCSpectro 
  { // Content of one QDC
    signed   measure   : 31;
    unsigned saturated :  1;
  };

  template<int n>
  struct QDCSpectro_xn : public DataStructure
  { // Creates a QDC_xn with n the number of gates
    QDCSpectro qdc[n];
    int32_t tdc = 0;

    inline constexpr auto         tdc_ps     ()              const noexcept {return tdc * tdc_clock_LSB_8bits;}
    inline constexpr auto const & operator[] (int const & i) const noexcept {return qdc[i];}

    friend std::ostream & operator << (std::ostream & out, QDCSpectro_xn<n> const & data)
    {
      out << std::setw(6) << data.tdc_ps() << " ps ";
      bool saturated = false;
      for (int i = 0; i<n; ++i) 
      {
        out << std::setw(10) << data[i].measure << " qdc" << i+1;
        saturated |= data[i].saturated;
      }
      if (saturated) out << " saturated";
      return out;
    }
  };

  struct QDCSpectro_all
  {// Creates 4 QDC_xn with n from 1 to 4.
    QDCSpectro_xn<1> qdc_spectro_x1;
    QDCSpectro_xn<2> qdc_spectro_x2;
    QDCSpectro_xn<3> qdc_spectro_x3;
    QDCSpectro_xn<4> qdc_spectro_x4;

    template <int n>
    inline QDCSpectro_xn<n> & get() noexcept
    {
           if constexpr (n == 1) return qdc_spectro_x1;
      else if constexpr (n == 2) return qdc_spectro_x2;
      else if constexpr (n == 3) return qdc_spectro_x3;
      else if constexpr (n == 4) return qdc_spectro_x4;
      else static_assert(n >= 1 && n <= 4, "Invalid QDC index"); // To check
    }

    template <int n>
    inline constexpr QDCSpectro_xn<n> const & get() const noexcept
    {
           if constexpr (n == 1) return qdc_spectro_x1;
      else if constexpr (n == 2) return qdc_spectro_x2;
      else if constexpr (n == 3) return qdc_spectro_x3;
      else if constexpr (n == 4) return qdc_spectro_x4;
      else static_assert(n >= 1 && n <= 4, "Invalid QDC index"); // To check
    }
  };

  TrapezSpectro    m_trapez_spectro;
  RFData           m_rf_data       ;
  CRRC4Spectro     m_crrc4_spectro ;
  QDCSpectro_all   m_qdc_data      ;
  
protected :

  constexpr inline void checkGroups() const noexcept
  {
    if (m_header.alias() == Alias::GROUP) error("Group mode is not handled in FasterReaderV2 (sry) !!!");
  }

  constexpr inline void extractTimestamp() noexcept
  {
    m_timestamp = 0;
    memcpy(&m_timestamp, m_header.clock, Config::__clockByteSize__); 
    m_timestamp *= Config::__tick_ps__;
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
  constexpr inline auto & getQDC() {return m_qdc_data.template get<n>();}

  template<int n> inline bool loadBufferQDC() 
  {
    auto & qdc_data = getQDC<n>();
    loadBuffer(qdc_data);
    m_timestamp += qdc_data.tdc * tdc_clock_LSB_8bits;
    return true;
  }

  bool loadBufferRF() noexcept
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

  static constexpr std::ostream & printSaturated(std::ostream & out) { out << Colib::Color::RED << " saturated" << Colib::Color::RESET; return out;}

  static constexpr long double tdc_clock_LSB_8bits = Config::__tick_ps__ / 256; //  sample length + 8 bits tdc -> lsb = 2000 / 2^8
  static constexpr long double tdc_clock_LSB_6bits = Config::__tick_ps__ / 64 ; //  sample length + 6 bits tdc -> lsb = 2000 / 2^6

  struct Header {
    // unsigned char  type_alias;
    unsigned char  type_alias; // unsigned char
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
  constexpr bool loadBuffer(T & structure) noexcept {return static_cast<bool>(memcpy(&structure, m_buffer, sizeof(T)));} // Helper function (should be private ?)
  
  Timeshifts m_timeshifts;
};

using FasterReaderV2 = FasterReaderV2_t<>;

#endif //FASTERREADERV2_HPP