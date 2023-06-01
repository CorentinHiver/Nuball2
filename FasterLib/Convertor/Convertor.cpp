
#define N_SI_129
// #define USE_LICORNE
#define USE_PARIS
#define USE_DSSD
// #define DATA
#define CORENTIN

#define USE_RF 50 

#include <Faster2Root.hpp>

// #include <EventBuilder.hpp>
// #include <CoincBuilder.hpp>
// #include <CoincBuilder2.hpp>
// #include <FasterReader.hpp>

// #include <Sorted_Event.hpp>
// #include <ParisLabel.hpp>
//
// #include <MTTHist.hpp>
// #include <MTCounter.hpp>
//
// #include <NearLine.hpp>


int main(int argc, char** argv)
{
  ushort nb_threads = 0;
  if (argc == 3 && strcmp(argv[1],"-m") == 0)
  {
    nb_threads = atoi(argv[2]);
  }
  else
  {
    nb_threads = 2;
  }
  
  MTObject::Initialize(nb_threads);

  Faster2Root f2r;

#if defined (DATA)
  std::string path = "/srv/data/nuball2/";
#elif defined (CORENTIN)
  std::string path = "/home/corentin/faster_data/";
#endif

#ifdef N_SI_129
  std::string ID_file = "../NearLine/ID/index_129.list";
  std::string cal_file = "../NearLine/Calibration/calib_129.cal";
#endif

  DetectorsList detList(ID_file);
  Calibration cal(cal_file, detList.size());

  Timeshifts ts;
  ts.setListDet(detList);
  ts.calculate(path+"N-SI-129/run_70.fast/", 2);

  return 1;
}
