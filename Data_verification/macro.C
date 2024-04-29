#include "../lib/libRoot.hpp"
#include "../lib/Classes/RF_Manager.hpp"

using Label = unsigned short;
inline bool isGe(Label const & label) {return (label < 200 && (label+1)%6 > 1);}
void macro(int choice = 0)
{
  TH2F* gg = new TH2F("gg", "gg", 4096,0,2048, 4096,0,2048);
  TH2F* pp = new TH2F("pp", "pp", 4096,0,2048, 4096,0,2048);
  TH2F* dd = new TH2F("dd", "dd", 4096,0,2048, 4096,0,2048);
  TH2F* E_dT = new TH2F("E_dT", "E_dT", 600,-100,200, 4096,0,2048);

  TChain* chain0 = new TChain("Nuball2");
  TChain* chain1 = new TChain("Nuball2");
        if (choice == 0 || choice == 2) chain0->Add("~/nuball2/N-SI-136-root_P/OLD/run_100/run_100_*");
  else  if (choice == 1 || choice == 2) chain1->Add("~/nuball2/N-SI-136-root_P/merged/run_100.root");
  // else  if (choice == 1 || choice == 2) chain1->Add("~/nuball2/N-SI-136-root_P/run_105/run_105_*");

  auto filename0 = "test_OLD.root";
  auto filename1 = "test_new.root";
  auto const & filename = (choice == 0) ? filename0 : filename1;

  int mult0 = 0;
  Label label0[255];
  ULong64_t timestamp0[255];
  float energy0[255];

  int mult1 = 0;
  ULong64_t stamp1 = 0ull;
  Label label1[255];
  Long64_t time1[255];
  float energy1[255];

  if (choice == 0 || choice == 2)
  {
    chain0 -> SetBranchAddress("mult"  , &mult0);
    chain0 -> SetBranchAddress("label" , &label0);
    chain0 -> SetBranchAddress("nrjcal", &energy0);
    chain0 -> SetBranchAddress("time"  , &timestamp0);
  }

  if (choice == 1 || choice == 2)
  {
    chain1 -> SetBranchAddress("mult"  , &mult1);
    chain1 -> SetBranchAddress("stamp" , &stamp1);
    chain1 -> SetBranchAddress("label" , &label1);
    chain1 -> SetBranchAddress("nrj"  , &energy1);
    chain1 -> SetBranchAddress("time" , &time1);
  }

  RF_Manager rf;
  bool rf_found = false;

  print("Startup done, let's work");

  if (choice == 0 || choice == 1)
  {
    auto chain = ((choice == 0) ? chain0 : chain1);
    for (int evt_i = 1; evt_i < chain->GetEntries(); evt_i++)
    {
      chain->GetEntry(evt_i);
      auto const & mult = ((choice == 0) ? mult0 : mult1);

      if (evt_i%((int)10.e+6) == 0) std::cout << evt_i/10.e+6 << "M events" << std::endl;
      for (int hit_i = 0; hit_i < mult; hit_i++)
      {
        auto const & label_i = ((choice == 0) ? label0[hit_i] : label1[hit_i]);
        if (choice == 0) 
        {
          if (label_i == 251) {rf.set(timestamp0[hit_i], energy0[hit_i]); rf_found = true; continue; }
          if (!rf_found) continue;
        }
        auto const & time_i = (choice == 0) ? rf.relTime_ns(timestamp0[hit_i]) : time1[hit_i]/1000.f;
        if (isGe(label_i)) 
        {
          auto const & nrj_i = (choice == 0) ? energy0[hit_i] : energy1[hit_i];
          E_dT->Fill(time_i, nrj_i);
          for (int hit_j = hit_i+1; hit_j < mult; hit_j++)
          {
            auto const & label_j = (choice == 0) ? label0[hit_j] : label1[hit_j];
            auto const & time_j = (choice == 0) ? rf.relTime_ns(timestamp0[hit_j]) : time1[hit_j]/1000.f;
            if (isGe(label_j)) 
            {
              auto const & nrj_j = (choice == 0) ? energy0[hit_j] : energy1[hit_j];
              gg->Fill(nrj_i, nrj_j);
              gg->Fill(nrj_j, nrj_i);
              if (-10 < time_i && time_i < 10 && -10 < time_j && time_j < 10) 
              {
                pp->Fill(nrj_i, nrj_j);
                pp->Fill(nrj_j, nrj_i);
              }
              if (60 < time_i && time_i < 150 && 60 < time_j && time_j < 150) 
              {
                dd->Fill(nrj_i, nrj_j);
                dd->Fill(nrj_j, nrj_i);
              }
            }
          }
        }
      }// End hits loop
    }// End events loop
    TFile* file = new TFile(filename,"recreate");
    file->cd();
    gg  -> Write();
    pp  -> Write();
    dd  -> Write();
    E_dT-> Write();
  }
  
  // else if (choice == 2)
  // {
  //   chain1->GetEntry(0);
  //   auto const first_timestamp0 = stamp1;
  //   int evt_i_0 = 1;
  //   int evt_i_1 = 1;
  //   for (; evt_i_0 < chain->GetEntries(); evt_i_0++)
  //   { // Find the beginning of both files :
  //     chain0->GetEntry(evt_i_0);
  //     if (timestamp[0] > first_timestamp0) break;
  //   }
  //   while (evt_i_0 < chain0->GetEntries() && evt_i_1 < chain1->GetEntries())
  //   {
  //     chain0->
  //   }
    
  // }

}