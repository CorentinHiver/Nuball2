#ifndef DETECTORS_HPP
#define DETECTORS_HPP

#include "../libRoot.hpp"
#include "Hit.hpp"

using dType = std::string;

int uiop = 0;

// All detectors lookup tables :
Bools isGe    (1000);
Bools isBGO   (1000);
Bools isLaBr3 (1000);
Bools isEden  (1000);
Bools isRF    (1000);
Bools isParis (1000);
Bools isDSSD  (1000);

// Clover specific lookup tables :
Bools isClover(1000);
std::vector<uchar> labelToClover(1000,0);
std::vector<uchar> labelToBGOcrystal(1000,0);
std::vector<uchar> labelToGecrystal(1000,0);
Strings clover_pos (1000,"");

// DSSD specific lookup tables :
Bools isSector(1000);
Bools isS1    (1000);
Bools isS2    (1000);
Bools isRing  (1000);
Bools isBack  (1000);
Bools isFront (1000);

// Other lookup tables :
Label_vec compressedLabel(1000,-1); // Used to put all the detectors one after the other
Ints m_index(1000,-1);


/**
 * @brief Loads the ID file and fills lookup tables
 * @details
 * Many lookup tables are created : 
 * ...
 * Detectors::index(Label label) : Returns the detector's index in its type. For instance, if there are 10 germaniums, 
 * and their labels ranges contiguously from 10 to 19, Detectors::index(12) = 2;
 * 
 * Detectors::type(Label label) : Returns the detector's type knowing its label. Taking previous example, 
 * Detectors::type(25) = "null" if there are no detector above label = 19, and Detectors::type(13) = "ge";
 */
class Detectors
{
public:

  Detectors(){}

  static Strings types_handled;

  // static bool typeHandled(int const & id) const {} TODO
  // static bool typeID(int const & id) const {} TODO
  static bool exists(Label const & label) {return exist_array[label];}

  /// @brief Reads the file and extracts the list of detectors, then fills the lookup tables 
  void load (std::string const & filename);

  /// @brief Reads the file and extracts the list of detectors
  void readFile(std::string const & file);

  /// @brief Fills the lookup tables
  void makeArrays();

  /// @brief Return the value of the maximum label, i.e. the size of the lookup tables
  auto const size() const {return Label_cast(m_list.size());}

  /// @brief  Return the real number of detectors
  static auto const number() {return nb_detectors;}

  // Iterate over the existing detectors :
  auto begin() {return m_list.begin();}
  auto end  () {return m_list.end  ();}

  auto const & get()        const {return m_list         ;}
  auto const & getExistsArray() const {return exist_array    ;}
  auto const & getLabelsArray() const {return m_labels_array;}
  auto const & getTypeIDArray() const {return m_types_ID_array;}
  auto const & typesArray() const {return m_types_ID_array;}
  static auto const & type(Label const & label) {return m_types[label];}
  auto const & types() const {return typesArray();}
  auto const & typeIndex(dType const & type) const {return m_types_index[type];}
  auto const & typeIndex(Label const & label) const {return m_types_index[this -> type(label)];}
  auto const & getTypeID(int const & type_i) const {return m_types_ID_array[type_i];}

  auto const & getName (Label       const & label) {return m_list[label]        ;}
  auto const & getLabel(std::string const & name ) {return m_labels_array[name];}

  void operator=(std::string const & filename) {this -> load(filename);}

  std::string const & operator[] (int i) const {return m_list[i];}
  operator bool() const & {return m_ok;}

  /// @brief Returns the number of types in the ID file
  static size_t nbTypes() {return m_types_ID_array.size()-1;}

  /// @brief Returns the number of detector of each type
  auto const & nbOfType(dType const & type) {return m_type_counter[type];}

  auto const & name(dType const & type, int const & index) {return m_names[type][index];}
  auto const & label(dType const & type, int const & index) {return m_labels[type][index];}
  auto const & index(Label const & label) const {return m_index[label];}

  static auto const & ADCBin(dType const & type)
  {
    auto const & it = ADC_bins.find(type);
    if (it != ADC_bins.end()) return it->second;
    else return ADC_bins["default"];
  }

  static auto const & energyBin(dType const & type)
  {
    auto const & it = energy_bins.find(type);
    if (it != energy_bins.end()) return it->second;
    else return energy_bins["default"];
  }
  
  static auto const & energyBidimBin(dType const & type)
  {
    auto const & it = energy_bidim_bins.find(type);
    if (it != energy_bidim_bins.end()) return it->second;
    else return energy_bidim_bins["default"];
  }

  static auto const & getADCBin() {return ADC_bins;}
  static auto const & getEnergyBin() {return energy_bins;}
  static auto const & getBidimBin() {return energy_bidim_bins;}

