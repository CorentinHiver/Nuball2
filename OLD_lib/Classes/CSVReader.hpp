
// #if (__cplusplus >= 201703L)

// /**
//  * @brief This class allows one to read one specific format of csv file (see details)
//  * 
//  * @details
//  * 
//  * The format of the data MUST be the following : 
//  * 
//  * [[Name of the columns]]
//  * [[First row data]]
//  * [[....]]
//  * [[Last row data]]
//  * 
//  * Then, declare the reader in two steps : 
//  * 
//  * first construct the reader using the constructor.
//  * 
//  * 
//  */
// template<class... T>
// class CSVReader
// {
// public:
//   CSVReader(std::string const & filename, char const & delim = ';') {this -> open(filename, delim);}

//   bool open(std::string const & filename, char const & delim = ';');

//   operator bool() const & {return m_ok;}

// private:
//   std::vector<std::string> m_header;
//   std::vector<std::tuple<T...>> m_data;
//   bool m_ok = false;
// };

// template<class... T>
// bool CSVReader<T...>::open(std::string const & filename, char const & delim)
// {
//   // Open file :
//   std::ifstream file(filename, std::ios::in);
//   if (!file) {print(filename, "not found"); return (m_ok = false);}

//   // Read names header : 
//   std::string reader;
//   std::getline(file, reader);
//   m_header = getList(reader, delim);
//   print(m_header);

//   // Read the types header :
//   while (std::getline(file, reader))
//   {
//     std::vector<std::string> typeNames = getList(reader, delim);
//      if (typeNames.size() != sizeof...(T))
//     {
//         // Handle the case where the number of types in the header
//         // doesn't match the number of template arguments.
//         print("Error: Number of types in the header doesn't match the template arguments.");
//         return (m_ok = false);
//     }
//   }


//   return true;
// }

// #endif //(__cplusplus >= 201703L)
