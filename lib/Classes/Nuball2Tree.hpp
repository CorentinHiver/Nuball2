#ifndef NUBALL2TREE_HPP
#define NUBALL2TREE_HPP

#include "Event.hpp"

/**
 * @brief test _ not finished
 * 
 */
class Nuball2Tree
{
public:
  Nuball2Tree(){}

  void reading(Event & event, std::string const & options = "ltEQ");
  void writting(Event & event, std::string const & name = "Nuball2", std::string const & options = "ltEQ");

  void writting(Hit & hit, std::string const & options = "ltEQp");

  TTree* get() {return m_tree.get();}
  TTree* operator-> () {return m_tree.get();}

  void Write(std::string outfilename);
  
private:
  std::unique_ptr<TTree> m_tree;
};

void Nuball2Tree::writting(Hit &hit, std::string const &options)
{
  m_tree.reset(new TTree("raw", "raw"));
  hit.writting(m_tree.get(), options);
}

void Nuball2Tree::reading(Event & event, std::string const & options)
{
  // OPEN FILE AND SO ON...
  event.reading(m_tree.get(), options);
}

void Nuball2Tree::writting(Event & event, std::string const & name, std::string const & options)
{
  m_tree.reset(new TTree(name.c_str(), name.c_str()));
  event.writting(m_tree.get(), options);
}

void Nuball2Tree::Write(std::string outfilename)
{
  if (extension(outfilename) != "root") outfilename = removeExtension(outfilename)+".root";
  std::unique_ptr<TFile> outFile (TFile::Open(outfilename.c_str(), "RECREATE"));
  outFile -> cd();
    m_tree -> Write();
    outFile -> Write();
  outFile -> Close();
}

#endif //NUBALL2TREE_HPP