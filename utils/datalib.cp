#include "datalib.h"

#include <stdlib.h>

using namespace datalib;
using namespace std;
using namespace std::tr1;
using namespace std::tr1::placeholders;

#define SIGNATURE "#datalib\n"
#define VERSION_STR "#version="
#define VERSION_MIN 2
#define VERSION 2

char *rfind( char *begin, char *end, char c )
{
	for( char *p = end - 1; p != end; p-- )
	{
		if( *p == c )
		{
			return p;
		}
	}

	return NULL;
}

// ================================================================================
// ===
// === CLASS __Column
// ===
// ================================================================================
__Column::__Column()
{
	name = "";
	type = INVALID;
	tname = format = NULL;
}

__Column::__Column( const char *name,
					datalib::Type type )
{
	this->name = name;
	this->type = type;

	switch( type )
	{
	case datalib::INT:
		tname = "int";
		format = "%-20d";
		break;
	case datalib::FLOAT:
		tname = "float";
		format = "%-20f";
		break;
	case datalib::STRING:
		tname = "string";
		format = "%-20s";
		break;
	default:
		assert( false );
	}
}


// ================================================================================
// ===
// === CLASS DataLibWriter
// ===
// ================================================================================

// ------------------------------------------------------------
// --- ctor()
// ------------------------------------------------------------
DataLibWriter::DataLibWriter( const char *path )
{
	f = fopen( path, "w" );

	if( f == NULL )
	{
		assert( false );
	}

	assert( f );

	table = NULL;

	fileHeader();
}

// ------------------------------------------------------------
// --- dtor()
// ------------------------------------------------------------
DataLibWriter::~DataLibWriter()
{
	if( table )
	{
		endTable();
	}

	fileFooter();

	fclose( f );
	f = NULL;
}

// ------------------------------------------------------------
// --- beginTable()
// ------------------------------------------------------------
void DataLibWriter::beginTable( const char *name,
								const char *colnames[],
								const datalib::Type coltypes[] )
{
	assert( table == NULL );

	tables.push_back( __Table(name) );
	table = &tables.back();

	table->offset = ftell( f );

	cols.clear();

	for( int i = 0; true; i++ )
	{
		const char *colname = colnames[i];
		if( colname == NULL )
		{
			break;
		}

		cols.push_back( __Column(colname,
								 coltypes[i]) );
	}

	tableHeader();

	table->data = ftell( f );
}

// ------------------------------------------------------------
// --- addRow()
// ------------------------------------------------------------
void DataLibWriter::addRow( Variant col0, ... )
{
	assert( table );

	table->nrows++;

	va_list args;
	char buf[4096];
	char *b = buf;
	bool first = true;
		
	sprintf( buf, "    " );
	b += strlen( buf );

	itfor( __ColVector, cols, it )
	{
		// floats are automatically promoted to double by ...
		// so we have to differentiate between actual type and
		// vararg type
#define DATA(VATYPE,CTYPE)								\
		{												\
			CTYPE val;									\
			if( first )									\
			{											\
				val = (CTYPE)col0;						\
				va_start( args, col0 );					\
			}											\
			else										\
			{											\
				val = (CTYPE)va_arg( args, VATYPE );	\
			}											\
														\
			sprintf( b,									\
					 it->format,						\
					 val );								\
		}


		switch( it->type )
		{
		case datalib::INT:
			DATA(int,int);
			break;
		case datalib::FLOAT:
			DATA(double,float);
			break;
		case datalib::STRING:
			DATA(const char *,const char *);
			break;
		default:
			assert( false );
		}

#undef DATA

		b += strlen( b );
		assert( size_t(b - buf) < sizeof(buf) );

		first = false;
	}

	*(b++) = '\n';
	assert( size_t(b - buf) <= sizeof(buf) );

	size_t nwrite = b - buf;

	// enforce fixed-length records
	if( table->rowlen == 0 )
	{
		table->rowlen = nwrite;
	}
	else
	{
		assert( nwrite == table->rowlen );
	}

	size_t n = fwrite( buf, 1, nwrite, f );
	assert( n == nwrite );
}

// ------------------------------------------------------------
// --- endTable()
// ------------------------------------------------------------
void DataLibWriter::endTable()
{
	assert( table );
		
	tableFooter();

	table = NULL;
}

// ------------------------------------------------------------
// --- fileHeader()
// ------------------------------------------------------------
void DataLibWriter::fileHeader()
{
	fprintf( f,
			 SIGNATURE
			 "%s%d\n",
			 VERSION_STR,
			 VERSION );
}

// ------------------------------------------------------------
// --- fileFooter()
// ------------------------------------------------------------
void DataLibWriter::fileFooter()
{
	size_t digest_start = ftell( f );

	fprintf( f, "#TABLES %lu\n", tables.size() );

	itfor( __TableVector, tables, it )
	{
		fprintf( f,
				 "# %s %lu %lu %lu %lu\n",
				 it->name.c_str(), it->offset, it->data, it->nrows, it->rowlen );
	}

	size_t digest_end = ftell( f );

	fprintf( f,
			 "#START %lu\n",
			 digest_start );

	fprintf( f,
			 "#SIZE %lu",
			 digest_end - digest_start );
}

