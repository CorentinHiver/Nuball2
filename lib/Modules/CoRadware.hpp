#ifndef CORADWARE_HPP
#define CORADWARE_HPP

#include "../libRoot.hpp"

class CoRadware
{public:
  CoRadware(TH2* _bidim, int autoRemoveBackground = 0)
  {
    file = gFile;
    gROOT->cd();
    m_bidim = (TH2F*) _bidim->Clone(TString(_bidim->GetName())+"_radware");
    if (autoRemoveBackground>0) CoLib::removeBackground(m_bidim, m_nb_it_bckg, autoRemoveBackground);
    init();
    proj();
    launch();
  }
  
  void init()
  {
    canvas->ToggleEventStatus();
    canvas->ToggleEditor();
    canvas->ToggleToolBar();
    canvas->cd();
    m_finish = false;
  }

  static bool m_finish;

  void launch()
  {
    std::thread root_thread([]()
    {
      while(!m_finish) 
      {
        // gPad->WaitPrimitive();
        gSystem->ProcessEvents();
        gSystem->Sleep(10);
      }
    });
    
    print(m_finish);
    std::string instruction;
    while(!m_finish)
    {
      std::cout << "> ";

      // Ctrl+D events :
      auto const & getline_ret = std::getline(std::cin, instruction);
      if (!getline_ret) 
      {
        error("CTRL-D pressed, interactive interface will not work next time. Use st command instead");
        m_finish = true;
      }
      else if (instruction == "") continue;
      else if (instruction == "todo") this->todo();
      else if (instruction == "ag") this->addGate();
      else if (instruction == "bd") {print("May take a while..."); this->draw(m_bidim, "colz");}
      else if (instruction == "bi") this->set_nb_it_bckg();
      else if (instruction == "cl") this->clean();
      // else if (instruction == "di") this->projectDiag();
      else if (instruction == "ex") this->ex();
      else if (instruction == "es") this->exportSpectrum();
      else if (instruction == "gs") this->setGateSize();
      else if (instruction == "h" ) this->printHelp();
      else if (instruction == "in") this->integral();
      else if (instruction == "ns") this->normalizeSpectra();
      else if (instruction == "pr") this->proj();
      else if (instruction == "px") this->px();
      else if (instruction == "py") this->py();
      else if (instruction == "rb") this->removeBackground();
      else if (instruction == "rp") this->removeProjection();
      else if (instruction == "rs") CoLib::Pad::remove_stats();
      else if (instruction == "sg") this->addGate(false);
      else if (instruction == "sh") CoLib::Pad::subtract_histos();
      else if (instruction == "sp") this->simulatePeak();
      else if (instruction == "sm") this->gate_same();
      else if (instruction == "st") {m_finish = true;}
      else if (instruction == "uz") this->unZoom();
      else if (instruction == "+") this->gate(m_gate_number+1);
      else if (instruction == "-") this->gate(m_gate_number-1);
      else if (isNumber(instruction)) this->gate(std::stod(instruction));
      else error("Wrong input...");
    }
    print("Exiting CoRadware");
    root_thread.join();
    free_resources();
  }

  void todo()
  {
    print("Faire un mode asymetrique où une gate génère les projections sur les deux axes côte à côte");
  }

  void printHelp()
  {
    print("List of commands :");
    print("ag : add gate");
    print("bd : display bidim");
    print("bi : set number of iterations for automatic background subtraction (rb) of one dimensional spectra");
    print("cl : clean");
    // print("di : project bidim diagonal");
    print("ex : set range spectrum");
    print("es : export spectra to the root environement");
    print("gs : set gate size in bin");
    print("h  : display this help");
    print("in : peak integral");
    print("ns : normalize spectra");
    print("pr : display total projection");
    print("px : projection onto the x axis");
    print("py : projection onto the y axis (default)");
    print("rb : remove background automatically");
    print("rp : remove total projection automatically");
    print("rs : remove stat box");
    print("sg : subtract gate");
    print("sh : subtract the two displayed histograms");
    print("sp : simulate peak");
    print("sm : overlay another gate");
    print("st : finish session");
    print("uz : unzoom axis");
  }

