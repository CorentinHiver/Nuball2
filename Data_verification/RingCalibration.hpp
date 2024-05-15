#ifndef RINGCALIBRATION_HPP
#define RINGCALIBRATION_HPP

class RingCalibration
{
  RingCalibration(std::string const & filename, std::string option = "ascii")
  {
    std::ifstream file(filename, std::ios::in);
    int i = 0; int Edssd = 0; int Ex = 0;
    while(file >> i >> Edssd >> Ex) m_data[i][]
  }
  constexpr int nb_rings = 16;
  constexpr int nb_subdivisions = 16;
  double m_data[int][485];
};

#endif //RINGCALIBRATION_HPP