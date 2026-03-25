#pragma once

#include "TH1F.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TH2I.h"
#include "TH3F.h"
#include "TH3D.h"
#include "TH3I.h"

namespace Colib
{
  namespace MT
  {
    template <class THist>
    class Histo
    {
      using histo_ptr = std::unique_ptr<THist>;
      
      void init()
      {
#ifdef CoMT
        // Subscribe to the initialization signal
        m_init_signal_id = MT::subscribeToInitialise([this](size_t n) {this->allocateForMT(n);});
        m_fini_signal_id = MT::subscribeToFinalise([this](size_t n [[maybe_unused]]) {this->merge();});
        if (MT::isActivated()) this->allocateForMT(MT::getNbThreads());
#endif// CoMT
      }

      void allocateForMT(size_t n_threads)
      {
        std::lock_guard<std::mutex> lock(m_alloc_mutex);
        
        histos.clear();
        histos.resize(n_threads);

        for (size_t i = 0; i < n_threads; ++i)
        {
          // Clone the master histogram to inherit binning, limits, and titles
          std::string new_name = std::string(histo->GetName()) + "_t" + std::to_string(i);
          THist* clone = static_cast<THist*>(histo->Clone(new_name.c_str()));
          clone->Reset();
          clone->SetDirectory(nullptr); // Vital for thread safety in ROOT!
          histos[i].reset(clone);
        }
      }

    public:

      template<class... ARGS>
      Histo(std::string const & name, std::string const & title, ARGS &&... args) 
      {
        // Initialize the master histogram
        histo = std::make_unique<THist>(name.c_str(), title.c_str(), std::forward<ARGS>(args)...);
#ifdef CoMT
        histo->SetDirectory(nullptr); // Detach from ROOT's global directory for safety
#endif// CoMT
        init();
      }

      ~Histo()
      {
#ifdef CoMT
        // Unsubscribe to prevent dangling pointers if this goes out of scope
        MT::unsubscribeToInitialise(m_init_signal_id);
        MT::unsubscribeToFinalise  (m_fini_signal_id);
#endif
      }

      template<class... ARGS>
      void Fill(ARGS &&... args)
      {
#ifdef CoMT
        if (MT::isActivated() && !histos.empty()) histos[MT::getThreadIndex()]->Fill(std::forward<ARGS>(args)...);
        else                                      histo                       ->Fill(std::forward<ARGS>(args)...);
#else 
        histo->Fill(std::forward<ARGS>(args)...);
#endif 
      }

      THist* operator->()
      {
#ifdef CoMT
        if (MT::isActivated() && MT::getThreadIndex() < histos.size()) return histos[MT::getThreadIndex()].get();
#endif
        return histo.get();
      }

      void merge()
      {
#ifdef CoMT
        if (histos.empty()) return;

        TList list;
        for (auto & h : histos) if (h) list.Add(h.get());
        
        histo->Merge(&list);
        
        // Clear the worker histos to free memory and prevent double-merging
        for (auto& h : histos) if (h) h->Reset(); // Keep memory allocated, but empty the bins for the next run
        histos.clear();
#endif 
      }

    private:
      histo_ptr histo;
      std::vector<histo_ptr> histos;
      size_t m_init_signal_id{0};
      size_t m_fini_signal_id{0};
      std::mutex m_alloc_mutex;
    };
  } 
}