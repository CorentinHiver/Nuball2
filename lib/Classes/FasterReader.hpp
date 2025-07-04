#ifndef FASTERREADER_HPP
#define FASTERREADER_HPP

#include "Hit.hpp"

// ******* fasterac includes ******* //
#include "fasterac/adc.h"
#include "fasterac/group.h"
#include "fasterac/qdc.h"
#include "fasterac/rf.h"
#include "fasterac/utils.h"

#ifdef COMULTITHREADING
  std::mutex fasterReaderMutex;
#endif //COMULTITHREADING

void fasterReaderLockMutex()
{
#ifdef COMULTITHREADING
  fasterReaderMutex.lock();
#endif //COMULTITHREADING
}

void fasterReaderUnlockMutex()
{
#ifdef COMULTITHREADING
  fasterReaderMutex.unlock();
#endif //COMULTITHREADING
}

/** 
 * @class FasterReader
 * @brief Class that to reads .fast data files
 * 
 * @details
 * This class is to be used in combination with the Hit struct
 * Here is the minimal code you can use to scan the files : 
 * 
 *      Hit hit;
 *      std::string filename;
 *      FasterReader reader(&hit, filename);
 *      
 *      while(reader.Read())
 *      {
 *        print(hit);
 *      }
 * 
 * This method deals with the following members of the Hit structure:(NOT UP TO DATE !!!)
 * 
 *        struct Hit
 *        {
 *          unsigned short label;  // The label number of the detector
 *          int            adc;    // The ADC/QDC value of the hit. For RF, contains the frequency.
 *          int            qdc2;   // The qdc2 value
 *          int            qdc3;   // The qdc2 value
 *          ULong64_t      stamp;  // Timestamp in ps 64 bit long
 *          bool           pileup; // Contains either pileup or saturated bit
 *          ....
 *        }
 *        They are used with
 * 
 * ---------- Define section ----------
 * 
 * Some options can be activated at compilation time. 
 * You have to #define them before including this library. e.g. : 
 * 
 *        ... other includes
 *        #define FASTER_GROUP
 *        #include <FasterReader.hpp>
 *        rest of the code....
 * 
 * or add them in the compilation line preceeded by -D (e.g. gcc -o ...... -D FASTER_GROUP ...)
 * 
 * -- QDC1MAX
 * 
 * By default the qdc2 field is handled. 
 * If no detector uses the qdc2 then declare QDC1MAX to gain some time (not sure if this is significant tho...)
 * 
 * -- QDC2MAX
 * 
 * Same as QDC1MAX but for the qdc3 field
 * 
 * -- FASTER_GROUP
 * 
 * If the data is grouped using a hardware trigger. From the user point of view, 
 * nothing changes but the execution speed. Simply #define FASTER_GROUP 
 * and then use the class as usual. 
 * 
 * The reading is done in two steps : first extracts all the hits of the group 
 * and put it in a vector, then each call of Read() moves in the group. 
 * No extra information is extracted : from outside of the class, everything goes as if there was no group.
 * 
 * @attention The pileup bit for CRRC4 is not handled
 * 
 */
class FasterReader
{
  public:
	///@brief Construct a new Faster Reader object
	FasterReader(Hit *_hit, std::string _filename) : m_hit(_hit), m_filename(_filename)
	{
		m_kReady = Initialise();
	}

	///@brief Destroy the Faster Reader object
	~FasterReader()
	{
		if (m_reader != NULL)
			faster_file_reader_close(m_reader);
	}

	///@brief Reset the cursor to the beginning of the document
	bool Reset();

	/**
   * @brief Main method. Extract the next hit from the data file and fills the Hit *m_hit object
   * @details
   * Hit hit;
   * 
   * FasterReader reader(&hit, filename);
   * 
   *      while(reader.Read())
   *      {
   *         // This hit is filled/updated at each iteration
   *         print(hit); 
   *      }
   * 
   * 
   * @return true if the end of the file is reached, false otherwise
   */
	bool Read();

