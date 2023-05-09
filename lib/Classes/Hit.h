#ifndef HIT_H
#define HIT_H
using Label  = UShort_t;
using Label_vec = std::vector<Label>;
using ADC    = Int_t;
using ADC_vec = std::vector<ADC>;
using NRJ = Float_t;
using Energy_vec = std::vector<NRJ>;
using Time   = ULong64_t;
using Time_vec = std::vector<Time>;
using Pileup = Bool_t;
using Pileup_vec = std::vector<Bool_t>;

enum Detector
{ null,              Clover,    Ge,         BGO,         LaBr3,   RF,   EDEN,   PhaseI_Ge,   PhaseI_BGO,   paris,    Paris_LaBr3,   Paris_NaI,   dssd};
std::vector<std::string> type_str =
{ "Unkown detector", "Clover", "Ge_Clover","BGO_Clover", "LaBr3", "RF", "EDEN", "PhaseI_Ge", "PhaseI_BGO", "Paris", "Paris_LaBr3", "Paris_NaI", "DSSD"};
using Detector_vec = std::vector<int>;


class Hit
{
public:
  Label   label  = 0; //Hit label
  Float_t nrj    = 0; //energy
  Float_t nrj2   = 0; //used if QDC2
  NRJ     nrjcal = 0; // Calibrated energy
  Time    time   = 0; //time
  bool    pileup = 0; //pile-up

  void reset()
  {
    label  = 0;
    nrj    = 0;
    nrj2   = 0;
    nrjcal = 0;
    time   = 0;
    pileup = 0;
  }

  Float_t gate_ratio(){if (nrj2 != 0) return ((Float_t)(nrj2-nrj)/nrj2); else return 0.f;}
};

class Hit_ptr
{
public:

  Label   * label  = nullptr; //Hit label
  Float_t * nrj    = nullptr; //energy
  Float_t * nrj2   = nullptr; //used if QDC2
  NRJ     * nrjcal = nullptr; // Calibrated energy
  Time    * time   = nullptr; //time
  float   * time2  = nullptr; //time
  bool    * pileup = nullptr; //pile-up

  void reset()
  {
    label  = nullptr;
    nrj    = nullptr;
    nrj2   = nullptr;
    nrjcal = nullptr;
    time   = nullptr;
    time2  = nullptr;
    pileup = nullptr;
  }

  Float_t gate_ratio(){if ((*nrj2) != 0) return ((Float_t)((*nrj2)-(*nrj))/(*nrj2)); else return 0.f;}

};

#endif //HIT_H
