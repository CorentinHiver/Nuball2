#ifndef UTILS_H_CO
#define UTILS_H_CO

#include "libCo.hpp"
#include "Classes/Hit.h"
#include "Classes/Timer.hpp"
#ifdef FASTERAC
#include "faster.hpp"
#endif //FASTERAC


// ------------- //
//     UNITS     //
// ------------- //

Float_t _ns = 1000;

struct GHit
{
  int label;
  float energy;
  float energy2;
  float time;
  int type;
  int eventID;
};

struct Clover_Hit
{//Add-backed Clover hit
  uchar clover = 0; //From 1 to 24
  Float_t       nrjcal = 0;
  Float_t       time   = 0.;
  uchar I = -1; // channel avec le plus d'énergie
};

struct DSSD_Hit
{
  Label label = 799; // needs to be filled
  NRJ nrj = 0.;
  Float_t time = 0;
  bool isRing = false; // ring or sector, needs to be filled
};

using DSSD_Event = std::vector < DSSD_Hit >;

struct LaBr3_Hit
{
  uchar label   = 0;
  Float_t       nrjcal  = 0;
  ULong64_t     time    = 0;
};


struct BGO_Hit
{
  uchar clover = 0;
  Float_t       nrjcal = 0;
};
//------------------//
//       Using      //
//------------------//

using Clover_Event = std::vector < Clover_Hit>;
using LaBr3_Event  = std::vector < LaBr3_Hit >;
using BGO_Event    = std::vector < BGO_Hit   >;

// using Gate = std::pair<Int_t, Int_t>;

using Timeshift  = std::vector < Int_t  > ;
using Options    = std::vector < Bool_t > ;
using Buffer     = std::vector < Hit    > ;
using Buffer_ptr = std::vector < Hit*   > ;
// using Gates      = std::vector < Gate   > ;
using Labels     = std::vector < std::string > ;

using TimeshiftMap = std::unordered_map < UShort_t, Int_t       > ;
using LabelsMap    = std::unordered_map < UShort_t, std::string > ;
using MapDetector  = std::unordered_map < Detector, Float_t     > ;
template<typename T>
using BinMap       = std::unordered_map < Int_t   , T           > ;

class RF_Manager
{
public:
  RF_Manager(){};
  Bool_t setHit(Hit const & hit);
  // Bool_t setHit(Event const & event, int const & index);

  Long64_t pulse_ToF(ULong64_t const & time, Long64_t const & offset = 0) const
  {
    // Shifts the time in order to be able to get hits before the hit :
    ULong64_t const shifted_time = time+offset;
    if (shifted_time>last_hit)
    {// Normal case : the RF reference timestamp is lower than the shifted timestamps (should also be the case of unshifted timestamp)
      // dT = (shifted_time-last_hit) corresponds to the time separating the current hit to the reference RF
      // Therefore, N = dT/period corresponds to the number of periods separating the two hits
      // In other words, it is the number of pulses between the reference RF and the current hit
      // Then, period*N is the timestamp of the pulse relative to the current hit
      // (Remember we are doing integer arithmetic, period*dT/period != dT)
      // And dT%period is the rest of the integer division, hence the time between the hit and its relative pulse
      // Finally, one need to substract the applied offset in order to get the correct result :
      return ( static_cast<Long64_t> ((time+offset-last_hit)%period)-offset );
    }
    else
    {// When the data is not correctly ordered :
      // In order to get a correct answer, one need to get a positive difference, so to invert the difference : now last_hit-shifted_time>0
      // But the result is inverted and we obtain really period-timestamp. We get the correct result by doing :
      // relative_timestamp = period - (period-timestamp)
      return ( static_cast<Long64_t> (period-(last_hit-time-offset)%period)-offset );
    }
  }

  Long64_t pulse_ToF(Hit const & hit, Long64_t const & offset = 0) const
  {
    return pulse_ToF(hit.time, offset);
  }

  Bool_t isPrompt(Hit const & hit, Long64_t const & borneMin, Long64_t const & borneMax)
  {
    return (pulse_ToF(hit,-borneMin) < borneMax);
  }

