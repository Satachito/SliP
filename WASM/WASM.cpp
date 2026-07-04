#include "../C++/SliP.hpp"

#include <emscripten/bind.h>

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
				$ << "\\u" << hex << uppercase << setw( 4 ) << setfill( '0' ) << (int)c;
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
SetRoundPrecision( int _ ) {
	extern int RoundPrecision;
	RoundPrecision = _;
}

void
INIT() {
	extern void Build();
	Build();
	extern void BuildJS();
	BuildJS();
}

extern SP< SliP >
Read( iReader&, char32_t );

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

string
REP( string const& _ ) {

	StringReader
	R( _ );

	try {
		return R"({ "response": ")"
		+	json_escape( Eval( C, Read( R, -1 ) )->REPR() )
		+	R"(" })"
		;
	} catch ( exception const& e ) {
		return R"({ "error": ")"
		+ 	json_escape( e.what() )
		+	R"(" })"
		;
	}
}

//	Program mode: evaluate every toplevel expression in the source.
//	Returns a JSON array of { "source": REPR of the expression read, "response": REPR of the result }.
//	On error the last element carries "error" ( with "source" if the expression was read ) and evaluation stops.
string
REPL( string const& _ ) {

	StringReader
	R( _ );

	string	$ = "[";
	auto	count = 0;

	auto
	Delimiter = [ & ]() -> string { return count++ ? "," : ""; };

	while( true ) {
		string source;
		try {
			auto S = Read( R, -1 );
			if( !S ) break;
			source = S->REPR();
			$ += Delimiter()
			+	R"({ "source": ")"	+ json_escape( source )
			+	R"(", "response": ")"	+ json_escape( Eval( C, S )->REPR() )
			+	R"(" })"
			;
		} catch ( exception const& e ) {
			$ += Delimiter()
			+	( source.size() ? R"({ "source": ")" + json_escape( source ) + R"(", "error": ")" : R"({ "error": ")" )
			+	json_escape( e.what() )
			+	R"(" })"
			;
			break;
		}
	}
	return $ + "]";
}

EMSCRIPTEN_BINDINGS( my_module ) {
	emscripten::function( "INIT", &INIT );
	emscripten::function( "SetRoundPrecision", &SetRoundPrecision );
	emscripten::function( "REP", &REP );
	emscripten::function( "REPL", &REPL );
}
