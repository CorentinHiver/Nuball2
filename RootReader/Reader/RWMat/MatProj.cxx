#include <sstream>
#include "TImage.h"
#include "TCanvas.h"
#include "TArrayD.h"
#include "TROOT.h"
#include "TColor.h"
#include "TAttImage.h"
#include "TEnv.h"
#include "TH2D.h"
#include "TF2.h"
#include "TH3I.h"
#include "TColor.h"
#include "TLine.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TApplication.h"
#include "TFile.h"
#include <fstream>
#include "TH1.h"
#include "TMath.h"
#include "TSpectrum2.h"
#include "TSpectrum2Painter.h"
#include "TSpectrum2Transform.h"
#include "TSpectrum3.h"
#include "TSpectrumFit.h"
#include "TSpectrum.h"
#include "TSpectrumTransform.h"
#include "TGraphErrors.h"
#include "TText.h"
#include "Spec.hxx"
#include "Spec.cxx"
#include "RWMat.hxx"
#include "RWMat.cxx"
#include "WriteRWSpec.cxx"


using namespace std;

//Returns the background under the spectrum transmitted.
void backsub(TH1D* TheSpectrum, int nsmooth)
{
  int j = 0;
  int nbins=TheSpectrum->GetNbinsX();
  Double_t *source=new Double_t[nbins];
  TSpectrum *s = new TSpectrum();
  for (j=0;j<nbins;j++) source[j]=TheSpectrum->GetBinContent(j+1);
  s->Background(source,nbins,nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
  for (j=0;j<nbins;j++) {TheSpectrum->SetBinContent(j+1, source[j]);}
  s->Delete();
  delete [] source;
}

//Returns the background subtracted spectrum
void backsubS(Spec* TheSpec, int nsmooth)
{
int j = 0;
int nbins=TheSpectrum->GetNbinsX();
Double_t *source=new Double_t[nbins];
TSpectrum *s = new TSpectrum();
for (j=0;j<nbins;j++) source[j]=TheSpec->GetChan(j);
s->Background(source,nbins,nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
for (j=0;j<nbins;j++) {TheSpec->SetChan(j, TheSpec->GetChan(j)-source[j]);}
s->Delete();
delete [] source;
}

int main(int argc, char **argv)
{
  if (argc <= 1) {cout << "type fname " << endl; exit(1);}

  string fname1=argv[1];

  string froot=fname1.substr(0,fname1.length()-4);
  //int thresh=70;
  int thresh=5;

  RWMat *mat1=new RWMat; //Raw matrix

  TH1D *px = new TH1D("px","px",4096,0,4096);  
  TH1D *py = new TH1D("py","py",4096,0,4096);  
  TH1D *bx = new TH1D("bx","bx",4096,0,4096);  
  TH1D *by = new TH1D("by","by",4096,0,4096); 
  mat1->Read(fname1);
  double t1=mat1->Integral();
  cout << "mat1 has " << t1 << " counts " << endl;

  for (int i=0; i < 4096; i++) //over x
  {
    double pxt=0, pyt=0;
    for (int j=0; j < 4096; j++) //over y
    {
      pxt+=mat1->Get(j,i);
      pyt+=mat1->Get(i,j);
    }
    px->SetBinContent(i,pxt);
    py->SetBinContent(i,pyt);
    bx->SetBinContent(i,pxt);
    by->SetBinContent(i,pyt);
  }
  backsub(bx,10);
  backsub(by,10);
  cout << px->Integral() << " " << bx->Integral() << endl;
  // Classic Radware subtraction
  Spec *projx=new Spec(4096);
  Spec *projy=new Spec(4096);
  Spec *bkx=new Spec(4096);
  Spec *bky=new Spec(4096);

  for (int i=0; i < 4096; i++) 
  {
    projx->SetChan(i,px->GetBinContent(i));
    projy->SetChan(i,py->GetBinContent(i));
    bkx->SetChan(i,bx->GetBinContent(i));
    bky->SetChan(i,by->GetBinContent(i));
  }
  string fnamepx=froot+"_px.spe";
  string fnamepy=froot+"_py.spe";
  string fnamebx=froot+"_bx.spe";
  string fnameby=froot+"_by.spe";

  projx->Write(fnamepx);
  projy->Write(fnamepy);
  bkx->Write(fnamebx);
  bky->Write(fnameby);

}

/*


g++ -g -o MatProj MatProj.cxx  ` root-config --cflags` `root-config --glibs` -lSpectrum


*/


