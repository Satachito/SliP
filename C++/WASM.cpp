#include <emscripten/bind.h>

#include "SliP.hpp"

EMSCRIPTEN_BINDINGS( SliP ) {
    emscripten::function( "rep", &ReadEvalPrint );
}
