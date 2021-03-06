
#include <stdio.h>
#include <memory.h>
#include <wfdb/wfdb.h>
#include <wfdb/ecgmap.h>
#include "analyze.h"
#include "EcgData.h"
#include "profiler.h"

#define STRING_MYANNOTATOR_EXT	((char *)"dbr")
extern int glb_AnalyzerGain;


int readAndAnalyze( char *recName )
{
	WFDB_Anninfo annoOut;
	WFDB_Sample *samp;
	WFDB_Siginfo *wfdbSignalInfo;

	/* open annotation file for writing */
	annoOut.name = STRING_MYANNOTATOR_EXT;
	annoOut.stat = WFDB_WRITE;
	if ( annopen( recName, &annoOut, 1 ) < 0 ) {
		fprintf( stderr, "Error 0x01 : opening annotation for writing \n" );
		wfdbquit();
		return 1;
	}


	int channel_count = isigopen( recName, NULL, 0 );	/* find out how many channels there are */
	if ( channel_count <= 0 ) {
		fprintf( stderr, "Error 0x03 : bad channel count : %d\n", channel_count );
		wfdbquit();
		return 3;
	}

	wfdbSignalInfo = ( WFDB_Siginfo * ) malloc( channel_count * sizeof( WFDB_Siginfo ) );
	channel_count = isigopen( recName, wfdbSignalInfo, channel_count );
	int samps_per_chan_per_sec = getifreq();

	samp = ( WFDB_Sample * ) malloc( channel_count * sizeof( WFDB_Sample ) );

	if ( wfdbSignalInfo == NULL || samp == NULL ) {
		fprintf( stderr, "Error 0x02 s: insufficient memory opening %s\n",  recName );
		wfdbquit();
		return ( 2 );
	}

	if ( wfdbSignalInfo->gain == 0 ) {
		wfdbSignalInfo->gain = 200;
	}

	init_analyzer( samps_per_chan_per_sec );

	// TimeyWimey tim("for ( ... )  analyzeNextSample");
	for ( long sampleCount = 0 ; getvec( samp ) > 0 ; sampleCount++ ) {

		int beatFound = analyzeNextSample( samp[0] * wfdbSignalInfo->gain / glb_AnalyzerGain, NULL, NULL );

		if ( beatFound ) {
			WFDB_Annotation annot;
			annot.subtyp = annot.chan = annot.num = 0;
			annot.aux = NULL;

			annot.anntyp = NORMAL;
			annot.time = sampleCount - beatFound;

			putann( 0, &annot );

			// fprintf( stdout, "\nDBG : beat found at pos %d \n", sampleCount - beatFound );
			// fprintf( stdout, " %d ", sampleCount - beatFound );
		}

	}

	wfdbquit();
	return 0;
}


