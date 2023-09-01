#include <fstream>
#include <sstream>
#include <iomanip>

Spec::Spec(int nchans, string name)
{
fNChannels=nchans;
fName=name;
fSpectrum=new double[fNChannels];
for (int i=0; i < fNChannels; i++) {fSpectrum[i]=0;}
fKeVPerChannel=0.5; //Default value
fWpar1=1.0;
fWpar2=3.0;
fEfficiency=new double[fNChannels];
for (int i=0; i < fNChannels; i++) {fEfficiency[i]=1.0;} //Default efficiency value is 1000
}
//________________________________________________________________________
//Construct equivalent Root Spectrum from this spectrum
Spec::Spec(TH1D *RootSpectrum)
{
fNChannels=RootSpectrum->GetNbinsX();
fName=RootSpectrum->GetTitle();
for (int i=0; i < fNChannels; i++) {fSpectrum[i]=RootSpectrum->GetBinContent(i-1);}
fKeVPerChannel=0.5; //Default value
fWpar1=1.0;
fWpar2=3.0;
fEfficiency=new double[fNChannels];
for (int i=0; i < fNChannels; i++) {fEfficiency[i]=1.0;} //Default efficiency value is 1000
}
//________________________________________________________________________
Spec::~Spec()
{
delete [] fSpectrum;

}
//________________________________________________________________________
bool Spec::Write()
{
//if (Integrate() > 0)
//{
int size=fNChannels;
cout << "Writing Spectrum <" << fName << "> to disk : Total counts = " << Integrate() << endl;

FILE *fprad;
fprad = fopen(fName.c_str(),"w");
int i;
int j = 24;
int toto;
int tab[4] = {0,1,1,1};
	 
tab[0] = size; 
toto = size * sizeof(float); 

float* buffer=new float[size];

for (int i=0; i < size; i++) {buffer[i]=float(fSpectrum[i]);}

int dum;	 
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(fName.c_str(), 8, 1, fprad); 
dum = fwrite(tab, sizeof(int), 4, fprad);
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(&toto, sizeof(int), 1, fprad);
dum = fwrite(buffer, sizeof(float), size, fprad);
dum = fwrite(&toto,sizeof(int),1,fprad);

fclose(fprad); 
return true;
//}
//cout << "Warning: Spectrum <" << fName << "> is not written because it has zero counts " <<endl;
//return false;
}
//________________________________________________________________________
bool Spec::Write(string fname)
{
fName=fname;
return Write();
}
//________________________________________________________________________
void Spec::Inc(int channel)
{
//cout << "incrementing " << endl;
if ((channel < fNChannels) && (channel >= 0))
	{
	fSpectrum[channel]++;
	//cout << "fSpectrum[channel] " << fSpectrum[channel] << endl;
	}
}
//________________________________________________________________________
void Spec::Inc(int channel, int counts)
{
//cout << "incrementing " << endl;
if ((channel < fNChannels) && (channel >= 0))
	{
	fSpectrum[channel]+=counts;
	//cout << "fSpectrum[channel] " << fSpectrum[channel] << endl;
	}
}
//________________________________________________________________________
void Spec::Inc(int channel, double counts)
{
//cout << "incrementing " << endl;
if ((channel < fNChannels) && (channel >= 0))
	{
	fSpectrum[channel]+=counts;
	//cout << "fSpectrum[channel] " << fSpectrum[channel] << endl;
	}
}
//________________________________________________________________________
double Spec::Integrate()
{
fTotalCounts=0;
	for (int i=0; i < fNChannels; i++)
	{
	fTotalCounts+=fSpectrum[i];
	}
return fTotalCounts;
}
//________________________________________________________________________
double Spec::Integrate(int c1, int c2)
{
double total=0;
if (c1 > c2) {int temp=c1; c2=c1; c1=temp;}
if ((c1 >= 0) && (c2 < fNChannels))
	{
	for (int i=c1; i < c2; i++)
		{
		total+=fSpectrum[i];
		}
	fTotalCounts=total;
	return total;
	}
else
	{
	return -1;
	}
	
}
//________________________________________________________________________
void Spec::Read(string Name)
{
fName=Name;
FILE *fprad;
	
fprad = fopen(fName.c_str(),"r");
if (fprad) {cout << "Reading spectrum : " << fName << endl;}
else {cout << "Error Reading spectrum : " << fName << endl; exit(1);}

// now read the spectrum
char nom[24], nam[8]; int ndim[4], dum[2], i, j, toto, rval;
int soi=sizeof(int);

rval=fread(&j, soi, 1, fprad);
rval=fread(&nam, 8, 1, fprad); 
rval=fread(&ndim, soi, 4, fprad);
rval=fread(&j, sizeof(int), 1, fprad);
rval=fread(&toto, sizeof(int), 1, fprad);

//cout << j << " " << nam << " " << toto << " " <<  ndim[0] << endl;
int size=ndim[0];
float* buffer=new float[size];

rval=fread(buffer, size, sizeof(float), fprad);
//fread(&toto,sizeof(int),1,fprad);

//fNChannels=size; 
cout << "Number of channels = " << fNChannels <<  endl;
cout << "Number of channels read in = " << size <<  endl;
//delete [] fSpectrum;
//fSpectrum=new double[size];
if (size > fNChannels) {cout << "Spectrum is too big to read in" << endl; exit(1);}
for (i=0; i<ndim[0]; i++) 
{
fSpectrum[i]=buffer[i];
//cout << fSpectrum[i] << endl;
}


}

