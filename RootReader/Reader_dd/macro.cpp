#include "../../lib/Classes/Detectors.hpp"
#include "../../lib/Classes/Event.hpp"
#include "../../lib/Classes/RF_Manager.hpp"
#include "../../lib/Classes/FilesManager.hpp"
// #include "../../lib/Analyse/SpectraCo.hpp"
#include "../../lib/Analyse/Clovers.hpp"
#include "../../lib/MTObjects/MTTHist.hpp"
#include "../../lib/Classes/RWMat.hxx"
#include "../../lib/MTObjects/MTList.hpp"

std::string g_outfilename = "all_runs_test.root";

float smear(float const & nrj, Label const & label, TRandom* random)
{
  if (isGe[label])    return random->Gaus(nrj, nrj*0.05);
  if (isParis[label]) return random->Gaus(nrj, nrj*0.1);
  else return nrj;
}

class Analysator
{
public:
  Analysator(int const & number_files, std::string const & datapath = "~/nuball2/N-SI-136-root_dd/")
  {
    m_files.addFolder(Path(datapath).string(), number_files);
    MTfiles = m_files.getListFiles();
    this->Initialise();
    MTObject::parallelise_function(analyse_t, *this);
    write();
  }

  void Initialise();
  void analyse();
  void write();

private:
  /// @brief static 
  static void analyse_t(Analysator & analysator) {analysator.analyse();}

  TRandom* random = new TRandom();
  FilesManager m_files;
  MTList MTfiles;

  // Histograms :

  MTTHist<TH1F> prompt_Ge;
  MTTHist<TH1F> delayed_Ge;
  MTTHist<TH1F> prompt_BGO;
  MTTHist<TH1F> delayed_BGO;
  MTTHist<TH1F> prompt_Paris;
  MTTHist<TH1F> delayed_Paris;
  MTTHist<TH1F> prompt_calo;
  MTTHist<TH1F> delayed_calo;

  MTTHist<TH2F> prompt_delayed_calo;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo;
  MTTHist<TH2F> delayed_Ge_VS_prompt_calo;

  MTTHist<TH1F> delayed_Ge_wp;
  MTTHist<TH1F> delayed_calo_wp;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wp;

  MTTHist<TH1F> delayed_Ge_wpp;
  MTTHist<TH1F> delayed_calo_wpp;
  MTTHist<TH1F> prompt_Ge_wpp;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wpp;

  MTTHist<TH1F> delayed_Ge_wppE;
  MTTHist<TH1F> delayed_calo_wppE;
  MTTHist<TH2F> delayed_Ge_VS_delayed_calo_wppE;

  MTTHist<TH2F> pp;
  MTTHist<TH2F> dd;
};

