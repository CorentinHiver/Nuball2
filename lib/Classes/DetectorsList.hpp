#ifndef DETECTORSLIST_H
#define DETECTORSLIST_H

class DetectorsList
{
public:
  DetectorsList(){}
  DetectorsList(std::string const & filename) : m_filename(filename) { this -> load(m_filename); }
  DetectorsList(DetectorsList const & otherList) : m_ok(otherList.m_ok), m_filename(otherList.m_filename), m_list(otherList.m_list), m_reversed_list(otherList.m_reversed_list) {  }
  void load (std::string const & filename);

  bool good() {return  m_ok;}  bool is_good() {return  m_ok;}
  bool bad()  {return !m_ok;}  bool is_bad()  {return !m_ok;}

  auto const size() const {return m_list.size();}

  auto begin() {return m_list.begin();}
  auto end  () {return m_list.end  ();}

  auto const & labelsList() const {return m_reversed_list;}
  auto const & reverse()    const {return m_reversed_list;}
  auto const & get()        const {return m_list         ;}

  auto const & getName (ushort      const & label) {return m_list[label]        ;}
  auto const & getLabel(std::string const & name ) {return m_reversed_list[name];}

  void operator=(std::string const & filename) {this -> load(filename);}
  DetectorsList& operator=(DetectorsList otherList)
  {
    m_reversed_list = otherList.labelsList();
    m_list = otherList.get();
    return *this;
  }

  std::string const & operator[] (int i) const {return m_list[i];}
  operator bool() const & {return m_ok;}


private:
  // Useful informations
  bool m_ok = false;
  std::string m_filename;

  // Containers
  std::vector<std::string> m_list;
  std::map<std::string, ushort> m_reversed_list;
};

void DetectorsList::load(std::string const & filename)
{
  std::ifstream inputfile(filename, std::ifstream::in);
  if (!inputfile.is_open())
  {
    m_ok = false;
    m_list.resize(0);
    print("CANNOT OPEN", filename, "!!");
    return;
  }
  else if (file_is_empty(inputfile))
  {
    m_ok = false;
    print(filename, "is empty !!");
    m_list.resize(0);
    return;
  }
  // ----------------------------------------------------- //
  //First reading of the file : extract the maximum label to be the size of the vector
  ushort size = 0; std::string line = "", name = ""; int label = 0;
  std::string oneline;
  while (getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label;
    if (size<label) size = label;
  }
  // ----------------------------------------------------- //
  //Second reading : fill the vector
  m_list.resize(size+1);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while(getline(inputfile, oneline))
  {
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1)
    {
      m_list[label] = name;
      if (name!="") m_reversed_list[name] = label;
    }
  }
  inputfile.close();
  m_ok = true;
  print("Labels extracted from", filename);
}
#endif //DETECTORSLIST_H
