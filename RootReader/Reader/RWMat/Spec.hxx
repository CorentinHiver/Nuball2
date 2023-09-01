#ifndef _Spec_
#define _Spec_ 

#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

class Spec
{
 public:
	Spec(int nchans, string name="test.spe");	//@- Default constructor
	Spec();	//@- Empty spectrum constructor for reading in
	Spec(TH1D *RootSpectrum); //Construct from pre-existing root spectrum
	~Spec(); 			//@- Normal destructor
	bool Write(); //write the spectrum out (returns true if successful)
	bool Write(string fname); //write the spectrum out (returns true if successful)
	void Inc(int channel); //increment channel by 1 count
	void Inc(int channel, int n); //increment channel by n counts
	void Inc(int channel, double n); //increment channel by n counts
	double Integrate(); //sum counts
	double Integrate(int c1, int c2); //sum counts
	void Read(string Name);
	double GetChan(unsigned short chan) {if (chan < fNChannels) {return fSpectrum[chan];} else {return 0;}}
	void SetChan(unsigned short chan, double val) {fSpectrum[chan]=val;}
	unsigned short GetNChans() {return fNChannels;}
	string GetName() {return fName;}
	void SetName(string Name) {fName=Name;}
	double FindCentroid(int c1, int c2, int width=30, int itdepth=2); // Finds the centroid value between these two channels
	int MaxChan(int c1=1, int c2=10000);//Find the channel with the most counts
	void Clear();//zero the spectrum
	Spec* AddSpecs(Spec* spec, double factor=1.0); //Add two specs together to make a third new one
	void AddSpec(Spec* spec); //Add another spectrum to this one
	int GetNumberOfChannels() {return fNChannels;}
	Spec* Rebin(int x1, int y1, int x2, int y2); // rebin section of spec into new spec
	void Mult(double factor) {for (int i=0; i < fNChannels; i++) {fSpectrum[i]*=factor;}}
	void Mult(Spec* ts) {for (int i=0; i < fNChannels; i++) {fSpectrum[i]*=ts->GetChan(i);}}
	void AddPeak(double centroid, double width, double counts);
	double sb(int b1,int p1,int p2,int b2);
	Spec* Clone(bool SetToZero=true);
	void Copy(Spec* ts) {for (int i=0; i < fNChannels; i++) {fSpectrum[i]=ts->GetChan(i);}}
	void Smooth(int width=10);
	void SmoothGauss(int width=10);
	void Normalize(double norm=1.0); // Normalize the area to norm counts
	void Contract(int CFactor, int offset=0); //contract the spectrum by an integer factor x
	Spec* Rebin2(int x1, int y1, int x2, int y2);  // rebin section of spec into new spec
	void Clear(int c1, int c2); //zero the spectrum between these channels
	void Divide(Spec* thespec); // Divide spectrum by another spec
	int MinChan(int c1, int c2); //Find the minimum channel between two limits
	TH1D* Convert2Root(string name=""); //Convert this spectrum to new root spectrum
	TH1D* backsub(TH1D* TheSpectrum, int nsmooth); //Returns the bkg under a root spectrum
	Spec* backsub(int nsmooth); //Returns the bkg under this spectrum
	void SetWidthCal(double par1, double par2) {fWpar1=par1; fWpar2=par2;} //Define a radware width calibration
	double GetFWHM(int chan) {return sqrt(fWpar1*fWpar1+(fWpar2*fWpar2*chan/1000.0));}
	double GetSigma(int chan) {return GetFWHM(chan)/2.355;}
	//New fit routines with passage of two spectra and three vectors (energy, intensity, intensity_err)
	void Fit1(TH1D* rs, TH1D* rsraw,int index,vector<double> &en,vector<double> &in,vector<double> &in_err); //Fixed Peak Fit
	void FFit1(TH1D* rs, TH1D* rsraw,int index,vector<double> &en,vector<double> &in,vector<double> &in_err); //Free Peak Fit
	void Fint1(TH1D* rs, TH1D* rsraw,int index,vector<double> &en,vector<double> &in,vector<double> &in_err); //Peak Integrate
	void Fit2(TH1D* rs, TH1D* rsraw,int idx1,int idx2,vector<double> &en,vector<double> &in,vector<double> &in_err); 
	//same as before but with a specified contaminant energy econt
	void Fit2(TH1D* rs, TH1D* rsraw,int idx1,double econt,vector<double> &en,vector<double> &in,vector<double> &in_err); 
	void SetEfficiency(string effspecname);
	void SetEfficiency(string effspecname, double scalefactor);	
double GetEffCor(int chan) {return 1000.0/fEfficiency[chan];}
	void MakeLabels(vector<double> &peaks, vector<double> &inten);
 protected:
	unsigned short 	fNChannels;
	double 		fKeVPerChannel;	
	string 		fName;
	double* 	fSpectrum;
	double		fTotalCounts;
	double		fWpar1;
	double		fWpar2;
	double*		fEfficiency;
};
#endif
