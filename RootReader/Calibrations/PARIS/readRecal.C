#include "../../../lib/libRoot.hpp"
#include "../../../lib/Classes/Detectors.hpp"


void readRecal(std::string const & filename = "CalibParis.recal")
{
  ifstream infile(filename, std::ios::in);
  std::string outFilename = removeExtension(filename)+".cal";
  ifstream outfile(outFilename, std::ios::out);

  detectors.load("../index_129.list");
  for (auto const & e : detectors.labels()) if (found(e.first, "PARIS")) std::cout << e << " ";
  std::cout << std::endl;

  std::string line;
  std::string name;
  Label label = 0;
  std::unordered_map<Label, std::vector<double>> coeffs;
  while (getline(infile, line))
  {
    std::istringstream iss(line);
    iss >> name;
    label = detectors.label("PARIS_"+name);
    print(label, line);
    while (iss>>coeffs[label]) if (iss.eof()) break;
  }
  infile.close();

  for (auto const & e : coeffs)
  {
    std::cout << e.first << " : ";
    for (auto const & c : e.second) std::cout << c << " ";
    std::cout << std::endl;
  }

}