//________________________________________________________________________
double Spec::FindCentroid(int c1, int c2, int width, int itdepth)
{

//cout << "c1 = " << c1 << " c2 = " << c2 << endl;
double total=0;
double xCounts=0;

if (c1 > c2) {int temp=c1; c2=c1; c1=temp;}
if ((c1 > 0) && (c2 < fNChannels))
	{
	for (int i=c1; i < c2; i++)
		{
		total+=fSpectrum[i];
		xCounts+=(fSpectrum[i]*i);
		}
	//cout << "counts*i = " <<  xCounts << " Total counts = "<< total<< endl;
	double centroid=xCounts/total;
	itdepth--;
	//cout << centroid << " " << width << endl;
	double oldcentroid=centroid;
	if (itdepth >= 1) {centroid=FindCentroid(int(centroid-width/1.5),int(centroid+width/1.5),width,itdepth);}
	if (fabs(oldcentroid-centroid) < 0.5) {itdepth=0;} // Quit the recursion. We have converged on a value
	return centroid;
	}
else
	{
	return -1;
	}
	
}
//________________________________________________________________________
int Spec::MaxChan(int c1, int c2)
{
int maxchan=0;
double maxval=0;
if (c2 > fNChannels){c2=fNChannels;}
for (int i=c1; i < c2; i++)
	{
	if (fSpectrum[i] > maxval) {maxval=fSpectrum[i]; maxchan=i;}
	}
return maxchan;
}
//________________________________________________________________________
void Spec::Clear()
{

for (int i=0; i < fNChannels; i++) {fSpectrum[i]=0;}
}
//________________________________________________________________________
void Spec::AddSpec(Spec* spec)
{
for (int i=0; i < fNChannels; i++) 
	{
	fSpectrum[i]+=spec->GetChan(i);
	}
}

//________________________________________________________________________
Spec* Spec::AddSpecs(Spec* spec, double factor)
{
Spec* test=new Spec(fNChannels);

for (int i=0; i < fNChannels; i++) 
	{
	test->SetChan(i,(fSpectrum[i]+(spec->GetChan(i)*factor)));
	}
return test;
}

