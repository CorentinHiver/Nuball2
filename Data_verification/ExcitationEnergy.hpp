#ifndef EXCITATIONENERGY_HPP
#define EXCITATIONENERGY_HPP

class ExcitationEnergy
{public:
  ExcitationEnergy(std::string const & filename, std::string option = "ascii")
  {
    if (option == "ascii")
    {
      std::ifstream file(filename, std::ios::in);
      int i = 0; int EDssd = 0; int Ex = 0; int i_prev = 0;
      while(file >> i >> EDssd >> Ex) 
      {
        Ex*=10;
        if (i>i_prev)
        {
          m_min_E_bin[i] = Ex;
          i_prev = i;
        }
        if (nb_rings < i) throw_error(concatenate("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : ring_i ", i, " > ", nb_rings));
        if (nb_subdivisions < EDssd) throw_error(concatenate("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : ring_i ", i, " > ", nb_rings));
        m_data[i][EDssd] = Ex;
      }
    }
    else error("in ExcitationEnergy::ExcitationEnergy(std::string filename, std::string option) : option", option, "unkown !");
  }
  constexpr double const & operator() (double const & nrj, int const & ring_i)
  {
    auto const & bin_nrj = int_cast(nrj*0.1);
    if (bin_nrj < m_min_E_bin[ring_i] || bin_nrj > max_E_bin) return bad_value;
    else return m_data[ring_i][bin_nrj];
  }
  constexpr static double bad_value = -1;
  constexpr static int nb_rings = 16;
  constexpr static int nb_subdivisions = 10000;
  constexpr static double max_E_bin = 7000;
  double m_data [nb_rings] [nb_subdivisions+1] ;
  double m_min_E_bin [nb_rings];
};

#endif //EXCITATIONENERGY_HPP