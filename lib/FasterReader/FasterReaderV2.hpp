#ifndef FASTERREADERV2_HPP
#define FASTERREADERV2_HPP

#include "zlib.h"
#include "../libCo.hpp"

struct FasterReaderConfig {
    static constexpr int __bufferSize__ = 4096;
    static constexpr int __clockByteSize__ = 6;
    static constexpr int __tick_ps__ = 2000;
};

template <class Config = FasterReaderConfig>
class FasterReaderV2_t
{

public :
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
	~FasterReaderV2_t() {close();};
	FasterReaderV2_t(std::string const & filename) noexcept
	{
		open(filename);
	}

	virtual bool open(std::string const & filename) noexcept
	{
    m_filename = filename;
		m_datafile = gzopen(filename.c_str(), "rb");
		if (!m_datafile) 
		{
			error("Couldn't find "+ filename + " !");
			return false;
		}
		return (m_open = true);
	}

  virtual Alias readNextHit()
  {
    while(loadData()) if (readData()) return m_header.alias();
    return Alias::EOF_FASTER;
  }

	virtual bool loadData()
	{
    ++m_cursor;
		if (gzeof(m_datafile)) return false; // At the end of the file, return false;
		int n = gzread(m_datafile, &m_header, sizeof(m_header)) ;
    if (n != sizeof(m_header))   Colib::throw_error("in FasterReaderV2::readNext() : header reading " + gzlibError(n));
		if (m_header.buff_size == 0) return true; // No data to read, according to fasterac it's not a big deal (maybe for counters ?)
    n = gzread(m_datafile, &m_buffer, m_header.buff_size);
    if (n != m_header.buff_size) Colib::throw_error("in FasterReaderV2::readNext() : data dumping " + gzlibError(n));
    if (m_header.magic != FASTER_MAGIC) Colib::throw_error("in FasterReaderV2::readNext() : magic check failed !!"+std::to_string(m_header.magic)+ "!="+std::to_string(FASTER_MAGIC));
    return true;
	}

  virtual bool readData()
  {
    checkGroups          (); // If the faster files have been written in group mode (i.e. event building), this functionnality is not available
    extractTimestamp     (); // Load the raw timestamp - the high resolution timestamp is added in switch_aliases
    return switch_aliases(); // Read the data based on the type_alias. Returns false if one is not handled.
	}

  auto const & getTimestamp() const {return m_timestamp;}
  
  auto const & getHeader() const {return m_header;}

  unsigned long long getClock() const
  {
    unsigned long long clock = 0;
    memcpy(&clock, m_header.clock, Config::__clockByteSize__); 
    return clock * Config::__tick_ps__;
  }
  
  double long getClock_ns() const
  {
    unsigned long long clock = 0;
    memcpy(&clock, m_header.clock, Config::__clockByteSize__); 
    return clock * Config::__tick_ps__ * 1e-3;
  }

  double long getClock_s() const
  {
    unsigned long long clock = 0;
    memcpy(&clock, m_header.clock, Config::__clockByteSize__); 
    return clock * Config::__tick_ps__ * 1e-12;
  }

