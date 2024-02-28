#ifndef DETECTORS_HPP
#define DETECTORS_HPP

#include "../libRoot.hpp"
#include "Hit.hpp"

using dType = std::string;

#define SIZE_LOOKUP 1000

// All detectors lookup tables :
Bools isGe    (SIZE_LOOKUP);
Bools isBGO   (SIZE_LOOKUP);
Bools isLaBr3 (SIZE_LOOKUP);
Bools isEden  (SIZE_LOOKUP);
Bools isRF    (SIZE_LOOKUP);
Bools isParis (SIZE_LOOKUP);
Bools isDSSD  (SIZE_LOOKUP);

// -- Clover specific lookup tables -- //

Bools isClover(SIZE_LOOKUP);// is the 
std::vector<uchar> labelToClover(SIZE_LOOKUP,0);
std::vector<uchar> labelToBGOcrystal(SIZE_LOOKUP,0);
std::vector<uchar> labelToGecrystal(SIZE_LOOKUP,0);
Strings clover_pos (SIZE_LOOKUP,"");

// DSSD specific lookup tables :
Bools isSector(SIZE_LOOKUP);
Bools isS1    (SIZE_LOOKUP);
Bools isS2    (SIZE_LOOKUP);
Bools isRing  (SIZE_LOOKUP);
Bools isBack  (SIZE_LOOKUP);
Bools isFront (SIZE_LOOKUP);

// Other lookup tables :
Label_vec compressedLabel(SIZE_LOOKUP,-1); // Used to put all the detectors one after the other
Ints m_index(SIZE_LOOKUP,-1);


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

  Strings types_handled = {"ge", "bgo", "labr", "paris", "dssd", "eden", "RF", "default"};

  // static bool typeHandled(int const & id) const {} TODO
  // static bool typeID(int const & id) const {} TODO

  /// @brief Does this label correspond to a detector ?
  bool exists(Label const & label) {return m_exists[label];}

  /// @brief Reads the file and extracts the list of detectors, then fills the lookup tables 
  void load (std::string const & filename);

  /// @brief Reads the file and extracts the list of detectors
  void readFile(std::string const & file);

  /// @brief Fills the lookup tables
  void makeArrays();

  /// @brief Return the value of the maximum label, i.e. the size of the lookup tables
  Label size() const {return Label_cast(m_list.size());}

  /// @brief  Return the number of detectors
  auto const & number() {return m_nb_detectors;}

  // Iterate over the existing detectors :
  auto begin() {return m_list.begin();}
  auto end  () {return m_list.end  ();}

  auto begin() const {return m_list.begin();}
  auto end  () const {return m_list.end  ();}

  auto const & get()            const {return m_list          ;}
  auto const & list()           const {return m_list          ;}
  auto const & names()          const {return m_list          ;}
  auto const & getExistsArray() const {return m_exists        ;}
  auto const & getLabelsMap()   const {return m_labels_map    ;}
  auto const & labels()         const {return m_labels_vector ;}
  auto const & types()          const {return m_types_ID;}
  auto const & typesArray()     const {return m_types;}
  auto const & type(Label const & label) {return m_types[label];}
  auto const & typeIndex(dType const & type ) {return m_types_index[type];}
  auto const & typeIndex(Label const & label) {return m_types_index[this -> type(label)];}
  auto const & getTypeName(int const & type_i) const {return m_types_ID[type_i];}

  auto const & getName (Label       const & label) {return m_list[label]        ;}
  auto const & getLabel(std::string const & name ) {return m_labels_map[name];}
  auto const & label(std::string const & name )    {return m_labels_map[name];}

  void operator=(std::string const & filename) {this -> load(filename);}

  /// @brief Extracts the name of the detector given its global label
  auto const & operator[] (Label const & label) const {return m_list[label];}

  /// @brief Extracts the global label given the detector's name
  auto const & operator[] (std::string const & name) {return m_labels_map[name];}

  /// @brief Returns true only of the detectors ID file has been loaded successfully
  operator bool() const & {return m_ok;}

  /// @brief Returns the number of types in the ID file
  auto const nbTypes() {return m_types_ID.size();}

  /// @brief Returns the number of detector of each type
  auto const & nbOfType(dType const & type) 
  {
    if (!found(m_types, type)) throw_error("Detectors " + type + " not handled");
    return m_type_counter[type];
  }

  /// @brief Get the name of a detector given its type and type index
  auto const & name(dType const & type, int const & index) {return m_names[type][index];}

  /// @brief Get the global label of a detector given its type and type index
  auto const & label(dType const & type, int const & index) {return m_labels[type][index];}

  /// @brief Get the type index of the detector. Each type of detector has its own indexing system 
  /// if you have 3 Germaniums, the Ge indexes ranges from 0 to 2)
  auto const & index(Label const & label) const {return m_index[label];}

  /// @brief Get the default ADC histogram binning for each type of detectors
  static auto const & ADCBin(dType const & type = "")
  {
    auto const & it = ADC_bins.find(type);
    if (it != ADC_bins.end()) return it->second;
    else return ADC_bins["default"];
  }

  /// @brief Get the default energy (keV) histogram binning for each type of detectors
  static auto const & energyBin(dType const & type = "")
  {
    auto const & it = energy_bins.find(type);
    if (it != energy_bins.end()) return it->second;
    else return energy_bins["default"];
  }
  
  /// @brief Get the default energy (keV) bidimensionnal histogram binning for each type of detectors
  static auto const & energyBidimBin(dType const & type = "")
  {
    auto const & it = energy_bidim_bins.find(type);
    if (it != energy_bidim_bins.end()) return it->second;
    else return energy_bidim_bins["default"];
  }

  /// @brief Get the default energy (keV) bidimensionnal histogram binning for all types of detectors
  static auto & getADCBin() {return ADC_bins;}
  /// @brief Get the default energy (keV) bidimensionnal histogram binning for all types of detectors
  static auto & getEnergyBin() {return energy_bins;}
  /// @brief Get the default energy (keV) bidimensionnal histogram binning for all types of detectors
  static auto & getBidimBin() {return energy_bidim_bins;}

  /// @brief Prints out the list of labels
  // TODO : a more clever print with also the type and index
  void Print() {for (auto const & d : *this) if (d!="") {std::cout << d << " ";} std::cout << std::endl;}

  void resize(ushort const & new_size);

  auto const & file()       {return m_filename;}
  auto const & file() const {return m_filename;}

  
