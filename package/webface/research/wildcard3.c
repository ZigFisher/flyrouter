/*
*
*   Copyright (c) 2003 Regents of The University of Michigan.
*   All Rights Reserved.  See COPYRIGHT.
*
*
*
Copyright (c) 2003 Regents of The University of Michigan.
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of The University
of Michigan not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission. This software is supplied as is without expressed or
implied warranties of any kind.

Research Systems Unix Group
The University of Michigan
c/o Wesley Craig
4251 Plymouth Road B1F2, #2600
Ann Arbor, MI 48105-2785
radmind@umich.edu
http://rsug.itd.umich.edu/software/radmind/

*   */

#include "config.h"

#include <stdlib.h>
#include <ctype.h>

#include "wildcard.h"

    int
wildcard( char *wild, char *p, int sensitive )
{
    int		min, max;
    int		i;

    for (;;) {
	switch ( *wild ) {
	case '*' :
	    wild++;

	    if ( *wild == '\0' ) {
		return( 1 );
	    }
	    for ( i = 0; p[ i ] != '\0'; i++ ) {
		if ( wildcard( wild, &p[ i ], sensitive )) {
		    return( 1 );
		}
	    }
	    return( 0 );

	case '<' :
	    wild++;

	    if ( ! isdigit( (int)*p )) {
		return( 0 );
	    }
	    i = atoi( p );
	    while ( isdigit( (int)*p )) p++;

	    if ( ! isdigit( (int)*wild )) {
		return( 0 );
	    }
	    min = atoi( wild );
	    while ( isdigit( (int)*wild )) wild++;

	    if ( *wild++ != '-' ) {
		return( 0 );
	    }

	    if ( ! isdigit( (int)*wild )) {
		return( 0 );
	    }
	    max = atoi( wild );
	    while ( isdigit( (int)*wild )) wild++;

	    if ( *wild++ != '>' ) {
		return( 0 );
	    }

	    if (( i < min ) || ( i > max )) {
		return( 0 );
	    }
	    break;

	case '\\' :
	    wild++;
	default :
	    if ( sensitive ) {
	       if ( *wild != *p ) {
		   return( 0 );
	       }
	    } else {
	       if ( tolower(*wild) != tolower(*p) ) {
		  return( 0 );
		}
	    }
	    if ( *wild == '\0' ) {
		return( 1 );
	    }
	    wild++, p++;
	}
    }
}
