

#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FasterReader.hpp"
#include "../lib/Classes/Alignator.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Timeshifts.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/Classes/CoincBuilder.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Timer.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/MTObjects/MTFasterReader.hpp"
#include "../lib/MTObjects/MultiHist.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

Time time_window = 50_ns;

int nb_threads = 2;
int nb_files = -1;
int nb_hits = -1;

// std::string source = "60Co";
// std::string source = "Na22";
std::string source = "152Eu";

std::map<Label, double> calibLaBr = {
{301, 1.00232},
{302, 1.00355},
{303, 0.998705},
{304, 0.998661},
{305, 1.00534},
{306, 1.00412},
{307, 1.00604},
{308, 0.995267},
{309, 1.00071},
{310, 0.999144},
{311, 0.995776},
{312, 0.999043},
{313, 0.999239},
{314, 0.989351},
{315, 1.00009},
{316, 0.995293},
{401, 1.00433},
{402, 0.977221},
{403, 0.980751},
{404, 0.972085},
{405, 0.988895},
{406, 0.967432},
{407, 0.975061},
{408, 0.999042},
{409, 0.989256},
{410, 0.967872},
{411, 0.979239},
{412, 0.959348},
{501, 1.01598},
{502, 1.01273},
{503, 0.997414},
{504, 0.9995},
{505, 1.0099},
{506, 1.01118},
{507, 1.01875},
{508, 1.01079},
{601, 1.09881},
{602, 0.796353},
{603, 0.972897},
{604, 0.822098},
{605, 1.00082},
{606, 0.761883},
{607, 1.01488},
{608, 0.995224},
{609, 0.996967},
{610, 0.981561},
{611, 0.980286},
{612, 0.879495},
{613, 0.990487},
{614, 0.955601},
{615, 0.989463},
{616, 0.749016},
{701, 0.994545},
{702, 1.00487},
{703, 0.997289},
{704, 0.942364},
{705, 0.980697},
{706, 0.992492},
{707, 0.960328},
{708, 0.985715},
{709, 0.970808},
{710, 0.997369},
{711, 0.969776},
{712, 1.00585}
};

