#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/SimpleParis.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

long max_cursor = -1;

void macro_EndRuns()
{
  Timer timer;

  PhoswitchCalib calibPhoswitches("../136/NaI_136_2024.angles");

  FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/60Co_center_after_calib/");
  // FilesManager files(Path::home().string()+"nuball2/N-SI-136-sources/end_runs_2/");
  MTList MTfiles(files.get());
  MTObject::Initialise(7);
  // MTObject::Initialise(1);
  MTObject::adjustThreadsNumber(files.size());
  CoefficientCorrection calibGe("../136/GainDriftCoefficients.dat");
  if (!calibGe) return;
  
  MTObject::parallelise_function([&]()
  {
    std::string filename;
    auto const & thread_i_str = std::to_string(MTObject::getThreadIndex());
    while(MTfiles>>filename)
    {
      std::string file_shortname = rmPathAndExt(filename);
      Nuball2Tree tree(filename);
      Event event(tree);
      CloversV2 clovers;
      SimpleParis paris(&calibPhoswitches);

      unique_TH1F singles(new TH1F(("singles_"+thread_i_str).c_str(),"singles",10000,0,10000));
      unique_TH2F singles_VS_ring_clover(new TH2F(("singles_VS_ring_clover_"+thread_i_str).c_str(),"singles_VS_ring_clover", 10,-2,8, 10000,0,10000));
      unique_TH1F rejected(new TH1F(("rejected_"+thread_i_str).c_str(),"rejected",10000,0,10000));
      unique_TH1F pure_singles(new TH1F(("pure_singles_"+thread_i_str).c_str(),"pure_singles",10000,0,10000));
      unique_TH2F gg(new TH2F(("gg_"+thread_i_str).c_str(),"gg",4096,0,4096, 4096,0,4096));
      unique_TH2F g_time(new TH2F(("g_time_"+thread_i_str).c_str(),"g_time;[keV];hours",4096,0,4096, 10000,0,2));
      unique_TH2F sumC2_time(new TH2F(("sumC2_time_"+thread_i_str).c_str(),"sumC2_time;[keV];hours",4096,0,4096, 10000,0,2));
      unique_TH2F ggC2(new TH2F(("ggC2_"+thread_i_str).c_str(),"ggC2",4096,0,4096, 4096,0,4096));
      unique_TH2F g_VS_sumC2(new TH2F(("g_VS_sumC2_"+thread_i_str).c_str(),"g_VS_sumC2",4096,0,2*4096, 4096,0,4096));
      unique_TH2F ggC3(new TH2F(("ggC3_"+thread_i_str).c_str(),"ggC3",4096,0,4096, 4096,0,4096));

      unique_TH1F singles_phoswitch(new TH1F(("singles_phoswitch_"+thread_i_str).c_str(),"singles_phoswitch",10000,0,10000));
      unique_TH1F clean_phoswitch(new TH1F(("clean_phoswitch_"+thread_i_str).c_str(),"clean_phoswitch",10000,0,10000));
      unique_TH1F rejected_phoswitch(new TH1F(("rejected_phoswitch_"+thread_i_str).c_str(),"rejected_phoswitch",10000,0,10000));
      unique_TH1F clean_LaBr3(new TH1F(("clean_LaBr3_"+thread_i_str).c_str(),"clean_LaBr3",10000,0,10000));
      unique_TH1F rejected_LaBr3(new TH1F(("rejected_LaBr3_"+thread_i_str).c_str(),"rejected_LaBr3",10000,0,10000));
      unique_TH1F mult_paris(new TH1F(("mult_paris_"+thread_i_str).c_str(),"mult_paris",20,0,20));
      
      unique_TH1F singles_modules(new TH1F(("singles_modules_"+thread_i_str).c_str(),"singles_modules",10000,0,10000));
      
      unique_TH2F Ge_VS_phoswitch(new TH2F(("Ge_VS_phoswitch_"+thread_i_str).c_str(),"Ge_VS_phoswitch",5000,0,10000, 5000,0,10000));
      unique_TH2F Ge_VS_clean_phoswitch(new TH2F(("Ge_VS_clean_phoswitch_"+thread_i_str).c_str(),"Ge_VS_clean_phoswitch",5000,0,10000, 5000,0,10000));
      unique_TH2F Ge_VS_clean_LaBr3(new TH2F(("Ge_VS_clean_LaBr3_"+thread_i_str).c_str(),"Ge_VS_clean_LaBr3",5000,0,10000, 5000,0,10000));
      unique_TH2F Ge_VS_paris_modules(new TH2F(("Ge_VS_paris_modules_"+thread_i_str).c_str(),"Ge_VS_paris_modules",5000,0,10000, 5000,0,10000));

      // Calorimetry
      unique_TH1F calo_hist(new TH1F(("calo_hist_"+thread_i_str).c_str(),"calo_hist",1000,0,10000));
      unique_TH2F gamma_VS_calo(new TH2F(("gamma_VS_calo_"+thread_i_str).c_str(),"gamma_VS_calo",500,0,10000, 10000,0,10000));
      unique_TH2F calo_VS_index(new TH2F(("calo_VS_index_"+thread_i_str).c_str(),"calo_VS_index",50,0,50, 500,0,10000));
      unique_TH1F calo_clover_hist(new TH1F(("calo_clover_hist_"+thread_i_str).c_str(),"calo_clover_hist",1000,0,10000));
      unique_TH1F calo_paris_hist(new TH1F(("calo_paris_hist_"+thread_i_str).c_str(),"calo_paris_hist",1000,0,10000));
      
      unique_TH1F calo_1350_bckg_gate(new TH1F(("calo_1350_bckg_gate_"+thread_i_str).c_str(),"calo_1350_bckg_gate",500,0,5000));
      unique_TH1F calo_1368_gate(new TH1F(("calo_1368_gate_"+thread_i_str).c_str(),"calo_1368_gate",500,0,5000));
      unique_TH1F calo_2754_gate(new TH1F(("calo_2754_gate_"+thread_i_str).c_str(),"calo_2754_gate",500,0,5000));
      unique_TH1F calo_bckg_2780_gate(new TH1F(("calo_bckg_2780_gate_"+thread_i_str).c_str(),"calo_bckg_2780_gate",500,0,5000));

      unique_TH1F calo_1333_gate(new TH1F(("calo_1333_gate_"+thread_i_str).c_str(),"calo_1333_gate",500,0,5000));
      unique_TH1F calo_bckg_1343_gate(new TH1F(("calo_bckg_1343_gate_"+thread_i_str).c_str(),"calo_bckg_1343_gate",500,0,5000));
      unique_TH1F calo_1172_gate(new TH1F(("calo_1172_gate_"+thread_i_str).c_str(),"calo_1172_gate",500,0,5000));
      unique_TH1F calo_bckg_2780_gate(new TH1F(("calo_bckg_2780_gate_"+thread_i_str).c_str(),"calo_bckg_2780_gate",500,0,5000));

      while(tree.readNext())
      {
        clovers.clear();
        paris.clear();
        if (tree.cursor()%(int)(1.e+6) == 0) 
        {
          printC(nicer_double(tree.cursor(), 0), "hits");
          if (max_cursor>0 && tree.cursor() > max_cursor) break;
        }

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          // auto const & time =  event.times[hit_i];
          auto const & nrj = event.nrjs[hit_i];
          // auto const & nrj2 = event.nrj2s[hit_i];
          if (CloversV2::isGe(label)) 
          {
            if ((find_key(CloversV2::maxE_Ge, label) && nrj>CloversV2::maxE_Ge.at(label))) continue;
            event.nrjs[hit_i] = calibGe.correct(nrj, 122, label);
          }

          if (nrj<20_keV) continue;
          
          clovers.fill(event, hit_i);
          paris.fill(event, hit_i);
        }

        clovers.analyze();
        paris.analyze();

        auto const & calo = clovers.calorimetryTotal+paris.calorimetry();
        auto const & mult = clovers.all.size()+paris.phoswitch_mult(); // To change to paris_module_mult when it works
        if (mult>1)
        {
          calo_hist->Fill(calo);
          if (clovers.all.size()    >0) calo_clover_hist->Fill(clovers.calorimetryTotal);
          if (paris.phoswitch_mult()>0) calo_paris_hist ->Fill(paris.calorimetry());
        }

        auto const absolute_time_h = double_cast(event.stamp)*1.e-12/3600.;

        auto const & Cmult = clovers.clean.size();
        for (size_t hit_i=0; hit_i<Cmult; ++hit_i)
        {
          auto const & clover_i = *(clovers.clean[hit_i]);
          auto const & nrj_i = clover_i.nrj;
          gamma_VS_calo->Fill(calo, nrj_i);
          calo_VS_index->Fill(clover_i.index(), calo);
          singles->Fill(nrj_i);
          g_time->Fill(nrj_i, absolute_time_h);
          singles_VS_ring_clover->Fill(clover_i.sub_ring(), nrj_i);
          if (Cmult == 1) pure_singles->Fill(nrj_i);
          for (size_t hit_j=hit_i+1; hit_j<clovers.clean.size(); ++hit_j)
          {
            auto const & clover_j = *(clovers.clean[hit_j]);
            auto const & nrj_j = clover_j.nrj;
            gg->Fill(nrj_i, nrj_j);
            gg->Fill(nrj_j, nrj_i);
            if (Cmult == 2)
            {
              sumC2_time->Fill(nrj_i+nrj_j, absolute_time_h);
              ggC2->Fill(nrj_i, nrj_j);
              ggC2->Fill(nrj_j, nrj_i);
              g_VS_sumC2->Fill(nrj_i+nrj_j, nrj_i);
              g_VS_sumC2->Fill(nrj_i+nrj_j, nrj_j);
            }
            if (Cmult == 3)
            {
              ggC3->Fill(nrj_i, nrj_j);
              ggC3->Fill(nrj_j, nrj_i);
            }
          }
          
          // Gated calorimetry :

          if (gate(1347, nrj_i, 1352))
          {
            double _calo = paris.calorimetry();
            for (auto const & clover : clovers) if (clover->index() != clover_i.index()) _calo+=clover->calo;
            calo_1350_bckg_gate->Fill(_calo);
          }
          if (gate(1367, nrj_i, 1372)) 
          {
            double _calo = paris.calorimetry();
            for (auto const & clover : clovers) if (clover->index() != clover_i.index()) _calo+=clover->calo;
            calo_1368_gate->Fill(_calo);
          }
          else if (gate(2722, nrj_i, 2728)) 
          {
            double _calo = paris.calorimetry();
            for (auto const & clover : clovers) if (clover->index() != clover_i.index()) _calo+=clover->calo;
            calo_bckg_2780_gate->Fill(_calo);
          }
          else if (gate(2752, nrj_i, 2758)) 
          {
            double _calo = paris.calorimetry();
            for (auto const & clover : clovers) if (clover->index() != clover_i.index()) _calo+=clover->calo;
            calo_2754_gate->Fill(_calo);
          }
        }

        for (size_t hit_i=0; hit_i<clovers.rejected.size(); ++hit_i) rejected->Fill(clovers.rejected[hit_i]->nrj);

        auto const & paris_phos_mult = paris.phoswitch_mult();
        mult_paris->Fill(paris_phos_mult);
        for (auto const phoswitch : paris.phoswitches)
        {
          singles_phoswitch->Fill(phoswitch->nrj);
          if (phoswitch->rejected) 
          {
            rejected_phoswitch->Fill(phoswitch->nrj);
            if(phoswitch->isLaBr3()) rejected_LaBr3->Fill(phoswitch->qshort);
          }
          else 
          {
            clean_phoswitch->Fill(phoswitch->nrj);
            if(phoswitch->isLaBr3()) clean_LaBr3->Fill(phoswitch->qshort);
          }
          for (auto const & clover : clovers.clean) 
          {
            Ge_VS_phoswitch->Fill(phoswitch->nrj, clover->nrj);
            if (!phoswitch->rejected) 
            {
              Ge_VS_clean_phoswitch->Fill(phoswitch->nrj, clover->nrj);
              if (phoswitch->isLaBr3()) Ge_VS_clean_LaBr3->Fill(phoswitch->nrj, clover->nrj);
            }
          }
        }
        for (auto const & module : paris.modules)
        {
          singles_modules->Fill(module->nrj);
          for (auto const & clover : clovers.clean) Ge_VS_paris_modules->Fill(module->nrj, clover->nrj);
        }

      }

      std::string out_filename = "data/60Co_center_after/"+file_shortname+".root";
      // std::string out_filename = "data/end_runs/"+file_shortname+".root";
      File Filename(out_filename); Filename.makePath();
      auto output(TFile::Open(Filename.c_str(), "recreate"));
      output->cd();

        singles->Write("singles", TObject::kOverwrite);
        singles_VS_ring_clover->Write("singles_VS_ring_clover", TObject::kOverwrite);
        g_time->Write("g_time", TObject::kOverwrite);
        sumC2_time->Write("sumC2_time", TObject::kOverwrite);
        pure_singles->Write("pure_singles", TObject::kOverwrite);
        rejected->Write("rejected", TObject::kOverwrite);
        gg->Write("gg", TObject::kOverwrite);
        ggC2->Write("ggC2", TObject::kOverwrite);
        g_VS_sumC2->Write("g_VS_sumC2", TObject::kOverwrite);
        ggC3->Write("ggC3", TObject::kOverwrite);

        singles_phoswitch->Write("singles_phoswitch", TObject::kOverwrite);
        clean_phoswitch->Write("clean_phoswitch", TObject::kOverwrite);
        rejected_phoswitch->Write("rejected_phoswitch", TObject::kOverwrite);
        clean_LaBr3->Write("clean_LaBr3", TObject::kOverwrite);
        rejected_LaBr3->Write("rejected_LaBr3", TObject::kOverwrite);
        mult_paris->Write("mult_paris", TObject::kOverwrite);
        
        singles_modules->Write("singles_modules", TObject::kOverwrite);

        Ge_VS_phoswitch->Write("Ge_VS_phoswitch", TObject::kOverwrite);
        Ge_VS_clean_phoswitch->Write("Ge_VS_clean_phoswitch", TObject::kOverwrite);
        Ge_VS_clean_LaBr3->Write("Ge_VS_clean_LaBr3", TObject::kOverwrite);
        Ge_VS_paris_modules->Write("Ge_VS_paris_modules", TObject::kOverwrite);
        
        calo_hist->Write("calo_hist", TObject::kOverwrite);
        calo_hist->Write("calo_hist", TObject::kOverwrite);
        gamma_VS_calo->Write("gamma_VS_calo", TObject::kOverwrite);
        calo_VS_index->Write("calo_VS_index", TObject::kOverwrite);
        calo_clover_hist->Write("calo_clover_hist", TObject::kOverwrite);
        calo_paris_hist->Write("calo_paris_hist", TObject::kOverwrite);
        calo_1350_bckg_gate->Write("calo_1350_bckg_gate", TObject::kOverwrite);
        calo_1368_gate->Write("calo_1368_gate", TObject::kOverwrite);
        calo_2754_gate->Write("calo_2754_gate", TObject::kOverwrite);
        calo_bckg_2780_gate->Write("calo_bckg_2780_gate", TObject::kOverwrite);

      output->Close();
      print(out_filename, "written");
    }
  });
  // print("hadd -d . -j 10 -f data/endruns.root data/end_runs/endruns136*");
  // system("hadd -d . -j 10 -f data/endruns.root data/end_runs/endruns136*");
  print("hadd -d . -j 10 -f data/60Co.root data/60Co_center_after/60Co136*");
  system("hadd -d . -j 10 -f data/60Co.root data/60Co_center_after/60Co136*");
  print(timer());
}