  // Binning informations :
  static std::unordered_map<dType, THBinning> energy_bins;
  static std::unordered_map<dType, THBinning> ADC_bins;
  static std::unordered_map<dType, THBinning> energy_bidim_bins;

protected:
  // Useful informations
  bool m_ok = false;
  bool m_loaded = false;
  bool m_initialized = false;
  static ushort nb_detectors;
  std::string m_filename;

  // Containers
  static Strings m_types;
  std::unordered_map<std::string, Label> m_labels_array;
  static Strings m_types_ID_array;
  static std::map<std::string, int> m_types_index;
  static Bools exist_array;
  static Strings m_list;
  std::unordered_map<dType, size_t> m_type_counter; // To get the number of detector in each alias. 
  std::unordered_map<dType, Strings> m_names;
  std::unordered_map<dType, Label_vec> m_labels;
} detectors;


/// @brief List of types of detectors that this code can handle
Strings Detectors::types_handled = {"ge", "bgo", "labr", "paris", "dssd", "eden", "RF", "null"};

/// @brief List of types of detectors found in the ID file
Strings Detectors::m_types_ID_array;
std::map<std::string, int> Detectors::m_types_index;
Strings Detectors::m_types = Strings(1000);

Bools Detectors::exist_array;
Strings Detectors::m_list;


/// @brief Number of detectors
ushort Detectors::nb_detectors = 0;

void Detectors::load(std::string const & filename)
{
  this -> readFile(filename);
  this -> makeArrays();
}

void Detectors::readFile(std::string const & filename)
{
  std::ifstream inputfile(filename, std::ifstream::in);
  if (!inputfile.is_open())
  {
    m_ok = m_loaded = false;
    m_list.clear();
    print("CANNOT OPEN", filename, "!!");
    return;
  }
  else if (file_is_empty(inputfile))
  {
    m_ok = m_loaded = false;
    print(filename, "is empty !!");
    m_list.clear();
    return;
  }
  m_filename = filename;
  // ----------------------------------------------------- //
  //First reading of the file : extract the maximum label to be the size of the vector
  ushort size = 0; std::string line = "", name = ""; ushort label = 0;
  std::string oneline;
  while (getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label;
    if (size<label) size = label;
  }
  // ----------------------------------------------------- //
  //Second reading : fill the vector
  m_list.resize(size+1);
  exist_array.resize(size+1);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while(getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1)
    {
      exist_array[label] = true;
      m_list[label] = name;
      if (name!="") m_labels_array[name] = label;
    }
  }
  inputfile.close();
  print("Labels extracted from", filename);
  m_ok = m_loaded = true;
}

void Detectors::makeArrays()
{
  if (!m_loaded) {throw std::runtime_error("ID file not loaded !!"); return;}

  // Looping through the labels
  for (Label label = 0; label<this->size(); label++)
  {
    auto const & det_name = m_list[label];
    std::istringstream is(replaceCharacter(det_name, '_', ' '));
    std::string str;

    // Looping through the subparts of the name "subpart_subpart_..._subpart"
    while(is >> str)
    {
      if (str == "red" || str == "green" || str == "black" || str == "blue" || str == "ge"){
        isGe    [label] = true;
        m_types[label] = types_handled[0];
      }
      else if (str == "BGO1" || str == "BGO2"){
        isBGO   [label] = true;
        m_types[label] = types_handled[1];
      }
      else if (str == "FATIMA"){
        isLaBr3 [label] = true;
        m_types[label] = types_handled[2];
      }
      else if (str == "PARIS" ){
        isParis [label] = true;
        m_types[label] = types_handled[3];
      }
      else if (str == "DSSD" ){
        m_types[label] = types_handled[4];
        isDSSD [label] = true;
      }
      else if (str == "EDEN"  ){
        m_types[label] = types_handled[5];
        isEden  [label] = true;
      }
      else if (str == "RF"    ){
        m_types[label] = types_handled[6];
        isRF    [label] = true;
      }
      // Add here any additionnal detector :
      
      // Additionnal position information :
      if (str[0] == 'R' && str[2] == 'A')
      {
        clover_pos[label] = str;
      }
      else if (str.substr(0,2) == "BR")
      {
        isBack[label] = true;
      }
      else if (str.substr(0,2) == "FR")
      {
        isFront[label] = true;
      }
      else if (str == "S1")
      {
        isSector[label] = true;
        isS1    [label] = true;
      }
      else if (str == "S2")
      {
        isSector[label] = true;
        isS2    [label] = true;
      }
      else if (str == "R"){
        isRing[label] = true;
      }
    }

    // Other lookup tables :
    isClover[label] = (label>22 && label<167);
    labelToClover[label] = uchar_cast((isClover[label]) ? (label-23)%6 : -1);
     
    if (exist_array[label])
    {
      // The compressed labels is filled with the current number of detectors :
      compressedLabel[label] = nb_detectors;
      // Increment the number of detectors :
      nb_detectors++;

      // Counts the number of detectors for the current type :
      auto const & detector_index = m_type_counter[m_types[label]]++; // NB: Returns the number before increment
      m_index[label] = detector_index;

      auto const & _type = m_types[label];
      // Lookup table to get the name from the type and detector index :
      m_names[_type].push_back(det_name);
      // Lookup table between the global label from the type and detector index :
      m_labels[_type].push_back(label);


    }
    else compressedLabel[label] = -1;
  }

  for (auto const & type : m_types)
  {// To make the list of all the detector types present in the ID file :
    if (m_type_counter[type]>0) m_types_ID_array.push_back(type);
  }
  // Reverse lookup : 
  for (size_t index = 0; index<m_types_ID_array.size(); index++)
  {
    m_types_index[m_types_ID_array[index]] = index;
  }
}

