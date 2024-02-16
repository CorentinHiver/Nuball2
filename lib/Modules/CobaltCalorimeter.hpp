#ifndef COBALTECALORIMETER_HPP
#define COBALTECALORIMETER_HPP

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
  CobaltCalorimeter() noexcept = default;
  void loadID(std::string const & filename) {m_IDfilename = filename;}
  void loadCalibration(Calibration calib) {m_calib = calib;}
  void loadTimeshifts(Timeshifts timeshifts) {m_timeshifts = timeshifts;}
  void setTimewindow_ns(Time_ns const & time_ns) {m_timewindow = Time_cast(time_ns*1000);}

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
  bool parisPiD(Hit & hit);
  bool parisPiD(float nrj, float nrj2);

  // Some parameters :
  std::string m_IDfilename;
  Time m_timewindow = 100000;
  Calibration m_calib;
  Timeshifts m_timeshifts;
  std::string m_outfilename = "CobaltCalorimetry.root";

  // Internal variables :
  int nb_trigg = 0;
  int m_minE = 1328;
  int m_maxE = 1338;
  // int m_minE = 1169;
  // int m_maxE = 1179;

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
  MTTHist<TH1F> spectrum_calorimetry;
  MTTHist<TH1F> spectrum_smeared_calorimetry;
  MTTHist<TH1F> spectrum_smeared_calorimetry_double_peak;

  MTTHist<TH2F> spectrum_smeared_calorimetry_VS_Multiplicity;

  MTTHist<TH2F> spectra_NaI_trigger;

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
};

