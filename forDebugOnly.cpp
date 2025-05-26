#define DEVCO
// #include "lib/libRoot.hpp"
// #include "lib/Classes/FilesManager.hpp"
#include "lib/Classes/Transitions.hpp"
// #include "lib/Analyse/SpectraAlignator.hpp"
// #include "lib/MTObjects/MultiHist.hpp"
// #include "lib/libRoot.hpp"
// #include "lib/libRootHeader.hpp"
// #include  "lib/Modules/CoRadware.hpp"

int main()
{
  Nucleus<236, 92> U236("Uranium 236");
  U236.addLevelFast(1230, 0, 1); // 0
  U236.addLevelFast(1010, 0, 0); // 1
  U236.addLevelFast(900, 1, 1);  // 2
  U236.addLevelFast(700, 2, 0);  // 3
  U236.addLevelFast(400, 3, 1);  // 4
  U236.addLevelFast(50, 4, 0);   // 5
  U236.addLevelFast(0, 5, 1);    // 6
   
  U236.processLevels();

  U236.addTransition(0, 1, 0.5);  // 220
  U236.addTransition(0, 2, 0.5);  // 330

  U236.addTransition(1, 2, 1/3.); // 110
  U236.addTransition(1, 3, 1/3.); // 310
  U236.addTransition(1, 4, 1/3.); // 610

  U236.addTransition(2, 3, 1/3.); // 200
  U236.addTransition(2, 4, 1/3.); // 500
  U236.addTransition(2, 6, 1/3.); // 900

  U236.addTransition(3, 4, 0.5);  // 300
  U236.addTransition(3, 5, 0.5);  // 650

  U236.addTransition(4, 5, 0.5);  // 350
  U236.addTransition(4, 6, 0.5);  // 400

  U236.addTransition(5, 6, 1);    // 50

  U236.processTransitions();

  print(U236.gate(350, 1, 1000)); // 4 -> 5 Feed direct 4 : 300, 500, 610 | Indirect feed : {3 : 200, 310} {2 : 110} | Direct decay : {50}

  // print(CoLib::typeRoot<float>());
  // auto fileRef = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); fileRef->cd();
  // auto histoRef = fileRef->Get<TH1F>("PARIS_FR1D5");

  // // CoLib::CalibAndScale test = {0,0.5,1};
  // // auto calibrated_lin = test(histo); calibrated_lin->SetName("calibrated");
  // // auto calibrated_quad = test(histo, "quad"); calibrated_quad->SetName("quad");

  // // auto file = TFile::Open("136/calibrate_2025/histos/run_76.root", "READ"); file->cd();
  // // auto histo = file->Get<TH1F>("PARIS_FR1D5");
  // // CoLib::InterpolatedSpectrum inter(histo, 1);

  // auto fileTest = TFile::Open("136/calibrate_2025/histos/run_77.root", "READ"); fileTest->cd();
  // auto histoTest = fileTest->Get<TH1F>("PARIS_FR1D5");
  
  // CoLib::MinimiserVariable b = {-10, 10, 1};
  // CoLib::MinimiserVariable a = {0.9, 1.1, 0.1};
  // CoLib::MinimiserVariable C = {0.5, 1.5, 0.1};

  
  // auto outFile = TFile::Open("Tests.root", "RECREATE"); outFile->cd();
  // CoLib::Minimiser mini(histoRef, histoRef, b, a, C);
  // mini->Write();
  // // histo->Write("histo");
  // // calibrated_lin->Write("cl");
  // // calibrated_quad->Write("cq");
  // outFile->Close();
  // print("Tests.root written");

  // fileTest->Close();
  // fileRef->Close();

  


  return 0;
};

// g++ -o exec forDebugOnly.cpp `root-config --cflags` `root-config --libs` -g -DDEBUG -Wall -Wextra