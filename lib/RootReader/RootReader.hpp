#pragma once

#include "../Classes/RootEvent.hpp"
#include "../Classes/Arguments.hpp"
#include "../Nuball2.hh"
#include "TFile.h"

class RootReader
{
public:
  RootReader() noexcept = default;
  RootReader(std::string const & filename)
  {
    open(filename);
  }

  void open(std::string const & filename)
  {
    m_file = TFile::Open(filename.c_str());
    if (!fileOk()) Colib::throw_error(filename+" not available !");
    m_tree = m_file->Get<TTree>("Nuball2");
    if (!treeOk()) Colib::throw_error(filename+" has no Nuball2 tree available !");
    if(std::string(m_tree -> GetTitle()) == "Nuball2_Hits") 
    {
      m_hit.reading(m_tree);
      m_useHits = true;
    }
    else if(std::string(m_tree -> GetTitle()) == "Nuball2_Events") 
    {
      m_event.reading(m_tree);
      m_useEvents = true;
    }
    m_maxEntries = m_tree->GetEntries();
    print(filename, "open with", m_maxEntries, "entries");
  }

  bool readNext()
  {
    if (m_cursor <= m_maxEntries) 
    {
      m_tree->GetEntry(m_cursor++);
      return true;
    }
    else return false;
  }

  void restart() {m_cursor = 0;}

  auto const & getCursor() const { return m_cursor;}

  friend std::ostream& operator<<(std::ostream& out, RootReader const & reader)
  {
    if (reader.useHits()) out << reader.getHit();
    else if (reader.useEvents()) out << reader.getEvent();
    return out;
  }

  bool const & useHits  () const {return m_useHits  ;}
  bool const & useEvents() const {return m_useEvents;}

  Hit   const & getHit  () const {return m_hit  ;}
  Event const & getEvent() const {return m_event;}

  bool treeOk() const {return (m_tree && !m_tree->IsZombie());}
  bool fileOk() const {return (m_file && !m_file->IsZombie());}

  auto nbEntries() const noexcept {return (treeOk()) ? m_maxEntries : 0;}
  void setMaxHits(ULong64_t nb) {m_maxEntries = nb;}

  void printLoadingPercents() const 
  {
    if ((m_cursor == nbEntries()) || (m_cursor % (nbEntries()/100) == 0)) 
      printsln(Colib::nicer_double(m_cursor), Colib::percent(m_cursor, ULong64_cast(m_tree->GetEntries())), "     ");
  }

protected:
  TFile* m_file = nullptr;
  TTree* m_tree = nullptr;
  RootHit m_hit;
  RootEvent m_event;

  // States:
  bool m_useHits   = false;
  bool m_useEvents = false;
  ULong64_t m_cursor = 0;
  ULong64_t m_maxEntries = 0;
};