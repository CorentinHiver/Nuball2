#ifndef DSSD_HPP
#define DSSD_HPP

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
int const nb_sectors = 32; // Number of ring channels
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
  float nrj  = 0.0;

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

  void static InitializeArrays()
  {
    if (!s_initialized)
    {
    #ifdef MTOBJECT_HPP
      lock_mutex(MTObject::mutex);
    #endif //MTOBJECT_HPP
      print("Initialising DSSD arrays");
      for (Label label = 0; label<Label_cast(1000); label++)
      {
        isS1    [label] = ((label>Label_cast(799)) && (label<Label_cast(816)));
        isS2    [label] = ((label>Label_cast(819)) && (label<Label_cast(836)));
        isSector[label] = ((label>Label_cast(799)) && (label<Label_cast(836)));
        isRing  [label] = ((label>Label_cast(839)) && (label<Label_cast(856)));

             if (isS1  [label]) indexes[label] = uchar_cast(label-Label_cast(800));
        else if (isS2  [label]) indexes[label] = uchar_cast(label-Label_cast(804));
        else if (isRing[label]) indexes[label] = uchar_cast(label-Label_cast(840));

      }
      s_initialized = true;
    }
  }

// _____________________________________
//                 DSSD
// _____________________________________
  DSSD()
  {
    InitializeArrays(); 
    Sectors.reserve(nb_sectors); 
    for (ushort i = 0; i<nb_sectors; i++) Sectors.emplace_back(i, (i+0.5)*2*3.141596/nb_sectors);
    Rings.reserve(nb_rings);
    for (ushort i = 0; i<nb_rings; i++) Rings.emplace_back(i, TMath::ATan( (innerRadius+( (i+0.5) * ring_thickness )) / distance ));
  }
  void Reset();

  // static void SetTimewalk(std::string const & fit_filename) {m_timewalk.loadFile(fit_filename);}
  // static std::array<Timewalk> m_timewalk;

  void SetEvent(Event const & event);
  void Fill(Event const & evt, int const & index);

  auto sectors() const {return m_Sector_Hits.size();}
  auto rings() const {return m_Ring_Hits.size();}
  bool oneParticle() const {return (sectors() == 1 && rings() == 1);}

  auto const & energy() const {return Sectors[m_Sector_Hits[0]].nrj ;}
  auto const & time()   const {return Sectors[m_Sector_Hits[0]].time;}

  /// @brief Don't return any angle but really the label of the DSSD
  auto const & angle() const {return Rings[m_Ring_Hits[0]].angle();} 

  std::vector<uchar> m_Sector_Hits = std::vector<uchar>(nb_sectors);
  std::vector<uchar> m_Ring_Hits = std::vector<uchar>(nb_rings);

  std::vector<SStrip> Sectors;
  std::vector<SStrip> Rings  ;

  std::size_t SectorMult = 0u;
  std::size_t RingMult   = 0u;

private:
  bool static s_initialized;
};

bool DSSD::s_initialized = false;

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
    Sectors[sector].Reset();
  }
  m_Sector_Hits.clear();

  for (auto const & ring : m_Ring_Hits)
  {
    Rings[ring].Reset();
  }
  m_Ring_Hits.clear();

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
    auto const nrj = event.nrjs[i] + gRandom->Uniform(0,1);
    auto const & time = event.time2s[i];
    auto const & isring = isRing[label];
    auto const & index = indexes[label];
    if (isring)
    {
      RingMult++;
      m_Ring_Hits.push_back(index);
      Rings[index].nrj = nrj;
      Rings[index].time = time;
    }
    else
    {
      SectorMult++;
      m_Sector_Hits.push_back(index);
      Sectors[index].nrj = nrj;
      Sectors[index].time = time;
    }
  }
}

#endif //DSSD_HPP
