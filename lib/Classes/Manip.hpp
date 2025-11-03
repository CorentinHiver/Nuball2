#ifndef MANIP_HPP
#define MANIP_HPP

#include "../libCo.hpp"
#include "../MTObjects/MTList.hpp"

/// @brief Loads the list file containing the list of the names of the folder containing the faster files to convert
/// @todo make it better !!
class Manip
{
public:

  Manip() {if(MTObject::ON) m_MTOn  = true;}

  Manip(std::string const & data) : Manip()
  {
    if (extension(data) == "list") addListFile(data);
    else                           addPath     (data);
  }
  Manip(Path const & path, std::string const & data) {addListFile(path, data);}

  void addPath(std::string const & path)
  {
    print(path);
    if (found(path, "*"))
    {
      auto list_rus = findFilesWildcard(path);
    }
    else
    {
      m_listRuns.push_back(path);
    }
    if (m_MTOn) m_listRunsMT = m_listRuns;
  }

  void addListFile(std::string const & runs_file)
  {
    std::ifstream file(runs_file);
    if (!file.good()){print(runs_file, "NOT FOUND !"); return;}
    std::string line;
    while(file >> line) m_listRuns.push_back(line);
    file.close();
    if (m_MTOn) m_listRunsMT = m_listRuns;
  }

  void addListFile(Path const & path, std::string const & runs_file)
  {
    std::ifstream file(runs_file);
    if (!file.good()){print(runs_file, "NOT FOUND !"); return;}
    std::string line;
    while(file >> line) m_listRuns.push_back(path.string()+line);
    file.close();
    if (m_MTOn) m_listRunsMT = m_listRuns;
  }

  auto const & get() const {return m_listRuns;}

  // /// @brief Add a folders
  // void addFolder(std::string const & folder)
  // {
  //   m_listRuns.push_back(folder);
  //   print(m_listRuns);
  //   if (m_MTOn) m_listRunsMT.push_back(folder);
  // }

  // /// @brief Clears the previously set list of folders and replace it with folder.
  // void setFolder(std::string const & folder)
  // {
  //   m_listRuns.clear();
  //   m_listRuns.push_back(folder);
  //   if (m_MTOn) m_listRunsMT = m_listRuns;
  //   m_ok = true;
  // }

  // /// @brief Reads the list file containing the list of folders
  // void readListFile()
  // {
  //   m_runs_files = m_dataPath + m_manip + m_file;
  //   readListFile(m_runs_files);
  // }

  // void setFileName(std::string const & filename)
  // {
  //   m_file = filename;
  // }

  ///@brief To allow several threads to work on different runs on parallel
  ///@details MTObject needs to have been Initialised before creating the 
  bool setMT(bool const & MTOn = true)
  { // To enable the parallel reading of the runs
    if(MTObject::ON) return (m_MTOn = MTOn);
    else 
    {
      if (MTOn) print("COMULTITHREADING ISN'T Initialised !");
      return false;
    }
  }

  operator bool() {return (m_listRuns.size()>0);}

  bool getNext(std::string & run)
  {
    if (m_MTOn) return m_listRunsMT.getNext(run);
    else 
    {
      if (m_cursor<m_listRuns.size())
      {
        run = m_listRuns[m_cursor++];
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

  void reset() { m_cursor = 0; }

  
  friend std::ostream& operator<<(std::ostream& out, Manip const & manip)
  {
    out << manip.get();
    return out;
  }

private:  
  uint m_cursor = 0;
  bool static inline m_MTOn = false; // Multithreading ON

  // Path m_dataPath;
  // Folder m_manip;
  // std::string m_file;

  Colib::Strings m_listRuns;
  MTList m_listRunsMT;
};


#endif //MANIP_HPP
