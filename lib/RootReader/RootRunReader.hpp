#pragma once

#include "RootReader.hpp"
#include "../Classes/RunReader.hpp"

class RootRunReader : public RunReader
{
public:
  void printArgs ()
  {
    print("Arguments of RootRunReader :");
    print();  
  }
  void readArgs(Arguments & args)
  {
    if (p_noArgs) printArgs();
    else while(args.next()) if (!RunReader::processArg(args))
    {
      // if (args == "-n")
    }
  }

  RootRunReader(Arguments & args) : RunReader()
  {
    readArgs(args);
    loadConfig(); 
  }
  
  RootRunReader(int argc, char** argv) : RunReader()
  {
    Arguments args(argc, argv);
    readArgs(args);
    loadConfig(); 
  }

  void loadConfig() noexcept override
  {
    if (0 < p_nbMaxHits) setMaxHits(p_nbMaxHits);
    // p_nbTotMaxHits
  }

  void setMaxHits(int nb) {m_reader.setMaxHits(nb);}

  void run()
  {

  }

private:
  RootReader m_reader;
};