void operator>>(const char * filename, Detectors & detectors) {detectors.load(std::string(filename));}
// void operator>>(std::string const & filename, Detectors & detectors) {detectors.load(filename);}

#if __cplusplus >= 201703L
std::unordered_map<dType, THBinning> Detectors::ADC_bins = 
{
  {"ge"     , {10000, 0., 100000. }},
  {"bgo"    , {1000 , 0., 100000. }},
  {"labr"   , {1000 , 0., 100000. }},
  {"paris"  , {1000 , 0., 100000. }},
  {"eden"   , {1000 , 0., 100000. }},
  {"dssd"   , {1000 , 0., 200000. }},
  {"default", {10000, 0., 1000000.}}
};

std::unordered_map<dType, THBinning> Detectors::energy_bins = 
{
  {"ge"     , {10000, 0., 10000.}},
  {"bgo"    , {1000 , 0., 1000. }},
  {"labr"   , {1000 , 0., 10000.}},
  {"paris"  , {1000 , 0., 10000.}},
  {"eden"   , {1000 ,-2., 2.    }},
  {"dssd"   , {1000 , 0., 20000.}},
  {"default", {1000 , 0., 50000.}}
};

std::unordered_map<dType, THBinning> Detectors::energy_bidim_bins = 
{
  {"ge"     , {5000, 0., 10000.}},
  {"bgo"    , {100 , 0., 10000.}},
  {"labr"   , {500 , 0., 10000.}},
  {"paris"  , {500 , 0., 10000.}},
  {"eden"   , {1000,-2., 2.    }},
  {"dssd"   , {200 , 0., 20000.}},
  {"default", {200 , 0., 20000.}}
};
#endif

#endif //DETECTORS_HPP


// using dAlias = std::string;
// auto ge_a    = Detectors::alias::ge;
// auto bgo_a   = Detectors::alias::bgo;
// auto labr_a  = Detectors::alias::labr;
// auto paris_a = Detectors::alias::paris;
// auto dssd_a  = Detectors::alias::dssd;
// auto eden_a  = Detectors::alias::eden;
// auto RF_a    = Detectors::alias::RF;
// auto null_a  = Detectors::alias::null;


  /// @brief Loads the list of detectors
  // Detectors(std::string const & filename) {this -> load(filename);}

  // /// @brief Loads the list of detectors
  // Detectors(const char* filename) {this -> load(filename);}

  // Detectors& operator=(Detectors otherList)
  // {
  //   exist_array          = otherList.exist_array;
  //   m_ok            = otherList.m_ok;
  //   m_filename      = otherList.m_filename;
  //   m_list          = otherList.m_list;
  //   m_labels_array = otherList.m_labels_array;
  //   m_type_counter = otherList.m_type_counter;
  //   return *this;
  // }

  // static size_t nb_aliases() {return null_a;} // as Detector::alias::null is defined at the last position in enum, it holds the number of detectors
    // size_t const & nb_det_in_alias(dAlias const & alias) const {return m_type_counter[alias];}

  //   /**
  //  * @brief Handles the type of detector
  //  * @details If you whish to add a new type of detector, do it there
  // */
  // enum alias {ge, bgo, labr, paris, dssd, eden, RF, null};


  /**
   * @brief Returns alias from string (e.g. "RF" will return Detector::alias::rf)
  */
  // alias getAlias(std::string const & str)
  // {
  //   auto it = std::find(alias_str.begin(), alias_str.end(), str);
  //   return (it == std::end(alias_str)) ? null : static_cast<alias>(it-alias_str.begin());
  // }

  
  // static std::string type (Label const & label)
  // {
  //   if      (isGe    [label]) return "ge"   ;
  //   else if (isBGO   [label]) return "bgo"  ;
  //   else if (isLaBr3 [label]) return "labr";
  //   else if (isParis [label]) return "paris";
  //   else if (isDSSD  [label]) return "dssd" ;
  //   else if (isRF    [label]) return "RF"   ;
  //   else if (isEden  [label]) return "eden" ;
  //   else                      return "null" ;
  // }