	// ------ Setters ------ :
	///@brief \test Set the Hit object
	///\nNever tested, but should work !
	void setHit(Hit *hit) { m_hit = hit; }

	///@brief Set the number of hits to read inside each file
	static void setMaxHits(ulonglong maxHits)
	{
		s_maxHits = maxHits;
		printC(nicer_double(s_maxHits, 0), "hits to read in the file");
	}
	static auto getMaxHits() { return s_maxHits; }
	static void setVerbose(int i = 1) { m_verbose = i; }

	// ------ Getters ------ :
	///@brief \deprecated Get the current Hit
	Hit *getHit() const { return m_hit; }

	/// @brief If the initialization went badly then returns false
	bool const &isReady() const { return m_kReady; }

	/// @brief If the initialization went badly then returns false
	operator bool() const & { return m_kReady; }

	///@brief Get the name of the file being read
	auto const &filename() const { return m_filename; }

	///@brief Get the name of the file being read
	auto const &getFilename() const { return m_filename; }

	///@brief Returns the hits counter
	auto const &getCounter() const { return m_counter; }

  private:
	///@brief Setup the fasterac.h library to read the data
	bool Initialise();

	bool ReadSimple();
	bool ReadGroup();
	void ReadDataGroup(faster_data_p const & _data);
	bool ReadData(faster_data_p const & _data);
	bool switch_alias(uchar const & _alias, faster_data_p const & _data);

	bool InitialiseReader();

	static ulonglong s_maxHits;
	thread_local static int m_verbose;
	// ulonglong m_cursor  =  0;
	ulonglong m_counter = 0;

	Hit * m_hit = nullptr;
	Hit m_empty_hit;
	std::string m_filename = "";
	bool m_kReady = false;
#ifdef FASTER_GROUP
	bool m_write = false;
	bool m_inGroup = false;
#endif //FASTER_GROUP

	// Grouped data management :
	ushort               m_group_read_cursor = 0;
    ushort               m_group_write_cursor = 0;
	std::vector<Hit *>   m_hit_group_buffer;
	faster_file_reader_p m_reader = NULL;
	faster_data_p        m_data = NULL;

	void TreatTrapez(faster_data_p const & _data);
	void TreatCRRC4 (faster_data_p const & _data);
	void TreatQDC1  (faster_data_p const & _data);
	void TreatQDC2  (faster_data_p const & _data);
	void TreatQDC3  (faster_data_p const & _data);
	void TreatRF    (faster_data_p const & _data);

	std::unordered_map<std::string, bool> error_message;
};

ulonglong FasterReader::s_maxHits = -1;
thread_local int FasterReader::m_verbose = 1;

// ================== //
//   INITIALIZATION   //
// ================== //

bool inline FasterReader::Reset()
{
	if (m_reader != NULL)
		faster_file_reader_close(m_reader);
	m_counter = 0;
	m_group_read_cursor = 0;
	m_group_write_cursor = 0;
	m_data = NULL;

#ifdef FASTER_GROUP
  m_write = false;
	m_inGroup = false;
#endif //FASTER_GROUP

	Initialise();
	return true;
}

bool inline FasterReader::Initialise()
{
#ifdef FASTER_GROUP
// TODO recode this because I'm just creating a vector of pointers all pointing to the same Hit !!
	static_assert(true, "FASTER_GROUP not working (emitted from FasterReader::Initialise())");
	m_hit_group_buffer.resize(5000, &m_empty_hit); //If the number of hits in one group exceeds 5000 then it will crash
	m_group_write_cursor = 0;
	m_group_read_cursor = 0;
#endif //FASTER_GROUP

	// Check if the file can be open and read :
	if (m_filename == "")
	{
		print("No file ", m_filename);
		return false;
	}
	std::ifstream file(m_filename);
	if (!file.is_open())
	{
		print("File ", m_filename, " not found...");
		return false;
	}
	else if (!file.good())
	{
		print("File ", m_filename, " not good...");
		return false;
	}
	file.close();

	// Check the extension of the file, then initialise the reader :
	if (extension(m_filename) == "fast")
	{
		if (m_verbose > 0) print("Reader set to", m_filename);
		return (m_kReady = InitialiseReader());
	}
	else
	{
		if (m_verbose > 0) print("File ", m_filename, " not a .fast file !!");
		return (m_kReady = false);
	}
}

