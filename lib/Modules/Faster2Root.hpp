#ifndef FASTER2ROOT_HPP
#define FASTER2ROOT_HPP

#include "../libCo.hpp"

#include "../Classes/Detectors.hpp"
#include "../Classes/CoincBuilder.hpp"
#include "../Classes/Nuball2Tree.hpp"
#include "../Classes/Timer.hpp"
#include "../Classes/Alignator.hpp"

#include "../MTObjects/MTFasterReader.hpp"

#include "Timeshifts.hpp"
#include "Calibration.hpp"

using Trigger = std::function<bool(const Event&)>;

/**
 * @brief Basic class to perform faster to root tree data conversion
 * 
 * @details 
 * 
 * Section 1 : Options 
 * 
 * 
 * Use this class to perform raw conversion. Without any option, no calibration, no time alignement and no event building are performed.
 * If you want to do more, you have to load the ID file with the -l option. This file has the following format : 
 * 
 *    label_nb name
 *    label_nb name
 *    ...
 *    label_nb name
 * 
 * You can build it from the index.pid file
 * 
 * You can import a calibration file using -c option. Considering the equation E_cal = a + b*ADC + c*ADC² ..., the file has the following structure : 
 * 
 *    label_nb a b (c) (d) 
 * 
 * If a detector has no calibration data, then leaving its line empty will automatically fills a = 0 and b = 1. 
 * c and d are optional, they will be filled with 0. You can choose the number of parameter line by line, 
 * because each detector needs a different correction order !
 * 
 * Now, let's attack the most difficult part : timing and event building. 
 * So far, the data are still shuffled because of time shifts induced by faster hardware.
 * So there are different options :
 * 
 *  -e [time_window_ns] : Simple event building. If no time realignment, the time shift can be big, it is recommended to use at least 1500 ns
 *  This option will always result in more time and memory consumption because the conversion is done in 3 step : 
 *  First the hits are time shifted (and put in a memory resident root tree). But this will result in a shuffle,
 *  because some hits can be shifted before the previous hit. Therefore, the data have to be realigned.
 *  And finally the conversion will take place with truly time aligned data.
 * 
 *  -m [threads_number] : Multithreading. use this option to choose the number of files to treat in parallel.
 * 
 *  -t [timeshift_file] : Load the timeshifts file.
 * 
 * Recap & ancillary parameters : 
 * 
 *  -c [calibration_file] : Load the calibration
 *  -e [time_window_ns]   : Perform event building with time_window_ns = 1500 ns by default
 *  -i [ID_file]          : Load ID file. For Nuball2 data, allows for nice lookup tables to know the type of detector like isDSSD[label] or isGe[label]
 *  -m [threads_number]   : Choose the number of files to treat in parallel
 *  -n [files_number]     : Choose the total number of files to treat inside a data folder
 *  -t [time_window_ns]   : Loads timeshift data
 *  --throw_single         : If you are not interested in single hits (hits not alone in their time range)
 * 
 * @attention The two first parameters must be /path/to/data/ and /path/to/output/
 * Example 1 : 
 * 
 *      Faster2Root("/path/to/data /path/to/output -i index_129.id -c calib.calib -e 200 -m 4 -t timeshifts.dT ")
 * 
 * Example 2 :
 * 
 *  We want to create a DSSD trigger :
 * 
 *      int main(int argc, argv** argv)
 *      {
 *        Faster2Root convertor(argc, argv, [](const Event& event)
 *        {
 *          for (int hit = 0; hit<event.mult; hit++)
 *          {
 *            if (isDSSD[event.labels[hit]]) return true;
 *          }
 *          return false;
 *        });
 *      }
 * 
 * Then compile the code and call : ./executable /path/to/data /path/to/output -i index_129.id -c calib.calib -e 200 -m 4 -t timeshifts.dT
 * 
 * 
 *  
 *  Section 2 : Format 
 * 
 * The root tree is made of 6 leaves : 
 *  
 *  type              name          Description
 *  int               mult          Multiplicity : number of hits in the event
 *  unsigned short [] label         Labels
 *  int/float      [] ADC/nrj       (ADC/QDC1) / Calibrated energy
 *  int/float      [] QDC2/nrj2     QDC2       / Calibrated energy in QDC2
 *  bool           [] pileup        Pileup/Saturation bit
 * 
 * The output will therefore depend on wether you calibrated the data or not
 * 
 * In the code, the main object handling the event data is Event. You have to have a look at its complete description
 * if you are to read the data
 *  
 * 
 */
