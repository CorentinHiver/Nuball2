// #define USE_RF 200
// #define N_SI_136
#define USE_DSSD
#define USE_PARIS
#define USE_RF 200

#include "Modules/Timeshifts.hpp"
// #include "Modules/Calibration.hpp"
// #include "../libCo.hpp"
#include <Detectors.hpp>

int main(int argc, char ** argv)
{
  MTObject::Initialize(3);

  int run_number = 76;
  std::string run_name = "run_"+std::to_string(run_number);
  Path runpath = "/home/corentin/faster_data/N-SI-136/"+run_name+".fast/";
  if (!runpath) throw std::runtime_error("NO DATA");
  Detectors det("index_129.list");
  auto nbFiles = (argc>1) ? std::atoi(argv[1]) : 20;

// if (true)
// {
//   Timeshifts ts;
//   ts.setDetectors(det);

//   ts.setOutDir(".");
//   ts.setEminADC(30000);
//   // ts.setMaxHits(10000);


//   ts.setMult(2,2);
//   ts.verbose(true);
//   ts.calculate(runpath, nbFiles);

//   // ts.load("tests/Timeshifts/run_20.dT");
//   ts.verify(runpath, nbFiles/5);

//   ts.write(run_name+"mult2"); 
// }

  RF_Manager::set_offset(30000);
  Timeshifts ts;
  ts.setDetectors(det);

  ts.setOutDir(".");
  // ts.setEminADC(30000);
  // ts.setMaxHits(10000);

  // ts.setMult(3,3);
  // ts.verbose(true);
  // if (run_number < 40) ts.checkForPreprompt();
  // ts.calculate(runpath, nbFiles);

  ts.load("Timeshifts/run_20_test.dT");
  ts.verify(runpath, 6);

  // ts.write(run_name+"_test"); 
  ts.writeRoot(run_name+"_verify"); 
  // ts.writeRoot(run_name+"calculate"); 

  // Calibration calib("index_129.list");
  // calib.Initialize();
  // MTObject::Initialize(2);

  // std::thread t2([]()
  // {
  //   int i; 
  //   std::cin>>i; 
  //   print("1",std::this_thread::get_id());
  // });

  // std::thread *t1 = new std::thread([](){
  //   for(int i = 0; i<10; i++)
  //   {
  //     print(i); 
  //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  //   };
  // });

  // auto t3 = t1;

  // t3->join();
  // t2.join();
  
  // Detectors::Initialize();
  // Timeshifts ts;
  // ts.setParameters("outDir: tests outRoot: tests.root");
  // ts.setIDFile(DetectorsList("index_129.list"));
  // ts.calculate("/home/corentin/faster_data/N-SI-136/run_20.fast", 20);

  return -1;
}