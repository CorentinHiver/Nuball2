#ifndef PARISCALIBRATOR_HPP
#define PARISCALIBRATOR_HPP

#include "ParisPoints.hpp"
#include "../MTObjects/MTRootReader.hpp"

class ParisCalibrator
{
  ParisCalibrator() noexcept = default;
  void RootToBidims(std::string path, std::string outFilename, int nb_files = -1);
};

void ParisCalibrator::RootToBidims(std::string path, std::string outFilename, int nb_files)
{
  std::vector<TH2F*> histos;
  for (int label = 300; label<800; ++label)
  {
    auto const & name = detectors[label];
    if (name == "") continue;
    histos.emplace_back(new TH2F(name.c_str()))
  }

  MTRootReader reader(path, nb_files);
  reader.read([&](Event & event, Nuball2Tree & tree){
    
  });
}


#endif //PARISCALIBRATOR_HPP