bool FasterReader::InitialiseReader()
{
	if (m_reader != NULL) faster_file_reader_close(m_reader); // If the reader has already been used
	m_reader = faster_file_reader_open(m_filename.c_str());
	if (m_reader == NULL)
	{
		if (m_verbose > 0)
			print("Faster file ", m_filename, " impossible to open");
		return false;
	}
	return true;
}

// ============ //
//     READ     //
// ============ //

bool inline FasterReader::Read()
{
	if (++m_counter > s_maxHits) return false;
#ifdef FASTER_GROUP
	return ReadGroup();
#else  // TRIGGERLESS DATA :
	return ReadSimple();
#endif //FASTER_GROUP
}

/**
 * @brief Read triggerless data
 * @details
 * This function is replaced by ReadGroup if the faster data contains groups, this one is simply faster
 */
bool inline FasterReader::ReadSimple()
{
	// faster_file_reader_next returns a pointer (= true in boolean) if the read was successful (fasterac library)
	while (bool_cast(m_data = faster_file_reader_next(m_reader)))
	{
    // Additionally, ReadData returns true only when a data whose alias is handled is reached
    // Therefore, this returns true only when the
		if(ReadData(m_data)) return true;
	}
  // End of the file :
  return false;
}

/**
 * @brief Replace the standard ReadSimple if the faster data contains groups
 * First of all, the m_write variable is used to make sure only the handled types are managed (see switch_alias() definition)
 * First hit read : goes inside ReadData()
 *   if it isn't a GROUP_TYPE_ALIAS then read it normally
 *   else :
 *     recursively read the data inside of the group and fill in m_hit_group_buffer
 *     returns the first hit // TO CHECK THIS !!!
 *     and sets m_inGroup to true
 * The other hits :
 *   if we still are in a group (m_inGroup is true):
 *     reads the next hit in m_hit_group_buffer
 *   else :
 *     if it isn't a GROUP_TYPE_ALIAS then read it normally
 *     else :
 *       recursively read the data inside of the group and fill in m_hit_group_buffer
 *       returns the first hit // TO CHECK THIS !!!
 *       and sets m_inGroup to true
 */
bool FasterReader::ReadGroup()
{
#ifdef FASTER_GROUP
	m_write = false;
	while (!m_write)
	{
		if (m_inGroup)
		{ // We currently are reading a group
			// if (m_group_read_cursor > m_group_write_cursor) // Might be losing the last hit with "==" condition ...
			if (m_group_read_cursor == m_group_write_cursor)
			{ // We are getting out of the group, time to read the next data or group of data
				m_inGroup = false;
				// Reset m_hit_group_buffer cursors
				m_group_read_cursor = 0;
				m_group_write_cursor = 0;
				m_data = faster_file_reader_next(m_reader);
				if (!m_data)
					return false; // This tags the end of the data
				ReadData(m_data);
			}
			else
			{ // We're still in the group :
				m_hit = m_hit_group_buffer[m_group_read_cursor];
				m_group_read_cursor++;
			}
		}
		else
		{ // We aren't in a group
			// Here is the "normal" data reading :
			m_data = faster_file_reader_next(m_reader);
			if (!m_data)
				return false; // This tags the end of the file
			ReadData(m_data);
		}
	}
#endif //FASTER_GROUP
	return m_data;
}

/**
 * @brief Treats faster data
 */