  void draw(TH1* histo, std::string options = "")
  {
    m_nb_sm = 0;
    canvas->cd();
    if (m_focus) histo->GetXaxis()->SetRangeUser(
      m_focus->GetXaxis()->GetBinLowEdge(m_focus->GetXaxis()->GetFirst()), 
      m_focus->GetXaxis()->GetBinUpEdge(m_focus->GetXaxis()->GetLast()));
    m_focus = histo;
    if (m_focus->InheritsFrom(TH2::Class())) options+="colz"; // For 2D
    else options+="hist"; // For 1D
    m_focus->Draw(options.c_str());
    gPad->Update();
  }

  void clean()
  {
    if (m_focus != m_gate) {print("Cleaning is for gated spectrum only..."); return;}
    int min_E, max_E;
    std::string instruction;

    std::cout << "normalisation peak min : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    min_E = std::stoi(instruction);

    std::cout << "normalisation peak max : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    max_E = std::stoi(instruction);

    auto clean_gate = CoLib::removeVeto(m_gate, m_proj, min_E, max_E);
    delete m_gate;
    m_gate = clean_gate;
    // m_gate->Draw();
    // gPad->Update();
    this->draw(m_gate);
  }

  void simulatePeak()
  {
    int peak, resolution, nb;
    std::string instruction;

    std::cout << "peak mean : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    peak = std::stoi(instruction);

    std::cout << "resolution : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    resolution = std::stoi(instruction);

    std::cout << "nb : ";
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    nb = std::stoi(instruction);

    CoLib::simulatePeak(m_focus, peak, resolution, nb, true);
    gPad->Update();
  }

  void normalizeSpectra()
  {
    CoLib::Pad::normalize_histos_min();
  }

  void exportSpectrum()
  {
    std::cout << "Name to set (return for " << m_focus->GetName() << ")";
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = m_focus->GetName();
    auto test = static_cast<TH1F*> (m_focus->Clone(name.c_str()));
    print(TString("exporting")+test->GetName());
  }

  void setGateSize() 
  {
    printC("Choose gate size (current ",m_gate_size,")");
    std::cin >> m_gate_size;
  }

  void setHist(TH2F* _bidim) 
  {
    m_bidim = _bidim;
    this->proj();
  }
  
  void proj()
  {
    if (m_py) 
    {
      m_proj = static_cast<TH1*> (m_bidim->ProjectionY()->Clone("total projection y"));
      m_proj->SetTitle("total projection y");
    }
    else 
    {
      m_proj = static_cast<TH1*> (m_bidim->ProjectionX()->Clone("total projection x"));
      m_proj->SetTitle("total projection x");
    }
    this->draw(m_proj);
  }

  void px()
  {
    m_py = false;
  }

  void py()
  {
    m_py = true;
  }

  void addGate(bool add = true)
  {
    if (!m_gate) {error("No gate so far..."); return;}

    std::cout << "gate to " << ((add) ? "add" : "subtract") << " :";
    std::string instruction;
    std::getline(std::cin, instruction);
    if (!checkIsNumber(instruction)) return;
    int bin = std::stoi(instruction);

    std::cout << "weight (rtn for 1.) :";
    std::getline(std::cin, instruction);
    double weight = 1.;
    if (!(instruction.empty()))
    {
      if (!checkIsNumber(instruction)) return;
      weight = std::stod(instruction);
    }

    if (!add) weight*=-1.;

    if (m_py) for (int y = 0; y<m_bidim->GetNbinsY(); ++y) for (int x = bin-m_gate_size; x<=bin+m_gate_size; ++x)
    {
      m_gate->AddBinContent(y, m_bidim->GetBinContent(x, y)*weight);
    }
    else for (int x = 0; x<m_bidim->GetNbinsY(); ++x) for (int y = bin-m_gate_size; y<=bin+m_gate_size; ++y)
    {
      m_gate->AddBinContent(x, m_bidim->GetBinContent(x, y)*weight);
    }
    this->draw(m_gate);
  }

  void ex()
  {
    auto const & low = CoLib::selectPointX(m_focus, "Low edge");
    auto const & high = CoLib::selectPointX(m_focus, "High edge");
    m_focus->GetXaxis()->SetRangeUser(low, high);
    this->draw(m_focus);
    gPad->Update();
  }

