#include "../lib/Classes/FilesManager.hpp"

// #define N_SI_120
#define N_SI_129
// #define N_SI_85

void RF_timeshifts_reader()
{
  FilesManager files;
  std::string filename;
  std::string path = "/home/corentin/faster_data/";
  // std::string path = "/srv/data/nuball2/";

  #if defined (N_SI_120)
  // files.addFolder("/home/corentin/faster_data/N-SI-120-root/Timeshifts/");
  files.addFolder(path+"N-SI-120-root/Timeshifts/");
  std::string outdir = "120/";

  std::unique_ptr<TH2F> h2_Uranium (new TH2F("June","June", 107,0,107, 15000,-15000/2,15000/2));
  std::unique_ptr<TH2F> h2_oct (new TH2F("October","October", 200,0,200, 15000,-15000/2,15000/2));

  #elif defined (N_SI_129)
  int zero = 233;
  std::string outdir = "129/";
  files.addFolder(path+"N-SI-129-root/Timeshifts/");
  std::unique_ptr<TH2F> h2_pulsed (new TH2F("pulsed","pulsed", 82,20,102, 15000,-1500/2,1500/2));
  std::unique_ptr<TH2F> h2_unpulsed (new TH2F("unpulsed","unpulsed", 57,100,157, 15000,-1500/2,1500/2));

  #endif

  while(files.nextFileName(filename))
  {
    int F = stoi(lastPart(filename,'_'));
    std::unique_ptr<TFile> readfile (TFile::Open(filename.c_str(),"read"));
    TH1F* h = (TH1F*)readfile->Get("RF_calculated");
    if (!h) continue;
    int nb = h->GetNbinsX();
    int max = h->GetMaximum();
    h->Scale(1/h->GetMaximum());
    for (int i = 0; i<nb; i++)
    {
      int pos = i;

      #if defined (N_SI_120)
      if (i > 15000/2+3000 && i<15000/2+4001) pos = i-4000;
      if (firstPart(rmPathAndExt(filename),'_') == "Uranium238") h2_Uranium -> SetBinContent(F,pos,h->GetBinContent(i));
      else if (firstPart(rmPathAndExt(filename),'_') == "run") h2_oct -> SetBinContent(F,pos,h->GetBinContent(i));

      #elif defined (N_SI_129)
      pos = pos-zero*10;
      if (pos < 15000/2-500 ) pos = pos+4000;
      else if (pos>15000/2+4000-zero*10) break;
      if (F<102) h2_pulsed   -> SetBinContent(F-19,pos,h->GetBinContent(i));
      else       h2_unpulsed -> SetBinContent(F-99,pos,h->GetBinContent(i));

      #endif
    }
    delete h;
  }
  std::string outfilename = "RF_evolution.root";
  std::unique_ptr<TFile> outfile (TFile::Open((outdir+outfilename).c_str(),"recreate"));
  outfile -> cd();

  #if defined (N_SI_120)
  h2_Uranium->Write();
  h2_oct->Write();

  #elif defined (N_SI_129)
  h2_pulsed->Write();
  h2_unpulsed->Write();

  #endif

  outfile->Write();
  outfile -> Close();
  std::cout << "written to " << outdir + outfilename << std::endl;
}
