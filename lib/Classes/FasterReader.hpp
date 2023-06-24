#ifndef FASTERREADER_H
#define FASTERREADER_H

#include <Hit.hpp>

// ******* fasterac includes ******* //
#include "fasterac/adc.h"
#include "fasterac/adc_caras.h"
#include "fasterac/electrometer.h"
#include "fasterac/farray.h"
#include "fasterac/fast_data.h"
#include "fasterac/fasterac.h"
#include "fasterac/group.h"
#include "fasterac/jdb_hv.h"
#include "fasterac/online.h"
#include "fasterac/qdc.h"
#include "fasterac/qdc_caras.h"
#include "fasterac/qtdc.h"
#include "fasterac/rf.h"
#include "fasterac/rf_caras.h"
#include "fasterac/sampler.h"
#include "fasterac/scaler.h"
#include "fasterac/spectro.h"
#include "fasterac/utils.h"

/** 
 * @class FasterReader
 * @brief Class used to read .fast files
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
 * This method deals with the 5 following members of the Hit structure:
 * 
 *        struct Hit
 *        {
 *          unsigned short label;  // The label number of the detector
 *          Float_t        nrj;    // The ADC/QDC value of the hit. For RF, contains the frequency.
 *          Float_t        nrj2;   // The QDC2 value (usually the longer gate)
 *          ULong64_t      time;   // High precision time in ps (time tick of 7,8125 ps)
 *          bool           pileup; // Contains either pileup or saturated bit
 *        }
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
 * 
 * -- QDC1MAX
 * 
 * By default the nrj2 is handled. 
 * If no detector uses the QDC2 then declare it
 * 
 * 
 * -- FASTER_GROUP
 * 
 * If the data is grouped using a hardware trigger then the reading is done in
 * two steps : first extracts all the hits of the group and put it in a vector,
 * then each call of Read() moves in the group. No group information is extracted :
 * from outside the class everything goes as if there was no group
 * 
 * 
 */
class FasterReader
{
public:
  /**
   * @brief Construct a new Faster Reader object
   */
  FasterReader(Hit* _hit, std::string _filename) : m_hit(_hit), m_filename(_filename) {m_kReady = Initialize();}

  /**
   * @brief Destroy the Faster Reader object
   */
  ~FasterReader()
  {
    faster_file_reader_close(m_reader);
  }
  
  /**
   * @brief TBD, reset the cursor to the begining of the document
   */
  bool Reset();
  
  /**
   * @brief Main method. Extract the next hit from the data file and fills the Hit *m_hit object
   * 
   * Hit hit;
   * 
   * FasterReader reader(&hit, filename);
   * 
   *      while(reader.Read())
   *      {
   *         // This hit is filled/updated at each iteration by the data read in the ReadData() private method
   *         print(hit); 
   *      }
   * 
   * 
   * @return true if the end of the file is reached
   * @return false if still in the middle of the file
   */
  bool Read();

  // ------ Setters ------ :
  /**
   * @brief \test Set the Hit object
   *  \nNever tested, but should work !
   */
  void setHit(Hit* hit) {m_hit = hit;}

  /**
   * @brief Set the Max Hits read
   */
  void setMaxHits(ulonglong maxHits) {m_maxHits = maxHits;}

  // ------ Getters ------ :
  /**
   * @brief \deprecated Get the current Hit 
   */
  Hit* getHit             () const { return m_hit        ;}

  /**
   * @brief If the initialization went badly then returns false
   */
  bool const & isReady    () const { return m_kReady     ;}

  /**
   * @brief Get the name of the file being read
   */
  std::string const & getFilename() const {return m_filename;}

private:
  
  /**
   * @brief Setup the fasterac objects for reading
   */
  bool Initialize();


  bool ReadSimple();
  bool ReadGroup();
  void ReadDataGroup(faster_data_p const & _data);
  bool ReadData(faster_data_p const & _data);
  bool switch_alias(uchar const & _alias, faster_data_p const & _data);


  bool InitializeReader();

  ulonglong m_maxHits = -1,
            m_cursor  =  0,
            m_counter =  0;

  Hit*        m_hit = nullptr;
  Hit         m_empty_hit;
  std::string m_filename = "";
  bool        m_kReady   = true,
              m_write    = false,
              m_QDCx[4]  = {1,0,0,0};
  ushort      m_group_read_cursor  = 0,
              m_group_write_cursor = 0,
              m_inGroup  = false;

  std::vector<Hit*>     m_hit_group_buffer;
  faster_file_reader_p  m_reader = NULL;
  faster_data_p         m_data;
  uchar                 m_alias = 0;

