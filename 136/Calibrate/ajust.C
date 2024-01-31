#include "../../lib/Analyse/AnalysedSpectra.hpp"
#include "../../lib/Analyse/SpectraCo.hpp"

/**
 * @brief second step : reads the 136_ajustation_checked.calib parameters 
 * and apply it on the initial calibration to get the 136_ajusted.calib
 * file, which ought to be the final calibration.
 * 
 */
void ajust()
{
  detectors.load("index_129.list");

  Calibration calib_init("../conversion_200ns/136_2024.calib");
  Calibration calib_adjustation("136_ajustation.calib");
  Calibration calib_adjusted; 
  calib_adjusted.resize(calib_init.size());

  for (int label = 0; label<calib_init.size(); label++)
  {
    if (calib_init.order(label)<0) continue;

    // If there is no correction : 
    if (calib_adjustation.order(label)<0) calib_adjusted.set(label, calib_init.intercept(label) , calib_init.slope(label));
    else
    {
      // Else :
      auto const & new_intercept = calib_adjustation.slope(label)*calib_init.intercept(label) 
                                + calib_adjustation.intercept(label);
      auto const & new_slope     = calib_adjustation.slope(label)*calib_init.slope(label);

      print(calib_adjustation.slope(label), calib_init.slope(label), new_slope);
      
      calib_adjusted.set(label, new_intercept, new_slope);
    }
  }
  calib_adjusted.write("136_ajusted");
}