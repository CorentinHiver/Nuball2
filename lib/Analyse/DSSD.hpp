#ifndef DSSD_HPP
#define DSSD_HPP

#include "../libCo.hpp"
#include "../libRoot.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/Event.hpp"
#include "../Classes/Timewalk.hpp"


/**
 * @brief Silicon Strip. Represents a strip of the DSSD
 * 
 */
class SiStrip
{
public:
  SiStrip(ushort const & label) {m_label = label;}
  SiStrip(ushort const & label, double const & angle) {m_label = label; m_angle = angle;}

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

  void static InitialiseArrays()
  {
    #ifdef COMULTITHREADING
      lock_mutex lock(MTObject::mutex);
    #endif //COMULTITHREADING
    if (!s_Initialised)
    {
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
      s_Initialised = true;
    }
  }

// _____________________________________
//                 DSSD
// _____________________________________
  DSSD()
  {
    InitialiseArrays(); 
    Sectors.reserve(nb_sectors); 
    for (ushort i = 0; i<nb_sectors; i++) Sectors.emplace_back(i, (i+0.5)*2*3.141596/nb_sectors);
    Rings.reserve(nb_rings);
    for (ushort i = 0; i<nb_rings; i++) Rings.emplace_back(i, TMath::ATan( (innerRadius+( (i+0.5) * ring_thickness )) / distance ));
  }
  void Reset();

  // static void SetTimewalk(std::string const & fit_filename) {m_timewalk.loadFile(fit_filename);}
  // static std::array<Timewalk> m_timewalk;

  void operator=(Event const & event) {this->SetEvent();}
  void SetEvent(Event const & event);
  void Fill(Event const & evt, int const & index);

  auto sectors() const {return Sector_Hits.size();}
  auto rings() const {return Ring_Hits.size();}
  bool oneParticle() const {return (sectors() == 1 && rings() == 1);}

  auto const & energy() const {return Sectors[Sector_Hits[0]].nrj ;}
  auto const & time()   const {return Sectors[Sector_Hits[0]].time;}

  /// @brief Don't return any angle but really the label of the ring
  auto const & angle() const {return Rings[Ring_Hits[0]].angle();} 

  auto begin() const {return Sectors.begin();}
  auto end  () const {return Sectors.end  ();}

  std::vector<uchar> Sector_Hits = std::vector<uchar>(nb_sectors);
  std::vector<uchar> Ring_Hits = std::vector<uchar>(nb_rings);

  std::vector<SiStrip> Sectors;
  std::vector<SiStrip> Rings  ;

  std::size_t SectorMult = 0u;
  std::size_t RingMult   = 0u;

  // Dimensions en mm:
  double const distance    = 20; // Distance of the DSSD to the target
  double const innerRadius = 15;
  double const outerRadius = 41; 
  int    const nb_rings    = 16; // Number of ring channels
  int    const nb_sectors  = 32; // Number of sectors channels
  double const ring_thickness = (outerRadius-innerRadius)/nb_rings;

private:
  bool static s_Initialised;
  
};

bool DSSD::s_Initialised = false;

// Initialise static members :
std::array<bool, 1000>  DSSD::isS1     = {0};
std::array<bool, 1000>  DSSD::isS2     = {0};
std::array<bool, 1000>  DSSD::isSector = {0};
std::array<bool, 1000>  DSSD::isRing   = {0};
std::array<uchar, 1000> DSSD::indexes  = {255};

// std::array<Timewalk> DSSD::m_timewalk;

void DSSD::Reset()
{
  for (auto const & sector : Sector_Hits)
  {
    Sectors[sector].Reset();
  }
  Sector_Hits.clear();

  for (auto const & ring : Ring_Hits)
  {
    Rings[ring].Reset();
  }
  Ring_Hits.clear();

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
      Ring_Hits.push_back(index);
      Rings[index].nrj = nrj;
      Rings[index].time = time;
    }
    else
    {
      SectorMult++;
      Sector_Hits.push_back(index);
      Sectors[index].nrj = nrj;
      Sectors[index].time = time;
    }
  }
}

#endif //DSSD_HPP
