
#include "profiler.h"
#include <string.h>
#include <malloc.h>


TimeyWimey::TimeyWimey()
{
	strLabel = NULL;
	clock_gettime( CLOCK_REALTIME, &beg_ );
}


TimeyWimey::TimeyWimey( const char *label )
{
	strLabel = (char *) malloc( strlen(label) + 1 );
	strcpy( strLabel, label );

	clock_gettime( CLOCK_REALTIME, &beg_ );
}


TimeyWimey::~TimeyWimey()
{
	if ( strLabel ) {
		printf("%s : %.3f ms\n", strLabel, elapsed() * 1000.0 );
	}
}


double TimeyWimey::elapsed()
{
	clock_gettime( CLOCK_REALTIME, &end_ );
	return end_.tv_sec - beg_.tv_sec + ( end_.tv_nsec - beg_.tv_nsec ) / 1000000000.;
}


void TimeyWimey::reset()
{
	clock_gettime( CLOCK_REALTIME, &beg_ );
}

