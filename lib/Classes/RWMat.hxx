#ifndef _RWMat_
#define _RWMat_ 

#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include "TH2F.h"

using namespace std;

class RWMat
{
 public:
	RWMat(string name="test.m4b",int nchans=4096);	//@- Default constructor
	RWMat(TH2F* RootMat);	//@- constructor from Root 2D histogram
	~RWMat(); 			//@- Normal destructor
	void Write(string name=""); //write the RWMat out
	void Read(string Filename="", bool IsInteger=true); //Read matrix from file
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
	string 		fName;
	int**  fRWMat;
	double		fTotalCounts;
	double		fMaxCounts;
};
#endif