// ------------------------------------------------------------
// --- tableHeader()
// ------------------------------------------------------------
void DataLibWriter::tableHeader()
{
	fprintf( f, "\n#<%s>\n", table->name.c_str() );
		
	// ---
	// --- colnames
	// ---
	fprintf( f, "#@L " );

	itfor( __ColVector, cols, it )
	{
		fprintf( f, "%-20s", it->name.c_str() );
	}

	fprintf( f, "\n"
			 "#\n" );

	// ---
	// --- coltypes
	// ---
	fprintf( f, "#@T " );

	itfor( __ColVector, cols, it )
	{
		fprintf( f, "%-20s", it->tname );
	}

	fprintf( f, "\n"
			 "#\n" );
}

// ------------------------------------------------------------
// --- tableFooter()
// ------------------------------------------------------------
void DataLibWriter::tableFooter()
{
	fprintf( f, "#</%s>\n\n", table->name.c_str() );
}


// ================================================================================
// ===
// === CLASS DataLibReader
// ===
// ================================================================================

// ------------------------------------------------------------
// --- ctor()
// ------------------------------------------------------------
DataLibReader::DataLibReader( const char *path )
{
	f = fopen( path, "r" );
	assert( f );
		
	table = NULL;
	this->path = path;

	parseHeader();
	parseDigest();
}

// ------------------------------------------------------------
// --- dtor()
// ------------------------------------------------------------
DataLibReader::~DataLibReader()
{
	fclose( f );
}

// ------------------------------------------------------------
// --- seekTable()
// ------------------------------------------------------------
bool DataLibReader::seekTable( const char *name )
{
	__TableMap::iterator it = tables.find( name );
	if( it == tables.end() )
	{
		table = NULL;
		return false;
	}

	table = &(it->second);
	row = -1;

	parseTableHeader();

	return true;
}

// ------------------------------------------------------------
// --- rewindTable()
// ------------------------------------------------------------
void DataLibReader::rewindTable()
{
	row = -1;
}

// ------------------------------------------------------------
// --- nrows()
// ------------------------------------------------------------
size_t DataLibReader::nrows()
{
	assert( table );

	return table->nrows;
}

// ------------------------------------------------------------
// --- seekRow()
// ------------------------------------------------------------
void DataLibReader::seekRow( int index )
{
	if( index < 0 )
	{
		index = table->nrows + index;
	}

	assert( (index >= 0) && ((size_t)index < table->nrows) );

	if( index == row )
	{
		return;
	}
	row = index;

	// ---
	// --- Read the row text
	// ---
	SYS( fseek(f,
			   table->data + ( index * table->rowlen ),
			   SEEK_SET) );

	char rowbuf[table->rowlen];

	size_t n = fread( rowbuf,
					  1,
					  table->rowlen,
					  f );
	assert( n == table->rowlen );

	// ---
	// --- Parse the row
	// ---
	struct local
	{
		static void add_col( __ColVector::iterator &it,
							 const char *start,
							 const char *end )
		{
			__Column &col = *it;

			switch( col.type )
			{
			case INT:
				col.rowdata = atoi(start);
			break;
			case FLOAT:
				col.rowdata = atof(start);
				break;
			case STRING: {
				char *_end = const_cast<char *>(end);
				char e = *_end;
				*_end = '\0';
				col.rowdata = start; // makes a strdup
				*_end = e;
				break;
			}
			default:
				assert(false);
			}

			it++;
		}
	};

	parseLine( rowbuf,
			   bind(local::add_col, cols.begin(), _1, _2) );
}

// ------------------------------------------------------------
// --- nextRow()
// ------------------------------------------------------------
bool DataLibReader::nextRow()
{
	int next = row + 1;

	if( (size_t)next >= table->nrows )
	{
		return false;
	}

	seekRow( next );

	return true;
}

// ------------------------------------------------------------
// --- col()
// ------------------------------------------------------------
const Variant &DataLibReader::col( const char *name )
{
	if( row == -1 )
	{
		seekRow( 0 );
	}

	__ColMap::iterator it = colmap.find(name);
	assert( it != colmap.end() );

	__Column *col = it->second;

	return col->rowdata;
}

// ------------------------------------------------------------
// --- parseHeader()
// ------------------------------------------------------------
void DataLibReader::parseHeader()
{
	char buf[128];

	size_t n = fread( buf, 1, sizeof(buf) - 1, f );
	assert( n > 0 );

	buf[n] = '\0';

	size_t len;

	len = strlen( SIGNATURE );
	assert( 0 == strncmp(buf, SIGNATURE, len) );

	char *version = buf + len;
	len = strlen( VERSION_STR );
	assert( 0 == strncmp(version, VERSION_STR, len) );

	int iversion = atoi(version + len);

	assert( (iversion >= VERSION_MIN) && (iversion <= VERSION) );
}

