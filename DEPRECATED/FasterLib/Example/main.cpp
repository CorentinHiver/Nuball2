#include <libCo.hpp>
#include <FasterReader.hpp>   // This class is the base for mono  threaded code
#include <MTFasterReader.hpp> // This class is the base for multi threaded code
#include <MTCounter.hpp> // Use this to safely count what you want
#include <MTTHist.hpp>   // Use this to safely fill histograms

// When using multithreading, you have to encapsulate the code in a function that
// will be duplicated on several files in parallel.
// This function has to have the following two arguments IN THIS ORDER :
// void your_function(Hit & hit, FasterReader & reader, arguments...)
// You can add as many arguments as you want (variadic template arguments)

void countHits(Hit & hit, FasterReader & reader, MTCounter & counterMT, MTTHist<TH1F> & histo, MTTHist<TH1F> & spectra)
{
  // See lib/Classes/Hit.hpp for the structure of a Hit
  int counter = 0;// It is better to deal with a local counter, and only at the end add it to the MTCounter
  while(reader.Read())
  {
    counter++;
    histo.Fill(hit.label);
    spectra.Fill(hit.nrj);
  }
  print(counter);
  counterMT+=counter;
}

void convert_simple(Hit & hit, FasterReader & reader)
{
  // See lib/Classes/Hit.hpp for the structure of a Hit
  std::unique_ptr<TTree> tree (new TTree("Nuball2","Nuball2"));
  tree -> Branch("label",&hit.label);
  tree -> Branch("nrj",  &hit.nrj  );
  tree -> Branch("time", &hit.time );

  int counter = 0;
  
  while(reader.Read())
  {
    counter++;
    if (counter%(int)1.E+7 == 0) print(counter);
    tree -> Fill();
  }

  std::string outName = "./"+rmPathAndExt(reader.filename())+".root";
  std::unique_ptr<TFile> file = TFile::Open(outName.c_str(), "RECREATE");
  file -> cd();
  tree -> Write();
  file -> Write();
  file -> Close();
}

int main()
{
  // MANDATORY : Initialise the multithreading !
  MTObject::Initialise(4);

  // Instantiate the reader
  MTFasterReader readerMT(/path/to/data/folder, nb_files); // leave nb_files empty or equals to -1 if reading all the files


  // Example n°1 : simple convertion : 
  readerMT.execute(convert);


  // Example n°2 : simple counter and histograms filling

  // Declare all the arguments :
  MTCounter counterMT;
  MTTHist<TH1F> scaler("Scaler", "Scaler", 1000,0,1000);
  MTTHist<TH1F> spectra("Spectra","Spectra", 1000,0,5000);

  // Then execute the function :
  readerMT.execute(countHits, counterMT, scaler, spectra);

  // Finally, write down the histograms and print the counts :
  std::unique_ptr<TFile> file;
  file.reset(TFile::Open("scaler.root","recreate"));
  file -> cd();
  scaler.Write();
  file->Write();
  file->Close();

  return 1;
}
