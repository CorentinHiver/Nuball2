#ifndef COBALTECALORIMETER_HPP
#define COBALTECALORIMETER_HPP

#include "../MTObjects/MTObject.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/Event.hpp"
#include "../Modules/Timeshifts.hpp"
#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTRootReader.hpp"
#include "../MTObjects/MTTHist.hpp"

class CobaltCalorimeter
{
public:
  CobaltCalorimeter() {}
  void loadID(std::string const & filename) {m_IDfilename = filename;}
  void loadCalibration(Calibration calib) {m_calib = calib;}
  void loadTimeshifts(Timeshifts timeshifts) {m_timeshifts = timeshifts;}
  void setTimewindow_ns(Time_ns const & time_ns) {m_timewindow = Time_cast(time_ns*1000);}
  void setOutName(std::string const & outfilename) {m_outfilename = outfilename;}

  void Initialise();
  void launchRoot(std::string const & foldername, int nb_files = -1);
  void work(Nuball2Tree & tree, Event & event);

  /// @brief legacy
  void launchFaster(std::string const & foldername, int nb_files = -1);
  /// @brief legacy
  void work(Hit & hit, Alignator & reader);

private:
  static void dispatch_workFaster(Hit & hit, Alignator & reader, CobaltCalorimeter & cb) {cb.work(hit, reader);}
  static void dispatch_workRoot(Nuball2Tree & tree, Event & event, CobaltCalorimeter & cb) {cb.work(tree, event);}
  void write();
  void Analyse();
  bool NaI_pid(Hit & hit);
  bool NaI_pid(float nrj, float nrj2);

  // Some parameters :
  std::string m_IDfilename;
  Time m_timewindow = 100000;
  Calibration m_calib;
  Timeshifts m_timeshifts;
  std::string m_outfilename = "CobaltCalorimetry.root";

  // Internal variables :
  int m_nb_trigg = 0;
  int m_nb_missed = 0; // Nb of mult = 1 when trigger (->trigger is alone)
  int m_minE = 1326;
  int m_maxE = 1339;
  // int m_minE = 1169;
  // int m_maxE = 1179;
  double m_calo_totale = 0.0;
  double m_calo_totale_Ge = 0.0;
  double m_calo_totale_BGO = 0.0;
  double m_calo_totale_LaBr3 = 0.0;
  double m_calo_totale_NaI = 0.0;

  // Histograms :
  
  MTTHist<TH1F> spectrum_Ge;
  MTTHist<TH1F> spectrum_BGO;
  MTTHist<TH1F> spectrum_LaBr3;
  MTTHist<TH1F> spectrum_NaI;
  MTTHist<TH1F> spectrum_Ge_trigger;
  MTTHist<TH1F> spectrum_BGO_trigger;
  MTTHist<TH1F> spectrum_LaBr3_trigger;
  MTTHist<TH1F> spectrum_NaI_trigger;
  MTTHist<TH2F> spectra_trigger;

  MTTHist<TH1F> calorimetry_Ge_histo;
  MTTHist<TH1F> calorimetry_LaBr_histo;
  MTTHist<TH1F> calorimetry_BGO_histo;
  MTTHist<TH1F> calorimetry_NaI_histo;

  MTTHist<TH1F> smeared_calorimetry_Ge_histo;
  MTTHist<TH1F> smeared_calorimetry_Clover_histo;
  MTTHist<TH1F> smeared_calorimetry_LaBr_histo;
  MTTHist<TH1F> smeared_calorimetry_BGO_histo;
  MTTHist<TH1F> smeared_calorimetry_NaI_histo;
  MTTHist<TH1F> smeared_calorimetry_Paris_histo;

  MTTHist<TH2F> smeared_calorimetry_Ge_histo_VS_cristalMult;
  MTTHist<TH2F> smeared_calorimetry_BGO_histo_VS_cristalMult;
  MTTHist<TH2F> smeared_calorimetry_Clover_histo_VS_cristalMult;
  MTTHist<TH2F> smeared_calorimetry_LaBr_histo_VS_cristalMult;
  MTTHist<TH2F> smeared_calorimetry_NaI_histo_VS_cristalMult;
  MTTHist<TH2F> smeared_calorimetry_Paris_histo_VS_cristalMult;
  
  MTTHist<TH1F> spectrum_calorimetry;
  MTTHist<TH1F> spectrum_smeared_calorimetry;
  MTTHist<TH1F> spectrum_smeared_calorimetry_double_cascade;

  MTTHist<TH2F> spectrum_smeared_calorimetry_VS_Multiplicity;
  MTTHist<TH2F> calo_double_cascades_VS_multiplicity;

  MTTHist<TH2F> spectra_NaI_trigger;

  MTTHist<TH2F> timewalk_Ge;
  MTTHist<TH2F> timewalk_BGO;
  MTTHist<TH2F> timewalk_LaBr;
  MTTHist<TH2F> timewalk_NaI;

  MTTHist<TH2F> timing_VS_trigger;
  MTTHist<TH2F> timing_VS_ref;
  MTTHist<TH2F> nrj_each;
  MTTHist<TH2F> nrj_each_trigger;
  MTTHist<TH1F> labels;
  MTTHist<TH2F> labels2D_trigger;

  MTTHist<TH1F> paris_pid;

  MTTHist<TH2F> Ge_Ge;
  MTTHist<TH2F> Ge_BGO;
  MTTHist<TH2F> Ge_LaBr;
  MTTHist<TH2F> Ge_NaI;
  MTTHist<TH2F> Ge_Ge_trigger;
  MTTHist<TH2F> Ge_BGO_trigger;
  MTTHist<TH2F> Ge_LaBr_trigger;
  MTTHist<TH2F> Ge_NaI_trigger;

  MTTHist<TH2F> Ge_VS_smeared_calorimetry;
  MTTHist<TH2F> BGO_VS_smeared_calorimetry;
  MTTHist<TH2F> Clover_VS_smeared_calorimetry;
  MTTHist<TH2F> NaI_VS_smeared_calorimetry;
  MTTHist<TH2F> LaBr_VS_smeared_calorimetry;
  MTTHist<TH2F> Paris_VS_smeared_calorimetry;

  MTTHist<TH2F> Ge_calo_VS_smeared_calorimetry;
  MTTHist<TH2F> BGO_calo_VS_smeared_calorimetry;
  MTTHist<TH2F> NaI_calo_VS_smeared_calorimetry;
  MTTHist<TH2F> LaBr_calo_VS_smeared_calorimetry;
  MTTHist<TH2F> Paris_calo_VS_smeared_calorimetry;
  MTTHist<TH2F> Clover_calo_VS_smeared_calorimetry;

