#include "SliP.hpp"

char**
Bridge( vector< string > const& reprs, size_t* oCount ) {

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

template< ranges::range R > vector< string >
EvalSliPs( R&& _ ) {
	auto							C = MS< Context >();
	return ranges::to< vector >(
		project(
			_
		,	[ & ]( SP< SliP > const& slip ) {
				return Eval( C, slip )->REPR();
			}
		)
	);
}

vector< string >
CoreSyntaxLoop( string const& _ ) {
	StringReader					R( _ );
	vector< SP< SliP > >			slips;
	while( auto _ = Read( R, -1 ) ) slips.push_back( _ );

	return EvalSliPs( slips );
}

vector< string >
SugaredSyntaxLoop( string const& _ ) {
	return EvalSliPs(
		project(
			Split( _ )
		,	[ & ]( string const& line ) {
				StringReader			R( line + ')' );
				return MS< Sentence	>( ReadList( R, U')' ) );
			}
		)
	);
}

extern "C" char**
BH_CoreSyntaxLoop( const char *input, size_t* oCount ) {
	return Bridge( CoreSyntaxLoop( string( input ) ), oCount );
}

extern "C" char**
BH_SugaredSyntaxLoop( const char *input, size_t* oCount ) {
	return Bridge( SugaredSyntaxLoop( string( input ) ), oCount );

}

extern "C" void
BH_FreeREPRs( char** reprs, size_t count ) {
	for ( size_t _ = 0; _ < count; ++_ ) delete[] reprs[ _ ];
	delete[] reprs;
}

