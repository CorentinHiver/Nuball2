#ifndef EXCITATIONENERGY_HPP
#define EXCITATIONENERGY_HPP

#include "../libRoot.hpp"
class ExcitationEnergy
{public:
  ExcitationEnergy() noexcept = default;
  ExcitationEnergy(std::string path_to_folder, std::string target, std::string projectile, std::string ejectile, std::string additionnal_information)
  {
    push_back_if_none(path_to_folder, '/');
    load(path_to_folder+target+"_"+projectile+"_"+ejectile+"_"+additionnal_information+"_Ex.root");
  }
  ExcitationEnergy(std::string _filename)
  {
    load(_filename);
  }

  void load(std::string _filename)
  {
    filename = _filename;
    clear();
    if (!file_exists(filename)) {error(filename, "does not exist !!"); return;}
    
    m_data_vector_Ex.resize(nb_rings);
    m_data_vector_EDssd.resize(nb_rings);

    if (extension(filename) == "Ex")
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
        m_min_E_bin_pt[id] = m_data_vector_EDssd[id].front();
        m_max_E_bin_pt[id] = m_data_vector_EDssd[id].back();
      }

      m_ok = true;
    }
    else if (extension(filename) == "root")
    {
      // Output splines_ in root file
    #ifdef LIBROOT_HPP

      m_type = 1;
      // unique_TFile rootfile (TFile::Open("../136/U5_d_d_10umAl_Ex.root", "READ"));
      unique_TFile rootfile (TFile::Open(filename.c_str(), "READ"));
      auto splines = Colib::file_get_map_of<TSpline3>(rootfile.get());
      splines_pt.resize(nb_rings, nullptr);
      splines_stop.resize(nb_rings, nullptr);
      for (auto & e : splines)
      {
        auto & name = e.first;
        auto spline = e.second;
        auto const & spline_id = std::atoi(split(name, '_').back().c_str());
        if (found(name, "pt"))
        {
          splines_pt[spline_id] = spline;
          m_min_E_bin_pt[spline_id] = spline->GetXmin();
          m_max_E_bin_pt[spline_id] = spline->GetXmax();
        }
        else if (found(name, "stop"))
        {
          splines_stop[spline_id] = spline;
          m_min_E_bin_stop[spline_id] = spline->GetXmin();
          m_max_E_bin_stop[spline_id] = spline->GetXmax();
        }
      }

      for (size_t spline_i = 0; spline_i<splines_pt.size(); ++spline_i) 
      {
        if (!splines_pt[spline_i]) m_max_E_bin_pt[spline_i] = 0;
        if (!splines_stop[spline_i]) m_max_E_bin_stop[spline_i] = 0;
      } 

      m_ok = true;

      rootfile->Close();

    #else //!LIBROOT_HPP
      error("libRoot.hpp not included but reading a root file :",filename);
    #endif //LIBROOT_HPP
    }
    else error("in ExcitationEnergy::ExcitationEnergy(std::string filename = ",filename,") : extension not handled !");
  }
  
  auto calculate (double const & nrj, int const & ring_i, bool const & punch_through = true)
  {
    if (punch_through)
    {
      if (m_type == 0)
      {
        auto const & bin_nrj = int_cast(nrj*0.1);
        if (m_data[ring_i][bin_nrj] != bad_value) return m_data[ring_i][bin_nrj];
        else return bad_value;
      }
      else if (m_type == 1)
      {
        // print(ring_i, nrj, m_min_E_bin_pt[ring_i], m_max_E_bin_pt[ring_i]);
        if (m_min_E_bin_pt[ring_i] < nrj && nrj < m_max_E_bin_pt[ring_i]) return int_cast(splines_pt[ring_i]->Eval(nrj));
        else return bad_value;
      }
    }
    else
    {
      if (m_type == 0) return bad_value;
      else if (m_type == 1)
      {
         if (m_min_E_bin_stop[ring_i] < nrj && nrj < m_max_E_bin_stop[ring_i]) return int_cast(splines_stop[ring_i]->Eval(nrj));
        else return bad_value;
      }
    }
    return bad_value;
  }

  auto operator() (double const & nrj, int const & ring_i, bool const & punch_through = true) {return calculate(nrj, ring_i, punch_through);}
  
  void clear()
  {
    m_ok = false;
    for (auto & d : m_data) for (auto & e : d) e = bad_value;
    for (auto & e : m_min_E_bin_pt) e = bad_value;
    for (auto & e : m_max_E_bin_pt) e = bad_value;
    for (auto & e : m_min_E_bin_stop) e = bad_value;
    for (auto & e : m_max_E_bin_stop) e = bad_value;
    splines_pt.clear();
    splines_stop.clear();
    m_data_vector_Ex.clear();
    m_data_vector_EDssd.clear();
  }

  operator bool() const & {return m_ok;}

  constexpr static int bad_value = -42;
  constexpr static int nb_rings = 16;
  constexpr static int nb_subdivisions = 1000;
  // constexpr static double max_E_bin = 7000;
  std::array<std::array<int, nb_subdivisions>, nb_rings> m_data;
  std::vector<std::vector<int>> m_data_vector_Ex;
  std::vector<std::vector<int>> m_data_vector_EDssd;
  std::array<int, nb_rings> m_min_E_bin_pt;
  std::array<int, nb_rings> m_max_E_bin_pt;
  std::array<int, nb_rings> m_min_E_bin_stop;
  std::array<int, nb_rings> m_max_E_bin_stop;

#ifdef LIBROOT_HPP
  void makeSplines()
  {
    for (int id = 0; id<nb_rings; ++id)
    {
      std::vector<double> x_data; x_data.reserve(nb_subdivisions);
      std::vector<double> y_data; y_data.reserve(nb_subdivisions); 
      for (int E = m_min_E_bin_pt[id]; E<m_max_E_bin_pt[id]; ++E)
      {
        x_data.push_back(E);
        y_data.push_back(m_data[id][E]);
      }
      splines_pt.emplace_back(new TSpline3(("spline_ring_"+std::to_string(id)).c_str(), x_data.data(), y_data.data(), y_data.size()));
    }
  }
  std::vector<TSpline3*> splines_pt;
  std::vector<TSpline3*> splines_stop;
#endif //LIBROOT_HPP

  std::string filename;

private:
  int m_type = 0;
  bool m_ok = false;
};

std::ostream& operator<<(std::ostream& out, ExcitationEnergy const & Ex)
{
  out << Ex.filename << std::endl;
  for (int ring_i = 0; ring_i<Ex.nb_rings; ++ring_i) for (int E_i = 0; E_i<Ex.nb_subdivisions; ++E_i) out << ring_i << " " << E_i*10 << " " << Ex.m_data[ring_i][E_i] << std::endl;
  return out;
}

#endif //EXCITATIONENERGY_HPP