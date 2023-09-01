#include <sstream>
#include "TImage.h"
#include "TText.h"
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
#include "bigpeaks.hxx"
#include "Spec.hxx"
#include "Spec.cxx"
#include "RWMat.hxx"
#include "RWMat.cxx"
#include "WriteRWSpec.cxx"

bool GateOnDelayed=true;
using namespace std;

//Returns the background under the spectrum transmitted.
void backsub(TH1D* TheSpectrum, int nsmooth)
{
int j;
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
int j;
int nbins=4096;
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
string fname2=fname1.substr(0,fname1.length()-4);
fname2=fname2+"d.m4b";

int lookup[8192];
for (int i=0; i < 18; i++) {for (int j=bigpeaks[i][0]; j <= bigpeaks[i][1]; j++) {lookup[j]=i+1;}}


//int thresh=70;
int thresh=5;

RWMat *mat1=new RWMat; //Raw matrix
RWMat *bksub1=new RWMat; //The final matrix (rw subtraction only)
RWMat *bksub2=new RWMat; //The final matrix (rw plus diagonal stripes removed)
RWMat *bksub3=new RWMat; //The final matrix (rw, diagonal stripes and horizontal stripes)
RWMat *bksub4=new RWMat; //The final matrix (rw, diagonal stripes and horizontal stripes)
RWMat *diabk1=new RWMat; //diagonals bkg
RWMat *diabk2=new RWMat; //result stripes bkg
RWMat *diabk3=new RWMat; //result stripes bkg sym. Symmetrized, since x and y stripes are different
RWMat *result=new RWMat; //sum of all bk that is not radware. Must subtract this off with MatAdd and then use xmesc

// NOTE result stripes horizontal and vertical add extra noise to the matrix, which is worse for poor statistics

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
		//if (lookup[i] || lookup[j]) {mat1->Set(i,j,0); mat1->Set(j,i,0);}
		//if ((i+j) < 4096)
			//{
			//if (lookup[(i+j)]) {mat1->Set(i,j,0); mat1->Set(j,i,0);}
			//}
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


// STEP 1: Classic Radware subtraction
double T=mat1->Integral();
for (int i=0; i < 4096; i++) //over x
	{
	for (int j=0; j < 4096; j++) //over y
		{
		
		double bkval=px->GetBinContent(i)*by->GetBinContent(j)+py->GetBinContent(j)*bx->GetBinContent(i);
		bkval-=(bx->GetBinContent(j)*by->GetBinContent(i));
		double counts=mat1->Get(i,j);
		bkval/=T;
		double newcounts=counts-bkval;		
		//if (newcounts < -100) {bksub1->Set(i,j,newcounts);}
		//else {bksub1->Set(i,j,1);}
		bksub1->Set(i,j,newcounts);
		}
	}


Spec *diagonal=new Spec(4096);
Spec *diagonalb=new Spec(4096);

// STEP 2: CORRECT THE OVER AND UNDERSUBTRACTED GATES

for (int i=0; i < 4096; i++)
{
bool ingate=false;
double oldval=0;
	for (int j=0; j < 4096; j++)
	{
	double val=bksub1->Get(i,j);
	if ((lookup[(i+j)]) || lookup[j]) {ingate=true; val=oldval;}
	else {ingate=false; oldval=val;}
	diagonalb->Inc(j,val);
	diagonal->Inc(j,val);
	}
backsubS(diagonal,16);
	for (int j=0; j < 4096; j++)
	{
	double val=diagonal->GetChan(j); //Regular matrix SLICE
	double val2=diagonalb->GetChan(j); //Background subtracted SLICE
	double bkg=(val2-val);
	diabk1->Set(i,j,bkg);
	double val3=bksub1->Get(i,j);
	bksub2->Set(i,j,val3-bkg);
	//bksub2->Set(j,i,val3-bkg);
	//if (lookup[i]) {bksub2->Set(j,i,val3-bkg);}
	}
if (i == 351) {diagonal->Write("dia1.spe"); diagonalb->Write("dia1b.spe");}
diagonal->Clear();
diagonalb->Clear();
}
//Resymmetrize, privelaging the x-axis which has the right set of y-spectra
for (int i=0; i < 4096; i++)
{
	for (int j=0; j < 4096; j++)
	{
	double val=bksub2->Get(i,j);
	if (lookup[i]) {bksub2->Set(j,i,val);}
	}
}
bksub2->ReSymmetrise();

// STEP 3: SUBTRACT OFF THE DIAGONALS
for (int i=1; i < 4096; i++)
{
	for (int j=0; j <=i; j++)
	{
	int x=j;
	int y=i-j;
	if ((x < 4096) && (y < 4096) && (y >= 0) && (x >= 0))
		{
		diagonalb->Inc(x,bksub2->Get(x,y));
		diagonal->Inc(x,bksub2->Get(x,y));
		}
	}
backsubS(diagonal,4);
	for (int j=0; j <=i; j++)
	{
	int x=j;
	int y=i-j;
	if ((x < 4096) && (y < 4096))
		{
		double val=diagonal->GetChan(x); //Background subtracted diagonal
		double val2=diagonalb->GetChan(x); //diagonal before bk sub
		double bkg=(val2-val);
		diabk2->Set(x,y,bkg); //The background matrix itself
		}
	}
diagonal->Clear();
diagonalb->Clear();
}

//Apply the diagonal correction matrix
for (int i=0; i < 4096; i++)
{
	for (int j=0; j < 4096; j++)
	{
	double val=bksub2->Get(i,j);
	double val2=diabk2->Get(i,j); //diagonal bks
	bksub3->Set(i,j,(val-val2));
	}
}
//Find the result matrix
for (int i=0; i < 4096; i++)
{
	for (int j=0; j < 4096; j++)
	{
	double correction=bksub3->Get(i,j)-bksub1->Get(i,j);
	double val=mat1->Get(i,j); 
	result->Set(i,j,(val+correction));
	}
}

bksub3->ReSymmetrise();
bksub1->Write("bksub1.m4b"); //classic radware bkg subtracted
bksub1->FindMinMax();
bksub2->Write("bksub2.m4b"); //classic RW + stripes corrected
bksub2->FindMinMax();
bksub3->Write("bksub3.m4b"); //classic RW + also the diagonal component removed
bksub3->FindMinMax();
result->Write("result.m4b"); //mat1 minus only the non-radware corrections

diabk1->Write("diabk1.m4b"); 
diabk2->Write("diabk2.m4b"); 

}

/*


g++ -g -o MatSubCrissCross MatSubCrissCross.cxx  ` root-config --cflags` `root-config --glibs` -lSpectrum


*/