  MTTHist<TH2F> Ge_VS_BGO_calorimetry;

  MTTHist<TH2F> Ge_calo_VS_BGO_calo;
  MTTHist<TH2F> LaBr3_calo_VS_BGO_calo;
  MTTHist<TH2F> NaI_calo_VS_BGO_calo;
  MTTHist<TH2F> Clover_calo_VS_BGO_calo;
  MTTHist<TH2F> Paris_calo_VS_BGO_calo;

  MTTHist<TH2F> Ge_calo_VS_Clover_calo;
  MTTHist<TH2F> BGO_calo_VS_Clover_calo;
  MTTHist<TH2F> LaBr3_calo_VS_Clover_calo;
  MTTHist<TH2F> NaI_calo_VS_Clover_calo;
  MTTHist<TH2F> Paris_calo_VS_Clover_calo;

  MTTHist<TH2F> Ge_calo_VS_Paris_calo;
  MTTHist<TH2F> LaBr3_calo_VS_Paris_calo;
  MTTHist<TH2F> NaI_calo_VS_Paris_calo;

  MTTHist<TH2F> NaI_calo_VS_LaBr_calo;
  
  MTTHist<TH1F> Paris_calo_without_Clover;
  MTTHist<TH1F> Clover_calo_without_Paris;

  MTTHist<TH1F> total_energy_per_detector;
};

