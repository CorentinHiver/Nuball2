#ifndef DATAFILE_HPP
#define DATAFILE_HPP

#include "../libCo.hpp"

using DataLine = std::vector<std::string>;

/**
 * @brief NOT FUNCTIONNAL !!! This is a homemade .data reader
 * 
 * @details This is a quite strict format : 
 * 
 * index value1 value2 value3 // The header is one line only, holding the name of each column
 * 10001 235321 2365 235321   // Each column is separated by the same separator
 * ...
 * end
 * 
 * Let's look at an example first : 
 * 
 * Imagine we want to store the temperature in two different rooms.
 * The index line will hold the time.
 * The first column holds the values of the first room and same for the second column.
 * Here is how it would look like : 
 * 
 * minutes room1 room2
 * 0 22.5 21.5
 * 1 22.3 21.3
 * 2 22.4 21.4
 * 3 22.2 21.2
 * 4 22.3 21.3
 * 5 22.4 21.4
 * 
 * Possible optimisation : 
 * 
 * You can #define UnorderedDataFile to have an unordered_map holding the data
 */
template<class Index, class T>
class DataFile
{
  using Indexes = std::vector<Index>;
  using DataPoints = std::vector<T>;
public:
  DataFile() = default;
  DataFile(std::string const & filename, bool const & has_header = true) {load (filename, has_header);}

  /// @brief A vector containing the list of the column index to read. By default, reads only the first lines.
  void setColumns(std::vector<int> columns) {m_columns_to_read = columns; m_nb_columns = int_cast(columns.size());}

  void load(std::string const & filename, bool const & has_header = true);

  void write(std::string const & filename);

  void checkOutputfile(std::ofstream & outputfile);
  void writeHeader(std::ofstream & outputfile);
  void writeData(std::ofstream & outputfile);

  ///@todo if needed
  ///@return true if the vectorisation is successfull
  bool vectorize(){return true;}
  void setHeader(DataLine const & header) {m_header = header;}
  auto const & indexes() const {return m_indexes;}

  auto const & data() const {return m_data;}
  auto & data() {return m_data;}

  T const & operator[](T t) const {return m_data[t][m_chosen_column];} 
  T & operator[](T t) {return m_data[t][m_chosen_column];} 

  auto const & size() const {return m_size;}
  auto & size() {return m_size;}

  /// @brief Setup the reading to the wanted column 
  void setColumn(int const & choice) {m_chosen_column = choice;}

  /// @brief Setup the reading to the wanted column 
  void setColumn(std::string const & column) 
  {
    m_chosen_column = first_index_in(m_labels_column, column);
    if (m_chosen_column == m_labels_column.size()) Colib::throw_error(concatenate(
      "column name ", column, " not found in the data loaded from ", m_input_filename));
  }

  template<class T2>
  void readColumn(std::vector<T2> & vector)
  {
    vector.reserve(m_data.size());
    for (int row_i = 0; row_i<m_data.size(); row_i++) vector[row_i] = m_data[m_labels_rows[row_i]][m_chosen_column];
  }

  ///@tparam COL: Either std::string (name of the column), or int (the column index)
  template<class T2, class COL>
  void readColumn(std::vector<T2> & vector, COL && column)
  {
    setColumn(std::forward<COL>(column));
    vector.reserve(m_data.size());
    for (int row_i = 0; row_i<m_data.size(); row_i++) vector[row_i] = m_data[m_labels_rows[row_i]][m_chosen_column];
  }

  template<class T2>
  void readColumn(std::map<Index, T2> & map)
  {
    for (int row_i = 0; row_i<m_data.size(); row_i++) map[row_i] = m_data[m_labels_rows[row_i]][m_chosen_column];
  }

  ///@tparam COL: Either std::string (name of the column), or int (the column index)
  template<class T2, class COL>
  void readColumn(std::map<Index, T2> & map, COL && column)
  {
    setColumn(std::forward<COL>(column));
    for (int row_i = 0; row_i<m_data.size(); row_i++) map[row_i] = m_data[m_labels_rows[row_i]][m_chosen_column];
  }

  /// @brief Returns the given column
  /// @attention This involves a copy of the data !!
  DataPoints column()
  {
    DataPoints ret;
    readColumn(ret);
    return ret;
  }

  /**
   * @brief Returns the given column
   * @tparam COL: Either std::string (name of the column), or int (the column index)
   * @attention This involves a copy of the data !!
   * 
   */
  template<class COL>
  DataPoints column(COL && columns)
  {
    setColumn(std::forward<COL>(column));
    DataPoints ret;
    readColumn(ret);
    return ret;
  }

  operator bool() const {return m_data.size()>0 && m_ok;}

protected:
  void checkInputfile(std::ifstream & inputfile);
  void loadHeader(std::ifstream & inputfile);
  void setupContainers();
  void loadData(std::ifstream & inputfile);

  std::string m_input_filename;
  std::string m_output_filename;

  std::vector<int> m_columns_to_read = {1}; // The index of the wanted rows
  int m_nb_columns = 1;                     // Size of the above vector
  int m_size = 1;                           // Number of loaded row
  int m_label_index = 0;                    // The index of the row label (usually = 0)
  char m_delim = ' ';                       // The delimiter of the csv file
  bool m_has_header = true;                 // Weither there is a header or not in the data
  
  DataLine m_header; // The header still in strings

