// 1. Parameters

// 2. Include library
#include <MTObject.hpp>
#include <libRoot.hpp>
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MultiHist.hpp>        // Use this to thread safely fill histograms
#include <Calibration.hpp>    // Either loads or calculate calibration coefficients
#include <Timeshifts.hpp>     // Either loads or calculate timeshifts between detectors
#include <RF_Manager.hpp>     // Eases manipulation of RF information

/*

All the distances are given in mm
Z axis from bottom to top
Y axis in the direction of the beam
X axis : if looking in the direction of the beam (accelerator in the back, EDEN in the front), x increases to the right

*/

// LABELS TO BE CONFIRMED

constexpr Label labelGe1 = 2;
constexpr Label labelGe2 = 3;
constexpr Label labelEDEN = 1;
constexpr Label labelX = 5;
constexpr Label labelTheta = 6;

class Point
{
public:
  Point(std::array<double, 2> point = {0,0}) :
    m_x(point[0]),
    m_y(point[1])
  {
  }

  Point(std::initializer_list<double> point = {0,0}) :
    m_x(*point.begin()),
    m_y(*(point.begin()+1))
  {
  }
  
  Point(Point const & point) :
    m_x(point.m_x),
    m_y(point.m_y)
  {
  }

  auto & x() {return m_x;} 
  auto & y() {return m_y;} 

  auto const & x() const {return m_x;} 
  auto const & y() const {return m_y;} 

  Point operator+(Point const & other) const {return Point({m_x + other.m_x, m_y + other.m_y});}
  Point operator-(Point const & other) const {return Point({m_x - other.m_x, m_y - other.m_y});}

  Point& operator+=(Point const & other) {m_x += other.m_x; m_y += other.m_y; return *this;}
  Point& shift     (Point const & other) {m_x += other.m_x; m_y += other.m_y; return *this;}
  Point& operator-=(Point const & other) {m_x -= other.m_x; m_y -= other.m_y; return *this;}

  Point& operator=(Point const & other) 
  {
    m_x = other.m_x;
    m_y = other.m_y;
    return *this;
  }

  Point& shift(std::array<double, 2> point = {0,0})
  {
    m_x += point[0];
    m_y += point[1];
    return *this;
  }

  Point& shiftX(double const & _shiftX)
  {
    m_x += _shiftX;
    return *this;
  }

  Point& shiftY(double const & _shiftY)
  {
    m_y += _shiftY;
    return *this;
  }

private:
  double m_x = 0;
  double m_y = 0;
};

class Cone
{
public:
  Cone(Point const & small_1, Point const & small_2, Point const & right_first, Point const & right_last) :
    m_small_1  (small_1),
    m_small_2   (small_2),
    m_right_first (right_first),
    m_right_last  (right_last)
  {}

  Cone(Point const & pointCenterSmall, double const & radiusSmall, Point const & pointCenterLarge, double const & radiusLarge) :
    m_small_1  (pointCenterSmall),
    m_small_2   (pointCenterSmall),
    m_right_first (pointCenterLarge),
    m_right_last  (pointCenterLarge)
  {
    m_small_1 .shiftX(-radiusSmall);
    m_small_2  .shiftX( radiusSmall);
    m_right_first.shiftX(-radiusLarge);
    m_right_last .shiftX( radiusLarge);
  }

private:
  Point m_small_1;
  Point m_small_2;
  Point m_right_first;
  Point m_right_last;

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
  NeutronCollimator(Point const & startPoint,  double const & length, double const & inner_radius, double const & outer_radius) :
    m_startPoint(startPoint),
    m_length(length),
    m_inner_radius(inner_radius),
    m_outer_radius(outer_radius)
  {}

private:
  Point  m_startPoint;
  double m_length;
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
  // ScanningTable(Point const & left_extrema, Point const & right_extrema, double const & radius) :
  //   m_left_extrema(left_extrema),
  //   m_right_extrema(right_extrema),
  //   m_radius(radius)
  // {
  // }

