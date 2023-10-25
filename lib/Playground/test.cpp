// #define USE_RF 200
// #define N_SI_136
// #define USE_DSSD
// #define USE_PARIS
// #define USE_RF 200

#include "../libRoot.hpp"
#include <CloverEfficiency.hpp>
// #include <Calibration.hpp>
// #include <RunMatrixator.hpp>
// #include <Detectors.hpp>
// #include <Timeshifts.hpp>
// #include <Convertor.hpp>
// #include <MTFasterReader.hpp>

int main(int argc, char ** argv)
{
  // std::string index_file = "index_129.list";
  // detectors.load(index_file);

  // fuse_all_histo("histos/", true);

  // auto file = TFile::Open("fused_histo_final.root", "READ");
  // std::vector<std::pair<double, double>> gates = {{840, 850},{639, 644}, {725, 730}, {507, 515}};
  // // auto histo = file->Get<TH2F>("S1_2_DSSD_prompt");
  // // auto histo = file->Get<TH2F>("PARIS_BR2D1_prompt");
  // // CoAnalyse::removeRandomY(histo, -1, -1, true, gates);

  // auto PARIS_BR2D1_prompt = file->Get<TH2F>("PARIS_BR2D1_prompt");
  // if (argc == 2) CoAnalyse::removeRandomBidim(PARIS_BR2D1_prompt, std::stoi(argv[1]), gates);
  // else CoAnalyse::removeRandomBidim(PARIS_BR2D1_prompt, gates, 1);

  // auto S1_2_DSSD_prompt = file->Get<TH2F>("S1_2_DSSD_prompt");
  // if (argc == 2) CoAnalyse::removeRandomBidim(S1_2_DSSD_prompt, std::stoi(argv[1]), gates);
  // else CoAnalyse::removeRandomBidim(S1_2_DSSD_prompt, gates, 1);

  // auto pp = file->Get<TH2F>("pp");
  // if (argc == 2) CoAnalyse::removeRandomBidim(pp, gates, std::stoi(argv[1]));
  // else CoAnalyse::removeRandomBidim(pp, gates, 1);
  // auto dd = file->Get<TH2F>("dd");
  // if (argc == 2) CoAnalyse::removeRandomBidim(dd, gates, std::stoi(argv[1]));
  // else CoAnalyse::removeRandomBidim(dd, gates, 1);
  // auto dp = file->Get<TH2F>("dp");
  // if (argc == 2) CoAnalyse::removeRandomBidim(dp, gates, std::stoi(argv[1]));
  // else CoAnalyse::removeRandomBidim(dp, gates, 1);

  // CoAnalyse::removeRandomY(pp, -1, -1, false, gates);
  // auto dd = file->Get<TH2F>("dd");
  // CoAnalyse::removeRandomY(dd, -1, -1, true, gates);
  // auto dp = file->Get<TH2F>("dp");
  // CoAnalyse::removeRandomY(dp, -1, -1, true, gates);
  

  // int nb_files = -1;
  // int nb_threads = 1;
  // if (argc > 1)
  // {
  //   std::string command;
  //   for(int i = 1; i < argc; i++)
  //   {
  //     command = argv[i];
  //          if (command == "-f") {nb_files = std::atoi(argv[i++]);}
  //     else if (command == "-n") {FasterReader::setMaxHits(std::atoi(argv[++i]));}
  //     else if (command == "-m") 
  //     {
  //       nb_threads = std::atoi(argv[++i]);
  //       MTObject::setThreadsNb(nb_threads);
  //       MTObject::Initialize();
  //     }
  //     else if (command == "-i") {index_file = argv[++i];}
  //     else {throw std::runtime_error("command " + command + " unkown");}
  //   }
  // }


  // CloverEfficiency ce(argc, argv);

  // std::string datapath;
  // auto const home = Path::home().string();
  // if (home == "/home/corentin/") datapath = home+"faster_data/";
  // else if (home == "/home/faster/") datapath = home+"nuball2/";

  // --- RUN MATRIXATOR : --- //

  // // rm.keepSingles();
  // for (int run_i = 75; run_i<76; run_i++)
  // {
  //   RunMatrixator rm;
  //   rm.dontMatrixate("ge");
  //   rm.dontMatrixate("paris");
  //   rm.maxRawMult(10);
  //   rm.setCalibration("../../136/conversion_200ns/136_final.calib");
  //   std::string run_str = "run_"+std::to_string(run_i); 
  //   Timeshifts ts(datapath+"N-SI-136-root_P/Timeshifts/"+run_str+".dT");
  //   rm.setTimeshifts(ts);
  //   rm.run(datapath+"N-SI-136/"+run_str+".fast/");
  // }

  // --- Up to date example : --- //
  // Calibration calib;
  // calib.load(/* Calibration file */);
  // calib.calibrateData(/* Europium run path */);
  // calib.writeCalibratedRoot(/* Name of the output_file.root */);

  // RunMatrixator rm("~/faster_data/N-SI-120-sources/");
  // RunMatrixator rm(argv[1]);
  // MTObject::Initialize(3);

  // --- Almost up to date / Not checked : --- //

  
  // Timeshifts ts;
  // ts.setOutDir(".");
  // ts.setMult(2,2);
  // ts.verbose(true);
  // ts.calculate(argv[1], (argc>2) ? std::stoi(argv[2]) : -1);
  // ts.verify(argv[1], (argc>3) ? std::stoi(argv[3]) : 5);
  // ts.write("136_60Co"); 
  

  // --- Other : --- //
  
  // std::vector<long> vec(100,0.0);
  // Timer timeVec;
  // for (int i = 0; i<2000000000; i++) 
  // {
  //   for (int i = 0; i<5; i++) vec.push_back(time(0));
  //   vec.clear();
  // }
  // print(timeVec(), timeVec.unit());

  // StaticVector<long> svec(100,0.0);
  // Timer timeSVec;
  // for (int i = 0; i<2000000000; i++) 
  // {
  //   for (int i = 0; i<5; i++) svec.push_back(time(0));
  //   svec.clear();
  // }
  // print(timeSVec(), timeSVec.unit());

  // Timer timeSVec2;
  // for (int i = 0; i<2000000000; i++) 
  // {
  //   for (int i = 0; i<5; i++) svec.push_back(time(0));
  //   svec.resize();
  // }
  // print(timeSVec2(), timeSVec2.unit());

  // StaticVector<long> svec2(2,0);
  // long i = 61;
  // long j = 62;
  // svec2.move_back(std::move(i));
  // svec2.move_back(std::move(j));
  // i = 5;
  // print(svec2, i, j);

  // Convertor convertor(argc, argv, [](Event const & event)
  // {
  //   for (int hit = 0; hit<event.mult; hit++)
  //   {
  //     auto const & label = event.labels[hit];
  //     if (isDSSD[label]) return true;
  //   }
  //   return false;
  // });




  // MTObject::Initialize(2);
  // Calibration calib;
  // calib.load("../../136/conversion_200ns/136_final.calib");
  // // calib.calibrateFasterData("/home/corentin/faster_data/N-SI-129/152EU_N1N2.fast/");
  // calib.calibrateRootData("/home/corentin/faster_data/N-SI-136-calibrations/152Eu_center_2/", 1);
  // calib.writeCalibratedHisto("calib/test_calib_rootdata_152Eu.root");

  // calibration.loadRootHisto("232Th.root");
  // pauseCo();
  // calibration.verbose(true);
  // calibration.calculate(argv[1], (argc>2) ? std::stoi(argv[2]) : -1, "152Eu");
  // calibration.loadData("/home/corentin/faster_data/N-SI-136/152_Eu_center_after.fast", 20);
  // calibration.loadData(argv[1], (argc>2) ? std::stoi(argv[2]) : -1);
  // calibration.verify();

    // int a = 1;
    // int b = 2;

    // auto const & c = a+b;
    // auto const d = a+b;
    // b = 5;
    // print(c,d);


  // calibration.load("../../136/conversion_200ns/129.calib");
  // calibration.calibrateData(argv[1], (argc>2) ? std::stoi(argv[2]) : 1);
  // calibration.writeCalibratedData("test_calib_data.root");

  // MTObject::Initialize(std::stoi(argv[1]));
  //   Timeshifts ts;
  //   ts.setDetectors(detectors);
  //   ts.setOutDir(".");
  //   ts.setMult(2,2);
  //   ts.verbose(true);
  //   ts.calculate("/home/corentin/faster_data/N-SI-136/run_15.fast", 50);
    // ts.calculate(argv[1], (argc>2) ? std::stoi(argv[2]) : -1);
    // ts.verify(argv[1], (argc>3) ? std::stoi(argv[3]) : 5);
    // ts.write("test"); 

  // ("/home/corentin/faster_data/N-SI-129/152Eu_N1_9.fast", "./tests/152Eu_N1_9", nb_files, false);
  // convertor.addFolder("t");
  // convertor.convert()

  // int run_number = 76;
  // std::string run_name = "run_"+std::to_string(run_number);
  // Path runpath = "/home/corentin/faster_data/N-SI-136/"+run_name+".fast/";
  // if (!runpath) throw std::runtime_error("NO DATA");
  // Detectors det("index_129.list");
  // auto nbFiles = (argc>1) ? std::atoi(argv[1]) : 20;

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

  // RF_Manager::set_offset(30000);
  // Timeshifts ts;
  // ts.setDetectors(det);

  // ts.setOutDir(".");
  // ts.setEminADC(30000);
  // ts.setMaxHits(10000);

  // ts.setMult(3,3);
  // ts.verbose(true);
  // if (run_number < 40) ts.checkForPreprompt();
  // ts.calculate(runpath, nbFiles);

  // ts.load("Timeshifts/run_20_test.dT");
  // ts.verify(runpath, 6);

  // ts.write(run_name+"_test"); 
  // ts.writeRoot(run_name+"_verify"); 
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

// #include <iostream>
// #include <vector>
// #include <chrono>
// using namespace std;



// int main()
// {
    
//     std::vector<int> vec;
//     int iterations = (int)(1.E9);
    
//     auto start = std::chrono::high_resolution_clock::now();

//     for (int loop = 0; loop<10; loop++)
//     {
//       vec.reserve(iterations);
//       for (int i = 0; i<iterations; i++)
//       {
//           vec[i] = i*time(0);
//       }
//       vec.clear();
//     }

//     auto end = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

//     // Print the duration
//     std::cout << "Time taken: " << duration.count() << " milliseconds\n";

//     return 0;
// }