class Faster2Root
{
public:

  Faster2Root() {}

  Faster2Root(int argc, char** argv) {load(argc, argv);}
  Faster2Root(int argc, char** argv, Trigger trigger )
  {
    setTrigger(trigger);
    load(argc, argv);
  }

  /// @brief Raw conversion :
  /// @deprecated
  Faster2Root(Path const & inputFolder, Path const & outputFolder, int const & nb_files = -1, bool const & buildEvents = false) 
  {// Raw convertor : no time shift nor energy calibration
    m_ok = true;
    Timer timer;
    this -> buildEvents(buildEvents);
    this -> setNbFiles(nb_files);
    this -> convert(inputFolder, outputFolder);

    print("Data written to", outputFolder, "in", timer(), timer.unit());
  }

  void setTimeshifts(Timeshifts const & timeshifts) {m_timeshifts = timeshifts;}
  void loadTimeshifts(std::string const & dTfile)   {m_timeshifts.load(dTfile);}
  void setCalibration(Calibration const & calibration) {m_calibration = calibration;}
  void loadCalibration(std::string const & calibFile)  {m_calibration.load(calibFile);}
  void setNbFiles(int const & nb_files = -1) {m_nb_files = nb_files;}
  void setNbThreads(int const & nb_threads = -1) {m_nb_threads = nb_threads;}
  void buildEvents(int time_window_ns) 
  {
    print("Performing event buiding with", time_window_ns, "ns time window");
    m_eventbuilding = true;
    m_time_window = time_window_ns*1000;
  }
  void overwrite( bool const & _overwrite) {m_overwrite = _overwrite;}

  void convert(std::string const & dataFolder, std::string const & outputFolder, int const & nb_files = -1);

  void setTrigger(std::function<bool(const Event&)> other) {m_trigger = other; m_use_trigger = true;}
  void loadTriggerFile(std::string const & file);
  void throwSingles(bool const & _throw_single = true) {m_throw_single = _throw_single;}

protected:
  void load(int argc, char** argv);
  static void s_convertFile(Hit & hit, FasterReader & reader, 
                            Faster2Root & convertor, Path const & outPath) 
  {convertor.convertFile(hit, reader, outPath);}
  virtual void printParameters() const;

  virtual void convertFile(Hit & hit, FasterReader & reader, Path const & outPath);

  Timeshifts m_timeshifts;
  Calibration m_calibration;

  int m_nb_threads = 1;
  int m_nb_files = -1;
  Long64_t m_time_window = 1500000; // By default, a 1.5 us time gate is used
  bool m_eventbuilding = false;
  bool m_calibrate = false;
  bool m_throw_single = false;
  bool m_overwrite = false;
  bool m_ok = false;
  bool m_use_RF = false;
  bool m_use_trigger = false;
  std::string m_trigger_file;
  bool m_loaded_trigger = false;

  MTCounter m_total_hits;
  MTCounter m_total_events;
  MTCounter m_trigg_events;
  Timer m_total_timer;
  std::function<bool(const Event&)> m_trigger = [](const Event&) { return true; };
  std::vector<Label> m_triggering_labels;
};

void Faster2Root::printParameters() const
{
  print("");
  print("Usage of Faster2Root : /path/to/data /path/to/output [[parameters]]");
  print("");
  print("parameters :");
  print("");
  print("-c [calibration_file]  : Loads the calibration file");
  print("-e [time_window_ns]    : Perform event building with time_window_ns = 1500 ns by default");
  print("-f [files_number]      : Choose the total number of files to treat inside a data folder");
  print("-i [ID_file]           : Load ID file (necessary if trigger is used)");
  print("-n [hits_number]       : Choose the number of hits to read inside a file");
  print("-m [threads_number]    : Choose the number of files to treat in parallel");
  print("-t [time_window_ns]    : Loads timeshift data");
  print("--throw-singles        : If you are not interested in single hits");
  print("--trigger [filename]   : Load a trigger file (look at documentation)");
  exit(1);
}

