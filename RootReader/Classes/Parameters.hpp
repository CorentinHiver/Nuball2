#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "../../lib/MTObjects/MTList.hpp"
#include "../../lib/Analyse/Sorted_Event.hpp"

class Parameters
{
public:
  Parameters(){};
  Parameters(int argc, char ** argv){setParameters(argc, argv);};

  // Deals with the parameters file :
  bool readParameters(std::string const & file = "Parameters/rootReader.setup");
  bool setParameters(int argc, char ** argv);
  bool checkParameters();
  bool setData();
  // To retrieve the parameters :
  std::vector<std::string> getParameters(std::string const & module = "all");

  // Get the variables
  auto const & threadsNb() const {return m_nbThreads;}
  FilesManager & files () {return m_files;}

  MTList<std::string> & filesMT() {return m_list_files;}
  MTList<std::string> & getRunsList() {return m_list_runs;}
  bool getNextFile(std::string & filename) {return m_list_files.getNext(filename);}
  bool getNextRun(std::string & run) {return m_list_runs.getNext(run);}
  bool getNextRun(std::string & run, size_t & run_index) {return m_list_runs.getNext(run, run_index);}

  ListFiles const & getRunFiles(std::string const & run)
  {
    return m_files.getFilesInFolder(m_dataPath+run);
  }

  std::string const & getDataPath() {return m_dataPath;}

  Timer totalTime;
  MTCounter totalCounter;
  MTCounter totalFilesSize;

  void printPerformances()
  {
    int time = totalTime(); // if we use print, we have to get the value before so the unit() is changing accordingly
    print("Analysis of", totalCounter()/1000000., "Mevts (", totalFilesSize(), "Mo) performed in", time, totalTime.unit(),
    "->", totalCounter/totalTime.TimeSec()/1000000., "Mevts/s (", totalFilesSize/totalTime.TimeSec(), "Mo/s)");
  }

private:
  std::string m_parameters_file;

  std::map<std::string, std::vector<std::string>> m_parameters; // key : module, value : list of module's parameters

  size_t m_nbThreads = 1;

  std::string m_dataPath;
  FilesManager m_files;
  MTList<std::string> m_list_files;
  MTList<std::string> m_list_runs;
  bool m_TW_correct = false;
  std::string m_TW_file;

  // Parameters :
  Float_t m_max_time = 9999999.;
};

bool Parameters::checkParameters()
{
  return true;
}

std::vector<std::string> Parameters::getParameters(std::string const & module)
{
  if(module == "all")
  {
    std::vector<std::string> ret;
    for (auto const & parameter : m_parameters) for (auto const & param : parameter.second) ret.push_back(param);
    return ret;
  }
  else
  {
    return m_parameters[module];
  }
}

bool Parameters::readParameters(std::string const & file)
{
  m_parameters_file = file;
  std::ifstream f(m_parameters_file);
  if (f && f.is_open())
  {
    std::stringstream parameters_file;
    parameters_file << f.rdbuf();
    f.close();

    std::string current_param;
    bool current_param_on = false;

    while (parameters_file.good())
    {
      std::string parameter;
      std::string temp1, temp2;
      getline(parameters_file, parameter);
      std::istringstream is(parameter);
      is.clear();
      temp1 = "NULL"; temp2 = "NULL";
      is>>temp1;
      if (temp1[0]=='#' || temp1[1]=='#') continue;

      // ****************** //
      //        LOAD        //
      // ****************** //
      if ( parameter.front() == '[' && parameter.back()==']')
      {// Chosing the module
        parameter.pop_back();
        parameter.erase(0,1); //"pop_front"
        current_param_on = true;
        current_param = parameter;
        continue;
      }

      else if (temp1.front() == '#' || temp1.substr(0,2) == "//") continue;

      else if (current_param_on == true)
      {
        if (temp1 == "end") current_param_on = false;
        else m_parameters[current_param].push_back(parameter);
      }
    }
  }
  else {print("Can't read the parameter file !");return false;}
  if (!this -> setData()) return false;

  if(m_nbThreads > m_files.size())
  {
    std::cout << "Number of threads too large (too few files to be processed) -> reset to " << m_files.size() << std::endl;
    m_nbThreads = m_files.size();
  }

  return true;
}

bool Parameters::setData()
{
  for (auto const param : m_parameters["Data"])
  {
    std::istringstream is(param);
    std::string temp = "NULL";
    while (is >> temp)
    {
      if (temp == "list_runs:")
      {
        is >> temp;
        m_list_runs = listFileReader(temp);
      }
      else if (temp == "folder:")
      {
        is >> m_dataPath;
        if (m_dataPath.back() != '/') m_dataPath.push_back('/');
      }
      else if (temp == "correctTW" || temp == "timewalk")
      {
        m_TW_correct = true;
        is >> m_TW_file;
      }
      else if (temp == "reject:")
      {
        is >> temp;
        if (temp == "time:")
        {
          is >> temp;
          if (temp == "sup:")
          {
            is >> m_max_time;
            Sorted_Event::setMaxTime(m_max_time);
            print("Rejecting events with Time >",m_max_time,"ns");
          }
        }
      }
      else {print(temp,"parameter unkown for Data module!!");return false;}
    }
  }

  if(m_list_runs.size()<1) { print("No runs list !"); return false;}
  for (auto const & run : m_list_runs) m_files.addFolder(m_dataPath+run);
  m_list_files = m_files.getListFiles();

  if (m_TW_correct) Sorted_Event::setTWcorrectionsDSSD(m_TW_file);

  return true;
}

bool Parameters::setParameters(int argc, char ** argv)
{
  for (int i = 1; i<argc; i++)
  {
        if (strcmp(argv[i], "-m") == 0)
    {
      i++;
      m_nbThreads = atoi(argv[i]);
      print("Number of threads : ", (int)m_nbThreads);
    }
    else if (strcmp(argv[i],"-d")==0)
    {
      i++;
      if (argv[i+1][0]=='-')  {m_files.addFolder(argv[i]);} // if it is the only arguemnt of -d
      else {m_files.addFolder(argv[i], atoi(argv[i+1]));i++;}
    }
    else if (strcmp(argv[i],"-f")==0)
    {
      i++;
      m_files.addFiles(argv[i]);
    }
  }
  return true;
}

#endif //PARAMETERS_H
