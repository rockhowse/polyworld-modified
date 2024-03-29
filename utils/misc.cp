/********************************************************************/
/* PolyWorld:  An Artificial Life Ecological Simulator              */
/* by Larry Yaeger                                                  */
/* Copyright Apple Computer 1990,1991,1992                          */
/********************************************************************/

// misc.cp: miscellaneous useful short procedures

// Self
#include "misc.h"

// System

#ifdef linux 
    // do nothing
#elif WIN32
    // do nothing
#else 
    // how does one say #ifdef MAC_OSX ? 
    #include <mach/mach_time.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
    #include <errno.h>
#else
    #include <sys/errno.h>
    #include <sys/resource.h>
#endif
#include <sys/time.h>

char* concat(const char* s1, const char* s2)
{
    char* s = new char[strlen(s1)+strlen(s2)+1];
    strcpy(s,s1);
    strcat(s,s2);
    return s;
}


char* concat(const char* s1, const char* s2, const char* s3)
{
    char* s = new char[strlen(s1)+strlen(s2)+strlen(s3)+1];
    strcpy(s,s1);
    strcat(s,s2);
    strcat(s,s3);
    return s;
}


char* concat(const char* s1, const char* s2, const char* s3, const char* s4)
{
    char* s = new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1];
    strcpy(s,s1);
    strcat(s,s2);
    strcat(s,s3);
    strcat(s,s4);
    return s;
}


char* concat(const char* s1, const char* s2, const char* s3, const char* s4, const char* s5)
{
    char* s= new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+strlen(s5)+1];
    strcpy(s,s1);
    strcat(s,s2);
    strcat(s,s3);
    strcat(s,s4);
    strcat(s,s5);
    return s;
}

char* concat(const char* s1, const char* s2, const char* s3, const char* s4, const char* s5, const char* s6)
{
    char* s= new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+strlen(s5)+
                      strlen(s6)+1];
    strcpy(s,s1);
    strcat(s,s2);
    strcat(s,s3);
    strcat(s,s4);
    strcat(s,s5);
    strcat(s,s6);
    return s;
}


char* concat(const char* s1, const char* s2, const char* s3, const char* s4, const char* s5, const char* s6, const char* s7)
{
    char* s= new char[strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+strlen(s5)+
                      strlen(s6)+strlen(s7)+1];
    strcpy(s,s1);
    strcat(s,s2);
    strcat(s,s3);
    strcat(s,s4);
    strcat(s,s5);
    strcat(s,s6);
    strcat(s,s7);
    return s;
}


char* itoa(long i)
{
    char* b = new char[256];
    sprintf(b,"%ld", i);
    char* a = new char[strlen(b)+1];
    strcpy(a, b);
    delete b;
    return a;
}


char* ftoa(float f)
{
    char* b = new char[256];
    sprintf(b,"%g", f);
    char* a = new char[strlen(b)+1];
    strcpy(a ,b);
    delete b;
    return a;
}


float logistic(float x, float slope)
{
    return (1.0 / (1.0 + exp(-1 * x * slope)));
}

float biasedLogistic(float x, float bias, float slope)
{
    return (1.0 / (1.0 + exp(-1 * (x+bias) * slope)));
}


float gaussian( float x, float mean, float variance )
{
    return( exp( -(x-mean)*(x-mean) / variance ) );
}


int SetMaximumFiles( long filecount )
{
    #ifdef linux
    struct rlimit lim;
	
	lim.rlim_cur = lim.rlim_max = (rlim_t) filecount;
	if( setrlimit( RLIMIT_NOFILE, &lim ) == 0 )
		return 0;
	else
		return errno;
    #else
    return 0;
    #endif
}

int GetMaximumFiles( long *filecount )
{
	#ifdef linux
	struct rlimit lim;
	
	if( getrlimit( RLIMIT_NOFILE, &lim ) == 0 )
	{
		*filecount = (long) lim.rlim_max;
		return 0;
	}
	else
		return errno;
	#else
	return 0;
	#endif
}

double random( void )
{
	const double numerator = float( rand() );
    	const double denominator = float( RAND_MAX );
        const double randomNum = numerator / denominator;
    	return randomNum;
}

