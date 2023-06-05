
#define N_SI_129
// #define USE_LICORNE
#define USE_PARIS
#define USE_DSSD
// #define DATA
#define CORENTIN

#define USE_RF 50 

#include <Faster2Root.hpp>

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


#if defined (DATA)
  std::string datapath = "/srv/data/nuball2/";
#elif defined (CORENTIN)
  std::string datapath = "/home/corentin/faster_data/";
#endif

#if defined (N_SI_129)
  std::string manip_name = "N-SI-129";
  std::string ID_file = "../NearLine/ID/index_129.list";
  std::string cal_file = "../NearLine/Calibration/calib_129.cal";
  std::string time_ref = "252";
  std::string timewindow = "1500";
  int nb_files_ts = 50;

#elif defined (N_SI_136)
  std::string manip_name = "N-SI-136";
  std::string ID_file = "../NearLine/ID/index_129.list";
  std::string cal_file = "../NearLine/Calibration/calib_136.cal";
  std::string time_ref = "252";
  std::string timewindow = "1500";
  int nb_files_ts = 50;
  
#elif defined (N_SI_120)
  std::string manip_name = "N-SI-120";
  std::string ID_file = "../NearLine/ID/index_120.list";
  std::string cal_file = "../NearLine/Calibration/calib_120.dat";
  std::string time_ref = "200";
  std::string timewindow = "1500";
  int nb_files_ts = 20;

#endif

  auto outdir=datapath+manip_name+"-root/";
  auto datadir=datapath+manip_name+"/";

  Faster2Root f2r(ID_file);

  Manip runs(datadir+"list_runs.list");
  std::string run_folder;
  while(runs.getNext(run_folder))
  {
    Path runpath = datadir+run_folder;
    f2r.setRunPath(runpath);

    if (!f2r.ok()) { print("ERREUR DE PROTOCOLE"); return -1; }

    auto run_name = rmPathAndExt(run_folder);
    auto ts_parameters = "timewindow: "+timewindow+" time_reference: "+time_ref+
            " outData: "+run_name+".dT outRoot: "+run_name+".root outDir: "+outdir;
    f2r.calculateTimeshifts(ts_parameters, nb_files_ts);
  }

  return 1;
}