void CobaltCalorimeter::Initialise()
{
  MTObject::Initialize();

  spectrum_Ge    . reset("spectrum_Ge"   , "Ge spectra;Energy [keV]"   , 4000, 0, 2000);
  spectrum_BGO   . reset("spectrum_BGO"  , "BGO spectra;Energy [keV]"  , 500 , 0, 2000);
  spectrum_LaBr3 . reset("spectrum_LaBr3", "LaBr3 spectra;Energy [keV]", 1000, 0, 2000);
  spectrum_NaI   . reset("spectrum_NaI"  , "NaI spectra;Energy [keV]"  , 1000, 0, 2000);
  
  spectrum_Ge_trigger   . reset("spectrum_Ge_trigger"   , "Ge spectra trigger;Energy [keV]"    , 4000, 0, 2000);
  spectrum_BGO_trigger  . reset("spectrum_BGO_trigger"  , "BGO spectra trigger;Energy [keV]"   , 500 , 0, 2000);
  spectrum_LaBr3_trigger. reset("spectrum_LaBr3_trigger", "LaBr3 spectra trigger;Energy [keV]" , 1000, 0, 2000);
  spectrum_NaI_trigger  . reset("spectrum_NaI_trigger"  , "NaI spectra trigger;Energy [keV]"   , 1000, 0, 2000);

  spectra_NaI_trigger . reset("spectra_NaI_trigger", "NaI spectra;Energy [keV]"    , 1000,0,1000, 100 ,0,2000);
  spectra_trigger     . reset("spectra_trigger"    , "Trigger spectra;Energy [keV]", 1000,0,1000, 2000,0,2000);
  
  calorimetry_Ge_histo  .reset("calorimetry_Ge_histo"  , "Calorimeter Ge ;Energy calorimeter[keV]"  , 3000, 0, 3000);
  calorimetry_BGO_histo .reset("calorimetry_BGO_histo" , "Calorimeter BGO ;Energy calorimeter[keV]" , 3000, 0, 3000);
  calorimetry_LaBr_histo.reset("calorimetry_LaBr_histo", "Calorimeter LaBr ;Energy calorimeter[keV]", 3000, 0, 3000);
  calorimetry_NaI_histo .reset("calorimetry_NaI_histo" , "Calorimeter NaI ;Energy calorimeter[keV]" , 3000, 0, 3000);

  smeared_calorimetry_Ge_histo    .reset("smeared_calorimetry_Ge_histo"    , "Calorimeter Ge ;Energy calorimeter[keV]"    , 3000, 0, 3000);
  smeared_calorimetry_Clover_histo.reset("smeared_calorimetry_Clover_histo", "Calorimeter Clover ;Energy calorimeter[keV]", 3000, 0, 3000);
  smeared_calorimetry_LaBr_histo  .reset("smeared_calorimetry_LaBr_histo"  , "Calorimeter LaBr ;Energy calorimeter[keV]"  , 3000, 0, 3000);
  smeared_calorimetry_NaI_histo   .reset("smeared_calorimetry_NaI_histo"   , "Calorimeter NaI ;Energy calorimeter[keV]"   , 3000, 0, 3000);
  smeared_calorimetry_Paris_histo .reset("smeared_calorimetry_Paris_histo" , "Calorimeter Paris ;Energy calorimeter[keV]" , 3000, 0, 3000);

  smeared_calorimetry_Ge_histo_VS_cristalMult    .reset("smeared_calorimetry_Ge_histo_VS_cristalMult"    , "Calorimeter Ge ;Energy calorimeter[keV]"    , 10,0,10, 3000,0,3000);
  smeared_calorimetry_BGO_histo_VS_cristalMult    .reset("BGO_VS_calorimetry_BGO_histo_VS_cristalMult"   , "Calorimeter BGO ;Energy calorimeter[keV]"   , 10,0,10, 3000,0,3000);
  smeared_calorimetry_Clover_histo_VS_cristalMult.reset("smeared_calorimetry_Clover_histo_VS_cristalMult", "Calorimeter Clover ;Energy calorimeter[keV]", 10,0,10, 3000,0,3000);
  smeared_calorimetry_LaBr_histo_VS_cristalMult  .reset("smeared_calorimetry_LaBr_histo_VS_cristalMult"  , "Calorimeter LaBr ;Energy calorimeter[keV]"  , 10,0,10, 3000,0,3000);
  smeared_calorimetry_NaI_histo_VS_cristalMult   .reset("smeared_calorimetry_NaI_histo_VS_cristalMult"   , "Calorimeter NaI ;Energy calorimeter[keV]"   , 10,0,10, 3000,0,3000);
  smeared_calorimetry_Paris_histo_VS_cristalMult .reset("smeared_calorimetry_Paris_histo_VS_cristalMult" , "Calorimeter Paris ;Energy calorimeter[keV]" , 10,0,10, 3000,0,3000);

  Ge_VS_BGO_calorimetry.reset("Ge_VS_BGO_calorimetry", "Ge VS BGO calorimeter;BGO calorimeter[keV];Ge [keV]", 300,0,3000, 2000,0,2000);

  spectrum_calorimetry         .reset("spectrum_calorimetry"        , "Calorimeter spectra;Energy calorimeter[keV]"        , 3000, 0, 3000);
  spectrum_smeared_calorimetry .reset("spectrum_smeared_calorimetry", "Smeared calorimeter spectra;Energy calorimeter[keV]", 3000, 0, 3000);

  Ge_VS_smeared_calorimetry    .reset("Ge_VS_smeared_calorimetry"     , "Ge VS calorimeter;Energy calorimeter[keV];Ge [keV]"       , 300,0,3000, 2000,0,2000);
  BGO_VS_smeared_calorimetry   .reset("BGO_VS_smeared_calorimetry"    , "BGO VS calorimeter;Energy calorimeter[keV];Energy [keV]"  , 300,0,3000, 2000,0,2000);
  NaI_VS_smeared_calorimetry   .reset("NaI_VS_smeared_calorimetry"    , "NaI VS calorimeter;Energy calorimeter[keV];Energy [keV]"  , 300,0,3000, 200 ,0,2000);
  LaBr_VS_smeared_calorimetry  .reset("LaBr_VS_smeared_calorimetry"   , "LaBr VS calorimeter;Energy calorimeter[keV];Energy [keV]" , 300,0,3000, 500 ,0,2000);
  Clover_VS_smeared_calorimetry.reset("Clover_VS_smeared_calorimetry" , "Ge VS calorimeter;Energy calorimeter[keV];Ge [keV]"       , 300,0,3000, 500 ,0,2000);
  Paris_VS_smeared_calorimetry .reset("Paris_VS_smeared_calorimetry"  , "Paris VS calorimeter;Energy calorimeter[keV];Energy [keV]", 300,0,3000, 500 ,0,2000);

  spectrum_smeared_calorimetry_VS_Multiplicity . reset("spectrum_smeared_calorimetry_VS_Multiplicity", 
                  "Smeared calorimeter spectra multiplicity VS multiplicity;Multiplicity;Energy calorimeter[keV]", 10,0,10, 3000,0,3000);

  Ge_calo_VS_smeared_calorimetry    .reset("Ge_calo_VS_smeared_calorimetry"    , "Ge calorimetry VS calorimeter;Energy calorimeter[keV];Ge calorimetry [keV]"        , 300,0,3000, 300,0,3000);
  BGO_calo_VS_smeared_calorimetry   .reset("BGO_calo_VS_smeared_calorimetry"   , "BGO calorimetry VS calorimeter;Energy calorimeter[keV];BGO calorimetry [keV]"      , 300,0,3000, 300,0,3000);
  NaI_calo_VS_smeared_calorimetry   .reset("NaI_calo_VS_smeared_calorimetry"   , "NaI calorimetry VS calorimeter;Energy calorimeter[keV];NaI calorimetry [keV]"      , 300,0,3000, 300,0,3000);
  LaBr_calo_VS_smeared_calorimetry  .reset("LaBr_calo_VS_smeared_calorimetry"  , "LaBr3 calorimetry VS calorimeter;Energy calorimeter[keV];LaBr3 calorimetry [keV]"  , 300,0,3000, 300,0,3000);
  Paris_calo_VS_smeared_calorimetry .reset("Paris_calo_VS_smeared_calorimetry" , "Paris calorimetry VS calorimeter;Energy calorimeter[keV];Paris calorimetry [keV]"  , 300,0,3000, 300,0,3000);
  Clover_calo_VS_smeared_calorimetry.reset("Clover_calo_VS_smeared_calorimetry", "Clover calorimetry VS calorimeter;Energy calorimeter[keV];Clover calorimetry [keV]", 300,0,3000, 300,0,3000);

  Ge_calo_VS_BGO_calo    .reset("Ge_calo_VS_BGO_calo"    ,"Ge calo VS BGO calo;BGO calorimetry[keV];Ge calorimetry[keV]"        , 300,0,3000, 300,0,3000);
  LaBr3_calo_VS_BGO_calo .reset("LaBr3_calo_VS_BGO_calo" ,"LaBr3 calo VS BGO calo;BGO calorimetry[keV];LaBr3 calorimetry[keV]"  , 300,0,3000, 300,0,3000);
  NaI_calo_VS_BGO_calo   .reset("NaI_calo_VS_BGO_calo"   ,"NaI calo VS BGO calo;BGO calorimetry[keV];NaI calorimetry[keV]"      , 300,0,3000, 300,0,3000);
  Clover_calo_VS_BGO_calo.reset("Clover_calo_VS_BGO_calo","Clover calo VS BGO calo;BGO calorimetry[keV];Clover calorimetry[keV]", 300,0,3000, 300,0,3000);
  Paris_calo_VS_BGO_calo .reset("Paris_calo_VS_BGO_calo" ,"Paris calo VS BGO calo;BGO calorimetry[keV];Paris calorimetry[keV]"  , 300,0,3000, 300,0,3000);

  
  Ge_calo_VS_Clover_calo   .reset("Ge_calo_VS_Clover_calo"   ,"Ge calo VS Clover calo;Clover calorimetry[keV];Ge calorimetry[keV]"      , 300,0,3000, 300,0,3000);
  BGO_calo_VS_Clover_calo  .reset("BGO_calo_VS_Clover_calo"   ,"BGO calo VS Clover calo;Clover calorimetry[keV];BGO calorimetry[keV]"  , 300,0,3000, 300,0,3000);
  LaBr3_calo_VS_Clover_calo.reset("LaBr3_calo_VS_Clover_calo","LaBr3 calo VS Clover calo;Clover calorimetry[keV];LaBr3 calorimetry[keV]", 300,0,3000, 300,0,3000);
  NaI_calo_VS_Clover_calo  .reset("NaI_calo_VS_Clover_calo"  ,"NaI calo VS Clover calo;Clover calorimetry[keV];NaI calorimetry[keV]"    , 300,0,3000, 300,0,3000);
  Paris_calo_VS_Clover_calo.reset("Paris_calo_VS_Clover_calo","Paris calo VS Clover calo;Clover calorimetry[keV];Paris calorimetry[keV]", 300,0,3000, 300,0,3000);
  
  Ge_calo_VS_Paris_calo    .reset("Ge_calo_VS_Paris_calo"    ,"Ge calo VS Paris calo;Paris calorimetry[keV];Ge calorimetry[keV]"      , 300,0,3000, 300,0,3000);
  LaBr3_calo_VS_Paris_calo .reset("LaBr3_calo_VS_Paris_calo" ,"LaBr3 calo VS Paris calo;Paris calorimetry[keV];LaBr3 calorimetry[keV]", 300,0,3000, 300,0,3000);
  NaI_calo_VS_Paris_calo   .reset("NaI_calo_VS_Paris_calo"   ,"NaI calo VS Paris calo;Paris calorimetry[keV];NaI calorimetry[keV]"    , 300,0,3000, 300,0,3000);

  NaI_calo_VS_LaBr_calo   .reset("NaI_calo_VS_LaBr_calo"   ,"NaI calo VS LaBr3 calo;LaBr3 calorimetry[keV];NaI calorimetry[keV]"      , 300,0,3000, 300,0,3000);

  Paris_calo_without_Clover.reset("Paris_calo_without_Clover", "Paris_calo_without_Clover", 300,0,3000);
  Clover_calo_without_Paris.reset("Clover_calo_without_Paris", "Clover_calo_without_Paris", 300,0,3000);
  
  spectrum_smeared_calorimetry_double_cascade.reset("spectrum_smeared_calorimetry_double_cascade", "Smeared calorimeter spectra 2 cascades;Energy calorimeter[keV]", 4000, 0, 4000);
  calo_double_cascades_VS_multiplicity.reset("calo_double_cascades_VS_multiplicity", 
                                             "Smeared calorimeter spectra 2 cascades VS Multiplicity;Multiplicity;Energy calorimeter[keV]", 10,0,10, 4000,0,4000);

  total_energy_per_detector.reset("total_energy_per_detector", "total_energy_per_detector;label;Total Energy [keV]", 1000,0,1000);

  timewalk_Ge       .reset("timewalk_Ge"  , "timewalk Ge"  , 200,-m_timewindow*0.1,m_timewindow*1.2, 1000,0,2000);
  timewalk_BGO      .reset("timewalk_BGO" , "timewalk BGO" , 200,-m_timewindow*0.1,m_timewindow*1.2, 500 ,0,2000);
  timewalk_LaBr     .reset("timewalk_LaBr", "timewalk LaBr", 200,-m_timewindow*0.1,m_timewindow*1.2, 500 ,0,2000);
  timewalk_NaI      .reset("timewalk_NaI" , "timewalk NaI" , 200,-m_timewindow*0.1,m_timewindow*1.2, 500 ,0,2000);

  timing_VS_trigger.reset("timing_VS_trigger", "timing_VS_trigger;label;time[ps]"           , 1000,0,1000, 200,-m_timewindow*0.1,m_timewindow*1.2);
  timing_VS_ref    .reset("timing_VS_ref"    , "timing_VS_ref;label;time[ps]"               , 1000,0,1000, 200,-m_timewindow*0.1,m_timewindow*1.2);
  nrj_each         .reset("nrj_each"         , "nrj_each;label;Energy[keV];"                , 1000,0,1000, 3000,0,3000);
  nrj_each_trigger .reset("nrj_each_trigger" , "nrj_each_trigger;label;Energy[keV];"        , 1000,0,1000, 3000,0,3000);

  labels2D_trigger .reset("labels2D_trigger" , "labels2D_trigger;label trigger;label others", 200, 0,200, 1000,0,1000);

  labels.reset("labels", "labels;label", 1000, 0, 1000);

  Ge_Ge  .reset("Ge_Ge"  , "Ge Ge;Energy [keV];Energy [keV]", 2000,0,2000, 2000,0,2000);
  Ge_BGO .reset("Ge_BGO" , "BGO;Energy [keV];Energy [keV]"  , 2000,0,2000, 250 ,0,2000);
  Ge_LaBr.reset("Ge_LaBr", "LaBr;Energy [keV];Energy [keV]" , 2000,0,2000, 500,0,2000);
  Ge_NaI .reset("Ge_NaI" , "NaI;Energy [keV];Energy [keV]"  , 2000,0,2000, 500 ,0,2000);
  
  Ge_Ge_trigger  .reset("Ge_Ge_trigger"  , "Ge Ge;Energy [keV];Energy [keV]"     , 2000,0,2000, 2000,0,2000);
  Ge_BGO_trigger .reset("Ge_BGO_trigger" , "BGO VS Ge;Energy [keV];Energy [keV]" , 2000,0,2000, 250 ,0,2000);
  Ge_LaBr_trigger.reset("Ge_LaBr_trigger", "LaBr VS Ge;Energy [keV];Energy [keV]", 2000,0,2000, 1000,0,2000);
  Ge_NaI_trigger .reset("Ge_NaI_trigger" , "NaI VS Ge;Energy [keV];Energy [keV]" , 2000,0,2000, 500 ,0,2000);

  paris_pid.reset("paris_pid", "paris_pid;(long-short)/long", 500,-2,3);
}

