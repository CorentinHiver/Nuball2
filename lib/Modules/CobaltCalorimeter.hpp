#ifndef COBALTECALORIMETER_HPP
#define COBALTECALORIMETER_HPP

#include "../Classes/Detectors.hpp"
#include "../Classes/Calibration.hpp"
#include "../Classes/Event.hpp"
#include "../Modules/Timeshifts.hpp"
#include "../MTObjects/MTFasterReader.hpp"
#include "../MTObjects/MTTHist.hpp"

class CobaltCalorimeter
{
public:
  CobaltCalorimeter() noexcept = default;
  void loadID(std::string const & filename) {m_IDfilename = filename;}
  void loadCalibration(Calibration calib) {m_calib = calib;}
  void loadTimeshifts(Timeshifts timeshifts) {m_timeshifts = timeshifts;}
  void setTimewindow_ns(Time_ns const & time_ns) {m_timewindow = Time_cast(time_ns*1000);}

  void launch(std::string const & foldername, int nb_files = -1);
  void Initialise();
  void work(Hit & hit, Alignator & reader);

private:
  static void dispatch_work(Hit & hit, Alignator & reader, CobaltCalorimeter & cb) {cb.work(hit, reader);} 
  void write();
  void Analyse();
  bool isNaI(Hit & hit);

  // Some parameters :
  std::string m_IDfilename;
  Time m_timewindow = 100000;
  Calibration m_calib;
  Timeshifts m_timeshifts;
  std::string m_outfilename = "CobaltCalorimetry.root";

  // Internal variables :
  int nb_1772 = 0;

  // Histograms :
  MTTHist<TH1F> spectrum_Ge;
  MTTHist<TH1F> spectrum_BGO;
  MTTHist<TH1F> spectrum_LaBr3;
  MTTHist<TH2F> spectrum_NaI;
  MTTHist<TH1F> spectra_trigger;
  MTTHist<TH1F> spectra_calorimetry;
  MTTHist<TH1F> spectra_smeared_calorimetry;
  MTTHist<TH1F> spectra_smeared_calorimetry_double_peak;

  MTTHist<TH2F> timing_VS_trigger;
  MTTHist<TH2F> timing_VS_ref;
  MTTHist<TH2F> nrj_each;
  MTTHist<TH2F> nrj_each_trigger;
  MTTHist<TH1F> labels;
  MTTHist<TH2F> labels2D;

  MTTHist<TH1F> paris_pid;

  MTTHist<TH2F> Ge_Ge;
};

void CobaltCalorimeter::Initialise()
{
  MTObject::Initialize();

  spectrum_Ge     . reset("spectrum_Ge", "Ge spectra;Energy [keV]", 4000,0,2000);
  spectrum_BGO     . reset("spectrum_BGO", "BGO spectra;Energy [keV]", 500,0,2000);
  spectrum_LaBr3     . reset("spectrum_LaBr3", "LaBr3 spectra;Energy [keV]", 1000,0,2000);
  spectrum_NaI     . reset("spectrum_NaI", "NaI spectra;Energy [keV]", 1000,0,1000, 100,0,2000);
  spectra_trigger     . reset("spectra_trigger", "Trigger spectra;Energy [keV]", 4000,0,2000);
  spectra_calorimetry . reset("spectra_calorimetry", "Calorimeter spectra;Energy calorimeter[keV]", 900, 1, 3000);
  spectra_smeared_calorimetry . reset("spectra_smeared_calorimetry", "Smeared calorimeter spectra;Energy calorimeter[keV]", 2999, 1, 3000);
  
  spectra_smeared_calorimetry_double_peak . reset("spectra_smeared_calorimetry_double_peak", "Smeared calorimeter spectra 2 cascades;Energy calorimeter[keV]", 3999, 1, 4000);

  timing_VS_trigger.reset("timing_VS_trigger", "timing_VS_trigger;label;time[ps]", 1000,0,1000, 200,-m_timewindow/2,m_timewindow/2);
  timing_VS_ref.reset("timing_VS_ref", "timing_VS_ref;label;time[ps]", 1000,0,1000, 200,-m_timewindow/2,m_timewindow/2);
  nrj_each.reset("nrj_each", "nrj_each;label;Energy[keV];", 1000,0,1000, 3000,0,3000);
  nrj_each_trigger.reset("nrj_each_trigger", "nrj_each_trigger;label;Energy[keV];", 1000,0,1000, 3000,0,3000);
  labels.reset("labels", "labels;label", 1000, 0, 1000);
  labels2D.reset("labels2D", "labels2D;label trigger;label others", 1000,0,1000, 1000,0,1000);

  paris_pid.reset("paris_pid", "paris_pid;(long-short)/long", 500,-2,3);

  Ge_Ge.reset("Ge_Ge", "Ge Ge;Energy [keV];Energy [keV]", 2000,0,2000, 2000,0,2000);
}

void CobaltCalorimeter::launch(std::string const & foldername, int nb_files)
{
  if (!detectors) detectors.load(m_IDfilename);
  if (!detectors) throw Detectors::Error();

  this -> Initialise();

  MTFasterReader reader(foldername, nb_files);
  reader.setTimeshifts(m_timeshifts.data());
  reader.readAligned(dispatch_work, *this);

  this->Analyse();
  this->write();
}