  ULong64_t last_hit = 0;
  ULong64_t period   = 399998;

  static UShort_t label;
};

UShort_t RF_Manager::label = 251;

Bool_t RF_Manager::setHit(Hit const & hit)
{
  if (hit.label == RF_Manager::label)
  {
    last_hit = hit.time;
    // period = (last_hit-hit.time)/100000;
    period = hit.nrjcal;
    return true;
  }
  else return false;
}

// Bool_t RF_Manager::setHit(Event const & event, int const & index);
// {
//   if (event.labels[index] == RF_Manager::label)
//   {
//     last_hit = event.times[index];
//     // period = (last_hit-hit.time)/100000;
//     period = event.nrjs[index];
//     return true;
//   }
//   else return false;
// }

struct Timing_ref
{
  UShort_t    label = 0 ;
  std::string name  = "";
};

struct Source_info
{
  Float_t ratio_seuil = -1;
  Int_t   ADC_seuil   = -1;
  std::vector<Float_t> peaks;
};

class Multiplicity
{
public:
  Multiplicity();
  UInt_t operator++() {return(m_multiplicity++);};
  operator UInt_t() {return m_multiplicity;}
  UInt_t increase(Detector const & det = null)
  {
    switch (det)
    {
      case null:   return m_multiplicity++;
      case LaBr3:  return m_LaBr3_mult++;
      case paris:  return m_Paris_mult++;
      case Clover: return m_Clover_mult++;
      case Ge:     return m_Ge_Clover_mult++;
      case BGO:    return m_BGO_Clover_mult++;
      default:     return 0;
    }
  }

  UInt_t getMultiplicity(Detector const & det = null)
  {
    switch (det)
    {
      case null:   return m_multiplicity;
      case LaBr3:  return m_LaBr3_mult;
      case paris:  return m_Paris_mult;
      case Clover: return m_Clover_mult;
      case Ge:     return m_Ge_Clover_mult;
      case BGO:    return m_BGO_Clover_mult;
      default:     return 0;
    }
  }

private:
  UInt_t m_multiplicity = 0;
  UInt_t m_LaBr3_mult = 0;
  UInt_t m_Paris_mult = 0;
  UInt_t m_Clover_mult = 0;
  UInt_t m_Ge_Clover_mult = 0;
  UInt_t m_BGO_Clover_mult = 0;
};

//------------------//
//       Enums      //
//------------------//

//-----------------------------//
//      TEMPLATE FUNCTIONS     //
//-----------------------------//

//Prints home made types :
template <class... T> void print (Detector const & t, T const & ... t2)
{
  std::cout << type_str[t] << " ";
  print(t2...);
}

template <class... T> void print(Hit const & hit, T const & ... t2)
{
  print(hit);
  print(t2...);
}

void print(Hit hit)
{
  std::cout << "l : " << hit.label;
  if (hit.nrj >0)   std::cout << " nrj :  "   << hit.nrj ;
  if (hit.nrj2>0)   std::cout << " nrj2 : "   << hit.nrj2;
  // if (hit.nrj3>0)   std::cout << " nrj3 : "   << hit.nrj3;
  // if (hit.nrj4>0)   std::cout << " nrj4 : "   << hit.nrj4;
  if (hit.nrjcal>0) std::cout << " nrjcal : " << hit.nrjcal;
  std::cout << " time : " << hit.time;
  if (hit.pileup) std::cout << " pileup";
  std::cout << std::endl;
}

template<typename T>
using LabelsArray = std::array<T, 1000>;

//////////////
// IS HISTO //
//////////////


inline Bool_t is_Clover_BGO(Hit const & hit)
{
  if (hit.label>22 && hit.label<167)
  {
    if (hit.label%6 == 0 || hit.label%6 == 5) return true;
    else return false;
  }
  else {return false;}
}

inline Bool_t is_Clover_BGO(UShort_t const & label)
{
  if (label>22 && label<167)
  {
    if (label%6 == 0 || label%6 == 5) return true;
    else return false;
  }
  else {return false;}
}

