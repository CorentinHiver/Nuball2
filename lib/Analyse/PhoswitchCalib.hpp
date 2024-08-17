#ifndef PHOSWITCHCALIB_HPP
#define PHOSWITCHCALIB_HPP

class PhoswitchCalib
{
public:
  PhoswitchCalib() noexcept = default;
  PhoswitchCalib(std::string const & filename) {read(filename);}

  void read(std::string const & filename = "NaI_136_2024.angles")
  {
    std::ifstream file(filename, std::ios::in);
    if (!file.good()) throw_error(concatenate("in PhoswitchCalib::PhoswitchCalib(std::string filename) : file", filename, " can't be open"));
    Label label = 0; double angle = 0; double coeff = 0;
    while(file >> label >> angle >> coeff) data.emplace(label, Coefficients({angle, coeff}));
    file.close();
    prepare();
  }

  void write(std::string const & filename = "NaI_136_2024.angles")
  {
    std::ofstream file(filename, std::ios::out);
    for (auto const & it : data) file << it.first << " " << it.second.angle << " " << it.second.coeff << std::endl;
    file.close();
    print(filename, "written");
  }

  void prepare() {for (auto & it : data) it.second.prepare();}

  double calibrate(Hit & hit) {return data.at(hit.label).calibrate(hit);}
  double calibrate(Label const & label, double const & qshort, double const & qlong) {return data.at(label).calibrate(qshort, qlong);}
  inline double calibrate(Label const & label, double const & qshort, double const & qlong) const {return data.at(label).calibrate(qshort, qlong);}

  class Coefficients
  {public:
    Coefficients(std::initializer_list<double> const & inputs)
    {
      if (inputs.size() == 2) 
      {
        auto it = inputs.begin();
        angle = *it;
        coeff = *(++it);
      }
    }

    Coefficients(std::pair<double, double> const & input)
    {
      angle = input.first;
      coeff = input.second;
    }

    Coefficients & operator=(std::initializer_list<double> const & inputs)
    {
      if (inputs.size() == 2) 
      {
        auto it = inputs.begin();
        angle = *it;
        coeff = *(++it);
      }
      return *this;
    }

    void prepare()
    {
      cos_a = cos(angle);
      sin_a = sin(angle);
    }

    double calibrate(Hit const & hit) {return (hit.nrj * sin_a + hit.nrj2 * cos_a) * coeff;}
    double calibrate(double const & qshort, double const & qlong) {return (qshort * sin_a + qlong * cos_a) * coeff;}
    double calibrate(double const & qshort, double const & qlong) const {return (qshort * sin_a + qlong * cos_a) * coeff;}

    double angle = 0;
    double cos_a = 0;
    double sin_a = 0;
    double coeff = 0;
  };

  std::map<Label, Coefficients> data;
};


#endif //PHOSWITCHCALIB_HPP