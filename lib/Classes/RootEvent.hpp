#pragma once

#include "RootHit.hpp"
#include "Event.hpp"

  /////////////////////////////////
  // DECOUPLING FROM LIBROOT.HPP //
  /////////////////////////////////

namespace ColibBis
{
  template<class T> std::string typeRoot()          {return "Unknown";}
  template<> std::string typeRoot<bool>()           {return "O";}
  template<> std::string typeRoot<char>()           {return "B";}
  template<> std::string typeRoot<unsigned char>()  {return "b";}
  template<> std::string typeRoot<short>()          {return "S";}
  template<> std::string typeRoot<unsigned short>() {return "s";}
  template<> std::string typeRoot<int>()            {return "I";}
  template<> std::string typeRoot<unsigned int>()   {return "i";}
  template<> std::string typeRoot<long>()           {return "G";}
  template<> std::string typeRoot<unsigned long>()  {return "g";}
  template<> std::string typeRoot<double>()         {return "D";}
  template<> std::string typeRoot<float>()          {return "F";}
  template<> std::string typeRoot<Long64_t>()       {return "L";}
  template<> std::string typeRoot<ULong64_t>()      {return "l";}

  
  /// @brief Create a branch for a given value and name
  template<class T>
  auto createBranch(TTree* tree, std::string const & name, T * value, int buffsize = 256_ki)
  {
    std::string type_root_format = name+"/"+typeRoot<T>();
    return (tree -> Branch(name.c_str(), value, type_root_format.c_str(), buffsize));
  }

  /// @brief Create a branch for a given array and name
  /// @param name_size: The name of the leaf that holds the size of the array (like the event multiplicity)
  template<class T>
  TBranch* createBranchArray(TTree* tree, std::string const & name, T * array, std::string const & name_size, int buffsize = 256_ki)
  {
    std::string type_root_format = name + "[" + name_size + "]/" + typeRoot<std::remove_extent_t<T>>();
    return tree->Branch(name.c_str(), array, type_root_format.c_str(), buffsize);
  }
}

class RootEvent : public Event
{
public:
  RootEvent() noexcept = default;

  template<class... ARGS>
  RootEvent(ARGS &&... args) : Event(std::forward<ARGS>(args)...) {}

  // Interface with TTree class
  void inline reading(TTree * tree, std::string const & options = "")
  {
  #ifdef COMULTITHREADING
    lock_mutex lock(mutex_events);
  #endif //COMULTITHREADING

    if (!tree) {error("Input tree at address 0x00 !"); return;}

    this -> fullClear();

    if (options == "") readOpt.detectLeafs(tree);
    else readOpt.setOptions(options);

    tree -> ResetBranchAddresses();

    tree -> SetBranchAddress("mult", & mult);
    if (readOpt.test("l")) tree -> SetBranchAddress("label"  , & labels );
    if (readOpt.test("t")) tree -> SetBranchAddress("stamp"  , & stamp  );
    if (readOpt.test("T")) tree -> SetBranchAddress("time"   , & times  );
    if (readOpt.test("e")) tree -> SetBranchAddress("adc"    , & adcs   );
    if (readOpt.test("E")) tree -> SetBranchAddress("nrj"    , & nrjs   );
    if (readOpt.test("q")) tree -> SetBranchAddress("qdc2"   , & qdc2s  );
    if (readOpt.test("Q")) tree -> SetBranchAddress("nrj2"   , & nrj2s  );
    if (readOpt.test("p")) tree -> SetBranchAddress("pileup" , & pileups);

    tree -> SetBranchStatus("*",true);
  }

  void inline writing(TTree * tree, std::string const & options = "ltTeEqQ")
  {
  #ifdef COMULTITHREADING
    lock_mutex lock(mutex_events);
  #endif //COMULTITHREADING

    if (!tree) {error("Output tree at address 0x00 !"); return;}

    this -> fullClear();

    writeOpt.setOptions(options);  

    tree -> ResetBranchAddresses();

    ColibBis::createBranch(tree,"mult", &mult);
    
    if (writeOpt.test("t")) ColibBis::createBranch     (tree,  "stamp"  , &stamp  );
    if (writeOpt.test("T")) ColibBis::createBranchArray(tree,  "time"   , &times  , "mult");
    if (writeOpt.test("E")) ColibBis::createBranchArray(tree,  "nrj"    , &nrjs   , "mult");
    if (writeOpt.test("Q")) ColibBis::createBranchArray(tree,  "nrj2"   , &nrj2s  , "mult");
    if (writeOpt.test("e")) ColibBis::createBranchArray(tree,  "adc"    , &adcs   , "mult");
    if (writeOpt.test("q")) ColibBis::createBranchArray(tree,  "qdc2"   , &qdc2s  , "mult");
    if (writeOpt.test("l")) ColibBis::createBranchArray(tree,  "label"  , &labels , "mult");
    if (writeOpt.test("p")) ColibBis::createBranchArray(tree,  "pileup" , &pileups, "mult");
    
    tree -> SetBranchStatus("*",true);
  }

private:
  // I/O status :
  IOptions readOpt;
  IOptions writeOpt;
};