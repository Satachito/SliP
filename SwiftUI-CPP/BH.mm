#include "SliP.hpp"

//	Read / ReadList are not declared in SliP.hpp; every consumer declares what it
//	uses, and this file had drifted into declaring neither.
extern SP< SliP >
Read( iReader&, char32_t );

extern V< SP< SliP > >
ReadList( iReader&, char32_t );

extern void
Build();

//	CLI, TEST and WASM each call Build() once from their own single entry point.
//	The bridge has no such point — either loop may be called first, and either may
//	be called again — so latch it on first use.  A file-scope initializer would be
//	a static-init-order race against BUILTINS in SliP.cpp.
static void
BuildOnce() {
	static auto
	$ = ( Build(), true );
	(void)$;
}

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
	BuildOnce();
	StringReader					R( _ );
	vector< SP< SliP > >			slips;
	while( auto _ = Read( R, -1 ) ) slips.push_back( _ );

	return EvalSliPs( slips );
}

vector< string >
SugaredSyntaxLoop( string const& _ ) {
	BuildOnce();
	return EvalSliPs(
		project(
			Split( _ )
		,	[ & ]( string const& line ) {
				//	Strip the comment before appending ')', or the reader — which
				//	now understands `//` — would swallow the synthetic close
				//	paren along with the comment.  The web UI strips in the same
				//	order, for the same reason.
				auto					$ = line.substr( 0, line.find( "//" ) );
				StringReader			R( $ + ')' );
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

