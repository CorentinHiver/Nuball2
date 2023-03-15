#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include "TImage.h"
#include "TCanvas.h"
#include "TArrayD.h"
#include "TROOT.h"
#include "TColor.h"
#include "TAttImage.h"
#include "TEnv.h"
#include "TH2D.h"
#include "TF2.h"
#include "TColor.h"
#include "TLine.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TApplication.h"
#include "TStopwatch.h"
#include "TFile.h"
#include "TTree.h"
#include "Spec.hxx"
#include "Spec.cxx"
#include "RWMat.hxx"
#include "RWMat.cxx"
#include "WriteRWSpec.cxx"
#include "idmap_232Th.hxx"

inline bool FileExists (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
// ALL THE FOLLOWING ARE NEEDED
// THE INPUT FILE
string FileList="runfiles_238ub.dat";
string InputDirectory="/srv/data/nuball/N-SI-103r/final/For_test/";
string OutputDirectory="/srv/data/nuball/N-SI-103r/JWdata3/";

int EVENT_TIME_WINDOW=700; //The event window in ns

int NCHANS=2000;
int NDETS=255;
int NRUNS=170;
int ReadFits=true; //Yes we want new correction of the time stamps
double TimeOffsets[256];

// MAKE LOOKUP TABLES OF ALL THESE
int *ring=new int[NDETS];
int *alveole=new int[NDETS];
int *module=new int[NDETS];
bool *isthere=new bool[NDETS];
bool *isge=new bool[NDETS];
bool *isbgo=new bool[NDETS];
bool *isphase1ge=new bool[NDETS];
bool *isphase1bgo=new bool[NDETS];
bool *isphase1=new bool[NDETS];
bool *isclover=new bool[NDETS];
bool *iscloverge=new bool[NDETS];
bool *iscloverbgo=new bool[NDETS];
bool *islabr3=new bool[NDETS];
bool *ismadrid=new bool[NDETS];
bool *iseden=new bool[NDETS];
bool *ispulse=new bool[NDETS];
bool *bigwalk=new bool[NDETS];
bool *isbad=new bool[NDETS];
bool *isused=new bool[NDETS];
int *isring1=new int[NDETS];
int *isring2=new int[NDETS];
int *isring3=new int[NDETS];
int *isring4=new int[NDETS];

// The gains correction array
double *gcor=new double[NDETS];//The gain correction arrays
double *ccor=new double[NDETS];

/////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
srand48(0);
// !!!!!!!!!!!!!!!!! EXTRACT RUN NUMBERS FROM FILE NAME !!!!!!!!!!!!!!!!!!!!!!
// Make 2d time shift vs run number spectrum like Nigel for each file

#include "SpecDefs.cxx"
#include "Lookup.cxx"


///////////////////////////////////////////////////////////////////// FILE HANDLING ////////////

// Make file names. Find run number

ifstream iflist(FileList.c_str());
string line;
// Read through
ULong64_t beampulse=0; //We want to keep the beam pulse from the previous file
UInt_t beamperiod=400000; //We want to keep the beam period from the previous file
ULong64_t StartTimeStamp=0;
ULong64_t PreviousTimeStamp=0;
int nfiles=0;
int RunNumber;
int FileNumber;
string OutputSpectraFile;
TFile *outputspectrafile;
while(iflist >> line)
{
nfiles++;
RunNumber=atoi(line.substr(3,3).c_str());
FileNumber=atoi(line.substr(line.size()-11,4).c_str());
cout << "Run " << RunNumber << " File " << FileNumber << endl;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! START TO PROCESS THIS FILE !!!!!!!!!!!!!!!!
TStopwatch timer;
timer.Reset();
timer.Start();

string InputFile=InputDirectory+line;
cout << "Reading " << InputFile << endl;
// KEEP IT ONE OUTPUT FILE FOR ONE INPUT FILE. ALLOWS US TO DO CONVERSION IN MIDDLE
// AND COPE WITH
string OutputFile=OutputDirectory+Form("Evts%d_%d.root",RunNumber,FileNumber);

string TimeShiftsFile=Form("/home/faster/codes_jon2/tpeaks/tpeaks%d.dat",RunNumber);
if (ReadFits) //Read in the new shifts file
	{
	ifstream f(TimeShiftsFile.c_str()); //Check if the drifts file exists or not
	if (f.good())
		{
		cout << "Reading Shifts file " << TimeShiftsFile << endl;
		int sid;
		double tshift;
		string line;
		while (f >> sid >> tshift)
			{
			TimeOffsets[sid]=(40.0-tshift);
			}
		f.close();
		}
	else {cout << "Cannot find tpeaks file " << TimeShiftsFile << endl;}
	//ReadFits=false;
	}


//////////////////////////////////////// THE INPUT/OUTPUT TREES ////////////////////////////////
ULong64_t tab_tm;
UChar_t tab_label;
Double_t tab_nrj;
UInt_t tab_nrj2;
Bool_t tab_pileup;

//DEFINE OUTPUT TREE
// New tree in which to write the sorted data
UInt_t size;
Double_t energies[256];
Double_t timestamps[256];
UInt_t ids[256]; //ids array
Bool_t pups[256]; //pileups array

//DEFINE INPUT TREE

//Does output file already exist. If yes then skip to the next entry
//ifstream f2(OutputFile.c_str());
//if (f2.good()) {f2.close(); cout << "Already exists. Skipping to next file"<<endl; continue;}
if (FileExists(OutputFile.c_str())) {continue;} // skip this file

TFile *fin = new TFile(InputFile.c_str(),"READ");
if (fin <= 0) {cout << "Can't open input file" <<endl; exit(1);}
//exit(1);

TTree* inputtree = (TTree*)fin->Get("DataTree");;
inputtree->SetBranchAddress("label",&tab_label);
inputtree->SetBranchAddress("nrj",&tab_nrj);
inputtree->SetBranchAddress("nrj2",&tab_nrj2);
inputtree->SetBranchAddress("time",&tab_tm);
inputtree->SetBranchAddress("pileup",&tab_pileup);

TFile* root_file=new TFile (OutputFile.c_str(), "recreate"); //Otherwise create a new one
TTree* outputtree;
outputtree = new TTree ("DataTree", "outputtree");
outputtree->Branch("size",&size,"size/i"); //
outputtree->Branch("ids",ids,"ids[size]/i"); //
outputtree->Branch("energies",energies,"energies[size]/D"); //
outputtree->Branch("timestamps",timestamps,"timestamps[size]/D"); //
outputtree->Branch("pups",pups,"pups[size]/O"); //

int entries=inputtree->GetEntries();
cout << "There are " << entries << " entries" << endl;
inputtree->GetEntry(0);
ULong64_t FirstTimeStamp=tab_tm;
inputtree->GetEntry(entries-1);
ULong64_t LastTimeStamp=tab_tm;
double nsecs=(LastTimeStamp-FirstTimeStamp)/1e12;
cout << "Amount of data (seconds) = " << nsecs << endl;
cout << "Hits in nuball per second = " << entries/nsecs << endl;


Double_t tdif=0; //timestamp difference to the beam pulse

int TriggerCount=0; //Number of triggers
int TriggerL2=0;
int TriggerL2C1=0;
int TriggerC0=0;
int TriggerC1=0;
int TriggerC2=0;
int TriggerC3=0;
int TriggerM3=0;
int TriggerM4=0;
int TriggerM5=0;
int TriggerM6=0;
int TriggerCount2=0; //Number of event writes
Double_t DeltaTS=0; //This is the variable for the NEW time stamps
int RawMult=0; //Count the raw multiplicity in an event
bool PromptTrigger=false; //Flag to say if the prompt trigger passed
bool PromptTriggerCalculated=false;
bool EndEvent=false; //Flag to say the event has ended
bool EndOfPrompt=false; //Flag to say the prompt part of the event has ended
double TotalEnergy=0;
double BGOEnergy=0;
double GeEnergy=0;
double LaEnergy=0;
int LaMult=0; // LaBr3 Mult
int TotalMult=0;
int PromptMult=0;
int PromptGeMult=0;
int DelayedMult=0;
int DelayedGeMult=0;
//Used for Compton suppression and addback in clovers
int* spat=new int[35]; //BGO hit pattern in the event
int* gpat=new int[35]; //Ge clover mult pattern in the event
double* Esum=new double[35]; //Ge module energies
double* Tsum=new double[35]; //Variable to make an "average" clover timestamp
double* penergy=new double[35]; //Array of prompt Ge module energies
double* ptime=new double[35]; //Array of prompt Ge module energies
double* pid=new double[35]; //Array of prompt Ge module energies

// PROCESS ONE FILE AT A TIME AND CORRECT TIME STAMPS FOR THE WHOLE FILE
int PutPeakAt=340; //Prompt peak at 40 ns
if (RunNumber <= 40) {PutPeakAt=PutPeakAt-147;}
if ((RunNumber > 40) && (RunNumber <= 84)) {PutPeakAt=PutPeakAt-343;}
if ((RunNumber > 84) && (RunNumber <= 89)) {PutPeakAt=PutPeakAt-179;}
//if ((RunNumber > 90) && (RunNumber <= 112)) {PutPeakAt=PutPeakAt;} //do nothing
if ((RunNumber > 113) && (RunNumber <= 131)) {PutPeakAt=PutPeakAt-226;}
if ((RunNumber > 132) && (RunNumber <= 147)) {PutPeakAt=PutPeakAt-302;}
if ((RunNumber > 148) && (RunNumber <= 167)) {PutPeakAt=PutPeakAt-120;}

//if (RunNumber > 84) {cout << "Brutal Stop" << endl; exit(1);}




        cout << "Building the interesting events" << endl;
for(ULong64_t i = 0; i < entries; i++)
	{
	inputtree->GetEntry(i);
        //Double_t r=drand48()-0.5;
	Double_t nrj=tab_nrj; // THE ENERGY
	Double_t nrj2=tab_nrj2; // THE ENERGY
	Int_t detid=tab_label; // THE DETECTOR ID
	Bool_t pileup=tab_pileup; // THE PILEUP FLAG
	ULong64_t timestamp=tab_tm+(PutPeakAt*1000);//Apply the offset in ps. 132 to put peak chanel 200.
	if (ReadFits) {timestamp+=int(TimeOffsets[detid]*1000);}
	if (detid==251) {beampulse=tab_tm; beamperiod=399998;}//No offset applied to beam pulse
	Long64_t TSBeamDifference=timestamp-beampulse;
	Double_t tdifps=((TSBeamDifference) % beamperiod);
	tdif=tdifps/1000.0; //convert to ns

	Double_t cnrj=nrj;
	//especs[detid]->Fill(cnrj);
	//tspecs[detid]->Fill(tdif);


	bool good=!isbad[detid]; //Detector is not on the blacklist. Otherwise ignore this hit

if (good)
{
        bool laprompt=false;
// !!!!!!!!!! THE UNIVERSAL TIME GATES ARE HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //if ((islabr3[detid]) && (tdif >= 25) && (tdif <= 40)) {laprompt=true;}
        if ((islabr3[detid]) && (tdif >= 325) && (tdif <= 375)) {laprompt=true;}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//StartTimeStamp is the first timestamp of the prompt gammas minus its tdif from beampulse to get event "zero".
	if (RawMult==0) {StartTimeStamp=(timestamp-tdifps);} //Get the "zero" value for this potential event
        DeltaTS=(timestamp-StartTimeStamp)/1000.0;


	// Is it a LaBr3? ADD THESE IN FIRST, NO MATTER WHAT.
	if ((islabr3[detid]) && (DeltaTS < EVENT_TIME_WINDOW))
		{
		if (!((laprompt) && (DeltaTS >=700))) //If it's not a subsequent prompt burst
			{
			//Add it to the event. LaBr3's are passengers
			energies[RawMult]=cnrj;
			timestamps[RawMult]=DeltaTS;
			ids[RawMult]=detid;
			pups[RawMult]=pileup;
			RawMult++;
			}
		//Add to the multiplicity and sumE
	        if ((laprompt) && (DeltaTS < 700)) {LaMult++; LaEnergy+=cnrj; PromptMult++;}
		}

        if ((DeltaTS < 700) && (!islabr3[detid]))
	// Confine prompt to first pulse only here for BGO and Ge. LaBr3 we want to add no matter what.
		{
		energies[RawMult]=cnrj;
		timestamps[RawMult]=DeltaTS;
		ids[RawMult]=detid;
		pups[RawMult]=pileup;
		RawMult++;
		PromptMult++;
                if (isge[detid]) {PromptGeMult++;}
		}
	//Different time windows for different detector types.
	//Germaniums decide end of prompt for everyone else.
        if (DeltaTS >= 700) {EndOfPrompt=true;} //Force the end of prompt part, no matter what

	//Delayed gammas now. Go calculate the prompt mult
        if ((EndOfPrompt) && !PromptTriggerCalculated)
	//At least 2 Ge before we even bother with trigger logic
        	{
		//End of Prompt gammas. Did event pass prompt trigger condition ok?
		//Clear the suppression array. Suppression and addback in the ge modules
		int CMult=0;
		int MMult=0;
		if (PromptGeMult >=2) //Don't bother to do the more advanced test if minimum test is not met
			{
			for (int k=1; k <= 34; k++) {spat[k]=0; gpat[k]=0;}
			for (int j=0; j < PromptMult; j++)
				{
				int mnum=module[ids[j]];
				if ((isge[ids[j]]) && (energies[j] < 10000)) {gpat[mnum]++; GeEnergy+=energies[j];}
				if ((isbgo[ids[j]]) && (energies[j] < 10000)) {spat[mnum]++; BGOEnergy+=energies[j];}
                		}
			for(int k = 1; k <= 34; k++) //loop through the Ge modules
				{
				if (spat[k] || gpat[k]) {MMult++;} //Either Ge or BGO or both
				// Caclulate the clean Ge multiplicity here. Compton Suppression pattern in spat[]
				if ((spat[k]==0) && (gpat[k] >= 1)) {CMult++;}
				} //end for k loop
			TotalMult=MMult+LaMult;//LaBr3 mult + module mult. Reduced Multiplicity
                	TotalEnergy=BGOEnergy+GeEnergy+LaEnergy;
// !!!!!!!!!!!!!!!!! PROMPT MULTIPLICITIES ARE NOW KNOWN HERE. WE CAN START TO TRIGGER !!!!!!!!!!
			}
		// *THE* PROMPT TRIGGER CONDITION.
		if (((CMult >= 2) || (LaMult >=2)) && (TotalMult >=3)) //Clean Ge module multiplicity of 2 or more
			{
			PromptTrigger=true;
			TriggerCount++;
			if (LaMult >= 2) {TriggerL2++;}
			if ((LaMult >= 2) && (CMult >=1)) {TriggerL2C1++;}
			if (CMult == 0) {TriggerC0++;}
			if (CMult == 1) {TriggerC1++;}
			if (CMult == 2) {TriggerC2++;}
			if (CMult >= 3) {TriggerC3++;}
			if (TotalMult >= 3) {TriggerM3++;}
			if (TotalMult >= 4) {TriggerM4++;}
			if (TotalMult >= 5) {TriggerM5++;}
			if (TotalMult >= 6) {TriggerM6++;}
			}
		else {EndEvent=true; PromptTrigger=false;} //Look for next event. Forced end.
		PromptTriggerCalculated=true;
		} //We're done finding the prompt Multiplicity. We do this part only ONCE.

// !!!!!!!!!!!!!!!!!!!!!!!!!!! THE LENGTH OF THE DELAYED PART DEFINED HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!
        if (DeltaTS >= EVENT_TIME_WINDOW) {EndEvent=true;} //Force event to end no matter what.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// DELAYED GAMMAS PART. Collect more delayed gammas ONLY if there is PromptTrigger
	//if ((PromptTrigger) && (!EndEvent) && (!islabr3[detid])) //Only add late Ge and BGO here
		//{
	        //if (tdif > 100) //Make sure it isn't in one of the subsequent prompt bursts
			//{
			//energies[RawMult]=cnrj;
			//timestamps[RawMult]=DeltaTS;
			//ids[RawMult]=detid;
			//pups[RawMult]=pileup;
			//RawMult++;
			//DelayedMult++;
			//if (isge[detid]) {DelayedGeMult++;}
			//}
		//}

	//Either the prompt trigger failed, or the next timestamp is out of the time window range
	if (EndEvent)
		{
		if (PromptTrigger) //THEN WRITE OUT THE EVENT
			{
			//CREATE two extra hits in the event for the measured sum energy and multiplicity
			//cout << TriggerCount << " "<<RawMult<<" "<<StartTimeStamp<<" : ";
			//for (int l=0; l < RawMult; l++) {cout << int(timestamps[l]) << " ";}
			//cout << endl;

			//timestamps[RawMult]=StartTimeStamp; //Timestamp is set to the start of the event
			//ids[RawMult]=253; //Label 253 is prompt sum energy
			//energies[RawMult]=TotalEnergy;
			//pups[RawMult]=false;
			//RawMult++;

			//timestamps[RawMult]=0; //Timestamp is set to the start of the event
			//ids[RawMult]=254; //Label 254 is prompt multiplicity
			//energies[RawMult]=TotalMult;
			//pups[RawMult]=false;
			//RawMult++;
			// WRITE THE EVENT OUT HERE TO THE NEW TREE
			size=RawMult;
			if (size < 256) {outputtree->Fill();}
			else {cout << "WARNING, very long event!";}
			}
		//Clear everything
		RawMult=0;
		PromptMult=0;
                PromptGeMult=0;
		DelayedMult=0;
		DelayedGeMult=0;
		PromptTrigger=false;
		EndEvent=false;
		EndOfPrompt=false;
		PromptTriggerCalculated=false;
		TotalEnergy=0;
		BGOEnergy=0;
		GeEnergy=0;
		LaEnergy=0;
		LaMult=0;
		TotalMult=0;
		// TRY AND START A NEW EVENT WITH THIS ENERGY. New StartTimeStamp
		StartTimeStamp=(timestamp-tdifps); //Get the "zero" value for this event
                DeltaTS=(timestamp-StartTimeStamp)/1000.0;//Caclulate the "new" DeltaTS for this hit

                //Use the new hit that ended the old event to try and start a new event
		if (DeltaTS < 700)//Add him to the beginning of the new event. No matter what type
			{
			energies[RawMult]=cnrj;
			timestamps[RawMult]=DeltaTS;
			ids[RawMult]=detid;
			pups[RawMult]=pileup;
			RawMult++;
                        if (isge[detid]) {PromptGeMult++; PromptMult++;}
			if (islabr3[detid] && laprompt) {LaMult++; LaEnergy+=cnrj; PromptMult++;}
                        if (isbgo[detid]) {PromptMult++;}
			}
		}
} //End of test for good detector

        } //End of loop to read the Tree entries

fin->Close(); //Close input tree file
timer.Stop();
Double_t rtime2 = timer.RealTime();
Double_t ctime2 = timer.CpuTime();
cout << "# RealTime=" << rtime2 << " seconds, CpuTime="<< ctime2 << " seconds" <<endl;
cout << "Total Triggers = " << TriggerCount << endl;
cout << "Trigger Rate = " << TriggerCount/nsecs << endl;
cout << "Percent L2 = " << 100.0*TriggerL2/TriggerCount <<endl;
cout << "Percent L2C1 = " << 100.0*TriggerL2C1/TriggerCount <<endl;
cout << "Percent C0 = " << 100.0*TriggerC0/TriggerCount <<endl;
cout << "Percent C1 = " << 100.0*TriggerC1/TriggerCount <<endl;
cout << "Percent C2 = " << 100.0*TriggerC2/TriggerCount <<endl;
cout << "Percent C3 = " << 100.0*TriggerC3/TriggerCount <<endl;
cout << "Percent M3 = " << 100.0*TriggerM3/TriggerCount <<endl;
cout << "Percent M4 = " << 100.0*TriggerM4/TriggerCount <<endl;
cout << "Percent M5 = " << 100.0*TriggerM5/TriggerCount <<endl;
cout << "Percent M6 = " << 100.0*TriggerM6/TriggerCount <<endl;


root_file->Write();
root_file->Close();
}//end of loop for getting the next file to process
OutputSpectraFile=OutputDirectory+Form("Spcs%d.root",RunNumber);
	cout << "Writing output spectra "<< OutputSpectraFile << endl;
	outputspectrafile = new TFile(OutputSpectraFile.c_str(),"RECREATE");
	for (int i=1; i < NDETS; i++) {especs[i]->Write();}
	//for (int i=1; i < NDETS; i++) {pspecs[i]->Write();}
	//for (int i=1; i < NDETS; i++) {nspecs[i]->Write();}
	for (int i=1; i < NDETS; i++) {tspecs[i]->Write();}

cout << "Finished" << endl;
	outputspectrafile->cd();
	outputspectrafile->Close();

}


/*


g++ -g -o BuildEvents3 BuildEvents3.cxx ` root-config --cflags` `root-config --glibs`


*/