void CobaltCalorimeter::Initialise()
{
  MTObject::Initialize();

  spectrum_Ge    . reset("spectrum_Ge"   , "Ge spectra;Energy [keV]"    , 4000, 0, 2000);
  spectrum_BGO   . reset("spectrum_BGO"  , "BGO spectra;Energy [keV]"   , 500 , 0, 2000);
  spectrum_LaBr3 . reset("spectrum_LaBr3", "LaBr3 spectra;Energy [keV]" , 1000, 0, 2000);
  spectrum_NaI   . reset("spectrum_NaI"  , "NaI spectra;Energy [keV]" , 1000, 0, 2000);
  spectrum_Ge_trigger   . reset("spectrum_Ge_trigger"   , "Ge spectra trigger;Energy [keV]"    , 4000, 0, 2000);
  spectrum_BGO_trigger  . reset("spectrum_BGO_trigger"  , "BGO spectra trigger;Energy [keV]"   , 500 , 0, 2000);
  spectrum_LaBr3_trigger. reset("spectrum_LaBr3_trigger", "LaBr3 spectra trigger;Energy [keV]" , 1000, 0, 2000);
  spectrum_NaI_trigger  . reset("spectrum_NaI_trigger"  , "NaI spectra trigger;Energy [keV]"     , 1000, 0, 2000);

  spectra_NaI_trigger . reset("spectra_NaI_trigger" , "NaI spectra;Energy [keV]"    , 1000,0,1000, 100,0,2000);
  spectra_trigger     . reset("spectra_trigger"    , "Trigger spectra;Energy [keV]" , 1000,0,1000, 2000, 0, 2000);
  
  calorimetry_Ge_histo  .reset("calorimetry_Ge_histo", "Calorimeter Ge ;Energy calorimeter[keV]"    , 2999, 1, 3000);
  calorimetry_LaBr_histo.reset("calorimetry_LaBr_histo", "Calorimeter LaBr ;Energy calorimeter[keV]", 2999, 1, 3000);
  calorimetry_BGO_histo .reset("calorimetry_BGO_histo", "Calorimeter BGO ;Energy calorimeter[keV]"  , 2999, 1, 3000);
  calorimetry_NaI_histo .reset("calorimetry_NaI_histo", "Calorimeter NaI ;Energy calorimeter[keV]"  , 2999, 1, 3000);
  spectrum_calorimetry                     . reset("spectrum_calorimetry", "Calorimeter spectra;Energy calorimeter[keV]", 2999, 1, 3000);
  spectrum_smeared_calorimetry             . reset("spectrum_smeared_calorimetry", "Smeared calorimeter spectra;Energy calorimeter[keV]", 2999, 1, 3000);
  spectrum_smeared_calorimetry_double_peak . reset("spectrum_smeared_calorimetry_double_peak", "Smeared calorimeter spectra 2 cascades;Energy calorimeter[keV]", 3999, 1, 4000);
  Ge_VS_smeared_calorimetry.reset("Ge_VS_smeared_calorimetry", "Ge VS calorimeter;Energy calorimeter[keV];Ge [keV]", 299,10,3000, 2000,0,2000);
  spectrum_smeared_calorimetry_VS_Multiplicity . reset("spectrum_smeared_calorimetry_VS_Multiplicity", "Smeared calorimeter spectra multiplicity VS multiplicity;Energy calorimeter[keV]", 
                                              10,0,10, 2999,1,3000);

  timing_VS_trigger.reset("timing_VS_trigger", "timing_VS_trigger;label;time[ps]", 1000,0,1000, 200,-m_timewindow/2,m_timewindow/2);
  timing_VS_ref.reset("timing_VS_ref", "timing_VS_ref;label;time[ps]", 1000,0,1000, 200,-m_timewindow/2,m_timewindow/2);
  nrj_each.reset("nrj_each", "nrj_each;label;Energy[keV];", 1000,0,1000, 3000,0,3000);
  nrj_each_trigger.reset("nrj_each_trigger", "nrj_each_trigger;label;Energy[keV];", 1000,0,1000, 3000,0,3000);
  labels.reset("labels", "labels;label", 1000, 0, 1000);
  labels2D_trigger.reset("labels2D_trigger", "labels2D_trigger;label trigger;label others", 1000,0,1000, 1000,0,1000);

  paris_pid.reset("paris_pid", "paris_pid;(long-short)/long", 500,-2,3);

  Ge_Ge.reset("Ge_Ge", "Ge Ge;Energy [keV];Energy [keV]", 2000,0,2000, 2000,0,2000);
  Ge_BGO.reset("Ge_BGO", "BGO;Energy [keV];Energy [keV]", 2000,0,2000, 250,0,2000);
  Ge_LaBr.reset("Ge_LaBr", "LaBr;Energy [keV];Energy [keV]", 2000,0,2000, 1000,0,2000);
  Ge_NaI.reset("Ge_NaI", "NaI;Energy [keV];Energy [keV]", 2000,0,2000, 500,0,2000);
  
  Ge_Ge_trigger.reset("Ge_Ge_trigger", "Ge Ge;Energy [keV];Energy [keV]", 2000,0,2000, 2000,0,2000);
  Ge_BGO_trigger.reset("Ge_BGO_trigger", "BGO VS Ge;Energy [keV];Energy [keV]", 2000,0,2000, 250,0,2000);
  Ge_LaBr_trigger.reset("Ge_LaBr_trigger", "LaBr VS Ge;Energy [keV];Energy [keV]", 2000,0,2000, 1000,0,2000);
  Ge_NaI_trigger.reset("Ge_NaI_trigger", "NaI VS Ge;Energy [keV];Energy [keV]", 2000,0,2000, 500,0,2000);
}

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

void CobaltCalorimeter::launchRoot(std::string const & foldername, int nb_files)
{
  if (!detectors) detectors.load(m_IDfilename);
  if (!detectors) throw Detectors::Error();

  this -> Initialise();

  MTRootReader reader(foldername, nb_files);
  reader.read(dispatch_workRoot, *this);

  this->Analyse();
  this->write();
}

bool CobaltCalorimeter::parisPiD(float nrj, float nrj2)
{
  // debug((nrj2-nrj)/double_cast(nrj2));
  // pauseDebug();
  paris_pid.Fill((nrj2-nrj)/double_cast(nrj2));
  return ((nrj2-nrj)/double_cast(nrj2) > 0.15);
}

bool CobaltCalorimeter::parisPiD(Hit & hit) 
{
  if (!isParis[hit.label] ) return false;
  paris_pid.Fill((hit.qdc2-hit.adc)/double_cast(hit.qdc2));
  return ((hit.qdc2-hit.adc)/double_cast(hit.qdc2) > 0.15);
}