  void TreatTrapez(faster_data_p const & _data);
  void TreatCRRC4 (faster_data_p const & _data);
  void TreatQDC1  (faster_data_p const & _data);
  void TreatQDC2  (faster_data_p const & _data);
  void TreatRF    (faster_data_p const & _data);
};

// ================== //
//   INITIALIZATION   //
// ================== //

bool FasterReader::Reset()
{
      //TBD
  return true;
}

bool FasterReader::Initialize()
{
  #ifdef FASTER_GROUP
  m_hit_group_buffer.resize(5000, &m_empty_hit); //If the number of hits in one group exceeds 5000 then it will crash
  m_group_write_cursor = 0; m_group_read_cursor = 0;
  #endif //FASTER_GROUP

  // Check if the file can be open and read :
  if (m_filename == "")
  {
    std::cout << "No file " << m_filename << std::endl;
    return false;
  }
  std::ifstream file(m_filename);
  if (!file.is_open())
  {
    std::cout << "File " << m_filename << " not found..." << std::endl;
    return false;
  }

  else if (!file.good())
  {
    std::cout << "File " << m_filename << " not good..." << std::endl;
    return false;
  }
  file.close();

  // Check the extension of the file, then initialise the reader :
  if (extension(m_filename) == "fast")
  {
    std::cout << "Reader set to " << m_filename << std::endl;
    return InitializeReader();
  }
  else
  {
    std::cout << "File not a .fast file !!" << std::endl;
    m_kReady = false; return false;
  }
  return false;
}

bool FasterReader::InitializeReader()
{
  if (m_reader != NULL) faster_file_reader_close(m_reader); // if the reader has already been used to read another file
  m_reader = faster_file_reader_open ( m_filename.c_str() );
  if (m_reader == NULL)
  {
    std::cout << "Faster file " << m_filename << " impossible to open" << std::endl;
    m_kReady = false; return false;
  }
  return true;
}

// ============ //
//     READ     //
// ============ //

bool FasterReader::Read()
{
  if (m_counter++>m_maxHits) 
  {
    return (m_write = false);
  }
#ifdef FASTER_GROUP
  return ReadGroup();
#else // TRIGGERLESS DATA
  return ReadSimple();
#endif //FASTER_GROUP
}

/**
 * @brief Read non grouped data
 * 
 * This function is replaced by ReadGroup if the faster data contains groups, 
 * this one is simply faster and serves as the "prototype" to read groupless data 
 * 
 */
bool FasterReader::ReadSimple()
{ 
  // m_write variable is used to make sure only the handled aliases are read (see switch_alias definition)
  m_write = false;
  // faster_file_reader_next returns a value > 0 (->true in boolean) if the read was successfull (fasterac library)
  // Ergo, the loop continues until the end of the file when the return value is 0
  while(!m_write && (m_data = faster_file_reader_next(m_reader)))
  {
    m_write = ReadData(m_data);
  }
  return static_cast<bool>(m_data);
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
  m_write = false;
  while(!m_write)
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
        if (!m_data) return false; // This tags the end of the data
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
      m_data = faster_file_reader_next(m_reader);
      if (!m_data) return false; // This tags the end of the file
      ReadData(m_data);
    }
  }
  return m_data;
}

/**
 * @brief Treats faster data
 */
bool FasterReader::ReadData(faster_data_p const & _data)
{ 
  m_hit->label = faster_data_label(_data);
#ifndef QDC1MAX
  m_hit->nrj2 = 0; // In order to clean the data, as nrj2 never gets cleaned if there was QDC2 in the previous hit
#endif //QDC1MAX
  m_alias = faster_data_type_alias(_data);
  if (m_alias == GROUP_TYPE_ALIAS)
  {// Creates a group reader
    #ifdef FASTER_GROUP
    // When read a label of a group, then go to ReadDataGroup() to fill in m_hit_group_buffer recursively
    ReadDataGroup(_data);
    #else //!FASTER_GROUP
    std::cout << "COMPILE IN GROUP MODE !!!" << std::endl;
    #endif //FASTER_GROUP
  }
  m_hit -> time = faster_data_hr_clock_ns(_data) * 1000;// Stores the time in ps
  std::cout << std::setprecision(20) << faster_data_hr_clock_ns(_data) << std::endl;
  m_write = switch_alias(m_alias, _data);
#ifdef FASTER_GROUP
  if (m_inGroup && m_write)
  {// We recursively end up here using ReadDataGroup() method (because m_inGroup is set to true)
    // Here we fill in the m_hit_group_buffer with the hits extracted by the previous lines
    // Hence, we convert a Data buffer (recursively readed) into a Hit buffer,
    // that one can read hit by hit using the ReadGroup method
    m_hit_group_buffer[m_group_write_cursor] = m_hit;
    m_group_write_cursor++;
  }
#endif //FASTER_GROUP
  return m_write;
}

