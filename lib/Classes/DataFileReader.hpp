#ifndef DATAFILEREADER_H_CO
#define DATAFILEREADER_H_CO

#include "../utils.hpp"

/*
  DEPRECATED SINCE NearLine3. USE FasterReader.hpp instead
*/

class DataFileReader
{//Can read .root as well as .fast datafiles
public:
  DataFileReader(){m_hit = new Hit; m_empty_hit = new Hit;};
  DataFileReader(Hit* _hit) : m_hit(_hit) {m_empty_hit = new Hit;};
  DataFileReader(std::string _filename) : m_filename(_filename) {m_kReady = Initialize();};
  DataFileReader(Hit* _hit, std::string _filename) : m_hit(_hit), m_filename(_filename) {m_empty_hit = new Hit; m_kReady = Initialize();};
  ~DataFileReader(){
    delete  m_ReadTree;
    delete m_empty_hit;
    switch (m_quick_rootOrFast)
    {
      case 'r':
        break;
      #ifndef NO_FASTERAC
      case 'f':
        faster_file_reader_close(m_reader);
        break;
      #endif //NO_FASTERAC
      default:
        break;
    }
  };

  Bool_t Initialize();
  Bool_t Initialize(std::string const & _filename);
  Bool_t Read(Hit* hit);
  Bool_t Read();
  Bool_t ReadRoot(Int_t const & pos);
  #ifndef NO_FASTERAC
  Bool_t ReadFast();
  Bool_t ReadFastGroup();
  //Faster functions
  void switch_alias(uchar const & _alias, faster_data_p const & _data,  Bool_t & _write);
  void readFastGroup(faster_data_p const & _data);
  Bool_t ReadFastData(faster_data_p const & _data);
  #endif //NO_FASTERAC
  //Root functions
  void cd();
  //Getters :
  Hit*        getHit              () const { return m_hit        ;};
  Bool_t      const & isReady     () const { return m_kReady     ;};
  Bool_t      const & isWritable  () const { return m_write      ;};
  std::string const & isRootOrFast() const { return m_rootORfast ;};

private:
  //Internal methods :
  Bool_t InitializeRootRead();
  #ifndef NO_FASTERAC
  Bool_t InitializeFasterRead();
  #endif //NO_FASTERAC

  bool m_verbose = false;

  // Containers :
  Hit*        m_hit = nullptr;
  Hit*        m_empty_hit = nullptr;
  std::string m_filename   = "",
              m_rootORfast = "";
  Bool_t      m_kReady   = true,
              m_write    = false,
              m_QDCx[4]  = {1,0,0,0};
  char        m_quick_rootOrFast = 0; // 'r' -> root ; 'f' -> faster
  ULong64_t   m_cursor = 0;//for root data
  UShort_t    m_group_read_cursor  = 0,//for faster data
              m_group_write_cursor = 0;//for faster data
  Buffer_ptr  m_group_buffer;

  //Readers :
  //1: if this is a .root
  std::unique_ptr<TFile> m_TFile;
  TTree *m_ReadTree = nullptr;

  //2: If this is a .fast
  #ifndef NO_FASTERAC
  faster_file_reader_p   m_reader = NULL;
  faster_data_p          m_data;
  uchar          m_alias = 0;
  #endif //NO_FASTERAC
  //For treating groups :
  Bool_t                 m_isGroup  = false;
};

Bool_t DataFileReader::Initialize()
{
#ifndef NO_FASTERAC
  #ifdef FASTER_GROUP
  m_group_buffer.resize(500, m_empty_hit);
  m_group_write_cursor = 0; m_group_read_cursor = 0;
  #endif //FASTER_GROUP
#endif //NO_FASTERAC
  m_cursor = 0;
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
  if (extension(m_filename) == "root")
  {
    m_rootORfast = "root";
    m_quick_rootOrFast = 'r';
    std::cout << "Reader set to " << m_filename << std::endl;
    return InitializeRootRead();
  }
  else if (extension(m_filename) == "fast")
  {
  #ifndef NO_FASTERAC
    m_rootORfast = "fast";
    m_quick_rootOrFast = 'f';
    std::cout << "Reader set to " << m_filename << std::endl;
    return InitializeFasterRead();
  #else
    print("No faster read !!");
  #endif //NO_FASTERAC
  }
  else
  {
    std::cout << "File not a .root nor a .fast file" << std::endl;
    m_kReady = false; return false;
  }
  return false;
}

Bool_t DataFileReader::Initialize(std::string const & _filename)
{
  m_filename = _filename;
  return Initialize();
}

