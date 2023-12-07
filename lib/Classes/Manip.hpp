#ifndef MANIP_HPP
#define MANIP_HPP

#include "../libCo.hpp"
#include "../MTObjects/MTList.hpp"

class Manip
{
public:

  Manip(){}

  Manip(std::string const & runs_file) : m_runs_files(runs_file) {readFile(m_runs_files);}

  // void operator=(std::string const & datapath, std::string const & runs_file) 
  // {
  //   m_datapath = datapath;
  //   m_runs_files = runs_file;
  //   readFile ( runs_file );
  // }
  
  // Manip(std::string const & datapath, std::string const & manipname, std::string const & filename) 
  // {
  //   setDataPath(datapath);
  //   setManipName(manipname);
  //   setFileName(filename);
  //   readFile();
  // }

  void readFile(std::string const & runs_file)
  {
    std::ifstream file;
    file.open(runs_file);
    if (!file.good()){print(runs_file, "NOT FOUND !"); m_ok = false; return;}
    std::string line;
    while(file >> line) list_runs.push_back(line);
    file.close();
    if (m_MTOn) list_runs_MT = list_runs;
    m_ok = true;
  }

  auto const & get() const {return list_runs;}

  void setFolder(std::string const & folder)
  {
    list_runs.clear();
    list_runs.push_back(folder);
    if (m_MTOn) list_runs_MT = list_runs;
    m_ok = true;
  }

  void addFolder(std::string const & folder)
  {
    list_runs.push_back(folder);
  }

  bool readFile()
  {
    m_runs_files = m_datapath + m_manip + m_file;
    return readFile();
  }

  void setDataPath(std::string const & datapath)
  {
    m_datapath = datapath;
  }

  void setManipName(std::string const & manipname)
  {
    m_manip = manipname;
  }

  void setFileName(std::string const & filename)
  {
    m_file = filename;
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

  operator bool() const & {return m_ok;}

  void Print() { (m_MTOn) ? list_runs_MT.Print() : print(list_runs); }

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

  bool operator>>(std::string & string)
  {
    return this->getNext(string);
  }

  void reset() { i = 0; }

private:  
  uint i = 0;
  bool m_ok;
  bool m_MTOn = false; // Multithreading on

  Path m_datapath;
  std::string m_runs_files;
  Folder m_manip;
  std::string m_file;

  std::vector<std::string> list_runs;
  MTList list_runs_MT;
};

std::ostream& operator<<(std::ostream& cout, Manip const & manip)
{
  cout << manip.get();
  return cout;
}

#endif //MANIP_HPP
