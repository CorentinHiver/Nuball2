#ifndef OUTPUTTFILE_HPP
#define OUTPUTTFILE_HPP

#include "../libRoot.hpp"

class OutputTFile : public TFile
{
public:
  OutputTFile(const char* filename, Option_t* option = "", const char* title = "", Int_t compress = 1) : 
    TFile(const char* filename, Option_t* option = "", const char* title = "", Int_t compress = 1) 
  {
    if ()
  }
  virtual ~OutputTFile() {}
};


#endif //OUTPUTTFILE_HPP