#ifndef __CINT__
int main(int argc, char** argv)
{
  if (argc == 2) max_cursor = int_cast(std::stod(argv[1]));
  macro_EndRuns();
  return 1;
}

#endif //__CINT__
// g++ -g -o exec macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o exec macro_EndRuns.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17







// unique_TH1F singles_paris_front(new TH1F(("singles_paris_front_"+thread_i_str).c_str(),"singles_paris_front",10000,0,10000));
// unique_TH2F singles_paris_front_VS_angle(new TH2F(("singles_paris_front_VS_angle_"+thread_i_str).c_str(),"singles_paris_front_VS_angle",200,-5,15, 10000,0,10000));
// unique_TH1F mult_paris_front(new TH1F(("mult_paris_front_"+thread_i_str).c_str(),"mult_paris_front",20,0,20));
// unique_TH1F singles_paris_back(new TH1F(("singles_paris_back_"+thread_i_str).c_str(),"singles_paris_back",10000,0,10000));
// unique_TH2F singles_paris_back_VS_angle(new TH2F(("singles_paris_back_VS_angle_"+thread_i_str).c_str(),"singles_paris_back_VS_angle",20,-5,15, 10000,0,10000));
// unique_TH1F mult_paris_back(new TH1F(("mult_paris_back_"+thread_i_str).c_str(),"mult_paris_back",20,0,20));
// unique_TH2F gg_paris(new TH2F(("gg_paris_"+thread_i_str).c_str(),"gg_paris",5000,0,10000, 5000,0,10000));
// unique_TH2F gg_paris_front(new TH2F(("gg_paris_front_"+thread_i_str).c_str(),"gg_paris_front",5000,0,10000, 5000,0,10000));
// unique_TH2F gg_paris_back(new TH2F(("gg_paris_back_"+thread_i_str).c_str(),"gg_paris_back",5000,0,10000, 5000,0,10000));
// unique_TH2F gg_paris_front_VS_back(new TH2F(("gg_paris_front_VS_back_"+thread_i_str).c_str(),"gg_paris_front_VS_back",5000,0,10000, 5000,0,10000));

