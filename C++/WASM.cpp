#include <emscripten/bind.h>

#include "SliP.hpp"

auto
C = MS< Context >();

string
REP( string const& _ ) {

	StringReader
	R( _ );

	extern SP< SliP >
	Read( iReader&, char32_t );

	extern SP< SliP >
	Eval( SP< Context >, SP< SliP > );

	return Eval( C, Read( R, -1 ) )->REPR();
}

void
INIT() {
	cerr << "INIT" << endl;
}

EMSCRIPTEN_BINDINGS( SliP ) {
    emscripten::function( "init", &INIT );

	extern void Build();
    emscripten::function( "build", &Build );

    emscripten::function( "rep", &REP );
}