void CobaltCalorimeter::launchRoot(std::string const & foldername, int nb_files)
{
  if (!detectors) detectors.load(m_IDfilename);
  if (!detectors) throw Detectors::Error();

  this -> Initialise();
  print("Histograms initialized");

  MTRootReader reader(foldername, nb_files);
  print("Launching the reader");
  reader.read(dispatch_workRoot, *this);

  this->Analyse();
  this->write();
}

bool CobaltCalorimeter::NaI_pid(float nrj, float nrj2)
{
  // debug((nrj2-nrj)/double_cast(nrj2));
  // pauseDebug();
  paris_pid.Fill((nrj2-nrj)/double_cast(nrj2));
  return ((nrj2-nrj)/double_cast(nrj2) > 0.15);
}

bool CobaltCalorimeter::NaI_pid(Hit & hit) 
{
  if (!isParis[hit.label] ) return false;
  paris_pid.Fill((hit.qdc2-hit.adc)/double_cast(hit.qdc2));
  return ((hit.qdc2-hit.adc)/double_cast(hit.qdc2) > 0.15);
}

void calibBGO(Hit & hit) {if (isBGO[hit.label]) hit.nrj*=1.15;}

float smear(float const & nrj, TRandom* random)
{
  return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
}

float smear(float const & nrj, Label const & label, TRandom* random)
{
  if (nrj>0)
  {
    if (isGe[label])         return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
    else if (isParis[label]) return random->Gaus(nrj,nrj*((400.0/sqrt(nrj))/100.0)/2.35);
    else return nrj;
  }
  else return 0;
}

bool inGate(double const & value, double const & min, double const & max) {return (value>min && value<max);}

