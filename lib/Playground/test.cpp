// #define USE_RF 200
// #define N_SI_136
#define USE_DSSD
#define USE_PARIS

#include "Modules/Timeshifts.hpp"

int main()
{
  MTObject::Initialize(2);
  
  Detectors::Initialize();
  Timeshifts ts;
  ts.setParameters("outDir: tests outRoot: tests.root");
  ts.setIDFile(DetectorsList("index_129.list"));
  ts.calculate("/home/corentin/faster_data/N-SI-136/run_20.fast", 20);

  return -1;
}