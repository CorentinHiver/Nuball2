#ifndef TIMESHIFTS_H
#define TIMESHIFTS_H

#include <libCo.hpp>
#include <Detectors.hpp>
#include <FasterReader.hpp>
#include <FilesManager.hpp>
#include <MTList.hpp>
#include <MTObject.hpp>
#include <MTTHist.hpp>
#include <Hit.h>
#include <RF_Manager.hpp>
#include <Event.hpp>
#include <CoincBuilder2.hpp>

/* 
 * Author : Corentin Hiver
 * Module part of the NearLine software
 * Goal 1 : calculate the time shifts between detectors
 * How : 
 * * Use one detector as reference, preferencially with good time resolution
 * * Create coincidences using the CoincBuilder event builder
 * * Calculate the difference in timestamp as follow : ref_time - detector_time
 * * Finally, output the (.dT) file containing the time shifts of all the detectors
 * Goal 2 : read a timeshift file (.dT) previously calculated by this module
 * Goal 3 : apply the timeshifts by calling the operator[] : time_correct = time_shifted + timeshifts[label];
 * This module includes multithreading management using MTObject::parallelise_function(function, parameters...) method
*/

class Timeshifts
{
public:
  Timeshifts() {Detectors::Initialize();}
  Timeshifts(std::string const & filename) : m_filename(filename) {this -> load(filename);}
  bool setParameters(std::string const & parameter);
  bool load(std::string const & filename);
  void setDetectorsList(DetectorsList const & detList) {m_detList = detList;}
  void setDetectorsList(DetectorsList * detList) {m_detList = *detList;}

  bool InitializeParam();
  bool calculate(std::string const & folder, int const & nb_files = -1);
  static void treatFilesMT(MTList<std::string> & files_MT, Timeshifts & ts);
  void treatFile(std::string const & filename);
  void analyse();
  void write();
  void Print() {print(m_timeshifts);}

  Long64_t const & operator[] (int const & i) const {return m_timeshifts[i];}

  std::vector<Long64_t> const & get() const {return m_timeshifts;}

private:
  bool m_ok = false;
  bool m_verbose = false;
  std::string m_filename = "";

  std::vector<Long64_t> m_timeshifts;

  float m_timewindow = 1500000;
  float m_timewindow_ns = 1500000;
  uchar m_max_mult = 2;
  uchar m_min_mult = 2;
  ushort m_timeRef_label = 252;
  std::string m_timeRef_name = "R1A9_FATIMA_LaBr3";
  std::map<std::string, Float_t> m_rebin = { {"LaBr3",100}, {"Ge",1000}, {"BGO",500}, {"EDEN",500}, {"RF",100}, {"paris",100}, {"dssd",1000}};

  std::string m_outdir = "";
  std::string m_ts_outdir = "Timeshifts/";
  std::string m_outRoot = "timeshifts.root";
  std::string m_outPath;
  std::string m_outdata = "";

  DetectorsList m_detList;
  FilesManager files;

  Vector_MTTHist<TH1F> m_histograms;
  MTTHist<TH1F> m_histo_RF;
  MTTHist<TH1F> m_EnergyRef;
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
      if (m_timewindow_ns == 0) {std::cout << "NO TIME WINDOW !!" << std::endl; return false; }
      m_timewindow = m_timewindow_ns*1000;
      std::cout << "Extracting timeshifts with a time window of " << m_timewindow_ns << " ns" << std::endl;
    }
    else if (temp == "time_reference:")
    {
      is >> m_timeRef_name;
      if (m_timeRef_name == "") {std::cout << "NO TIME REFERENCE !!" << std::endl; return false; }
      if (isNumber(m_timeRef_name))
      {
        m_timeRef_label = std::stoi(m_timeRef_name);
        m_timeRef_name = m_detList[m_timeRef_label];
      }
      else 
      {
        m_timeRef_label = m_detList.getLabel(m_timeRef_name);
      }
      std::cout << "Reference detector set to be " << m_timeRef_name << " (n°" << m_timeRef_label << ")" << std::endl;
    }
    else if (temp == "outDir:")   {is >> m_outdir; makePath(m_outdir);}
    else if (temp == "outRoot:")  {is >> m_outRoot;}
    else if (temp == "outData:")  {is >> m_outdata;}
    else if (temp == "mult:")     {is >> m_min_mult >> m_max_mult;} //by default
    else if (temp == "verbose")   {m_verbose = true;}
    else {std::cout << std::endl << "ATTENTION, parameter " << temp << " not recognized !" << std::endl << std::endl; return false;}
  }
  if (m_outdata == "") m_outdata = removeExtension(m_outRoot)+".dT";
  return true;
}