Bool_t is_LaBr3(UShort_t const & _l)
{
  #ifdef FATIMA
    #if defined N_SI_120
    return (_l>199 && _l<220);
    #elif defined N_SI_121
    return (_l>199 && _l<210);
    #elif defined (N_SI_129) || defined (N_SI_85)
    return (_l == 252);
    #else
    return false;
    #endif
  #else //!FATIMA
    return false;
  #endif //FATIMA
}

Bool_t is_Paris(UShort_t const & _l)
{
  #ifdef PARIS
  return (
         (_l > 200 && _l < 209)
      || (_l > 300 && _l < 317)
      || (_l > 400 && _l < 413)
      || (_l > 500 && _l < 509)
      || (_l > 600 && _l < 617)
      || (_l > 700 && _l < 713)
     );
  #else
  return false;
  #endif //PARIS
}

Bool_t is_EDEN(UShort_t const & _l)
{
  #ifdef LICORNE
  return (_l == 500 || _l == 501);
  #else
  return static_cast<bool>(_l);
  #endif //LICORNE
}

inline Bool_t is_Clover(UShort_t const & label)
{
  return (label>22 && label<167);
}

inline Bool_t is_Clover_Ge(Hit const & hit)
{
  if (hit.label>22 && hit.label<167)
  {
    if (hit.label%6 == 1 || hit.label%6 == 2 || hit.label%6 == 3 || hit.label%6 == 4) return true;
    else return false;
  }
  else {return false;}
}

inline Bool_t is_Clover_Ge(UShort_t const & label)
{
  if (label>22 && label<167)
  {
    if (label%6 == 1 || label%6 == 2 || label%6 == 3 || label%6 == 4) return true;
    else return false;
  }
  else {return false;}
}

inline Bool_t is_RF(UShort_t const & label)
{
  return (label == 251);
}

inline Bool_t is_RF(Hit const & hit)
{
  return is_RF(hit.label);
}

inline UShort_t labelToClover(Hit const & hit)
{
  return ( (hit.label+1)/6 - 4 );
}

inline UShort_t labelToClover(UShort_t const & label)
{
  return ( (label+1)/6 - 4 );
}

std::array<uchar,167> labelToClover_fast;
void setLabelToClover_fast ()
{
  for (int i = 0; i<23; i++) labelToClover_fast[i] = -1;
  for (int i = 23; i<167; i++)
  {
    labelToClover_fast[i] = labelToClover(i);
  }
}

uchar labelToCloverCrystal(Label const & l)
{
  return (l-1)%6;
}

std::array<uchar,167> labelToCloverCrystal_fast;
void setlabelToCloverCrystal_fast ()
{
  for (int i = 23; i<167; i++)
  {
    labelToCloverCrystal_fast[i] = labelToCloverCrystal(i);
  }
}

inline UShort_t labelToCloverLabel(UShort_t const & label)
{
  return label ;
}


// inline Label labelToDSSDSectorLabel(UShort_t const & label)
// {
//   return
// }

template <class T>
Bool_t is_histo(T) {return std::is_base_of<TH1, T>::value;}

//---------------------//
//     DECLARATIONS    //
//---------------------//
Detector type_det(UShort_t const & label);

std::vector<char> isClover;
std::vector<char> isGe;
std::vector<char> isBGO;
std::vector<char> isLaBr3;
std::vector<char> isParis;
std::vector<char> isDSSD;
std::vector<char> isDSSD_Sector;
std::vector<char> isDSSD_Ring;

void setIsClover(Label const & nb_labels)
{
  for (Label i =0; i<nb_labels; i++) isClover.push_back(is_Clover(i));
}
void setIsGe(Label const & nb_labels)
{
  isGe.resize(nb_labels);
  for (Label i =0; i<nb_labels; i++) isGe[i] = is_Clover_Ge(i);
}

void setIsBGO(Label const & nb_labels)
{
  isBGO.resize(nb_labels);
  for (Label i =0; i<nb_labels; i++) isBGO[i] = static_cast<char> (is_Clover_BGO(i));
}

void setIsLaBr3(Label const & nb_labels)
{
  for (Label i =0; i<nb_labels; i++) isLaBr3.push_back(is_LaBr3(i));
}