void calibBGO(Hit & hit) {if (isBGO[hit.label]) hit.nrj*=1.15;}

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
  TRandom* random = new TRandom();
  random->SetSeed(time(0));

  Bools isNaI;
  double smeared_calorimetry_multiple_cascades = 0;
  int nb_cascades = false;

  auto const & nb_evts = tree->GetEntries();
  for (int evt_i = 0; evt_i<nb_evts; ++evt_i)
  {
    tree->GetEntry(evt_i);

    // Calibrate data :
    isNaI.resize(event.mult, false);
    for (int hit_i = 0; hit_i<event.mult; hit_i++) 
    {
      auto const & label = event.labels[hit_i];
      auto const & adc = event.adcs[hit_i];
      auto const & qdc2 = event.qdc2s[hit_i];
      auto & nrj = event.nrjs[hit_i];
      auto & nrj2 = event.nrj2s[hit_i];

      // debug(event);

      if (isParis[label] && parisPiD(adc, qdc2)) 
      {
        isNaI[hit_i] = true;
        nrj = m_calib.calibrate(qdc2, label);
      }
      else nrj = m_calib.calibrate(adc, label);

      if (isBGO[label]) nrj*=1.15;

      if (isGe[label])        {spectrum_Ge .Fill(nrj);}
      else if (isBGO[label])  {spectrum_BGO.Fill(nrj);}
      else if (isParis[label])
      {
        if (isNaI[hit_i])       {spectrum_NaI  .Fill(nrj);}
        else                    {spectrum_LaBr3.Fill(nrj);}
      }
    }

    // Event analysis :
    bool trigger1772 = false;
    Hit Ge_hit;
    int Ge_hit_i = -1;
    double calorimetry = 0;
    double calorimetry_Ge = 0;
    double calorimetry_LaBr = 0;
    double calorimetry_BGO = 0;
    double calorimetry_NaI = 0;
    double smeared_calorimetry = 0;

    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      auto const & label_i = event.labels[hit_i];
      auto const & nrj_i = event.nrjs[hit_i];

      

      if (isGe[label_i])
      {
        if (inGate(nrj_i, m_minE, m_maxE))
        {
          Ge_hit_i = hit_i;
          trigger1772 = true;
          nb_cascades++;
          Ge_hit = event[hit_i];
        } 

        for (int hit_j = hit_i+1; hit_j<event.mult; hit_j++)
        {
          auto const & label_j = event.labels[hit_j];
          auto const & nrj_j = event.nrjs[hit_j];

          if (isGe[label_j])
          {
            Ge_Ge.Fill(nrj_i, nrj_j);
            Ge_Ge.Fill(nrj_j, nrj_i);
          }
          else if (isBGO[label_j])   {Ge_BGO.Fill(nrj_i, nrj_j);}
          else if (isParis[label_j])
          {
            if (isNaI[hit_j]) {Ge_NaI .Fill(nrj_i, nrj_j);}
            else              {Ge_LaBr.Fill(nrj_i, nrj_j );}
          }
        }
      }

      // Calculating the calorimetry :
      if (Ge_hit_i != hit_i)
      {
        calorimetry+=nrj_i;
        smeared_calorimetry += smear(nrj_i, label_i, random);
             if (isGe[label_i])    calorimetry_Ge  +=nrj_i;
        else if (isBGO[label_i])   calorimetry_BGO +=nrj_i;
        else if (isParis[label_i])
        {
          if (isNaI[hit_i]) calorimetry_NaI +=nrj_i;
          else              calorimetry_LaBr+=nrj_i;
        }
      }
    }

    // Treat events of interest :
    if (trigger1772)
    {
      nb_trigg++;
      spectrum_calorimetry.Fill(calorimetry);
      spectrum_smeared_calorimetry.Fill(smeared_calorimetry);
      spectrum_smeared_calorimetry_VS_Multiplicity.Fill(event.mult, smeared_calorimetry);

      calorimetry_Ge_histo  . Fill(calorimetry_Ge);
      calorimetry_BGO_histo . Fill(calorimetry_BGO);
      calorimetry_LaBr_histo. Fill(calorimetry_LaBr);
      calorimetry_NaI_histo . Fill(calorimetry_NaI);

      for (int hit_i = 0; hit_i<event.mult; hit_i++)
      {
        auto const & label = event.labels[hit_i];
        auto const & nrj   = event.nrjs  [hit_i];

        if (hit_i == Ge_hit_i) continue;

        if (isGe[label]) Ge_VS_smeared_calorimetry.Fill(smeared_calorimetry, nrj);

        spectra_trigger.Fill(label, nrj);

             if (isGe[label])    {spectrum_Ge_trigger   .Fill(nrj);}
        else if (isBGO[label])   {spectrum_BGO_trigger        .Fill(nrj);}
        else if (isParis[label])
        {
          if (isNaI[hit_i]) {spectrum_NaI_trigger  .Fill(nrj);}
          else              {spectrum_LaBr3_trigger.Fill(nrj);}
        }

        for (int hit_j = hit_i+1; hit_j<event.mult; hit_j++)
        {
          auto const & label_j = event.labels[hit_j];
          auto const & nrj_j = event.nrjs[hit_j];

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
      smeared_calorimetry_multiple_cascades+=smeared_calorimetry;
      if (nb_cascades == 2)
      {
        auto const calo = smeared_calorimetry_multiple_cascades;
        spectrum_smeared_calorimetry_double_peak.Fill(calo);
        nb_cascades=0;
        smeared_calorimetry_multiple_cascades = 0.0;
      }
    }

  }
}

