#ifndef DSSD_H
#define DSSD_H

#include "../NearLine.hpp"

class DSSD
{
public:
  DSSD(NearLine* n = nullptr){if(n)set(n);}

  void set(NearLine* n)
  {
    for (size_t i = 0; i<n->m_labelToName.size(); i++)
    {
      if (i > 800 && i<856) m_is_vector.push_back(true);
      else  m_is_vector.push_back(false);
    }
  }
  
  int m_bins_raw = 10000;
  int m_min_raw = 0;
  int m_max_raw = 10000;
  int m_bins_calib = 6000;
  int m_min_calib = 0;
  int m_max_calib = 3000;
  int m_bins_bidim = 3000;
  int m_min_bidim = 0;
  int m_max_bidim = 3000;

  bool operator() (Hit const & hit) {return m_is_vector[hit.label];}

  int get_bins_raw() {return m_bins_raw;}
  int get_min_raw() {return m_min_raw;}
  int get_max_raw() {return m_max_raw;}
  int get_bins_calib() {return m_bins_calib;}
  int get_min_calib() {return m_min_calib;}
  int get_max_calib() {return m_max_calib;}
  int get_bins_bidim() {return m_bins_bidim ;}
  int get_min_bidim() {return m_min_bidim;}
  int get_max_bidim() {return m_max_bidim;}

private:

  std::vector<UShort_t> m_labels =
  {
    800,801,802,803,804,805,806,807,808,809,810,811,812,813,814,815,
    820,821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,
    840,841,842,843,844,845,846,847,848,849,850,851,852,853,854,855
  };

  std::vector<Bool_t> m_is_vector;
};


#endif //DSSD_H
