#ifndef DSSD_H
#define DSSD_H

#include <libCo.hpp>
#include <libRoot.hpp>

#include <Event.hpp>
#include <Timewalk.hpp>

class DSSD
{
public:
  // static std::vector<uchar> m_labels;
  static inline bool isRing  (Label const & l) { return ( (l>800) && (l<840) );}
  static inline bool isSector(Label const & l) { return ( (l>839) && (l<856) );}
  DSSD(){}
  void Initialize(Label const & label_nb)
  {
    // m_labels.resize(label_nb);
    // for (std::size_t l = 0; l<label_nb; l++)
    // {
    //   m_labels = l-800;
    // }
  }
  void Reset();

  // static void SetTimewalk(std::string const & fit_filename) {m_timewalk.loadFile(fit_filename);}
  // static std::array<Timewalk> m_timewalk;

  void Fill(Event const & evt, int const & index);

  StaticVector<float> m_Sectors = StaticVector<float>(32);
  StaticVector<float> m_Rings = StaticVector<float>(16);

  std::array<float, 32> m_Sectors_time;
  std::array<float, 32> m_Sectors_nrj;
  std::array<float, 16> m_Rings_time;
  std::array<float, 16> m_Rings_nrj;

  std::array<float, 512> m_cells_nrj;
  std::array<float, 512> m_cells_time;


};

// Initialize static members :
// std::array<Timewalk> DSSD::m_timewalk;

void DSSD::Reset()
{
  for (auto const & sector : m_Sectors)
  {
    m_Sectors_time[sector] = 0.;
    m_Sectors_nrj[sector] = 0.;
  }
  m_Sectors.resize();
  for (auto const & ring : m_Rings)
  {
    m_Rings_time[ring] = 0.;
    m_Rings_nrj[ring] = 0.;
  }
  m_Rings.resize();
}

void DSSD::Fill(Event const & event, int const & i)
{
  // auto const & label = event.labels[i];
  //
  // if (isDSSD[label])
  // {
  //   auto const & nrj = event.nrjs[i];
  //   auto const & time = event.times[i];
  //   auto const & isring = isRing(label);
  //   // auto const & face_label = DSSD_Label[isring][label]
  //   if (isring)
  //   {
  //     auto const & ring = face_label;
  //     m_Rings.push_back(ring);
  //     m_Rings_nrj[ring] = nrj;
  //     m_Rings_time[ring] = time;
  //   }
  //   else
  //   {
  //     auto const & sector = face_label;
  //     m_Sectors.push_back(sector);
  //     m_Sectors_nrj[sector] = nrj;
  //     m_Sectors_time[sector] = time;
  //   }
  // }
}

// std::vector<uchar> DSSD_Label::m_labels;

#endif //DSSD_H
