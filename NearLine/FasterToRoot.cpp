// Define BEFORE include NearLine.hpp
#define FASTERAC
#define N_SI_129
// #define N_SI_120

//Define the trigger here :
// #define D1_TRIG
// #define NO_TRIG
#define M2G1_TRIG

//Include the base library
#include "NearLine-lib/NearLine.hpp"

std::vector<std::string> all_runs(std::string const & listfilename);

int main()
{
  // std::string path = "/home/corentin/faster_data/";
  std::string path = "/srv/data/nuball2/";
  // std::string path = "/data_nuball2/";

#if defined (N_SI_120)
  std::string datadir = path+"N-SI-120/";
  std::string outdir  = path+"N-SI-120-root/";

#elif defined (N_SI_129)
  std::string datadir = path+"N-SI-129/";
  std::string outdir  = path+"N-SI-129-root/";
#endif

  std::string dT_folder = outdir;
  std::vector<std::string> runs = all_runs(datadir+"list_runs.list");
  // std::vector<std::string> runs = all_runs(datadir+"list_files.list");

  print();
  print("Reading data from folder ", datadir);
  print("Writting data to folder " , outdir);
  print();

  NearLine *app;
  for (size_t i = 0; i<runs.size(); i++)
  {
    print("-------------");
    print("Treating run n°", std::to_string(i+1), ":", rmPathAndExt(runs[i]));
    print("...");

    //Timeshift :
    std::string dT_file = rmPathAndExt(runs[i])+".dT";
    std::string dT_root = rmPathAndExt(runs[i])+".root";
    if ( file_exists(dT_folder+"Timeshifts/"+dT_file) ) {print("Timeshift data file",dT_folder+"Timeshifts/"+dT_file,"already exists !");}
    else
    {
      app = new NearLine;
      app -> setConfig("NB_THREADS: 10");

    #if defined (N_SI_120)
    app -> setConfig("USE_RF: 40");
      if (!app -> setConfig("ID: ID/index_120.dat")) return 0;
      if (!app -> setConfig("TIMESHIFT: time_reference: R1A1_FATIMA_LaBr3 timewindow: 1500 "
        "mult: 2 3 outRoot: "+dT_root+" outData: "+dT_file)) return 0;

    #elif defined (N_SI_129)
      app -> setConfig("USE_RF: 60");
      if (!app -> setConfig("ID: ID/index_129.dat")) return 0;
      if (!app -> setConfig("TIMESHIFT: time_reference: R1A9_FATIMA_LaBr3 timewindow: 1500 "
        "mult: 2 3 outRoot: "+dT_root+" outData: "+dT_file)) return 0;
    #endif

      if (!app -> setConfig("OUTDIR: "+dT_folder)) return 0;
      if (!app -> setConfig("DATADIR: "+datadir+runs[i]+" nb: 10")) return 0;

      if (!app -> launch()) {print ("CANT RUN !"); return 0;}
      delete app;
      print("Timeshifts calculated, now processing the data :");
    }

    //Faster2root :
    app = new NearLine;
    // app -> setConfig("NB_THREADS: 5");
    app -> setConfig("NB_THREADS: 30");

  #if defined (N_SI_120)
    if (!app -> setConfig("ID: ID/index_120.dat")) return 0;
    app -> setConfig("USE_RF: 40");
    if (firstPart(removePath(runs[i]), '_') == "Uranium238") {if (!app -> setConfig("CALIBRATION: Calibration/152Eu_Uranium238_after_108.dat")) return 0;}
    else if (firstPart(removePath(runs[i]), '_') == "run") {if (!app -> setConfig("CALIBRATION: Calibration/run_oct2022_before.dat")) return 0;}
    else {print("Can't recognize the run of "+removePath(runs[i])); continue;}

  #elif defined (N_SI_129)
    if (!app -> setConfig("ID: ID/index_129.dat")) return 0;
    app -> setConfig("USE_RF: 60");
    if (!app -> setConfig("CALIBRATION: Calibration/calib_129.dat")) return 0;
  #endif

    if (!app -> setConfig("TIMESHIFT_DATA: "+dT_folder+"Timeshifts/"+dT_file)) return 0;
    if (!app -> setConfig("DATADIR: "+datadir+runs[i])) return 0;


    if (!app -> setConfig("FASTER2ROOT: outDir: "+outdir+rmPathAndExt(runs[i]))) return 0;
    if (!app -> launch()) {print ("CANT RUN !"); return 0;}
    delete app;


    print("-------------");
    print();
  }
  return 1;
}

std::vector<std::string> all_runs(std::string const & listfilename)
{
  std::vector<std::string> ret;
  std::ifstream file;
  file.open(listfilename);
  if (!file.good()){print(listfilename, "NOT FOUND !"); return ret;}
  std::string line;
  while(file.good())
  {
    file >> line;
    ret.push_back(line);
  }
  return ret;
}
