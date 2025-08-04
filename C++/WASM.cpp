#include <emscripten/bind.h>

#include "SliP.hpp"

auto
C = MS< Context >();

void
INIT() {
    extern void Build();
    Build();
    std::cout << "INIT" << std::endl;
}

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

EMSCRIPTEN_BINDINGS( my_module ) {
    emscripten::function( "init", &INIT );
    emscripten::function( "rep", &REP );
}