bool inline FasterReader::ReadData(faster_data_p const &_data)
{
  // 1. Extract the label (id number) of the data :
	m_hit->label = faster_data_label(_data);

  // 2. Clean the qdc2 and qdc3 fields if they are being used 
  // because they will keep the previous value if the current data don't fill them :
#ifndef QDC1MAX
    m_hit->qdc2 = 0;
  #ifndef QDC2MAX
      m_hit->qdc3 = 0;
  #endif //QDC2MAX
#endif //QDC1MAX

  // 3. Extracts the alias of the detector from the data (=data type (trapez, qdc...)) :
	auto const & alias = faster_data_type_alias(_data);

  // 3.special : Handle the case when the data is in fact the beginning of an event :
	if (alias == GROUP_TYPE_ALIAS)
	{
#ifdef FASTER_GROUP
		// When read a label of a group, then go to ReadDataGroup() to fill in m_hit_group_buffer recursively
		ReadDataGroup(_data);
#else  //!FASTER_GROUP
		print("COMPILE IN GROUP MODE !!!");
#endif //FASTER_GROUP
	}

  // 4. Extracts the timestamp of the hit. It is retrieved in ns, but we work with ps in this code
  // Furthermore, the type is float but we want unsigned long long, which Timestamp is an alias for :
	m_hit->stamp = Timestamp_cast(faster_data_hr_clock_ns(_data) * 1000); // Stores the timestamp in ps

#ifndef FASTER_GROUP
	// 5. This function fills the other fields of the hit depending on the alias of the data (trapez, qdc...):
	return switch_alias(alias, _data);
#else //FASTER_GROUP
	//  This function fills the other fields of the hit depending on the alias of the data (trapez, qdc...):
	m_write = switch_alias(alias, _data);
	if (m_inGroup && m_write)
	{ // We recursively end up here using ReadDataGroup() method (because m_inGroup is set to true)
		// Here we fill in the m_hit_group_buffer with the hits extracted by the previous lines
		// Hence, we convert a Data buffer (recursively readd) into a Hit buffer,
		// that one can read hit by hit using the ReadGroup method
		m_hit_group_buffer[m_group_write_cursor] = m_hit;
		m_group_write_cursor++;
	}
	return m_write;
#endif //FASTER_GROUP
}

/**
 * @brief Treats the faster data groups
 * 
 * \internal Trigger mode only. Allows one to recursively call FasterReader::ReadData 
 * for each hit in a faster group
 * 
 */
void FasterReader::ReadDataGroup(faster_data_p const &_data)
{
	unsigned short lsize = faster_data_load_size(_data);
	void *group_buffer = faster_data_load_p(_data);
	auto group_reader = faster_buffer_reader_open(group_buffer, lsize);
	faster_data_p group_data;
	// Extracts all information in the "Group Data", which is the first "Data" of a group
	// Loop over data in data group buffer :
#ifdef FASTER_GROUP
	m_inGroup = true;
#endif //FASTER_GROUP
	while ((group_data = faster_buffer_reader_next(group_reader)))
	{
		// Recursive call to ReadData in order to read all the data stored in group_buffer and fill m_hit_group_buffer
		ReadData(group_data);
	}
	faster_buffer_reader_close(group_reader);
}

/**
 * @brief Treat the specific part of data (QDC gates, spectro ADC ...)
 * 
 * \private Internal method that is used to fill hit.adc depending
 * on the alias of the data, that correspond to a certain kind of 
 * faster data.
 * 
 */
bool inline FasterReader::switch_alias(uchar const &_alias, faster_data_p const &_data)
{
	switch (_alias)
	{
	case TRAPEZ_SPECTRO_TYPE_ALIAS: // trapez_spectro
		TreatTrapez(_data);
		return true;

	case QDC_TDC_X1_TYPE_ALIAS: // qdc_t_x1
		TreatQDC1(_data);
		return true;

#ifndef QDC1MAX
	case QDC_TDC_X2_TYPE_ALIAS: // qdc_t_x2
		TreatQDC2(_data);
		return true;

  #ifndef QDC2MAX
    case QDC_TDC_X3_TYPE_ALIAS: // qdc_t_x3
      TreatQDC3(_data);
      return true;
  #endif //QDC2MAX
#endif //QDC1MAX

	case RF_DATA_TYPE_ALIAS: // rf_data
		TreatRF(_data);
		return true;

	case CRRC4_SPECTRO_TYPE_ALIAS: // crrc4_spectro
		TreatCRRC4(_data);
		return true;

	case SYNCHRO_TYPE_ALIAS:
	case RF_COUNTER_TYPE_ALIAS:
	case QDC_COUNTER_TYPE_ALIAS:
	case SPECTRO_COUNTER_TYPE_ALIAS:
	default:
		return false;
	}
}

