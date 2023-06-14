#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp> // Use this to safely count what you want
#include <MTTHist.hpp>   // Use this to safely fill histograms

// When using multithreading, you have to encapsulate the code in a function that 
// will be duplicated on several files in parallel.
// This function has to have the following two arguments IN THIS ORDER :
// void your_function(Hit & hit, FasterReader & reader, arguments...)
// You can add as many arguments as you want 

void countHits(Hit & hit, FasterReader & reader, MTCounter & counterMT, MTTHist<TH1F> & histo)
{
  // See lib/Classes/Hit.h for the structure of a Hit
  int counter = 0;// It is better to deal with a local counter, and only at the end add it to the MTCounter
  while(reader.Read())
  {
    counter++;
    histo.Fill(hit.label);
  }
  print(counter);
  counterMT+=counter;
}

int main()
{
  // MANDATORY : initialize the multithreading !
  MTObject::Initialize(4);
  // In the main, first declare the reader : 
  int n = 4; // We want only the 4 first files here
  Path data ("/home/corentin/faster_data/N-SI-136/run_10.fast"); // A simple wrapper around std::string
  // Path data ("/path/to/data/folder/"); // Path is a simple wrapper around std::string to handle path 
  MTFasterReader readerMT(data, n);
  // You will now read the n first files of the data folder
  // Declare all the arguments needed for your function :
  MTCounter counterMT;
  MTTHist<TH1F> scaler;
  scaler.reset("Scaler", "Scaler", 1000,0,1000); // In this example, we are producing an image of the total count for each detector
  // Now run the reader in multithread mode : 
  readerMT.execute(countHits, counterMT, scaler);
  // You first write down the name of the function you wanna use, then use commas to separate all the arguments
  // If many arguments are needed, the best way is to create a struct and pass it as an argument
  // Or else
  // In order to access the arguments directly, you have to use the address of the variable using & 
  print(counterMT);
  // Write down the histogram :
  std::unique_ptr<TFile> file;
  file.reset(TFile::Open("scaler.root","recreate"));
  file -> cd();
  scaler.Write();
  file->Write();
  file->Close();
  return 1;
}