
struct quick_parameters
{
#ifdef N_SI_129

#if defined (CORENTIN)

  std::string outDir = "129/conversion_v3/";
  std::string fileID = "ID/index_129.dat";
  std::string runs_list = "Parameters/runs_pulsed_Corentin.list";
  std::string dataPath = "/home/corentin/faster_data/N-SI-129-root/M2G1_D1_TRIG/";
  // std::string dataPath = "../Data_129/M2G1/";
  // std::string dataPath = "../Data_129/conversion_v2/";
  UShort_t nb_threads = 4;
  int nb_max_evts_in_file = 1000000; // 1 millions evts/fichier

#elif defined (DATA2)
  std::string outDir = "/srv/data/nuball2/N-SI-129-root/DSSD_TRIG_v2/";
  std::string fileID = "ID/index_129.dat";
  std::string runs_list = "Parameters/list_runs_pulsed.list";
  std::string dataPath = "/srv/data/nuball2/N-SI-129-root/conversion_v2/";
  UShort_t nb_threads = 10;
  int nb_max_evts_in_file = 5000000; // 5 millions evts/fichier

  #ifndef USE_RF
  outDir.pop_back();
  outDir = outDir+"_NO_RF/";
  #endif //NO USE_RF

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
