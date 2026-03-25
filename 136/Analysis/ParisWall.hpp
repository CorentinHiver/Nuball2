#pragma once

#include "../../lib/Analysis/Paris.hpp"

namespace NSI136
{
  static constexpr std::array<double, 8> ParisR1_x =
  {
    -1,  0,  1,
      1,
      1,  0, -1,
    -1
  };

  static constexpr std::array<double, 8> ParisR1_y =
  {
      1,  1,  1,
      0,
    -1, -1, -1,
      0
  };

  static constexpr std::array<double, 16> ParisR2_x =
  {
        -1,  0,  1,  2,
      2,  2,  2,
      2,  1,  0, -1, -2,
    -2, -2, -2,
    -2
  };

  static constexpr std::array<double, 16> ParisR2_y =
  {
          2,  2,  2,  2,
      1,  0, -1,
    -2, -2, -2, -2, -2,
    -1,  0,  1,
      2
  };

  static constexpr std::array<double, 12> ParisR3_x =
  {
    -1,  0,  1,
      3,  3,  3,
      1,  0, -1,
    -3, -3, -3,
  };

  static constexpr std::array<double, 12> ParisR3_y =
  {
      3,  3,  3,
      1,  0, -1,
    -3, -3, -3,
    -1,  0,  1,
  };

  constexpr auto ParisR1size = ParisR1_x.size();
  constexpr auto ParisR2size = ParisR2_x.size();
  constexpr auto ParisR3size = ParisR3_x.size();

  class ParisWall : public Paris::Cluster<ParisR1size + ParisR2size + ParisR3size>
  {
    static inline std::array<Colib::Point, cluster_size> positions;
  public:
    // static constexpr auto size() {return cluster_size;}
    template<class... ARGS> ParisWall(ARGS... args) : 
      Paris::Cluster<36>(std::forward<ARGS>(args)...)
    {
      using namespace Colib;
      auto const & i_min_R2 = ParisR1_x.size();
      auto const & i_min_R3 = ParisR1_x.size()+ParisR2_x.size();
      // Filling the position lookup table :
      for (size_t i = 0; i<=cluster_size; i++)
      {
             if (i<i_min_R2) positions[i] = Point{ParisR1_x[i         ], ParisR1_y[i         ]}; // Ring 1
        else if (i<i_min_R3) positions[i] = Point{ParisR2_x[i-i_min_R2], ParisR2_y[i-i_min_R2]}; // Ring 2
        else                 positions[i] = Point{ParisR3_x[i-i_min_R3], ParisR3_y[i-i_min_R3]}; // Ring 3
      }

      // Filling the distance lookup table :
      for (size_t i = 0; i<cluster_size; i++)
      {
        for (size_t j = i+1; j<cluster_size; j++)
        {
          auto const distanceij = distance(positions[i], positions[j]);
          distances[i][j] = distanceij;
          distances[j][i] = distanceij;
        }
      }
    }
  };

  class ParisWalls
  {
  public:
    ParisWall back ;
    ParisWall front;
    std::vector<Index> phoswitches;
    std::vector<Index> modules_id;
    int module_mult{};
    NRJ calorimetry{};
    int const & moduleMult() const {return module_mult;}

    Phoswitch& fill(Event const & event, int const & hit_i)
    {
      auto const & id = ParisClusterIndex[hit_i];
      phoswitches.push_back(id);
           if (isFront[hit_i]) return front.fill(event, hit_i, id);
      else if (isBack [hit_i]) return front.fill(event, hit_i, id);
      else return front.emptyPhoswitch;
    }

    void analyze()
    {
      back .addback();
      front.addback();

      modules_id.reserve(front.modules_id.size() + back.modules_id.size());
      modules_id.insert(modules_id.end(), back.modules_id.begin(), back.modules_id.end());
      modules_id.insert(modules_id.end(), front.modules_id.begin(), front.modules_id.end());

      module_mult = front.module_mult + back.module_mult;
      calorimetry = front.calorimetry + back.calorimetry;
    }

    void clear()
    {
      back       .clear();
      front      .clear();
      phoswitches.clear();
      modules_id .clear();
      module_mult = {};
      calorimetry = {};
    }
  };
}