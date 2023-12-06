#include "../NearLine-lib/NearLine2.h"

struct Param
{
  std::string sharedFolder;
  std::string dataFolder;
  std::string configFile;
  int time_wait = 1000;//ms
  bool m_hr = false;
  bool m_hc = false;
  bool m_a  = false;

  bool finito = false;
};

void checkInput(std::mutex & mutex, Param & parameters)
{
  std::string input;
  // char input;
  while(!parameters.finito)
  {
    std::cin>>input;
    print(" >> ", input);
    if(input == "quit")
    {
      parameters.finito = true;
    }
    else if(input == "stop")
    {
      parameters.finito = true;
    }
  }
}

void fillHisto(std::mutex & mutex, Param & parameters)
{
  NearLine app;
  parameters.finito = !app.loadConfig(parameters.configFile);
  if(!app.configOk()) parameters.finito = true;
  std::string folder = parameters.dataFolder;
  print(parameters.finito);
  mutex.lock();
  parameters.m_hr = true;
  parameters.m_hc = false;
  parameters.m_a  = false;
  mutex.unlock();
  if (folder.back() != '/') folder.push_back('/');
  int nb_files = 0, new_nb_files = 0;
  nb_files = nb_files_in_folder(folder);
  std::string filename;
  new_nb_files = check_new_file(folder, filename);
  while (!parameters.finito)
  {
    if (nb_files == new_nb_files)
    {
      mutex.lock();
      print("Waiting for new file");
      mutex.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(parameters.time_wait));
    }
    else if (nb_files>new_nb_files) return;
    else
    {
      mutex.lock();
      mutex.unlock();
      nb_files++;
      app.processFile(folder+filename);
      app.WriteData();
      gSystem -> Exec(("touch "+parameters.sharedFolder+std::to_string(nb_files)).c_str());
    }
    mutex.lock();
    new_nb_files = check_new_file(folder, filename);
    mutex.unlock();
  }
}

void showHisto(std::mutex & mutex, Param & parameters)
{
  std::string folder = parameters.sharedFolder;
  std::string filename;
  int nb_files = 0, new_nb_files = 0;
  nb_files = nb_files_in_folder(folder);
  new_nb_files = check_new_file(folder, filename);
  while (!parameters.finito)
  {
    print(nb_files, " ", new_nb_files);
    if (nb_files == new_nb_files)
    {
      print("Shower waiting");
      std::this_thread::sleep_for(std::chrono::milliseconds(parameters.time_wait));
    }
    else if (nb_files>new_nb_files) return;
    else
    {
      nb_files++;
      if (parameters.m_a)  gSystem -> Exec("root analyse.root");
      if (parameters.m_hr) gSystem -> Exec("root raw_histo.root");
      if (parameters.m_hc) gSystem -> Exec("root calib_histo.root");
    }
    mutex.lock();
    new_nb_files = check_new_file(folder, filename);
    mutex.unlock();
  }
}

int main(int argc, char **argv)
{
  Param parameters;
  if (argc == 2)
  {
    parameters.dataFolder = argv[1];
    parameters.configFile = "parameters.dat";
  }
  else if (argc == 3)
  {
    parameters.dataFolder = argv[1];
    parameters.configFile = argv[2];
  }
  else
  {
    print("Which folder do you want to monitor?");
    return -1;
  }
  parameters.sharedFolder = ("internFiles/popupFiles/"+std::to_string(time(0))+"/");
  gSystem -> Exec(("mkdir "+parameters.sharedFolder).c_str());
  std::mutex  mutex;
  mutex.lock();
  std::thread t1([&mutex, &parameters](){fillHisto (mutex, parameters);});
  mutex.unlock();
  mutex.lock();
  std::thread t3([&mutex, &parameters](){checkInput(mutex, parameters);});
  mutex.unlock();
  mutex.lock();
  std::thread t2([&mutex, &parameters](){showHisto (mutex, parameters);});
  mutex.unlock();
  t1.join();
  t2.join();
  t3.join();
  return 1;
}
