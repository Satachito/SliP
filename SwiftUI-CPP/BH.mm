#include "SliP.hpp"

char**
Bridge( const vector< string >& reprs, size_t* oCount ) {

	*oCount = reprs.size();

	auto
	$ = new char*[ *oCount ];

	for( size_t _ = 0; _ < *oCount; _++ ) {
		auto repr = reprs[ _ ];
		$[ _ ] = new char[ repr.length() + 1 ];
		strcpy( $[ _ ], repr.c_str() );
	}
	return $;
}

extern "C" char**
BH_Simple( const char *input, size_t* oCount ) {
	return Bridge( Simple( string( input ) ), oCount );
}

extern "C" char**
BH_Sugared( const char *input, size_t* oCount ) {
	return Bridge( Sugared( string( input ) ), oCount );

}

extern "C" void
FreeREPRs( const char** reprs, size_t count ) {
	for ( size_t _ = 0; _ < count; ++_ ) delete[] reprs[ _ ];
	delete[] reprs;
}

