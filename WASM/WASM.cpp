#include "../C++/Embed.hpp"

#include <emscripten/bind.h>

//	Everything here is a binding.  The evaluation session, the JSON contract and
//	the error handling live in C++/Embed.cpp, shared with the macOS bridge, so
//	the browser and the app cannot drift apart.

static void
INIT() {
	extern void Build();
	Build();
	extern void BuildJS();		//	Canvas / WebGL operators; browser only
	BuildJS();
}

EMSCRIPTEN_BINDINGS( my_module ) {
	emscripten::function( "VERSION", &Version );
	emscripten::function( "INIT", &INIT );
	emscripten::function( "SetRoundPrecision", &SetRoundPrecision );
	emscripten::function( "ResetContext", &Reset );
	emscripten::function( "REP", &REP );
	emscripten::function( "REPL", &REPL );
	emscripten::function( "Sugared", &Sugared );
}
