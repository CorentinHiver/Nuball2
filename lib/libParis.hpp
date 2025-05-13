#ifndef LIBPARIS_HPP
#define LIBPARIS_HPP

#include "libRoot.hpp"

/**
 * @brief Meant for Paris matrix rotation method
 * 
 * @param bidim 
 * @param angle 
 * @param coeff 
 * @return TH2F* 
 */
TH2F* rotateAndCalibrate(TH2F* bidim, double angle, double const & coeff)
{
  auto const & name = bidim->GetName();
  auto const & title = bidim->GetTitle();
  
  auto xAxis = bidim->GetXaxis();
  auto const & binsX =  xAxis->GetNbins();
  auto const & minX  =  xAxis->GetXmin();
  auto const & maxX  =  xAxis->GetXmax();

  auto yAxis = bidim->GetYaxis();
  auto const & binsY = yAxis->GetNbins();
  auto const & minY  = yAxis->GetXmin();
  auto const & maxY  = yAxis->GetXmax();

  auto rotated_bidim = new TH2F((name+std::string("_rotated")).c_str(), (title+std::string(" rotated")).c_str(), binsX,minX,maxX, binsY,minY,maxY);
  // return rotated_bidim;
  double _sin = sin(angle);
  double _cos = cos(angle);

  for (int binX = 0; binX<binsX; binX++)
  {
    for (int binY = 0; binY<binsY; binY++)
    {
      auto const & nb_hits   = bidim->GetBinContent(binX, binY);
      auto const & old_long  = bidim->GetYaxis()->GetBinCenter(binY);
      auto const & old_short = bidim->GetXaxis()->GetBinCenter(binX);

      auto const & old_long_range  = bidim->GetYaxis()->GetBinCenter(binY+1)-old_long ;
      auto const & old_short_range = bidim->GetXaxis()->GetBinCenter(binX+1)-old_short;
      for (int hit_i = 0; hit_i<nb_hits; hit_i++)
      {
        auto const & rand_short = old_short + randomCo::uniform(0, old_short_range);
        auto const & rand_long  = old_long  + randomCo::uniform(0, old_long_range);

        // Rotate the NaI+both toward the long gate :
        auto const & new_short = rand_short * _cos - rand_long * _sin; // * (abs(_tan)/_tan);
        auto const & new_long  = coeff * (rand_short * _sin + rand_long * _cos); // * (abs(_tan)/_tan);

        rotated_bidim->Fill(new_short, new_long);
      }
    }
  }
  return rotated_bidim;
}


/**
 * @brief Meant for Paris matrix Q_{short}/Q_{long} VS Qshort or Qlong
 * @param slope: true -> slope; false -> inverted slope
 * @param VSaxisX: true -> slope VS x axis ; false -> slope VS y axis
 */
TH2F* slopeVSaxis(TH2F* bidim, bool slope = true, bool VSaxisX = true)
{
  auto const & name = bidim->GetName();
  auto const & title = bidim->GetTitle();
  
  auto xAxis = bidim->GetXaxis();
  auto const & binsX =  xAxis->GetNbins();
  auto const & minX  =  xAxis->GetXmin();
  auto const & maxX  =  xAxis->GetXmax();

  auto yAxis = bidim->GetYaxis();
  auto const & binsY = yAxis->GetNbins();
  // auto const & minY  = yAxis->GetXmin();
  // auto const & maxY  = yAxis->GetXmax();

  auto slopeBidim = new TH2F((name+std::string("_slope")).c_str(), (title+std::string(" slope")).c_str(), binsX,minX,maxX, 1000, 0, 10);
  // return slopeBidim;

  for (int binX = 0; binX<binsX; binX++)
  {
    for (int binY = 0; binY<binsY; binY++)
    {
      auto const & nb_hits   = bidim->GetBinContent(binX, binY);
      auto const & oldX  = bidim->GetXaxis()->GetBinCenter(binX);
      auto const & oldY = bidim->GetYaxis()->GetBinCenter(binY);

      auto const & oldX_range  = bidim->GetYaxis()->GetBinCenter(binX+1)-oldX ;
      auto const & oldY_range = bidim->GetXaxis()->GetBinCenter(binY+1)-oldY;
      for (int hit_i = 0; hit_i<nb_hits; hit_i++)
      {
        auto const & randX = oldY + randomCo::uniform(0, oldY_range);
        auto const & randY  = oldX  + randomCo::uniform(0, oldX_range);
        auto const & _slope = (slope) ? randX/randY : randY/randX;
        auto const & _xaxis = (VSaxisX) ? randX : randY;
        slopeBidim->Fill(_xaxis, _slope);
      }
    }
  }
  return slopeBidim;
}


#endif //LIBPARIS_HPP