bool Timeshifts::InitializeParam()
{
  if (m_ok) return true; // To prevent multiple initializations
  m_outPath = m_outdir+m_ts_outdir;
  create_folder_if_none(m_outPath);
  if (!folder_exists(m_outPath, true)) return (m_ok = false);
  if (extension(m_outdata) != "dT") m_outdata = removeExtension(m_outdata);
  if (m_detList.size() == 0)
  {
    print("PLEASE LOAD THE ID FILE IN THE TIMESHIFT MODULE");
    return (m_ok = false);
  }
  m_EnergyRef.reset("Energy_spectra", "Energy_spectra", 1000,0,5000);
  m_histograms.resize(m_detList.size());
  for (ushort l = 0; l<m_detList.size(); l++)
  {
    auto const & name = m_detList[l];
    auto const & type = Detectors::type(l);

    if (name == "") continue;

  #ifdef USE_RF
    if (l == 251) m_histo_RF.reset( "RF_calculated", "RF_calculated", m_timewindow/m_rebin["RF"], -m_timewindow/2, m_timewindow/2);
  #endif //USE_RF
  
    else m_histograms[l].reset(name.c_str(), name.c_str(), m_timewindow/m_rebin[type], -m_timewindow/2, m_timewindow/2);
  }
  return (m_ok = true);
}

bool Timeshifts::calculate(std::string const & folder, int const & nb_files)
{
  if (!InitializeParam()) 
  {
    print("Someting went wrong in the timeshifts initialization...");
    return false;
  }

  files.addFolder(folder, nb_files);

  if (MTObject::ON)
  {// If multithreading, treat each data file of the folder in parallel
    print("Calculating timeshifts with", MTObject::getThreadsNb(),"threads");
    // The FileManager object isn't thread safe. 
    // That is why one has to encapsulate the files list inside a MTList (Multi-Threaded List) :
    MTList<std::string> files_MT(files.getListFiles());
    MTObject::parallelise_function(treatFilesMT, files_MT, *this);
  }
  else
  {// If no multithreading, treat each data file sequentially
    print("Calculating timeshifts without multithreading");
    std::string filename;
    while (files.nextFileName(filename))
    {
      treatFile(filename);
    }
  }
  
  // Once the histograms have been filled, fit the peaks and write the values down
  analyse();

  // Write down the out files
  write();

  return true;
}

void Timeshifts::treatFilesMT(MTList<std::string> & files_MT, Timeshifts & ts)
{
  // This method is a bit odd, as it has to be static in order to be multithreaded.
  // Thus, the only way it can be "link" to the class (i.e. having access to its public members) is to pass the class itself as an argument.
  // This is done by passing "Timeshifts & ts" in arguments and fill this argument with "*this" in the piece of code that calls it
  // Finally, we can use the same function as the not multithreaded piece of code.
  std::string filename;
  while (files_MT.getNext(filename))
  {
    ts.treatFile(filename);
  }
}

void Timeshifts::treatFile(std::string const & filename)
{
  Hit hit;
  FasterReader reader(&hit, filename);
  Event event;
  CoincBuilder2 coincBuilder(&event, m_timewindow);

  RF_Manager rf;
  uint counter = 0;

  while(reader.Read())
  {
    if ( (counter%(uint)(1.E+6)) == 0 ) print(counter);
    counter++;
  #ifdef USE_RF
    if(isRF[hit.label]) {rf.last_hit = hit.time; rf.period = hit.nrj; continue;}
    else if (hit.label == m_timeRef_label)
    {
      hit.nrjcal = hit.nrj; // In order to 
       m_histo_RF.Fill(rf.pulse_ToF(hit.time));
    }
    else if (isDSSD[hit.label])
    {
      m_histograms[hit.label].Fill(rf.pulse_ToF(hit.time, USE_RF));
      continue;
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
          Long64_t deltaT = 0;
          m_EnergyRef.Fill(event.nrjs[refPos]);
          for (uchar j = 0; j<mult; j++)
          {
            if (event.labels[j] == m_timeRef_label) continue; // To reject the time spectra of the time reference
            deltaT = event.times[refPos]-event.times[j];
            m_histograms[event.labels[j]].Fill(deltaT); //stored in ps
          }
        }
      }
    }
  }
}

