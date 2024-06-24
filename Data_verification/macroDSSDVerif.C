#include "../lib/MTObjects/MTObject.hpp"
#include "../lib/libRoot.hpp"
#include "../lib/Classes/FilesManager.hpp"
#include "../lib/Classes/Calibration.hpp"
#include "../lib/MTObjects/MTList.hpp"
#include "../lib/Analyse/CloversV2.hpp"
#include "../lib/Analyse/CloversV3.hpp"
#include "../lib/Analyse/WarsawDSSD.hpp"
#include "../lib/Classes/Nuball2Tree.hpp"
#include "../lib/Classes/Timer.hpp"
#include "../lib/Classes/CoProgressBar.hpp"

#include "CoefficientCorrection.hpp"
#include "Utils.h"

constexpr static bool kCalibGe = true;

class SpectraGate
{
  SpectraGate() noexcept = default;
  SpectraGate(std::initializer_list<size_t> gate_bin_sizes, std::initializer_list<size_t> const & gates)
    : m_gate_bin_sizes(gate_bin_sizes),
      m_gates(gates)
  {
    create();
  }
  SpectraGate(size_t gate_bin_size, std::initializer_list<size_t> const & gates) 
    : m_gates(gates)
  {
    for (size_t i = 0; i<m_gates.size(); ++i) m_gate_bin_sizes.push_back(gate_bin_size);
    create();
  }

  int operator() (int const & bin)
  {
    if (gate(m_gate_bin_min, size_cast(bin), m_gate_bin_max))
    {
      if (m_gate_lookup[bin]) return true;
      else return false;
    }
    else return false;
  }

  /// @brief Return the lookup value. Careful, you need to be sure it is in the range (i.e. if (gate(bin)) return gate.lut(bin);)
  int lut(int const & bin)
  {
    return m_id_gate_lkp[bin];
  }

private:
  void create()
  {
    // Get the minimum and maximum range of the gates. Handle the case where the vectors are not ordered (to do it before !)
    m_gate_bin_min = minimum(m_gates)-maximum(m_gate_bin_sizes);
    m_gate_bin_max = maximum(m_gates)+maximum(m_gate_bin_sizes);

    m_gate_lookup.reserve(m_gate_bin_max);
    m_id_gate_lkp.reserve(m_gate_bin_max);
    for (size_t gate_index = 0;  gate_index<m_gates.size(); ++gate_index)
    {
      auto const & gate_bin_size = m_gate_bin_sizes[gate_index];
      auto const & _gate = m_gates[gate_index];
      for (size_t temp_bin = 0; temp_bin<size_cast(m_gate_bin_max); ++temp_bin) 
      {
        if (temp_bin<_gate-gate_bin_size) 
        {
          m_gate_lookup.push_back(false);
          m_id_gate_lkp.push_back(0);
        }
        else if (temp_bin<_gate+gate_bin_size) 
        {
          m_gate_lookup.push_back(true);
          m_id_gate_lkp.push_back(gate_index);
        }
        else break;
      }
    }
  }

  std::vector<size_t> m_gate_bin_sizes;
  std::vector<size_t> m_gates;
  size_t m_gate_bin_min = 0;
  size_t m_gate_bin_max = 0;
  std::vector<bool> m_gate_lookup;
  std::vector<int> m_id_gate_lkp;
};

