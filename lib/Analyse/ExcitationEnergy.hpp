#ifndef EXCITATIONENERGY_HPP
#define EXCITATIONENERGY_HPP

class ExcitationEnergy
{public:
  ExcitationEnergy() noexcept = default;
  ExcitationEnergy(std::string const & _filename) : filename(_filename)
  {
    clear();
    if (extension(_filename) == "Ex")
    {
      // Output in Jon format
      m_type = 0;
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
    else if (extension(_filename) == "root")
    {
      // Output splines in root file
    #ifdef LIBROOT_HPP
      m_type = 1;
      auto rootfile = TFile::Open(_filename.c_str(), "READ");
      auto spline_names = map_of<TSpline3>(rootfile);
      splines.resize(16);
      for (auto & e : spline_names)
      {
        auto & name = e.first;
        auto spline = e.second;
        auto const & spline_id = atoi(split(name, '_').back().c_str());
        splines[spline_id] = spline;
        m_min_E_bin[spline_id] = spline->GetXmin();
        m_max_E_bin[spline_id] = spline->GetXmax();
      }
    #else //!LIBROOT_HPP
      error("libRoot.hpp not included but reading a root file :",_filename);
    #endif //LIBROOT_HPP
    }
    else error("in ExcitationEnergy::ExcitationEnergy(std::string filename = ",_filename,") : extension not handled !");
  }
  auto operator() (double const & nrj, int const & ring_i)
  {
    if (m_type == 0)
    {
      auto const & bin_nrj = int_cast(nrj*0.1);
      if (m_data[ring_i][bin_nrj] != bad_value) return m_data[ring_i][bin_nrj];
      else return bad_value;
    }
    else if (m_type == 1)
    {
      if (m_min_E_bin[ring_i] < nrj && nrj < m_max_E_bin[ring_i]) return int_cast(splines[ring_i]->Eval(nrj));
      else return bad_value;
    }
    return bad_value;
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

private:
  int m_type = 0;
};

std::ostream& operator<<(std::ostream& out, ExcitationEnergy const & Ex)
{
  out << Ex.filename << std::endl;
  for (int ring_i = 0; ring_i<Ex.nb_rings; ++ring_i) for (int E_i = 0; E_i<Ex.nb_subdivisions; ++E_i) out << ring_i << " " << E_i*10 << " " << Ex.m_data[ring_i][E_i] << std::endl;
  return out;
}

#endif //EXCITATIONENERGY_HPP