  //Manages the columns :
  Colib::Strings m_labels_column; // List of the index of the columns
  DataPoints m_data_points; // the label of wanted columns casted in the data type
  Bools m_column_name_casted; // To check which column label could not be casted 
  bool m_all_column_name_casted = false; // To check if all the column label could be caster

  // Manages the rows :
  DataLine m_labels_rows; // List of the index of the lines
  Indexes m_indexes; // List of indexes

  bool m_ok = true;                           // Is everything ok ?
  bool m_warning_is_error = true;             // Do not continue if there is any issue during loading ?
  
#ifdef UnorderedDataFile
  std::unordered_map<Index, DataPoints> m_data;
#else 
  std::map<Index, DataPoints> m_data;
#endif //UnorderedDataFile

  // Vectorized data : TODO if needed
  std::vector<DataPoints> m_data_vector;

  // Helpers for the file reading : 
  std::string m_reader;

  // Helpers for the data reading :
  int m_chosen_column = 0;
};

///////////
// INPUT //
///////////

template<class Index, class T>
inline void DataFile<Index, T>::loadHeader(std::ifstream & inputfile)
{
  go_to_beginning(inputfile);
  std::getline(inputfile, m_reader);
  fillList(m_header, m_reader, m_delim);
  if (!m_has_header) go_to_beginning(inputfile);
}

template <class Index, class T>
inline void DataFile<Index, T>::setupContainers()
{
  for (int index = 0; index<m_nb_columns; index++) 
  {
    auto const & data_index = m_columns_to_read[index]; // The actual index of the row in the file
    m_labels_column.push_back(m_header[data_index]);
    try
    {
      m_data_points.push_back(string_to<T> (m_header[data_index]));
      m_column_name_casted.push_back(true);
    }
    catch(CastImpossible const & e)
    {
      // If m_header[data_index] is not convertible to double then this column is of no interest
      std::cerr << e.what() << '\n';
      m_column_name_casted.push_back(false);
    }

    // The following is used to know if all the name of the columns are castable to double :
    m_all_column_name_casted = m_column_name_casted.AND();
  }
}

template<class Index, class T>
void DataFile<Index, T>::loadData(std::ifstream & inputfile)
{
  m_size = -1;
  DataLine data;
  while (std::getline(inputfile, m_reader))
  {
    m_size++;
    data.clear();
    fillList(data, m_reader, m_delim);
    if (data.size() != m_header.size())
    {
      print("Error: Number of entries at line", m_size, "do not match the header size");
      m_ok = false;
    }
    else
    {
      m_labels_rows.push_back(data[m_label_index]);
      Index label;
      try
      {
        label = string_to<Index>(data[m_label_index]);
        m_indexes.push_back(label);
      }
      catch(std::invalid_argument const & e)
      {
        print(m_labels_rows.back(), "in line", m_size, "raised", e.what());
        continue;
      }
      // std::cout << (label) << " : ";

      // The line containing the data :
      std::vector<T> newLineData(m_nb_columns, -1);
      
      // Loop over all the wanted rows :
      for (int index = 0; index < m_nb_columns; index++)
      {
        auto const & data_index = m_columns_to_read[index];// The actual index of the row in the file
        try
        {
          newLineData[index] = (string_to<T>(data[data_index]));
        }
        catch(std::invalid_argument const & e)
        {
          print("line", m_size, "column", index, "raised", e.what());
        }
      }
      m_data.emplace(label, newLineData);
    }
  }
}

template<class Index, class T>
void DataFile<Index, T>::load(std::string const & filename, bool const & has_header)
{
  std::ifstream inputfile((m_input_filename = filename), std::ios::in);
  
  m_has_header = has_header;

  checkInputfile(inputfile);

  loadHeader(inputfile);

  setupContainers();

  loadData(inputfile);

  inputfile.close();
}

////////////
// OUTPUT //
////////////

template<class Index, class T>
void DataFile<Index, T>::checkInputfile(std::ifstream & outputfile)
{
  if (!outputfile.good()) print("Carefull", m_input_filename, "is not good");
}

template<class Index, class T>
inline void DataFile<Index, T>::checkOutputfile(std::ofstream & outputfile)
{
  if (!outputfile.good()) print("Carefull", outputfile, "is not good");
}

template<class Index, class T>
void DataFile<Index, T>::writeHeader(std::ofstream & outputfile)
{
  outputfile << m_header << "\n";
}

template<class Index, class T>
void DataFile<Index, T>::writeData(std::ofstream & outputfile)
{
#ifdef UnorderedDataFile
  print("unordered datafile don't have an order output yet !!!");
#else // not UnorderedDataFile
  for (auto const & data : m_data)
#endif //UnorderedDataFile
  print(m_output_filename, "written");
}

template<class Index, class T>
void DataFile<Index, T>::write(std::string const & filename)
{
  std::ofstream outputfile((m_output_filename = filename), std::ios::out);
  
  checkOutputfile(outputfile);

  writeHeader(outputfile);

  writeData(outputfile);

  outputfile.close();
}

template<class Index, class T>
std::ostream& operator<<(std::ostream& out, DataFile<Index, T> const & data)
{
  for (auto const & data : data.data())
  {
    out << data.first << " : " << data.second << "\n";
  }
  return out;
}


//////////////////////
// SIMPLE DATA FILE //
//////////////////////

// template<class Index, class T>
// class SimpleDataFile 
// {
// public:
//   T const & operator[](T const & t) {return data[t][0];} 
//   DataFile<Index, T> data;
// };


#endif //DATAFILE_HPP