protected:

  // Useful informations :
  bool m_ok = false;
  bool m_loaded = false;
  bool m_initialized = false;
  ushort m_nb_detectors = 0;
  std::string m_filename;

  // Arrays :
  Strings m_types = Strings(SIZE_LOOKUP);
  Bools   m_exists;
  Strings m_list;
  Strings m_types_ID;

  std::vector<Label> m_labels_vector;
  std::unordered_map<std::string, Label> m_labels_map;
  std::unordered_map<dType, int> m_types_index;
  std::unordered_map<dType, int> m_type_counter; // To get the number of detectors of each type. 
  std::unordered_map<dType, Strings> m_names;
  std::unordered_map<dType, Label_vec> m_labels;

  // Binning informations :
  static std::unordered_map<dType, THBinning> energy_bins;
  static std::unordered_map<dType, THBinning> ADC_bins;
  static std::unordered_map<dType, THBinning> energy_bidim_bins;

  public:
  
  class Error
  { public:
    Error() noexcept {error("Detector ID file not loaded");}
    Error(std::string const & message) noexcept {error(message);}
  };
} detectors;

void Detectors::resize(ushort const & new_size)
{
  print("Maximum label is", new_size);
  m_exists.resize(new_size, false);
  m_list.resize(new_size, "");
  m_types.resize(new_size, "null");
}

