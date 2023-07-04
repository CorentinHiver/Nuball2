#ifndef DETECTORS_HPP
#define DETECTORS_HPP

#include "../libCo.hpp"

using Label = ushort;

std::vector<bool> isClover  (1000,false); // Bit field 
std::vector<bool> isGe      (1000,false);
std::vector<bool> isBGO     (1000,false);
std::vector<bool> isLaBr3   (1000,false);
std::vector<bool> isEden    (1000,false);
std::vector<bool> isRF      (1000,false);
std::vector<bool> isParis   (1000,false);
std::vector<bool> isDSSD    (1000,false);
std::vector<bool> isSector  (1000,false);
std::vector<bool> isS1      (1000,false);
std::vector<bool> isS2      (1000,false);
std::vector<bool> isRing    (1000,false);

std::vector<bool> isBack(1000, true);
std::vector<bool> isFront(1000,true);

std::vector<Label> labelToClover(1000,0);
std::vector<std::string> clover_pos(1000,"");

namespace Detector 
{
  /**
   * @brief Handles the type of detector
  */
  enum det_alias {Ge, BGO, LaBr3, paris, dssd, Eden, RF, null};

  /**
   * @brief Labels the type of detectors
  */
  std::vector<std::string> det_str {"Ge", "BGO", "LaBr3", "paris", "dssd", "Eden", "RF", "null"};
}


class Detectors
{
public:
  Detectors(){}
  Detectors(std::string const & filename) : m_filename(filename) { this -> load(m_filename); }
  Detectors(Detectors const & otherList) : m_ok(otherList.m_ok), m_filename(otherList.m_filename), m_list(otherList.m_list), m_reversed_list(otherList.m_reversed_list) {  }
  void load (std::string const & filename);
  void makeArrays();

  bool good() {return  m_ok;}  bool is_good() {return  m_ok;}
  bool bad()  {return !m_ok;}  bool is_bad()  {return !m_ok;}

  auto const size() const {return m_list.size();}

  auto begin() {return m_list.begin();}
  auto end  () {return m_list.end  ();}

  auto const & labelsList() const {return m_reversed_list;}
  auto const & reverse()    const {return m_reversed_list;}
  auto const & get()        const {return m_list         ;}

  auto const & getName (Label      const & label) {return m_list[label]        ;}
  auto const & getLabel(std::string const & name ) {return m_reversed_list[name];}

  void operator=(std::string const & filename) {this -> load(filename);}
  Detectors& operator=(Detectors otherList)
  {
    m_reversed_list = otherList.labelsList();
    m_list = otherList.get();
    return *this;
  }

  std::string const & operator[] (int i) const {return m_list[i];}
  operator bool() const & {return m_ok;}

  static std::string type (Label const & label)
  {
    if      (isGe    [label]) return "Ge"   ;
    else if (isBGO   [label]) return "BGO"  ;
    else if (isLaBr3 [label]) return "LaBr3";
    else if (isParis [label]) return "paris";
    else if (isDSSD  [label]) return "dssd" ;
    else if (isRF    [label]) return "RF"   ;
    else if (isEden  [label]) return "Eden" ;
    else                      return "null" ;
  }

  static Detector::det_alias alias (Label const & label)
  {
    if      (isGe    [label]) return Detector::Ge   ;
    else if (isBGO   [label]) return Detector::BGO  ;
    else if (isLaBr3 [label]) return Detector::LaBr3;
    else if (isParis [label]) return Detector::paris;
    else if (isDSSD  [label]) return Detector::dssd ;
    else if (isRF    [label]) return Detector::RF   ;
    else if (isEden  [label]) return Detector::Eden ;
    else                      return Detector::null ;
  }

private:

  // Useful informations
  bool m_ok = false;
  bool m_loaded = false;
  bool m_arrays_ok = false;
  std::string m_filename;

  // Containers
  std::vector<std::string> m_list;
  std::map<std::string, Label> m_reversed_list;
};

void Detectors::load(std::string const & filename)
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
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while(getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1)
    {
      m_list[label] = name;
      if (name!="") m_reversed_list[name] = label;
    }
  }
  inputfile.close();
  print("Labels extracted from", filename);
  m_ok = m_loaded = true;
  this -> makeArrays();
}

void Detectors::makeArrays()
{
  if (!m_loaded) {throw std::runtime_error("ID file not loaded !!"); return;}

  for (auto label = 0ul; label<this->size(); label++)
  {
    isClover[label] = label>22 && label<167;
    std::istringstream is(replaceCharacter(m_list[label], '_', ' '));
    std::string str;
    while(is >> str)
    {
      if (str == "red" || str == "green" || str == "black" || str == "blue" || str == "Ge")
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

      // Additionnal position informations 
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

  // Other arrays :
  labelToClover[label] = (isClover[label]) ? (label-23)%6 : -1;
  }
  m_arrays_ok = true;
}

#endif //DETECTORS_HPP