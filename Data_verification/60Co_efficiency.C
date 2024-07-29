

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

long max_cursor = -1;

void 60Co_efficiency()
{
  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/60Co_center_after_calib/");
  // FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/end_runs_2/");
  MTList MTfiles(files.get());
  MTObject::Initialise(7);
  // MTObject::Initialise(1);
  MTObject::adjustThreadsNumber(files.size());
  CoefficientCorrection calibGe("../136/GainDriftCoefficients.dat");
  if (!calibGe) return;
  MTObject::parallelise_function([&]()
  {
    std::string filename;
    auto const & thread_i_str = std::to_string(MTObject::getThreadIndex());
    while(MTfiles>>filename)
    {
      std::string file_shortname = rmPathAndExt(filename);
      Nuball2Tree tree(filename);
      Event event(tree);
      CloversV2 clovers;
      SimpleParis paris(&calibPhoswitches);
    }
  });
  // print("hadd -d . -j 10 -f data/endruns.root data/end_runs/endruns136*");
  // system("hadd -d . -j 10 -f data/endruns.root data/end_runs/endruns136*");
  print("hadd -d . -j 10 -f data/60Co.root data/60Co_center_after/60Co136*");
  system("hadd -d . -j 10 -f data/60Co.root data/60Co_center_after/60Co136*");
  print(timer());
}
