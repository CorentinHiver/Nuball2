#ifndef OUTPUTTFILE_HPP
#define OUTPUTTFILE_HPP

#include "../libRoot.hpp"

class OutputTFile : public TFile
{
public:
  OutputTFile(std::string filename, std::string option = "", std::string title = "", Int_t compress = 1) : 
    TFile(filename.c_str(), option.c_str(), title.c_str(), compress) 
  {
    if (!TFile::IsOpen())
    {
      pauseCo("Can't open", filename, "in recreate mode !! If already open, please close it if it is open then press a button");
      TFile::Open(filename.c_str(), option.c_str(), title.c_str(), compress);
    }
  }

  template<class... ARGS>
  Write(ARGS... args) override {Write(std::forward<ARGS>(args)...); print(m_name, "written");}

  virtual ~OutputTFile() {}
};


#endif //OUTPUTTFILE_HPP