std::ostream& operator<<(std::ostream& out, Detectors const & detectors)
{
  for (auto const & d : detectors) {out << d << " ";}
  return out;
}

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
    error(concatenate("Can't open ID file named '", filename, "'"));
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
  size++;// The size of the vector must be label_max+1
  // ----------------------------------------------------- //
  //Second reading : fill the vector
  m_list.resize(size);
  m_exists.resize(size);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while(getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1)
    {
      m_exists[label] = true;
      m_list[label] = name;
      if (name!="") 
      {
        m_labels_map[name] = label;
        m_labels_vector.push_back(label);
      }
    }
  }
  std::sort(m_labels_vector.begin(), m_labels_vector.end());
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
        isGe   [label] = true;
        m_types[label] = types_handled[0];
      }
      else if (str == "BGO1" || str == "BGO2"){
        isBGO  [label] = true;
        m_types[label] = types_handled[1];
      }
      else if (str == "FATIMA"){
        isLaBr3[label] = true;
        m_types[label] = types_handled[2];
      }
      else if (str == "PARIS" ){
        isParis[label] = true;
        m_types[label] = types_handled[3];
      }
      else if (str == "DSSD" ){
        m_types[label] = types_handled[4];
        isDSSD [label] = true;
      }
      else if (str == "EDEN"  ){
        m_types[label] = types_handled[5];
        isEden [label] = true;
      }
      else if (str == "RF"    ){
        m_types[label] = types_handled[6];
        isRF   [label] = true;
      }
      // Add here any additionnal detector :
      
      // Additionnal position information :
      if (str.size()>2 && str[0] == 'R' && str[2] == 'A')
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
     
    if (m_exists[label])
    {
      // The compressed labels is filled with the current number of detectors :
      compressedLabel[label] = m_nb_detectors;
      // Increment the number of detectors :
      m_nb_detectors++;

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

  // Make the list of all the detector types present in the ID file :
  for (auto const & type : types_handled) if (m_type_counter[type]>0) m_types_ID.push_back(type);
  
  // Reverse lookup : 
  for (size_t index = 0; index<m_types_ID.size(); index++) m_types_index[m_types_ID[index]] = int_cast(index);
}

std::unordered_map<dType, THBinning> Detectors::ADC_bins = 
{
  {"ge"     , {10000, 0., 200000. }},
  {"bgo"    , {1000 , 0., 200000. }},
  {"labr"   , {2000 , 0., 200000. }},
  {"paris"  , {2000 , 0., 200000. }},
  {"eden"   , {1000 , 0., 200000. }},
  {"dssd"   , {1000 , 0., 200000. }},
  {"default", {10000, 0., 1000000.}}
};

std::unordered_map<dType, THBinning> Detectors::energy_bins = 
{
  {"ge"     , {10000, 0., 10000.}},
  {"bgo"    , {1000 , 0., 10000.}},
  {"labr"   , {2000 , 0., 10000.}},
  {"paris"  , {2000 , 0., 10000.}},
  {"eden"   , {1000 ,-2., 2.    }},
  {"dssd"   , {1000 , 0., 20000.}},
  {"default", {1000 , 0., 50000.}}
};

std::unordered_map<dType, THBinning> Detectors::energy_bidim_bins = 
{
  {"ge"     , {5000, 0., 10000.}},
  {"bgo"    , {250 , 0., 10000.}},
  {"labr"   , {1000, 0., 10000.}},
  {"paris"  , {1000, 0., 10000.}},
  {"eden"   , {1000,-2., 2.    }},
  {"dssd"   , {200 , 0., 20000.}},
  {"default", {200 , 0., 20000.}}
};

std::map<Label, TH1F*> loadFormattedTH1F(TFile * file)
{
  if (!file || file->IsZombie()) throw_error(concatenate(file->GetName(), " can't be open !!"));
  if(!detectors) throw_error(concatenate("As the histograms are labeled with names, one has to provide the correct index.list file !! ",
                                        "Use parameter -i [filename] or detectors.load(filename)."));
  std::map<Label, TH1F*> ret;
  auto list_histo = get_TH1F_map(file);
  // ret.reserve(list_histo.size()); // Might be unnecessary/dangerous optimisation

  for (auto const & pair : list_histo)
  {
    std::string name = pair.first;
    auto const & histo = pair.second;
    Strings possible_additionnal_text = {"_adc", "_energy", "_calib", "_raw"};
    for (auto const & text : possible_additionnal_text) remove(name, text);
    if (found(detectors.names(), name))
    {// Either the histogram label is already the label in int, or is the name of the detector
      try                                 {ret.emplace(string_to<Label>(name), histo);}
      catch(CastImpossible const & error) {ret.emplace(       detectors[name], histo);}
    }
  }
  return ret;
}


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
  //   m_exists          = otherList.m_exists;
  //   m_ok            = otherList.m_ok;
  //   m_filename      = otherList.m_filename;
  //   m_list          = otherList.m_list;
  //   m_labels_map = otherList.m_labels_map;
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
