#include <emscripten/bind.h>

#include "SliP.hpp"

void
SugaredSyntaxLoop() {
}

EMSCRIPTEN_BINDINGS( SliP ) {
    emscripten::function( "rep", &SugaredSyntaxLoop );
}
