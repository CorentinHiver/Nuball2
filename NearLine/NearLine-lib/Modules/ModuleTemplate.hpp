#ifndef PARIS_H
#define PARIS_H
#include "../NearLine.hpp"

//Start doing the following :
//search (ctrl+F or equivalent)
//type "ModuleTemplate"
//then replace by the name of your class
//And modify the template class the way you wante
//
//You'll need to make the following steps in NearLine:
// 1 : forward declaration before the NearLine class : class ModuleTemplate;
// 2 : declare the class friend like this : friend class ModuleTemplate;
// 3 : declare the class like this : ModuleTemplate *m_mt = nullprt;
// 4 : import the module AFTER NearLine class
// 5 : instantiate the module in the NearLine::setConfig member : m_mt = new ModuleTemplate(this,whatever...);
// 6 : initialize the module in the NearLine::Initialize() member : m_mt.Initialize();
// 7 : play with it in the NearLine::processFile() member (using Fill or any other user defined function)
// 8 : Calculate or Write anything at the end of runs in the NearLine::WriteData() member : m_mt.Write() | m_mt.Calculate()
// 9 : in the final method to use it, dont forget to DELETE IT after checking it exists

class ModuleTemplate
{
public:
  ModuleTemplate(NearLine* nearline) : n(nearline){}

  operator bool() {return m_activated;}
  void operator=(bool activate) {m_activated = activate;}
  // User defined operator overloads :

  Bool_t Initialize();
  Bool_t SetConfig(std::istringstream & is);
  Bool_t Check();
  void Fill(Hit & hit, UShort_t const & thread_nb);
  void Fill(Buffer & buffer, UShort_t const & thread_nb);
  void Fill(Event & event, UShort_t const & thread_nb);
  void Calculate();
  void Write();

  // User defined methods :

  // User defined setters :

  // User defined getters :


private:
  Bool_t m_activated = true;
  NearLine* n = nullptr;

  // User defined members :

  // User defined histograms :

};

Bool_t ModuleTemplate::SetConfig(std::istringstream & is)
{
  // Fill here any configuration
  return true;
}

Bool_t Analyse::Check()
{
  // Conditions to verify to make the code work correctly
}

Bool_t ModuleTemplate::Initialize()
{
  // Here initialize all your histograms :
  return true;
}

void ModuleTemplate::Fill(Hit const & hit)
{
  // Implement here how to manage a single hit
}

void ModuleTemplate::Fill(Buffer const & buffer)
{
  // Implement here how to manage a buffer of hits (see utils.hpp)
}

void ModuleTemplate::Fill(Event const & event)
{
  // Implement here how to manage an event (see Classes/Event.hpp)
}

void ModuleTemplate::Calculate()
{
  // How to proceed the stored histograms to extract informations
}
void ModuleTemplate::Write()
{
  // How to write the stored histograms
}

#endif //PARIS_H