void macroDSSDVerif(int nb_files = -1, long nb_hits_read = 1.e+12, int nb_threads = 5)
{
  detectors.load("../136/index_129.list");
  std::string target = "U";
  std::string trigger = "P";
  Timer timer;
  std::atomic<int> files_total_size(0);
  std::atomic<int> total_evts_number(0);
  int freq_hit_display=(nb_hits_read < 2.e+7) ? 1.e+6 : 1.e+7;

  Path data_path("~/nuball2/N-SI-136-root_"+trigger+"/merged/");
  Calibration ring_calib("../136/DSSD_ring.calib");
  FilesManager files(data_path.string(), nb_files);
  MTList MTfiles(files.get());
  MTObject::setThreadsNb(nb_threads);
  MTObject::adjustThreadsNumber(files.size());
  MTObject::Initialise();


  ExcitationEnergy Ex("../136/U5_d_p_10umAl.Ex");
  CoefficientCorrection calibGe;
  
  std::mutex write_mutex;

  MTObject::parallelise_function([&](){
    std::string file;
    auto const & thread_i = MTObject::getThreadIndex();
    std::string thread_i_str = std::to_string(thread_i);
    while(MTfiles.getNext(file))
    {
      auto const & filename = removePath(file);
      // auto const & run_name = removeExtension(filename);
      int const & run_number = std::stoi(split(filename, '_')[1]);
      if (target == "Th" && run_number>74) continue;
      if (target == "U" && run_number<75) continue;
      Nuball2Tree tree(file.c_str());
      if (!tree) continue;

      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);

      std::string outFolder = "data/"+trigger+"/"+target+"/DSSD_verif/";
      Path::make(outFolder);
      std::string out_filename = outFolder+filename;

      unique_TH1F p_pure_singles (new TH1F(("p_pure_singles_"+thread_i_str).c_str(), "p_pure_singles", 10_k,0,10_MeV));
      unique_TH1F d_pure_singles (new TH1F(("d_pure_singles_"+thread_i_str).c_str(), "d_pure_singles", 10_k,0,10_MeV));
      unique_TH1F p_no_singles (new TH1F(("p_no_singles_"+thread_i_str).c_str(), "p_no_singles", 10_k,0,10_MeV));
      unique_TH1F d_no_singles (new TH1F(("d_no_singles_"+thread_i_str).c_str(), "d_no_singles", 10_k,0,10_MeV));
      
      unique_TH2F p_pure_singles_VS_dssd_energy (new TH2F(("p_pure_singles_VS_dssd_energy_"+thread_i_str).c_str(), "p_pure_singles_VS_dssd_energy", 2.5_k,0,50_MeV, 10_k,0,10_MeV));
      unique_TH2F d_pure_singles_VS_dssd_energy (new TH2F(("d_pure_singles_VS_dssd_energy"+thread_i_str).c_str(), "d_pure_singles_VS_dssd_energy", 2.5_k,0,50_MeV, 10_k,0,10_MeV));
      unique_TH2F p_no_singles_VS_dssd_energy (new TH2F(("p_no_singles_VS_dssd_energy_"+thread_i_str).c_str(), "p_no_singles_VS_dssd_energy", 2.5_k,0,50_MeV, 10_k,0,10_MeV));
      unique_TH2F d_no_singles_VS_dssd_energy (new TH2F(("d_no_singles_VS_dssd_energy"+thread_i_str).c_str(), "d_no_singles_VS_dssd_energy", 2.5_k,0,50_MeV, 10_k,0,10_MeV));
      
      unique_TH2F p_pure_singles_VS_dssd_time (new TH2F(("p_pure_singles_VS_dssd_time_"+thread_i_str).c_str(), "p_pure_singles_VS_dssd_time", 250,-50_ns,200_ns, 10_k,0,10_MeV));
      unique_TH2F d_pure_singles_VS_dssd_time (new TH2F(("d_pure_singles_VS_dssd_time"+thread_i_str).c_str(), "d_pure_singles_VS_dssd_time", 250,-50_ns,200_ns, 10_k,0,10_MeV));
      unique_TH2F p_no_singles_VS_dssd_time (new TH2F(("p_no_singles_VS_dssd_time_"+thread_i_str).c_str(), "p_no_singles_VS_dssd_time", 250,-50_ns,200_ns, 10_k,0,10_MeV));
      unique_TH2F d_no_singles_VS_dssd_time (new TH2F(("d_no_singles_VS_dssd_time"+thread_i_str).c_str(), "d_no_singles_VS_dssd_time", 250,-50_ns,200_ns, 10_k,0,10_MeV));
      
      unique_TH2F E_vs_index (new TH2F(("E_vs_index_"+thread_i_str).c_str(), "E_vs_index", 50,0,50, 5_k,0,50_MeV));
      unique_TH2F mult_vs_index (new TH2F(("mult_vs_index_"+thread_i_str).c_str(), "mult_vs_index", 50,0,50, 10,0,10));
      unique_TH2F T_vs_index (new TH2F(("T_vs_index_"+thread_i_str).c_str(), "T_vs_index", 50,0,50, 250,-50_ns,200_ns));
      
      unique_TH1F Ex_histo_sectors (new TH1F(("Ex_histo_sectors_"+thread_i_str).c_str(), "Ex_histo_sectors", 1_k,0,10_MeV));
      unique_TH2F Ex_vs_Ep_sectors (new TH2F(("Ex_vs_Ep_sectors_"+thread_i_str).c_str(), "Ex_vs_Ep_sectors;E_{proton} [keV];E_{x} [keV]", 1_k,0,10_MeV, 1_k,0,10_MeV));
      unique_TH2F Ex_vs_angle_sectors (new TH2F(("Ex_vs_angle_sectors_"+thread_i_str).c_str(), "Ex_vs_angle_sectors", 40,20,60, 1_k,0,10_MeV));
      unique_TH2F Edssd_vs_angle_sectors (new TH2F(("Edssd_vs_angle_sectors_"+thread_i_str).c_str(), "Edssd_vs_angle_sectors", 40,20,60, 3_k,0,30_MeV));
      
      unique_TH1F Ex_histo_rings_only (new TH1F(("Ex_histo_rings_only_"+thread_i_str).c_str(), "Ex_histo_rings_only", 1_k,0,10_MeV));
      unique_TH2F Ex_vs_Ep_rings_only (new TH2F(("Ex_vs_Ep_rings_only_"+thread_i_str).c_str(), "Ex_vs_Ep_rings_only;E_{proton} [keV];E_{x} [keV]", 1_k,0,10_MeV, 1_k,0,10_MeV));
      unique_TH2F Ex_vs_angle_rings_only (new TH2F(("Ex_vs_angle_rings_only_"+thread_i_str).c_str(), "Ex_vs_angle_rings_only", 40,20,60, 1_k,0,10_MeV));
      unique_TH2F Edssd_vs_angle_rings_only (new TH2F(("Edssd_vs_angle_rings_only_"+thread_i_str).c_str(), "Edssd_vs_angle_rings_only", 40,20,60, 3_k,0,30_MeV));

      std::unique_ptr<TH3I> pp_VS_DSSD_nrj (new TH3I(("pp_VS_DSSD_nrj"+thread_i_str).c_str(), "pp_VS_DSSD_nrj;E_{#gamma_{1}} [keV];E_{#gamma_{2}} [keV];E_dssd [keV]", 2048,0,4096, 2048,0,4096, 20,0,20_MeV));
      std::unique_ptr<TH3I> dd_VS_DSSD_nrj (new TH3I(("dd_VS_DSSD_nrj"+thread_i_str).c_str(), "pp_VS_DSSD_nrj;E_{#gamma_{1}} [keV];E_{#gamma_{2}} [keV];E_dssd [keV]", 2048,0,4096, 2048,0,4096, 20,0,20_MeV));
      
      std::vector<unique_TH2F> E_ring_VS_sectors;
      std::vector<unique_TH2F> T_ring_VS_sectors;
      std::vector<unique_TH2F> dE_VS_dT_ring_VS_sectors;
      std::vector<unique_TH2F> T_ring_VS_T_ring;
      std::vector<unique_TH2F> E_ring_VS_E_ring;
      std::vector<unique_TH2F> sum_E_rings_VS_E_sector;
      std::vector<unique_TH2F> dE_VS_dT_ring_VS_ring;
      std::vector<unique_TH2F> dT_ring_VS_ring_index;
      for (size_t ring_id = 0; ring_id<DSSD::nb_rings; ++ring_id)
      {
        std::string ring_id_str = std::to_string(ring_id);

        auto name = "E_ring_"+ring_id_str+"_VS_sectors_"+thread_i_str;
        E_ring_VS_sectors.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";sector [keV];ring [keV]").c_str(), 1.5_k,0,30_MeV, 1.5_k,0,30_MeV)));

        name = "dT_VS_dE_ring_"+ring_id_str+"_VS_sectors_"+thread_i_str;
        dE_VS_dT_ring_VS_sectors.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";#DeltaT ring-sector [ps];#DeltaE ring-sector [ps]").c_str(), 250,-50_ns,200_ns, 3_k,-15_MeV,15_MeV)));

        name = "T_ring_"+ring_id_str+"_VS_sectors_"+thread_i_str;
        T_ring_VS_sectors.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";sector [ps];ring [ps]").c_str(), 250,-50_ns,200_ns, 250,-50_ns,200_ns)));
        
        name = "T_ring_"+ring_id_str+"_VS_T_ring_"+thread_i_str;
        T_ring_VS_T_ring.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";ring [ps];ring [ps]").c_str(), 250,-50_ns,200_ns, 250,-50_ns,200_ns)));
        
        name = "E_ring_"+ring_id_str+"_VS_E_ring_"+thread_i_str;
        E_ring_VS_E_ring.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";ring [keV];ring [keV]").c_str(), 1.5_k,0,15_MeV, 1.5_k,0,15_MeV)));
        
        name = "Esum_ring_"+ring_id_str+"_E_sector_"+thread_i_str;
        sum_E_rings_VS_E_sector.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";sector [keV];E_{sum} 2 rings [keV]").c_str(), 1.5_k,0,30_MeV, 1.5_k,0,30_MeV)));
        
        name = "dE_VS_dT_ring_"+ring_id_str+"_VS_rings_"+thread_i_str;
        dE_VS_dT_ring_VS_ring.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";ring2-ring1 #DeltaT[ps];ring2-ring1 #DeltaE[keV]").c_str(), 200,-100_ns,100_ns, 3_k,-15_MeV,15_MeV)));
        
        name = "dT_ring_"+ring_id_str+"_VS_ring_label_"+thread_i_str;
        dT_ring_VS_ring_index.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";ring2-ring1 #DeltaT[ps];ring2 id").c_str(), 50,0,50, 250,-50_ns,200_ns)));
      }

      std::vector<unique_TH2F> E_sector_vs_angle;
      // std::vector<unique_TH2F> E_sector_VS_runs;
      for (size_t sector_id = 0; sector_id<DSSD::nb_sectors; ++sector_id)
      {
        std::string sector_id_str = std::to_string(sector_id);
        auto  name = "E_vs_angle_sector_"+sector_id_str+"_"+thread_i_str;
        E_sector_vs_angle.emplace_back(unique_TH2F((new TH2F(name.c_str(), "E_sector_vs_angle", 50,0,50, 250,-50_ns,200_ns))));
        
        // name = "E_sector_"+sector_id_str+"_VS_runs_"+thread_i_str;
        // E_sector_VS_runs.emplace_back(unique_TH2F((new TH2F(name.c_str(), (name+";run").c_str(), 50,0,50, 250,-50_ns,200_ns))));
      }

      std::vector<unique_TH2F> T_vs_run;
      std::vector<unique_TH2F> E_vs_run;
      std::vector<unique_TH2F> E_VS_Clover_each;
      std::vector<unique_TH2F> Ex_VS_Clover_each;
      std::vector<unique_TH2F> E_VS_T_each;
      std::vector<unique_TH2F> E_VS_angle_each_27Al_2210;
      std::vector<unique_TH2F> E_VS_angle_each_28Al_2270;
      std::vector<unique_TH2F> E_VS_angle_each_28Al_1130;
      std::vector<unique_TH2F> E_VS_angle_each_13C_3850;
      std::vector<unique_TH2F> E_VS_angle_each_25Mg_3683;
      std::vector<unique_TH2F> E_VS_angle_each_56Fe_1238;
      std::vector<unique_TH2F> E_VS_angle_each_16O_6129;
      std::vector<unique_TH2F> E_VS_angle_each_17O_2184;
      std::vector<unique_TH2F> E_VS_angle_each_236U_642;

      for (size_t strip_id = 0; strip_id<DSSD::nb_strips; ++strip_id)
      {
        std::string strip_id_str = std::to_string(strip_id);

        auto name = "T_strip_"+strip_id_str+"_VS_runs_"+thread_i_str;
        T_vs_run.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+"run number;Time [ps];").c_str(), 100,50,150, 250,-50_ns,200_ns)));
        
        name = "E_strip_"+strip_id_str+"_VS_runs_"+thread_i_str;
        E_vs_run.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+"run number;Energy [keV];").c_str(), 100,50,150, 1.5_k,0,30_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_clover_"+thread_i_str;
        E_VS_Clover_each.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";Clover [keV];Strip [keV]").c_str(), 6_k,0,6_MeV, 1.5_k,0,30_MeV)));

        name = "Ex_strip_"+strip_id_str+"_VS_clover_"+thread_i_str;
        Ex_VS_Clover_each.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";Clover [keV];Ex [keV]").c_str(), 10_ki,0,10_MeV, 1_ki,0,10_MeV)));
      
        name = "E_strip_"+strip_id_str+"_VS_time_"+thread_i_str;
        E_VS_T_each.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 250,-50_ns,200_ns, 3_k,0,30_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_27Al_2210_"+thread_i_str;
        E_VS_angle_each_27Al_2210.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_28Al_2270_"+thread_i_str;
        E_VS_angle_each_28Al_2270.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_28Al_1130_"+thread_i_str;
        E_VS_angle_each_28Al_1130.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_13C_3683_"+thread_i_str;
        E_VS_angle_each_13C_3850.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_25Mg_3683_"+thread_i_str;
        E_VS_angle_each_25Mg_3683.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_56Fe_1238_"+thread_i_str;
        E_VS_angle_each_56Fe_1238.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_16O_6129_"+thread_i_str;
        E_VS_angle_each_16O_6129.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_17O_2184_"+thread_i_str;
        E_VS_angle_each_17O_2184.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));

        name = "E_strip_"+strip_id_str+"_VS_time_236U_642_"+thread_i_str;
        E_VS_angle_each_236U_642.emplace_back(unique_TH2F(new TH2F((name).c_str(), (name+";time [ps];Energy [keV]").c_str(), 40,20,60, 2_k,0,20_MeV)));
      }

      Event event;
      event.reading(tree, "lTE");
      WarsawDSSD dssd(&Ex);

      CloversV2 pclovers; // prompt  clovers
      // CloversV2 nclovers; // neutron clovers
      CloversV2 dclovers; // delayed clovers
      
      tree.setMaxHits(nb_hits_read);
      // #ifndef DEBUG
      //   tree.loadRAM(); // If not debugging, one can load the tree in memory to speed up the process
      // #endif //DEBUG
      CoProgressBar progressbar(&tree.cursor(), tree.getMaxHits());
      while(tree.readNext())
      {
        if (MTObject::getThreadsNb()==1) progressbar.show();
        else if (tree.cursor()%freq_hit_display == 0) print (nicer_double (tree.cursor(), 0));

        // Clear the containers for the new loop
        pclovers.clear();
        // nclovers.clear();
        dclovers.clear();
        dssd.clear();

        for (int hit_i = 0; hit_i < event.mult; hit_i++)
        {
          auto const & label = event.labels[hit_i];
          auto const & time =  event.times[hit_i];
          auto const & nrj = event.nrjs[hit_i];

          if (nrj<20) continue;

          if(DSSD::is[label]) 
          {
            if (label<802) continue; // The 2 first sectors are bad
            if (DSSD::isRing[label]) 
            {
              event.times[hit_i] -= 50_ns;
              event.nrjs[hit_i] = ring_calib(nrj, label);
            }
            auto const & index = (DSSD::isRing[label]) ? DSSD::index_ring[label]+DSSD::nb_sectors : DSSD::index_sector[label];
            E_vs_index->Fill(index, nrj);
            T_vs_index->Fill(index, time);
            T_vs_run[index]->Fill(run_number, time);
            E_vs_run[index]->Fill(run_number, nrj);
            dssd.fill(event, hit_i);
          }

          else if (CloversV2::isClover(label))
          {
            if (gate(-10_ns, time, 10_ns)) 
            {
              if (kCalibGe) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
              pclovers.fill(event, hit_i);
            }
            // else if (gate( 10_ns, time,  40_ns)) nclovers.fill(event, hit_i);
            else if (gate( 40_ns, time, 170_ns)) 
            {
              if (found(CloversV2::blacklist, label)) continue;
              if (found(CloversV2::maxE_Ge, label) && CloversV2::maxE_Ge[label] < nrj) continue;
              if (kCalibGe) event.nrjs[hit_i] = calibGe.correct(nrj, run_number, label);
              dclovers.fill(event, hit_i);
            }
          }
        }

        auto const & nR = dssd.rings().size();
        auto const & nS = dssd.sectors().size();

        pclovers.analyze();
        // nclovers.analyze();
        dclovers.analyze();
        dssd.analyze();

        // Clovers : 

        if (pclovers.clean.size() == 1) 
        {
          p_pure_singles->Fill(pclovers.clean[0]->nrj);
          if (dssd.ok) 
          {
            if (dssd.time !=0) p_pure_singles_VS_dssd_time->Fill(dssd.time, pclovers.clean[0]->nrj);
            p_pure_singles_VS_dssd_energy->Fill(dssd.nrj, pclovers.clean[0]->nrj);
          }
        }

        else for (size_t clover_i = 0; clover_i< pclovers.clean.size(); ++clover_i)
        {
          auto const & clover = *(pclovers.clean[clover_i]);
          p_no_singles->Fill(clover.nrj);
          if (dssd.ok)
          {
             if (dssd.time !=0) p_no_singles_VS_dssd_time->Fill(dssd.time, clover.nrj);
            p_no_singles_VS_dssd_energy->Fill(dssd.nrj, clover.nrj);
          }
          for (size_t clover_j = clover_i+1; clover_j < pclovers.clean.size(); ++clover_j)
          {
            auto const & clover_bis = *(pclovers.clean[clover_j]);
            pp_VS_DSSD_nrj->Fill(clover.nrj, clover_bis.nrj, dssd.nrj);
            pp_VS_DSSD_nrj->Fill(clover_bis.nrj, clover.nrj, dssd.nrj);
          }
        }

        if (dclovers.clean.size() == 1) 
        {
          d_pure_singles->Fill(dclovers.clean[0]->nrj);
          if (dssd.ok) 
          {
            if (dssd.time !=0) d_pure_singles_VS_dssd_time->Fill(dssd.time, dclovers.clean[0]->nrj);
            d_pure_singles_VS_dssd_energy->Fill(dssd.nrj, dclovers.clean[0]->nrj);
          }
        }

        else for (size_t clover_i = 0; clover_i< dclovers.clean.size(); ++clover_i)
        {
          auto const & clover = *(dclovers.clean[clover_i]);
          d_no_singles->Fill(clover.nrj);
          if (dssd.ok)
          {
             if (dssd.time !=0) d_no_singles_VS_dssd_time->Fill(dssd.time, clover.nrj);
            d_no_singles_VS_dssd_energy->Fill(dssd.nrj, clover.nrj);
          }
          for (size_t clover_j = clover_i+1; clover_j < dclovers.clean.size(); ++clover_j)
          {
            auto const & clover_bis = *(dclovers.clean[clover_j]);
            if (abs(clover.time-clover_bis.time)>50_ns) continue;
            dd_VS_DSSD_nrj->Fill(clover.nrj, clover_bis.nrj, dssd.nrj);
            dd_VS_DSSD_nrj->Fill(clover_bis.nrj, clover.nrj, dssd.nrj);
          }
        }

        if (dssd.ok)
        {
          if (nS>0)
          {
            Ex_histo_sectors->Fill(dssd.Ex_p);
            Ex_vs_Ep_sectors->Fill(dssd.nrj, dssd.Ex_p);
            Ex_vs_angle_sectors->Fill(to_deg(dssd.angle), dssd.Ex_p);
            Edssd_vs_angle_sectors->Fill(to_deg(dssd.angle), dssd.nrj);
          }
          else
          {
            Ex_histo_rings_only->Fill(dssd.Ex_p);
            Ex_vs_Ep_rings_only->Fill(dssd.nrj, dssd.Ex_p);
            Ex_vs_angle_rings_only->Fill(to_deg(dssd.angle), dssd.Ex_p);
            Edssd_vs_angle_rings_only->Fill(to_deg(dssd.angle), dssd.nrj);
          }
        }

        // DSSD :

        for (size_t ring_i = 0; ring_i<nR; ++ring_i) 
        {
          auto const & id_i = dssd.rings().all_id[ring_i];
          auto const & plot_id = id_i+DSSD::nb_sectors;
          auto const & ring1 = dssd.rings()[id_i];
          mult_vs_index->Fill(id_i, nR);
          std::vector<double> E_sums;
          for (size_t ring_j = ring_i+1; ring_j<nR; ++ring_j)
          {
            auto id_j = dssd.rings().all_id[ring_j];
            auto const & ring2 = dssd.rings()[id_j];
            T_ring_VS_T_ring[id_i]->Fill(ring1.time, ring2.time);
            E_ring_VS_E_ring[id_i]->Fill(ring1.nrj, ring2.nrj);
            dE_VS_dT_ring_VS_ring[id_i]->Fill(ring2.time-ring1.time, ring2.nrj-ring1.nrj);
            dT_ring_VS_ring_index[id_i]->Fill(ring2.index(), ring2.time-ring1.time);
            E_sums.push_back(ring2.nrj+ring1.nrj);
          }

          E_VS_T_each[plot_id]->Fill(ring1.time, ring1.nrj);

          for (size_t sector_i = 0; sector_i<nS; ++sector_i)
          {
            auto id_sector = dssd.sectors().all_id[sector_i];
            auto const & sector1 = dssd.sectors()[id_sector];
            T_ring_VS_sectors[id_i]->Fill(sector1.time, ring1.time);
            E_ring_VS_sectors[id_i]->Fill(sector1.nrj, ring1.nrj);
            dE_VS_dT_ring_VS_sectors[id_i]->Fill(ring1.time-sector1.time, ring1.nrj-sector1.nrj);
            for (auto const & E : E_sums) sum_E_rings_VS_E_sector[id_i]->Fill(sector1.nrj, E);
          }

          for (auto const & clover : pclovers.clean) 
          {
            E_VS_Clover_each [plot_id]->Fill(clover->nrj, ring1.nrj);
            Ex_VS_Clover_each[plot_id]->Fill(clover->nrj, dssd.Ex_p);

                 if (gate(2210-5, clover->nrj, 2210+5)) E_VS_angle_each_27Al_2210[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(2270-5, clover->nrj, 2270+5)) E_VS_angle_each_28Al_2270[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(1130-5, clover->nrj, 1130+5)) E_VS_angle_each_28Al_1130[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(3830  , clover->nrj, 3870  )) E_VS_angle_each_13C_3850[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(3683-5, clover->nrj, 3683+5)) E_VS_angle_each_25Mg_3683[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(1238-5, clover->nrj, 1238+5)) E_VS_angle_each_56Fe_1238[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(6129-5, clover->nrj, 6129+5)) E_VS_angle_each_16O_6129[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(2184-5, clover->nrj, 2184+5)) E_VS_angle_each_17O_2184[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
            else if (gate(642-2 , clover->nrj, 642+2 )) E_VS_angle_each_236U_642[plot_id]->Fill(to_deg(ring1.angle), ring1.nrj);
          }
        }

        for (size_t sector_i = 0; sector_i<nS; ++sector_i) 
        {
          auto id_i = dssd.sectors().all_id[sector_i];
          auto const & sector1 = dssd.sectors()[id_i];
          mult_vs_index->Fill(id_i, nS);
          if (!dssd.ok) continue;
          E_VS_T_each[id_i]->Fill(sector1.time, sector1.nrj);
          for (auto const & clover : pclovers.clean) 
          {
            E_VS_Clover_each [id_i]->Fill(clover->nrj, sector1.nrj);
            Ex_VS_Clover_each[id_i]->Fill(clover->nrj, dssd.Ex_p);
            if (dssd.ok)
            {
                  if (gate(2210-5, clover->nrj, 2210+5)) E_VS_angle_each_27Al_2210[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(2270-5, clover->nrj, 2270+5)) E_VS_angle_each_28Al_2270[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(2270-5, clover->nrj, 2270+5)) E_VS_angle_each_28Al_1130[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(3683-5, clover->nrj, 3683+5)) E_VS_angle_each_13C_3850[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(3683-5, clover->nrj, 3683+5)) E_VS_angle_each_25Mg_3683[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(1238-5, clover->nrj, 1238+5)) E_VS_angle_each_56Fe_1238[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(6129-5, clover->nrj, 6129+5)) E_VS_angle_each_16O_6129[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(2184-5, clover->nrj, 2184+5)) E_VS_angle_each_17O_2184[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
              else if (gate(642-2, clover->nrj, 642+2)) E_VS_angle_each_236U_642[id_i]->Fill(to_deg(dssd.angle), sector1.nrj);
            }
          }
        } 
      }
      progressbar.showFast();

      files_total_size.fetch_add(size_file(file,"Mo"), std::memory_order_relaxed);
      auto const & thread_i_str = std::to_string(thread_i);
      lock_mutex lock(write_mutex);
      std::unique_ptr<TFile> file (TFile::Open(out_filename.c_str(),"recreate"));
      file->cd();
        if (p_pure_singles->Integral() > 1) p_pure_singles->Write("p_pure_singles", TObject::kOverwrite);
        if (d_pure_singles->Integral() > 1) d_pure_singles->Write("d_pure_singles", TObject::kOverwrite);
        if (p_no_singles->Integral() > 1) p_no_singles->Write("p_no_singles", TObject::kOverwrite);
        if (d_no_singles->Integral() > 1) d_no_singles->Write("d_no_singles", TObject::kOverwrite);
        if (p_pure_singles_VS_dssd_time->Integral() > 1) p_pure_singles_VS_dssd_time->Write("p_pure_singles_VS_dssd_time", TObject::kOverwrite);
        if (d_pure_singles_VS_dssd_time->Integral() > 1) d_pure_singles_VS_dssd_time->Write("d_pure_singles_VS_dssd_time", TObject::kOverwrite);
        if (p_no_singles_VS_dssd_time->Integral() > 1) p_no_singles_VS_dssd_time->Write("p_no_singles_VS_dssd_time", TObject::kOverwrite);
        if (d_no_singles_VS_dssd_time->Integral() > 1) d_no_singles_VS_dssd_time->Write("d_no_singles_VS_dssd_time", TObject::kOverwrite);
        if (p_pure_singles_VS_dssd_energy->Integral() > 1) p_pure_singles_VS_dssd_energy->Write("p_pure_singles_VS_dssd_energy", TObject::kOverwrite);
        if (d_pure_singles_VS_dssd_energy->Integral() > 1) d_pure_singles_VS_dssd_energy->Write("d_pure_singles_VS_dssd_energy", TObject::kOverwrite);
        if (p_no_singles_VS_dssd_energy->Integral() > 1) p_no_singles_VS_dssd_energy->Write("p_no_singles_VS_dssd_energy", TObject::kOverwrite);
        if (d_no_singles_VS_dssd_energy->Integral() > 1) d_no_singles_VS_dssd_energy->Write("d_no_singles_VS_dssd_energy", TObject::kOverwrite);
        if (E_vs_index->Integral() > 1) E_vs_index->Write("E_vs_index", TObject::kOverwrite);
        if (mult_vs_index->Integral() > 1) mult_vs_index->Write("mult_vs_index", TObject::kOverwrite);
        if (T_vs_index->Integral() > 1) T_vs_index->Write("T_vs_index", TObject::kOverwrite);
        if (Ex_histo_sectors->Integral() > 1) Ex_histo_sectors->Write("Ex_histo_sectors", TObject::kOverwrite);
        if (Ex_vs_Ep_sectors->Integral() > 1) Ex_vs_Ep_sectors->Write("Ex_vs_Ep_sectors", TObject::kOverwrite);
        if (Ex_vs_angle_sectors->Integral() > 1) Ex_vs_angle_sectors->Write("Ex_vs_angle_sectors", TObject::kOverwrite);
        if (Edssd_vs_angle_sectors->Integral() > 1) Edssd_vs_angle_sectors->Write("Edssd_vs_angle_sectors", TObject::kOverwrite);
        if (Ex_histo_rings_only->Integral() > 1) Ex_histo_rings_only->Write("Ex_histo_rings_only", TObject::kOverwrite);
        if (Ex_vs_Ep_rings_only->Integral() > 1) Ex_vs_Ep_rings_only->Write("Ex_vs_Ep_rings_only", TObject::kOverwrite);
        if (Ex_vs_angle_rings_only->Integral() > 1) Ex_vs_angle_rings_only->Write("Ex_vs_angle_rings_only", TObject::kOverwrite);
        if (Edssd_vs_angle_rings_only->Integral() > 1) Edssd_vs_angle_rings_only->Write("Edssd_vs_angle_rings_only", TObject::kOverwrite);
        if (pp_VS_DSSD_nrj->Integral() > 1) pp_VS_DSSD_nrj->Write("pp_VS_DSSD_nrj", TObject::kOverwrite);
        if (pp_VS_DSSD_nrj->Integral() > 1) pp_VS_DSSD_nrj->Write("pp_VS_DSSD_nrj", TObject::kOverwrite);
        if (pp_VS_DSSD_nrj->Integral() > 1) pp_VS_DSSD_nrj->Write("pp_VS_DSSD_nrj", TObject::kOverwrite);
        if (dd_VS_DSSD_nrj->Integral() > 1) dd_VS_DSSD_nrj->Write("dd_VS_DSSD_nrj", TObject::kOverwrite);
        
        for (auto & histo : T_ring_VS_T_ring) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_ring_VS_E_ring) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : sum_E_rings_VS_E_sector) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : dE_VS_dT_ring_VS_ring) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : dT_ring_VS_ring_index) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : T_vs_run) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_vs_run) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_sector_vs_angle) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : dE_VS_dT_ring_VS_sectors) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);

        for (auto & histo : T_ring_VS_sectors) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_ring_VS_sectors) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_T_each) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_Clover_each) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : Ex_VS_Clover_each) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_27Al_2210) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_28Al_1130) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_28Al_2270) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_13C_3850) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_25Mg_3683) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_56Fe_1238) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_16O_6129) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_17O_2184) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        for (auto & histo : E_VS_angle_each_236U_642) if (histo->Integral()>1) histo->Write((removeLastPart(histo->GetName(), '_')).c_str(), TObject::kOverwrite);
        

      file->Close();
      print(out_filename, "written");
      // mutex freed
    }
  });
  print("Reading speed of", files_total_size/timer.TimeSec(), "Mo/s | ", 1.e-6*total_evts_number/timer.TimeSec(), "M events/s");
}

#ifndef __CINT__
int main(int argc, char** argv)
{
       if (argc == 1) macroDSSDVerif();
  else if (argc == 2) macroDSSDVerif(std::stoi(argv[1]));
  else if (argc == 3) macroDSSDVerif(std::stoi(argv[1]), std::stod(argv[2]));
  else if (argc == 4) macroDSSDVerif(std::stoi(argv[1]), std::stod(argv[2]), std::stoi(argv[3]));
  else error("invalid arguments");

  return 1;
}
#endif //__CINT__
// g++ -g -o macroDSSDVerif macroDSSDVerif.C ` root-config --cflags` `root-config --glibs` -DDEBUG -lSpectrum -std=c++17 -Wall -Wextra
// g++ -O2 -o macroDSSDVerif macroDSSDVerif.C ` root-config --cflags` `root-config --glibs` -lSpectrum -std=c++17