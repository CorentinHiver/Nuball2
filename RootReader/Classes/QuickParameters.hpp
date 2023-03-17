
struct quick_parameters
{
#ifdef N_SI_129

#if defined (DSSD_TRIG)
  std::string const outDir = "129/DSSD_TRIG/";
#elif defined (M2G1_TRIG)
  std::string const outDir = "129/M2G1_TRIG/";
#endif //DSSD_TRIG

#if defined (CORENTIN)

    #ifdef DSSD_TRIG
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/runs_pulsed_129.list";
  std::string const dataPath = "~/faster_data/N-SI-129-root/";
  UShort_t nb_threads = 4;
  int const nb_max_evts_in_file = 1000000; // 1 millions evts/fichier

#elif defined (DATA2)
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/list_runs_pulsed.list";
  std::string const dataPath = "/srv/data/nuball2/N-SI-129-root/M2G1_TRIG/";
  UShort_t nb_threads = 3;
  int const nb_max_evts_in_file = 5000000; // 5 millions evts/fichier

#endif

  std::vector<std::string> runs;
  size_t current_run = 0;

  Long64_t RF_shift = 60000; //ps

  std::mutex mutex;

  bool getNextRun(std::string & run)
  {
    if (nb_threads>1) mutex.lock();
    if (current_run<runs.size())
    {
      run = runs[current_run];
      current_run++;
      if (nb_threads>1) mutex.unlock();
      return true;
    }
    else
    {
      current_run++;
      if (nb_threads>1) mutex.unlock();
      return false;
    }
  }

  #endif //N_SI_129x
};
