#ifndef HISTOGRAMOR_HPP
#define HISTOGRAMOR_HPP

#include "../libRoot.hpp"
#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"

class Histogramor
{
public:
  Histogramor(int argc, char ** argv);
  static printParameters();
  bool readParam(int argc, char ** argv);

  bool load_binning(std::string const & filename);

private:
  Vector_MTTHist<TH1F> m_each_spectra;
  MTTHist<TH2F> m_all_spectra;

};

Histogramor(int argc, char ** argv)
{

}

Histogramor::printParameters();
{
  print("");
  print("-i [ID_file]      : Load ID file");
  print("-b [binning_file] : Load .binning file");
  print("");
}

bool Histogramor::readParam(int argc, char ** argv)
{

}

bool load_binning(std::string const & filename)
{
  std::ifstream file(filename, std::ios::in);

  std::string line;
  std::vector<std::string> name;
  std::vector<int> n_bins;
  std::vector<int> min_bins;
  std::vector<int> max_bins;
  while(getline(file, line))
  {
    std::istringstream iss(line);
    int n = 0; int mi = 0; int ma = 0;
    iss >> name >> n >> mi >> ma;
    n_bins.push_back(n);
    min_bins.push_back(mi);
    max_bins.push_back(ma);
  }
  
}

#endif // HISTOGRAMOR_HPP