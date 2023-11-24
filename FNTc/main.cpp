// 1. Parameters
  // RF : 
#define USE_RF 200 //ns
  // Detectors :
#define USE_DSSD
#define USE_PARIS
  // Triggers :
#define TRIGGER
  // Event building :
#define PREPROMPT
// #define UNSAFE

// 2. Include library
// #include <libCo.hpp>
// #include <FasterReader.hpp>   // This class is the base for mono  threaded code
// #include <Alignator.hpp>      // Align a TTree if some events are shuffled in time
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
// #include <MTCounter.hpp>      // Use this to thread safely count what you want²
// #include <MTTHist.hpp>        // Use this to thread safely fill histograms
// #include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
// #include <Calibration.hpp>    // Either loads or calculate calibration coefficients
// #include <Detectors.hpp>      // Eases the manipulation of detector's labels
// #include <RF_Manager.hpp>     // Eases manipulation of RF information

/*

All the distances are given in mm
Z axis from bottom to top
Y axis in the direction of the beam
X axis : if looking in the direction of the beam (towards y positive), x increases to the right

*/

class Point
{
public:
  Point(std::array<double, 2> point = {0,0}) :
    m_x(point[0]),
    m_y(point[1])
  {
  }

  auto & x() {return m_x;} 
  auto & y() {return m_y;} 

  auto const & x() const {return m_x;} 
  auto const & y() const {return m_y;} 

  Point operator+(Point const & other) const {return Point({m_x + other.m_x, m_y + other.m_y});}
  Point operator-(Point const & other) const {return Point({m_x - other.m_x, m_y - other.m_y});}

  Point& shift(Point const & other)

private:
  double m_x;
  double m_y;
};

class Cone
{
public:
  Cone(Point const & left_first, Point const & left_last, Point const & right_first, Point const & right_last) :
    m_left_first  (left_first),
    m_left_last   (left_last),
    m_right_first (right_first),
    m_right_last  (right_last)
  {}

  Cone(Point const & pointCenterSmall, double const & radiusSmall, Point const & pointCenterLarge, Point const & radiusLarge) :
    m_left_first  (pointCenterSmall),
    m_left_last   (left_last),
    m_right_first (right_first),
    m_right_last  (right_last)
  {
    
  }

private:
  Point m_left_first;
  Point m_left_last;
  Point m_right_first;
  Point m_right_last

};

class LicorneSource
{
public:
  LicorneSource(Point const & center) :
    m_center(center)
  {
  }

private:
  Point m_center;
};

class NeutronCollimator
{
public:
  NeutronCollimator(Point const & startPoint,  double const & m_lenght, double const & m_inner_radius, double const & m_outer_radius = 100) :
    m_startPoint(startPoint)
  {}

private:
  Point  m_startPoint;
  double m_lenght;
  double m_inner_radius;
  double m_outer_radius;
};

class NeutronBeam
{
public:
  NeutronBeam(LicorneSource & source, NeutronCollimator & collimator) :
    m_source (&source),
    m_collimator (&collimator)
  {}

private:
  LicorneSource *m_source;
  NeutronCollimator *m_collimator;
};

class ScanningTable
{
public:
  ScanningTable(Point const & left_extrema, Point const & right_extrema, double const & radius) :
    m_left_extrema(left_extrema),
    m_right_extrema(right_extrema),
    m_radius(radius)
  {
  }

private:
  Point m_left_extrema;
  Point m_right_extrema;
  double m_radius;
  double m_y;
};

class Setup
{
public:
  Setup(NeutronBeam & beam, ScanningTable & table) :
    m_beam (&beam),
    m_table (&table)
  {}

  Cone conePath(double const & angle, double const & posX)
  {
    
  }

private:
  NeutronBeam * m_beam;
  ScanningTable * m_table;
};

class Sinogram
{
public:
  Sinogram(){}

private:
  TH2F* m_spectra;
};

class Sinogram3D : public Sinogram
{
public:
  Sinogram(){}

private:
};

// 5. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  std::string run = "";
  bool sinogram = false;
  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string const & command = argv[i];
      if (command == "-r" || command == "--run") run = argv[++i];
      else if (command == "-s" || command == "--sinogram") sinogram = true;
    }
  }

  if (run=="") throw_error("No run...");

  // Setup the path accordingly to the machine :
  Path path = "~/FNT/"+run;

  // Load some modules :
  detectors.load("index_FNTc.list");
  Calibration calib("FNTc.calib");
  Timeshifts timeshifts("FNTc.timeshifts");

  FilesManager files(path);

  LicorneSource source({0, 0});
  NeutronCollimator collimator({0,0}, 100, 10);
  NeutronBeam beam(source, collimator);
  ScanningTable table();

  Setup setup(beam, table);

  std::string filename;
  while (files.getNext(filename))
  {
    Hit hit;
    FasterReader reader(&hit, filename);

    unique_tree tempTree(new TTree("temp","temp"));
    hit.writting(tempTree);

    while(reader.Read())
    {
      hit.stamp+=timeshifts[hit.label];
      tempTree->Fill();
    }

    hit.reset();
    hit.reading(tempTree);

    Alignator gindex(readTree.get());
    RF_Manager rf;
    RF_Extractor first_rf(tempTree.get(), rf, hit, gindex);

    auto const & nb_hits = tempTree->GetEntries();

    for (int hit_i = 0; hit_i<nb_hits; hit_i++)
    {
      tempTree->GetEntries();
      //Hi, add stuff here
    }
    
  }

  return 1;
}
