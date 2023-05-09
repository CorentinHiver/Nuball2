
struct quick_parameters
{
#ifdef N_SI_129

#if defined (DSSD_TRIG)
  std::string const out = "DSSD_TRIG_v2/";
#elif defined (M2G1_TRIG)
  std::string const out = "M2G1_TRIG_v2/";
#endif //DSSD_TRIG

#if defined (CORENTIN)
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/runs_pulsed_Corentin.list";
  std::string const dataPath = "../Data_129/M2G1/";
  std::string const outDir = "129/"+out;
  UShort_t nb_threads = 4;
  int const nb_max_evts_in_file = 5000000; // 1 millions evts/fichier

#elif defined (DATA2)
  std::string const fileID = "ID/index_129.dat";
  std::string const runs_list = "Parameters/list_runs.list";
  std::string const dataPath = "/srv/data/nuball2/N-SI-129-root/conversion_v2/";
  std::string const outDir = "/srv/data/nuball2/N-SI-129-root/"+out;
  UShort_t nb_threads = 30;
  int const nb_max_evts_in_file = 5000000; // 5 millions evts/fichier

#endif

  MTList<std::string> runs;
  size_t current_run = 0;

  Long64_t RF_shift = 50000; //ps

  std::vector<TH2F*> RF_VS_LaBr3;
  std::vector<TH2F*> RF_VS_Ge;

  void writeHisto()
  {
    std::unique_ptr<TFile> file (TFile::Open((outDir+"histo.root").c_str(),"recreate"));

    for (auto const & histo : RF_VS_LaBr3) if (histo) histo -> Write();
    for (auto const & histo : RF_VS_Ge) if (histo) histo -> Write();

    file -> Write();
    file -> Close();

    for (auto const & histo : RF_VS_LaBr3) if (histo) delete histo;
    for (auto const & histo : RF_VS_Ge) if (histo) delete histo;
  }

  #endif //N_SI_129
};
