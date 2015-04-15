//EcgAnalyse.cpp
//Analyse ECG data stream to detect R peaks
//
// Version 12.04.2015
//
//*******************************************************************

#include "EcgData.h"

struct SouPeak //Smoothed source peaks
{
	int hS;	//Peak height (or depth), relative to averaged SouBeatSum
	int xS; //
};

#define MAX_SOU_PEAKS 12

static	SouPeak 
			PPos[MAX_SOU_PEAKS],  //Positive smoothed  source peaks
			PNeg[MAX_SOU_PEAKS];  //Negative peaks	

static int NPos=0,NNeg=0;	//Current number of positive and negative peaks

#define MAX_FRAME_SIZE 360

#define MAX_SMOOTH_RANGE 30 /* Max range for smoothing of source signal and derivative*/

static PEAK Peak[MAX_PEAK];	//Detected derivative peaks
static int nPeaks=0;		//Number of active last peaks

static int
		sampleRate=360,		 //Samples per sec
		Counter=0,			 //Total counter of samples processed 
		LastPeakReturned=-1, //Counter for last returned peak (to avoid double return)	
		BeatRange = MAX_FRAME_SIZE; //Used for indirect calculation of final derivative (to filter peaks)

//Circular buffer for source data and derivatives
static int16_t 
		SouData[MAX_SMOOTH_RANGE],	  //Unchanged source data
		SmoothData[MAX_SMOOTH_RANGE], //Smoothed source data, offset from Sou is SouQrsRange/2
		Deriv[MAX_SMOOTH_RANGE],	  //Primary derivative values, offset from Smooth is 3
		FinDeriv[MAX_SMOOTH_RANGE];	  //Final(smoothed) derivative, offset from Deriv is FinQrsRange/2
									  //  used for Peaks detection	

//Total FinDeriv and Peaks offset is (SouQrsRange +  FinQrsRange)/2 + 3
static int FinOffset=0;

//Range width for different values
static	int
		SouQrsRange=5,		//Used for primary smoothing
		SmoothQrsRange=30,	//Used for get primary derivative, wide range for exact positioning of peaks
		DerivQrsRange=20,	//Collection range for primary derivative
		FinQrsRange=20,		//Final derivative range - for peaks detection
		DerivShortSum=0,	//Sum of derivative on QRS range
		SouDataSum=0,		//Sum of source data on QRS range

		SouBeatSum=0,		//Average source signal sum for last beat (calculated indirectly) 
		DerivBeatSum=0;		//Average derivative sum for last beat  

#define FIRST_COUNTER 20   /*Initial counter value over QRS range for correct calculation of derivatives*/

//
//Set new sample rate and initialize all variables
//Returns final offset from Current Sou position (for debug only)
int init_analyzer( int16_t newSampleRate )
{
  int i;	
  sampleRate = newSampleRate;
  BeatRange = sampleRate*2;

  //Initialize counters and average values
  Counter = FIRST_COUNTER; //Set counter initially over QRS range for correct calculation of derivatives
  DerivBeatSum = 0;
  DerivShortSum = 0;
  SouDataSum = 0;
  LastPeakReturned = -1;
  FinOffset = (SouQrsRange +  FinQrsRange)/2 + 3;
  nPeaks=0;
  NPos=0;
  NNeg=0;

  for(i=0; i < MAX_SMOOTH_RANGE ; i++) Deriv[i]=0; 
  for(i=0; i < MAX_SMOOTH_RANGE ; i++) FinDeriv[i]=0; 

  return FinOffset;
}