void setIsParis(Label const & nb_labels)
{
  for (Label i =0; i<nb_labels; i++) isParis.push_back(is_Paris(i));
}

void setIsDSSD(Label const & nb_labels)
{
  isDSSD.resize(nb_labels, false);
  isDSSD_Sector.resize(nb_labels, false);
  isDSSD_Ring.resize(nb_labels, false);
  for (Label i = 0; i<nb_labels; i++)
  {
    isDSSD[i] = ( (i>799 && i<816) || (i>819 && i<836) || (i>839 && i<856) );
    isDSSD_Sector[i] = ( (i>799 && i<816) || (i>819 && i<836) );
    isDSSD_Ring[i] = ( (i>839 && i<856) );
  }
  // isDSSD[829] = false;
  // isDSSD[830] = false;
  // isDSSD_Sector[829] = false;
  // isDSSD_Sector[830] = false;
}


inline UShort_t label_to_BGO_crystal(UShort_t const & i)
{//       |  clover n°  | | crystal n° |
  return   2*((i+1)/6-4) +   (i+1)%6     ;
}

std::vector<char> labelToBGOcrystal;
void setlabelToBGOcrystal (Label const & nb_labels)
{
  labelToBGOcrystal.resize(nb_labels);
  Label label = 0;
  for (auto & e : labelToBGOcrystal)
  {
    if (isBGO[label]) e = label_to_BGO_crystal(label);
    else              e = 0;
    label++;
  }
}

inline UShort_t label_to_Paris_crystal(Label const & label)
{
  if (label<201) return 0;
  else if (label<209) return label-201;
  else if (label<317) return label-301+8;
  else if (label<413) return label-401+24;
  else if (label<509) return label-501+36;
  else if (label<617) return label-601+44;
  else if (label<713) return label-701+60;
  else return 0;
}

std::vector<char> labelToPariscrystal;
void setlabelToPariscrystal (Label const & nb_labels)
{
  labelToPariscrystal.resize(nb_labels);
  Label label = 0;
  for (auto & e : labelToPariscrystal)
  {
    if (isParis[label]) e = label_to_Paris_crystal(label);
    else              e = 0;
    label++;
  }
}

std::pair<Float_t,Float_t> sectorCoincToPos(Label const & Ring, Label const & Sector)
{
  Float_t r = 55-Ring;
  r+=gRandom->Uniform(-1,1)*0.25;

  int label_sector = (Sector<16) ? (Sector) : (Sector-4);
  Float_t phi = 0.19634975*label_sector; // 0,19634975 = 2*pi/32
  phi += 0.19634975*gRandom->Uniform(0.05,0.95);
  r = 1.5+r*(4.1-1.5)/15;
  return (std::make_pair(r*TMath::Cos(phi), r*TMath::Sin(phi)));
}

std::vector<Double_t> Ge_Labels =
{ 25 ,26 ,27 ,28,  31 ,32 ,33 ,34 , 37 ,38 ,39 ,40 , 43 ,44 ,45 ,46 , 49 ,50 ,51 ,52 , 55 ,56 ,57 ,58 ,
  61 ,62 ,63 ,64,  67 ,68 ,69 ,70 , 73 ,74 ,75 ,76 , 79 ,80 ,81 ,82 , 85 ,86 ,87 ,88 , 91 ,92 ,93 ,94 ,
  97 ,98 ,99 ,100, 103,104,105,106, 109,110,111,112, 115,116,117,118, 121,122,123,125, 127,128,129,130,
  133,134,135,136, 139,140,141,142, 145,146,147,148, 151,152,153,154, 157,158,159,160, 163,164,165,166
};

std::vector<UShort_t> m_DSSD_labels =
{
  800,801,802,803,804,805,806,807,808,809,810,811,812,813,814,815,
  820,821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,
  840,841,842,843,844,845,846,847,848,849,850,851,852,853,854,855
};

std::vector<Double_t> LaBr3_Labels =
{
  200,201,202,203,204, 205,206,207,208,209,
  210,211,212,213,214, 215,216,217,218,219
};

