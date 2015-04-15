
#ifndef int16_t
#define int16_t short
#endif

#define MAX_PEAK 20

struct PEAK {
 int xD; 	//Derivative Peak center point
 int hD;	//Derivative Peak height
 int xS;    //Source data peak 
 int hS;	//Source peak height
};

int init_analyzer( int16_t newSampleRate);
int analyzeNextSample( int16_t sample, int *avgBeat, int *avgQRS);