void CobaltCalorimeter::work(Nuball2Tree & tree, Event & event)
{
  thread_local TRandom* random = new TRandom();
  random->SetSeed(time(0));

  debug("thread", MTObject::getThreadIndex());

  // The following local variables will be fused with the global variable of the object
  thread_local int nb_trigg = 0;
  thread_local int nb_missed = 0;
  thread_local double calo_totale = 0;
  thread_local double calo_totale_Ge = 0;
  thread_local double calo_totale_BGO = 0;
  thread_local double calo_totale_LaBr = 0;
  thread_local double calo_totale_NaI = 0;

  Bools isNaI;
  Bools isRejected;

  double calo_double_cascade = 0;
  int mult_double_cascade = 0;
  int nb_cascades = 0;

  auto const & nb_evts = tree->GetEntries();
  for (int evt_i = 0; evt_i<nb_evts; ++evt_i)
  { // Iterate over the events of the file

    tree->GetEntry(evt_i);

    isNaI.resize(event.mult, false);
    isRejected.resize(event.mult, false);

    int nb_Ge = 0;
    int nb_BGO = 0;
    int nb_NaI = 0;
    int nb_LaBr = 0;

    bool trigger = false;

    Hit Ge_hit;
    int Ge_hit_i = -1;
    
    double calorimetry = 0;
    double calorimetry_Ge = 0;
    double calorimetry_LaBr = 0;
    double calorimetry_BGO = 0;
    double calorimetry_NaI = 0;

    double smeared_calorimetry = 0;
    double smeared_Ge = 0;
    double smeared_NaI = 0;
    double smeared_LaBr = 0;

    int multiplicity = event.mult;

    // Calibrate data and pre-treatment :
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {// Iterate over the hits of the event
      auto const & label = event.labels[hit_i];
      auto const & time  = event.times [hit_i];
      auto const & adc   = event.adcs  [hit_i];
      auto const & qdc2  = event.qdc2s [hit_i];

      auto & nrj = event.nrjs[hit_i];

      if (isParis[label] && NaI_pid(adc, qdc2)) 
      {
        isNaI[hit_i] = true;
        nrj = m_calib.calibrate(qdc2, label);
        nrj*=1.1; // CALIBRATE BETTER !!!
      }
      else nrj = m_calib.calibrate(adc, label);

      if (isBGO[label]) nrj*=1.11; // CALIBRATE BETTER !!!

      // Throw events with too low energy or too far away from the first hit :
      if (nrj<5 || time>60000 || (isParis[label] && time>30000))
      {
        isRejected[hit_i] = true;
        --multiplicity;
        continue;
      }

           if (isGe[label])   {spectrum_Ge .Fill(nrj);   nb_Ge++;  }
      else if (isBGO[label])  {spectrum_BGO.Fill(nrj);   nb_BGO++; }
      else if (isParis[label])
      {
        if (isNaI[hit_i])     {spectrum_NaI  .Fill(nrj); nb_NaI++; }
        else                  {spectrum_LaBr3.Fill(nrj); nb_LaBr++;}
      }

      // Trigger on the wanted gamma ray in the Germaniums
      if (!trigger && isGe[label] && inGate(nrj, m_minE, m_maxE))
      {
        // Do not count the trigger as a hit in the event :
        --multiplicity;
        --nb_Ge; 

        // Toggle the trigger boolean on
        trigger = true;
        
        // Save the hit for later use
        Ge_hit_i = hit_i;
        Ge_hit = event[hit_i];
      } 

      // Calculating the calorimetry :
      else
      {
        calorimetry+=nrj;
        auto const & smeared_energy = smear(nrj, label, random);
        smeared_calorimetry += smeared_energy;
             if (isGe[label])  {calorimetry_Ge += nrj; smeared_Ge += smeared_energy;}
        else if (isBGO[label]) {calorimetry_BGO +=nrj;}
        else if (isParis[label])
        {
          if (isNaI[hit_i])    {calorimetry_NaI +=nrj; smeared_NaI +=smeared_energy;}
          else                 {calorimetry_LaBr+=nrj; smeared_LaBr+=smeared_energy;}
        }
      }
    }

    // Treat events of interest :
    if (trigger)
    {
      ++nb_trigg;
      ++nb_cascades;

      auto const & Clover_calo = smeared_Ge  + calorimetry_BGO;
      auto const & Paris_calo  = smeared_NaI + smeared_LaBr;
      auto const & nb_Clover   = nb_Ge       + nb_BGO;
      auto const & nb_Paris    = nb_LaBr     + nb_NaI;

      // Get the sum of the total calorimetry :
      calo_totale     +=calorimetry;
      calo_totale_Ge  +=calorimetry_Ge;
      calo_totale_BGO +=calorimetry_BGO;
      calo_totale_LaBr+=calorimetry_LaBr;
      calo_totale_NaI +=calorimetry_NaI;
      
      // Fill calorimeter polts :
      if (multiplicity>0)
      {
        spectrum_calorimetry.Fill(calorimetry);
        spectrum_smeared_calorimetry.Fill(smeared_calorimetry);
        spectrum_smeared_calorimetry_VS_Multiplicity.Fill(multiplicity, smeared_calorimetry);

        if(nb_Ge  >0) calorimetry_Ge_histo   .Fill(calorimetry_Ge  );
        if(nb_BGO >0) calorimetry_BGO_histo  .Fill(calorimetry_BGO );
        if(nb_LaBr>0) calorimetry_LaBr_histo .Fill(calorimetry_LaBr);
        if(nb_NaI >0) calorimetry_NaI_histo  .Fill(calorimetry_NaI );

        if(nb_Ge    >0) smeared_calorimetry_Ge_histo    .Fill(smeared_Ge  );
        if(nb_LaBr  >0) smeared_calorimetry_LaBr_histo  .Fill(smeared_LaBr);
        if(nb_NaI   >0) smeared_calorimetry_NaI_histo   .Fill(smeared_NaI );
        if(nb_Clover>0) smeared_calorimetry_Clover_histo.Fill(Clover_calo );
        if(nb_Paris >0) smeared_calorimetry_Paris_histo .Fill(Paris_calo  );

        if(nb_BGO   >0) smeared_calorimetry_BGO_histo_VS_cristalMult   .Fill(nb_BGO   , calorimetry_BGO);
        if(nb_Ge    >0) smeared_calorimetry_Ge_histo_VS_cristalMult    .Fill(nb_Ge    , smeared_Ge     );
        if(nb_LaBr  >0) smeared_calorimetry_LaBr_histo_VS_cristalMult  .Fill(nb_LaBr  , smeared_LaBr   );
        if(nb_NaI   >0) smeared_calorimetry_NaI_histo_VS_cristalMult   .Fill(nb_NaI   , smeared_NaI    );
        if(nb_Clover>0) smeared_calorimetry_Clover_histo_VS_cristalMult.Fill(nb_Clover, Clover_calo    );
        if(nb_Paris >0) smeared_calorimetry_Paris_histo_VS_cristalMult .Fill(nb_Paris , Paris_calo     );

        if (nb_Clover == 0) Paris_calo_without_Clover.Fill(Paris_calo );
        if (nb_Paris  == 0) Clover_calo_without_Paris.Fill(Clover_calo);

        if (multiplicity > 1)
        {
          if (multiplicity>nb_Ge    && nb_Ge    >0) Ge_calo_VS_smeared_calorimetry    .Fill(smeared_calorimetry, smeared_Ge     );
          if (multiplicity>nb_BGO   && nb_BGO   >0) BGO_calo_VS_smeared_calorimetry   .Fill(smeared_calorimetry, calorimetry_BGO);
          if (multiplicity>nb_LaBr  && nb_LaBr  >0) LaBr_calo_VS_smeared_calorimetry  .Fill(smeared_calorimetry, smeared_LaBr   );
          if (multiplicity>nb_NaI   && nb_NaI   >0) NaI_calo_VS_smeared_calorimetry   .Fill(smeared_calorimetry, smeared_NaI    );
          if (multiplicity>nb_Clover&& nb_Clover>0) Clover_calo_VS_smeared_calorimetry.Fill(smeared_calorimetry, Clover_calo    );
          if (multiplicity>nb_Paris && nb_Paris >0) Paris_calo_VS_smeared_calorimetry .Fill(smeared_calorimetry, Paris_calo     );
          
          if(nb_BGO > 0)
          {
            if (nb_Ge    >0) Ge_calo_VS_BGO_calo    .Fill(calorimetry_BGO, smeared_Ge  );
            if (nb_LaBr  >0) LaBr3_calo_VS_BGO_calo .Fill(calorimetry_BGO, smeared_LaBr);
            if (nb_NaI   >0) NaI_calo_VS_BGO_calo   .Fill(calorimetry_BGO, smeared_NaI );
            if (nb_Ge    >1) Clover_calo_VS_BGO_calo.Fill(calorimetry_BGO, Clover_calo );
            if (nb_Paris >0) Paris_calo_VS_BGO_calo .Fill(calorimetry_BGO, Paris_calo  );
          }

          if (nb_Clover > 0)
          {
            if (nb_Ge>0 && nb_BGO>0) 
            {
              Ge_calo_VS_Clover_calo.Fill(Clover_calo, smeared_Ge);
              BGO_calo_VS_Clover_calo.Fill(Clover_calo, calorimetry_BGO);
            }
            if (nb_LaBr >0) LaBr3_calo_VS_Clover_calo.Fill(Clover_calo, smeared_LaBr);
            if (nb_NaI  >0) NaI_calo_VS_Clover_calo  .Fill(Clover_calo, smeared_NaI );
            if (nb_Paris>0) Paris_calo_VS_Clover_calo.Fill(Clover_calo, Paris_calo  );
          }
          
          if (nb_Paris > 0)
          {
            if (nb_Ge  >0) Ge_calo_VS_Paris_calo.Fill(Paris_calo, smeared_Ge);
            if (nb_LaBr>0 && nb_NaI>0)
            {
              LaBr3_calo_VS_Paris_calo .Fill(Paris_calo, smeared_LaBr);
              NaI_calo_VS_Paris_calo   .Fill(Paris_calo, smeared_NaI );
            }
          }

          if (nb_NaI > 0 && nb_LaBr > 0) NaI_calo_VS_LaBr_calo.Fill(calorimetry_LaBr, calorimetry_NaI);
        }
      }
      else {nb_missed++;}
      // Some gamma-gamma plots :
      for (int hit_i = 0; hit_i<event.mult; hit_i++)
      {
        auto const & label = event.labels[hit_i];
        auto const & nrj   = event.nrjs  [hit_i];
        auto const & time  = event.times [hit_i];

        if (hit_i == Ge_hit_i || isRejected[hit_i]) continue;
        
        timing_VS_trigger.Fill(label, time);
        
        total_energy_per_detector.SetBinContent(label, total_energy_per_detector.GetBinContent(label)+nrj);

        if (isGe[label]) 
        {
          Ge_VS_smeared_calorimetry.Fill(smeared_calorimetry, nrj);
          if (nb_BGO>0) Ge_VS_BGO_calorimetry.Fill(calorimetry_BGO, nrj);
          timewalk_Ge.Fill(time, nrj);
        }

        else if (isBGO[label]) 
        {
          timewalk_BGO.Fill(time, nrj);
          BGO_VS_smeared_calorimetry.Fill(smeared_calorimetry, nrj);
        }
        else if (isParis[label])
        {
          if (isNaI[hit_i]) 
          {
            NaI_VS_smeared_calorimetry.Fill(smeared_calorimetry, nrj);
            timewalk_NaI.Fill(time, nrj);
          }
          else // isLaBr3
          {
            LaBr_VS_smeared_calorimetry.Fill(smeared_calorimetry, nrj);
            timewalk_LaBr.Fill(time, nrj);
          }
        }

        // We need add-back for the two following :
        // Clover_VS_smeared_calorimetry
        // Paris_VS_smeared_calorimetry

        spectra_trigger.Fill(label, nrj);

             if (isGe[label])    {spectrum_Ge_trigger .Fill(nrj);}
        else if (isBGO[label])   {spectrum_BGO_trigger.Fill(nrj);}
        else if (isParis[label])
        {
          if (isNaI[hit_i]) {spectrum_NaI_trigger  .Fill(nrj);}
          else              {spectrum_LaBr3_trigger.Fill(nrj);}
        }

        if (isNaI[hit_i]) spectra_NaI_trigger.Fill(label, nrj);

        for (int hit_j = hit_i+1; hit_j<event.mult; hit_j++)
        {
          auto const & label_j = event.labels[hit_j];
          auto const & nrj_j = event.nrjs[hit_j];

          if (hit_j == Ge_hit_i) continue;

          if (isGe[label_j])
          {
            Ge_Ge_trigger.Fill(nrj, nrj_j);
            Ge_Ge_trigger.Fill(nrj_j, nrj);
          }
          else if (isBGO[label_j]) {Ge_BGO_trigger.Fill(nrj, nrj_j);}
          else if (isNaI[hit_j]) {Ge_NaI_trigger.Fill(nrj, nrj_j);}
          else if (isParis[label_j]) {Ge_LaBr_trigger.Fill(nrj, nrj_j );}
        }
        
        labels2D_trigger.Fill(Ge_hit.label, label);
      }

      // To simulate a double cascade :
      calo_double_cascade+=smeared_calorimetry;
      mult_double_cascade += multiplicity;
      if (nb_cascades==2)
      {
        auto const calo = calo_double_cascade;
        auto const mult = mult_double_cascade;
        spectrum_smeared_calorimetry_double_cascade.Fill(calo);
        calo_double_cascades_VS_multiplicity.Fill(mult, calo);
        calo_double_cascade = 0.0;
        mult_double_cascade = 0;
        nb_cascades = 0;
      }
    }
  }

  // Update the global variables :
  {
    lock_mutex(MTObject::mutex);
    m_nb_trigg    += nb_trigg;
    m_nb_missed   += nb_missed;
    m_calo_totale += calo_totale;
    m_calo_totale_Ge    += calo_totale_Ge;
    m_calo_totale_BGO   += calo_totale_BGO;
    m_calo_totale_LaBr3 += calo_totale_LaBr;
    m_calo_totale_NaI   += calo_totale_NaI;
  }
}

