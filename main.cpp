
#include <stdlib.h>
#include "analyze.h"

int glb_AnalyzerGain = 200;


int main( int argc, char *argv[] )
{

	if ( argc > 2 ) {
		glb_AnalyzerGain = atoi( argv[2] );
	}

	if ( argc > 1 ) {
		char *recName = argv[1];

		readAndAnalyze( recName );
	}
}


