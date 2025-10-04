#ifndef PARISBIDIMANGLES_HPP
#define PARISBIDIMANGLES_HPP

// Dealing with the Qshort VS Qlong matrix
class ParisBidimAngles
{
public:
  ParisBidimAngles() noexcept {};
  ParisBidimAngles(std::string const & filename) : m_filename(filename) {load(filename);}
  
  void load(std::string const & filename);
  void write(std::string const & filename);
  void calculate(std::string const & filename);

  std::pair<double, double> const & operator[] (std::string name) {return m_angles.at(name);}
  
  double const & angleNaI (std::string name) const {return m_angleNaI .at(name);}
  double         angleNaI (std::string name)       {return m_angleNaI .at(name);}
  double const & angleLaBr(std::string name) const {return m_angleLaBr.at(name);}
  double         angleLaBr(std::string name)       {return m_angleLaBr.at(name);}

  void set(std::string const & name, double const & angleLaBr, double const & angleNaI);

  auto const & names() const {return m_names;}
  auto         names()       {return m_names;}

  void sortNames() {std::sort(m_names.begin(), m_names.end());}

  void printDeg()
  {
    this->sortNames();

    std::cout << "name angle_NaI angle_LaBr3" << std::endl;
    for (auto const & name : m_names) std::cout << name << " " << this->angleLaBr(name)/3.141596*180 
                                                << " " << this->angleNaI(name)/3.141596*180 << std::endl;
  }

  std::unordered_map<std::string, double> const & LaBr_angles() {return m_angleLaBr;}
  std::unordered_map<std::string, double> const & NaI_angles () {return m_angleNaI ;}

  operator bool() const {return m_ok;}

private:

  bool m_ok = false;
  std::string m_filename;
  static std::string m_extension;

  std::vector<std::string> m_names;
  std::unordered_map<std::string, double> m_angleLaBr;
  std::unordered_map<std::string, double> m_angleNaI;
  std::unordered_map<std::string, std::pair<double, double>> m_angles; // First angle is LaBr3 and second angle NaI

public:
  class FileError
  {public:
    FileError(std::string const & filename) {error("No PARIS file found at", filename);}
  };
};

std::string ParisBidimAngles::m_extension = "angles";

std::ostream& operator<<(std::ostream& out, ParisBidimAngles & angles)
{
  angles.sortNames();

  out << "name angle_NaI angle_LaBr3" << std::endl;
  for (auto const & name : angles.names()) out << name << " " << angles.angleNaI(name) << " " << angles.angleLaBr(name) << std::endl;
  
  return out;
}

std::ostream& operator<<(std::ostream& out, ParisBidimAngles const & angles)
{
  out << "name angle_LaBr3 angle_NaI" << std::endl;
  for (auto const & name : angles.names()) out << name << " " << angles.angleNaI(name) << " " << angles.angleLaBr(name) << std::endl;
  
  return out;
}

void calculate(std::string const & filename)
{
  
}

void ParisBidimAngles::write(std::string const & filename)
{
  File outFile(filename);
  outFile.setExtension(m_extension);
  if (outFile.exists()) {print("Overwritting the file", filename, ", press the button to proceed"); std::cin.get();}
  else (outFile.makePath()); // Create the path if it doesn't already exist

  std::ofstream outfile(outFile.string(), std::ios::out);
  outfile << *this;
  outfile.close();
  print(outFile.string(), "written");
}

void ParisBidimAngles::set(std::string const & name, double const & angleNaI, double const & angleLaBr)
{
  m_names.push_back(name);
  m_angleNaI .emplace(name, angleNaI );
  m_angleLaBr.emplace(name, angleLaBr);
  m_angles   .emplace(name, std::pair<double, double>(angleNaI, angleLaBr));
}

void ParisBidimAngles::load(std::string const & _filename)
{
  File file(_filename);
  file.setExtension(m_extension);
  if (!file.exists()) throw FileError(file.string());

  auto const & filename = file.string();

  std::ifstream inputfile(filename, std::ios::in);

  if (!inputfile.good()) {print("CAN'T OPEN THE PARIS ANGLES FILE " + filename); Colib::throw_error("PARIS ANGLES");}
  else if (file_is_empty(inputfile)) {print("PARIS ANGLES FILE", filename, "EMPTY !"); Colib::throw_error("PARIS ANGLES");}

  std::string line;
  getline(inputfile, line);
  std::string name;
  double angleLaBr = 0; double angleNaI = 0; 
  while (getline(inputfile, line))
  {
    std::istringstream iss (line);
    iss >> name >> angleNaI >> angleLaBr;
    this -> set(name, angleNaI, angleLaBr);
    angleLaBr = angleNaI = 0;
  }
}

#endif //PARISBIDIMANGLES_HPP