// unique_TH2F Ge_VS_phoswitch_front(new TH2F(("Ge_VS_phoswitch_front_"+thread_i_str).c_str(),"Ge_VS_phoswitch_front",5000,0,10000, 5000,0,10000));
// unique_TH2F Ge_VS_phoswitch_back(new TH2F(("Ge_VS_phoswitch_back_"+thread_i_str).c_str(),"Ge_VS_phoswitch_back",5000,0,10000, 5000,0,10000));

// mult_paris_front->Fill(paris.front.module_mult);
// mult_paris_back->Fill(paris.back.module_mult);

// for (size_t fr_i = 0; fr_i<paris.front.module_mult; ++fr_i)
// {
//   auto const & fr_id_i = paris.front.modules_id[fr_i];
//   auto const & fr_module_i = paris.front.modules[fr_id_i];
// singles_phoswitch->Fill(fr_module_i.nrj);
//   singles_paris_front->Fill(fr_module_i.nrj);
//   singles_paris_front_VS_angle->Fill(fr_module_i.angle_to_beam(), fr_module_i.nrj);

//   for (size_t fr_j = fr_i+1; fr_j<paris.front.module_mult; ++fr_j)
//   {
//     auto const & fr_id_j = paris.front.modules_id[fr_j];
//     auto const & fr_module_j = paris.front.modules[fr_id_j];

