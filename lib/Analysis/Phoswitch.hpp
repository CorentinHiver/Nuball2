#pragma once

#include "../Nuball2.hh"

class Phoswitch
{
public:
  Phoswitch() : m_index(g_index++) {}

  operator bool() const noexcept {return (0 < crystal);}

  void inline clear()
  {
    time    = {};
    nrj     = {};
    crystal = {};
  }

  Index inline fill(NRJ nrj1, NRJ nrj2, Time _time);

  template<class PID>
  Index inline fillWithPid(NRJ nrj1, NRJ nrj2, Time _time, PID const & pid);

  static Index inline simplePid(float const & nrj1, float const & nrj2);
  static Index inline testGateClassic(float const & nrj1, float const & nrj2);
  static Index inline testGateTan(float const & nrj1, float const & nrj2);

  // constexpr inline auto isRejected() const {return crystal == Rejected;}
  // constexpr inline auto isLaBr3   () const {return crystal == Fast;}
  // constexpr inline auto isNaI     () const {return crystal == Slow;}
  // constexpr inline auto isMixed   () const {return crystal == Mix;}

  NRJ   nrj      = {};
  Time  time     = {};
  Index crystal  = {}; // 0 : rejected, 1 : Fast, 2 : Slow, 3 : mixed event

  enum Cristals {Rejected, Fast, Slow, Mix, NbCrystals};
  static constexpr inline std::array<std::string_view, NbCrystals> cristals = {"Rejected", "Fast", "Slow", "Mix"};
  static inline constexpr auto const & crystalName(Index _cristal) {return cristals[_cristal];}
  inline constexpr auto const & getCrystalName() const {return crystalName(crystal);}
  
  class RotationCalib
  {
  public:
    RotationCalib() noexcept = default;
    RotationCalib(std::string const & filename) {read(filename);}

    void read(std::string const & filename = "NaI_136_2024.angles")
    {
      std::ifstream file(filename, std::ios::in);
      if (!file.good()) Colib::throw_error(Colib::concatenate("in Phoswitch::RotationCalib::RotationCalib(std::string filename) : file", filename, " can't be open"));
      Label label = {}; 
      double angle, coeff = {};
      while(file >> label >> angle >> coeff) data.emplace(label, Calibration({angle, coeff}));
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

    inline double calibrate      (Label const & label, double const & qshort, double const & qlong) const {return data.at(label).calibrate      (qshort, qlong);}
    inline double calibrate_short(Label const & label, double const & qshort, double const & qlong) const {return data.at(label).calibrate_short(qshort, qlong);}

    auto getCoeffs() const
    {
      std::unordered_map<Label, double> ret;
      for (auto const & calib : data) ret.emplace(calib.first, calib.second.coeff);
      return ret;
    }

    bool hasData() const {return 0 < data.size();}

    class Calibration
    {
    public:
      Calibration(std::initializer_list<double> const & inputs)
      {
        if (inputs.size() == 2) 
        {
          auto it = inputs.begin();
          angle = *it;
          coeff = *(++it);
        }
      }

      Calibration(std::pair<double, double> const & input)
      {
        angle = input.first;
        coeff = input.second;
      }

      Calibration & operator=(std::initializer_list<double> const & inputs)
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

      double calibrate      (double const & qshort, double const & qlong) const {return (qshort * sin_a + qlong * cos_a) * coeff;}
      double calibrate_short(double const & qshort, double const & qlong) const {return (qshort * cos_a - qlong * sin_a) * coeff;}

      double angle = 0;
      double cos_a = 0;
      double sin_a = 0;
      double coeff = 0;
    };

    std::unordered_map<Label, Calibration> data;

  private:
    void prepare() {for (auto & it : data) it.second.prepare();}
  };

  auto const & index() const noexcept {return m_index;}
  static void resetIndexes() noexcept {g_index = 0;}

  friend std::ostream& operator<<(std::ostream& out, Phoswitch const & phoswitch)
  {
    out << "crystal : ";
    out <<  (0 == phoswitch.crystal) ? ("rejected") : phoswitch.getCrystalName(); 
    out << " time : " << phoswitch.time << " ps nrj : " << phoswitch.nrj << " keV ";
    return out;
  }


private:
  Label static inline thread_local g_index = 0;
  Label mutable m_index;

  static RotationCalib m_calib;
};

Index inline Phoswitch::fill(NRJ nrj1, NRJ nrj2, Time _time)
{
  if (nrj1 < 10_keV || nrj2 < 10_keV) return 0;
  
  crystal = simplePid(nrj1, nrj2);
  time = _time;

  switch (crystal)
  {
    case Fast : nrj = (m_calib.hasData()) ? m_calib.calibrate_short(m_index, nrj1, nrj2) : nrj1         ; break;
    case Slow : nrj = (m_calib.hasData()) ? m_calib.calibrate      (m_index, nrj1, nrj2) : nrj2         ; break;
    case Mix  : nrj = (m_calib.hasData()) ? m_calib.calibrate      (m_index, nrj1, nrj2) : (nrj1+nrj2)/2; break;
  }

  return crystal;
}

template<class PID>
Index inline Phoswitch::fillWithPid(NRJ nrj1, NRJ nrj2, Time _time, PID const & pid)
{
  if (nrj1 < 10_keV || nrj2 < 10_keV) return 0;
  
  crystal = pid(nrj1, nrj2);
  time = _time;

  switch (crystal)
  {
    case Fast : nrj = (m_calib.hasData()) ? m_calib.calibrate_short(m_index, nrj1, nrj2) : nrj1         ; break;
    case Slow : nrj = (m_calib.hasData()) ? m_calib.calibrate      (m_index, nrj1, nrj2) : nrj2         ; break;
    case Mix  : nrj = (m_calib.hasData()) ? m_calib.calibrate      (m_index, nrj1, nrj2) : (nrj1+nrj2)/2; break;
  }

  return crystal;
}

Index inline Phoswitch::simplePid(float const & nrj1, float const & nrj2)
{
  if (nrj2 == NRJ{0}) return 0;
  auto const & ratio = nrj1/nrj2;
       if (ratio < 0.3) return 0;
  else if (ratio < 0.5) return 2;
  else if (ratio < 0.8) return 3;
  else if (ratio < 1.2) return 1;
  else                  return 0;
}

Index inline Phoswitch::testGateClassic(float const & nrj1, float const & nrj2)
{
  if (nrj2 == NRJ{0}) return 0;
  auto const & ratio = (nrj2-nrj1)/nrj2;
       if (ratio < -0.2) return 0;
  else if (ratio <  0.2) return 1;
  else if (ratio <  0.5) return 3;
  else if (ratio <  0.7) return 2;
  else                   return 0;
}

/// @brief 
/// @todo 
Index inline Phoswitch::testGateTan(float const & nrj1, float const & nrj2)
{
  if (nrj2 == NRJ{0}) return 0;
  auto const & ratio = tan((nrj2-nrj1)/nrj2);
       if (ratio < -0.2) return 0;
  else if (ratio <  0.2) return 1;
  else if (ratio <  0.5) return 3;
  else if (ratio <  0.7) return 2;
  else                   return 0;
}