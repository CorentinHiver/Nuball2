#ifndef TIMESHIFTS_H
#define TIMESHIFTS_H

#include "../libCo.hpp"

#include "Detectors.hpp"
#include "FasterReader.hpp"
#include "FilesManager.hpp"
#include "MTList.hpp"
#include "MTTHist.hpp"
#include "Hit.h"
#include "RF_Manager.hpp"
#include "Event.hpp"
#include "CoincBuilder2.hpp"
#include "Timer.hpp"
#include "Performances.hpp"

#include "../MTObjects/MTObject.hpp"

/* 
 * Author : Corentin Hiver
 * Module part of the NearLine software
 * The timestamps are in picoseconds, so are the timeshifts
 * Goal 1 : calculate the time shifts between detectors
 * How : 
 * * Use one detector as reference, preferencially with good time resolution
 * * Create coincidences using the CoincBuilder event builder
 * * Calculate the difference in timestamp as follow : ref_time - detector_time
 * * Finally, output the (.dT) file containing the time shifts of all the detectors
 * * Also produces a .root file containing the raw and corrected time spectra.
 * * This module includes multithreading management using MTObject::parallelise_function(function, parameters...)
 * Goal 2 : read a timeshift file (.dT) previously calculated by this module
 * Goal 3 : apply the timeshifts by calling the operator[] : time_correct = time_shifted + timeshifts[label];
 * Example at the end
*/

using Shift_t = Long64_t;

class Timeshifts
{
public:
  Timeshifts() {Detectors::Initialize();}
  Timeshifts(std::string const & filename) : m_filename(filename) {this -> load(filename);}

  bool setParameters(std::string const & parameter);
  bool load(std::string const & filename);
  void setDetectorsList(DetectorsList const & detList) {m_detList = detList;}
  void setDetectorsList(DetectorsList * detList) {m_detList = *detList;}

  bool setTimeWindow(int const & timewindow_ns);
  bool setTimeWindow(std::string const & timewindow_ns);
  bool setTimeReference(int const & timeRef_label);
  bool setTimeReference(std::string const & timeRef_name);
  void setOutDir(std::string const & outDir);
  void verbose(bool const & _verbose = true) {m_verbose = _verbose;}
  void setOutRoot(std::string const & outroot);
  void setOutData(std::string const & outdata);
  void setMult(uchar const & _max_mult, uchar const & _min_mult) {m_max_mult = _max_mult; m_min_mult = _min_mult;}
  void setMaxHits(ulong const & max_hits = -1) {m_max_hits = max_hits;}

  bool InitParameters();
  bool calculate(std::string const & folder, int const & nb_files = -1);
  static void treatFilesMT(Timeshifts & ts, MTList<std::string> & files_MT);
  void treatFile(std::string const & filename);
  void analyse();
  void write();
  void Print() {print(m_timeshifts);}

  TH1F* shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit = "ps");

  Shift_t const & operator[] (int const & i) const {return m_timeshifts[i];}

  std::vector<Shift_t> const & get() const {return m_timeshifts;}
  Shift_t const & get(int const & i) const {return m_timeshifts[i];}

private:
  bool m_ok = false;
  bool m_verbose = false;
  std::string m_filename = "";

  std::vector<Shift_t> m_timeshifts;

  float m_timewindow = 1500000;
  float m_timewindow_ns = 1500;
  uchar m_max_mult = 2;
  uchar m_min_mult = 2;
  ushort m_timeRef_label = 252;
  std::string m_timeRef_name = "R1A9_FATIMA_LaBr3";
  ulong m_max_hits = -1;

  // This map holds the bin width of the histograms in ps (e.g. for LaBr3 there is one bin every 100 ps)
  std::map<std::string, float> m_rebin = { {"LaBr3", 100}, {"Ge", 1000}, {"BGO", 500}, {"EDEN", 500}, {"RF", 100}, {"paris", 100}, {"dssd", 1000}};

  std::string m_outDir = "";
  std::string m_ts_outdir = "Timeshifts/";
  std::string m_outPath;
  std::string m_outRoot = "timeshifts.root";
  std::string m_outData = "";

  DetectorsList m_detList;
  FilesManager m_files;
  Performances m_perf;

  Vector_MTTHist<TH1F> m_histograms;
  std::vector<std::unique_ptr<TH1F>> m_histograms_corrected;
