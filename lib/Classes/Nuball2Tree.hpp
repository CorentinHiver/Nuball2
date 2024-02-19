#ifndef NUBALL2TREE_HPP
#define NUBALL2TREE_HPP

#include "../libCo.hpp"
#include "Event.hpp"

/**
 * @brief Access to the underlying TTree using -> operator (e.g. nuball2tree->GetEntry(n);)
 * 
 */
class Nuball2Tree
{
public:
  Nuball2Tree(std::string const & filename){this -> Open(filename);}
  Nuball2Tree(std::string const & filename, Event & event)
  {
    this -> Open(filename);
    event.reading(*this);
  }

  ~Nuball2Tree() 
  {
    if (file_opened)
    {
      m_file->Close();
      delete m_file;
    }
  }

  bool Open(std::string const & filename);

  TTree* get() {return m_tree;}
  auto const get() const {return m_tree;}
  TTree* operator-> () {return m_tree;}
  operator TTree*() {return m_tree;}

  bool const & ok() const {return m_ok;}
  
private:
  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;
  bool m_ok = false;
  bool file_opened = false;
  std::string m_name = "Nuball2";
  std::string m_title;
};

bool Nuball2Tree::Open(std::string const & filename)
{
  // Opens the file :
  m_file = TFile::Open(filename.c_str(), "READ");

  if (!m_file) 
  {
    print("Could not find", filename, "!"); 
    return (m_ok = file_opened = false);
  }
  
  file_opened = true;
  
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
  
  return (m_ok = true);
}

std::ostream& operator<<(std::ostream& out, Nuball2Tree const & tree)
{
  out << tree.get();
  return out;
}

#endif //NUBALL2TREE_HPP