//     gg_paris->Fill(fr_module_i.nrj, fr_module_j.nrj);
//     gg_paris->Fill(fr_module_j.nrj, fr_module_i.nrj);

//     gg_paris_front->Fill(fr_module_i.nrj, fr_module_j.nrj);
//     gg_paris_front->Fill(fr_module_j.nrj, fr_module_i.nrj);
//   }

//   for (size_t ba_j = 0; ba_j<paris.back.module_mult; ++ba_j)
//   {
//     auto const & ba_id_j = paris.back.modules_id[ba_j];
//     auto const & ba_module_j = paris.back.modules[ba_id_j];

//     gg_paris->Fill(fr_module_i.nrj, ba_module_j.nrj);
//     gg_paris->Fill(ba_module_j.nrj, fr_module_i.nrj);

//     gg_paris_front_VS_back->Fill(ba_module_j.nrj, fr_module_i.nrj);
//   }
// }

// for (size_t ba_i = 0; ba_i<paris.back.module_mult; ++ba_i)
// {
//   auto const & ba_id_i = paris.back.modules_id[ba_i];
//   auto const & ba_module_i = paris.back.modules[ba_id_i];

//   singles_phoswitch->Fill(ba_module_i.nrj);
//   singles_paris_back->Fill(ba_module_i.nrj);
//   singles_paris_back_VS_angle->Fill(ba_module_i.angle_to_beam(), ba_module_i.nrj);

