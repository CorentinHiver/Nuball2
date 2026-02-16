#define DEVCO
// #define MULTITHREAD 2

#include "lib/Classes/Timer.hpp"

// #include "lib/libCo.hpp"
// #include "lib/libRoot.hpp"
// #include "lib/CoMT/CoMT.hpp"
// #include "lib/print.hpp"
// #include "lib/randomCo.hpp"
// #include "lib/Classes/FilesManager.hpp"
// #include "lib/Classes/Hit.hpp"
// #include "lib/Classes/Event.hpp"
// #include "lib/Classes/Arguments.hpp"
// #include "lib/Classes/Calibration.hpp"
// #include "lib/Classes/Transitions.hpp"
// #include "lib/Analyse/SpectraAlignator.hpp"
// #include "lib/MTObjects/MultiHist.hpp"
// #include "lib/libRootHeader.hpp"
// #include "lib/RootReader/RootReader.hpp"
// #include "lib/RootReader/TimeshiftCalculator.hpp"
// #include "lib/FasterReader/FasterRootInterface.hpp"
// #include "lib/RootReader/RootRunReader.hpp"
#include "lib/FasterReader/FasterRunReader.hpp"
// #include  "lib/Modules/CoRadware.hpp"
// #include  "lib/Modules/Timeshiftor.hpp"
// #include  "lib/Classes/CalibAndScale.hpp"
// #include  "lib/Classes/FilesManager.hpp"
// #include  "lib/Classes/Detectors.hpp"
// #include  "lib/Analyse/CloversV2.hpp"
// #include  "lib/Classes/FlexibleHisto.hpp"
// #include  "lib/Analyse/Chi2Minimiser.hpp"
// #include  "lib/Classes/CalibAndScale.hpp"
// #include "lib/FasterReader/FasterRootInterface.hpp"
// #include "lib/FasterReader/FasterReader.hpp"
// #include "TH1F.h"
// #include "TH2F.h"
// #include "TFile.h"

using namespace Colib;