  void print() const
  {
    std::cout << std::setw(10) << m_cursor       << " ";
    std::cout << std::setw( 3) << m_header.label << " ";
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
      default: ;
    } 
    std::cout << std::endl;
  }

  auto const & getCursor() const noexcept {return m_cursor;}

	virtual void close()
	{
    if (m_open) gzclose(m_datafile);
	}

  struct TrapezSpectro 
  {//  from Trapezoidal_Spectro_Caras or Trapezoidal_Spectro_Mosahr
    signed   measure   : 23;
    signed   tdc       :  6;
    unsigned pileup    :  1;
    unsigned saturated :  1;
    unsigned sat_cpz   :  1;

    constexpr auto tdc_ps() const {return tdc * tdc_clock_LSB_6bits;}
    
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

  struct RFData
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

  struct CRRC4Spectro
  { //  from CRRC4_Spectro_Caras or CRRC4_Spectro_Mosahr
    unsigned pad1      : 10;
    signed   delta_t   : 16; // position of max relative to trigger (ns)
    unsigned pad2      :  6;
    signed   measure   : 22;
    unsigned pad3      :  8;
    unsigned saturated :  1;
    unsigned pileup    :  1;

    int deltaT_ps(){return delta_t * 8000;}
    
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
  struct QDCSpectro_xn
  { // Creates a QDC_xn with n the number of gates
    QDCSpectro qdc[n];
    int32_t tdc = 0;

    constexpr auto tdc_ps() const {return tdc * tdc_clock_LSB_8bits;}
    auto const & operator[] (int const & i) const {return qdc[i];}

    friend std::ostream & operator << (std::ostream & out, QDCSpectro_xn<n> const & data)
    {
      out << std::setw(6) << data.tdc_ps() << " ps ";
      bool saturated = false;
      for (int i = 0; i<n; ++i) 
      {
        out << std::setw(10) << data[i].measure << " qdc" << i+1;
        saturated |= data[i].saturated;
      }
      // if (saturated) printSaturated(out);
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
    QDCSpectro_xn<n> & get()
    {
           if constexpr (n == 1) return qdc_spectro_x1;
      else if constexpr (n == 2) return qdc_spectro_x2;
      else if constexpr (n == 3) return qdc_spectro_x3;
      else if constexpr (n == 4) return qdc_spectro_x4;
      else static_assert(n >= 1 && n <= 4, "Invalid QDC index");
    }

    template <int n>
    QDCSpectro_xn<n> const & get() const
    {
           if constexpr (n == 1) return qdc_spectro_x1;
      else if constexpr (n == 2) return qdc_spectro_x2;
      else if constexpr (n == 3) return qdc_spectro_x3;
      else if constexpr (n == 4) return qdc_spectro_x4;
      else static_assert(n >= 1 && n <= 4, "Invalid QDC index");
    }
  };

  TrapezSpectro    m_trapez_spectro;
  RFData           m_rf_data       ;
  CRRC4Spectro     m_crrc4_spectro ;
  QDCSpectro_all   m_qdc_data      ;
  
protected :

  virtual void checkGroups()
  {
    if (m_header.alias() == Alias::GROUP) Colib::throw_error("Group mode is not handled in FasterReaderV2 (sry) !!!");
  }

  void extractTimestamp() // should be private
  {
    m_timestamp = 0; // TEST !!
    memcpy(&m_timestamp, m_header.clock, Config::__clockByteSize__); 
    m_timestamp *= Config::__tick_ps__;
  }

  bool switch_aliases()
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

  static constexpr std::string gzlibError(int err)  noexcept// Should make an object to handle gzlib errors
  {
    if (err < 0) return "failed, gzlib error, verify file integrity with \"gunzip -t filename\", and check thread safety";
    else         return "incomplete read, got "+std::to_string(err)+" instead of "+std::to_string(sizeof(m_header));
  }

  template<class T>
  constexpr bool loadBuffer(T & structure) noexcept {return bool_cast(memcpy(&structure, m_buffer, sizeof(T)));} // Helper function (should be private ?)

  bool loadBufferTrapez () 
  {
    loadBuffer(m_trapez_spectro);
    m_timestamp += m_trapez_spectro.tdc * tdc_clock_LSB_6bits;
    return true;
  }

  template<int n> auto & getQDC(){return m_qdc_data.template get<n>();}
  template<int n>
  bool loadBufferQDC() 
  {
    auto & qdc_data = getQDC<n>();
    loadBuffer(qdc_data);
    m_timestamp += qdc_data.tdc * tdc_clock_LSB_8bits;
    return true;
  }

  bool loadBufferRF() 
  { // Maybe to correct, I'm not using pll_dt ...
    loadBuffer(m_rf_data);
    m_timestamp += m_rf_data.trig_dt * tdc_clock_LSB_8bits; 
    return true;
  }

  bool loadBufferCRRC4() 
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
    auto alias() const noexcept {return static_cast<Alias>(type_alias);}
  };

  unsigned long long m_timestamp;

  size_t m_cursor = 0;

	std::string m_filename;
	gzFile m_datafile;
  bool m_open = false;

	Header m_header;
  char m_buffer[Config::__bufferSize__];

  // Hit *m_hit = nullptr;
};

using FasterReaderV2 = FasterReaderV2_t<>;

#endif //FASTERREADERV2_HPP