void Analysator::Initialise()
{
  
  detectors.load("index_129.list");
  random->SetSeed(time(0));


  prompt_Ge.reset("prompt_Ge" , "prompt Ge;E[keV]" , 20000,0,10000);
  delayed_Ge.reset("delayed_Ge", "delayed Ge;E[keV]", 20000,0,10000);
  prompt_BGO.reset("prompt_BGO" , "prompt BGO;E[keV]" , 2000,0,10000);
  delayed_BGO.reset("delayed_BGO", "delayed BGO;E[keV]", 2000,0,10000);
  prompt_Paris.reset("prompt_Paris" , "prompt Paris;E[keV]" , 5000,0,10000);
  delayed_Paris.reset("delayed_Paris", "delayed Paris;E[keV]", 5000,0,10000);

  // Calorimetry
  prompt_calo.reset("prompt_calo" , "prompt calorimetry;E[keV]" , 5000,0,10000);
  delayed_calo.reset("delayed_calo", "delayed calorimetry;E[keV]", 5000,0,10000);
  prompt_delayed_calo.reset("prompt_delayed_calo", "Delayed VS prompt calorimetry;Prompt Calorimetry[keV];Delayed Calorimetry[keV]", 
      1000,0,20000, 1000,0,20000);
  delayed_Ge_VS_delayed_calo.reset("delayed_Ge_VS_delayed_calo", "Delayed Ge VS delayed calorimetry;Delayed Calorimetry[keV];E[keV]", 
      500,0,10000, 5000,0,10000);
  delayed_Ge_VS_prompt_calo.reset("delayed_Ge_VS_prompt_calo", "Delayed Ge VS prompt calorimetry;Prompt Calorimetry[keV];E[keV]", 
      20000,0,10000, 500,0,20000);

  // Prompt condition
  delayed_Ge_wp.reset("delayed_Ge_wp", "delayed Ge with prompt condition;E[keV]", 20000,0,10000);
  delayed_calo_wp.reset("delayed_calo_wp", "delayed calo with prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wp.reset("delayed_Ge_VS_delayed_calo_wp", "delayed calo with prompt condition;E[keV]", 
      500,0,20000, 10000,0,10000);

  // Prompt particle condition
  prompt_Ge_wpp.reset("prompt_Ge_wpp", "prompt Ge with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_wpp.reset("delayed_Ge_wpp", "delayed Ge with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_calo_wpp.reset("delayed_calo_wpp", "delayed calo with particle prompt condition;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wpp.reset("delayed_Ge_VS_delayed_calo_wpp", "delayed calo with particle prompt condition;E[keV]", 
      500,0,20000, 10000,0,10000);

  // Prompt particle energy condition
  delayed_Ge_wppE.reset("delayed_Ge_wppE", "delayed Ge with particle prompt with E<5MeV;E[keV]", 20000,0,10000);
  delayed_calo_wppE.reset("delayed_calo_wppE", "delayed calo with particle prompt with E<5MeV;E[keV]", 20000,0,10000);
  delayed_Ge_VS_delayed_calo_wppE.reset("delayed_Ge_VS_delayed_calo_wppE", "delayed calo with particle prompt with E<5MeV;E[keV]", 
      500,0,20000, 10000,0,10000);


  // Final bidims
  pp.reset("pp", "pp;E[keV];E[keV]", 3096,0,3096, 3096,0,3096);
  dd.reset("dd", "dd;E[keV];E[keV]", 3096,0,3096, 3096,0,3096);
}

void Analysator::analyse()
{
  std::string _file;
  while(MTfiles.getNext(_file))
  {
    File file(_file);
    if (!found(file.string(), "run")) continue;
    int run_number = std::stoi(getList(file.shortName(), '_')[1]);

    auto tfile (TFile::Open(file.c_str(), "READ"));
    tfile->cd();
    auto tree (tfile->Get<TTree>("Nuball2"));

    print("reading from", file);

    Event event(tree);
    RF_Manager rf;
    // Clovers clovers;

    auto const & nb_events = tree->GetEntries();
    for (int event_i = 0; event_i<nb_events; event_i++)
    {
      if(event_i%int_cast(10.e+6) == 0) print(event_i/1.e+6, "Mevts");

      tree->GetEntry(event_i);
      // clovers.Reset();

      float totalE_prompt = 0;
      float totalE_delayed = 0;
      float energyDSSD = 0;

      // Some triggers :
      bool has_prompt = false;
      bool has_prompt_particle = false;
      bool has_prompt_particle_E5 = false; // Particle prompt with energy<5MeV

      if (event.size()==1) continue;
      
      for (int hit_i = 0; hit_i<event.size(); hit_i++)
      {
        // clovers.Fill(event, hit_i);
        if (rf.setEvent(event)) continue;
        auto const & label = event.labels[hit_i];
        auto const & nrj   = event.nrjs  [hit_i];
        auto const & nrj2  = event.nrj2s [hit_i];
        auto const & time  = event.times [hit_i];

        if (nrj<5) continue;
        
        float const & time_ns = time/1000.f;

        // First attempt : rejecting all the previous prompt hits
        if (time_ns<-20) continue;
        if (time_ns<5) 
        {
          has_prompt = true;
          if (isDSSD[label]) 
          {
            has_prompt_particle = true;
            if (isSector[label])
            {
              energyDSSD = nrj;
              if (nrj<5000) has_prompt_particle_E5 = true;
            }
          }
        }

        // Calculate calorimetry :
        if (isParis[label] || isBGO[label] || isGe[label]) 
        {
          if(time_ns<5) totalE_prompt+=smear(nrj, label, random);
          else if(time_ns>20) totalE_delayed+=smear(nrj, label, random);
          // print(nrj,smear(nrj, label, random), totalE_prompt, totalE_delayed);
          // pauseCo();
        }

        if (isGe[label])
        {
          if (time_ns<20) prompt_Ge.Fill(nrj);
          else
          {
            delayed_Ge.Fill(nrj);
            if (has_prompt) delayed_Ge_wp.Fill(nrj);
            if (has_prompt_particle) delayed_Ge_wpp.Fill(nrj);
            if (has_prompt_particle_E5) delayed_Ge_wppE.Fill(nrj);
          }
        }
        else if (isParis[label])
        {
          if (time_ns<5) prompt_Paris.Fill(nrj);
          else if (time_ns>20) delayed_Paris.Fill(nrj);
        }
        else if (isBGO[label])
        {
          if(time_ns<5) prompt_BGO.Fill(nrj);
          else if(time_ns>20) delayed_BGO.Fill(nrj);
        }
      }
      
      // clovers.Analyse();

      if (totalE_prompt>5) 
      {
        prompt_calo.Fill(totalE_prompt);
      }

      if (totalE_delayed>5)
      {
        delayed_calo.Fill(totalE_delayed);
        if(has_prompt) delayed_calo_wp.Fill(totalE_delayed);
        if(has_prompt_particle) delayed_calo_wpp.Fill(totalE_delayed);
        if(has_prompt_particle_E5) delayed_calo_wppE.Fill(totalE_delayed);
        // for (size_t hit_i = 0; hit_i<clovers.CleanGe.size(); hit_i++)
        for (size_t hit_i = 0; hit_i<event.size(); hit_i++) 
        {
          if (isGe[event.labels[hit_i]] && event.times[hit_i]>20000)
          {
            delayed_Ge_VS_delayed_calo.Fill(totalE_delayed, event.nrjs[hit_i]);
            if(has_prompt) delayed_Ge_VS_delayed_calo_wp.Fill(totalE_delayed, event.nrjs[hit_i]);
            if(has_prompt_particle) delayed_Ge_VS_delayed_calo_wpp.Fill(totalE_delayed, event.nrjs[hit_i]);
            if(has_prompt_particle_E5) delayed_Ge_VS_delayed_calo_wppE.Fill(totalE_delayed, event.nrjs[hit_i]);
          }
        }
      }

      if (totalE_prompt>5 && totalE_delayed>5) 
      {
        prompt_delayed_calo.Fill(totalE_prompt, totalE_delayed);
        for (size_t hit_i = 0; hit_i<event.size(); hit_i++) if (isGe[event.labels[hit_i]])
        {
          if (totalE_prompt<5000 && totalE_delayed < 2800) 
          {
            if (isGe[event.labels[hit_i]]) 
            {
              for (size_t hit_j = 0; hit_j<event.size(); hit_j++) if(isGe[event.labels[hit_j]])
              {
                dd.Fill(event.nrjs[hit_i], event.nrjs[hit_j]);
                dd.Fill(event.nrjs[hit_j], event.nrjs[hit_i]);
              }
            }
          }
        }
      }
    }
    tfile->Close();
  }
}

void Analysator::write()
{
  RWMat RW_dd(dd); RW_dd.Write();
  auto outfile(TFile::Open(g_outfilename.c_str(), "RECREATE"));
  outfile->cd();

  prompt_Ge.Write();
  delayed_Ge.Write();
  prompt_BGO.Write();
  delayed_BGO.Write();
  prompt_Paris.Write();
  delayed_Paris.Write();
  prompt_calo.Write();
  delayed_calo.Write();

  prompt_delayed_calo.Write();
  print(delayed_Ge_VS_delayed_calo.Integral());
  delayed_Ge_VS_prompt_calo.Write();

  delayed_Ge_wp.Write();
  delayed_calo_wp.Write();

  delayed_Ge_wpp.Write();
  delayed_calo_wpp.Write();
  prompt_Ge_wpp.Write();

  delayed_Ge_wppE.Write();
  delayed_calo_wppE.Write();

  delayed_Ge_VS_delayed_calo.Write();
  delayed_Ge_VS_delayed_calo_wp.Write();
  delayed_Ge_VS_delayed_calo_wpp.Write();
  delayed_Ge_VS_delayed_calo_wppE.Write();

  dd.Write();

  outfile->Write();
  outfile->Close();
  print(g_outfilename, "written");
}

void macro(int number_files = -1)
{
  MTObject::Initialize(2);
  Analysator analysator(number_files);
}

#ifndef __CINT__
int main(int argc, char** argv)
{
  macro((argc>1) ? std::stoi(argv[1]) : -1);
  return 1;
}
#endif //__CINT__