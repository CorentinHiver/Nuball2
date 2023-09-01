//Function to write out a radware singles spectrum
void WriteRadware(TH1F *TheSpectrum)
{
//cout << "Number of bins = " << TheSpectrum->GetSize() <<endl;
int chans=TheSpectrum->GetSize()-2; //Subtract 2 for the overflow and underflow bins

//Double_t* fSpectrum=TheSpectrum->GetBuffer();
string fname=TheSpectrum->GetName();
//cout << "name = " << fname << endl;
//exit(1);

int len=fname.size();
//string extn=fname.substr(len-4,len);
//cout << extn << endl;
std::string outname=fname.substr(0,len)+".spe";
cout << "Writing spectrum to file: " << outname << endl;

FILE *fprad;
fprad = fopen(outname.c_str(),"w");
int j = 24;
int toto;
int tab[4] = {0,1,1,1};
	 
tab[0] = chans; 
toto = chans * sizeof(float); 

float* buffer=new float[chans];

for (int i=0; i < chans-1; i++) {buffer[i]=float(TheSpectrum->GetBinContent(i+1));}

int dum;	 
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(outname.c_str(), 8, 1, fprad); 
dum = fwrite(tab, sizeof(int), 4, fprad);
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(&toto, sizeof(int), 1, fprad);
dum = fwrite(buffer, sizeof(float), chans, fprad);
dum = fwrite(&toto,sizeof(int),1,fprad);

fclose(fprad);
delete [] buffer;
}

void WriteRadware(TH1D *TheSpectrum)
{
//cout << "Number of bins = " << TheSpectrum->GetSize() <<endl;
int chans=TheSpectrum->GetSize()-2; //Subtract 2 for the overflow and underflow bins

//Double_t* fSpectrum=TheSpectrum->GetBuffer();
string fname=TheSpectrum->GetName();
//cout << "name = " << fname << endl;
//exit(1);

int len=fname.size();
//string extn=fname.substr(len-4,len);
//cout << extn << endl;
string outname=fname.substr(0,len)+".spe";
cout << "Writing spectrum to file: " << outname << endl;

FILE *fprad;
fprad = fopen(outname.c_str(),"w");
int j = 24;
int toto;
int tab[4] = {0,1,1,1};
	 
tab[0] = chans; 
toto = chans * sizeof(float); 

float* buffer=new float[chans];

for (int i=0; i < chans-1; i++) {buffer[i]=float(TheSpectrum->GetBinContent(i+1));}

int dum;	 
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(outname.c_str(), 8, 1, fprad); 
dum = fwrite(tab, sizeof(int), 4, fprad);
dum = fwrite(&j, sizeof(int), 1, fprad);
dum = fwrite(&toto, sizeof(int), 1, fprad);
dum = fwrite(buffer, sizeof(float), chans, fprad);
dum = fwrite(&toto,sizeof(int),1,fprad);

fclose(fprad);
delete [] buffer;
}