bool CobaltCalorimeter::isNaI(Hit & hit) 
{
  if (!isParis[hit.label] ) return false;
  debug(hit);
  debug((hit.qdc2-hit.adc)/double_cast(hit.qdc2));
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

void CobaltCalorimeter::work(Hit & hit, Alignator & reader)
{
  auto const & nb_hits = reader->GetEntries();
  TRandom* random = new TRandom();
  random->SetSeed(time(0));

  for(int hit_i = 0; hit_i<nb_hits; ++hit_i)
  {
    reader->GetEntry(hit_i);
    isGe[hit.label]
  }

  for(int hit_i = 0; hit_i<nb_hits; ++hit_i)
  {
    reader->GetEntry(hit_i);
    if ( isNaI(hit) ) continue;
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
        ++nb_1772;
        spectra_trigger.Fill(hit.nrj);
        while(--hit_i > 0)
        {
          reader->GetEntry(hit_i);
          auto const & dT = Time_cast(stamp_Ge-hit.stamp);
          if (dT>m_timewindow) break;
          if (isNaI(hit)) spectrum_NaI.Fill(hit.label, hit.nrj);
          labels2D.Fill(label_Ge, hit.label);
          labels.Fill(hit.label);
          m_calib(hit); calibBGO(hit);
          timing_VS_trigger.Fill(hit.label, dT);

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
          if (isNaI(hit)) spectrum_NaI.Fill(hit.label, hit.nrj);
          labels2D.Fill(label_Ge, hit.label);
          labels.Fill(hit.label);
          m_calib(hit); calibBGO(hit);
          timing_VS_trigger.Fill(hit.label, dT);

               if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
          else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
          else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
          nrj_each_trigger.Fill(hit.label, hit.nrj);

          calorimetry+=hit.nrj;
          calo_BGO+=hit.nrj;
          smeared_calorimetry+=smear(hit.nrj, hit.label, random);
        }
        --hit_i; // Starting next loop with the hit that closed the event
        spectra_calorimetry.Fill(calorimetry);
        spectra_smeared_calorimetry.Fill(smeared_calorimetry);

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
              ++nb_1772;
              spectra_trigger.Fill(hit.nrj);
              while(--hit_i > 0)
              {
                reader->GetEntry(hit_i);
                auto const & dT = Time_cast(stamp_Ge-hit.stamp);
                if (dT>m_timewindow) break;
                if (isNaI(hit)) spectrum_NaI.Fill(hit.label, hit.nrj);
                labels2D.Fill(label_Ge, hit.label);
                labels.Fill(hit.label);
                m_calib(hit); calibBGO(hit);
                timing_VS_trigger.Fill(hit.label, dT);

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
                if (isNaI(hit)) spectrum_NaI.Fill(hit.label, hit.nrj);
                labels2D.Fill(label_Ge, hit.label);
                labels.Fill(hit.label);
                m_calib(hit); calibBGO(hit);
                timing_VS_trigger.Fill(hit.label, dT);

                     if (isGe[hit.label]) spectrum_Ge.Fill(hit.nrj);
                else if (isBGO[hit.label]) spectrum_BGO.Fill(hit.nrj);
                else if (isParis[hit.label]) spectrum_LaBr3.Fill(hit.nrj);
                nrj_each_trigger.Fill(hit.label, hit.nrj);

                calorimetry2+=hit.nrj;
                calo_BGO2+=hit.nrj;
                smeared_calorimetry2+=smear(hit.nrj, hit.label, random);
              }
              --hit_i; // Starting next loop with the hit that closed the event
              spectra_calorimetry.Fill(calorimetry2);
              spectra_smeared_calorimetry.Fill(smeared_calorimetry2);
              spectra_smeared_calorimetry_double_peak.Fill(smeared_calorimetry+smeared_calorimetry2);
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

void CobaltCalorimeter::Analyse()
{
  // Calculate the pic/total of the calorimeter
  print("----------------");
  print("");
  auto calo_spectrum = spectra_smeared_calorimetry.Merge();
  if (!calo_spectrum) return;
  print("total efficiency :", calo_spectrum->Integral()/nb_1772, calo_spectrum->Integral(), "/", nb_1772);
  
  PeakFitter pe(calo_spectrum, 600, 2000);
  pe->Draw("same");
  auto background(pe.getBackground());
  auto const & integral_peak = pe->Integral(600, 2000);
  auto const & background_peak = background->Integral(600, 2000);
  print(integral_peak, (integral_peak-background_peak)/nb_1772);
  print("----------------");
}

void CobaltCalorimeter::write()
{
  File outfilename(m_outfilename);
  outfilename.setExtension("root");
  auto outfile(TFile::Open(outfilename.c_str(), "RECREATE"));
  if (!outfile) throw_error(concatenate("Can't create outfile ", outfilename.c_str()).c_str());

  outfile->cd();
  labels.Write();
  labels2D.Write();
  spectrum_Ge.Write();
  spectrum_BGO.Write();
  spectrum_LaBr3.Write();
  spectrum_NaI.Write();
  spectra_trigger.Write();
  spectra_calorimetry.Write();
  spectra_smeared_calorimetry->Write();
  spectra_smeared_calorimetry_double_peak.Write();
  timing_VS_trigger.Write();
  timing_VS_ref.Write();
  nrj_each.Write();
  nrj_each_trigger.Write();
  paris_pid.Write();
  outfile->Write();
  outfile->Close();
  print(outfilename, "written");
}

#endif //COBALTECALORIMETER_HPP