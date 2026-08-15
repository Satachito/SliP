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

struct
EmbedSession {
	SP< Context >	context = MS< Context >();
};

static EmbedSession
theSession;

void
Reset() {
	theSession.context = MS< Context >();
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
		auto response = Eval( theSession.context, form )->REPR();
		return Entry( source, "response", response );
	} catch ( exception const& e ) {
		return Entry( "", "error", e.what() );
	}
}

//	Shared by every mode and every host: walk a sequence of forms, reporting per
//	form.  They differ in how forms are produced, and in what a failure means.
//
//	Programming mode stops: a later form usually depends on an earlier one, so
//	continuing past a failure reports errors that are only consequences of the
//	first.  Calculator mode continues: the lines are largely independent, and a
//	typo on line 1 should not hide the answer on line 6.  The web calculator has
//	always behaved this way, one try per line.
template < typename F > static V< EmbedEntry >
Each( SP< Context > context, F NextForm, bool stopOnError ) {
	V< EmbedEntry >	$;

	while( true ) {
		EmbedEntry	entry;
		try {
			auto form = NextForm();
			if( !form ) break;
			entry.source = form->REPR();
			entry.value  = Eval( context, form )->REPR();
		} catch ( exception const& e ) {
			entry.error  = e.what();
			entry.failed = true;
		}
		$.push_back( entry );
		if( entry.failed && stopOnError ) break;
	}
	return $;
}

//	Producing the forms: the two ways a host can mean a piece of source.
static auto
Forms( SP< StringReader > R ) {
	return [ R ]() { return Read( *R, -1 ); };
}

//	Calculator mode: each non-empty line is read as the contents of a sentence,
//	so `2πr` behaves as `( 2πr )`.  Comments are stripped before the synthetic
//	close paren is appended, or the reader — which understands `//` — would
//	swallow the paren along with the comment.
static auto
Lines( SP< V< string > > lines, SP< size_t > index ) {
	return [ lines, index ]() -> SP< SliP > {
		while( *index < lines->size() ) {
			auto line = ( *lines )[ (*index)++ ];
			auto $ = line.substr( 0, line.find( "//" ) );
			if( $.find_first_not_of( " \t\r" ) == string::npos ) continue;
			StringReader	R( $ + ')' );
			return MS< Sentence >( ReadList( R, U')' ) );
		}
		return nullptr;
	};
}

//	The JSON is this list written out.  The delimiter is taken as each entry is
//	appended and never while an evaluation is in flight: taking it inline once
//	let Eval throw after the count had already advanced, so the catch emitted a
//	second delimiter and the array came back as "[,{ … }]" — not parseable.
static string
JSON( V< EmbedEntry > const& _ ) {
	string	$ = "[";
	auto	count = 0;
	for( auto const& entry: _ ) {
		if( count++ ) $ += ",";
		$ += entry.failed
		?	Entry( entry.source, "error", entry.error )
		:	Entry( entry.source, "response", entry.value )
		;
	}
	return $ + "]";
}

string
REPL( string const& _ ) {
	return JSON( Each( theSession.context, Forms( MS< StringReader >( _ ) ), true ) );
}

string
Sugared( string const& _ ) {
	return JSON(
		Each( theSession.context, Lines( MS< V< string > >( Split( _ ) ), MS< size_t >( 0 ) ), false )
	);
}

EmbedSession*
NewEmbedSession() {
	return new EmbedSession;
}

void
DeleteEmbedSession( EmbedSession* session ) {
	delete session;
}

void
ResetEmbedSession( EmbedSession* session ) {
	if( !session ) return;
	session->context = MS< Context >();
	ClearStack();
}

V< EmbedEntry >
SessionRun( EmbedSession* session, string const& source, bool calculator ) {
	if( !session ) return { EmbedEntry{ "", "", "Missing interpreter session", true } };
	return calculator
	?	Each( session->context, Lines( MS< V< string > >( Split( source ) ), MS< size_t >( 0 ) ), false )
	:	Each( session->context, Forms( MS< StringReader >( source ) ), true )
	;
}

string
SessionREPL( EmbedSession* session, string const& source ) {
	return JSON( SessionRun( session, source, false ) );
}

string
SessionSugared( EmbedSession* session, string const& source ) {
	return JSON( SessionRun( session, source, true ) );
}
