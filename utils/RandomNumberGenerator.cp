#include "RandomNumberGenerator.h"

#include <assert.h>
#include <stdlib.h>
#include <gsl/gsl_rng.h>

#include "misc.h"

RandomNumberGenerator::Type RandomNumberGenerator::types[];

namespace __RandomNumberGenerator
{
	class ModuleInit
	{
	public:
		ModuleInit()
		{
			RandomNumberGenerator::init();
		}
	} init;
}

void RandomNumberGenerator::set( Role role,
								 Type type )
{
	types[role] = type;
}

RandomNumberGenerator *RandomNumberGenerator::create( Role role )
{
	return new RandomNumberGenerator( types[role] );
}

void RandomNumberGenerator::dispose( RandomNumberGenerator *rng )
{
	delete rng;
}

void RandomNumberGenerator::init()
{
	for( Role role = (Role)0;
		 role < __NROLES;
		 role = (Role)(role + 1) )
	{
		set( role, GLOBAL );
	}
}

RandomNumberGenerator::RandomNumberGenerator( Type type )
{
	this->type = type;

	switch( type )
	{
	case LOCAL:
		state = gsl_rng_alloc( gsl_rng_mt19937 );
		break;
	case GLOBAL:
		state = NULL;
		break;
	default:
		assert( false );
	}
}

RandomNumberGenerator::~RandomNumberGenerator()
{
	switch( type )
	{
	case LOCAL:
		gsl_rng_free( (gsl_rng *)state );
		break;
	case GLOBAL:
		// no-op
		break;
	}
}

void RandomNumberGenerator::seed( long x )
{
	switch( type )
	{
	case LOCAL:
		gsl_rng_set( (gsl_rng *)state,
					 x );
		break;
	case GLOBAL:
        {
		#ifdef linux
		srand48( x );
		#endif

		#ifdef WIN32
		srand( x );
		#endif

		break;
	}
	default:
		assert( false );
	}
}

double RandomNumberGenerator::drand()
{
	switch( type )
	{
	case LOCAL:
		return gsl_rng_uniform( (gsl_rng *)state );
	case GLOBAL:
		//return drand48();
		#ifdef linux
		       return rand48();
		#endif

		#ifdef WIN32
		       return double( rand() ) / double( RAND_MAX );
		#endif

		break;
	default:
		assert( false );
	}
}

double RandomNumberGenerator::range( double lo,
									 double hi )
{
	return interp( drand(),
				   lo,
				   hi );
}