int main(int argc, char** argv)
{
  Timer totTimer;


  // Event event;


  // MT::parallelise_function_nb<4>([](){
  //   for (int i = 0; i<10000; ++i) {print(i); if(MT::isKilled()) break;}
  //   MT::kill();
  // });


  // print (randomCo::ultra_fast_random_generator());



  // TimeshiftCalculator ts_cal(argv[1]);
  // ts_cal.setBins<NSI136::maxLabel>([](Label label) -> Time {
  //        if (NSI136::isGe [label])   return   1_ns;
  //   else if (NSI136::isBGO[label])   return 500_ps;
  //   else if (label == 251)           return 100_ps;
  //   else if (label == 252)           return 100_ps;
  //   else if (NSI136::isParis[label]) return 100_ps;
  //   else if (NSI136::isDSSD[label])  return   1_ns;
  //   else                             return  10_ns;
  // });
  // ts_cal.calculate();



  // RootReader reader(argv[1]);
  // while(reader.readNext()) print(reader);



  // RootRunReader reader(argc, argv);
  // reader.run();



  // FasterRootInterface::setTreeInMemory();
  FasterRunReader reader(argc, argv);
  reader.setEventTrigger([](Event const & event){
    for (int hit_i = 0; hit_i<event.mult; ++hit_i) if (NSI136::isGe[event.labels[hit_i]]) return true;
    return false;
  });
  reader.run();
  


  // FasterRootInterface::setTreeInMemory();
  // FasterRunReader reader(argc, argv);
  // reader.run();
  


  // FasterRunReader reader;
  // reader.addFiles(argv[1]);
  // if (argc>2) reader.setMaxHits(static_cast<int>(atof(argv[2])));
  // reader.setTimeWindow(500_ns);
  // reader.setMaxLoadedFiles(5);
  // reader.setOverwrite();
  // reader.setEventTrigger([](Event const & event) -> bool {
  //   return ((1 < event.mult) && (event.mult < 5));
  // });
  // reader.mergeAndConvertWithRF(251);
  // reader.mergeAndConvertWithRef(252);

  

  // FasterRootInterface interface(argv[1]);
  // // FasterRootInterface interface("/home/corentin/faster/136/60Co_center.fast/60Co_center_0001.fast");
  // if (2<argc) interface.setMaxHits(static_cast<int>(atof(argv[2])));
  // // interface.convert_raw("test.root");

  // interface.loadDatafile();
  // interface.checkTimeSorting();
  // interface.writeData("test.root");
  

  // Faster2RootV2 reader;
  // reader.addFiles(argv[1]);
  // size_t nbHits = 1e4;

  // Timer timer;
  // auto tree = new TTree("test", "test");
  // Hit hit;
  // hit.writing(tree);
  // FasterRootInterface reader("/home/corentin/nuball2/N-SI-136/run_75.fast/run_75_0001.fast", &hit);
  // reader.setMaxHits(nbHits);
  // while (reader.readNextRootHit()) tree->Fill();
  // reader.close();
  // print(timer);


  // auto histo = new TH1F("test", "test", nbHits, 0, nbHits);
  // auto histo2 = new TH2F("test2", "test2", 1000,0,1000, 1000,0,1e13);
  // Timer timer2;
  // FasterReader reader2("/home/corentin/nuball2/N-SI-136/run_75.fast/run_75_0001.fast");
  // while(bool_cast(reader2.readNextHit()) && reader2.getCursor() < nbHits) 
  // //   {
  //     reader.print();
  // //     histo->SetBinContent(reader.getCursor(), reader.getTimestamp());
  // //     histo2->Fill(reader.getHeader().label, reader.getTimestamp());
  // //   }
  // reader2.close();
  // print(timer2());
  // // Hit hit;
  // // FasterReader reader(&hit, "/home/corentin/nuball2/N-SI-136/run_75.fast/run_75_0001.fast");
  // // while(reader.readNextHit() && reader.getCursor() < nbHits)
  // // {
  // //   histo->SetBinContent(reader.getCursor(), hit.stamp);
  // //   histo2->Fill(hit.label, hit.stamp);
  // // }
  // auto out = TFile::Open("test.root", "recreate");
  // out->cd();
  // tree->Write();
  // // histo2->Write();
  // // out->Write();
  // out->Close();


  // TH1F* histo = new TH1F("test","test",1000,0,1000);
  // for (int i = 0; i<1000; ++i) histo->Fill(randomCo::fast_dirty_uniform());
  // FlexibleHisto histo_flex(histo);
  // Chi2Minimiser min;
  // CalibAndScale calib;
  // Timeshiftor ts;
  // detectors.load("136/index_129.list");
  // ts.calculate("/home/corentin/nuball2/N-SI-136/60Co_center_after.fast/");
  // CloversV2 clovers;

  // print(m_calib);
  // CalibAndScale _calib; _calib.readFrom(alignFilename, "100");
  // print(_calib);
  
  // for (auto const & alignFilename : alignFiles)
  // {
  //   print(File(alignFilename).filename());
  //   CalibAndScale _calib; _calib.readFrom(alignFilename);
  //   m_alignement.emplace(File(alignFilename).filename(), _calib);
  // }

  // Nucleus<236, 92> U236("Uranium 236");
  // U236.addLevelFast(1230, 0, 1); // 0
  // U236.addLevelFast(1010, 0, 0); // 1
  // U236.addLevelFast(900, 1, 1);  // 2
  // U236.addLevelFast(700, 2, 0);  // 3
  // U236.addLevelFast(400, 3, 1);  // 4
  // U236.addLevelFast(50, 4, 0);   // 5
  // U236.addLevelFast(0, 5, 1);    // 6
   
  // U236.processLevels();

  // U236.addTransition(0, 1, 0.5);  // 220
  // U236.addTransition(0, 2, 0.5);  // 330

  // U236.addTransition(1, 2, 1/3.); // 110
  // U236.addTransition(1, 3, 1/3.); // 310
  // U236.addTransition(1, 4, 1/3.); // 610

  // U236.addTransition(2, 3, 1/3.); // 200
  // U236.addTransition(2, 4, 1/3.); // 500
  // U236.addTransition(2, 6, 1/3.); // 900

  // U236.addTransition(3, 4, 0.5);  // 300
  // U236.addTransition(3, 5, 0.5);  // 650

  // U236.addTransition(4, 5, 0.5);  // 350
  // U236.addTransition(4, 6, 0.5);  // 400

  // U236.addTransition(5, 6, 1);    // 50

  // U236.processTransitions();

  // print(U236.gate(350, 1, 1000)); // 4 -> 5 Feed direct 4 : 300, 500, 610 | Indirect feed : {3 : 200, 310} {2 : 110} | Direct decay : {50}

  // auto fileRef = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); fileRef->cd();
  // auto histoRef = fileRef->Get<TH1F>("PARIS_FR1D5");

  // // Colib::CalibAndScale test = {0,0.5,1};
  // // auto calibrated_lin = test(histo); calibrated_lin->SetName("calibrated");
  // // auto calibrated_quad = test(histo, "quad"); calibrated_quad->SetName("quad");

  // // auto file = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); file->cd();
  // // auto histo = file->Get<TH1F>("PARIS_FR1D5");
  // // Colib::InterpolatedSpectrum inter(histo, 1);

  // auto fileTest = TFile::Open("136/calibrate_2025/histos/run_77.root", "READ"); fileTest->cd();
  // auto histoTest = fileTest->Get<TH1F>("PARIS_FR1D5");
  
  // Colib::MinimiserVariable b = {-10, 10, 1};
  // Colib::MinimiserVariable a = {0.9, 1.1, 0.1};
  // Colib::MinimiserVariable C = {0.5, 1.5, 0.1};

  
  // auto outFile = TFile::Open("Tests.root", "RECREATE"); outFile->cd();
  // Colib::Minimiser mini(histoRef, histoRef, b, a, C);
  // mini->Write();
  // // histo->Write("histo");
  // // calibrated_lin->Write("cl");
  // // calibrated_quad->Write("cq");
  // outFile->Close();
  // print("Tests.root written");

  // fileTest->Close();
  // fileRef->Close();

  print("Time : ", totTimer);
  return 0;
};

// g++ -o exec forDebugOnly.cpp `root-config --cflags` `root-config --libs` -g -DDEBUG -Wall -Wextra