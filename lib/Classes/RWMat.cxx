#include <fstream>
#include <sstream>
#include <iomanip>
#include "RWMat.hxx"


RWMat::RWMat(string name, int nchans) //default constructor
{
fNChannels=nchans;
fName=name;
fRWMat=new int*[fNChannels];
for(int i=0 ; i < fNChannels ; i++) {fRWMat[i] = new int[fNChannels];}
}
RWMat::RWMat(TH2F* RootMat) //constructor from root object
{
int xchans=RootMat->GetNbinsX();
int ychans=RootMat->GetNbinsY();
fNChannels=4096;
fName=RootMat->GetName();
fName=fName+".m4b";
fRWMat=new int*[fNChannels];
for(int i=0 ; i < fNChannels ; i++) {fRWMat[i] = new int[fNChannels];}
double val=0;
for (int i=0; i < xchans ; i++)
{
	for (int j=0; j < ychans ; j++)
	{
	fRWMat[i][j]=RootMat->GetBinContent(i,j);
	val+=fRWMat[i][j];
	}	
}
//cout << "RW Matrix Created. Total Counts = " << val << endl;
//cout << "filename = " << fName << endl;
}
//________________________________________________________________________
RWMat::~RWMat()
{
for (int i=0; i < fNChannels; i++)
 {
 delete [] fRWMat[i];
 }
 delete [] fRWMat;


}
//________________________________________________________________________
//void RWMat::WriteProjection(string fname)
//{
//cout << "Writing Projection <" << fname << "> to disk " << endl;
//Integrate();
//WriteSpec(fname, fProjection);
//}
//________________________________________________________________________
//void RWMat::WriteProjectiony(string fname)
//{
//cout << "Writing Y Projection <" << fname << "> to disk " << endl;
//Integrate();
//WriteSpec(fname, fProjectiony);
//}
//________________________________________________________________________
/*void RWMat::WriteSpec(string fname, double* spectrum)
{
int size=fNChannels;

FILE *fprad;
fprad = fopen(fname.c_str(),"w");
int i;
int j = 24;
int toto;
int tab[4] = {0,1,1,1};
	 
tab[0] = size; 
toto = size * sizeof(float); 
float* buffer=new float[size];

for (int i=0; i < size; i++) {buffer[i]=float(spectrum[i]);}

int dum;	 
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(fName.c_str(), 8, 1, fprad); 
dum = fwrite(tab, sizeof(int), 4, fprad);
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(&toto, sizeof(int), 1, fprad);
dum = fwrite(spectrum, sizeof(float), size, fprad);
dum = fwrite(&toto,sizeof(int),1,fprad);

fclose(fprad); 
}*/
//________________________________________________________________________
void RWMat::Fill(unsigned short i, unsigned short j)
{
if ((i < fNChannels) && (j < fNChannels)){fRWMat[i][j]++;}
}
//________________________________________________________________________
void RWMat::Read(string fname, bool IsInteger)
{
fName=fname;
FILE *fprad;
	
fprad = fopen(fname.c_str(),"r");
if (fprad) {cout << "Reading RWMat : " << fname << endl;}
else {cout << "Error Reading RWMat : " << fname << endl; exit(1);}

int size=fNChannels;

 double* buffer=new  double[size];
 int* bufferi=new  int[size], rval;

cout << "Number of channels = " << fNChannels <<  endl;
for (int i=0; i<size; i++) 
{
	if (IsInteger) 
		{
		rval=fread(bufferi, size, sizeof( int), fprad);
		for (int j=0; j<size; j++) {fRWMat[i][j]=bufferi[j];}
		}
	else 
		{
		rval=fread(buffer, size, sizeof( double), fprad);
		for (int j=0; j<size; j++) {fRWMat[i][j]=buffer[j];}
		}
	
}
fclose(fprad);
delete [] buffer;
delete [] bufferi;
}
//________________________________________________________________________
void RWMat::Write(string name)
{

FILE *fprad;	
if (name!="") {fName=name;}
else {name=fName;}

fprad = fopen(name.c_str(),"w");
if (fprad) {cout << "Writing RWMat : " << name << endl;}
else {cout << "Error Writing RWMat : " << name << endl; exit(1);}

int size=fNChannels;

int* buffer=new  int[size];

cout << "channels = " << fNChannels << " counts = " << this->Integral()<< endl;
for (int i=0; i<size; i++) 
{
	for (int j=0; j<size; j++) 
	{
	buffer[j]=fRWMat[i][j];
	}
fwrite(buffer, size, sizeof( int), fprad);
}
fclose(fprad);

delete [] buffer;

}
//________________________________________________________________________
RWMat* RWMat::Add(RWMat* Matrix,double val)
{
RWMat *mat3=new RWMat();
for (int i=0; i<fNChannels; i++) 
{
	for (int j=0; j<fNChannels; j++) 
	{
	mat3->Set(i,j,(fRWMat[i][j]+(Matrix->Get(i,j)*val)));
	}
}

return mat3;
}
//________________________________________________________________________
double RWMat::Integral()
{
double val=0;
for (int i=0; i<fNChannels; i++) 
{
	for (int j=0; j<fNChannels; j++) 
	{
	val+=fRWMat[i][j];
	}
}

return val;
}

//________________________________________________________________________
void RWMat::ReSymmetrise()
{
for (int i=0; i<fNChannels; i++) 
{
	for (int j=0; j<i; j++) 
	{
	double val1=fRWMat[i][j];
	double val2=fRWMat[j][i];
	fRWMat[i][j]=(val1+val2)/2.0;
	fRWMat[j][i]=(val1+val2)/2.0;
	}
}
}
//________________________________________________________________________
double RWMat::FindMinMax()
{
double maxval=0;
double minval=0;
int maxx,maxy,minx,miny;

for (int i=0; i<fNChannels; i++) 
{
	for (int j=0; j<i; j++) 
	{
	double val1=fRWMat[i][j];
	if (val1 > maxval) {maxval=val1; maxx=i; maxy=j;}
	if (val1 < minval) {minval=val1; minx=i; miny=j;}
	}
}
cout << "Max Value = " << maxval << " at " <<maxx << " "<<maxy<<endl;
cout << "Min Value = " << minval << " at " <<minx << " "<<miny<<endl;
return minval;
}
//________________________________________________________________________
int RWMat::FindMinChan()
{
double minval=0;
int minx,miny;

for (int i=0; i<fNChannels; i++) 
{
	for (int j=0; j<i; j++) 
	{
	double val1=fRWMat[i][j];
	if (val1 < minval) {minval=val1; minx=i; miny=j;}
	}
}
if (minx > miny) {miny=minx;}

return miny;
}
