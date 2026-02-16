#pragma once

#include "TTree.h"
#include "../Nuball2.hh"

/**
 * @brief 
 * ReadIO options. All branches are false by default and need to be activated, 
 * except mult that is true by default and needs to be deactivated
 * @details
 * 
 * legend 
 *    symbol : branch_name  full name  SI_unit  c++_type  default_value
 * 
 * m : mult   multiplicity (events)  N/A int       true
 * l : label  label                  N/A ushort    false
 * t : stamp  absolute timestamp     ps  ULong64_t false
 * T : time   relative time          ps  Long64_t  false
 * e : adc    energy                 ADC int       false
 * E : nrj    energy                 keV float     false
 * q : qdc2   energy qdc2            ADC int       false
 * Q : nrj2   energy qdc2            keV float     false
 * 3 : qdc3   energy qdc3            ADC int       false
 * R : nrj3   energy qdc3            keV float     false
 * p : pileup pileup                 N/A bool      false
 */
struct IOptions
{
  IOptions() noexcept { reset(); }
  IOptions(std::string const & options) { setOptions(options); }

  void reset() noexcept
  {
    m_state.reset();
  }

  void setOptions(std::string const & options)
  {
    reset();
    for (char option : options)
    {
      auto const index = Colib::findIndex(fieldNames, option);
      if (index == fieldNames.size()) throw std::invalid_argument("Unknown parameter '" + std::string(1, option) + "' for io data");
      else set(index);
    }
    set(s);
  }

  std::string getOptions() const noexcept 
  {
    std::string out;

    if (is_set())
    {
      for (size_t field_i = 0; field_i<11; ++field_i)
        if (test(field_i) ) out.push_back(fieldNames[field_i]);
    }

    return out;
  }

  std::vector<std::string_view> detectLeafs(TTree * tree)
  {
    std::vector<std::string_view> m_ret;
    if (!tree) return m_ret; // Safety check
    reset();
    
    TObjArray* branches = tree->GetListOfBranches();
    if (!branches) return m_ret;

    for (int i = 0; i < branches->GetEntries(); ++i) 
    {
      auto branch = dynamic_cast<TBranch*>(branches->At(i));
      if (!branch) continue;

      m_ret.push_back(branch->GetName());

      auto const index = Colib::findIndex(fieldRootNames, m_ret.back());
      
      if (index != fieldRootNames.size()) set(index);
    }
    set(s);
    set(r);
    return m_ret;
  }

  friend std::ostream& operator<<(std::ostream& out, IOptions const & options)
  {
    if (options.is_set())  
    {
      for (size_t field_i = 0; field_i<options.fieldNames.size(); ++field_i)
        if(options.test (field_i )) out << options.fieldNames[field_i] << " ";
      out << std::endl;
    }
    else 
    {
      out << "IOption not set" << std::endl;
    }
    return out;
  }

  bool test (uchar const bit) const {return m_state.test(bit);}
  bool test (std::string_view field) const {return test(Colib::findIndex(fieldNames, field[0]));}
  bool is_set() const {return test(s);}

  enum fields
  {
    m,  // multiplicity (events)
    l,  // label
    t,  // timestamp in ps
    T,  // relative time in ps
    e,  // energy in ADC
    E,  // calibrated energy in keV
    q,  // qdc2 in ADC
    Q,  // calibrated qdc2 in keV
    q3, // qdc3 in ADC
    Q3, // calibrated qdc3 in keV
    p,  // pileup
    s,  // is the option set or not
    r,  // read mode 
    w,   // write mode
    COUNT // To get the size of the enum
  };

  inline static constexpr std::array<char, COUNT> fieldNames =  {
    'm', 'l', 't', 'T', 'e', 'E', 'q', 'Q', '3', 'R', 'p', 's', 'r', 'w'};

  inline static constexpr std::array<std::string_view, 11> fieldRootNames =  {
    "mult" ,"label" ,"stamp" ,"time" ,"adc" ,"nrj" ,"qdc2" ,"nrj2" ,"qdc3" ,"nrj3" ,"pileup"};
  
private:
  void set  (uchar const bit) {m_state.set  (bit);}
  void unset(uchar const bit) {m_state.reset(bit);}

  std::bitset<COUNT> m_state;
};