#ifndef COEFFICIENTCORRECTION_HPP
#define COEFFICIENTCORRECTION_HPP


class CoefficientCorrection
{public:
  CoefficientCorrection(std::string const & filename)
  {
    for (size_t i = 0; i<nb_runs; ++i) for (size_t j = 0; j<nb_det; ++j) for (size_t k = 0; k<nb_coeff; ++k) m_coeff[i][j][k] = -1.e-42f;

    std::ifstream file(filename);
    if (!file) {error("CoefficientCorrection : ", filename, "not found"); return;}
    print("Reading in the Gain Drifts ");
    size_t id, r;
    float c0,c1,c2,v1,v2;
    std::string line;
    while (file >> r >> id >> c0 >> c1 >> c2 >> v1 >> v2)
    {
      if (r>nb_runs-1) throw_error(concatenate ("run number", r, "out of bounds", 0, nb_runs-1));
      if (id>nb_det-1) throw_error(concatenate ("detector number", r, "out of bounds", 0, nb_det-1));
      m_coeff[r][id][0]=c0;
      m_coeff[r][id][1]=c1;
      m_coeff[r][id][2]=c2;
    }
    m_ok = true;
  }

  float correct(float const & nrj, int const run_number, Label const & label) const
  {
    return m_coeff[run_number][label][0]+(nrj*m_coeff[run_number][label][1])+(nrj*nrj*m_coeff[run_number][label][2]);
  }

  std::array<double, 3> coeff(int const run_number, Label const & label)
  {
    std::array<double, 3> ret;
    return (ret = {m_coeff[run_number][label][0], m_coeff[run_number][label][1], m_coeff[run_number][label][2]});
  }

  static constexpr size_t nb_runs = 124;
  static constexpr size_t nb_det = 1000;
  static constexpr size_t nb_coeff = 3;
  std::array<std::array<std::array<float, nb_coeff>, nb_det>, nb_runs> m_coeff;

  operator bool() const & {return m_ok;}

private:
  bool m_ok = false;
};

#endif //COEFFICIENTCORRECTION_HPP