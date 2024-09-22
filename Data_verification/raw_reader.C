

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FasterReader.hpp"
#include "../lib/Classes/Alignator.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Timeshifts.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Timer.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTFasterReader.hpp"
#include "../lib/MTObjects/MultiHist.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

Time time_window = 50_ns;

int nb_threads = 10;
int nb_files = -1;

std::string source = "60Co";
// std::string source = "Na22";


void raw_reader()
{
  SimpleCluster::setDistanceMax(2.1);

  Timer timer;

  PhoswitchCalib calibPhos("../136/NaI_136_2024.angles");

  int nb_gate = 0;
  int nb_gated = 0;
  int nb_missed = 0;

  MTObject::Initialise(nb_threads);

  TRandom* random = new TRandom(time(0));

  Vector_MTTHist<TH2F> bidims; 
  bidims.resize(ParisArrays::labels.size());
  Vector_MTTHist<TH2F> bidims_rot; 
  Vector_MTTHist<TH2F> bidims_rot_all; 
  bidims_rot.resize(ParisArrays::labels.size());
  bidims_rot_all.resize(ParisArrays::labels.size());
  for (auto const & label : ParisArrays::labels)
  {
    auto const & index = Paris::index[label];
    auto const & label_str = std::to_string(label);
    bidims[index].reset(("qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong").c_str(), 1000,0,200000, 1000,0,200000);
    bidims_rot[index].reset(("rotated_qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong_rotated").c_str(), 1000,0,200000, 1000,0,200000);
    bidims_rot_all[index].reset(("all_rotated_qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong_all_rotated").c_str(), 1000,0,200000, 1000,0,200000);
  }

  Path path;
  if (source == "Na22") path = Path::home() + "nuball2/N-SI-136/Na22_center.fast/";
  else if (source == "Co60") path = Path::home() + "nuball2/N-SI-136/60Co_center_after.fast/";

  MTFasterReader reader(path, nb_files);
  Timeshifts ts("../136/136_Co.dT");
  reader.setTimeshifts(ts);
  reader.readAligned([&](Alignator & tree, Hit & hit)
  {
    auto const & thread_i = MTObject::getThreadIndex();
    while(tree.Read())
    {
      if (source == "Na22")
      {
        if (Paris::is[hit.label]) 
        {
          auto const & nrjcal = calibPhos.calibrate(hit.label, hit.adc, hit.qdc2);
          auto const & short_cal = calibPhos.calibrate_short(hit.label, hit.adc, hit.qdc2);
          bidims[Paris::index[hit.label]].Fill(hit.qdc2, hit.adc);
          bidims_rot[Paris::index[hit.label]].Fill(nrjcal, hit.adc);
          bidims_rot_all[Paris::index[hit.label]].Fill(nrjcal, short_cal);
        }
      }
    }
  });

  // Efficiency
  TFile* outfile = nullptr;
  if (source == "Na22") outfile = TFile::Open("22Na_bidim_paris.root", "recreate");
  else if (source == "Co60") outfile = TFile::Open("60Co_bidim_paris.root", "recreate");
  outfile->cd();

  for (auto & histo : bidims) histo.Write();
  for (auto & histo : bidims_rot) histo.Write();
  for (auto & histo : bidims_rot_all) histo.Write();

  outfile->Close();

  if (source == "Na22") print("22Na_bidim_paris.root written");
  else if (source == "Co60") print("22Na_bidim_paris.root written");
  print(timer());
}

int main(int argc, char** argv)
{
  if (argc==2) nb_files = int(std::stod(argv[1]));
  else if (argc==3) 
  {
    nb_files = int(std::stod(argv[1]));
    nb_threads = int(std::stod(argv[2]));
  }
  raw_reader();
  return 1;
}

// g++ -g -o raw_reader raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o raw_reader raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -lSpectrum -std=c++17