// #include "Analyse/ParisLabel.hpp"

void setArrays(size_t const & nb_labels)
{
  setIsClover(nb_labels);
  setIsGe(nb_labels);
  setIsBGO(nb_labels);
  setIsParis(nb_labels);
  setIsLaBr3(nb_labels);
  setIsDSSD(nb_labels);
  setlabelToBGOcrystal(nb_labels);
  setlabelToPariscrystal(nb_labels);
  setLabelToClover_fast();
  setlabelToCloverCrystal_fast();
  // ParisLabel::setArrays();
}


//--------------------//
//       CLASSES      //
//--------------------//

class pic_fit_result
{
public:
  pic_fit_result(){};

  void resize(int size)
  {
    peaks.resize(size);
    cmeasures.resize(size);
    mean.resize(size);
    sigma.resize(size);
  };

  //Getter :
  size_t size() const {return peaks.size();};

  //Getter & setter :
  Bool_t exists(UInt_t _exist = 666)
  {
    if (_exist == 666)
    {// Getter mode
      return this->exist;
    }
    else if (_exist<2)
    {// Setter mode
      exist = static_cast<bool>(_exist);
      return true;
    }
    else
    {
      print("internal error with pic_fit_result.exists()");
      return false;
    }
  };

  Bool_t too_few_counts(UInt_t _few_counts = 666)
  {
    if (_few_counts == 666)
    {//Getter condition
      return this->few_counts;
    }
    else if (_few_counts<2)
    {
      few_counts = (Bool_t) _few_counts;
      return true;
    }
    else
    {
      print("internal error with pic_fit_result.too_few_counts()");
      return false;
    }
  };

  Int_t label = 0;

  std::vector<Float_t> peaks;
  std::vector<Float_t> cmeasures;

  std::vector<Float_t> mean;
  std::vector<Float_t> sigma;

  Int_t   nb_hits = -1;
  Float_t chi2 = -1;
  Float_t parameter0 = 0;
  Float_t parameter1 = 1;
  Float_t parameter2 = 0.f;
  Float_t parameter3 = 0.f;
  Float_t scalefactor = 0.f;
  Float_t keVperADC = 0.f;

private:
  Bool_t exist = false;
  Bool_t few_counts = false;
};

using Fits   = std::vector <pic_fit_result>;


//--------------------//
// NEARLINE FUNCTIONS //
//--------------------//

void alignator(TTree * tree, Int_t *NewIndex)
{
  Int_t const NHits = tree -> GetEntries();
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Read only the time
  // tree -> SetBranchStatus("label", true);// Read only the time
  std::vector<ULong64_t> TimeStampBuffer(NHits,0);
  ULong64_t TimeStamp = 0; tree->SetBranchAddress("time", &TimeStamp);
  // UShort_t label = 0; tree->SetBranchAddress("label", &label);
  for (int i = 0; i<NHits; i++)
  {
    tree -> GetEntry(i);
    // if (i<20) std::cout << label << "\t" << TimeStamp << std::endl;
    TimeStampBuffer[i]=TimeStamp;
  }
  Int_t i = 0, j = 0;
  ULong64_t a = 0;
  NewIndex[0]=0;
	for (j=0; j<NHits;j++)
	{
  	NewIndex[j]=j;
  	a=TimeStampBuffer[j]; //Focus on this time stamp
  	i=j;
		// Find the place to insert it amongst the previously sorted
  	while((i > 0) && (TimeStampBuffer[NewIndex[i-1]] > a))
  	{
    	NewIndex[i]=NewIndex[i-1];
    	i--;
  	}
  	NewIndex[i]=j;
	}
  tree -> SetBranchStatus("*", true); //enables again the whole tree
}