//   for (size_t ba_j = ba_i+1; ba_j<paris.back.module_mult; ++ba_j)
//   {
//     auto const & ba_id_j = paris.back.modules_id[ba_j];
//     auto const & ba_module_j = paris.back.modules[ba_id_j];

//     gg_paris->Fill(ba_module_i.nrj, ba_module_j.nrj);
//     gg_paris->Fill(ba_module_j.nrj, ba_module_i.nrj);

//     gg_paris_back->Fill(ba_module_i.nrj, ba_module_j.nrj);
//     gg_paris_back->Fill(ba_module_j.nrj, ba_module_i.nrj);
//   }
// }


// singles_paris_front->Write("singles_paris_front", TObject::kOverwrite);
// singles_paris_front_VS_angle->Write("singles_paris_front_VS_angle", TObject::kOverwrite);
// mult_paris_front->Write("mult_paris_front", TObject::kOverwrite);
// singles_paris_back->Write("singles_paris_back", TObject::kOverwrite);
// singles_paris_back_VS_angle->Write("singles_paris_back_VS_angle", TObject::kOverwrite);
// mult_paris_back->Write("mult_paris_back", TObject::kOverwrite);
// gg_paris->Write("gg_paris", TObject::kOverwrite);
// gg_paris_front->Write("gg_paris_front", TObject::kOverwrite);
// gg_paris_back->Write("gg_paris_back", TObject::kOverwrite);
// gg_paris_front_VS_back->Write("gg_paris_front_VS_back", TObject::kOverwrite);
// Ge_VS_phoswitch_front->Write("Ge_VS_phoswitch_front", TObject::kOverwrite);
// Ge_VS_phoswitch_back->Write("Ge_VS_phoswitch_back", TObject::kOverwrite);