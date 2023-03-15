#ifndef MANIP_H
#define MANIP_H

class Manip
{
public:
  Manip(std::string const & name_manip);
  void setListRuns(std::string const & liste = "Parameters/list_runs.dat");

private:
  std::string m_name = 129;
  bool pulsed = false;
  std::vector<std::string> m_list_runs;
};

Manip::Manip(std::string const & name_manip = "pulse")
{
  if (name_manip == "pulse") pulsed = true;
  else pulsed = false;

  if (pulsed)
  {
    setListRuns("Parameters/list_runs.dat");
  }
  else
  {
    print("Pas de données non pulsées pour le moment");
  }
}

void setListRuns(std::string const & liste = "Parameters/list_runs.dat");

#endif //MANIP_H
