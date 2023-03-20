#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "../../lib/MTObjects/MTList.hpp"

class Parameters
{
public:
  Parameters(){};
  Parameters(int argc, char ** argv){setParameters(argc, argv);};
  bool readParameters(std::string const & file = "Parameters/rootReader.setup");

  bool setParameters(int argc, char ** argv);

  std::string & get_parameters_ed() {return m_param_ed;}
  std::string & get_parameters_ma() {return m_param_ma;}
  std::string & get_parameters_rc() {return m_param_rc;}
  std::string & get_parameters_ai() {return m_param_ai;}

  auto const & threadsNb() const {return m_nbThreads;}
  // Get the variables
  FilesManager & files () {return m_files;}

  MTList<std::string> & filesMT() {return m_list_files;}
  bool getNextFile(std::string & filename) {return m_list_files.getNext(filename);}

  MTCounter<size_t> totalCounter;
private:
  uchar m_nbThreads = 1;

  FilesManager m_files;
  MTList<std::string> m_list_files;
  MTList<std::string> m_list_runs;

  std::string m_parameters_file;

  std::string m_param_ed;
  std::string m_param_ma;
  std::string m_param_rc;
  std::string m_param_ai;
};


bool Parameters::readParameters(std::string const & file)
{
  m_parameters_file = file;
  std::ifstream f(m_parameters_file);
  if (f && f.is_open())
  {
    std::stringstream parameters_file;
    parameters_file << f.rdbuf();
    f.close();
    bool ed = false;
    bool ma = false;
    bool rc = false;
    bool ai = false;
    bool da = false;
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
      if ( temp1.front() == '[' && temp1.back()==']')
      {// Chosing the module
        temp1.pop_back();
        temp1.erase(0,1); //"pop_front"
        if (temp1 == "EachDetector") ed = true;
        else if (temp1 == "Matrices") ma = true;
        else if (temp1 == "Run Check") rc = true;
        else if (temp1 == "Isomer") ai = true;
        else if (temp1 == "Data") da = true;
      }

      else if (temp1.front() == '#' || temp1.substr(0,2) == "//") continue;

      else if (ed)
      {
        if (temp1 == "end") ed = false;
        else m_param_ed+=parameter+"\n";
      }
      else if (ma)
      {
        if (temp1 == "end") ma = false;
        else m_param_ma+=parameter+"\n";
      }
      else if (rc)
      {
        if (temp1 == "end") rc = false;
        else m_param_rc+=parameter+"\n";
      }
      else if (ai)
      {
        if (temp1 == "end") ai = false;
        else m_param_ai+=parameter+"\n";
      }
      else if (da)
      {
        if (temp1 == "end") da = false;
        else if (temp1 == "list_runs:")
        {
          is >> temp2;
          m_list_runs = listFileReader(temp2);
        }
        else if (temp1 == "folder:")
        {
          if(m_list_runs.size()<1) { print("Set the list of runs before setting the folder !"); return false;}
          is >> temp2;
          for (auto const & run : m_list_runs) m_files.addFolder(temp2+run);
          m_list_files = m_files.getListFiles();
        }
        else {print(temp1,"parameter unkown !!");return -1;}
      }
    }
  }
  else {print("Can't read the parameter file !");return false;}
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
      print("Number of threads : ", m_nbThreads);
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
