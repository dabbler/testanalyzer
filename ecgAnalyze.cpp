
#include <stdlib.h>
#include "ecgAnalyze.h"

int analyzeNextSample( int samp )
{
	if ( (rand() % 10000) == 0 ) {
		return (rand() % 100) + (rand() % 100);
	}
	return 0;
}


int init_analyzer( int sampleRate )
{
	return 0;
}

