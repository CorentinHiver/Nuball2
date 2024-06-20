#ifndef COEFFICIENTCORRECTION_HPP
#define COEFFICIENTCORRECTION_HPP


class CoefficientCorrection
{public:
  CoefficientCorrection(std::string const & filename = "../136/GainDriftCoefficients.dat")
  {
    std::ifstream file(filename);

    print("Reading in the Gain Drifts ");
    int id, r;
    double c0,c1,c2,v1,v2;
    std::string line;
    while (file >> r >> id >> c0 >> c1 >> c2 >> v1 >> v2)
    {
      coeff[r][id][0]=c0;
      coeff[r][id][1]=c1;
      coeff[r][id][2]=c2;
    }
  }
  double correct(float const & nrj, int const run_number, Label const & label) const
  {
    return coeff[run_number][label][0]+(nrj*coeff[run_number][label][1])+(nrj*nrj*coeff[run_number][label][2]);
  }

  double coeff[124][1000][3];
};

#endif //COEFFICIENTCORRECTION_HPP