void Timeshifts::analyse()
{
  Float_t pospic, amppic, dump_sigma;
  Float_t cte, Mean, sigma;
  std::vector<Float_t> array_labels(m_detList.size());
  for (size_t i = 0; i<array_labels.size(); i++) array_labels[i] = i;
  std::vector<Float_t> array_resolutions(m_detList.size(),0);
  std::ofstream outDeltaTfile(m_outPath+m_outdata, std::ios::out);
  // Calculate the RF timeshift first :
#ifdef USE_RF
  m_histo_RF.Merge();
  Long64_t deltaT_RF = ( m_histo_RF->GetMaximumBin() - (m_histo_RF -> GetNbinsX()/2) ) * m_rebin["RF"];
#endif //USE_RF
  for (size_t it = 0; it<m_histograms.size(); it++)
  {
    std::unique_ptr<TF1> gaus_pol0, fittedPic;
    auto const & type = Detectors::type(it);
    m_histograms[it].Merge();
  #ifdef USE_RF
    if (type == "RF")
    {
      if (m_verbose) print("RF :",deltaT_RF,"with",m_histo_RF->GetMaximum(),"bins in peak");
      outDeltaTfile << it << "\t" << deltaT_RF << std::endl;
      continue;
    }
  #endif //USE_RF
    if (!m_histograms[it].exists()) continue; // Eliminate empty histograms
    if (m_verbose) print(m_detList[it]);
  #ifdef LICORNE
    if (type == "Eden")
    {
      outDeltaTfile << it << "\t" << 0;
      continue;
    }
  #endif //LICORNE
  #ifdef USE_DSSD
    if (type == "dssd")
    {
    #ifdef USE_RF
      amppic = m_histograms[it] -> GetMaximum();
      Float_t zero = ( m_histograms[it] -> FindFirstBinAbove(amppic/2) - (m_histograms[it] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
      // print("amppic :", amppic, "zero :", zero, "deltaT_RF", deltaT_RF, "RF-zero = ", zero - deltaT_RF);
      outDeltaTfile << it << "\t" << (Float_t)(deltaT_RF-zero) << std::endl;
      if (m_verbose) print("Edge :", zero, "with", (int) m_histograms[it] -> GetMaximum(), "counts in peak");
    #else //NO USE_RF
      amppic = m_histograms[it] -> GetMaximum();
      pospic = ( m_histograms[it] -> FindLastBinAbove(amppic*0.8) - (m_histograms[it] -> GetNbinsX()/2) ) * m_rebin["dssd"] ;
      outDeltaTfile << it << "\t" << pospic << std::endl;
      if (m_verbose) print( "mean : ", m_histograms[it] -> GetMean(), "with", (int) m_histograms[it] -> GetMaximum(), "counts in peak");
    #endif //USE_RF
    continue;
    }
  #endif
    amppic = m_histograms[it] -> GetMaximum();
    pospic = (Float_t) (m_histograms[it] -> GetMaximumBin() - (m_histograms[it] -> GetNbinsX()/2))*m_rebin[type];
    dump_sigma = (Float_t) (m_histograms[it] -> FindLastBinAbove(amppic/2) - m_histograms[it] -> FindFirstBinAbove(amppic/2))*m_rebin[type]/2;
    if (dump_sigma<2.) dump_sigma = 2.;
    if (m_verbose) print("Dump parameters \t|mean : ", std::setprecision(3), pospic/1000, " ns sigma : ", dump_sigma, " ps -- amppic : ", amppic, "coups");

    gaus_pol0.reset(new TF1("gaus+pol0","gaus(0)+pol0(3)",pospic-20*dump_sigma,pospic+20*dump_sigma));
    gaus_pol0 -> SetParameters(amppic, pospic, dump_sigma, 1);
    gaus_pol0 -> SetRange(pospic-dump_sigma*20,pospic+dump_sigma*20);
    m_histograms[it] -> Fit(gaus_pol0.get(),"R+q");

    fittedPic.reset(m_histograms[it] -> GetFunction("gaus+pol0"));
    if (!fittedPic) continue; // Eliminate non existing fits
    cte = fittedPic -> GetParameter(0);
    Mean = fittedPic -> GetParameter(1);
    sigma = fittedPic -> GetParameter(2);
    if (m_verbose) print("Fit results \t\t|mean : ", std::setprecision(3), Mean/1000, "ns sigma : ", sigma, " ps | Cte : ", cte);
    array_resolutions[it] = sigma*2.35;
    if (it == m_timeRef_label) outDeltaTfile << it << "\t" << 0 << std::endl;
    else                           outDeltaTfile << it << "\t" << (Int_t) Mean << std::endl;
  }
  outDeltaTfile.close();

  std::cout << "Timeshifts data written to " << m_outdir+m_ts_outdir+m_outdata << std::endl;
}

void Timeshifts::write()
{
  std::unique_ptr<TFile> outFile(TFile::Open((m_outPath+m_outRoot).c_str(),"RECREATE"));
  if (outFile.get() == nullptr) {print("Cannot open file ", m_outPath+m_outRoot, " !!!\nAbort !");return;}

  m_EnergyRef.Write();

#ifdef USE_RF
  m_histo_RF.Write();
#endif //USE_RF

  for (auto & histo : m_histograms) histo.Write();

  outFile -> Write();
  outFile -> Close();

  std::cout << "Timeshifts root file saved to " << m_outPath+m_outRoot << std::endl;
}

#endif //TIMESHIFTS_H