  ScanningTable(Point const & center, double const & radius) :
    m_center(center),
    m_radius(radius)
  {
  }

private:
  // Point m_left_extrema;
  // Point m_right_extrema;
  // double m_radius;
  Point m_center;
  double m_radius;
};

class Setup
{
public:
  Setup(NeutronBeam * beam, ScanningTable * table) :
    m_beam (beam),
    m_table (table)
  {}

private:
  NeutronBeam * m_beam = nullptr;
  ScanningTable * m_table = nullptr;
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
  Sinogram3D() : Sinogram(){}

private:
};

// 5. Main
int main(int argc, char** argv)
{
  int nb_threads = 2;
  std::string run = "";
  // bool sinogram = false;
  if (argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      std::string const & command = argv[i];
      if (command == "-r" || command == "--run") run = argv[++i];
      // else if (command == "-s" || command == "--sinogram") sinogram = true;
    }
  }

  if (run=="") Colib::throw_error("No run...");

  // Setup the path accordingly to the machine :
  Path path = "~/FNT/"+run;

  // Load some modules :
  // // detectors.load("index_FNTc.list");
  Calibration calib("FNTc.calib");
  Timeshifts timeshifts("FNTc.timeshifts");

  // LicorneSource source({0, 0});
  // NeutronCollimator collimator({0,0}, 100, 10, 100);
  // ScanningTable table({0,0}, 200);
  // NeutronBeam beam(source, collimator);
  // Setup setup(&beam, &table);

  RF_Manager::label = 99;
  thread_local RF_Manager rf;

  MultiHist<TH2F> sinogram("simple sinogram", "simple sinogram", 1000,-20,20, 1000,0,180);
  MultiHist<TH1F> germanium("Germanium", "Germanium", 10000,0,10000);
  MultiHist<TH1F> Ge1("Ge1", "Ge1", 10000,0,10000);
  MultiHist<TH1F> Ge2("Ge2", "Ge2", 10000,0,10000);
  MultiHist<TH1F> EDEN("EDEN", "EDEN", 10000,0,10000);

  // auto isX = [](Label const & label) {return label == 5;};
  // auto isTheta = [](Label const & label) {return label == 6;};
  // auto isGe = [](Label const & label) {return label == 3;};
  // auto isEden = [](Label const & label) {return label == 4;};

  static auto constexpr isGe    = Colib::LUT<10> ([](Label const & label) {return label == labelGe1 || label == labelGe2;});
  static auto constexpr isGe1   = Colib::LUT<10> ([](Label const & label) {return label == labelGe1;});
  static auto constexpr isGe2   = Colib::LUT<10> ([](Label const & label) {return label == labelGe2;});
  static auto constexpr isEden  = Colib::LUT<10> ([](Label const & label) {return label == labelEDEN;});
  static auto constexpr isX     = Colib::LUT<10> ([](Label const & label) {return label == labelX;});
  static auto constexpr isTheta = Colib::LUT<10> ([](Label const & label) {return label == labelTheta;});

  MTFasterReader reader(path);
  reader.setTimeshifts(timeshifts);
  reader.readAligned([&](Alignator & tree, Hit & hit){
    // Treat each faster file in parallel
    double x = NAN;
    double theta = NAN;
    auto const & nb_hits = tree->GetEntries();
    for (int hit_i = 0; hit_i<nb_hits; ++hit_i)
    {
      tree.GetEntry(hit_i); // Alignator::GetEntry is a wrapper around TTree::GetEntry that takes into account the time re-ordering
           if (rf.setHit(hit)) continue;
      else if (isX[hit.label]) x = hit.nrj;
      else if (isTheta[hit.label]) theta = hit.nrj;
      else if (x != NAN && theta != NAN)
      {
        hit.nrj = calib(hit.nrj, hit.label);
             if (isGe[hit.label])
        {
          
        }
        else if (isEden[hit.label])
        {
          sinogram.Fill(theta, x);
        }
      }
    }
  });

  auto file (TFile::Open("test.root", "recreate")) ;
  file->cd();
  sinogram.Write();
  file->Close();

  return 1;
}
