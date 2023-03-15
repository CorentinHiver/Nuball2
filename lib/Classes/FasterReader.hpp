#ifndef FASTERREADER_H
#define FASTERREADER_H
#include "../utils.hpp"

class FasterReader
{//Can read .root as well as .fast datafiles
public:
  FasterReader(){};
  FasterReader(Hit* _hit) : m_hit(_hit) {};
  FasterReader(std::string _filename) : m_filename(_filename) {m_kReady = Initialize();};
  FasterReader(Hit* _hit, std::string _filename) : m_hit(_hit), m_filename(_filename) {m_kReady = Initialize();};

  ~FasterReader()
  {
    faster_file_reader_close(m_reader);
  }

  Bool_t Initialize();
  Bool_t Initialize(std::string const & _filename);
  Bool_t Read(Hit* hit);
  Bool_t Read();
  Bool_t ReadSimple();
  Bool_t ReadGroup();
  //Faster functions
  Bool_t switch_alias(unsigned char const & _alias, faster_data_p const & _data);
  void ReadDataGroup(faster_data_p const & _data);
  void ReadData(faster_data_p const & _data);

  //Setters :
  void setHit(Hit* hit) {delete m_hit; m_hit = hit;} //Never tested, but should work !

  //Getters :
  Hit*   getHit             () const { return m_hit        ;};
  Bool_t const & isReady    () const { return m_kReady     ;};
  Bool_t const & isWritable () const { return m_write      ;};

private:
  //Internal methods :
  Bool_t InitializeReader();

  // Containers :
  Hit*        m_hit = nullptr;
  Hit         m_empty_hit;
  std::string m_filename   = "";
  Bool_t      m_kReady   = true,
              m_write    = false,
              m_QDCx[4]  = {1,0,0,0};
  UShort_t    m_group_read_cursor  = 0,
              m_group_write_cursor = 0;
  Buffer_ptr  m_hit_group_buffer;
  faster_file_reader_p   m_reader = NULL;
  faster_data_p          m_data;
  unsigned char          m_alias = 0;
  //For treating groups :
  Bool_t                 m_inGroup  = false;
};

// ================== //
//   INITIALIZATION   //
// ================== //

Bool_t FasterReader::Initialize(std::string const & _filename)
{
  m_filename = _filename;
  return Initialize();
}

Bool_t FasterReader::Initialize()
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

Bool_t FasterReader::InitializeReader()
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

Bool_t FasterReader::Read()
{
  // To get access to the hit, you need to use the method GetHit();
  #ifdef FASTER_GROUP
  return ReadGroup();
  #else
  return ReadSimple();
  #endif //FASTER_GROUP
}

Bool_t FasterReader::Read(Hit* hit)
{
  // This function allows one to directly link an external hit to m_hit
  Bool_t ret = Read();
  hit = m_hit;
  return ret;
}

Bool_t FasterReader::ReadSimple()
{// This function is replaced by ReadGroup if the faster data contains groups, this one is simply faster
  // and serves as the "prototype" to read groupless data
  // m_write variable is used to make sure only the handled types are read (see switch_alias definition)
  m_write = false;
  // faster_file_reader_next returns a value > 0 (->true in boolean) if the read was successfull (fasterac library)
  while(!m_write && (m_data = faster_file_reader_next(m_reader)))
  {
    ReadData(m_data);
  }
  return m_data;
}

Bool_t FasterReader::ReadGroup()
{// Replace the standard ReadSimple if the faster data contains groups
  /*
    First of all, the m_write variable is used to make sure only the handled types are managed (see switch_alias() definition)
    First hit read : goes inside ReadData()
      if it isn't a GROUP_TYPE_ALIAS then read it normally
      else :
        recursively read the data inside of the group and fill in m_hit_group_buffer
        returns the first hit // TO CHECK THIS !!!
        and sets m_inGroup to true
    The other hits :
      if we still are in a group (m_inGroup is true):
        reads the next hit in m_hit_group_buffer
      else :
        if it isn't a GROUP_TYPE_ALIAS then read it normally
        else :
          recursively read the data inside of the group and fill in m_hit_group_buffer
          returns the first hit // TO CHECK THIS !!!
          and sets m_inGroup to true
    */
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

void FasterReader::ReadData(faster_data_p const & _data)
{// Treats faster data
  m_hit->label = faster_data_label(_data);
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
}

void FasterReader::ReadDataGroup(faster_data_p const & _data)
{ // Treats the faster data groups
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

Bool_t FasterReader::switch_alias(unsigned char const & _alias, faster_data_p const & _data)
{//Treat the specific part of data (QDC gates, spectro ADC ...)
  switch(m_alias)
  {
    case TRAPEZ_SPECTRO_TYPE_ALIAS: // trapez_spectro
      TreatTrapez(m_hit, _data);
      return true;

    case QDC_TDC_X1_TYPE_ALIAS: // qdc_t_x1
      TreatQDC1(m_hit, _data);
      return true;
    #ifdef QDC2
    case QDC_TDC_X2_TYPE_ALIAS: // qdc_t_x2
      TreatQDC2(m_hit, _data);
      return true;
    #endif //QDC2

    case RF_DATA_TYPE_ALIAS: // rf_data
      TreatRF(m_hit, _data);
      return true;
      break;
    //
    // case QDC_TDC_X3_TYPE_ALIAS: // qdc_t_x3
    //   TreatQDC3(m_hit, _data);
    //   return true;
    //
    // case QDC_TDC_X4_TYPE_ALIAS: // qdc_t_x4
    //   TreatQDC4(m_hit, _data);
    //   return true;

    case SYNCHRO_TYPE_ALIAS: // synchro
      return false;

    case CRRC4_SPECTRO_TYPE_ALIAS: // crrc4_spectro
      TreatCRRC4(m_hit, _data);
      return true;

    case RF_COUNTER_TYPE_ALIAS:
      return false;
    case QDC_COUNTER_TYPE_ALIAS: // qdc_counter
      return false;
    case SPECTRO_COUNTER_TYPE_ALIAS: // spectro_counter
      return false;
    default:
      return false;
  }
}

#endif //FASTERREADER_H