/**
 * @brief Load Trapez data
 * 
 * \private Internal method used to extract ADC value from trapezoid filter
 * 
 */
void inline FasterReader::TreatTrapez(const faster_data_p &data)
{
	trapez_spectro adc;
	faster_data_load(data, &adc);

	m_hit->adc = adc.measure;
	m_hit->pileup = (adc.pileup == 1 || adc.saturated == 1);
}

/**
 * @brief Load CRRC4 data
 * 
 * \private Internal method used to extract ADC value from CRRC4 filter
 * 
 * \attention m_hit->pileup = (false); //TO BE LOOKED AT
 * 
 */
void inline FasterReader::TreatCRRC4(const faster_data_p &data)
{
	crrc4_spectro crrc4_adc;
	faster_data_load(data, &crrc4_adc);

	m_hit->adc = crrc4_adc.measure;
	m_hit->pileup = (false); //TO BE LOOKED AT
}

/**
 * @brief Load QDC1 data
 * 
 * \private Internal method used to extract QDC values with 1 gate
 * 
 */
void inline FasterReader::TreatQDC1(const faster_data_p &data)
{
	qdc_t_x1 qdc;
	faster_data_load(data, &qdc);

	m_hit->adc = qdc.q1;
	m_hit->pileup = (qdc.q1_saturated == 1);
}

/**
 * @brief Load qdc2 data
 * 
 * \private Internal method used to extract QDC values with 2 gates
 * 
 */
void inline FasterReader::TreatQDC2(const faster_data_p &data)
{
	qdc_t_x2 qdc;
	faster_data_load(data, &qdc);
	m_hit->adc = qdc.q1;
#ifdef QDC1MAX
	if (!error_message["QDC2"])
	{
		error("QDC2 found despite #define QDC1MAX ");
		error_message["QDC2"] = true;
	}
  #else
    m_hit->qdc2 = qdc.q2;
    #ifdef QDC2MAX
      if (!error_message["QDC3"])
      {
        error("QDC3 found despite #define QDC1MAX or #define QDC2MAX");
        error_message["QDC3"] = true;
      }
    #endif //QDC2MAX
#endif //QDC1MAX
	m_hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1);
}

/**
 * @brief Load qdc3 data
 * 
 * \private Internal method used to extract QDC values with 3 gates
 */
void inline FasterReader::TreatQDC3(const faster_data_p &data)
{
	qdc_t_x3 qdc;
	faster_data_load(data, &qdc);
	m_hit->adc = qdc.q1;
#ifdef QDC1MAX
    if (!error_message["QDC3"])
    {
      print("QDC3 found despite #define QDC1MAX ");
      error_message["QDC3"] = true;
    }
  #else
    m_hit->qdc2 = qdc.q2;
    #ifdef QDC2MAX
      if (!error_message["QDC3"])
      {
        print("QDC3 found despite #define QDC1MAX or #define QDC2MAX");
        error_message["QDC3"] = true;
      }
    #else
    m_hit->qdc3 = qdc.q3;
  #endif //QDC2MAX
#endif //QDC1MAX
	m_hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1 || qdc.q3_saturated);
}

/**
 * @brief Load RF data
 * 
 * @private Internal method used to extract RF period
 * 
 */
void inline FasterReader::TreatRF(const faster_data_p& data)
{
   rf_data rf;
   faster_data_load(data, &rf);

   m_hit->adc = ADC_cast(rf_period_ns(rf)*1000);
 #ifndef QDC1MAX
  m_hit->qdc2 = 0;
  #ifndef QDC2MAX
    m_hit->qdc3 = 0;
  #endif //QDC2MAX
#endif //QDC1MAX
   m_hit->pileup = false;
}

#endif //FASTERREADER_HPP