void Faster2Root::load(int argc, char** argv)
{
  // Handle the case of not enough parameters (0 or 1) and print the parameters :
  if (argc<3) 
  {
    print("Not enough parameters !!!");
    printParameters();
  }

  // Path of the input data :
  Path dataPath = argv [1];

  // Path of the output data :
  Path outPath = argv [2];

  // Other parameters :
  for (int i = 3; i<argc; i++)
  {
    std::string command = argv[i];
         if (command == "-c") loadCalibration(argv[++i]);
    else if (command == "-e") buildEvents(std::stoi(argv[++i]));
    else if (command == "-f") setNbFiles(std::stoi(argv[++i]));
    else if (command == "-i") detectors.load(argv[++i]);
    else if (command == "-m") setNbThreads(std::stoi(argv[++i]));
    else if (command == "-n") FasterReader::setMaxHits(std::stoi(argv[++i]));
    else if (command == "-o") overwrite(true);
    else if (command == "-rf") m_use_RF = true; // TBD
    else if (command == "-t") loadTimeshifts(argv[++i]);
    else if (command == "--throw-singles") m_throw_single = true;
    else if (command == "--trigger") loadTriggerFile(argv[++i]);
    else {throw_error("Unkown command" + command);}
  }

  // Checking consistency of parameters :
  if (m_timeshifts && !m_eventbuilding) 
  {
    print("This combination of time alignement without event building is not handled");
    throw_error(error_message["DEV"]);
  }
  if (m_use_trigger && !m_eventbuilding)
  {
    throw_error("Can't use a trigger without event building ! Use parameter -e [time_window_ns] or method Faster2Root::buildEvents(time_window_ns).");
  }

  // Perform initialisations :
  if (m_nb_threads > 1) MTObject::Initialize(m_nb_threads);
  std::cout << std::setprecision(4);

  // Run the conversion :
  this -> convert(dataPath, outPath);

  // Print some additionnal informations at the end of the conversion :
  print(m_total_hits/m_total_timer.TimeSec()*1.E-6, "Mhits/s");
  if (m_eventbuilding) print(m_total_events/m_total_timer.TimeSec()*1.E-6, "Mevts/s");
}

void Faster2Root::loadTriggerFile(std::string const & filename)
{
  std::ifstream file((m_trigger_file = filename), std::ios::in);
  std::string line;
  while(getline(file, line)) m_triggering_labels.push_back(std::stoi(line));
  setTrigger([this](Event const & event)
  {
    // print("coucou", m_triggering_labels);
    for (int hit_i = 0; hit_i<event.mult; hit_i++)
    {
      if (found(m_triggering_labels, event.labels[hit_i])) return true;
    }
    return false;
  });
  m_loaded_trigger = true;
}

void Faster2Root::convert(std::string const & dataFolder, std::string const & outputFolder, int const & nb_files)
{
  Path dataPath(dataFolder);
  if (!dataPath) {print(dataFolder, "NOT FOUND"); throw std::runtime_error("DATA");}
  Path outPath(outputFolder, 1);
  MTFasterReader readerMT(dataPath, (nb_files>0) ? nb_files : m_nb_files);
  readerMT.readRaw(s_convertFile, *this, outputFolder);
}

/**
 * @brief 
 * 
 * @param hit 
 * @param reader 
 * @param outPath 
 */