void CobaltCalorimeter::Analyse()
{
  // Calculate the pic/total of the calorimeter
  print("----------------");
  print("");
  auto const & counting_efficiency = 1. - m_nb_missed/double_cast(m_nb_trigg) ;

  auto calo_spectrum = spectrum_smeared_calorimetry.Merge();
  if (!calo_spectrum) return;
  PeakFitter pe(calo_spectrum, 600, 2000);
  auto background(pe.getBackground());
  auto const & integral_peak = pe->Integral(600, 2000);
  auto const & background_peak = background->Integral(600, 2000);
  print("Mean of the peak is at : ", pe.getMean());

  auto const & total_energy = m_nb_trigg*1172.0;
  auto const & PE_efficiency = (integral_peak-background_peak)/m_nb_trigg;

  print("Total energy released =", total_energy);
  print("calo totale : ", m_calo_totale);
  print("counting efficiency :", int_cast(counting_efficiency*100.), "%");
  print("total efficiency : ", int_cast(100*m_calo_totale/total_energy), "%");
  print("calo totale Ge : ", int_cast(100*m_calo_totale_Ge/m_calo_totale), "%");
  print("calo totale BGO : ", int_cast(100*m_calo_totale_BGO/m_calo_totale), "%");
  print("calo totale LaBr3 : ", int_cast(100*m_calo_totale_LaBr3/m_calo_totale), "%");
  print("calo totale NaI : ", int_cast(100*m_calo_totale_NaI/m_calo_totale), "%");

  print("full energy efficiency : ", int_cast(PE_efficiency*100.), "%");

  print("----------------");

  labels2D_trigger.Merge();
}

