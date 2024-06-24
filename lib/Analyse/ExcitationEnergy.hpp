#ifndef EXCITATIONENERGY_HPP
#define EXCITATIONENERGY_HPP

class ExcitationEnergy
{public:
  ExcitationEnergy() noexcept = default;
  ExcitationEnergy(std::string const & _filename, std::string option = "ascii") : filename(_filename)
  {
    clear();
    if (option == "ascii")
    {
      std::ifstream file(filename, std::ios::in);
      int id = 0; int EDssd = 0; int Ex = 0;
      while(file >> id >> EDssd >> Ex) 
      {
        Ex*=10;
        if (nb_rings < id) throw_error(concatenate("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : ring_i ", id, " > ", nb_rings));
        if (nb_subdivisions < EDssd) throw_error(concatenate("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : ring_i ", id, " > ", nb_rings));
        m_data[id][EDssd] = Ex;
        m_data_vector_Ex[id].push_back(Ex);
        m_data_vector_EDssd[id].push_back(EDssd);
      }

      // Find the maximum and the minimum value for each ring
      for (id = 0; id<nb_rings; ++id) 
      {
        m_min_E_bin[id] = m_data_vector_EDssd[id].front();
        m_max_E_bin[id] = m_data_vector_EDssd[id].back();
      }
    }
    else error("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : option", option, "unkown !");
  }
  auto const & operator() (double const & nrj, int const & ring_i)
  {
    auto const & bin_nrj = int_cast(nrj*0.1);
    // print(m_min_E_bin[ring_i], bin_nrj, m_max_E_bin[ring_i]);
    // if (m_min_E_bin[ring_i] < bin_nrj && bin_nrj < m_max_E_bin[ring_i]) return m_data[ring_i][bin_nrj];
    if (m_data[ring_i][bin_nrj] != bad_value) return m_data[ring_i][bin_nrj];
    else return bad_value;
  }
  
  void clear()
  {
    for (auto & d : m_data) for (auto & e : d) e = bad_value;
    for (auto & e : m_min_E_bin) e = bad_value;
    for (auto & e : m_max_E_bin) e = bad_value;
    m_data_vector_Ex.clear();
    m_data_vector_EDssd.clear();
    m_data_vector_Ex.resize(nb_rings);
    m_data_vector_EDssd.resize(nb_rings);
  }
  constexpr static int bad_value = -42;
  constexpr static int nb_rings = 16;
  constexpr static int nb_subdivisions = 1000;
  // constexpr static double max_E_bin = 7000;
  std::array<std::array<int, nb_subdivisions>, nb_rings> m_data;
  std::vector<std::vector<int>> m_data_vector_Ex;
  std::vector<std::vector<int>> m_data_vector_EDssd;
  std::array<int, nb_rings> m_min_E_bin;
  std::array<int, nb_rings> m_max_E_bin;

#ifdef LIBROOT_HPP
  void makeSplines()
  {
    for (int id = 0; id<nb_rings; ++id)
    {
      std::vector<double> x_data; x_data.reserve(nb_subdivisions);
      std::vector<double> y_data; y_data.reserve(nb_subdivisions); 
      for (int E = m_min_E_bin[id]; E<m_max_E_bin[id]; ++E)
      {
        x_data.push_back(E);
        y_data.push_back(m_data[id][E]);
      }
      splines.emplace_back(new TSpline3(("spline_ring_"+std::to_string(id)).c_str(), x_data.data(), y_data.data(), y_data.size()));
    }
  }
  std::vector<TSpline3*> splines;
#endif //LIBROOT_HPP

  std::string filename;
};

std::ostream& operator<<(std::ostream& out, ExcitationEnergy const & Ex)
{
  out << Ex.filename << std::endl;
  for (int ring_i = 0; ring_i<Ex.nb_rings; ++ring_i) for (int E_i = 0; E_i<Ex.nb_subdivisions; ++E_i) out << ring_i << " " << E_i*10 << " " << Ex.m_data[ring_i][E_i] << std::endl;
  return out;
}

#endif //EXCITATIONENERGY_HPP