// ------------------------------------------------------------
// --- parseDigest()
// ------------------------------------------------------------
void DataLibReader::parseDigest()
{
	// ---
	// --- Read end of file
	// ---
	char buf[64];
	size_t n = sizeof(buf) - 1;

	SYS( fseek(f, -n, SEEK_END) );

	n = fread( buf,
			   1,
			   n,
			   f );
	assert( n > 0 );
	buf[n] = '\0';
	for( size_t i = n - 1; i >= 0; i++ )
	{
		if( buf[i] == '\n' )
		{
			buf[i] = '\0';
			n = i;
		}
		else
		{
			break;
		}
	}

	// ---
	// --- Parse start & size
	// ---
	char *line_size = rfind( buf, buf + n, '\n' ) + 1;
	char *line_start = rfind( buf, line_size - 1, '\n' ) + 1;

	size_t size;
	sscanf( line_size,
			"#SIZE %lu",
			&size );

	size_t start;
	sscanf( line_start,
			"#START %lu",
			&start );

	// ---
	// --- Read digest
	// ---
	char digest[size + 1];
	SYS( fseek(f, start, SEEK_SET) );

	n = fread( digest,
			   1,
			   size,
			   f );
	assert( n == size );
	digest[size] = '\0';

	// ---
	// --- Parse digest
	// ---

	// --- number of tables
	size_t ntables;
	sscanf( digest,
			"#TABLES %lu",
			&ntables );

	// --- table info
	char *line = digest;
	for( size_t i = 0; i < ntables; i++ )
	{
		line = 1 + strchr( line, '\n' );

		char name[256];
		__Table table;

		sscanf( line,
				"# %s %lu %lu %lu %lu",
				name, &table.offset, &table.data, &table.nrows, &table.rowlen );

		table.name = name;

		tables[name] = table;
	}
}

// ------------------------------------------------------------
// --- parseTableHeader()
// ------------------------------------------------------------
void DataLibReader::parseTableHeader()
{
	SYS( fseek(f,
			   table->offset,
			   SEEK_SET) );

	char buf[1025];
	size_t n = fread( buf,
					  1,
					  sizeof(buf) - 1,
					  f );
	buf[n] = '\0';

	char *line = buf;

#define NEXT() line = strchr( line, '\n' ) + 1;

	// ---
	// --- Parse Names
	// ---
	NEXT();
	NEXT();

	vector<string> names;
	struct local0
	{
		static void add_name( vector<string> *names,
							  const char *start,
							  const char *end )
		{
			if( *start == '#' ) return;
			names->push_back( string(start, end - start) );
		}
	};

	parseLine( line,
			   bind(local0::add_name, &names, _1, _2) );

	// ---
	// --- Parse Types
	// ---
	NEXT();
	NEXT();

	vector<datalib::Type> types;
	struct local1
	{
		static void add_type( vector<datalib::Type> *types,
							  const char *start,
							  const char *end )
		{
			if( *start == '#' ) return;

			unsigned int n = end - start;
			if( 0 == strncmp(start, "int", n) )
			{
				types->push_back( datalib::INT );
			}
			else if( 0 == strncmp(start, "float", n) )
			{
				types->push_back( datalib::FLOAT );
			}
			else if( 0 == strncmp(start, "string", n) )
			{
				types->push_back( datalib::STRING );
			}
			else
			{
				assert( false );
			}
		}
	};

	parseLine( line,
			   bind(local1::add_type, &types, _1, _2) );

#undef NEXT

	assert( types.size() == names.size() );

	// ---
	// --- Create Columns
	// ---
	cols.clear();
	colmap.clear();

	for( size_t i = 0, n = types.size(); i < n; i++ )
	{
		__Column col( names[i].c_str(),
					  types[i] );

		cols.push_back( col );
	}

	itfor( __ColVector, cols, it )
	{
		__Column *pcol = &(*it);

		colmap[pcol->name] = pcol;
	}
}

// ------------------------------------------------------------
// --- parseLine()
// ------------------------------------------------------------
void DataLibReader::parseLine( const char *line,
							   function< void (const char *start,
											   const char *end)> callback )
{
	const char *start = NULL;

	for( const char *curr = line; true; curr++ )
	{
		char c = *curr;

		if( c == '\n' || c == '\0' )
		{
			if(start)
			{
				callback( start, curr );
			}
			break;
		}

		if( start )
		{
			if( c == ' ' || c == '\t' )
			{
				callback( start, curr );
				start = NULL;
			}
		}
		else
		{
			if( c != ' ' && c != '\t' )
			{
				start = curr;
			}
		}
	}
}