//________________________________________________________________________
Spec* Spec::Rebin(int x1, int y1, int x2, int y2)
{
// Map channel x1 to y1 and channel x2 to channel y2.
Spec* newspec=this->Clone();
double m=double((y2-y1)-1)/double((x2-x1)-1); 
// y1=m*x1+c;
double c=y1-m*x1;

// mapping (x2-x1) bins to (y2-y1) bins
//cout << "m = " << m << " c = " << c << endl;
for (int i=x1; i < x2; i++) 
	{
	double counts=fSpectrum[i];
	//where should the counts from bin i go?
	//jth channel is 
	double j=m*double(i)+c;
	double j1=m*double(i+1)+c;
	double p=(j-int(j))/(j1-j);
	//doublep2=(j1-int(j1))/(j1-j);
	double q=1.0-p;
	//cout << j << " " << j1 << " " << p << " " << q << endl;
	double f1,f2,f3;
	f1=p; f2=1.0-p;
	f3=0;
	//if (1) {f1=p; f2=1.0-p;} else {f2=0; f1=0;}
	//if (f2 > 1.0/m) {f3=f2-1.0/m;}
	//if (f1 > 1.0/m) {f3=f1-1.0/m;}
	newspec->Inc(int(j),counts*f2);
	newspec->Inc(int(j+1),counts*f1);
	//if (f3 > 0 ) {newspec->Inc(int(j-1),-counts*f3); newspec->Inc(int(j),counts*f3);}
	
	}
// Get rid of spike artifacts
if (y2 > 2)
{
for (int j=y1; j < y2-2; j++) 
{
double v1=newspec->GetChan(j);
double v2=newspec->GetChan(j+1);
double v3=newspec->GetChan(j+2);
if ((v2 < v1) && (v3 > v1)) {newspec->SetChan(j+1,(v2+v3)/2.0); newspec->SetChan(j+2,(v2+v3)/2.0);}
}
}
return newspec;
}
//________________________________________________________________________

void Spec::AddPeak(double centroid, double width, double counts)
{

double sigma=width/2.3548;
int c1=int(centroid-width*2.0);
int c2=int(centroid+width*2.0);

for (int j=c1; j <= c2; j++)
	{
	if ((j >= 0) && (j < fNChannels))
		{
		double x=double(j);
		double x0=centroid;
		double f=exp(-((x-x0)*(x-x0)/(2.0*(sigma*sigma))));
		double val=(f*counts)/(sigma*sqrt(2*3.14159));
		fSpectrum[j]+=val;
		}
	}
}
//________________________________________________________________________

double Spec::sb(int b1,int p1,int p2,int b2)
{
double back1=Integrate(b1,p1);
double peak=Integrate(p1,p2);
double back2=Integrate(p2,b2);
double backavg=((back1/double(p1-b1))+(back2/double(b2-p2)))/2;
double backtot=backavg*double(p2-p1);
cout << back1 << " "<< peak<<" "<< back2<<" "<< backavg << " "<<backtot<<endl; 

return peak-backtot;
}
//________________________________________________________________________

Spec* Spec::Clone(bool SetToZero)
{
Spec* newspec=new Spec(fNChannels); 
newspec->SetWidthCal(fWpar1,fWpar2); //Copy the width calibration

for (int i=0; i < fNChannels; i++) 
{
if (SetToZero) {newspec->SetChan(i,0);}
else {newspec->SetChan(i,fSpectrum[i]);}
}

return newspec;
}
//________________________________________________________________________

void Spec::SmoothGauss(int width)
{
Spec* newspec=this->Clone();

for (int i=0; i < fNChannels; i++)
	{
	double counts=fSpectrum[i];
	newspec->AddPeak(i,width,counts);
	}
for (int i=0; i < fNChannels; i++) {fSpectrum[i]=newspec->GetChan(i);}
}

//________________________________________________________________________

void Spec::Smooth(int width)
{
Spec* newspec=this->Clone();

for (int i=0; i < fNChannels; i++)
	{
	double val=0;
	int nchans=0;
	int c1=int(i-width/2.0);
	int c2=int(i+width/2.0);
	for (int j=c1; j <= c2; j++)
		{
		if ((j >=0) && (j < fNChannels))
			{
			val+=fSpectrum[j];
			nchans++;
			}
		}
	newspec->SetChan(i,val/double(nchans));
	}
for (int i=0; i < fNChannels; i++) {fSpectrum[i]=newspec->GetChan(i);}
}

//________________________________________________________________________

void Spec::Normalize(double norm)
{
double val=Integrate();
if (val != 0) {Mult(norm/val);}
}