void test_alignator(TTree *tree, Int_t* NewIndex= nullptr, bool useNewIndex = false)
{
  tree -> SetBranchStatus("*", false);// Disables all the branches readability
  tree -> SetBranchStatus("time", true);// Eneables to read only the time
  ULong64_t TimeStamp; tree->SetBranchAddress("time", &TimeStamp);
  ULong64_t PrevTimeStamp = 0; int j = 0;
  int maxIt = tree -> GetEntries();
  for (int i = 0; i < maxIt; i++)
  {
    if (useNewIndex) j = NewIndex[i] ;
    else j = i;
    tree -> GetEntry(j);
    if ((Long64_t) (TimeStamp - PrevTimeStamp) < 0)
    std::cout << j << " -> " << (Long64_t) (TimeStamp - PrevTimeStamp) << std::endl;
    PrevTimeStamp = TimeStamp;
  }
  tree -> SetBranchStatus("*", true); //enables again the whole tree
}

Timeshift arrayTimeShift(std::string const & deltaT_filename = "")
{//vector<int>
  Timeshift timeshift(0);
  std::ifstream inputfile(deltaT_filename, std::ifstream::in);
  if (!inputfile.is_open())
  {
    print("Could not open the time shifts file - '", deltaT_filename, "'");
    return timeshift;
  }
  else if (file_is_empty(inputfile))
  {
    print("time shift file - '", deltaT_filename, "' is empty !");
    return timeshift;
  }
  // ----------------------------------------------------- //
  //First reading of the file : extract the maximum label to be the size of the vector
  UShort_t size = 0; std::string line = ""; int label = 0,  deltaT = 0;
  while (getline(inputfile, line))
  {//First extract the number of lines
     std::istringstream iss(line);
     iss >> label;
     if (size<label) size = label;
  }
  timeshift.resize(size+1);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  //Second reading : fill the vector
  while (getline(inputfile, line))
  {//Then fill the array
    std::istringstream iss(line);
    iss >> label >> deltaT;
    timeshift[label] = deltaT;
  }
  inputfile.close();
  return timeshift;
}

Labels arrayID(std::string const & IDpid)
{//vector<string>
  Labels labels(1,"");
  std::ifstream inputfile(IDpid, std::ifstream::in);
  if (!inputfile.is_open())
    {std::cerr << "Could not open the array file - '"
    << IDpid << "'" << std::endl;return labels;}
  else if (file_is_empty(inputfile))
    {std::cerr << "time shift file - '" << IDpid
    << "' is empty !" << std::endl;return labels;}
  // ----------------------------------------------------- //
  //First reading of the file : extract the maximum label to be the size of the vector
  UShort_t size = 0; std::string line = "", name = ""; int label = 0;
  std::string oneline;
  while (inputfile.good())
  {
    getline(inputfile, oneline);
    std::istringstream is(oneline);
    is>>label;
    if (size<label) size = label;
  }
  // ----------------------------------------------------- //
  //Second reading : fill the vector
  labels.resize(size+1,"");
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  for (size_t i = 0; i < labels.size(); i++)
  {
    getline(inputfile, oneline);
    std::istringstream is(oneline);
    is>>label>>name;
    if (oneline.size()>1) labels[label] = name;
    // else labels[label] = std::to_string(i);
  }
  inputfile.close();
  return labels;
}

bool is_DSSD(UShort_t const & label)
{
  #ifdef USE_DSSD
  return (label > 800 && label < 900);
  #else
  return false;
  #endif
}

Detector type_det(UShort_t const & label)
{
  if      (isGe    [label]) return Ge;
  else if (isBGO   [label]) return BGO;
  else if (isLaBr3 [label]) return LaBr3;
  else if (isParis [label]) return paris;
  else if (isDSSD  [label]) return dssd;
  else if (is_RF   (label)) return RF;
  else if (is_EDEN (label)) return EDEN;
  else                      return null;
}

inline Detector type_det(Hit const & hit) {return type_det(hit.label);}

Detector type_det(std::string name)
{
  std::string type = name.substr(name.find_last_of("_")+1);
  if (type == "raw"|| type == "calib")
  {
    name = name.substr(0,name.find_last_of("_"));
    type = name.substr(name.find_last_of("_")+1);
  }
  if ( (type=="blue") || (type=="red") || (type=="green") || (type=="black")) return Ge;
  else if ( (type == "BGO1") || (type == "BGO2") || (type == "BGO") ) return BGO;
  else if (type == "LaBr3") return LaBr3;
  else if (name.substr(0,name.find("_")) == "PARIS") return paris;
  else if (lastPart(name,'_') == "DSSD") return dssd;
  else return null;
}