#ifdef USE_RF
  MTTHist<TH1F> m_histo_ref_VS_RF;
  Vector_MTTHist<TH1F> m_histograms_VS_RF;
  std::vector<std::unique_ptr<TH1F>> m_histograms_corrected_RF;
  std::unique_ptr<TH1F> m_histo_RF_corrected;
#endif //USE_RF
  MTTHist<TH1F> m_EnergyRef;
  MTTHist<TH2F> m_EnergyRef_bidim;
};

bool Timeshifts::load(std::string const & filename)
{
  std::ifstream inputfile(filename, std::ifstream::in);
  if (!inputfile.is_open())
  {
    print("Could not open the time shifts file - '", filename, "'");
    m_ok = false;
    return m_ok;
  }
  else if (file_is_empty(inputfile))
  {
    print("Time shift file - '", filename, "' is empty !");
    m_ok = false;
    return m_ok;
  }
  // ----------------------------------------------------- //
  UShort_t size = 0; std::string line = ""; int label = 0,  deltaT = 0;
  while (getline(inputfile, line))
  {//First extract the maximum label
    std::istringstream iss(line);
    iss >> label;
    if (size<label) size = label;
  }
  // ----------------------------------------------------- //
  //Second reading : fill the vector
  m_timeshifts.resize(size+1);
  inputfile.clear();
  inputfile.seekg(0, inputfile.beg);
  while (getline(inputfile, line))
  {//Then fill the array
    std::istringstream iss(line);
    iss >> label >> deltaT;
    m_timeshifts[label] = deltaT;
  }
  inputfile.close();
  print("Timeshifts extracted from", filename);
  m_ok = true;
  return m_ok;
}

bool Timeshifts::setParameters(std::string const & parameter)
{
  std::string temp;
  std::istringstream is(parameter);
  while(is >> temp)
  {
    if (temp == "timewindow:" || temp == "time_window:")
    {
      is >> m_timewindow_ns;
      if (!this -> setTimeWindow(m_timewindow_ns)) return false;
    }
    else if (temp == "time_reference:" || temp == "time_ref:")
    {
      is >> m_timeRef_name;
      if (!this -> setTimeReference(m_timeRef_name)) return false;
    }
    else if (temp == "outDir:")  {is >> m_outDir; setOutDir(m_outDir);}
    else if (temp == "outRoot:") {is >> m_outRoot; setOutRoot(m_outRoot);}
    else if (temp == "outData:") {is >> m_outData; setOutData(m_outData);}
    else if (temp == "mult:")    {is >> m_min_mult >> m_max_mult; setMult(m_min_mult, m_max_mult);} //by default
    else if (temp == "verbose")  {verbose(true);}
    else {std::cout << std::endl << "ATTENTION, parameter " << temp << " not recognized !" << std::endl << std::endl; return false;}
  }
  if (m_outData == "") 
  {
    m_outData = removeExtension(m_outRoot)+".dT";
    print("Name of the output timeshifts file automatically set to", m_outData);
  }
  return true;
}