//
// Takes 16-bit ECG samples (5 uV/LSB) as input and returns the detection delay when a QRS is detected, 
// i.e. it will return how many samples back the beat was detected.
//
int analyzeNextSample( int16_t sample,				//new sample value
					   int *avgSou, int *avgDeriv)	//pointers to save calculated values (DEBUG only!)
{
	int i,j,k,Rx,begRx,smv, 
		absval, 
		AvDerivShort, //Current value of FinDeriv 
		SmoothValue,  //Averaged sou value	
		iThreshold,	  //Threshold to filter peaks
		tFinValue,	  //Height of found peak
		LastPeak,	  //Last found peak position
		AvSmBeat;	  //Smooth signal beat average value
	
	int isNotMax = false; //Flag to skip wrong peaks

	//Fill data ranges initially for correct calculation of average values
	if(Counter == FIRST_COUNTER)
	{
		for(i=0; i < SouQrsRange ; i++) 
			SouData[i] = sample;

		for(i=0; i < SmoothQrsRange ; i++)
			SmoothData[i] = sample;

		SouDataSum = sample * SouQrsRange;
	}

	//Update source sum of QRS range and smoothed data
	SouDataSum += sample - SouData[Counter % SouQrsRange];  
	SouData[Counter % SouQrsRange] = sample;
	SmoothValue = (SouDataSum / SouQrsRange);
	SmoothData[Counter % SmoothQrsRange] = (int16_t)SmoothValue;

	//Calculate primary derivative value (absolute)
	absval = SmoothData[Counter % SmoothQrsRange] - SmoothData[(Counter-6) % SmoothQrsRange];
	if(absval < 0) absval = -absval;

	//Update DerivShortSum - used for average derivative calculation
	DerivShortSum -= Deriv[Counter % DerivQrsRange];	
	DerivShortSum += absval;
	Deriv[Counter % DerivQrsRange] = (int16_t)absval;

	//Final derivative value (averaged) - used for peaks detection
	AvDerivShort = DerivShortSum / DerivQrsRange;

	FinDeriv[Counter % FinQrsRange] = (int16_t)AvDerivShort;

	if(avgDeriv) *avgDeriv = AvDerivShort; //For debug only

	//Indirect beat averaging of source and final data
	SouBeatSum = (int)(SouBeatSum * (BeatRange - 1.0)/BeatRange + SmoothValue); 
	AvSmBeat = SouBeatSum / BeatRange;
	DerivBeatSum = (int)(DerivBeatSum * (BeatRange - 1.0)/BeatRange + AvDerivShort); 

	//Now check for positive and negative peaks of source signal (smoothed)
	k = Counter - 3;
	smv = SmoothData[k % SmoothQrsRange];
	//1st - positive check
	if(smv >= SmoothData[(k+1) % SmoothQrsRange] && smv >= SmoothData[(k-1) % SmoothQrsRange] && 
	   smv > SmoothData[(k+3) % SmoothQrsRange]  &&  smv > SmoothData[(k-3) % SmoothQrsRange] ) 
	{
		if(NPos && k - PPos[NPos-1].xS < sampleRate / 5)  //Min 0.2 sec between peaks -> replace
		{
			if(smv > PPos[NPos-1].hS)
			{
				PPos[NPos-1].xS = k;
				PPos[NPos-1].hS = smv;
			}
		}
		else //Add new 
		{
			PPos[NPos].xS = k;
			PPos[NPos].hS = smv;
			if(++NPos >= MAX_SOU_PEAKS)  //Remove 1st 5 peaks
			{
				for(j=5; j < NPos ; j++)  PPos[j-5] = PPos[j];
				NPos -= 5;
			}
		}
	}
	//2nd - negative check
	if(smv <= SmoothData[(k+1) % SmoothQrsRange] && smv <= SmoothData[(k-1) % SmoothQrsRange] && 
	   smv < SmoothData[(k+3) % SmoothQrsRange]  &&  smv < SmoothData[(k-3) % SmoothQrsRange] ) 
	{
		if(NNeg && k - PNeg[NNeg-1].xS < sampleRate / 5)  //Min 0.2 sec between peaks -> replace
		{
			if(smv < PNeg[NNeg-1].hS)
			{
				PNeg[NNeg-1].xS = k;
				PNeg[NNeg-1].hS = smv;
			}
		}
		else //Add new 
		{
			PNeg[NNeg].xS = k;
			PNeg[NNeg].hS = smv;
			if(++NNeg >= MAX_SOU_PEAKS)  //Remove 1st 5 peaks
			{
				for(j=5; j < NNeg ; j++)  PNeg[j-5] = PNeg[j];
				NNeg -= 5;
			}
		}
	}

	//Threshold to separate final derivative peaks
	iThreshold = (int)(DerivBeatSum * 3.5)/BeatRange;
	if(avgSou) *avgSou = iThreshold;

	j = Counter - 5; //Check FinDeriv values 5 steps ago to find peaks
	tFinValue = FinDeriv[j % FinQrsRange];

	//Check if current point may be a peak
	if( tFinValue > iThreshold && iThreshold > 0 &&
		tFinValue >= FinDeriv[(j-1) % FinQrsRange] && tFinValue >= FinDeriv[(j+1) % FinQrsRange] && 
	    tFinValue >= FinDeriv[(j-2) % FinQrsRange] && tFinValue >= FinDeriv[(j+2) % FinQrsRange] &&
  	    tFinValue >= FinDeriv[(j-3) % FinQrsRange] && tFinValue > FinDeriv[(j+3) % FinQrsRange])
		{
			if(nPeaks) //Compare with previous peak
			{
			  if(Peak[nPeaks-1].xD > j - sampleRate/4) //Join peaks, separated by 0.25 sec or less
			  {
				  if(Peak[nPeaks-1].hD < tFinValue) //If distance to previous peak is small
				  {
					Peak[nPeaks-1].hD = tFinValue;	//Replace previous peak
					Peak[nPeaks-1].xD = j;
					nPeaks--;
				  }
				  else
					  isNotMax = true;		//or simply skip current point
			  }
			  
			  //Skip high T peaks
			  if( j - Peak[nPeaks-1].xD < sampleRate*0.4 &&  
				  tFinValue - iThreshold <= (Peak[nPeaks-1].hD -iThreshold)/7 ) 
				  isNotMax = true;
			} 

			//Skip unclear first peak, may be wrong, normalize levels later!!
			if(Counter < sampleRate && (iThreshold <= 2 && tFinValue < 15 || tFinValue < 8))  //Temporary solution, improve	
				isNotMax = true;
			
			if( !isNotMax && Counter > 50) 
			{
			  if(nPeaks==1) //1st peak may be wrong because of lack of statistics	
				 if( Peak[0].hD - iThreshold < (tFinValue - iThreshold)/6 ) 
					 nPeaks=0;

			  Peak[nPeaks].hD = tFinValue;
				
			  int PosPeak=0, NegPeak=0, PosDist=0, NegDist=0;
			  if(NPos) 
			  {
				  PosDist = j - PPos[NPos-1].xS;
				  if(PosDist < sampleRate / 10)
					PosPeak = PPos[NPos-1].hS - AvSmBeat;
			  }
			  if(NNeg) 
			  {
 				  NegDist = j - PNeg[NNeg-1].xS;
				  if(NegDist < sampleRate / 10)
					 NegPeak = AvSmBeat - PNeg[NNeg-1].hS;
			  }

			  if(NegPeak < PosPeak * 1.2) NegPeak=0;
			  else
				  if(PosPeak < NegPeak*0.8) PosPeak=0;

			  Rx = begRx = j - FinOffset + 2; //Position of corresponding Smooth data 
			  if(PosPeak) Rx = PPos[NPos-1].xS;
			  else
				  if(NegPeak) Rx = PNeg[NNeg-1].xS;

			  //Make exact positioning by source value peaks
			  //while(SmoothData[(Rx+2) % SmoothQrsRange] > SmoothData[Rx % SmoothQrsRange] && Rx < begRx + 15)  
				//  Rx += 2;
			  //while(SmoothData[(Rx-2) % SmoothQrsRange] > SmoothData[Rx % SmoothQrsRange] && Rx > begRx - 15)  
				//  Rx -= 2;

			  //Keep detected peak for furthere analysis (compare with next peaks)
			  Peak[nPeaks].xD = j;
			  Peak[nPeaks].xS = Rx;
			  Peak[nPeaks].hS = SmoothData[Rx % SmoothQrsRange];
			  if(++nPeaks >= MAX_PEAK-2) //Clean old peaks, if array is filled
			  {
  				  nPeaks -= 8;
				  for(k=0; k < nPeaks ; k++) 
				  { 
					  Peak[k] = Peak[k+8];	
				  }
			  }
		    }
		}

	Counter++;

	//Find 1st not sent peak, older, than 0.5 sec 
	for(i=0; i < nPeaks ; i++)
	{
		LastPeak = Peak[i].xS;
		if(LastPeak > Counter - sampleRate/2) break; //Don't send peaks inside 1/2 sec
		if( LastPeak > LastPeakReturned)  //Peak was not sent yet
		{
			LastPeakReturned = LastPeak;
			return Counter - LastPeak;
		}
	}

	return 0;
}

