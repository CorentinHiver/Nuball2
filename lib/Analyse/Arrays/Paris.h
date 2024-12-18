#ifndef PARIS_ARRAYS
#define PARIS_ARRAYS

namespace ParisArrays
{
  static constexpr std::array<Label, 72> labels =
  {
    201, 202, 203, 204, 205, 206, 207, 208,
    301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316,
    401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412,
    501, 502, 503, 504, 505, 506, 507, 508,
    601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616,
    701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712
  };

  static constexpr std::array<double, 8> Paris_R1_x =
  {
    -1,  0,  1,
     1,
     1,  0, -1,
    -1
  };

  static constexpr std::array<double, 8> Paris_R1_y =
  {
     1,  1,  1,
     0,
    -1, -1, -1,
     0
  };

  static constexpr std::array<double, 16> Paris_R2_x =
  {
        -1,  0,  1,  2,
     2,  2,  2,
     2,  1,  0, -1, -2,
    -2, -2, -2,
    -2
  };

  static constexpr std::array<double, 16> Paris_R2_y =
  {
         2,  2,  2,  2,
     1,  0, -1,
    -2, -2, -2, -2, -2,
    -1,  0,  1,
     2
  };

  static constexpr std::array<double, 12> Paris_R3_x =
  {
    -1,  0,  1,
     3,  3,  3,
     1,  0, -1,
    -3, -3, -3,
  };

  static constexpr std::array<double, 12> Paris_R3_y =
  {
     3,  3,  3,
     1,  0, -1,
    -3, -3, -3,
    -1,  0,  1,
  };

}
#endif //PARIS_ARRAYS