bool Timeshifts::InitParameters()
{
  if (m_ok) return true; // To prevent multiple initializations
  m_outPath = m_outDir+m_ts_outdir;
  create_folder_if_none(m_outPath);
  if (!folder_exists(m_outPath, true)) return (m_ok = false);
  if (extension(m_outData) != "dT") m_outData = removeExtension(m_outData)+".dT";
  if (m_detList.size() == 0)
  {
    print("PLEASE LOAD THE ID FILE IN THE TIMESHIFT MODULE");
    return (m_ok = false);
  }

  m_EnergyRef.reset("Energy_spectra", "Energy_spectra", 10000, 0, 1000000);
  m_EnergyRef_bidim.reset("Energy_VS_time", "Energy VS time", 100,-1,1, 1000,0,1000000);

  m_histograms.resize(m_detList.size());
#ifdef USE_RF
  m_histograms_VS_RF.resize(m_detList.size());
#endif //USE_RF
  for (ushort label = 0; label<m_detList.size(); label++)
  {
    auto const & name = m_detList[label];
    if (name == "") continue;

    auto const & type = Detectors::type(label);

  #ifdef USE_RF
    if (label == 251) 
    {
      m_histo_ref_VS_RF.reset( "RF_calculated", "RF_calculated", m_timewindow/m_rebin["RF"], -m_timewindow/2, m_timewindow/2);
    }
    m_histograms_VS_RF[label].reset((name+"_RF").c_str(), name.c_str(), m_timewindow/m_rebin[type], -m_timewindow/2, m_timewindow/2);
  #endif //USE_RF
    m_histograms[label].reset(name.c_str(), name.c_str(), m_timewindow/m_rebin[type], -m_timewindow/2, m_timewindow/2);
  }
  return (m_ok = true);
}

bool Timeshifts::calculate(std::string const & folder, int const & nb_files)
{
  Timer timer;
  if (!InitParameters()) 
  {
    print("Someting went wrong in the timeshifts initialization...");
    return false;
  }

  m_files.addFolder(folder, nb_files);

  if (MTObject::ON)
  {// If multithreading, treat each data file of the folder in parallel
    print("Calculating timeshifts with", MTObject::getThreadsNb(),"threads");
    // The FileManager object isn't thread safe. 
    // That is why one has to encapsulate the files list inside a MTList (Multi-Threaded List) :
    MTList<std::string> files_MT(m_files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, *this, files_MT);
  }
  else
  {// If no multithreading, treat each data file sequentially
    print("Calculating timeshifts without multithreading");
    std::string filename;
    while (m_files.nextFileName(filename))
    {
      treatFile(filename);
    }
  }

  // Once the histograms have been filled, fit the peaks
  this -> analyse();

  // Write down the timeshitfs file in a .dT file, and the root histograms in a .root file
  write();

  print("Timeshifts calculated in", timer(), timer.unit());

  return true;
}

void Timeshifts::treatFilesMT(Timeshifts & ts, MTList<std::string> & files_MT)
{
  // This method has to be static in order to be multithreaded.
  // Thus, the only way it can be "link" to the class (i.e. having access to its public members) is to pass the class itself as an argument.
  // This is done by passing "Timeshifts & ts" in arguments and fill this argument with "*this" in the piece of code that calls it
  // Finally, we can use the same function as the not multithreaded piece of code.
  std::string filename;
  while (files_MT.getNext(filename))
  {
    ts.treatFile(filename);
  }
}

void get_first_RF_of_file(FasterReader & reader, Hit & hit, RF_Manager & rf)
{
  // int i = 0;
  while (reader.Read() && !(hit.label == RF_Manager::label)) continue;
  rf.setHit(hit);
}