void CobaltCalorimeter::write()
{
  File outfilename(m_outfilename);
  outfilename.setExtension("root");
  auto outfile(TFile::Open(outfilename.c_str(), "RECREATE"));
  if (!outfile) throw_error(concatenate("Can't create outfile ", outfilename.c_str()).c_str());

  outfile->cd();
  // labels.Write();
  labels2D_trigger->Write();

  spectrum_Ge.Write();
  spectrum_BGO.Write();
  spectrum_LaBr3.Write();
  spectrum_NaI.Write();

  spectrum_Ge_trigger.Write();
  spectrum_BGO_trigger.Write();
  spectrum_LaBr3_trigger.Write();
  spectrum_NaI_trigger.Write();

  calorimetry_Ge_histo.Write();
  calorimetry_LaBr_histo.Write();
  calorimetry_BGO_histo.Write();
  calorimetry_NaI_histo.Write();

  smeared_calorimetry_Ge_histo.Write();
  smeared_calorimetry_LaBr_histo.Write();
  smeared_calorimetry_NaI_histo.Write();
  smeared_calorimetry_Clover_histo.Write();
  smeared_calorimetry_Paris_histo.Write();
  
  smeared_calorimetry_Ge_histo_VS_cristalMult.Write();
  smeared_calorimetry_BGO_histo_VS_cristalMult.Write();
  smeared_calorimetry_Clover_histo_VS_cristalMult.Write();
  smeared_calorimetry_LaBr_histo_VS_cristalMult.Write();
  smeared_calorimetry_NaI_histo_VS_cristalMult.Write();
  smeared_calorimetry_Paris_histo_VS_cristalMult.Write();

  spectrum_calorimetry.Write();

  spectrum_smeared_calorimetry->Write();
  spectrum_smeared_calorimetry_VS_Multiplicity.Write();
  
  spectrum_smeared_calorimetry_double_cascade.Write();
  calo_double_cascades_VS_multiplicity.Write();

  spectra_NaI_trigger.Write();
  spectra_trigger.Write();

  Ge_Ge.Write();
  Ge_BGO.Write();
  Ge_LaBr.Write();
  Ge_NaI.Write();

  Ge_Ge_trigger.Write();
  Ge_BGO_trigger.Write();
  Ge_LaBr_trigger.Write();
  Ge_NaI_trigger.Write();

  Ge_VS_BGO_calorimetry.Write();

  Ge_VS_smeared_calorimetry.Write();
  Clover_VS_smeared_calorimetry.Write();
  NaI_VS_smeared_calorimetry.Write();
  LaBr_VS_smeared_calorimetry.Write();
  Paris_VS_smeared_calorimetry.Write();
  
  Ge_calo_VS_smeared_calorimetry.Write();
  BGO_calo_VS_smeared_calorimetry.Write();
  NaI_calo_VS_smeared_calorimetry.Write();
  LaBr_calo_VS_smeared_calorimetry.Write();

  Ge_calo_VS_BGO_calo.Write();
  LaBr3_calo_VS_BGO_calo.Write();
  NaI_calo_VS_BGO_calo.Write();
  Clover_calo_VS_BGO_calo.Write();
  Paris_calo_VS_BGO_calo.Write();

  Ge_calo_VS_Clover_calo.Write();
  BGO_calo_VS_Clover_calo.Write();
  LaBr3_calo_VS_Clover_calo.Write();
  NaI_calo_VS_Clover_calo.Write();
  Paris_calo_VS_Clover_calo.Write();

  Ge_calo_VS_Paris_calo.Write();
  LaBr3_calo_VS_Paris_calo.Write();
  NaI_calo_VS_Paris_calo.Write();
  NaI_calo_VS_LaBr_calo.Write();

  Paris_calo_without_Clover.Write();
  Clover_calo_without_Paris.Write();

  total_energy_per_detector.Write();

  paris_pid.Write();

  timing_VS_trigger.Write();
  timewalk_Ge.Write();
  timewalk_BGO.Write();
  timewalk_LaBr.Write();
  timewalk_NaI.Write();


  outfile->Write();
  outfile->Close();
  print(outfilename, "written");
}





