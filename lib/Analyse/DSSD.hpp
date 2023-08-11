#ifndef DSSD_H
#define DSSD_H

#include <libCo.hpp>
#include <libRoot.hpp>

#include <Event.hpp>
#include <Timewalk.hpp>
#include <Detectors.hpp>


// Dimensions en mm:
double distance = 20; // Distance of the DSSD to the target
double innerRadius = 15;
double outerRadius = 41; 
int const nb_rings = 16; // Number of ring channels
int const nb_sectors = 16; // Number of ring channels
double ring_thickness = (outerRadius-innerRadius)/nb_rings; // Thickness of a 
double ring_deg_thick = TMath::ATan(ring_thickness/distance);

class SStrip
{
public:
  SStrip(ushort const & label) {m_label = label;}
  SStrip(ushort const & label, double const & angle) {m_label = label; m_angle = angle;}

  void Reset()
  {
    time = 0.0;
    nrj  = 0.0;
  }

  double time = 0.0;
  double nrj  = 0.0;

  void setAngle(double const & angle) {m_angle = angle;}
  auto const & angle() const {return m_angle;}
  auto const & angle() {return m_angle;}
  auto const & label() const {return m_label;}
  auto const & label() {return m_label;}

protected:
  double m_angle = 0;
  ushort m_label = 0u;
};

class DSSD
{
public:

// _____________________________________
//            LOOKUP TABLES
// _____________________________________
  static std::array<bool, 1000>  isS1    ;
  static std::array<bool, 1000>  isS2    ;
  static std::array<bool, 1000>  isSector;
  static std::array<bool, 1000>  isRing  ;
  static std::array<uchar, 1000> indexes ;

  void Initialize()
  {
    if (!m_initialized)
    {
      for (Label label = 0; label<1000u; label++)
      {
        isS1    [label] = ((label>799) && (label<816));
        isS2    [label] = ((label>819) && (label<836));
        isSector[label] = ((label>799) && (label<836));
        isRing  [label] = ((label>839) && (label<856));

             if (isS1  [label]) indexes[label] = label-800;
        else if (isS2  [label]) indexes[label] = label-804;
        else if (isRing[label]) indexes[label] = label-840;

      }
      m_initialized = true;
    }
  }

// _____________________________________
//                 DSSD
// _____________________________________
  DSSD()
  {
    Initialize(); 
    m_Sectors.reserve(nb_sectors); 
    for (ushort i = 0; i<32u; i++) m_Sectors.emplace_back(i, (i+0.5)*2*3.141596/nb_sectors);
    m_Rings.reserve(nb_rings);
    for (ushort i = 0; i<16u; i++) m_Rings.emplace_back(i, TMath::ATan( (innerRadius+( (i+0.5) * ring_thickness )) / distance ));
  }
  void Reset();

  // static void SetTimewalk(std::string const & fit_filename) {m_timewalk.loadFile(fit_filename);}
  // static std::array<Timewalk> m_timewalk;

  void SetEvent(Event const & event);
  void Fill(Event const & evt, int const & index);

  size_t const & sectors() const {return m_Sector_Hits.size();}
  size_t const & rings() const {return m_Ring_Hits.size();}
  bool oneParticle() const {return (sectors() == 1 && rings() == 1);}

  auto const & energy() const {return m_Sectors[m_Sector_Hits[0]].nrj ;}
  auto const & time()   const {return m_Sectors[m_Sector_Hits[0]].time;}

  /// @brief Don't return any angle but really the label of the DSSD
  auto const & angle() const {return m_Rings[m_Ring_Hits[0]].angle();} 

  StaticVector<uchar> m_Sector_Hits = StaticVector<uchar>(nb_sectors);
  StaticVector<uchar> m_Ring_Hits = StaticVector<uchar>(nb_rings);

  std::vector<SStrip> m_Sectors;
  std::vector<SStrip> m_Rings  ;

  std::size_t SectorMult = 0u;
  std::size_t RingMult   = 0u;

private:
  bool m_initialized;
};

// Initialize static members :
std::array<bool, 1000>  DSSD::isS1     = {0};
std::array<bool, 1000>  DSSD::isS2     = {0};
std::array<bool, 1000>  DSSD::isSector = {0};
std::array<bool, 1000>  DSSD::isRing   = {0};
std::array<uchar, 1000> DSSD::indexes  = {255};

// std::array<Timewalk> DSSD::m_timewalk;

void DSSD::Reset()
{
  for (auto const & sector : m_Sector_Hits)
  {
    m_Sectors[sector].Reset();
  }
  m_Sector_Hits.resize();

  for (auto const & ring : m_Ring_Hits)
  {
    m_Rings[ring].Reset();
  }
  m_Ring_Hits.resize();

  SectorMult = 0u;
  RingMult   = 0u;
}

void DSSD::SetEvent(Event const & event)
{
  this -> Reset();
  for(int i = 0; i<event.mult; i++) this -> Fill(event, i);
}

void DSSD::Fill(Event const & event, int const & i)
{
  auto const & label = event.labels[i];
  
  if (isDSSD[label])
  {
    auto const & nrj = event.nrjcals[i];
    auto const & time = event.time2s[i];
    auto const & isring = isRing[label];
    auto const & index = indexes[label];
    if (isring)
    {
      RingMult++;
      m_Ring_Hits.push_back(index);
      m_Rings[index].nrj = nrj;
      m_Rings[index].time = time;
    }
    else
    {
      SectorMult++;
      m_Sector_Hits.push_back(index);
      m_Sectors[index].nrj = nrj;
      m_Sectors[index].time = time;
    }
  }
}

#endif //DSSD_H
