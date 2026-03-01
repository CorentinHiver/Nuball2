#pragma once

#include "IOptions.hpp"
#include "Hit.hpp"

class RootHit : public Hit
{
#ifdef CoMT
  inline static std::mutex mutex_hits;
#endif // CoMT

public:
  template<class... ARGS>
  RootHit(ARGS &&... args) : Hit(std::forward<ARGS>(args)...) {}

  void reading(TTree * tree, std::string const & options = "")
  {
  #ifdef CoMT
    lock_mutex lock(mutex_hits);
  #endif // CoMT

    if (!tree) {print("Input tree at address 0x00 !"); return;}

    this -> clear();

    if (options == "") readOpt.detectLeafs(tree);
    else readOpt.setOptions(options);

    tree -> ResetBranchAddresses();

    if (readOpt.test("l")) tree -> SetBranchAddress("label"  , & label  );
    if (readOpt.test("t")) tree -> SetBranchAddress("stamp"  , & stamp  );
    if (readOpt.test("T")) tree -> SetBranchAddress("time"   , & time   );
    if (readOpt.test("e")) tree -> SetBranchAddress("adc"    , & adc    );
    if (readOpt.test("E")) tree -> SetBranchAddress("nrj"    , & nrj    );
    if (readOpt.test("q")) tree -> SetBranchAddress("qdc2"   , & qdc2   );
    if (readOpt.test("Q")) tree -> SetBranchAddress("nrj2"   , & nrj2   );
    if (readOpt.test("3")) tree -> SetBranchAddress("qdc3"   , & qdc3   );
    if (readOpt.test("R")) tree -> SetBranchAddress("nrj3"   , & nrj3   );
    if (readOpt.test("p")) tree -> SetBranchAddress("pileup" , & pileup );
  }

  void writing(TTree * tree, std::string const & options = "lteqp")
  {
  #ifdef CoMT
    lock_mutex lock(mutex_hits);
  #endif // CoMT

    if (!tree) {print("Input tree at address 0x00 !"); return;}

    writeOpt.setOptions(options);

    tree -> ResetBranchAddresses();

    if (writeOpt.test("l")) tree -> Branch("label"  , & label  );
    if (writeOpt.test("t")) tree -> Branch("stamp"  , & stamp  );
    if (writeOpt.test("T")) tree -> Branch("time"   , & time   );
    if (writeOpt.test("e")) tree -> Branch("adc"    , & adc    );
    if (writeOpt.test("E")) tree -> Branch("nrj"    , & nrj    );
    if (writeOpt.test("q")) tree -> Branch("qdc2"   , & qdc2   );
    if (writeOpt.test("Q")) tree -> Branch("nrj2"   , & nrj2   );
    if (writeOpt.test("3")) tree -> Branch("qdc3"   , & qdc3   );
    if (writeOpt.test("R")) tree -> Branch("nrj3"   , & nrj3   );
    if (writeOpt.test("p")) tree -> Branch("pileup" , & pileup );
  }
private:

  IOptions readOpt;
  IOptions writeOpt;
};
