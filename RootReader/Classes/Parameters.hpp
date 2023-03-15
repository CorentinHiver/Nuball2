#ifndef PARAMETERS_H
#define PARAMETERS_H


class Parameters
{
public:
  Parameters(){};
  Parameters(int argc, char ** argv){setParameters(argc, argv);};
  bool readParameters(std::string const & file = "Parameters/rootReader.setup");

  bool setParameters(int argc, char ** argv);

  std::string & get_ed_setup() {return m_param_ed;}
  std::string & get_ma_setup() {return m_param_ma;}
  std::string & get_rc_setup() {return m_param_rc;}

  // Get the variables
  FilesManager & files () {return m_files;}

private:
  FilesManager m_files;

  std::string m_param_ed;
  std::string m_param_ma;
  std::string m_param_rc;
};

bool Parameters::setParameters(int argc, char ** argv)
{
  if (strcmp(argv[1],"-d")==0)
  {
        if (argc == 3)
    {
      m_files.addFolder(argv[2]);
    }

    else if (argc == 4)
    {
      m_files.addFolder(argv[2], atoi(argv[3]));
    }

    else
    {
      print("ERREUR INPUT in -d mode"); return false;
    }

  }
  else if (strcmp(argv[1],"-f")==0)
  {
    if (argc == 3)
    {
      m_files.addFiles(argv[2]);
    }
  }
  else
  {
    print("ERREUR INPUT, wrong mode"); return false;
  }
  return true;
}

bool Parameters::readParameters(std::string const & file)
{
  std::ifstream f(file);
  if (f && f.is_open())
  {
    std::stringstream parameters_file;
    parameters_file << f.rdbuf();
    f.close();
    bool ed = false;
    bool ma = false;
    bool rc = false;
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

      if (temp1 == "EachDetector:") ed = true;
      else if (temp1 == "Matrices:") ma = true;
      else if (temp1 == "Run Check:") rc = true;

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
    }
  }
  else {print("Can't read the parameter file !");return false;}
  return true;
}

#endif //PARAMETERS_H