//________________________________________________________________________
Spec* Spec::Rebin2(int x1, int y1, int x2, int y2)
{
// Map channel x1 to y1 and channel x2 to channel y2.
Spec* reb=new Spec(fNChannels);
double m=double((y2-y1))/double((x2-x1)); 
// y1=m*x1+c;
double c=double(y1)-m*double(x1);

for (int i=x1; i < x2; i++)
{
// For integers it's easy. 1 chan maps to 2 chans. Put 1/2 counts in a and 1/2
//counts in b
double counts=fSpectrum[i];
double newchan=m*double(i)+c;
double newchan2=m*double(i+1)+c; // next mapping channel
if ((newchan >= 0) && (counts > 0) && (newchan2 < 1024))
	{
	double spread=(newchan2-newchan); // Number of new channels to spread over
	int inewchan=int(newchan);
	int inewchan2=int(newchan2);


	if (inewchan==inewchan2) {reb->Inc(inewchan,counts);} // Dump ALL the counts in this bin!
	else
		{
		//First the fractional new channels
		double end1=1.0-(newchan-inewchan);
		double end2=newchan2-inewchan2;
		double fraccounts1=counts*end1/spread;
		double fraccounts2=counts*end2/spread;
		reb->Inc(inewchan,fraccounts1); //  fractional new chanel 1 (beginning)
		reb->Inc(inewchan2,fraccounts2);// fractional new channel 2 (end)
		for (int j=inewchan+1; j < inewchan2; j++)
			{
			reb->Inc(j,counts/spread); // whole new channel in the middle
			}
		}
	}
}

return reb;
}
//________________________________________________________________________

