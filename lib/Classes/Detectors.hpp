#ifndef DETECTORS_HPP
#define DETECTORS_HPP

#include "../libCo.hpp"

using Label = ushort;
using Labels = std::vector<Label>;
using LabelMap = std::map<std::string, Label>;


Bools isClover  (1000,false);
Bools isGe      (1000,false);
Bools isBGO     (1000,false);
Bools isLaBr3   (1000,false);
Bools isEden    (1000,false);
Bools isRF      (1000,false);
Bools isParis   (1000,false);
Bools isDSSD    (1000,false);
Bools isSector  (1000,false);
Bools isS1      (1000,false);
Bools isS2      (1000,false);
Bools isRing    (1000,false);

Bools isBack    (1000,false);
Bools isFront   (1000,false);

Labels labelToClover(1000,0);
Labels labelToBGOcrystal(1000,0);
Labels labelToGecrystal(1000,0);
Strings clover_pos  (1000,"");

Labels compressedLabel(1000,-1); // Used to put all the detectors one after the other

#include "../Analyse/Clovers.hpp"

/**
 * @brief Contains the alias and name of detectors types (e.g. Germanium, labr, ...)
 */
namespace Detector 
{
  /**
   * @brief Handles the type of detector
  */
  enum alias {ge, bgo, labr, paris, dssd, eden, RF, null};

  /**
   * @brief Labels the type of detectors
  */
  Strings alias_str {"ge", "bgo", "labr", "paris", "dssd", "eden", "RF", "null"};

  /**
   * @brief Returns alias from string (e.g. "RF" will return Detector::alias::rf)
  */
  alias getAlias(std::string const & str)
  {
    auto it = std::find(alias_str.begin(), alias_str.end(), str);
    return (it == std::end(alias_str)) ? null : static_cast<alias>(it-alias_str.begin());
  }
}

using dAlias = Detector::alias;
// using dAlias_str = Detector::alias_str;

/**
 * @brief 
 * 
 */
class Detectors
{
public:
  Detectors(){}

  /// @brief Loads the list of detectors
  Detectors(std::string const & filename) : m_filename(filename) {this -> load(m_filename);}

  /// @brief Loads the list of detectors
  Detectors(const char* filename) : m_filename(filename) {this -> load(m_filename);}

  /// @brief Copy constructor
  Detectors(Detectors const & otherList) : 
    exists (otherList.exists), 
    m_ok(otherList.m_ok), 
    m_filename(otherList.m_filename),
    m_list(otherList.m_list), 
    m_reversed_list(otherList.m_reversed_list) 
  {}

  /// @brief Reads the file and extracts the list of detectors, then fills the lookup tables
  void load (std::string const & filename);

  /// @brief Reads the file and extracts the list of detectors
  void readFile(std::string const & file);

  /// @brief Fills the lookup tables
  void makeArrays();

  /// @brief Return the value of the maximum label, i.e. the size of the lookup tables
  auto const size() const {return m_list.size();}

  /// @brief  Return the real number of detectors
  static auto const number() {return nb_detectors;}

  // Iterate over the existing detectors :
  auto begin() {return m_list.begin();}
  // Iterate over the existing detectors :
  auto end  () {return m_list.end  ();}

  auto const & get()        const {return m_list         ;}
  auto const & existsList() const {return exists         ;}
  auto const & labelsList() const {return m_reversed_list;}
  auto const & reverse()    const {return m_reversed_list;}

  auto const & getName (Label       const & label) {return m_list[label]        ;}
  auto const & getLabel(std::string const & name ) {return m_reversed_list[name];}

  void operator=(std::string const & filename) {this -> load(filename);}
  Detectors& operator=(Detectors otherList)
  {
    exists          = otherList.exists;
    m_ok            = otherList.m_ok;
    m_filename      = otherList.m_filename;
    m_list          = otherList.m_list;
    m_reversed_list = otherList.m_reversed_list;
    return *this;
  }

  std::string const & operator[] (int i) const {return m_list[i];}
  operator bool() const & {return m_ok;}

  static std::string type (Label const & label)
  {
    if      (isGe    [label]) return "ge"   ;
    else if (isBGO   [label]) return "bgo"  ;
    else if (isLaBr3 [label]) return "labr";
    else if (isParis [label]) return "paris";
    else if (isDSSD  [label]) return "dssd" ;
    else if (isRF    [label]) return "RF"   ;
    else if (isEden  [label]) return "eden" ;
    else                      return "null" ;
  }

  static dAlias alias (Label const & label)
  {
    if      (isGe    [label]) return dAlias::ge   ;
    else if (isBGO   [label]) return dAlias::bgo  ;
    else if (isLaBr3 [label]) return dAlias::labr;
    else if (isParis [label]) return dAlias::paris;
    else if (isDSSD  [label]) return dAlias::dssd ;
    else if (isRF    [label]) return dAlias::RF   ;
    else if (isEden  [label]) return dAlias::eden ;
    else                      return dAlias::null ;
  }

  Bools exists;

private:
  // Useful informations
  bool m_ok = false;
  bool m_loaded = false;
  static ushort nb_detectors;
  std::string m_filename;

  // Containers
  Strings m_list;
  LabelMap m_reversed_list;
};

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
    m_list.resize(0);
    print("CANNOT OPEN", filename, "!!");
    return;
  }
  else if (file_is_empty(inputfile))
  {
    m_ok = m_loaded = false;
    print(filename, "is empty !!");
    m_list.resize(0);
    return;
  }
  // ----------------------------------------------------- //
  //First reading of the file : extract the maximum label to be the size of the vector
  ushort size = 0; std::string line = "", name = ""; int label = 0;
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
  exists.resize(size+1);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while(getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1)
    {
      exists[label] = true;
      m_list[label] = name;
      if (name!="") m_reversed_list[name] = label;
    }
  }
  inputfile.close();
  print("Labels extracted from", filename);
  m_ok = m_loaded = true;
}

void Detectors::makeArrays()
{
  if (!m_loaded) {throw std::runtime_error("ID file not loaded !!"); return;}

    // The following is used to fill the compressedLabel array :

  // Looping around the labels
  for (auto label = 0ul; label<this->size(); label++)
  {
    std::istringstream is(replaceCharacter(m_list[label], '_', ' '));
    std::string str;


    // Looping around the subparts of the name "subpart_..._subpart"
    while(is >> str)
    {
      if (str == "red" || str == "green" || str == "black" || str == "blue" || str == "ge")
      {
        isGe    [label] = true;
      }
      else if (str == "BGO1" || str == "BGO2")
      {
        isBGO   [label] = true;
      }
      else if (str == "FATIMA")
      {
        isLaBr3 [label] = true;
      }
      else if (str == "PARIS" )
      {
        isParis [label] = true;
      }
      else if (str == "DSSD" )
      {
        isDSSD [label] = true;
      }
      else if (str == "EDEN"  )
      {
        isEden  [label] = true;
      }
      else if (str == "RF"    )
      {
        isRF    [label] = true;
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
      else if (str == "R")
      {
        isRing[label] = true;
      }

      // Additionnal positionnal information
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
    }

    // Other lookup tables :
    isClover[label] = label>22 && label<167;
    labelToClover[label] = (isClover[label]) ? (label-23)%6 : -1;
    // labelToBGOcrystal[label] = (isBGO[label]) ? (2*(label-23)/6 + (label+1)%6) : (-1);
     
    if (exists[label])
    {
      compressedLabel[label] = nb_detectors;
      nb_detectors++;
    }
    else compressedLabel[label] = -1;
  }
}

#endif //DETECTORS_HPP