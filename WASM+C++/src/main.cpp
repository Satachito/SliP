#include <emscripten/bind.h>

#include "SliP.hpp"

shared_ptr< SliP >
ReadEvalPrint( const string& _ ) {
	return Print( Eval( make_shared< Context >(), Read( _ ) ) );
}

EMSCRIPTEN_BINDINGS( SliP ) {
    emscripten::function( "rep", &ReadEvalPrint );
}
