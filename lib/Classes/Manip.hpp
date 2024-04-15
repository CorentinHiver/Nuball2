#ifndef MANIP_HPP
#define MANIP_HPP

#include "../libCo.hpp"
#include "../MTObjects/MTList.hpp"

/// @brief Loads the list file containing the list of the names of the folder containing the faster files to convert
/// @todo make it better !!
class Manip
{
public:

  Manip(){}

  Manip(std::string const & runs_file) : m_runs_files(runs_file) {readFile(m_runs_files);}

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

  /// @brief Clears the previously set list of folders and replace it with folder.
  void setFolder(std::string const & folder)
  {
    list_runs.clear();
    list_runs.push_back(folder);
    if (m_MTOn) list_runs_MT = list_runs;
    m_ok = true;
  }

  /// @brief Clears the previously set list of folders and replace it with folder.
  void addFolder(std::string const & folder)
  {
    list_runs.push_back(folder);
    if (m_MTOn) list_runs_MT.push_back(folder);
  }

  /// @brief Reads the list file containing the list of folders
  void readFile()
  {
    m_runs_files = m_dataPath + m_manip + m_file;
    readFile(m_runs_files);
  }

  /// @brief @deprecated Set the path of the list file containing the list of folders
  void setDataPath(std::string const & dataPath)
  {
    m_dataPath = dataPath;
  }

  /// @brief @deprecated Set the name of the folder inside the dataPath containing the list file containing the list of folders
  void setManipName(std::string const & manipName)
  {
    m_manip = manipName;
  }

  void setFileName(std::string const & filename)
  {
    m_file = filename;
  }

  ///@brief To allow several threads to work on different runs on parallel
  ///@details MTObject needs to have been Initialised before creating the 
  bool setMT(bool const & MTOn = true)
  { // To enable the parallel reading of the runs
    if(MTObject::ON) return (m_MTOn = MTOn);
    else 
    {
      if (MTOn) print("MULTITHREADING ISN'T InitialiseD !");
      return false;
    }
  }

  operator bool() const & {return m_ok;}

  // void Print() { (m_MTOn) ? list_runs_MT.Print() : print(list_runs); }

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
  bool m_ok = false;
  bool m_MTOn = false; // Multithreading on

  Path m_dataPath;
  std::string m_runs_files;
  Folder m_manip;
  std::string m_file;

  std::vector<std::string> list_runs;
  MTList list_runs_MT;

public:
  class NotFound
  { public:
    NotFound(Manip const & manip) 
    {
      print("the manip at", manip.m_manip.string(), "is not okay ...");
    }
  };
};

std::ostream& operator<<(std::ostream& cout, Manip const & manip)
{
  cout << manip.get();
  return cout;
}

#endif //MANIP_HPP