UShort_t checkThreads(UShort_t nOfThreads, UInt_t const &numberFiles)
{
  if(nOfThreads > numberFiles)
  {
    nOfThreads = numberFiles;
    std::cout << "Number of threads too large (too few files to be processed) -> reset to " << nOfThreads << std::endl;
  }
  return nOfThreads;
}

void checkThreadsNb(UShort_t & nOfThreads, UInt_t const &numberFiles)
{
  if(nOfThreads > std::thread::hardware_concurrency())
  {
    nOfThreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads too large (hardware) -> reset to " << nOfThreads << std::endl;
  }
  if(nOfThreads > numberFiles)
  {
    nOfThreads = numberFiles;
    std::cout << "Number of threads too large (too few files to be processed) -> reset to " << nOfThreads << std::endl;
  }
}

std::vector<Detector> Type_det;

void set_type_det(Labels const & labels)
{
  Type_det.resize(labels.size());
  for (size_t l = 0; l<Type_det.size(); l++)
  {
    Type_det[l] = type_det(l);
  }
}

bool isTripleAlpha(std::string const & source_name)
{
  return (source_name == "3-alpha" || source_name == "3alpha"  ||
          source_name == "triple-alpha" || source_name == "triplealpha");
}

// MATHS :

// Doppler correction :

std::array<Label,96> theta_subring =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

NRJ doppler_shift(NRJ const & nrj, Float_t const & theta, Float_t const & velocity)
{
  return nrj/(1+velocity/30*cos(theta));
}

// FASTER2ROOT FUNCTIONS

class Trigger
{
public:
  Trigger(){};
  void addCondition(std::string condition)
  {
    CLM.push_back({0,0,0});
    CL.push_back({0,0});
    for (size_t i = 0; i<condition.size(); i++)
    {
      if (condition[i] == 'C')
      {
        CLM[n_cond][0] = (Int_t)(condition[i+1] - '0');
        CL [n_cond][0] = (Int_t)(condition[i+1] - '0');
      }
      else if (condition[i] == 'L')
      {
        CLM[n_cond][1] = (Int_t)(condition[i+1] - '0');
        CL [n_cond][1] = (Int_t)(condition[i+1] - '0');
      }
      else if (condition[i] == 'M')
      {
        CLM[n_cond][2] = condition[i+1] - '0';
      }
    }
    CL [n_cond][0]-=1;
    CL [n_cond][1]-=1;
    n_cond++;
  }

  Bool_t operator() (Int_t C , Int_t L, Int_t M) {return check(C,L,M);}
  Bool_t operator() (Int_t C , Int_t L)          {return check(C,L)  ;}

  Bool_t check(Int_t C, Int_t L, Int_t M)
  {
    for (size_t i = 0; i<n_cond; i++)
    {
      if(C>CLM[i][0] && L>CLM[i][1] && M>CLM[i][2] ) return true; //If at least one condition is fulfilled
    }
    return false;
  }
  Bool_t check(Int_t C, Int_t L)
  {
    //If at least one condition is fulfilled
    for (size_t i = 0; i<n_cond; i++)
    {
      if(C>CL[i][0] && L>CL[i][1]) return true;
    }
    return false;
  }
private:
  size_t n_cond = 0;
  std::vector<std::array<Int_t,3>> CLM; // vector of AND conditions {C,L,M};
  std::vector<std::array<Int_t,2>> CL; // vector of AND conditions {C,L,M};
};

inline bool gate(Hit const & hit, Float_t const & E_min, Float_t const & E_max)
{
  return (hit.nrjcal>E_min && hit.nrjcal<E_max);
}

inline bool gate(Float_t const & E, Float_t const & E_min, Float_t const & E_max)
{
  return (E>E_min && E<E_max);
}