void Spec::Contract(int CF, int offset)
{
Spec* newspec=this->Clone();
for (int i=0; i < (fNChannels-offset); i++) {
//fSpectrum[i/CF]+=fSpectrum[i];
newspec->Inc(i/CF,fSpectrum[i+offset]);
}
for (int i=0; i < fNChannels; i++) {cout << i << " " << newspec->GetChan(i)<< endl;}
//for (int i=fNChannels/CF; i < fNChannels; i++) {fSpectrum[i]=0;}
for (int i=0; i < fNChannels; i++) {fSpectrum[i]=newspec->GetChan(i)/CF;}
}
//________________________________________________________________________
void Spec::Clear(int c1, int c2)
{
for (int i=c1; i <= c2; i++) {fSpectrum[i]=0;}
}
//________________________________________________________________________
void Spec::Divide(Spec* thespec)
{
for (int i=0; i < fNChannels; i++) 
	{
	if (thespec->GetChan(i) != 0) {fSpectrum[i]=fSpectrum[i]/thespec->GetChan(i);}
	}
}
//________________________________________________________________________
int Spec::MinChan(int c1, int c2)
{
int theminchan;
double maxval=10000000000000000;
for (int i=c1; i <= c2; i++) 
	{
	if (fSpectrum[i] < maxval) {maxval=fSpectrum[i]; theminchan=i;}
	}

return theminchan;
}
//________________________________________________________________________
TH1D* Spec::Convert2Root(string name)
{
if (name.size()==0) {name=fName.substr(0,fName.size()-4);} //Lop off the .spe for the name
else {fName=name;}
cout << "Creating new root spectrum : " << name << " with " << fNChannels << " channels" << endl;
TH1D* RootSpectrum=new TH1D(fName.c_str(),fName.c_str(),fNChannels,0,fNChannels);
for (int i=0; i < fNChannels; i++) {RootSpectrum->SetBinContent(i+1,this->GetChan(i));}
return RootSpectrum;
}
//________________________________________________________________________
TH1D* Spec::backsub(TH1D* TheSpectrum, int nsmooth)
{
int j;
int nbins=TheSpectrum->GetNbinsX();
string name=TheSpectrum->GetTitle();
name=name+"b";

TH1D* TheNewSpectrum= new TH1D(name.c_str(),name.c_str(),nbins,0,nbins);

Double_t *source=new Double_t[nbins];
TSpectrum *s = new TSpectrum();
for (j=0;j<nbins;j++) source[j]=TheSpectrum->GetBinContent(j+1);
s->Background(source,nbins,nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
for (j=0;j<nbins;j++) {TheNewSpectrum->SetBinContent(j+1, TheSpectrum->GetBinContent(j+1)-source[j]);}
s->Delete();
delete [] source;
return TheNewSpectrum;
}
//________________________________________________________________________
Spec* Spec::backsub(int nsmooth)
{
Spec* newspec=this->Clone();
int j;
int nbins=4096;
Double_t *source=new Double_t[nbins];
TSpectrum *s = new TSpectrum();
for (j=0;j<nbins;j++) source[j]=this->GetChan(j);
s->Background(source,nbins,nsmooth,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kTRUE,TSpectrum::kBackSmoothing3,kFALSE);
for (j=0;j<nbins;j++) {newspec->SetChan(j, fSpectrum[j]-source[j]);}
s->Delete();
delete [] source;
return newspec;
}
// ROOT GAUSSIAN FITTING FUNCTION HAS THE FOLLOWING FORM
//f(x) = p0*exp(-0.5*((x-p1)/p2)^2)
//po is the HEIGHT, p1 is the position and p2 is the sigma
//HEIGHT = AREA / (sigma*sqrt(2pi))
//AREA=HEIGHT*(sigma*sqrt(2pi))
//________________________________________________________________________
void Spec::Fit1(TH1D* spec,TH1D* rawspec, int index, vector<double> &en, vector<double> &in,  vector<double> &in_err)
{
// FIXED Fit. Do not allow the energy to vary from its true value.
double energy=en[index];
TF1 *g1 = new TF1("g1","gaus",energy-(GetFWHM(energy)*2),energy+(GetFWHM(energy)*2));
//cout << GetFWHM(energy) << " " << GetSigma(energy) << endl;
g1->FixParameter(1,energy+0.5);
//g1->SetParLimits(1,energy+0.25,energy+0.75);
g1->FixParameter(2,GetSigma(energy));
//g1->SetParLimits(2,GetSigma(energy)*0.8,GetSigma(energy)*1.2);

spec->Fit(g1,"R+B");
cout << "Chi squared = " << g1->GetChisquare() << endl;

double lo=(energy+0.5)-(GetSigma(energy));
double hi=(energy+0.5)+(GetSigma(energy));

double rscounts=rawspec->Integral(lo,hi);
double stat_err=sqrt(rscounts);
double h2a=GetSigma(energy)*2.50662827463;

in[index]=g1->GetParameter(0)*fEfficiency[int(energy)]*h2a;
in_err[index]=stat_err*fEfficiency[int(energy)]*h2a;

if (in[index] < 0) {in[index]=0;} //Set negative intensity to zero.

cout << "Energy, Fitted-Intensity, Error " << endl;
cout << energy << " "  <<in[index] <<" "<< in_err[index]<< endl;
}
//________________________________________________________________________
void Spec::FFit1(TH1D* spec,TH1D* rawspec, int index, vector<double> &en, vector<double> &in,  vector<double> &in_err)
{
//FREE FIT! Allow the energy to vary a tiny bit +/- 0.25 keV of its true value.
double energy=en[index];
TF1 *g1 = new TF1("g1","gaus",energy-(GetFWHM(energy)*2),energy+(GetFWHM(energy)*2));
//cout << GetFWHM(energy) << " " << GetSigma(energy) << endl;
g1->FixParameter(1,energy+0.5);
g1->SetParLimits(1,energy+0.25,energy+0.75);
g1->FixParameter(2,GetSigma(energy));
g1->SetParLimits(2,GetSigma(energy)*0.8,GetSigma(energy)*1.2);
spec->Fit(g1,"R+B");
cout << "Chi squared = " << g1->GetChisquare() << endl;

double lo=(energy+0.5)-(GetSigma(energy));
double hi=(energy+0.5)+(GetSigma(energy));

double rscounts=rawspec->Integral(lo,hi);
double stat_err=sqrt(rscounts);
double h2a=GetSigma(energy)*2.50662827463;

in[index]=g1->GetParameter(0)*fEfficiency[int(energy)]*h2a;
in_err[index]=stat_err*fEfficiency[int(energy)]*h2a;

if (in[index] < 0) {in[index]=0;} //Set negative intensity to zero.

cout << "Energy, Fitted-Intensity, Error " << endl;
cout << energy << " "  <<in[index] <<" "<< in_err[index]<< endl;
}

//________________________________________________________________________
void Spec::Fint1(TH1D* spec,TH1D* rawspec, int index, vector<double> &en, vector<double> &in,  vector<double> &in_err)
{
double energy=en[index];

double nsigma=2.0; //Two sigma range for the integration. Plus or minus 2 sigma = 95% of the counts
double lo=(energy+0.5)-(GetSigma(energy)*nsigma);
double hi=(energy+0.5)+(GetSigma(energy)*nsigma);

//Integrate first and then force fit
double scounts=spec->Integral(lo,hi+1);
double rscounts=rawspec->Integral(lo,hi+1);
double stat_err=sqrt(rscounts);

//Two sigma range for the forced fit
TF1 *g1 = new TF1("g1","gaus",energy-(GetSigma(energy)*nsigma),energy+(GetSigma(energy)*nsigma)+1);
cout << GetFWHM(energy) << " " << GetSigma(energy) << endl;
g1->FixParameter(0,scounts/(GetSigma(energy)*2.506));
g1->FixParameter(1,energy+0.5);
g1->FixParameter(2,GetSigma(energy));
spec->Fit(g1,"R+B");


in[index]=scounts*fEfficiency[int(energy)];
in_err[index]=stat_err*fEfficiency[int(energy)];
if (in[index] < 0) {in[index]=0;} //Set negative intensity to zero.

cout << "Energy, Integrated-Intensity, Error " << endl;
cout << energy << " "  <<in[index] <<" "<< in_err[index]<< endl;

}

//________________________________________________________________________
void Spec::Fit2(TH1D* spec, TH1D* rawspec,int idx1,int idx2,vector<double> &en,vector<double> &in,vector<double> &in_err)
{
double energy1=en[idx1];
double energy2=en[idx2];
TF1 *g1 = new TF1("g1","gaus",energy1-(GetFWHM(energy1)*2),energy1+(GetFWHM(energy1)*2));
TF1 *g2 = new TF1("g2","gaus",energy2-(GetFWHM(energy2)*2),energy2+(GetFWHM(energy2)*2));

Double_t par[6];

g1->FixParameter(1,energy1+0.5);
g1->FixParameter(2,GetSigma(energy1));
spec->Fit(g1,"R+B");
cout << "Chi squared = " << g1->GetChisquare() << endl;

g2->FixParameter(1,energy2+0.5);
g2->FixParameter(2,GetSigma(energy2));
spec->Fit(g2,"R+B");
cout << "Chi squared = " << g2->GetChisquare() << endl;
  
TF1 *total = new TF1("total","gaus(0)+gaus(3)",100,1500);   
total->SetLineColor(8);  
g1->GetParameters(&par[0]);
g2->GetParameters(&par[3]);

total->FixParameter(1,energy1+0.5);
total->FixParameter(2,GetSigma(energy1));

total->FixParameter(4,energy2+0.5);
total->FixParameter(5,GetSigma(energy2));

total->SetParameters(par);
spec->Fit(total,"R+B");

double lo1=(energy1+0.5)-(GetSigma(energy1));
double hi1=(energy1+0.5)+(GetSigma(energy1));
double lo2=(energy2+0.5)-(GetSigma(energy2));
double hi2=(energy2+0.5)+(GetSigma(energy2));

double rscounts1=rawspec->Integral(lo1,hi1);
double stat_err1=sqrt(rscounts1);
double rscounts2=rawspec->Integral(lo2,hi2);
double stat_err2=sqrt(rscounts2);

double h2a1=GetSigma(energy1)*2.50662827463;
double h2a2=GetSigma(energy2)*2.50662827463;

in[idx1]=total->GetParameter(0)*fEfficiency[int(energy1)]*h2a1;
in_err[idx1]=stat_err1*fEfficiency[int(energy1)]*h2a1;

in[idx2]=total->GetParameter(3)*fEfficiency[int(energy2)]*h2a2;
in_err[idx2]=stat_err2*fEfficiency[int(energy2)]*h2a2;
if (in[idx1] < 0) {in[idx1]=0;} //Set negative intensity to zero.
if (in[idx2] < 0) {in[idx2]=0;} //Set negative intensity to zero.

cout << energy1 << " "  <<in[idx1] <<" "<< in_err[idx1]<< endl;
cout << energy2 << " "  <<in[idx2] <<" "<< in_err[idx2]<< endl;

}

//________________________________________________________________________
void Spec::Fit2(TH1D* spec, TH1D* rawspec,int idx1,double econt,vector<double> &en,vector<double> &in,vector<double> &in_err)
{
double energy1=en[idx1];
double energy2=econt;

TF1 *g1 = new TF1("g1","gaus",energy1-(GetFWHM(energy1)*2),energy1+(GetFWHM(energy1)*2));
TF1 *g2 = new TF1("g2","gaus",energy2-(GetFWHM(energy2)*2),energy2+(GetFWHM(energy2)*2));

Double_t par[6];

g1->FixParameter(1,energy1+0.5);
g1->FixParameter(2,GetSigma(energy1));
spec->Fit(g1,"R+B");
cout << "Chi squared = " << g1->GetChisquare() << endl;

g2->FixParameter(1,energy2+0.5);
g2->FixParameter(2,GetSigma(energy2));
spec->Fit(g2,"R+B");
cout << "Chi squared = " << g2->GetChisquare() << endl;
  
TF1 *total = new TF1("total","gaus(0)+gaus(3)",100,1500);   
total->SetLineColor(8);  
g1->GetParameters(&par[0]);
g2->GetParameters(&par[3]);

total->FixParameter(1,energy1+0.5);
total->FixParameter(2,GetSigma(energy1));

//total->FixParameter(4,energy2+0.5);
total->FixParameter(5,GetSigma(energy2));

total->SetParameters(par);
spec->Fit(total,"R+B");

double lo1=(energy1+0.5)-(GetSigma(energy1));
double hi1=(energy1+0.5)+(GetSigma(energy1));
double lo2=(energy2+0.5)-(GetSigma(energy2));
double hi2=(energy2+0.5)+(GetSigma(energy2));

double rscounts1=rawspec->Integral(lo1,hi1);
double stat_err1=sqrt(rscounts1);
double rscounts2=rawspec->Integral(lo2,hi2);
double stat_err2=sqrt(rscounts2);

double h2a1=GetSigma(energy1)*2.50662827463;
double h2a2=GetSigma(energy2)*2.50662827463;

in[idx1]=total->GetParameter(0)*fEfficiency[int(energy1)]*h2a1;
in_err[idx1]=stat_err1*fEfficiency[int(energy1)]*h2a1;

if (in[idx1] < 0) {in[idx1]=0;} //Set negative intensity to zero.
//in[idx2]=total->GetParameter(3);
//in_err[idx2]=stat_err2;

cout << energy1 << " "  <<in[idx1] <<" "<< in_err[idx1]<< endl;
cout << energy2 << " C "  << total->GetParameter(3)*h2a2<< " " <<stat_err2*h2a2<< endl; //contaminant energy

}
//________________________________________________________________________
void Spec::SetEfficiency(string effspecname) //Default scale factor is 1000
{
Spec* dummy=new Spec(fNChannels);
dummy->Read(effspecname.c_str());
for (int i=0; i < fNChannels; i++) {fEfficiency[i]=1000.0/dummy->GetChan(i);}
//delete dummy;
}
//________________________________________________________________________
void Spec::SetEfficiency(string effspecname, double scalefactor)
{
Spec* dummy=new Spec(fNChannels);
dummy->Read(effspecname.c_str());
for (int i=0; i < fNChannels; i++) {fEfficiency[i]=scalefactor/dummy->GetChan(i);}
//delete dummy;
}
//________________________________________________________________________
void Spec::MakeLabels(vector <double> &peaks, vector <double> &inten)
{
TText *t = new TText();
t->SetTextAlign(32);
t->SetTextSize(0.023);

for (int i=0; i < peaks.size(); i++)
{
string lab=Form("%d",i);
t->DrawText(peaks[i]+1,inten[i]/2.56,lab.c_str());
}

}

