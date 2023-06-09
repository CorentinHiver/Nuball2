#ifndef _RWMat_
#define _RWMat_

#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "TH2F.h"

#include "../libCo.hpp"

class RWMat
{
 public:
	RWMat(std::string name="test.m4b",int nchans=4096);	//@- Default constructor
	RWMat(TH2F* RootMat);	//@- constructor from Root 2D histogram
  template<class T>
	RWMat(MTTHist<T> & MTRootMat);	//@- constructor from MTThist 2D histogram
	~RWMat(); 			//@- Normal destructor
  template<class THist>
  void Reset(THist* RootMat);
	void Write(std::string name="", std::string path = "./"); //write the RWMat out
	void Read(std::string Filename="", bool IsInteger=true); //Read matrix from file
	double Get(unsigned short i, unsigned short j) {return fRWMat[i][j];}
	void Set(unsigned short i, unsigned short j, int val) {fRWMat[i][j]=val;}
	void Set(unsigned short i, unsigned short j, double val) {fRWMat[i][j]=val;}
	void Fill(unsigned short i, unsigned short j); //increment channel by 1 count
	RWMat* Add(RWMat* Matrix,double val);
	double Integral();
	void ReSymmetrise();
	double FindMinMax();
	int FindMinChan();

 protected:
	unsigned short 	fNChannels;
	std::string 		fName;
	int**  fRWMat;
	double		fTotalCounts;
	double		fMaxCounts;
};

RWMat::RWMat(std::string name, int nchans) //default constructor
{
  fNChannels=nchans;
  fName=name;
  fRWMat=new int*[fNChannels];
  for(int i=0 ; i < fNChannels ; i++) fRWMat[i] = new int[fNChannels];
}

template<class T>
RWMat::RWMat(MTTHist<T> & MTRootMat)
{
  MTRootMat.Merge();
  if (!MTRootMat -> InheritsFrom("TH2")) print(MTRootMat.GetName(),"is not a TH2x !!");
  else Reset(MTRootMat.get());
}

RWMat::RWMat(TH2F* RootMat) //constructor from root object
{
   Reset(RootMat);
}

template<class THist>
void RWMat::Reset(THist* RootMat)
{
  if (RootMat->Integral() < 0)
  {
    print(RootMat->GetName(), "empty !!");
    return;
  }
  if (!RootMat -> InheritsFrom("TH2")) {print(RootMat->GetName(),"is not a TH2x !!"); return;}
  else
  {
    int xchans=RootMat->GetNbinsX();
    int ychans=RootMat->GetNbinsY();
    fNChannels=4096;
    fName=RootMat->GetName();
    fName=fName+".m4b";
    print(fName);
    fRWMat=new int*[fNChannels];
    for(int i=0 ; i < fNChannels ; i++) fRWMat[i] = new int[fNChannels];
    double val=0;
    for (int i=0; i < xchans ; i++)
    {
      for (int j=0; j < ychans ; j++)
      {
        fRWMat[i][j]=RootMat->GetBinContent(i,j);
        val+=fRWMat[i][j];
      }
    }
  }
}
//________________________________________________________________________
RWMat::~RWMat()
{
  for (int i=0; i < fNChannels; i++) delete [] fRWMat[i];
  delete [] fRWMat;
}
//________________________________________________________________________
void RWMat::Fill(unsigned short i, unsigned short j)
{
  if ((i < fNChannels) && (j < fNChannels)){fRWMat[i][j]++;}
}
//________________________________________________________________________
void RWMat::Read(std::string fname, bool IsInteger)
{
  fName=fname;
  FILE *fprad;

  fprad = fopen(fname.c_str(),"r");
  if (fprad) {std::cout << "Reading RWMat : " << fname << std::endl;}
  else {std::cout << "Error Reading RWMat : " << fname << std::endl; exit(1);}

  int size=fNChannels;

  double* buffer=new  double[size];
  int* bufferi=new  int[size];

  std::cout << "Number of channels = " << fNChannels <<  std::endl;
  for (int i=0; i<size; i++)
  {
		fread(bufferi, size, ( (IsInteger) ? sizeof(int) : sizeof(double) ), fprad);
		for (int j=0; j<size; j++) fRWMat[i][j]=bufferi[j];
  }
  fclose(fprad);
  delete [] buffer;
  delete [] bufferi;
}
  //________________________________________________________________________
void RWMat::Write(std::string name, std::string path)
{
  FILE *fprad;
  if (name=="") {name=fName;}
  else {fName=name;}
  if (path.back() != '/') path.push_back('/');
  name = path+name;
  fprad = fopen(name.c_str(),"w");
  if (fprad) {std::cout << "Writing RWMat : " << name << std::endl;}
  else {std::cout << "Error Writing RWMat : " << name << std::endl; exit(1);}

  int size=fNChannels;

  int* buffer=new  int[size];

  std::cout << "channels = " << fNChannels << " counts = " << this->Integral() << " size = " << size*size*sizeof(int)/1048576 << "Mo" << std::endl;

  for (int i=0; i<size; i++)
  {
    for (int j=0; j<size; j++)
    {
      buffer[j]=fRWMat[i][j];
    }
    fwrite(buffer, size, sizeof(int), fprad);
  }

  fclose(fprad);
  delete [] buffer;
}
//________________________________________________________________________
RWMat* RWMat::Add(RWMat* Matrix,double val)
{
  RWMat *mat3=new RWMat();
  for (int i=0; i<fNChannels; i++)
    for (int j=0; j<fNChannels; j++)
      mat3->Set(i,j,(fRWMat[i][j]+(Matrix->Get(i,j)*val)));
  return mat3;
}
//________________________________________________________________________
double RWMat::Integral()
{
  double val=0.;
  for (int i=0; i<fNChannels; i++) for (int j=0; j<fNChannels; j++) val+=fRWMat[i][j];
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
  int maxx = 0, maxy = 0, minx = 0, miny = 0;

  for (int i=0; i<fNChannels; i++)
  {
  	for (int j=0; j<i; j++)
  	{
    	double val1=fRWMat[i][j];
    	if (val1 > maxval) {maxval=val1; maxx=i; maxy=j;}
    	if (val1 < minval) {minval=val1; minx=i; miny=j;}
  	}
  }
  std::cout << "Max Value = " << maxval << " at " <<maxx << " "<<maxy<<std::endl;
  std::cout << "Min Value = " << minval << " at " <<minx << " "<<miny<<std::endl;
  return minval;
}
//________________________________________________________________________
int RWMat::FindMinChan()
{
  double minval=0;
  int minx = 0, miny = 0;

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
#endif