void Timeshifts::treatFile(std::string const & filename)
{
  Hit hit;
  FasterReader reader(&hit, filename);
  Event event;
  CoincBuilder2 coincBuilder(&event, m_timewindow);

  RF_Manager rf;
  uint counter = 0;
  
  bool maxHitsToRead = (m_max_hits>0);

// #ifdef USE_RF
//   get_first_RF_of_file()

  while(reader.Read() && ((maxHitsToRead) ? (counter<m_max_hits) : true) )
  {
    if ( (counter%static_cast<uint>(1.E+6)) == 0 ) print(counter);
    counter++;

  #ifdef USE_RF
    if(isRF[hit.label]) {rf.last_hit = hit.time; rf.period = hit.nrj;}
    else if (hit.label == m_timeRef_label)
    {
      hit.nrjcal = hit.nrj;
      m_histo_ref_VS_RF.Fill(rf.pulse_ToF(hit.time));
    }
    else
    {
      m_histograms_VS_RF[hit.label].Fill(rf.pulse_ToF(hit.time, USE_RF));
    }
  #endif //USE_RF

    if (coincBuilder.build(hit))
    {
      auto mult = event.size();
      if (mult >= m_max_mult &&  mult <= m_min_mult) continue;
      for (size_t i = 0; i < mult; i++)
      {
        if ( event.labels[i] == m_timeRef_label ) // if the ref detector triggered
        {
          auto const & refPos = i;
          Shift_t deltaT = 0;
          m_EnergyRef.Fill(event.nrjs[refPos]);
          for (uchar j = 0; j<mult; j++)
          {
            if (event.labels[j] == m_timeRef_label) continue; // To reject the time spectra of the time reference
            deltaT = event.times[refPos]-event.times[j];
            m_histograms[event.labels[j]].Fill(deltaT); //stored in ps
}}}}}}

int getBin0(TH1F* spectra)
{
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  std::vector<double> lowEdges(bins);
  spectra -> GetXaxis() -> GetLowEdge(lowEdges.data());
  int bin0 = 0;
  while(lowEdges[bin0] < 0) bin0++;
  return bin0;
}

bool getMeanPeak(TH1F* spectra, double & mean)
{
  // Declaration :
  TF1 *gaus_pol0, *fittedPic;
  float pospic, amppic, dump_sigma;
  float cte, Mean, sigma;

  // Histogram characteristics :
  auto const bins = spectra -> GetXaxis() -> GetNbins();
  auto const xmax = spectra -> GetXaxis() -> GetXmax();
  auto const xmin = spectra -> GetXaxis() -> GetXmin();

  auto const xPerBin = (xmax-xmin)/bins;
  auto const bin0 = getBin0(spectra);
  
  // Extract dump parameters :
  amppic = spectra -> GetMaximum();
  pospic = static_cast<double>( (spectra->GetMaximumBin() - bin0)*xPerBin );
  dump_sigma = static_cast<double>( (spectra->FindLastBinAbove(amppic/2) - spectra->FindFirstBinAbove(amppic/2)) * xPerBin/2 );

  // Fits the peak :
  gaus_pol0 = new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*dump_sigma,pospic+20*dump_sigma);
  gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
  gaus_pol0 -> SetRange(pospic-dump_sigma*20,pospic+dump_sigma*20);
  spectra -> Fit(gaus_pol0,"R+q");

  // Extracts the fitted parameters :
  fittedPic = spectra -> GetFunction("gaus+pol0");
  if (!fittedPic) return false; // Eliminate non existing fits, when not enough statistics fit doesn't converge
  cte = fittedPic -> GetParameter(0);
  mean = Mean = fittedPic -> GetParameter(1);
  sigma = fittedPic -> GetParameter(2);

  if (false) print("cte", cte, "Mean", Mean, "sigma", sigma);

  return true;
}

