#ifndef TIMESHIFTS_HPP
#define TIMESHIFTS_HPP

#include "../libCo.hpp"

class Timeshifts
{
public:

  using Timeshift_t = int64_t;
  using Timeshifts_t = std::vector<Timeshift_t>;

  /// @brief Default constructor
  Timeshifts() = default;

  /// @brief Constructor to instanciate a Timeshift object with already loaded data located in the given .dT file
  Timeshifts(std::string const & filename) : m_filename(filename) 
  {
    this -> load(m_filename);
  }
  
  /// @brief Default copy constructor
  Timeshifts(Timeshifts const & other) noexcept = default;
  
  /// @brief Default copy method
  Timeshifts& operator=(Timeshifts const & other) noexcept = default;

  /// @brief Use this method to load timeshifts from a .dT file
  bool load(std::string const & filename);

  /// @brief Saves the timeshift file to disk
  void write(std::string const & fullpath, std::string const & name);

  /// @brief Returns the timeshift of the detector with the given label
  auto const & operator[] (int const & label) const {return m_timeshifts[label];}

  /// @brief Returns the timeshift of the detector with the given label
  auto const & get(int const & i) const {return m_timeshifts[i];}

  #ifdef HIT_HPP // If Hit.hpp is included (Nuball2)
    /// @brief Returns the timeshift of the detector with the label of the given hit
    void operator() (Hit & hit) const {hit.stamp += m_timeshifts[hit.label];}
  #endif //HIT_HPP

  /// @brief Multiplies each element of the timeshift vector by a given factor f
  Timeshifts & operator*= (double const & f) {for (auto & dT : m_timeshifts) dT*=f; return *this;}

  /// @brief Returns a reference to the timeshift vector (std::vector<int64_t>)
  Timeshifts_t & get() {return m_timeshifts;}

  /// @brief Returns a read-only access to the timeshift vector (std::vector<int64_t>)
  Timeshifts_t const & data() const {return m_timeshifts;}
  
  /// @brief Returns weither the timestamp vector has been filled correctly
  operator bool() const & {return m_ok;}

  /// @brief Returns weither the timestamp vector has been filled correctly
  auto const & isOk() const {return m_ok;}

  /// @brief Returns weither the timestamp vector is empty
  auto empty() const {return m_timeshifts.empty();}

  /// @brief Returns the size of the timestamp vector
  auto size() const {return m_timeshifts.size();}

private:
  std::string m_filename;
  Timeshifts_t m_timeshifts;
  bool m_ok = false;
  int m_nb_detectors = 0;
  Colib::Path m_outPath;
  
  public:
  class NotFoundError
  {
  public:
    NotFoundError(std::string const & filename) : m_filename(filename) {error("Timeshiftor::NotFoundError", filename);}
    auto const & getFilename() const {return m_filename;}
  private:
    std::string m_filename;
  };

};

bool Timeshifts::load(std::string const & filename)
{
  std::ifstream inputFile(filename, std::ifstream::in);
  if (!inputFile.good()) {throw NotFoundError(filename);}
  else if (Colib::fileIsEmpty(inputFile)) {print("TIMESHIFT FILE", filename, "EMPTY !");return false;}
  std::string line = ""; // Reading buffer
  size_t label = 0; // Reading buffer
  while (getline(inputFile, line))
  { 
    std::istringstream iss(line);
    iss >> label;
    if (m_timeshifts.size() <= label) m_timeshifts.resize(label+1);
    iss >> m_timeshifts[label];
    ++m_nb_detectors;
  }
  inputFile.close();
  print("Timeshifts extracted from", filename);
  return (m_ok = !m_timeshifts.empty());
}

void Timeshifts::write(std::string const & fullpath, std::string const & name)
{
  m_outPath = Colib::Path (fullpath, true);
  if (!m_outPath) {m_ok = false; return;}

  Colib::File outData (m_outPath+name);
  outData.setExtension(".dT");

  std::ofstream outTimeshiftsFile(outData, std::ios::out);
  
  for (size_t label = 0; label<m_timeshifts.size(); label++)
  {
    auto const & dT = m_timeshifts[label];
    if (dT != 0) outTimeshiftsFile << label << "\t" << dT << std::endl;
  }

  outTimeshiftsFile.close();

  print("Timeshifts data written to", outData);
}

#endif //TIMESHIFTS_HPP