std::pair<Float_t,Float_t> calculate_DSSD_pos(int const & ring, int const & sector)
{// The labels must be Label-800 to range from 0 to 55

  Float_t r = 1.5 + (55.-ring+gRandom->Uniform(-0.4,0.4))*(4.1-1.5)/15; // r_min + (r_ring+random)*(r_max-r_min)/nb_rings

  Float_t phi = 0.19634975 * ( static_cast<int>((sector<16)?(sector):(sector-4)) + gRandom->Uniform(0.05, 0.95)); // (2*pi/32) * (ith_sector + random)

  return std::make_pair(r*TMath::Cos(phi),r*TMath::Sin(phi));
}

template < class T >
std::istringstream & operator >> (std::istringstream & is, std::vector<T>& v)
{
  T t;
  is >> t;
  v.push_back(t);
  return is;
}

#ifdef PARIS
std::map<std::string,int> paris_vmaxchan =
{
  {"PARIS_BR1D1", 48500},  {"PARIS_BR1D2", 64000},  {"PARIS_BR1D3", 51000},  {"PARIS_BR1D4", 63500},
  {"PARIS_BR1D5", 66000},  {"PARIS_BR1D6", 71000},  {"PARIS_BR1D7", 65000},  {"PARIS_BR1D8", 72500},
  {"PARIS_BR2D1", 65500},  {"PARIS_BR2D2", 70500},  {"PARIS_BR2D3", 44500},  {"PARIS_BR2D4", 59500},
  {"PARIS_BR2D5", 53000},  {"PARIS_BR2D6", 59000},  {"PARIS_BR2D7", 59000},  {"PARIS_BR2D8", 55500},
  {"PARIS_BR2D9", 52000},  {"PARIS_BR2D10", 54000}, {"PARIS_BR2D11", 49000}, {"PARIS_BR2D12", 60000},
  {"PARIS_BR2D13", 48500}, {"PARIS_BR2D14", 70000}, {"PARIS_BR2D15", 42000}, {"PARIS_BR2D16", 60500},
  {"PARIS_BR3D1", 72000},  {"PARIS_BR3D2", 59000},  {"PARIS_BR3D3", 49000},  {"PARIS_BR3D4", 60000},
  {"PARIS_BR3D5", 60000},  {"PARIS_BR3D6", 60000},  {"PARIS_BR3D7", 68000},  {"PARIS_BR3D8", 44000},
  {"PARIS_BR3D9", 61000},  {"PARIS_BR3D10", 58500}, {"PARIS_BR3D11", 45000}, {"PARIS_BR3D12", 56000},
  {"PARIS_FR1D1", 61000},  {"PARIS_FR1D2", 53000},  {"PARIS_FR1D3", 62500},  {"PARIS_FR1D4", 63000},
  {"PARIS_FR1D5", 54000},  {"PARIS_FR1D6", 65000},  {"PARIS_FR1D7", 69500},  {"PARIS_FR1D8", 0},
  {"PARIS_FR2D1", 59500},  {"PARIS_FR2D2", 54000},  {"PARIS_FR2D3", 62500},  {"PARIS_FR2D4", 62000},
  {"PARIS_FR2D5", 66000},  {"PARIS_FR2D6", 67000},  {"PARIS_FR2D7", 53000},  {"PARIS_FR2D8", 63000},
  {"PARIS_FR2D9", 51000},  {"PARIS_FR2D10", 62000}, {"PARIS_FR2D11", 55000}, {"PARIS_FR2D12", 49000},
  {"PARIS_FR2D13", 43500}, {"PARIS_FR2D14", 67000}, {"PARIS_FR2D15", 68000}, {"PARIS_FR2D16", 60000},
  {"PARIS_FR3D1", 59500},  {"PARIS_FR3D2", 43500},  {"PARIS_FR3D3", 50000},  {"PARIS_FR3D4", 62500},
  {"PARIS_FR3D5", 54000},  {"PARIS_FR3D6", 60500},  {"PARIS_FR3D7", 60500},  {"PARIS_FR3D8", 55000},
  {"PARIS_FR3D9", 62500},  {"PARIS_FR3D10", 59500}, {"PARIS_FR3D11", 50000}, {"PARIS_FR3D12", 58500}
};
#endif //PARIS




#endif //UTILS_H_CO
