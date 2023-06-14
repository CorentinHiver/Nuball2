
#define N_SI_136
// #define USE_LICORNE
#define USE_PARIS
#define USE_DSSD
#define QDC2
// #define DATA
#define CORENTIN

#define USE_RF 200 

#include <MTFasterReader.hpp>
#include <Faster2Root.hpp>

void countHits(Hit & hit, FasterReader & reader)
{
  int counter = 0;
  while(reader.Read()) 
  {
    counter++;
  }
  print("File with", counter, "hits");
}

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

  Path datapath = std::string(std::getenv("HOME"));
       if ( datapath == "/home/corentin") datapath+="/faster_data/";
  else if ( datapath == "/home/faster") datapath="/srv/data/nuball2/";
  else {print("Unkown HOME path, please add yours on top of this line ---|---"); return -1;}

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
  std::string time_ref = "301";
  std::string timewindow = "1500";
  int nb_files_ts = 20;
  
#elif defined (N_SI_120)
  std::string manip_name = "N-SI-120";
  std::string ID_file = "../NearLine/ID/index_120.list";
  std::string cal_file = "../NearLine/Calibration/calib_120.dat";
  std::string time_ref = "200";
  std::string timewindow = "1500";
  int nb_files_ts = 20;

#endif

  auto run = datapath+"faster_data/N-SI-136/run_50.fast/";

  MTFasterReader reader(run);
  reader.execute(countHits);
  // MTObject::parallelise_function(MTFasterReader::Read, reader, countHits);

  // Timeshifts m_timeshifts;
  // m_timeshifts.setDetectorsList(ID_file);
  // auto run = datapath+"faster_data/N-SI-136/run_50.fast/";
  // std::string run_name = "run_50";
  // m_timeshifts.setTimeWindow(timewindow);
  // m_timeshifts.setMaxHits((ulong)1.E+7);
  // m_timeshifts.setTimeReference(time_ref);
  // m_timeshifts.setOutData(run_name+".dT");
  // m_timeshifts.setOutRoot(run_name+".root");
  // m_timeshifts.calculate(run, 5);

  // auto outdir=datapath+manip_name+"-root-2/";
  // auto datadir=datapath+manip_name+"/";

  // Faster2Root f2r(ID_file);
  // f2r.verbose();

  // Manip runs(datadir+"list_runs.list");
  // std::string run_folder;
  // while(runs.getNext(run_folder))
  // {
  //   Path runpath = datadir+run_folder;
  //   f2r.setRunPath(runpath);

  //   if (!f2r.ok()) { print("ERREUR DE PROTOCOLE"); return -1; }

  //   auto run_name = rmPathAndExt(run_folder);
  //   auto ts_file = outdir+"Timeshifts/"+run_name+".dT";
  //   if (!file_exists(ts_file))
  //   {
  //     auto ts_parameters = "timewindow: "+timewindow+" time_reference: "+time_ref+
  //             " outData: "+run_name+".dT outRoot: "+run_name+".root outDir: "+outdir;
  //     if (!f2r.calculateTimeshifts(ts_parameters, nb_files_ts)) return -1;
  //   }

  //   if (!f2r.loadTimeshifts(ts_file)) return -1;
  //   if (!f2r.loadCalibration(cal_file)) return -1;
  //   f2r.convert(10);
  // }

  return 1;
}