/**
 * @brief Treats the faster data groups
 * 
 * \internal Trigger mode only. Allows one to recursively call FasterReader::ReadData 
 * for each hit in a faster group
 * 
 */
void FasterReader::ReadDataGroup(faster_data_p const & _data)
{
  // Extracts all information in the "Group Data", which is the first "Data" of a group
  unsigned short lsize = faster_data_load_size  (_data);
  void* group_buffer   = faster_data_load_p     (_data);
  auto group_reader    = faster_buffer_reader_open(group_buffer, lsize);
  faster_data_p group_data;
  // Loop over data in data group buffer :
  m_inGroup=true;
  while( (group_data = faster_buffer_reader_next (group_reader)) )
  {
    // Recursive call to ReadData in order to read all the data stored in group_buffer and fill m_hit_group_buffer
    ReadData(group_data);
  }
  faster_buffer_reader_close(group_reader);
}

/**
 * @brief Treat the specific part of data (QDC gates, spectro ADC ...)
 * 
 * \private Internal method that is used to fill hit.nrj depending
 * on the alias of the data, that correspond to a certain kind of 
 * faster data.
 * 
 */
bool FasterReader::switch_alias(uchar const & _alias, faster_data_p const & _data)
{ 
  switch(_alias)
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
  #endif //QDC1MAX

    case RF_DATA_TYPE_ALIAS: // rf_data
      TreatRF(_data);
      return true;
      break;

      case CRRC4_SPECTRO_TYPE_ALIAS: // crrc4_spectro
      TreatCRRC4(_data);
      return true;

    case SYNCHRO_TYPE_ALIAS: case RF_COUNTER_TYPE_ALIAS: case QDC_COUNTER_TYPE_ALIAS: case SPECTRO_COUNTER_TYPE_ALIAS: default:
      return false;
  }
}

/**
 * @brief Load Trapez data
 * 
 * \private Internal method used to extract ADC value from trapezoid filter
 * 
 */
void FasterReader::TreatTrapez(const faster_data_p& data)
{  
   trapez_spectro adc;
   faster_data_load(data, &adc);

   m_hit->nrj = adc.measure;
 #ifndef QDC1MAX
   m_hit->nrj2 = 0;
 #endif //QDC1MAX
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
void FasterReader::TreatCRRC4(const faster_data_p& data)
{  
   crrc4_spectro crrc4_adc;
   faster_data_load(data, &crrc4_adc);

   m_hit->nrj = crrc4_adc.measure;
 #ifndef QDC1MAX
   m_hit->nrj2 = 0;
 #endif //QDC1MAX
   m_hit->pileup = (false); //TO BE LOOKED AT
}

/**
 * @brief Load QDC1 data
 * 
 * \private Internal method used to extract QDC values with 1 gate
 * 
 */
void FasterReader::TreatQDC1(const faster_data_p& data)
{ 
  qdc_t_x1 qdc;
  faster_data_load(data, &qdc);

  m_hit->nrj = qdc.q1;

#ifndef QDC1MAX
  m_hit->nrj2 = 0;
#endif //QDC1MAX

  m_hit->pileup = (qdc.q1_saturated == 1);
}

/**
 * @brief Load QDC2 data
 * 
 * \private Internal method used to extract QDC values with 2 gates
 * 
 */
void FasterReader::TreatQDC2(const faster_data_p& data)
{ 
  qdc_t_x2 qdc;
  faster_data_load(data, &qdc);
  m_hit->nrj = qdc.q1;
#ifndef QDC1MAX
  m_hit->nrj2 = qdc.q2;
#endif //QDC1MAX
  m_hit->pileup = (qdc.q1_saturated == 1 || qdc.q2_saturated == 1);
}

/**
 * @brief Load RF data
 * 
 * \private Internal method used to extract RF period
 * 
 */
void FasterReader::TreatRF(const faster_data_p& data)
{
   rf_data rf;
   faster_data_load(data, &rf);

   m_hit->nrj = rf_period_ns(rf)*1000;
 #ifndef QDC1MAX
   m_hit->nrj2 = 0;
 #endif //QDC1MAX
   m_hit->pileup = false;
}

#endif //FASTERREADER_H