Bool_t DataFileReader::Read(Hit* hit)
{
  Bool_t ret = Read();
  hit = m_hit;
  return ret;
}

Bool_t DataFileReader::Read()
{
  switch (m_quick_rootOrFast)
  {
    case 'r':
      return (ReadRoot(m_cursor));
      break;

  #ifndef NO_FASTERAC
    case 'f':
    #ifdef FASTER_GROUP
      return ReadFastGroup();
    #else
      return ReadFast();
    #endif //FASTER_GROUP
      break;
  #endif //NO_FASTERAC

    default:
      return 0;
  }
}

//===================
//    ROOT FILES
//===================
Bool_t DataFileReader::ReadRoot(Int_t const & pos)
{
  m_cursor++;
  return m_ReadTree -> GetEntry(pos);
}

void DataFileReader::cd()
{
  if (m_quick_rootOrFast=='r')
  m_TFile->cd();
  else std::cout << "Not a root file !!" << std::endl;
}

//====================
//    FASTER FILES
//====================
#ifndef NO_FASTERAC

Bool_t DataFileReader::ReadFast()
{//replace this function by ReadFastGroup if the faster data contains groups, this one is simply faster
m_write = false;
while(!m_write && (m_data = faster_file_reader_next(m_reader)))
{
  m_write = ReadFastData(m_data);
}
return m_write;
}

Bool_t DataFileReader::ReadFastGroup()
{//replace the standard ReadFast if the faster data contains groups
m_write = false;
while(!m_write)
{
  if (m_isGroup)
  {
    m_hit = m_group_buffer[m_group_read_cursor];
    if (m_group_read_cursor == m_group_write_cursor)
    {
      m_isGroup = false;
      for (int i = 0; i< m_group_write_cursor; i++)
      {
        m_group_buffer[i] = m_empty_hit;
      }
      m_group_read_cursor = 0;
      m_group_write_cursor = 0;
      m_data = faster_file_reader_next(m_reader);
      if (!m_data) return false;
      m_write =  ReadFastData(m_data);
    }
    else
    {
      m_group_read_cursor++;
    }
    m_write = (Bool_t) m_hit->label;
  }
  else
  {
    m_data = faster_file_reader_next(m_reader);
    if (!m_data) return false;
    m_write = ReadFastData(m_data);
  }
}
return m_write;
}

void DataFileReader::switch_alias(uchar const & _alias, faster_data_p const & _data,  Bool_t & _write)
{//Treat the specific part of data
  _write = false;
  // !!! ATTENTION : HARD CODE, TO IMPROVE !!!!
  if (m_hit->label==251) //!!
  {//!!
    TreatRF(m_hit, _data);//!!
    _write = true;//!!
    return;//!!
  }//!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  switch(m_alias)
  {
    case TRAPEZ_SPECTRO_TYPE_ALIAS: // trapez_spectro
      TreatTrapez(m_hit, _data);
      _write = true;
      break;
    case QDC_TDC_X1_TYPE_ALIAS: // qdc_t_x1
      TreatQDC1(m_hit, _data);
      _write = true;
      break;
    case QDC_TDC_X2_TYPE_ALIAS: // qdc_t_x2
      TreatQDC2(m_hit, _data);
      _write = true;
      break;
    case QDC_TDC_X3_TYPE_ALIAS: // qdc_t_x3
      TreatQDC3(m_hit, _data);
      _write = true;
      break;
    case QDC_TDC_X4_TYPE_ALIAS: // qdc_t_x4
      TreatQDC4(m_hit, _data);
      _write = true;
      break;
    case SYNCHRO_TYPE_ALIAS: // synchro
      break;
    case TREF_TYPE_ALIAS: // rf_data
      TreatRF(m_hit, _data);
      _write = true;
      break;
    case CRRC4_SPECTRO_TYPE_ALIAS: // crrc4_spectro
      TreatCRRC4(m_hit, _data);
      _write = true;
      break;
    case RF_COUNTER_TYPE_ALIAS:
      break;
    case QDC_COUNTER_TYPE_ALIAS: // qdc_counter
      break;
    case SPECTRO_COUNTER_TYPE_ALIAS: // spectro_counter
      break;
    default:
      break;
  }
}

