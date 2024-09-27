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
  Nuball2Tree(std::string const & filename, Event & event, std::string const & options) noexcept : m_filename(filename)
  {
    this -> Open(m_filename);
    event.reading(*this, options);
  }
  Nuball2Tree(std::string const & filename, Hit & hit) noexcept : m_filename(filename)
  {
    this -> Open(m_filename);
    hit.reading(*this);
  }
  Nuball2Tree(std::string const & filename, Hit & hit, std::string const & options) noexcept : m_filename(filename)
  {
    this -> Open(m_filename);
    hit.reading(*this, options);
  }

  ~Nuball2Tree() {if (m_file_opened) this->Close();}

  bool Open(std::string const & filename);
  void Close() 
  {
    if (m_file_opened) 
    {
      m_file -> Close(); 
      m_file_opened = false;
    }
    else print(m_filename, "not open or already closed");
  }

  void loadRAM();
  auto const & filename() {return m_filename;}
  auto cd() {return m_file->cd();}

  auto get() {return m_tree;}
  auto get() const {return m_tree;}
  TTree* operator-> () {return m_tree;}
  operator TTree*() {return m_tree;}

  bool const & ok() const {return m_ok;}
  operator bool() const {return m_ok;}

  bool readNext()
  {
    if (++m_cursor < m_entries)
    {
      m_tree->GetEntry(m_cursor);
      return true;
    }
    else return false;
  }

  void setMaxHits(long const & maxHits) 
  {
    if (0 < m_entries && 0 < maxHits && maxHits < m_entries) 
    {
      print("reading", nicer_double(maxHits,0), "events in", rmPathAndExt(m_filename), "out of", nicer_double(m_entries,0));
      m_entries = maxHits;
    }
  }

  auto const & getMaxHits() const {return m_entries;}

  auto const & cursor() const {return m_cursor;}

  auto & cursor() {return m_cursor;}

  void reset() {m_cursor = 0;}
  
private:
  unique_TFile m_file;
  TTree* m_tree;
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
  m_file.reset(TFile::Open(filename.c_str(), "READ"));

  if (!m_file.get()) 
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

void Nuball2Tree::loadRAM()
{
  // auto memory_tree = (m_tree->CloneTree());
  // memory_tree->SetDirectory(nullptr);
  m_tree->SetDirectory(nullptr);
  m_file->Close();
  // m_tree = memory_tree;
  m_file_opened = false;
}

std::ostream& operator<<(std::ostream& out, Nuball2Tree const & tree)
{
  out << tree.get();
  return out;
}

#endif //NUBALL2TREE_HPP