void CobaltCalorimeter::Analyse()
{
  // Calculate the pic/total of the calorimeter
  print("----------------");
  print("");
  auto calo_spectrum = spectrum_smeared_calorimetry.Merge();
  if (!calo_spectrum) return;
  print("total efficiency :", calo_spectrum->Integral()/nb_trigg, calo_spectrum->Integral(), "/", nb_trigg);
  
  PeakFitter pe(calo_spectrum, 600, 2000);
  pe->Draw("same");
  auto background(pe.getBackground());
  auto const & integral_peak = pe->Integral(600, 2000);
  auto const & background_peak = background->Integral(600, 2000);
  print(integral_peak, (integral_peak-background_peak)/nb_trigg);
  print("----------------");
}

void CobaltCalorimeter::write()
{
  File outfilename(m_outfilename);
  outfilename.setExtension("root");
  auto outfile(TFile::Open(outfilename.c_str(), "RECREATE"));
  if (!outfile) throw_error(concatenate("Can't create outfile ", outfilename.c_str()).c_str());

  outfile->cd();
  // labels.Write();
  labels2D_trigger.Write();

  spectrum_Ge.Write();
  spectrum_BGO.Write();
  spectrum_LaBr3.Write();
  spectrum_NaI.Write();

  spectrum_Ge_trigger.Write();
  spectrum_BGO_trigger.Write();
  spectrum_LaBr3_trigger.Write();
  spectrum_NaI_trigger.Write();

  spectra_NaI_trigger.Write();
  spectra_trigger.Write();

  calorimetry_Ge_histo.Write();
  calorimetry_LaBr_histo.Write();
  calorimetry_BGO_histo.Write();
  calorimetry_NaI_histo.Write();
  spectrum_calorimetry.Write();
  spectrum_smeared_calorimetry->Write();
  spectrum_smeared_calorimetry_double_peak.Write();
  spectrum_smeared_calorimetry_VS_Multiplicity.Write();

  Ge_Ge.Write();
  Ge_BGO.Write();
  Ge_LaBr.Write();
  Ge_NaI.Write();
  Ge_Ge_trigger.Write();
  Ge_BGO_trigger.Write();
  Ge_LaBr_trigger.Write();
  Ge_NaI_trigger.Write();
  Ge_VS_smeared_calorimetry.Write();

  paris_pid.Write();

  outfile->Write();
  outfile->Close();
  print(outfilename, "written");
}





//////////////////////////////////////
//            DEPRECATED            //
//////////////////////////////////////





/// @brief Deprecated @deprecated
void CobaltCalorimeter::work(Hit & hit, Alignator & reader)
{
  auto const & nb_hits = reader->GetEntries();
  TRandom* random = new TRandom();
  random->SetSeed(time(0));

  for(int hit_i = 0; hit_i<nb_hits; ++hit_i)
  {
    reader->GetEntry(hit_i);
    if ( parisPiD(hit) ) continue;
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
        ++nb_trigg;
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
          if (parisPiD(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

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
          if (parisPiD(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

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
              ++nb_trigg;
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
                if (parisPiD(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

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
                if (parisPiD(hit)) spectra_NaI_trigger.Fill(hit.label, hit.nrj);

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
              spectrum_smeared_calorimetry_double_peak.Fill(smeared_calorimetry+smeared_calorimetry2);
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