  void gate_same(std::string val_str = "")
  {
    print("Choose energy to draw on top");
    if (val_str == "") std::cin >> val_str;
    TH1* gate = nullptr;
    std::string name;
    if (val_str == "pr") 
    {
      name = "m_proj2";
      gate = ((TH1*)m_proj->Clone(name.c_str()));
      gate->SetTitle("total projection");
    }
    else 
    {
      if (!checkIsNumber(val_str)) return;
      auto const & e = std::stoi(val_str);
      auto const & bin = m_bidim->GetXaxis()->FindBin(e)-1;
      auto const & low_e = e-m_gate_size;
      auto const & high_e = e+m_gate_size;
      name = std::to_string(bin)+" g same";
      gate = static_cast<TH1*>((m_py) 
        ? m_bidim->ProjectionY("g", low_e, high_e)->Clone(name.c_str())
        : m_bidim->ProjectionX("g", low_e, high_e)->Clone(name.c_str()));
      gate->SetTitle((std::to_string(bin)+" gate").c_str());
    }
    gate->Draw("same");
    gate->SetLineColor(CoLib::ROOT_nice_colors[(++m_nb_sm & 111)]);
    m_list_histo_to_delete.push_back(name);
    gPad->Update();
  }

  void gate(double e)
  {
    delete m_gate;
    m_gate_number = e;
    auto const & bin = m_bidim->GetXaxis()->FindBin(e)-1;
    auto const & low_e = e-m_gate_size;
    auto const & high_e = e+m_gate_size;
    m_gate = static_cast<TH1*> ((m_py)
      ? m_bidim->ProjectionY("g", low_e, high_e)->Clone((std::to_string(bin)+" g").c_str())
      : m_bidim->ProjectionX("g", low_e, high_e)->Clone((std::to_string(bin)+" g").c_str()));
    m_gate->SetTitle((std::to_string(bin)+" gate").c_str());
    this->draw(m_gate);
    print(CoLib::peakIntegral(m_focus, low_e, high_e, m_nb_it_bckg));
  }

  void integral()
  {
    double start, stop;
    print("Choose borne inf");
    std::cin >> start;
    print("Choose borne sup");
    std::cin >> stop;
    print(CoLib::peakIntegral(m_proj, start, stop, m_nb_it_bckg));
  }

  void removeBackground()
  {
    if (m_focus == m_bidim)
    {
      print("May take a while..."); 
      CoLib::removeBackground(m_bidim, m_nb_it_bckg);
      this->proj();
    }
    else 
    {
      CoLib::removeBackground(m_focus, m_nb_it_bckg, "", true);
      this->draw(m_focus);
    }
  }

  void removeProjection()
  {
    if (m_focus == m_bidim)
    {
      print("Can't remove projection of the bidim...");
      return;
    }
    if (!m_proj) m_proj = (m_py) ? m_bidim->ProjectionY() : m_bidim->ProjectionX();
    this->draw(m_proj, "same");
    CoLib::Pad::normalize_histos_min();
    CoLib::Pad::subtract_histos();
    m_focus = CoLib::Pad::get_histos()[0];
    gPad->Clear();
    this->draw(m_focus);
  }

  ~CoRadware()
  {
    free_resources();
  }

  void unZoom()
  {
    print("unzoom");
    m_focus->GetXaxis()->UnZoom();
    if (m_focus->InheritsFrom(TH2::Class())) m_focus->GetYaxis()->UnZoom();
    draw(m_focus);
  }

  void set_nb_it_bckg()
  {
    print("Choose iterations number for automatic background research");
    std::cin >> m_nb_it_bckg;
  }

private:

  void free_resources()
  {
    delete m_bidim;
    delete m_proj;
    delete m_gate;
    // delete m_focus;
    for (auto const & name : m_list_histo_to_delete) delete gDirectory->Get(name.c_str());
    canvas->Close();
    if (file) file->cd();
  }

  bool checkIsNumber(std::string instruction, std::string message = "")
  {
    auto const & ret = isNumber(instruction);
    if (!ret) print((message == "") ? message : "error input : must be a number");
    return ret;
  }

  TFile* file = nullptr;

  TCanvas *canvas = new TCanvas("RadwareCanvas", "RadwareCanvas");
  int m_nb_it_bckg = 20;
  TH2F* m_bidim = nullptr;
  TH1* m_proj = nullptr;
  TH1* m_gate = nullptr;
  TH1* m_focus = nullptr;
  Strings m_list_histo_to_delete;

  bool m_py = true;
  int m_gate_size = 1;
  double m_gate_number = 0;
  int m_nb_sm = 0;
};
bool CoRadware::m_finish = false;


#endif //CORADWARE_HPP