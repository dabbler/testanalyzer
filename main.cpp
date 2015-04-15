
#include <wfdb/wfdb.h>
#include <wfdb/ecgmap.h>

#include "analyze.h"


int main( int argc, char *argv[] )
{
/*
    WFDB_Anninfo an[2];
    char record[8], iann[10], oann[10];
    WFDB_Annotation annot;

    an[0].name = iann; an[0].stat = WFDB_READ;
    an[1].name = oann; an[1].stat = WFDB_WRITE;
    if (annopen(record, an, 2) < 0) exit(1);
	while (getann(0, &annot) == 0) {
		if (isqrs(annot.anntyp)) {
			annot.anntyp = NORMAL;
			if (putann(0, &annot) < 0) break;
		}
	}
*/

	if ( argc > 2 ) {
		setwfdb( argv[2] );
	}

	if ( argc > 1 ) {
		char *recName = argv[1];

		readAndAnalyze( recName );

		wfdbquit();
	}
}


