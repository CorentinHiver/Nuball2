#ifndef MANIP_H
#define MANIP_H

#include <MTList.hpp>
#include <libCo.hpp>

class Manip
{
public:

  Manip(std::string const & runs_file) : m_runs_files(runs_file)
  {
    readFile ( runs_file );
  }

  void readFile(std::string const & runs_file)
  {
    std::ifstream file;
    file.open(runs_file);
    if (!file.good()){print(runs_file, "NOT FOUND !"); return;}
    std::string line;
    while(file >> line) list_runs.push_back(line);
    file.close();
    if (m_MTOn) list_runs_MT = list_runs;
  }

  bool setMT(bool const & MTOn = true)
  { // To enable the parallel reading of the runs
    if(MTObject::ON) return (m_MTOn = MTOn);
    else 
    {
      if (MTOn) print("MULTITHREADING ISN'T INITIALIZED !");
      return false;
    }
  }

  void Print()
  {
     if (m_MTOn) list_runs_MT.Print(); else print(list_runs);
  }

  bool getNext(std::string & run)
  {
    if (m_MTOn) return list_runs_MT.getNext(run);
    else 
    {
      if (i<list_runs.size())
      {
        run = list_runs[i++];
        return true;
      }
      else
      {
        return false;
      }
    }
  }

  void reset() { i = 0; }

private:  
  uint i = 0;
  bool m_MTOn = false; // Multithreading on
  std::string m_runs_files;
  std::vector<std::string> list_runs;
  MTList<std::string> list_runs_MT;
};


#endif //MANIP_H
