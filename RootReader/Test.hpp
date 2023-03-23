#ifndef TEST_H
#define TEST_H
#include "../../lib/utils.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../Classes/Parameters.hpp"

std::string param_string = "Test";

class Test
{
public:
  Test(){};
  launch(Parameters & p);
  setParameters(std::vector<std::string> const &  param);
  void InitializeManip();
  static void run(Parameters & p, Test & test);
  void FillRaw(Event const & event);
  void FillSorted(Sorted_Event const & event_s, Event const & event);
  void Write();
private:
  // Parameters
  friend class MTObject;
  std::string outDir  = "129/Test/";
  std::string outRoot = "Test.root";
};

bool Test::launch(Parameters & p)
{
  if (!this -> setParameters(p.getParameters(param_string))) return false;
  this -> InitializeManip();
  print("Starting !");
  MTObject::parallelise_function(run, p, *this);
  this -> Write();
  return true;
}

#endif //TEST_H