void Faster2Root::convertFile(Hit & hit, FasterReader & reader, Path const & outPath)
{
  Timer timer;

  //Filename manipulations :
  File inputFile = reader.filename();
  File outputFile = outPath+inputFile.shortName()+".root";

  if (!m_overwrite && outputFile.exists()) {print(outputFile, "already exists ! Use option -o or method Faster2Root::overwrite(true)"); return;}
  
  unique_tree tree(new TTree("Nuball2","Nuball2"));
  tree -> SetDirectory(nullptr); // Set the tree RAM resident : increases memory usage but speeds the process
  Event event;
  unique_tree tempTree;
  if (m_eventbuilding)
  {// Event building is done in two times : first filling a temporary tree, then reorder it after timeshift and finally create events
    tempTree.reset(new TTree("temp","temp"));
    tempTree -> SetDirectory(nullptr);
    hit.writting(tempTree.get(), (m_calibration) ? "lsEQp" : "lseqp");
    event.writting(tree.get(), (m_calibration) ? "lstEQp" : "lsteqp");
  }
  else
  {
    tree->SetTitle("Nuball2 without event building");
    event.writting(tree.get(), (m_calibration) ? "lsEQp" : "lseqp");
  }

  if (m_calibration) tree -> SetTitle((tree->GetTitle()+std::string(" with calibration")).c_str());
  if (m_timeshifts)  tree -> SetTitle((tree->GetTitle()+std::string(" with timeshifts" )).c_str());
  if (m_loaded_trigger)  tree -> SetTitle((tree->GetTitle()+std::string(" with trigger loaded from ") + m_trigger_file).c_str());
  else if (m_use_trigger) tree -> SetTitle((tree->GetTitle()+std::string(" with user defined trigger")).c_str());

// Read faster data to fill the memory resident tree :
  while(reader.Read())
  {
    if (m_timeshifts) hit.stamp += m_timeshifts[hit.label];
    if (m_calibration)
    {
      hit.nrj  = m_calibration(hit.adc, hit.label);
      hit.nrj2 = (hit.qdc2==0) ? m_calibration(hit.qdc2, hit.label) : 0.0;
    }
    if (m_eventbuilding) tempTree -> Fill();
    else
    {// If no event building, directly writes the hits in the output tree
      event = hit;
      tree -> Fill();
    }
  }

  if (reader.getCounter()<1) {print("NO DATA IN FILE", reader.getFilename()); throw_error("DATA");}
  m_total_hits+=reader.getCounter();
  longlong evts = 0;
  longlong evts_trigg = 0;
  print();
  if (m_eventbuilding)
  {// Read the temporary tree and perform event building :
    auto const & nb_data = tempTree->GetEntries();
    Alignator alignator(tempTree.get());
    hit.reading(tempTree.get(), (m_calibration) ? "lsEQp" : "lseqp");
    CoincBuilder builder(&event, m_time_window);
    builder.keepSingles(!m_throw_single);

    longlong loop = 0;
    while (loop<nb_data)
    {
      tempTree -> GetEntry(alignator[loop++]);
      if (builder.build(hit)) 
      {
        ++evts;
        print(event);
        pauseCo();
        if(m_trigger(event)) 
        {
          ++evts_trigg;
          tree->Fill();
        }
      }
    }
  }

  m_total_events+=evts;
  m_trigg_events+=evts_trigg;
  float trigger_rate = 100.*evts_trigg/evts;

  // Write down the tree : 
  unique_TFile file(new TFile(outputFile.c_str(), "RECREATE"));
  file -> cd();
  tree -> Write();
  file -> Write();
  file -> Close();

  std::string m_eventbuilding_output = ")";
  if (m_eventbuilding) m_eventbuilding_output = "( "+std::to_string(evts*1.E-6)+" Mevts ( trigger in "+std::to_string(trigger_rate)+" % ))";

  print(outputFile, "written (", reader.getCounter()*1.E-6, "Mhits in", timer(), timer.unit(), m_eventbuilding_output);
}


////////////////////////////
// USER DEFINED CONVERTOR //
////////////////////////////

// Example of user-defined convertor :
class MySimpleConvertor : public Faster2Root
{
public:
  // With this constructor, myConvertor becomes a simple wrapper around Faster2Root class
  template <class... ARGS>
  MySimpleConvertor(ARGS &&... args) : Faster2Root(std::forward<ARGS>(args)...) {}

  // You can write down your own convertor :
  void convertFile(Hit & hit, FasterReader & reader, Path const & outPath) override;

  // Define here the classes and parameters you would like to share bewteen threads.
  // You already share all the protected members of Faster2Root
  // Be careful not to write in them !!
};

void MySimpleConvertor::convertFile(Hit & hit, FasterReader & reader, Path const & outPath)
{
  // First, declare a tree :
  unique_tree tree(new TTree("simple","simple"));
  tree -> SetDirectory(nullptr); // To make it memory resident
  // Next, connect the hit to the tree (Hit::writting() is a shortcut for the many TTree::Branch method to be called, 
  // see Hit class for more information). You can do it manually of course !
  hit.writting(tree.get(), "lseqp");

  // Now, we can read the faster file :
  while(reader.Read())
  {
    // The hit is loaded with FasterReader::Read(), so all we have to do is fill the tree :
    tree -> Fill(); 
  }

  // Write down this tree : 
  unique_TFile file(new TFile(outPath+"simple.root"));
  file -> cd();
  tree -> Write();
  file -> Write();
  file -> Close();

  // Notes : In case the multi-threading is activated for the convertor, this function is split between each thread
  // If you want instead to treat many folders in parallel (instead of many files from the same folder), simply
  // set 
}

#endif //FASTER2ROOT_HPP