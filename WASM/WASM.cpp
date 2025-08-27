#include <emscripten/bind.h>

#include "../C++/SliP.hpp"

string
json_escape( string const& _ ) {
	ostringstream $;
	for( char c : _ ) {
		switch ( c ) {
		case '\"': $ << "\\\""; break;
		case '\\': $ << "\\\\"; break;
		case '\n': $ << "\\n"; break;
		case '\r': $ << "\\r"; break;
		case '\t': $ << "\\t"; break;
		default:
			if ('\x00' <= c && c <= '\x1f') {
				$ << "\\u" << hex << uppercase << (int)c;
			} else {
				$ << c;
			}
		}
	}
	return $.str();
}

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

	try {
		return R"( { "response": ")" + json_escape( Eval( C, Read( R, -1 ) )->REPR() ) + R"(" })";
	} catch ( exception const& e ) {
		return R"( { "error": ")" + json_escape( e.what() ) + R"(" })";
	}
}

EMSCRIPTEN_BINDINGS( my_module ) {
	emscripten::function( "INIT", &INIT );
	emscripten::function( "REP", &REP );
}

