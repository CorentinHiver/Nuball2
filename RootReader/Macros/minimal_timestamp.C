#define N_SI_120
#include "/media/sf_OneDrive/CNRS/FASTER/NearLine3/NearLine-lib/utils.hpp"
void minimal_timestamp()
{
  int nb_labels = 220;
  Long64_t rf_shift_ns = 50;
  std::string outRoot = "timestamps_test.root";


  std::string rootfile = "/media/sf_OneDrive/CNRS/FASTER/NearLine3/tests_faster2root/run_oct2022_20_0001.root";
  // std::string rootfile = "/media/sf_OneDrive/CNRS/FASTER/NearLine3/faster2root/N-SI-120/run_oct2022_20/run_oct2022_20_0001.root";
  std::unique_ptr<TFile> file (TFile::Open(rootfile.c_str(), "READ"));
  std::unique_ptr<TTree> tree (file->Get<TTree>("Nuball"));

  std::vector<unique_ptr<TH1F>> timestamps;
  timestamps.resize(nb_labels);
  std::string name;
  for (size_t label = 0; label<timestamps.size(); label++)
  {
    name = "relative timestamp for "+std::to_string(label);
    if (label>199)
    timestamps[label].reset(new TH1F(name.c_str(),name.c_str(), 2500, -100, 400));
    else
    timestamps[label].reset(new TH1F(name.c_str(),name.c_str(), 500, -100, 400));
  }

  // UChar_t mult = 0;
  int     mult = 0;
  Label   labels [255];
  Time    times  [255];
  Float_t nrjs   [255];


  tree -> SetBranchAddress("mult" , &mult);
  tree -> SetBranchAddress("label", &labels);
  tree -> SetBranchAddress("time" , &times);
  tree -> SetBranchAddress("nrj"  , &nrjs);

  ULong64_t prev_timestamp = 0;
  auto size = tree->GetEntries();

  // RF:
  ULong64_t rf_time = 0;
  Long64_t  rf_period = 399998;
  Long64_t  rf_shift = rf_shift_ns*1000;
  Long64_t  relative_time = 0.;
  Long64_t  relative_shifted = 0.;

  for (auto i = 0; i<size; i++)
  {
    tree->GetEntry(i);
    for (auto hit = 0; hit<mult; hit++)
    {
      if(prev_timestamp>times[hit]) print("Oups");
      if (labels[hit] == 251)
      {
        rf_time = times[hit];
        rf_period = nrjs[hit];
        continue;
      }

      else if (labels[hit]>220) continue;

      relative_shifted = (times[hit]+rf_shift-rf_time)%rf_period;
      relative_time = relative_shifted-rf_shift;
      if (relative_shifted<0)print("Oups");
      timestamps[labels[hit]] -> Fill(relative_time/_ns);
      prev_timestamp = times[hit];
    }
  }
  std::unique_ptr<TFile> outfile(TFile::Open(outRoot.c_str(),"RECREATE"));
  outfile->cd();
  for (auto const & hist : timestamps) if(hist->Integral()>1) hist->Write();
  outfile->Write();
  outfile->Close();
  print("analyse file writtent to", outRoot);
}
