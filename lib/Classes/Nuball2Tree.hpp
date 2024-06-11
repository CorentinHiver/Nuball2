#ifndef NUBALL2TREE_HPP
#define NUBALL2TREE_HPP

#include "../libRoot.hpp"
#include "Event.hpp"

/**
 * @brief Access to the underlying TTree using -> operator (e.g. nuball2tree->GetEntry(n);)
 * @details
 * 
 */
class Nuball2Tree
{
public:
  Nuball2Tree() noexcept = default;
  Nuball2Tree(std::string const & filename) noexcept : m_filename(filename)  {this -> Open(m_filename);}
  Nuball2Tree(std::string const & filename, Event & event) noexcept : m_filename(filename)
  {
    this -> Open(m_filename);
    event.reading(*this);
  }

  ~Nuball2Tree() {if (m_file_opened) this->Close();}

  bool Open(std::string const & filename);
  void Close() {if (m_file_opened) {m_file -> Close(); m_file_opened = false; delete m_file;} else {print(m_filename, "not open");}}

  auto const & filename() {return m_filename;}
  auto cd() {return m_file->cd();}

  TTree* get() {return m_tree;}
  auto get() const {return m_tree;}
  TTree* operator-> () {return m_tree;}
  operator TTree*() {return m_tree;}

  bool const & ok() const {return m_ok;}

  bool readNext()
  {
    if (++m_cursor < m_entries)
    {
      m_tree->GetEntry(m_cursor);
      return true;
    }
    else return false;
  }

  void setMaxHits(int const & maxHits) 
  {
    if (m_file_opened && maxHits < m_entries) 
    {
      m_entries = maxHits;
      print("reading", m_entries, "events in", m_filename);
    }
  }

  auto const & cursor() {return m_cursor;}
  
private:
  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;
  bool m_ok = false;
  bool m_file_opened = false;
  std::string m_filename;
  std::string m_name = "Nuball2";
  std::string m_title;

  long m_cursor = 0;
  long m_entries = 0;
};

bool Nuball2Tree::Open(std::string const & filename)
{
  // Opens the file :
  m_file = TFile::Open(filename.c_str(), "READ");

  if (!m_file) 
  {
    print("Could not find", filename, "!"); 
    return (m_ok = m_file_opened = false);
  }
  
  m_file_opened = true;
  
  if (m_file->IsZombie()) 
  {
    print(filename, "is a Zombie !"); 
    return (m_ok = false);
  }

  // Extracts the tree :
  m_tree = m_file->Get<TTree>("Nuball2");

  if (!m_tree) 
  {
    print("Nuball2 tree not found in", filename); 
    return (m_ok = false);
  }
  else if (m_tree->IsZombie()) 
  {
    print(filename, "Nuball2 tree is a Zombie !"); 
    return (m_ok = false);
  }

  print("Reading", filename);
  
  m_entries = m_tree->GetEntries();

  return (m_ok = true);
}

std::ostream& operator<<(std::ostream& out, Nuball2Tree const & tree)
{
  out << tree.get();
  return out;
}

#endif //NUBALL2TREE_HPP