void Timeshifts::analyse()
{
  print("Get the timeshifts from the time spectra");
  std::vector<float> array_labels(m_detList.size());
  for (size_t i = 0; i<array_labels.size(); i++) array_labels[i] = i;
  // std::vector<float> array_resolutions(m_detList.size(),0);
  std::ofstream outDeltaTfile(m_outPath+m_outData, std::ios::out);

#ifdef USE_RF
  // Calculate the RF timeshift first :
  m_histo_ref_VS_RF.Merge();
  m_timeshifts[RF_Manager::label] = static_cast<Shift_t>(( m_histo_ref_VS_RF->GetMaximumBin() - (m_histo_ref_VS_RF -> GetNbinsX()/2) ) * m_rebin["RF"]);
#endif //USE_RF

  for (size_t label = 0; label<m_detList.size(); label++)
  {
    auto const & type = Detectors::type(label);
    if (type == "null") continue;
    m_histograms[label].Merge();
    m_histograms_VS_RF[label].Merge();

  #ifdef USE_RF
    if (label == RF_Manager::label)
    {
      if (m_verbose) print("RF :", m_timeshifts[RF_Manager::label], "with", m_histo_ref_VS_RF->GetMaximum(), "bins in peak");
      m_histo_RF_corrected.reset(shiftTimeSpectra(m_histo_ref_VS_RF, 251));
      outDeltaTfile << label << "\t" << m_timeshifts[RF_Manager::label] << std::endl;
      continue;
    }
  #endif //USE_RF

    if (!m_histograms[label].exists()) continue; // Eliminate empty histograms
    if (m_verbose) print(m_detList[label]);

  #ifdef LICORNE
    if (type == "Eden")
    {
      outDeltaTfile << label << "\t" << 0;
      continue;
    }
  #endif //LICORNE

  #ifdef USE_DSSD
    if (type == "dssd")
    {
    #ifdef USE_RF
      auto amppic = m_histograms_VS_RF[label] -> GetMaximum();
      float zero = ( m_histograms_VS_RF[label] -> FindFirstBinAbove(amppic/2) - (m_histograms_VS_RF[label] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
      outDeltaTfile << label << "\t" << static_cast<float>(m_timeshifts[RF_Manager::label]-zero) << std::endl;
      if (m_verbose) print("Edge :", zero, "with", static_cast<int>(m_histograms_VS_RF[label] -> GetMaximum()), "counts in peak");

    #else //NO USE_RF
      auto amppic = m_histograms[label] -> GetMaximum();
      auto pospic = ( m_histograms[label] -> FindLastBinAbove(amppic*0.8) - (m_histograms[label] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
      outDeltaTfile << label << "\t" << pospic << std::endl;
      if (m_verbose) print( "mean : ", m_histograms[label] -> GetMean(), "with", (int) m_histograms[label] -> GetMaximum(), "counts in peak");
    #endif //USE_RF
    
    continue;
    }
  #endif

    if (label == m_timeRef_label) m_timeshifts[label] = 0; // Force the reference detector to be at 0;
    else
    {
      double mean = 0;
      if (getMeanPeak(m_histograms[label], mean)) m_timeshifts[label] = static_cast<Shift_t> (-mean);
    }

    outDeltaTfile << label << "\t" << m_timeshifts[label] << std::endl;
  
  }

  // Now create the corrected time spectra :
  for (size_t label = 0; label<m_histograms.size(); label++)
  {
    auto & histo = m_histograms[label];
    if (histo) 
    {
      m_histograms_corrected[label] . reset(shiftTimeSpectra(histo, label));
    }

  #ifdef USE_RF
    auto & histo_RF = m_histograms_VS_RF[label];
    if (histo_RF) m_histograms_corrected_RF[label] . reset(shiftTimeSpectra(histo_RF, label));
  #endif //USE_RF
  }

  outDeltaTfile.close();

  std::cout << "Timeshifts data written to " << m_outPath+m_outData << std::endl;
}

void Timeshifts::write()
{
  std::unique_ptr<TFile> outFile(TFile::Open((m_outPath+m_outRoot).c_str(),"RECREATE"));
  if (outFile.get() == nullptr) {print("Cannot open file ", m_outPath+m_outRoot, " !!!\nAbort !");return;}

  m_EnergyRef.Write();

#ifdef USE_RF
  m_histo_ref_VS_RF.Write();
  if (THist_exists(m_histo_RF_corrected.get())) m_histo_RF_corrected->Write();
#endif //USE_RF

  for (auto & histo : m_histograms) histo.Write();
  for (auto & histo : m_histograms_VS_RF) histo.Write();
  for (auto & histo : m_histograms_corrected) if (THist_exists(histo.get())) histo->Write();
  for (auto & histo : m_histograms_corrected_RF) if (THist_exists(histo.get())) histo->Write();

  outFile -> Write();
  outFile -> Close();

  std::cout << "Timeshifts root file saved to " << m_outPath+m_outRoot << std::endl;
}

bool Timeshifts::setTimeWindow(int const & timewindow_ns) 
{
  if (timewindow_ns == 0) {std::cout << "NO TIME WINDOW !!" << std::endl; return false; }
  m_timewindow_ns = timewindow_ns;
  m_timewindow = m_timewindow_ns*1000;
  print("Extracting timeshifts with a time window of ", m_timewindow_ns, " ns");
  return true;
}

bool Timeshifts::setTimeWindow(std::string const & timewindow_string) 
{
  auto const timewindow_ns = stoi(timewindow_string);
  return setTimeWindow(timewindow_ns);
}

bool Timeshifts::setTimeReference(int const & timeRef_label)
{
  m_timeRef_label = timeRef_label;
  m_timeRef_name = m_detList[m_timeRef_label];
  m_timeshifts.resize(m_detList.size(), 0);
  m_histograms_corrected.resize(m_detList.size());
  m_histograms_corrected_RF.resize(m_detList.size());
  m_histograms.resize(m_detList.size());
  std::cout << "Reference detector set to be " << m_timeRef_name << " (n°" << m_timeRef_label << ")" << std::endl;
  return true;
}

bool Timeshifts::setTimeReference(std::string const & timeRef_name)
{
  m_timeRef_name = timeRef_name;
  if (timeRef_name == "") {std::cout << "NO TIME REFERENCE !!" << std::endl; return false; }
  // If the string contains the label number then convert the string to int (stoi)
  // If the string contains the detector name then extract the label number from the detectors list (m_detList)
  return setTimeReference( (isNumber(timeRef_name)) ? std::stoi(timeRef_name) : m_detList.getLabel(timeRef_name));
}

void Timeshifts::setOutDir(std::string const & outDir)
{
  m_outDir = outDir;
  makePath(m_outDir);
}

void Timeshifts::setOutRoot(std::string const & outroot) 
{
  m_outRoot = outroot; 
  if (extension(m_outRoot) != "root") 
  {
    m_outRoot = removeExtension(m_outRoot)+"root";
  }
}

void Timeshifts::setOutData(std::string const & outdata)
{
  m_outData = outdata;
  if (extension(m_outRoot) != "dT") 
  {
    m_outRoot = removeExtension(m_outRoot)+"dT";
  }
}

TH1F* Timeshifts::shiftTimeSpectra(TH1F* histo, Label const & label, std::string const & unit)
{
  std::string name = histo->GetName();
  if (name == "")  return (new TH1F());
  
  auto axis = histo -> GetXaxis();

  auto bins    = axis -> GetNbins();
  auto timeMax = axis -> GetXmax ();
  auto timeMin = axis -> GetXmin ();

  auto binToTime = (timeMax-timeMin)/bins;
  auto time_to_ps = 1;

  if (unit != "ps")
  {
    if (unit == "ns") time_to_ps*=1000;
    else if (unit == "ticks") time_to_ps*=128; // Deprecated
    else print("UNKOWN TIME UNIT", unit);
    return nullptr;
  }

  auto corrected_histo = static_cast<TH1F*>(histo -> Clone((name+"_corrected").c_str()));

  for (int bin = 1; bin<bins; bin++)
  {
    auto const time_ps = time_to_ps*(binToTime*bin + timeMin); // time_to_ps * (coeff*bin + offset_time)
    auto const new_time_ps = time_ps + m_timeshifts[label];
    auto const new_time = new_time_ps/time_to_ps;
    int const newbin = ( new_time - timeMin ) / binToTime; // ( (time_ps/time_to_ps) - offset_time) / coeff

    if( newbin>0 && newbin < bins )
    {
      corrected_histo -> SetBinContent(newbin, histo->GetBinContent(bin));
    }
  }
  return corrected_histo;
}


#endif //TIMESHIFTS_H