void raw_reader()
{
  Timer timer;

  PhoswitchCalib calibPhos("../136/NaI_136_2024.angles");
  Calibration calib("../136/136_2024_Co.calib");

  MTObject::Initialise(nb_threads);
  if (nb_hits>0) FasterReader::setMaxHits(nb_hits);

  // TRandom* random = new TRandom(time(0));

  auto const & label_nb = ParisArrays::labels.size();

  Vector_MultiHist<TH2F> bidims; bidims.resize(label_nb);
  Vector_MultiHist<TH2F> bidims_rot; bidims_rot.resize(label_nb);
  Vector_MultiHist<TH2F> bidims_rot_all; bidims_rot_all.resize(label_nb);

  for (auto const & label : ParisArrays::labels)
  {
    auto const & index = Paris::index[label];
    auto const & label_str = std::to_string(label);
    bidims[index].reset(("qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong").c_str(), 1000,0,200000, 1000,0,200000);
    bidims_rot[index].reset(("rotated_qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong_rotated").c_str(), 1000,0,200000, 1000,0,200000);
    bidims_rot_all[index].reset(("all_rotated_qshort_VS_qlong_"+label_str).c_str(), (label_str+"_qshort_VS_qlong_all_rotated").c_str(), 1000,0,200000, 1000,0,200000);
  }

  MultiHist<TH2F> raw_spectra;
  MultiHist<TH2F> calibrated_spectra;
  MultiHist<TH1F> Ge;
  MultiHist<TH2F> Ge_Ge;
  MultiHist<TH2F> Ge_BGO;
  MultiHist<TH2F> Ge_ModuleParis;
  MultiHist<TH2F> Ge_Phoswitch;
  if (source == "152Eu") 
  {
    raw_spectra.reset("raw_spectra", "raw_spectra;label;Energy [keV]", 1000,0,1000, 200_ki,0,2_Mi);
    calibrated_spectra.reset("calibrated_spectra", "calibrated_spectra;label;Energy [keV]", 1000,0,1000, 2000,0,2000);
    Ge.reset("Ge", "Ge;HPGe [keV]", 2000,0,2000);
    Ge_Ge.reset("Ge_Ge", "Ge_Ge;HPGe [keV];Ge Energy [keV]", 2000,0,2000, 2000,0,2000);
    Ge_BGO.reset("Ge_BGO", "Ge_BGO;HPGe [keV];BGO Energy [keV]", 2000,0,2000, 500,0,5000);
    Ge_ModuleParis.reset("Ge_ModuleParis", "Ge_ModuleParis;HPGe [keV];Paris Energy [keV]", 2000,0,2000, 2000,0,2000);
    Ge_Phoswitch.reset("Ge_Phoswitch", "Ge_Phoswitch;HPGe [keV];Paris Energy [keV]", 2000,0,2000, 2000,0,2000);
  }

  std::string path;
  if (source == "Na22") path = Path::home().string()+"nuball2/N-SI-136/Na22_center.fast/";
  else if (source == "Co60") path = Path::home().string()+"nuball2/N-SI-136/60Co_center_after.fast/";
  else if (source == "152Eu") path = Path::home().string()+"nuball2/N-SI-136/152_Eu_center_after.fast/";

  MTFasterReader reader(path, nb_files);
  Timeshifts ts("../136/136_Co.dT");

  Event event;
  CoincBuilder builder(&event, time_window);

  reader.setTimeshifts(ts);
  reader.readAligned([&](Alignator & tree, Hit & hit)
  {
    CloversV2 clovers;
    SimpleParis paris(&calibPhos);
    while(tree.Read())
    {
      calib.calibrate(hit);
      raw_spectra.Fill(hit.label, hit.adc);
      calibrated_spectra.Fill(hit.label, hit.nrj);

      if (Paris::is[hit.label]) 
      {
        auto const & nrjcal = calibPhos.calibrate(hit.label, hit.adc, hit.qdc2);
        auto const & short_cal = calibPhos.calibrate_short(hit.label, hit.adc, hit.qdc2);
        bidims[Paris::index[hit.label]].Fill(hit.qdc2, hit.adc);
        bidims_rot[Paris::index[hit.label]].Fill(nrjcal, hit.adc);
        bidims_rot_all[Paris::index[hit.label]].Fill(nrjcal, short_cal);
        auto const & nrj = (Paris::pid_LaBr3(hit.adc, hit.qdc2)) ? hit.adc*calibLaBr[hit.label] : calibPhos.calibrate(hit.label, hit.adc, hit.qdc2);
        calibrated_spectra.Fill(hit.label, nrj);
      }

      
      if (builder.build(hit))
      {
        clovers.clear();
        // paris.clear();
        clovers = event;
        // paris = event;
        clovers.analyze();
        // paris.analyze();
        for (size_t hit_i = 0; hit_i<clovers.clean.size(); ++hit_i)
        {
          auto const & clover_i = *(clovers.clean[hit_i]);
          Ge.Fill(clover_i.nrj);
          for (size_t hit_j = hit_i+1; hit_j<clovers.clean.size(); ++hit_j)
          {
            auto const & clover_j = *(clovers.clean[hit_j]);
            Ge_Ge.Fill(clover_i.nrj, clover_j.nrj);
            Ge_Ge.Fill(clover_j.nrj, clover_i.nrj);
          }

          for (auto const & BGO_id : clovers.BGO_id) Ge_BGO.Fill(clover_i.nrj, clovers[BGO_id].nrjBGO);
          // for (auto const & module : paris.modules) Ge_ModuleParis.Fill(clover_i.nrj, module->nrj);
          // for (auto const & phoswitch : paris.phoswitches) Ge_Phoswitch.Fill(clover_i.nrj, phoswitch->nrj);
        }
      }
    }
  });

  // Efficiency
  TString name;
  if (source == "Na22") name = "22Na_bidim_paris.root";
  else if (source == "Co60") name = "60Co_bidim_paris.root";
  else if (source == "152Eu") name = "152Eu_test.root";

  auto outfile = TFile::Open(name, "recreate");
  outfile->cd();

  for (auto & histo : bidims) histo.Write();
  for (auto & histo : bidims_rot) histo.Write();
  for (auto & histo : bidims_rot_all) histo.Write();

  raw_spectra.Write();
  calibrated_spectra.Write();  
  Ge_Ge.Write();
  Ge_BGO.Write();
  Ge_ModuleParis.Write();
  Ge_Phoswitch.Write();

  outfile->Close();

  print(name, "written");
  print(timer());
}

int main(int argc, char** argv)
{
  if (argc>1) nb_files = int(std::stod(argv[1]));
  else if (argc>2) nb_threads = int(std::stod(argv[2]));
  else if (argc>3) nb_hits = int(std::stod(argv[2]));
  raw_reader();
  return 1;
}

// g++ -g -o raw_reader raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o raw_reader raw_reader.C $(root-config --cflags) $(root-config --glibs) $(pkg-config --cflags libfasterac) $(pkg-config --libs libfasterac) -lSpectrum -std=c++17