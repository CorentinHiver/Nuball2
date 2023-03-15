//g++ test.cpp $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o test -g -std=c++17
#include "NearLine-lib/utils.hpp"
#include "NearLine-lib/Classes/MTTHxx.hpp"
int main()
{
  TFile* f = TFile::Open("test.root","recreate");
  f -> cd();
  MTTHxx<TH2F> hist;
  MTObject::nb_threads = 4;
  hist.reset("test","test",1000,0,1, 1000,0,1000);
  int thread_number = 0;
  std::vector<std::thread> threads;
  for (size_t i = 0; i<MTObject::nb_threads; i++)
  {
    threads.emplace_back([&hist, &thread_number]()
    {
      int nb_thread = thread_number;
      thread_number++;
      print(nb_thread);
      for (int i = 0; i<10; i++) hist[nb_thread] -> Fill(i%2,i);
    });
  }
  for(unsigned int i = 0; i < threads.size(); i++) threads.at(i).join(); //Closes threads
  hist.Write();
  f -> Write();
  f -> Close();
  delete f;
  return 1;
}