void DataFileReader::readFastGroup(faster_data_p const & _data)
{//Treat the faster groups
  // unsigned short label = faster_data_label      (_data);
  unsigned short lsize = faster_data_load_size  (_data);
  void* group_buffer   = faster_data_load_p     (_data);
  auto group_reader  = faster_buffer_reader_open(group_buffer, lsize);
  faster_data_p group_data;
  // Loop over data in buffer
  while( (group_data = faster_buffer_reader_next (group_reader)) )//!= NULL)
  {
    m_isGroup=true;
    ReadFastData(group_data);
    // std::cout << "Inside the group : " << hit.label << "\t" << hit.time/1e9 << std::endl;
  }
  faster_buffer_reader_close(group_reader);
}

Bool_t DataFileReader::ReadFastData(faster_data_p const & _data)
{//Treat faster data
  Bool_t write = false;
  m_hit->label = faster_data_label(_data);
  m_alias = faster_data_type_alias(_data);
  if (m_alias == GROUP_TYPE_ALIAS)
  {//Creates a group reader
    #ifndef FASTER_GROUP
    std::cout << "COMPILE IN GROUP MODE !!!" << std::endl;
    #else
    readFastGroup(_data);
    #endif
  }
  m_hit->time = faster_data_hr_clock_ns(_data) * 1000;// Store the time in ps
  // Treat(faster_data_p const & _data, uchar const & m_alias)
  switch_alias(m_alias, _data, write);
  #ifdef FASTER_GROUP
  if (write && m_isGroup)
  {
    m_group_buffer[m_group_write_cursor] = m_hit;
    m_group_write_cursor++;
  }
  #endif
  return write;
}

#endif //NO_FASTERAC

//====================
//  Privates methods
//====================

Bool_t DataFileReader::InitializeRootRead()
{
  if (m_TFile.get()) m_TFile->Close();
  m_TFile.reset(TFile::Open(m_filename.c_str(),"read"));
  // delete m_TFile;
  // m_TFile = (TFile::Open(m_filename.c_str(),"read"));
  // if (m_TFile.get() == nullptr)
  if (m_TFile.get() == nullptr)
  {
    std::cout << "Data file " << m_filename << " impossible to open" << std::endl;
    m_kReady = false; return false;
  }
  m_TFile->cd();
  // m_ReadTree.reset(nullptr);
  // m_ReadTree.reset(m_TFile->Get<TTree>("DataTree"));
  // if (m_ReadTree!=nullptr)
  //   delete m_ReadTree;
  m_ReadTree = (m_TFile->Get<TTree>("DataTree"));
  // if (m_ReadTree.get() == nullptr)
  if (m_ReadTree == nullptr)
  {
    m_ReadTree = (m_TFile->Get<TTree>("Nuball"));
    if (m_ReadTree == nullptr)
    {
      std::cout << "Data Tree " << m_filename <<  " impossible to get" << std::endl;
      m_kReady = false; return false;
    }
  }

  auto branches = m_ReadTree->GetListOfBranches();

  for (int i = 0; i<branches->GetEntries(); i++)
  {
         if (std::string(branches->At(i)->GetName()) == "nrj")  m_QDCx[0] = true;
    else if (std::string(branches->At(i)->GetName()) == "nrj2") m_QDCx[1] = true;
    else if (std::string(branches->At(i)->GetName()) == "nrj3") m_QDCx[2] = true;
    else if (std::string(branches->At(i)->GetName()) == "nrj4") m_QDCx[3] = true;
  }
  m_ReadTree -> SetBranchAddress("label" , &(*m_hit).label  );
  m_ReadTree -> SetBranchAddress("nrj"   , &(*m_hit).nrj    );
  if (m_QDCx[1] == true) m_ReadTree -> SetBranchAddress("nrj2", &(*m_hit).nrj2   );
  if (m_QDCx[2] == true) m_ReadTree -> SetBranchAddress("nrj3", &(*m_hit).nrj3   );
  if (m_QDCx[3] == true) m_ReadTree -> SetBranchAddress("nrj4", &(*m_hit).nrj4   );
  m_ReadTree -> SetBranchAddress("time"  , &(*m_hit).time   );
  m_ReadTree -> SetBranchAddress("pileup", &(*m_hit).pileup );
  return true;
}

#ifndef NO_FASTERAC

Bool_t DataFileReader::InitializeFasterRead()
{
  if (m_reader != NULL) faster_file_reader_close(m_reader);
  m_reader = faster_file_reader_open ( m_filename.c_str() );
  if (m_reader == NULL)
  {
    std::cout << "Faster file " << m_filename << " impossible to open" << std::endl;
    m_kReady = false; return false;
  }
  return true;
}
#endif //NO_FASTERAC

#endif //DATAFILEREADER_H_CO
