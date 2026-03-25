#include "../../lib/CoMT.hpp"
#include "../../lib/Classes/MThisto.hpp"
#include "../../lib/RootReader/RootReader.hpp"

#include "../../lib/Analysis/Clovers.hpp"
#include "EventAnalyser.hpp"
#include "ParisWall.hpp"

using namespace std;
using namespace Colib;
using namespace NSI136;

int main(int argc, char**argv)
{
  vector<string> files(findFilesWildcard(nicerPath("~/nuball2/N-SI-136_root/run_C2/")+"run*_rf.root"));
  if (argc>1) MT::Initialise(std::min(size_cast(stoi(argv[1])), files.size()));

  MT::Histo<TH1F> p_raw("p_raw", "p_raw", 15000,0,15000);
  MT::Histo<TH1F> d_raw("d_raw", "d_raw", 15000,0,15000);
  MT::Histo<TH1F> p("p", "p", 15000,0,15000);
  MT::Histo<TH1F> d("d", "d", 15000,0,15000);
  MT::Histo<TH2F> pp("pp", "pp", 4096,0,4096, 4096,0,4096);
  MT::Histo<TH2F> pp_label("pp_label", "pp_label", 30,0,30, 30,0,30);
  MT::Histo<TH2F> dd("dd", "dd", 4096,0,4096, 4096,0,4096);
  MT::Histo<TH2F> dd_label("dd_label", "dd_label", 30,0,30, 30,0,30);
  
  MT::Histo<TH2F> dd_PM1_5__PC0_3___DM1_5__DC0_3("dd_PM1_5__PC0_3___DM1_5__DC0_3", "dd 0<PM<6 and 0<PC<3 MeV and 0<DM<3 and 0<DC<3 MeV ", 4096,0,4096, 4096,0,4096);

  std::array<Gate_t<Time>, NSI136::Detector::nbTypes> pGate{};
  pGate[Detector::Ge   ] = {-15_ns,  15_ns};
  pGate[Detector::BGO  ] = {-15_ns,  15_ns};
  pGate[Detector::Paris] = {-10_ns,  10_ns};
  pGate[Detector::DSSD ] = {-20_ns,  50_ns};

  std::array<Gate_t<Time>, NSI136::Detector::nbTypes> dGate{};
  dGate[Detector::Ge   ] = { 40_ns, 160_ns};
  dGate[Detector::BGO  ] = { 40_ns, 160_ns};
  dGate[Detector::Paris] = { 40_ns, 160_ns};

  Gate_t<int> M1_5{1, 5};
  Gate_t<int> M1_3{1, 3};
  Gate C0_3{0_MeV, 3_MeV};
  
  auto const dispatched_runs = MT::distribute(files);

  MT::parallelise_function([&](){ for (auto const & run : dispatched_runs[MT::getThreadIndex()])
  {
    RootReader reader(run);
    Clovers    pclovers;
    Clovers    dclovers;
    ParisWalls pparis;
    ParisWalls dparis;

    while(reader.readNext())
    {
      pclovers.clear();
      dclovers.clear();
      pparis  .clear();
      dparis  .clear();

      EventAnalyser eventAnalyser(reader.getEvent());
      auto const & event = reader.getEvent();
      for (auto hit_i : eventAnalyser.promptHits)
      {
        auto const & label = event.labels[hit_i];
        auto const & nrj = event.nrjs[hit_i];
        if (isGe[label]) p_raw -> Fill(nrj);
        pclovers.fill(event, hit_i);
        pparis  .fill(event, hit_i);
      }
      for (auto hit_i : eventAnalyser.delayedHits)
      {
        auto const & label = event.labels[hit_i];
        auto const & nrj = event.nrjs[hit_i];
        if (isGe[label]) d_raw -> Fill(nrj);
        dclovers.fill(event, hit_i);
        dparis  .fill(event, hit_i);
      }

      pclovers.analyze();
      dclovers.analyze();
      pparis  .analyze();
      dparis  .analyze();

      size_t PM = pclovers.moduleMult() + pparis.moduleMult();
      size_t DM = dclovers.moduleMult() + dparis.moduleMult();
      size_t PC = pclovers.calorimetryTotal + pparis.calorimetry;
      size_t DC = dclovers.calorimetryTotal + dparis.calorimetry;


      for (size_t clover_i = 0; clover_i<pclovers.Ge.size(); ++clover_i)
      {
        auto const & index1 = pclovers.Ge[clover_i];
        auto const & clover1 = pclovers[index1];
        p->Fill(clover1.nrj);
        for (size_t clover_j = clover_i+1; clover_j<pclovers.Ge.size(); ++clover_j)
        {
          auto const & index2 = pclovers.Ge[clover_j];
          auto const & clover2 = pclovers[index2];
          pp->Fill(clover1.nrj, clover2.nrj);
          pp->Fill(clover2.nrj, clover1.nrj);
          pp_label->Fill(index1, index2);
          pp_label->Fill(index2, index1);
        }
      }
      for (size_t clover_i = 0; clover_i<dclovers.Ge.size(); ++clover_i)
      {
        auto const & index1 = dclovers.Ge[clover_i];
        auto const & clover1 = dclovers[index1];
        d->Fill(clover1.nrj);
        for (size_t clover_j = clover_i+1; clover_j<dclovers.Ge.size(); ++clover_j)
        {
          auto const & index2 = dclovers.Ge[clover_j];
          auto const & clover2 = dclovers[index2];
          dd->Fill(clover1.nrj, clover2.nrj);
          dd->Fill(clover2.nrj, clover1.nrj);
          dd_label->Fill(index1, index2);
          dd_label->Fill(index2, index1);

          if (M1_5.isIn(PM) && M1_3.isIn(DM) && C0_3.isIn(PC) && C0_3.isIn(DC))
          {
            dd_PM1_5__PC0_3___DM1_5__DC0_3->Fill(clover1.nrj, clover2.nrj);
            dd_PM1_5__PC0_3___DM1_5__DC0_3->Fill(clover2.nrj, clover1.nrj);
          }
        }
      }
    }
  }});

  auto outFile = TFile::Open("output.root", "recreate");

  p_raw->Write();
  d_raw->Write();
  p->Write();
  d->Write();
  pp->Write();
  pp_label->Write();
  dd->Write();
  dd_label->Write();
  dd_PM1_5__PC0_3___DM1_5__DC0_3->Write();

  outFile->Close();
  print("output.root written");
}