//////////////////////////////////////
//            DEPRECATED            //
//////////////////////////////////////



/// @brief Deprecated @deprecated
void CobaltCalorimeter::launchFaster(std::string const & foldername, int nb_files)
{
  if (!detectors) detectors.load(m_IDfilename);
  if (!detectors) throw Detectors::Error();

  this -> Initialise();

  MTFasterReader reader(foldername, nb_files);
  reader.setTimeshifts(m_timeshifts.data());
  reader.readAligned(dispatch_workFaster, *this);

  this->Analyse();
  this->write();
}

/// @brief Deprecated @deprecated
void CobaltCalorimeter::work(Hit & hit, Alignator & reader)
{
  auto const & nb_hits = reader->GetEntries();
  TRandom* random = new TRandom();
  random->SetSeed(time(0));

  for(int hit_i = 0; hit_i<nb_hits; ++hit_i)
  {
    reader->GetEntry(hit_i);
    if ( NaI_pid(hit) ) continue;
    m_calib(hit);

    nrj_each.Fill(hit.label, hit.nrj);

    if (isGe[hit.label])
    {
      auto const stamp_Ge = hit.stamp;
      auto const label_Ge = hit.label;
      auto const hit_Ge = hit_i;
      double calorimetry = 0.0;
      double smeared_calorimetry = 0.0;
      double calo_BGO = 0.0;

      // if (hit.nrj>1325 && hit.nrj<1335)
      if (hit.nrj>1170 && hit.nrj<1178)
      {
        // ++nb_trigg;
        // spectra_trigger.Fill(hit.nrj);
        while(--hit_i > 0)
        {
          reader->GetEntry(hit_i);
          auto const & dT = Time_cast(stamp_Ge-hit.stamp);
          if (dT>m_timewindow) break;
          labels2D_trigger.Fill(label_Ge, hit.label);
          labels.Fill(hit.label);
          m_calib(hit); calibBGO(hit);
          timing_VS_trigger.Fill(hit.label, dT);
          if (NaI_pid(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

               if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
          else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
          else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
          nrj_each_trigger.Fill(hit.label, hit.nrj);

          calorimetry+=hit.nrj;
          calo_BGO+=hit.nrj;
          smeared_calorimetry+=smear(hit.nrj, hit.label, random);
        }
        hit_i = hit_Ge;
        while(++hit_i < nb_hits)
        {
          reader->GetEntry(hit_i);
          auto const & dT = Time_cast(stamp_Ge-hit.stamp);
          if (-dT>m_timewindow) break;
          labels2D_trigger.Fill(label_Ge, hit.label);
          labels.Fill(hit.label);
          m_calib(hit); calibBGO(hit);
          timing_VS_trigger.Fill(hit.label, dT);
          if (NaI_pid(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

               if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
          else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
          else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
          nrj_each_trigger.Fill(hit.label, hit.nrj);

          calorimetry+=hit.nrj;
          calo_BGO+=hit.nrj;
          smeared_calorimetry+=smear(hit.nrj, hit.label, random);
        }
        --hit_i; // Starting next loop with the hit that closed the event
        spectrum_calorimetry.Fill(calorimetry);
        spectrum_smeared_calorimetry.Fill(smeared_calorimetry);

        // Now, looking for another peak to simulate a double cascade :
        while(++hit_i < nb_hits)
        {
          reader->GetEntry(hit_i);
          
          m_calib(hit);
          nrj_each.Fill(hit.label, hit.nrj);
          if (isGe[hit.label])
          {
            auto const stamp_Ge = hit.stamp;
            auto const hit_Ge = hit_i;
            double calorimetry2 = 0.0;
            double calo_BGO2 = 0.0;
            double smeared_calorimetry2 = 0.0;

            if (hit.nrj>1170 && hit.nrj<1178)
            {
              // ++nb_trigg;
              // spectra_trigger.Fill(hit.nrj);
              while(--hit_i > 0)
              {
                reader->GetEntry(hit_i);
                auto const & dT = Time_cast(stamp_Ge-hit.stamp);
                if (dT>m_timewindow) break;
                labels2D_trigger.Fill(label_Ge, hit.label);
                labels.Fill(hit.label);
                m_calib(hit); calibBGO(hit);
                timing_VS_trigger.Fill(hit.label, dT);
                if (NaI_pid(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

                     if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
                else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
                else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
                nrj_each_trigger.Fill(hit.label, hit.nrj);

                calorimetry2+=hit.nrj;
                calo_BGO2+=hit.nrj;
                smeared_calorimetry2+=smear(hit.nrj, hit.label, random);
              }
              hit_i = hit_Ge;
              while(++hit_i < nb_hits)
              {
                reader->GetEntry(hit_i);
                auto const & dT = Time_cast(stamp_Ge-hit.stamp);
                if (-dT>m_timewindow) break;
                labels2D_trigger.Fill(label_Ge, hit.label);
                labels.Fill(hit.label);
                m_calib(hit); calibBGO(hit);
                timing_VS_trigger.Fill(hit.label, dT);
                if (NaI_pid(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

                     if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
                else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
                else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
                nrj_each_trigger.Fill(hit.label, hit.nrj);

                calorimetry2+=hit.nrj;
                calo_BGO2+=hit.nrj;
                smeared_calorimetry2+=smear(hit.nrj, hit.label, random);
              }
              --hit_i; // Starting next loop with the hit that closed the event
              spectrum_calorimetry.Fill(calorimetry2);
              spectrum_smeared_calorimetry.Fill(smeared_calorimetry2);
              spectrum_smeared_calorimetry_double_cascade.Fill(smeared_calorimetry+smeared_calorimetry2);
              break;
            }
          }
        }
      }
    }
    else if (isLaBr3[hit.label])
    {
      auto const stamp_ref = hit.stamp;
      auto const hit_ref = hit_i;
      while(--hit_i > 0)
      {
        reader->GetEntry(hit_i);
        auto const & dT = Time_cast(stamp_ref-hit.stamp);
        if (dT>m_timewindow) break;
        timing_VS_ref.Fill(hit.label, dT);
      }
      hit_i = hit_ref;
      while(++hit_i < nb_hits)
      {
        reader->GetEntry(hit_i);
        auto const & dT = Time_cast(stamp_ref-hit.stamp);
        if (-dT>m_timewindow) {--hit_i; break;}
        timing_VS_ref.Fill(hit.label, dT);
      }
      hit_i = hit_ref;
    }
  }
}


#endif //COBALTECALORIMETER_HPP