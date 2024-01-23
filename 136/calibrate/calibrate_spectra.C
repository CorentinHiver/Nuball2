#include "../../lib/Modules/Calibrator.hpp"

void calibrate_spectra
(
  std::string const & histo_file = "~/faster_data/N-SI-136-U_histo/total/fused_histo.root", 
  std::string const & calibration_file = "../conversion_200ns/136_2024.calib",
  std::string const & out_filename = "test"
)

{
  detectors.load("index_129.list");
  Calibrator calib_test;
  calib_test.loadCalibration(calibration_file);
  calib_test.verbose(true);
  calib_test.loadRootHisto(histo_file);
  calib_test.verify(out_filename);

  print("Now verification time : you have to look at the calibrated spectra in test_calib.root "
  "in order to correct for any huge error. To do so, copy 136.calib to 136_checked.calib and make"
  "the appropriate changes there. Now run recal.C to adjust the coefficients using some run peaks.");
}