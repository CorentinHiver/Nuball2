#ifndef DSSD_HPP
#define DSSD_HPP

#include "../Classes/Event.hpp"
#include "ExcitationEnergy.hpp"

namespace DSSD
{
  static constexpr size_t LUT_size = 1000;
  static constexpr auto is        = Colib::LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 856;});
  static constexpr auto isRing    = Colib::LUT<LUT_size> ([](Label const & label) {return 839 < label && label < 856;});
  static constexpr auto isSector  = Colib::LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 840;});
  static constexpr auto isSector1 = Colib::LUT<LUT_size> ([](Label const & label) {return 799 < label && label < 820;});
  static constexpr auto isSector2 = Colib::LUT<LUT_size> ([](Label const & label) {return 819 < label && label < 840;});
  
  static constexpr double z  = 3.25 ; // cm : Distance DSSD-target
  static constexpr double Ri = 1.58; // cm : Inner radius 
  static constexpr double Re = 4.05; // cm : Outer radius
  static constexpr std::size_t nb_rings = 16;
  static constexpr std::size_t nb_sectors = 32;
  static constexpr std::size_t nb_strips = nb_rings+nb_sectors;

  static constexpr auto index_ring = Colib::LUT<LUT_size> ([]  (Label const & label) 
  {
    return (DSSD::isRing[label]) ? label-840 : -1;
  });

  static constexpr auto index_sector = Colib::LUT<LUT_size> ([]  (Label const & label) 
  {
         if (DSSD::isSector1[label]) return label-800;
    else if (DSSD::isSector2[label]) return label-805;
    else return -1;
  });

  static constexpr auto index = Colib::LUT<LUT_size> ([]  (Label const & label) 
  {
         if (DSSD::isSector[label]) return int(index_sector[label]);
    else if (DSSD::isRing  [label]) return int(index_ring[label]+nb_sectors);
    else return int{-1};
  });


  static constexpr double angular_size_sector = 2*3.14159/nb_sectors;  
  static constexpr double size_ring = (Re-Ri)/nb_rings; // cm : 
  static constexpr inline double ring_radius(Index const & _index) {return Ri + size_ring*(nb_rings-_index) + size_ring/2.;}
  static constexpr inline double angle_ring  (Index const & _index) {return atan( ring_radius(_index) / z);}
  static constexpr inline double angle_sector(Index const & _index) 
  {
    Index physical_index = (_index<8) ? _index+23 : _index-8;
    return physical_index*angular_size_sector;
  }
  std::pair<double, double> polar(Index const & ring_id, Index const & sector_id, TRandom * random) 
  {
    auto const & r = ring_radius(ring_id)+random->Uniform(-size_ring/2, size_ring/2);
    auto const & phi = angle_sector(sector_id)+random->Uniform(-angular_size_sector/2, angular_size_sector/2);
    return {r*cos(phi), r*sin(phi)};
  }

  static constexpr Time ring_coinc_tw = 60_ns;
};

#endif //DSSD_HPP