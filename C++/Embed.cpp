#include "Embed.hpp"

extern SP< SliP >		Read( iReader&, char32_t );
extern V< SP< SliP > >	ReadList( iReader&, char32_t );
extern SP< SliP >		Eval( SP< Context >, SP< SliP > );
extern void				ClearStack();

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

string
Version() {
	return SLIP_VERSION;
}

static SP< Context >
theContext = MS< Context >();

void
Reset() {
	theContext = MS< Context >();
	ClearStack();
}

void
SetRoundPrecision( int _ ) {
	extern int RoundPrecision;
	RoundPrecision = _;
}

static string
Entry( string const& source, string const& key, string const& value ) {
	return ( source.size() ? R"({ "source": ")" + json_escape( source ) + R"(", ")" : R"({ ")" )
	+	key
	+	R"(": ")"
	+	json_escape( value )
	+	R"(" })"
	;
}

string
REP( string const& _ ) {
	StringReader	R( _ );
	try {
		auto form = Read( R, -1 );
		if( !form ) return R"({ "response": "" })";
		auto source = form->REPR();
		//	Evaluate before building the entry: taking the source and the
		//	response in one expression once let a throw leave the entry
		//	half-built.  See the REPL delimiter note below.
		auto response = Eval( theContext, form )->REPR();
		return Entry( source, "response", response );
	} catch ( exception const& e ) {
		return Entry( "", "error", e.what() );
	}
}

//	Shared by REPL and Sugared: both walk a sequence of forms and report per
//	form.  They differ in how forms are produced, and in what a failure means.
//
//	Programming mode stops: a later form usually depends on an earlier one, so
//	continuing past a failure reports errors that are only consequences of the
//	first.  Calculator mode continues: the lines are largely independent, and a
//	typo on line 1 should not hide the answer on line 6.  The web calculator has
//	always behaved this way, one try per line.
template < typename F > static string
Each( F NextForm, bool stopOnError ) {
	string	$ = "[";
	auto	count = 0;
	auto	Delimiter = [ & ]() -> string { return count++ ? "," : ""; };

	while( true ) {
		string	source;
		try {
			auto form = NextForm();
			if( !form ) break;
			source = form->REPR();
			//	Finish evaluating before the delimiter is taken.  Taking it
			//	inline let Eval throw after the count had already advanced, so
			//	the catch emitted a second delimiter and the array came back as
			//	"[,{ … }]" — not parseable as JSON.
			auto response = Eval( theContext, form )->REPR();
			$ += Delimiter() + Entry( source, "response", response );
		} catch ( exception const& e ) {
			$ += Delimiter() + Entry( source, "error", e.what() );
			if( stopOnError ) break;
		}
	}
	return $ + "]";
}

string
REPL( string const& _ ) {
	auto	R = MS< StringReader >( _ );
	return Each( [ R ]() { return Read( *R, -1 ); }, true );
}

//	Calculator mode: each non-empty line is read as the contents of a sentence,
//	so `2πr` behaves as `( 2πr )`.  Comments are stripped before the synthetic
//	close paren is appended, or the reader — which understands `//` — would
//	swallow the paren along with the comment.
string
Sugared( string const& _ ) {
	auto	lines = MS< V< string > >( Split( _ ) );
	auto	index = MS< size_t >( 0 );
	return Each(
		[ lines, index ]() -> SP< SliP > {
			while( *index < lines->size() ) {
				auto line = ( *lines )[ (*index)++ ];
				auto $ = line.substr( 0, line.find( "//" ) );
				if( $.find_first_not_of( " \t\r" ) == string::npos ) continue;
				StringReader	R( $ + ')' );
				return MS< Sentence >( ReadList( R, U')' ) );
			}
			return nullptr;
		}
	,	false
	);
}
