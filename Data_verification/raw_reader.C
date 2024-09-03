

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FasterReader.hpp"
#include "../lib/Classes/Alignator.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Modules/Timeshifts.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Timer.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTFasterReader.hpp"
#include "../lib/MTObjects/MTTHist.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

Time time_window = 50_ns;

int constexpr nb_threads = 10;
int constexpr nb_files = -1;


// All the gates are inclusive
void raw_reader()
{
  SimpleCluster::setDistanceMax(2.1);

  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");
  Calibration calib("../136/136_2024_Co.calib");
  Timeshifts ts("../136/Timeshifts/60Co_after.dT");

  int nb_gate = 0;
  int nb_gated = 0;
  int nb_missed = 0;

  MTObject::Initialise(nb_threads);

  TRandom* random = new TRandom(time(0));

  Vector_MTTHist<TH2F> bidims; 
  bidims.resize(ParisArrays::labels.size());
  for (auto const & label : ParisArrays::labels)
  {
    auto const & index = Paris::index[label];
    auto const & label_str = std::to_string(label);
    bidims[index].reset(("qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong").c_str(), 1000,0,200000, 1000,0,200000);
  }

  // Event event(tree);
  CloversV2 clovers;
  SimpleParis paris(&calibPhoswitches);
  std::vector<double> rej_Ge;

  MTFasterReader reader(Path(Path::home() + "nuball2/N-SI-136-sources/Na22_center.fast/"), nb_files);
  reader.setTimeshifts(ts.data());
  reader.readAligned([&](Hit & hit, Alignator & tree)
  {
    auto const & thread_i = MTObject::getThreadIndex();
    while(tree.Read())
    {
      if (Paris::is[hit.label]) 
      {
        bidims[Paris::index[hit.label]].Fill(hit.qdc2, hit.adc);
      }
    }
  });

  // Efficiency
  auto outfile = TFile::Open("22Na_bidim_paris.root", "recreate");
  outfile->cd();

  for (auto & histo : bidims) histo.Write();

  outfile->Close();

  print("22Na_bidim_paris.root written");
  print(timer());
}

int main()
{
  raw_reader();
  return 1;
}

// g++ -g -o exec raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -lSpectrum -std=c++17